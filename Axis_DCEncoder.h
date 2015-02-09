/*
 * Copyright (C) 2014, Jason S. McMullan
 * All right reserved.
 * Author: Jason S. McMullan <jason.mcmullan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef AXIS_DCENCODER_H
#define AXIS_DCENCODER_H

#include <Wire.h>
#include <AFMotor.h>
#include <Encoder.h>

#include "Axis.h"

class Axis_DCEncoder : public Axis {
    private:
        static const int _overshoot = 10;

        int _minPos;
        int _maxPos;
        int _pinEncoderA;
        int _pinEncoderB;
        uint8_t _pwmMinimum;
        uint8_t _pwmMaximum;

        float _mm_to_position;

        AF_DCMotor _motor;
        Encoder _encoder;

        enum {
            IDLE,
            MOVING, MOVING_OVERSHOOT,
            HOMING_STOP,
            HOMING_STOP_QUIESCE,
            HOMING_STOP_BACKOFF,
            HOMING_STALL,
            HOMING_STALL_QUIESCE,
            HOMING_STALL_BACKOFF
        } _mode;
        struct {
            unsigned long timeout;
            int overshoot;
        } _moving;
        struct {
            int32_t home, position, target;
            uint32_t timeout;
            enum axis_stop_e pin;
            int dir;
            uint8_t pwm;
        } _homing;

    public:
        Axis_DCEncoder(int afmotor, uint8_t pwm_min, uint8_t pwm_max,
                       int enc_a, int enc_b,
                       int stop_min, int stop_max,
                       float mm, int32_t minpos, int32_t maxpos) :
            Axis(stop_min, stop_max),
            _motor(afmotor),
            _encoder(enc_a, enc_b)
        {
            _minPos = minpos;
            _maxPos = maxpos;
            _pinEncoderA = enc_a;
            _pinEncoderB = enc_b;
            _pwmMinimum = pwm_min;
            _pwmMaximum = pwm_max;
            _mm_to_position = (float)(_maxPos - _minPos)/mm;

            if (stop_min >= 0) {
                _mode = HOMING_STOP;
                _homing.pin = Axis::STOP_MIN;
                _homing.dir = BACKWARD;
                _homing.home = _minPos - _overshoot;
                _homing.pwm = _pwmMaximum;
            } else if (stop_max >= 0) {
                _mode = HOMING_STOP;
                _homing.pin = Axis::STOP_MAX;
                _homing.dir = FORWARD;
                _homing.home = _maxPos + _overshoot;
                _homing.pwm = _pwmMaximum;
            } else {
                _mode = HOMING_STALL;
                _homing.pin = Axis::STOP_NONE;
                _homing.dir = BACKWARD;
                _homing.home = _minPos;
                _homing.pwm = (_pwmMinimum + _pwmMaximum) / 2;
            }
        }

        virtual void begin()
        {
            pinMode(_pinEncoderA, INPUT);
            pinMode(_pinEncoderB, INPUT);

            _encoder.write(0);
            Axis::begin();
        }

        virtual void home(int32_t pos)
        {
            _homing.target = pos;

            if (_homing.pin == Axis::STOP_NONE)
                _mode = HOMING_STALL;
            else
                _mode = HOMING_STOP;

            _motor.setSpeed(_homing.pwm);
            _motor.run(_homing.dir);
        }

        virtual float position_min()
        {
            return _minPos / _mm_to_position;
        }

        virtual float position_max()
        {
            return _maxPos / _mm_to_position;
        }

        virtual void motor_enable(bool enabled = true)
        {
            if (!enabled)
                _motor.run(RELEASE);
            Axis::motor_enable(enabled);
        }

        virtual bool motor_active()
        {
            return _mode != IDLE;
        }

        virtual float position_get(void)
        {
            return _encoder.read() / _mm_to_position;
        }

        virtual bool update(unsigned long ms_now)
        {
            int32_t pos = _encoder.read();
            int32_t tar = target_get() * _mm_to_position;

            if (tar >= _maxPos)
                tar = _maxPos - 1;

            if (tar < _minPos)
                tar = _minPos;

            switch (_mode) {
            case IDLE:
                if (tar != pos)
                    _mode = MOVING;
                break;
            case HOMING_STALL:
                _homing.timeout = ms_now+10;
                _homing.position = pos;
                _mode = HOMING_STALL_QUIESCE;
                break;
            case HOMING_STALL_QUIESCE:
                if (ms_now >= _homing.timeout) {
                    if (_encoder.read() == _homing.position) {
                        _motor.setSpeed(0);
                        _motor.run(RELEASE);
                        _homing.timeout = ms_now + 100;
                        _mode = HOMING_STALL_BACKOFF;
                       break;
                    }
                    _mode = HOMING_STALL;
                }
                break;
            case HOMING_STALL_BACKOFF:
                if (ms_now >= _homing.timeout) {
                    _motor.setSpeed(_homing.pwm);
                    _encoder.write(_homing.home);
                    Axis::home(_homing.target);
                    _mode = IDLE;
                }
                break;
            case HOMING_STOP:
                if (endstop(_homing.pin)) {
                    _homing.timeout = ms_now+1;
                    _mode = HOMING_STOP_QUIESCE;
                }
                break;
            case HOMING_STOP_QUIESCE:
                if (ms_now >= _homing.timeout) {
                    _mode = HOMING_STOP_BACKOFF;
                    _motor.run(RELEASE);
                    _motor.setSpeed(_pwmMinimum);
                    _motor.run(_homing.dir == FORWARD ? BACKWARD : FORWARD);
                    _homing.timeout = ms_now+10;
                    _homing.pwm = _pwmMinimum;
                    _mode = HOMING_STOP_BACKOFF;
                }
                break;
            case HOMING_STOP_BACKOFF:
                if (ms_now >= _homing.timeout) {
                    if (endstop(_homing.pin)) {
                        if (_homing.pwm < _pwmMaximum)
                            _homing.pwm++;
                        _motor.setSpeed(_homing.pwm);
                        _homing.timeout = ms_now+10;
                    } else {
                        _encoder.write(_homing.home);
                        _motor.setSpeed(0);
                        _mode = IDLE;
                        Axis::home(_homing.target);
                    }
                }
                break;
            case MOVING:
            case MOVING_OVERSHOOT:
                if (tar >= pos && endstop(Axis::STOP_MAX)) {
                    _encoder.write(_minPos);
                    _motor.setSpeed(0);
                    _mode = IDLE;
                    break;
                }

                if (tar <= pos && endstop(Axis::STOP_MIN)) {
                    _encoder.write(_maxPos);
                    _motor.setSpeed(0);
                    _mode = IDLE;
                    break;
                }

                int32_t distance = (pos > tar) ? (pos - tar) : (tar - pos);

                if (distance == 0) {
                    _motor.run(BRAKE);
                    _motor.setSpeed(0);
                    if (_mode == MOVING) {
                        _moving.timeout = ms_now + 1;
                        _moving.overshoot = _overshoot;
                        _mode = MOVING_OVERSHOOT;
                    } else {
                        if (ms_now < _moving.timeout)
                            break;
                        if (_moving.overshoot) {
                            _moving.overshoot--;
                            _moving.timeout = ms_now + 1;
                        } else {
                            _mode = IDLE;
                        }
                    }
                    break;
                }

                int speed;

                if (distance > 50)
                    speed = _pwmMaximum;
                else
                    speed = _pwmMinimum + distance;

                _motor.setSpeed(speed);
                _motor.run((pos < tar) ? FORWARD : BACKWARD);
                break;
                if (ms_now > _moving.timeout) {
                    if (_moving.overshoot) {
                        _moving.overshoot--;
                        _motor.run(BRAKE);
                        _motor.setSpeed(0);
                        _mode = MOVING;
                    }
                }
                break;
            }

             return (_mode == IDLE) ? false : true;
        }
};


#endif /* AXIS_DCENCODER_H */
/* vim: set shiftwidth=4 expandtab:  */

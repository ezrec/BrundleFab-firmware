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
        int _pinStopMin;
        int _pinStopMax;
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
            int pin, dir;
            uint8_t pwm;
        } _homing;

    public:
        Axis_DCEncoder(int afmotor, uint8_t pwm_min, uint8_t pwm_max,
                       int enc_a, int enc_b,
                       int stop_min, int stop_max,
                       float mm, int32_t minpos, int32_t maxpos) :
            Axis(),
            _motor(afmotor),
            _encoder(enc_a, enc_b)
        {
            _minPos = minpos;
            _maxPos = maxpos;
            _pinStopMin = stop_min;
            _pinStopMax = stop_max;
            _pinEncoderA = enc_a;
            _pinEncoderB = enc_b;
            _pwmMinimum = pwm_min;
            _pwmMaximum = pwm_max;
            _mm_to_position = (float)(_maxPos - _minPos)/mm;
        }

        virtual void begin()
        {
            pinMode(_pinEncoderA, INPUT);
            pinMode(_pinEncoderB, INPUT);
            if (_pinStopMin >= 0)
                pinMode(_pinStopMin, INPUT_PULLUP);
            if (_pinStopMax >= 0)
                pinMode(_pinStopMax, INPUT_PULLUP);

            _encoder.write(0);
            Axis::begin();
        }

        virtual void home(int32_t pos)
        {
            _homing.target = pos;

            if (_pinStopMin >= 0) {
                _mode = HOMING_STOP;
                _homing.pin = _pinStopMin;
                _homing.dir = BACKWARD;
                _homing.home = _minPos - _overshoot;
                _homing.pwm = _pwmMaximum;
            } else if (_pinStopMax >= 0) {
                _mode = HOMING_STOP;
                _homing.pin = _pinStopMax;
                _homing.dir = FORWARD;
                _homing.home = _maxPos + _overshoot;
                _homing.pwm = _pwmMaximum;
            } else {
                _mode = HOMING_STALL;
                _homing.dir = BACKWARD;
                _homing.home = _minPos;
                _homing.pwm = (_pwmMinimum + _pwmMaximum) / 2;
            }

            _motor.setSpeed(_homing.pwm);
            _motor.run(_homing.dir);
        }

        virtual const int32_t position_min()
        {
            return _minPos;
        }

        virtual const int32_t position_max()
        {
            return _maxPos;
        }

        virtual const float mm_to_position()
        {
            return _mm_to_position;
        }

        virtual void motor_enable(bool enabled = true)
        {
            if (!enabled)
                _motor.run(RELEASE);
            Axis::motor_enable(enabled);
        }

        virtual void motor_halt()
        {
            _motor.setSpeed(0);
            Axis::motor_halt();
        }

        virtual bool motor_active()
        {
            return _mode != IDLE;
        }

        virtual int32_t position_get(void)
        {
            return _encoder.read();
        }

        virtual bool update()
        {
            int32_t pos = position_get();
            int32_t tar = target_get();

            if (tar >= position_max())
                tar = position_max() - 1;

            if (tar < position_min())
                tar = position_min();

            switch (_mode) {
            case IDLE:
                if (tar != pos)
                    _mode = MOVING;
                break;
            case HOMING_STALL:
                _homing.timeout = millis()+10;
                _homing.position = pos;
                _mode = HOMING_STALL_QUIESCE;
                break;
            case HOMING_STALL_QUIESCE:
                if (millis() >= _homing.timeout) {
                    if (_encoder.read() == _homing.position) {
                        _motor.setSpeed(0);
                        _motor.run(RELEASE);
                        _homing.timeout = millis() + 100;
                        _mode = HOMING_STALL_BACKOFF;
                       break;
                    }
                    _mode = HOMING_STALL;
                }
                break;
            case HOMING_STALL_BACKOFF:
                if (millis() >= _homing.timeout) {
                    _motor.setSpeed(_homing.pwm);
                    _encoder.write(_homing.home);
                    Axis::home(_homing.target);
                    _mode = IDLE;
                }
                break;
            case HOMING_STOP:
                if (digitalRead(_homing.pin) == 1) {
                    _homing.timeout = millis()+1;
                    _mode = HOMING_STOP_QUIESCE;
                }
                break;
            case HOMING_STOP_QUIESCE:
                if (millis() >= _homing.timeout) {
                    _mode = HOMING_STOP_BACKOFF;
                    _motor.run(RELEASE);
                    _motor.setSpeed(_pwmMinimum);
                    _motor.run(_homing.dir == FORWARD ? BACKWARD : FORWARD);
                    _homing.timeout = millis()+10;
                    _homing.pwm = _pwmMinimum;
                    _mode = HOMING_STOP_BACKOFF;
                }
                break;
            case HOMING_STOP_BACKOFF:
                if (millis() >= _homing.timeout) {
                    if (digitalRead(_homing.pin) == 1) {
                        if (_homing.pwm < _pwmMaximum)
                            _homing.pwm++;
                        _motor.setSpeed(_homing.pwm);
                        _homing.timeout = millis()+10;
                    } else {
                        motor_halt();
                        _encoder.write(_homing.home);
                        _mode = IDLE;
                        Axis::home(_homing.target);
                    }
                }
                break;
            case MOVING:
            case MOVING_OVERSHOOT:
                if (_pinStopMin >= 0 && digitalRead(_pinStopMin) == 1) {
                    if (tar <= pos) {
                        _mode = IDLE;
                        _encoder.write(_minPos);
                        motor_halt();
                        break;
                    }
                }

                if (_pinStopMax >= 0 && digitalRead(_pinStopMax) == 1) {
                    if (tar >= pos) {
                        _mode = IDLE;
                        _encoder.write(_maxPos);
                        motor_halt();
                        break;
                    }
                }

                int32_t distance = (pos > tar) ? (pos - tar) : (tar - pos);

                if (distance == 0) {
                    _motor.run(BRAKE);
                    _motor.setSpeed(0);
                    if (_mode == MOVING) {
                        _moving.timeout = millis() + 1;
                        _moving.overshoot = _overshoot;
                        _mode = MOVING_OVERSHOOT;
                    } else {
                        if (millis() < _moving.timeout)
                            break;
                        if (_moving.overshoot) {
                            _moving.overshoot--;
                            _moving.timeout = millis() + 1;
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
                if (millis() > _moving.timeout) {
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

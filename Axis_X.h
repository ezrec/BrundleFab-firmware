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

#ifndef AXIS_X_H
#define AXIS_X_H

#include <Wire.h>
#include <AFMotor.h>
#include <Encoder.h>

#include "pinout.h"
#include "Axis.h"

class Axis_X : public Axis {
    private:
        static const int _overshoot = 10;
        static const int _adaMotor = 1;
        static const int _pinEncoderA = XENC_A;
        static const int _pinEncoderB = XENC_B;
        static const int _pinStopMax =  XSTP_MAX;

        static const int _pwmMinimum = 98;
        static const int _pwmMaximum = 255;

        static const int _maxPos = 7100;
        static const int _minPos = -4400;

       float _mm_to_position;

        AF_DCMotor _motor;
        Encoder _encoder;

        enum {
            IDLE,
            HOMING, HOMING_QUIESCE, HOMING_BACKOFF,
            MOVING, MOVING_OVERSHOOT,
        } mode;
        struct {
            unsigned long timeout;
            int speed;
        } _homing;
        struct {
            unsigned long timeout;
            int overshoot;
        } _moving;

    public:
        Axis_X() : Axis(),
                 _motor(_adaMotor), _encoder(_pinEncoderA, _pinEncoderB)
        {
                _mm_to_position = (float)(_maxPos - _minPos)/(float)(870 - 130);
        }
        virtual void begin()
        {
            pinMode(_pinEncoderA, INPUT_PULLUP);
            pinMode(_pinEncoderB, INPUT_PULLUP);
            pinMode(_pinStopMax, INPUT_PULLUP);

            Axis::begin();
        }

        virtual void home()
        {
            mode = HOMING;
            _motor.setSpeed(255);
            _motor.run(FORWARD);
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

        virtual void motor_disable()
        {
            _motor.run(RELEASE);
            Axis::motor_disable();
        }

        virtual void motor_halt()
        {
            _motor.setSpeed(0);
            Axis::motor_halt();
        }

        virtual void position_set(int32_t pos)
        {
            _encoder.write(pos);
        }

        virtual int32_t position_get(void)
        {
            return _encoder.read();
        }

        virtual bool update()
        {
            int32_t pos = position_get();
            int32_t tar = target_get();

            switch (mode) {
            case IDLE:
                if (tar != pos)
                    mode = MOVING;
                break;
            case HOMING:
                if (digitalRead(_pinStopMax) == 0) {
                    _homing.timeout = millis()+1;
                    mode = HOMING_QUIESCE;
                }
                break;
            case HOMING_QUIESCE:
                if (millis() >= _homing.timeout) {
                    mode = HOMING_BACKOFF;
                    _motor.run(RELEASE);
                    _motor.setSpeed(_pwmMinimum);
                    _motor.run(BACKWARD);
                    _homing.timeout = millis()+10;
                    _homing.speed = _pwmMinimum;
                    mode = HOMING_BACKOFF;
                }
                break;
            case HOMING_BACKOFF:
                if (millis() >= _homing.timeout) {
                    if (digitalRead(_pinStopMax) == 0) {
                        if (_homing.speed < _pwmMaximum)
                            _homing.speed++;
                        _motor.setSpeed(_homing.speed);
                        _homing.timeout = millis()+10;
                    } else {
                        motor_halt();
                        position_set(_maxPos);
                        target_set(0);
                        mode = IDLE;
                    }
                }
                break;
            case MOVING:
            case MOVING_OVERSHOOT:
                if (digitalRead(_pinStopMax) == 0) {
                    if (tar >= pos) {
                        mode = IDLE;
                        motor_halt();
                        break;
                    }
                }

                int32_t distance = (pos > tar) ? (pos - tar) : (tar - pos);

                if (distance == 0) {
                    _motor.run(BRAKE);
                    _motor.setSpeed(0);
                    if (mode == MOVING) {
                        _moving.timeout = millis() + 1;
                        _moving.overshoot = _overshoot;
                        mode = MOVING_OVERSHOOT;
                    } else {
                        if (millis() < _moving.timeout)
                            break;
                        if (_moving.overshoot) {
                            _moving.overshoot--;
                            _moving.timeout = millis() + 1;
                        } else {
                            mode = IDLE;
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
                        mode = MOVING;
                    }
                }
                break;
            }

             return (mode == IDLE) ? false : true;
        }
};


#endif /* AXIS_X_H */
/* vim: set shiftwidth=4 expandtab:  */

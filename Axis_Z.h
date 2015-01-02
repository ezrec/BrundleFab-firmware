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

#ifndef AXIS_Z_H
#define AXIS_Z_H

#include <Wire.h>
#include <Adafruit_MotorShield.h>

#include "Axis.h"

extern Adafruit_MotorShield AFMS;

class Axis_Z : public Axis {
    private:
        static const int _adaMotor = 1; /* 1 = M1/M2, 2 = M3/M4 */
        static const int _stepsPerRotation = 200; /* 1.8 degrees */
        static const float _mmPerRotation = 4.0;  /* 4mm pitch */
        static const int _pinStopMax = ZSTP_MAX;

        static const int _maxPos = 35000;
        static const int _minPos = 0;

        float _mm_to_position;
        int32_t _position;

        Adafruit_StepperMotor *_motor;

        enum {
            IDLE,
            HOMING, HOMING_QUIESCE,
            HOMING_BACKOFF,
            MOVING
        } mode;
        struct {
            unsigned long timeout;
            int32_t steps;
        } _homing;
        struct {
            unsigned long timeout;
        } _moving;

    public:
        Axis_Z() : Axis()
        {
            _motor = AFMS.getStepper(_stepsPerRotation, _adaMotor);
            _mm_to_position = _stepsPerRotation / _mmPerRotation;
            _position = 0;
        }

        virtual void begin()
        {
            _motor->setSpeed(200);       /* 200 RPM */
            pinMode(_pinStopMax, INPUT_PULLUP);
            Axis::begin();
        }

        virtual void home()
        {
            mode = HOMING;
        }

        virtual const float mm_to_position()
        {
            return _mm_to_position;
        }

        virtual const int32_t position_min()
        {
            return _minPos;
        }

        virtual const int32_t position_max()
        {
            return _maxPos;
        }
        virtual void motor_disable()
        {
            _motor->release();
            Axis::motor_disable();
        }

        virtual void position_set(int32_t pos)
        {
            _position = pos;
            Axis::position_set(pos);
        }

        virtual int32_t position_get(void)
        {
            return _position;
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
                } else {
                    _motor->step(_homing.steps, FORWARD, SINGLE);
                }
                break;
            case HOMING_QUIESCE:
                if (millis() >= _homing.timeout) {
                    mode = HOMING_BACKOFF;
                    _homing.timeout = millis()+10;
                    _homing.steps = _stepsPerRotation / _mmPerRotation;
                    mode = HOMING_BACKOFF;
                }
                break;
            case HOMING_BACKOFF:
                if (millis() >= _homing.timeout) {
                    if (digitalRead(_pinStopMax) == 0) {
                        _motor->step(_homing.steps, BACKWARD, SINGLE);
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
                if (digitalRead(_pinStopMax) == 0) {
                    if (tar >= pos) {
                        mode = IDLE;
                        motor_halt();
                        break;
                    }
                }

                if (pos == tar) {
                    mode = IDLE;
                } else if (pos < tar) {
                    _motor->step(1, FORWARD, SINGLE);
                } else {
                    _motor->step(1, BACKWARD, SINGLE);
                }
                break;
            }

             return (mode == IDLE) ? false : true;
        }
};


#endif /* AXIS_Z_H */
/* vim: set shiftwidth=4 expandtab:  */

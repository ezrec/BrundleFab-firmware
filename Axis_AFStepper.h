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

#ifndef AXIS_AFSTEPPER_H
#define AXIS_AFSTEPPER_H

#include <Wire.h>
#include <Adafruit_MotorShield.h>

#include "Axis.h"

extern Adafruit_MotorShield AFMS;

class Axis_AFStepper : public Axis {
    private:
        int _adaMotor; /* 1 = M1/M2, 2 = M3/M4 */
        static const int _stepsPerRotation = 200; /* 1.8 degrees */
        static const float _mmPerRotation = 4.0;  /* 4mm pitch */
        int _pinStopMin;
        int _pinStopMax;

        static const int _maxPos = 11500;
        static const int _minPos = 0;

        float _mm_to_position;
        int32_t _position;

        Adafruit_StepperMotor *_motor;

        enum {
            IDLE,
            HOMING, HOMING_QUIESCE,
            HOMING_BACKOFF,
            MOVING
        } _mode;
        struct {
            unsigned long timeout;
            int32_t steps;
            int dir;
            int pin;
        } _homing;
        struct {
            unsigned long timeout;
        } _moving;

    public:
        Axis_AFStepper(int af_motor, int pinStopMin = -1, int pinStopMax = -1) : Axis()
        {
            _adaMotor = af_motor;
            _motor = AFMS.getStepper(_stepsPerRotation, _adaMotor);
            _mm_to_position = _stepsPerRotation / _mmPerRotation;
            _pinStopMin = pinStopMin;
            _pinStopMax = pinStopMax;

            position_set((_minPos + _maxPos) / 2);
            if (_pinStopMin >= 0) {
                _homing.dir = BACKWARD;
                _homing.pin = _pinStopMin;
            } else if (_pinStopMax >= 0) {
                _homing.dir = FORWARD;
                _homing.pin = _pinStopMax;
            } else {
                _homing.dir = BRAKE;
                _homing.pin = -1;
            }
        }

        virtual void begin()
        {
            _motor->setSpeed(100);       /* 100 RPM */
            if (_pinStopMin >= 0)
                pinMode(_pinStopMin, INPUT_PULLUP);
            if (_pinStopMax >= 0)
                pinMode(_pinStopMax, INPUT_PULLUP);
            Axis::begin();
        }

        virtual void home()
        {
            _homing.steps = 1;
            _mode = HOMING;
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

            switch (_mode) {
            case IDLE:
                if (tar != pos)
                    _mode = MOVING;
                break;
            case HOMING:
                if (_homing.dir == BRAKE) {
                    position_set(0);
                    target_set(0);
                    _mode = IDLE;
                    break;
                }
                if (digitalRead(_homing.pin) == 0) {
                    _homing.timeout = millis()+1;
                    _mode = HOMING_QUIESCE;
                } else {
                    _motor->step(_homing.steps, _homing.dir, SINGLE);
                }
                break;
            case HOMING_QUIESCE:
                if (millis() >= _homing.timeout) {
                    _mode = HOMING_BACKOFF;
                    _homing.timeout = millis()+10;
                    _mode = HOMING_BACKOFF;
                }
                break;
            case HOMING_BACKOFF:
                if (millis() >= _homing.timeout) {
                    if (digitalRead(_homing.pin) == 0) {
                        _motor->step(_homing.steps, (_homing.dir == FORWARD) ? BACKWARD : FORWARD, SINGLE);
                        _homing.timeout = millis()+10;
                    } else {
                        motor_halt();
                        position_set((_homing.dir == FORWARD) ? _maxPos : _minPos);
                        target_set(0);
                        _mode = IDLE;
                    }
                }
                break;
            case MOVING:
                if (tar >= pos && _pinStopMax >= 0) {
                    if (digitalRead(_pinStopMax) == 0) {
                        Serial.println("ENDSTOP: Max\n");
                        _mode = IDLE;
                        target_set(_maxPos);
                        motor_halt();
                        break;
                    }
                }

                if (tar <= pos && _pinStopMin >= 0) {
                    if (digitalRead(_pinStopMin) == 0) {
                        Serial.println("ENDSTOP: Min\n");
                        _mode = IDLE;
                        target_set(_minPos);
                        motor_halt();
                        break;
                    }
                }

                if (pos == tar) {
                    _mode = IDLE;
                } else if (pos < tar) {
                    _motor->step(1, FORWARD, SINGLE);
                    _position++;
                } else {
                    _motor->step(1, BACKWARD, SINGLE);
                    _position--;
                }
                break;
            }

            return (_mode == IDLE) ? false : true;
        }
};


#endif /* AXIS_AFSTEPPER_H */
/* vim: set shiftwidth=4 expandtab:  */

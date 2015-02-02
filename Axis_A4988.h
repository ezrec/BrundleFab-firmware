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

#ifndef AXIS_A4988_H
#define AXIS_A4988_H

#include "Axis.h"

class Axis_A4988 : public Axis {
    private:
        int _stepsPerRotation;
        float _mmPerRotation;
        int _pinStopMin;
        int _pinStopMax;

        int _maxPos;
        static const int _minPos = 0;

        float _mm_to_position;
        int32_t _position;

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
            int32_t target;
        } _homing;
        struct {
            unsigned long timeout;
        } _moving;

        int _enable, _step, _dir;

    public:
        Axis_A4988(int enable, int step, int dir,
                   int pinStopMin, int pinStopMax,
                   unsigned int maxPos,
                   unsigned int stepsPerRotation, float mmPerRotation)
            : Axis()
        {
            _enable = enable;
            _step = step;
            _dir = dir;
            _maxPos = maxPos;
            _stepsPerRotation = stepsPerRotation;
            _mmPerRotation = mmPerRotation;
            _mm_to_position = _stepsPerRotation / _mmPerRotation;
            _pinStopMin = pinStopMin;
            _pinStopMax = pinStopMax;

            _position = 0;
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
            digitalWrite(_enable, LOW);
            digitalWrite(_step, LOW);
            digitalWrite(_dir, LOW);

            pinMode(_enable, OUTPUT);
            pinMode(_step, OUTPUT);
            pinMode(_dir, OUTPUT);
        }

        virtual void home(int32_t position)
        {
            _homing.steps = 1;
            _homing.target = position;
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

        virtual void motor_enable(bool enabled = true)
        {
            digitalWrite(_enable, enabled ? HIGH : LOW);
            Axis::motor_enable(enabled);
        }

        virtual bool motor_active()
        {
            return _mode != IDLE;
        }

        virtual int32_t position_get(void)
        {
            return _position;
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
            case HOMING:
                if (_homing.dir == BRAKE) {
                    _mode = IDLE;
                    Axis::home(_homing.target);
                    break;
                }
                if (digitalRead(_homing.pin) == 1) {
                    _homing.timeout = millis()+1;
                    _mode = HOMING_QUIESCE;
                } else {
                    _move(_homing.steps, _homing.dir);
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
                    if (digitalRead(_homing.pin) == 1) {
                        _move(_homing.steps, (_homing.dir == FORWARD) ? BACKWARD : FORWARD);
                        _homing.timeout = millis()+10;
                    } else {
                        motor_halt();
                        _position = (_homing.dir == FORWARD) ? _maxPos : _minPos;
                        _mode = IDLE;
                        Axis::home(_homing.target);
                    }
                }
                break;
            case MOVING:
                if (tar >= pos && _pinStopMax >= 0) {
                    if (digitalRead(_pinStopMax) == 1) {
                        _mode = IDLE;
                        motor_halt();
                        break;
                    }
                }

                if (tar <= pos && _pinStopMin >= 0) {
                    if (digitalRead(_pinStopMin) == 1) {
                        _mode = IDLE;
                        motor_halt();
                        break;
                    }
                }

                if (pos == tar) {
                    _mode = IDLE;
                } else if (pos < tar) {
                    _move(1, FORWARD);
                    _position++;
                } else {
                    _move(1, BACKWARD);
                    _position--;
                }
                break;
            }

            return (_mode == IDLE) ? false : true;
        }

    private:
        void _move(unsigned int steps, uint8_t dir)
        {
            dir = (dir == FORWARD) ? HIGH : LOW;
            while (steps--) {
                digitalWrite(_step, LOW);
                digitalWrite(_dir, dir);
                digitalWrite(_step, HIGH);
            }
            digitalWrite(_step, LOW);
        }
};


#endif /* AXIS_AF2STEPPER_H */
/* vim: set shiftwidth=4 expandtab:  */

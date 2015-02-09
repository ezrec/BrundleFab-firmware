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

#ifndef AXIS_STEPPER_H
#define AXIS_STEPPER_H

#include "Axis.h"

class Axis_Stepper : public Axis {
    private:
        unsigned int _stepsPerRotation;
        float _mmPerRotation;
        unsigned int _microSteps;

        int32_t _maxPos;
        static const int32_t _minPos = 0;

        float _mm_to_usteps;
        int32_t _position;
        int32_t _target_position;
        struct {
            unsigned int msec;
            unsigned int usec;
        } _delay_per_step;

        enum {
            IDLE,
            HOMING, HOMING_QUIESCE,
            HOMING_BACKOFF,
            MOVING
        } _mode;
        struct {
            unsigned long timeout;
            int32_t steps;
            enum axis_stop_e pin;
            float target;
        } _homing;
        struct {
            unsigned long timeout;
        } _moving;

        struct {
            unsigned long last;
            unsigned long this_step;
            unsigned long per_step;
        } _udelay;

    public:
        Axis_Stepper(int pinStopMin, int pinStopMax,
                     float maxPosMM, unsigned int microSteps,
                     unsigned int stepsPerRotation, float mmPerRotation)
            : Axis(pinStopMin, pinStopMax)
        {
            _microSteps = microSteps;
            _stepsPerRotation = stepsPerRotation;
            _mmPerRotation = mmPerRotation;
            _mm_to_usteps = _stepsPerRotation * _microSteps / _mmPerRotation;
            _maxPos = maxPosMM * _mm_to_usteps;

            _position = 0;
            if (pinStopMin >= 0) {
                _homing.steps = min(-0.5 * _mm_to_usteps, -1);
                _homing.pin = Axis::STOP_MIN_SWITCH;
            } else if (pinStopMax >= 0) {
                _homing.steps = max(0.5 * _mm_to_usteps, 1);
                _homing.pin = Axis::STOP_MAX_SWITCH;
            } else {
                _homing.steps = 0;
                _homing.pin = Axis::STOP_NONE;
            }
        }

        /* Required to be implemented by your base class.
         * Returns the (signed) number of steps that were
         * executed in a *single* action.
         *
         * If 'steps' > 0, then move forward,
         * if 'steps' < 0, then move backward.
         */
        virtual int step(int32_t steps) = 0;

        virtual void home(float mm)
        {
            _homing.target = mm;
            _mode = HOMING;
        }

        virtual float position_min()
        {
            return _minPos / _mm_to_usteps;
        }

        virtual float position_max()
        {
            return _maxPos / _mm_to_usteps;
        }

        virtual bool motor_active()
        {
            return _mode != IDLE;
        }

        virtual float position_get(void)
        {
            return _position / _mm_to_usteps;
        }

        virtual void target_set(float mm, unsigned long ms = 0)
        {
            Axis::target_set(mm, ms);

            _target_position = mm * _mm_to_usteps;
            if (ms) {
                _udelay.per_step = ms * 1000 / abs(_target_position - _position);
            } else {
                _udelay.per_step = 1000;
            }
        }

        virtual bool update()
        {
            int32_t pos = _position;
            int32_t tar = _target_position;

            if (tar >= _maxPos)
                tar = _maxPos - 1;

            if (tar < _minPos)
                tar = _minPos;

            switch (_mode) {
            case IDLE:
                if (tar != pos) {
                    _udelay.last = micros();
                    _udelay.this_step = 0;
                    _mode = MOVING;
                }
                break;
            case HOMING:
                if (_homing.steps == 0) {
                    _mode = IDLE;
                    Axis::home(_homing.target);
                    break;
                }
                if (endstop(_homing.pin)) {
                    _homing.timeout = millis()+1;
                    _mode = HOMING_QUIESCE;
                } else {
                    step(_homing.steps);
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
                    if (endstop(_homing.pin)) {
                        /* If we are still on the endstop, back slowly */
                        step(_homing.steps > 0 ? -1 : 1);
                        _homing.timeout = millis()+10;
                    } else {
                        _position = (_homing.steps > 0) ? _maxPos : _minPos;
                        _mode = IDLE;
                        Axis::home(_homing.target);
                    }
                }
                break;
            case MOVING:
                if (tar >= pos && endstop(Axis::STOP_MAX)) {
                    _mode = IDLE;
                    break;
                }

                if (tar <= pos && endstop(Axis::STOP_MIN)) {
                    _mode = IDLE;
                    break;
                }

                if (pos == tar)
                    _mode = IDLE;
                else {
                    unsigned long usec_now = micros();
                    if ((usec_now - _udelay.last) >= _udelay.this_step) {
                        int steps = step(tar - pos);
                        _position += steps;
                        _udelay.this_step = steps  * _udelay.per_step;
                    }
                }
                break;
            }

            return (_mode == IDLE) ? false : true;
        }
};


#endif /* AXIS_STEPPER_H */
/* vim: set shiftwidth=4 expandtab:  */

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

        float _usteps_per_mm;
        int32_t _position;
        int32_t _target_position;

        enum {
            IDLE,
            HOMING, HOMING_QUIESCE,
            HOMING_BACKOFF,
            MOVING
        } _mode;
        struct {
            unsigned long timeout;
            int steps;
            enum axis_stop_e pin;
            int32_t position;
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
        Axis_Stepper(int pinStopMin, int pinStopMax, unsigned int mm_per_min_max,
                     float maxPosMM, unsigned int microSteps,
                     unsigned int stepsPerRotation, float mmPerRotation)
            : Axis(pinStopMin, pinStopMax)
        {
            _microSteps = microSteps;
            _stepsPerRotation = stepsPerRotation;
            _mmPerRotation = mmPerRotation;
            _usteps_per_mm = _stepsPerRotation * _microSteps / _mmPerRotation;
            _maxPos = maxPosMM * _usteps_per_mm;

            _position = 0;
            if (pinStopMin >= 0) {
                _homing.steps = microSteps;
                _homing.pin = Axis::STOP_MIN_SWITCH;
                _homing.position = _minPos;
            } else if (pinStopMax >= 0) {
                _homing.steps = microSteps;
                _homing.pin = Axis::STOP_MAX_SWITCH;
                _homing.position = _maxPos;
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
            Axis::home(mm);

            _mode = HOMING;
        }

        virtual float position_min()
        {
            return _minPos / _usteps_per_mm;
        }

        virtual float position_max()
        {
            return _maxPos / _usteps_per_mm;
        }

        virtual bool motor_active()
        {
            return _mode != IDLE;
        }

        virtual float position_get(void)
        {
            return _position / _usteps_per_mm;
        }

        virtual void target_set(float mm, unsigned long ms = 0)
        {
            Axis::target_set(mm, ms);

            _target_position = mm * _usteps_per_mm;
            /* usec/ustep = usec/minute * minute/mm * mm/ustep */
            _udelay.per_step = 60000000UL / _target.velocity /  _usteps_per_mm;
        }

        virtual bool update()
        {
            int32_t pos = _position;
            int32_t tar = _target_position;

            switch (_mode) {
            case IDLE:
                if (tar >= _maxPos)
                    tar = _maxPos - 1;

                if (tar < _minPos)
                    tar = _minPos;

                if (tar != pos) {
                    _udelay.last = micros();
                    _udelay.this_step = 0;
                    _mode = MOVING;
                }
                break;
            case HOMING:
                if (_homing.steps == 0) {
                    _mode = IDLE;
                    break;
                }
                if (endstop(_homing.pin)) {
                    _homing.timeout = millis()+1;
                    _mode = HOMING_QUIESCE;
                } else {
                    _step(_homing.steps);
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
                        if (!_step(_homing.steps > 0 ? -1 : 1))
                            break;
                        _homing.timeout = millis()+10;
                    } else {
                        _position = _homing.position;
                        _mode = IDLE;
                    }
                }
                break;
            case MOVING:
                if (tar > pos && endstop(Axis::STOP_MAX)) {
                    _position = _maxPos;
                    _mode = IDLE;
                    break;
                }

                if (tar < pos && endstop(Axis::STOP_MIN)) {
                    _position = _minPos;
                    _mode = IDLE;
                    break;
                }

                if (pos == tar)
                    _mode = IDLE;
                else
                    _step(tar-pos);

                break;
            }

            return (_mode == IDLE) ? false : true;
        }

    private:
        bool _step(int32_t steps)
        {
            unsigned long usec_now = micros();
            if ((usec_now - _udelay.last) >= _udelay.this_step) {
                int stepped;
                stepped = step(steps);
                _position += stepped;
                _udelay.this_step = (unsigned long)abs(stepped) * _udelay.per_step;
                _udelay.last = micros();
                return true;
            }
            return false;
        }
};

#endif /* AXIS_STEPPER_H */
/* vim: set shiftwidth=4 expandtab:  */

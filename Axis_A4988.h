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

#include "Axis_Stepper.h"

class Axis_A4988 : public Axis_Stepper {
    private:
        int _enable, _step, _dir;

    public:
        Axis_A4988(int enable, int step, int dir,
                   int pinStopMin, int pinStopMax,
                   float maxPosMM, int microsteps,
                   unsigned int stepsPerRotation, float mmPerRotation)
            : Axis_Stepper(pinStopMin, pinStopMax,
                           maxPosMM, microsteps,
                           stepsPerRotation, mmPerRotation)
        {
            _enable = enable;
            _step = step;
            _dir = dir;
        }

        virtual void begin()
        {
            digitalWrite(_enable, HIGH);
            digitalWrite(_step, LOW);
            digitalWrite(_dir, LOW);

            pinMode(_enable, OUTPUT);
            pinMode(_step, OUTPUT);
            pinMode(_dir, OUTPUT);

            Axis_Stepper::begin();
        }

        virtual void motor_enable(bool enabled = true)
        {
            digitalWrite(_enable, enabled ? LOW : HIGH);
            Axis_Stepper::motor_enable(enabled);
        }

        virtual int step(int32_t steps)
        {
            int dir;

            if (!steps)
                return 0;

            dir = (steps > 0) ? HIGH : LOW;
            digitalWrite(_dir, dir);
            digitalWrite(_step, HIGH);
            digitalWrite(_step, LOW);

            return dir ? 1 : -1;
        }
};


#endif /* AXIS_AF2STEPPER_H */
/* vim: set shiftwidth=4 expandtab:  */

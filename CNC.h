/*
 * Copyright (C) 2015, Jason S. McMullan
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

#ifndef CNC_H
#define CNC_H

#include "Axis.h"
#include "ToolHead.h"

class CNC {
    private:
        Axis *_axis[AXIS_MAX];
        ToolHead *_toolhead;

    public:
        CNC(Axis *x, Axis *y, Axis *z, Axis *e, ToolHead *t)
        {
            _axis[AXIS_X] = x;
            _axis[AXIS_Y] = y;
            _axis[AXIS_Z] = z;
            _axis[AXIS_E] = e;
            _toolhead = t;
        }

        Axis *axis(int n)
        {
            return _axis[n];
        }

        ToolHead *toolhead()
        {
            return _toolhead;
        }

        void motor_enable(bool enabled = true)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_enable(enabled);
        }

        void motor_halt()
        {
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_halt();
        }

        void motor_disable()
        {
            motor_enable(false);
        }

        void target_get_mm(float *pos)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                pos[i] = _axis[i]->target_get_mm();
        }

        void sleep()
        {
            motor_halt();
        }

        void stop()
        {
            motor_disable();
            _toolhead->stop();
        }

        bool update()
        {
            bool motion;

            for (int i = 0; i < AXIS_MAX; i++)
                motion |= _axis[i]->update();

            if (motion)
                _toolhead->update();

            return motion;
        }
};

#endif /* CNC_H */
/* vim: set shiftwidth=4 expandtab:  */

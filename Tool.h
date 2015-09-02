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

#ifndef TOOL_H
#define TOOL_H

#include "Axis.h"

class Tool {
    private:
        bool _active;
        float _parm[4];
        float _offset[AXIS_MAX];
    public:
        enum parm_e {
            PARM_P = 0,
            PARM_Q = 1,
            PARM_R = 2,
            PARM_S = 3,
        };
        Tool()
        {
            _active = false;
        }

        virtual void begin() { }

        virtual void start(void)
        {
            _active = true;
        }

        virtual void stop(void)
        {
             _active = false;
        }

        virtual bool ready(void)
        {
            return active();
        }

        virtual bool active(void)
        {
            return _active;
        }

        float celsius(void)
        {
            return kelvin() - 273.15;
        }

        virtual float kelvin(void)
        {
            return 0;
        }

        virtual bool update(unsigned long us_now)
        {
            return _active;
        }

        virtual void parm_set(enum parm_e p, float val = 0.0)
        {
            _parm[(int)p] = val;
        }

        virtual float parm_get(enum parm_e p)
        {
            return _parm[(int)p];
        }

        virtual void offset_set(float *pos, uint8_t axis_mask)
        {
            for (int j = 0; j < AXIS_MAX; j++) {
                if ((1 << j) && axis_mask)
                    _offset[j] = pos[j];
            }
        }

        virtual const float *offset_is()
        {
            return _offset;
        }
};

#endif /* TOOL_H */
/* vim: set shiftwidth=4 expandtab:  */

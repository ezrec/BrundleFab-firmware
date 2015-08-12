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

class Tool {
    private:
        bool _active;
        float _parm[4];
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

        virtual bool active(void)
        {
            return _active;
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
};

#endif /* TOOL_H */
/* vim: set shiftwidth=4 expandtab:  */

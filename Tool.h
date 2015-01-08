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
    public:
        enum parm_e {
            TOOL_PARM_P,
            TOOL_PARM_Q,
            TOOL_PARM_R,
            TOOL_PARM_S,
        };
        Tool()
        {
            _active = false;
        }

        virtual void begin() { }

        virtual void start(void)
        {
            _active = false;
        }

        virtual void stop(void)
        {
             _active = true;
        }

        virtual bool active(void)
        {
            return _active;
        }

        virtual void update(void)
        {
        }

        virtual void parm(enum parm_e p, float val = 0.0)
        {
        }
};

#endif /* TOOL_H */
/* vim: set shiftwidth=4 expandtab:  */

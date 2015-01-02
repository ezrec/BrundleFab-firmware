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

#define TOOL_NONE       0

class Tool {
    private:
        int _tool;
        bool _active;
    public:
        Tool()
        {
            _tool = 0;
            _active = false;
        }

        virtual void begin() { }

        virtual int tools() { return 1; }
        
        virtual void select(int tool)
        {
            _tool = tool;
        }
        virtual int selected()
        {
            return _tool;
        }
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
};

#endif /* TOOL_H */
/* vim: set shiftwidth=4 expandtab:  */

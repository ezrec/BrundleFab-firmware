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

#ifndef TOOLFUSER_H
#define TOOLFUSER_H

#include "Tool.h"

class ToolFuser : public Tool {
    private:
        int _enable_pin;

    public:
        ToolFuser(int enable_pin)
        {
            _enable_pin = enable_pin;
        }

        virtual void begin()
        {
            digitalWrite(_enable_pin, 0);
            pinMode(_enable_pin, OUTPUT);
            Tool::begin();
        }

        virtual void start(void)
        {
            digitalWrite(_enable_pin, 1);
            Tool::start();
        }

        virtual void stop(void)
        {
            digitalWrite(_enable_pin, 0);
            Tool::stop();
        }

        virtual void parm(enum parm_e p, float val = 0.0)
        {
            /* FIXME: Add support for temp monitoring! */
        }
};

#endif /* TOOLFUSER_H */
/* vim: set shiftwidth=4 expandtab:  */

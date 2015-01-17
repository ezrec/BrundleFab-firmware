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

#ifndef TOOLINK_H
#define TOOLINK_H

#include <stdint.h>

#include "Tool.h"

#include "pinout.h"

class ToolInk : public Tool {
    private:
        uint16_t _pattern;
        uint32_t _pulse_per_minute;
    public:
        virtual void parm(enum parm_e p, float val = 0.0)
        {
            switch (p) {
            case PARM_P: _pattern = (uint16_t)val; break;
            case PARM_S: _pulse_per_minute = (uint32_t)val; break;
            default: break;
            }
        }

        virtual void update(void);

        virtual bool active(void)
        {
            return (_pattern && (_pulse_per_minute > 0)) ? true : false;
        }
};

#endif /* TOOLINK_H */
/* vim: set shiftwidth=4 expandtab:  */

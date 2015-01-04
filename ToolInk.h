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

#include <InkShieldMega.h>

#include "Tool.h"

#include "pinout.h"

#define INK_PATTERN_SIZE        (8 * 96)

extern INKSHIELD_CLASS InkShield_Black;

class ToolInk : public Tool {
    private:
        uint16_t _pattern;
    public:
        virtual void parm(float p, float q = 0.0, float r = 0.0, float s = 0.0)
        {
            _pattern = (uint16_t)(int)p;
        }

        virtual void update(void)
        {
            if (!active())
                return;

            InkShield_Black.spray_ink(_pattern);
        }
};

#endif /* TOOLINK_H */
/* vim: set shiftwidth=4 expandtab:  */

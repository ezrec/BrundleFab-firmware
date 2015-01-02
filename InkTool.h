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

#ifndef INKTOOL_H
#define INKTOOL_H

#include <InkShieldMega.h>

#include "Tool.h"

#include "pinout.h"

extern INKSHIELD_CLASS InkShield;

class InkTool : public Tool {
    public:
        virtual int tools()
        {
            return (1 << 12);
        }

        virtual void update()
        {
            uint16_t mask;

            if (!active())
                return;
            
            mask = selected() - 1;

            InkShield.spray_ink(mask);
        }
};

#endif /* TOOL_H */
/* vim: set shiftwidth=4 expandtab:  */

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

#include <InkShieldMega.h>

#include "ToolInk.h"

#include "pinout.h"

INKSHIELD_CLASS InkShield_Black(INKSHIELD_PULSE);


void ToolInk::update(void)
{
    if (!active())
        return;

    InkShield_Black.spray_ink(_pattern);
};

/* vim: set shiftwidth=4 expandtab:  */

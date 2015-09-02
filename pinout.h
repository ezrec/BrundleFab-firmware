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

#ifndef PINOUT_H
#define PINOUT_H

#include "config.h"

#if defined(SHIELD_ramps)
#include "pinout-ramps.h"
#elif defined(SHIELD_brundle)
#include "pinout-brundle.h"
#else
#error Unknown SHIELD_xxxx pinout!
#endif

#define TOOL_INK_BLACK          1
#define TOOL_FUSER              20
#define TOOL_RECOAT             21
#define TOOL_BED_TEMP           22

#endif /* PINOUT_H */
/* vim: set shiftwidth=4 expandtab:  */

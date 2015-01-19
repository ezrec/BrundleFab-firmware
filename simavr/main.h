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

#ifndef SIMAVR_MAIN_H
#define SIMAVR_MAIN_H

#include <SDL.h>

extern "C" {

void simavr_update_gui(SDL_Surface *surf);

};

extern unsigned long _micros;

#endif /* SIMAVR_MAIN_H */
/* vim: set shiftwidth=4 expandtab:  */

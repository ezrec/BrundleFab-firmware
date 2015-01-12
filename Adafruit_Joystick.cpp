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

#include <Arduino.h>

#include "Adafruit_Joystick.h"

static int const _joy_calibration[5] = {
    40,         /* down */
    204,        /* right */
    307,        /* select */
    409,        /* up */
    655,        /* left */
};

int Adafruit_Joystick::read()
{
    int val = analogRead(_pin);

    for (int i = 0; i < 5; i++)
        if (val < _joy_calibration[i])
            return i+1;

    return 0;
}

/* vim: set shiftwidth=4 expandtab:  */

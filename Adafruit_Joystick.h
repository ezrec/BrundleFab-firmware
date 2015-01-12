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

#ifndef ADAFRUIT_JOYSTICK_H
#define ADAFRUIT_JOYSTICK_H

#define AFJOYSTICK_NONE         0
#define AFJOYSTICK_DOWN         1
#define AFJOYSTICK_RIGHT        2
#define AFJOYSTICK_SELECT       3
#define AFJOYSTICK_UP           4
#define AFJOYSTICK_LEFT         5

/* Supports the Adafruit_ST7735 analog pin based digital joystick
 * https://learn.adafruit.com/1-8-tft-display/reading-the-joystick
 */
class Adafruit_Joystick {
    private:
        int _pin;
    public:
        Adafruit_Joystick(int analog_pin = 3)
        {
            _pin = analog_pin;
        }

        int read();
};


#endif /* ADAFRUIT_JOYSTICK_H */
/* vim: set shiftwidth=4 expandtab:  */

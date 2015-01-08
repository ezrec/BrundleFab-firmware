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

#include <sys/time.h>

#include "Arduino.h"

static int digitalPin[54];
static int analogPin[6];

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin >= 54)
        return;
}

void digitalWrite(uint8_t pin, uint8_t value)
{
    if (pin >= 54)
        return;
    digitalPin[pin] = value;
}

int digitalRead(uint8_t pin)
{
    if (pin >= 54)
        return -1;
    return digitalPin[pin];
}

int analogRead(uint8_t pin)
{
    if (pin >= 6)
        return -1;

    return analogPin[pin];
}

void analogWrite(uint8_t pin, int val)
{
    if (pin >= 6)
        return;

    analogPin[pin] = val;
}

unsigned long millis(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

char * ltoa (long val, char *s, int radix)
{
    if (radix == 8)
        sprintf(s, "%lo", val);
    if (radix == 10)
        sprintf(s, "%ld", val);
    else
        sprintf(s, "%lx", val);

    return s;
}

char * utoa (unsigned int val, char *s, int radix)
{
    if (radix == 8)
        sprintf(s, "%o", val);
    if (radix == 10)
        sprintf(s, "%u", val);
    else
        sprintf(s, "%x", val);

    return s;
}

char * ultoa (unsigned long val, char *s, int radix)
{
    if (radix == 8)
        sprintf(s, "%lo", val);
    if (radix == 10)
        sprintf(s, "%lu", val);
    else
        sprintf(s, "%lx", val);

    return s;
}

char * itoa (int val, char *s, int radix)
{
    if (radix == 8)
        sprintf(s, "%o", val);
    if (radix == 10)
        sprintf(s, "%d", val);
    else
        sprintf(s, "%x", val);

    return s;
}


/* vim: set shiftwidth=4 expandtab:  */

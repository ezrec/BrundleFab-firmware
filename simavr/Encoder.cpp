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

#include <math.h>

#include "Arduino.h"

#include "Encoder.h"

#define SPEED_SCALE     1.7f

static struct encoder {
    unsigned long time;
    uint8_t dir;
    float speed;
    int32_t position;
    uint32_t range;
    int32_t offset;

    int pin_min;
    int pin_max;
} _encoder[54];

void encoder_pinstop(int n, uint32_t range, int pin_min, int pin_max)
{
    _encoder[n].range = (range + 20);
    _encoder[n].pin_min = pin_min;
    _encoder[n].pin_max = pin_max;
}

void encoder_reset(int n)
{
    _encoder[n].dir = BRAKE;
    _encoder[n].speed = 0;
    _encoder[n].position = 0;
}

void encoder_set(int n, int32_t pos)
{
    _encoder[n].offset = (pos - encoder_get(n));
}

static float encoder_position(int n, unsigned long now)
{
    float pos;

    if ((_encoder[n].dir == FORWARD ||
        _encoder[n].dir == BACKWARD) &&
            now != _encoder[n].time) {
        float delta;
 
        delta = (now - _encoder[n].time) * _encoder[n].speed * SPEED_SCALE;

        if (_encoder[n].dir == BACKWARD)
            delta *= -1.0;
            
        pos = _encoder[n].position + delta;

        if (pos > _encoder[n].range) {
            pos = _encoder[n].range;
            digitalWrite(_encoder[n].pin_max, 1);
        } else {
            digitalWrite(_encoder[n].pin_max, 0);
        }

        if (pos < 0) {
            pos = 0;
            digitalWrite(_encoder[n].pin_min, 1);
        } else {
            digitalWrite(_encoder[n].pin_min, 0);
        }
    } else {
        pos =  _encoder[n].position;
    }

    return pos;
}

int32_t encoder_get(int n)
{
    unsigned long now = micros();
   
    return encoder_position(n, now) + _encoder[n].offset;
}

void encoder_speed(int n, float speed)
{
    int dir = _encoder[n].dir;
    unsigned long now = micros();

    _encoder[n].position = encoder_position(n, now);
    _encoder[n].speed = cos(M_PI + speed * M_PI) + 1.0;
    _encoder[n].time = now;
}

void encoder_dir(int n, uint8_t dir)
{
    unsigned long now = micros();
  
    _encoder[n].position = encoder_position(n, now);
    _encoder[n].dir = dir;
    _encoder[n].time = now;
}

/* vim: set shiftwidth=4 expandtab:  */

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

#include "Arduino.h"

#include "Encoder.h"

#define ENCODER_SCALE   64

static struct encoder {
    unsigned long time;
    uint8_t dir;
    uint8_t pwm;
    int32_t position;
    uint32_t range;
    int32_t offset;

    int pin_min;
    int pin_max;
} _encoder[54];

void encoder_pinstop(int n, uint32_t range, int pin_min, int pin_max)
{
    _encoder[n].range = (range + 20) * ENCODER_SCALE;
    _encoder[n].pin_min = pin_min;
    _encoder[n].pin_max = pin_max;
}

void encoder_reset(int n)
{
    _encoder[n].dir = BRAKE;
    _encoder[n].pwm = 0;
    _encoder[n].position = 0;
}

void encoder_set(int n, int32_t pos)
{
    _encoder[n].offset = (pos - encoder_get(n));
}

int32_t encoder_get(int n)
{
    encoder_dir(n, _encoder[n].dir);
    return _encoder[n].position / ENCODER_SCALE + _encoder[n].offset;
}

void encoder_speed(int n, uint8_t pwm)
{
    int dir = _encoder[n].dir;

    encoder_dir(n, BRAKE);
    _encoder[n].pwm = pwm;
    encoder_dir(n, dir);
}

void encoder_dir(int n, uint8_t dir)
{
    int32_t delta;
    unsigned long now = micros();
   
    if ((_encoder[n].dir == FORWARD ||
        _encoder[n].dir == BACKWARD) &&
            now != _encoder[n].time) {
        int speed = (_encoder[n].pwm > 97)  ? (_encoder[n].pwm - 97) : 0;
        delta = speed;

        if (_encoder[n].dir == FORWARD)
            _encoder[n].position += delta;
        else if (_encoder[n].dir == BACKWARD)
            _encoder[n].position -= delta;

        if (_encoder[n].position > _encoder[n].range) {
            digitalWrite(_encoder[n].pin_max, 1);
            printf("ENDSTOP: %d Max\r\n", n);
        } else {
            digitalWrite(_encoder[n].pin_max, 0);
        }

        if (_encoder[n].position < 0) {
            _encoder[n].position = 0;
            printf("ENDSTOP: %d Min\r\n", n);
            digitalWrite(_encoder[n].pin_min, 1);
        } else {
            digitalWrite(_encoder[n].pin_min, 0);
        }
    }

    _encoder[n].dir = dir;
    _encoder[n].time = now;
}

/* vim: set shiftwidth=4 expandtab:  */

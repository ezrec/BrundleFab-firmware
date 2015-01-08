/*
 * Copyright (C) 2014, Jason S. McMullan
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

#ifndef AXIS_Y_H
#define AXIS_Y_H

#include "pinout.h"
#include "Axis_DCEncoder.h"

class Axis_Y : public Axis_DCEncoder {
    private:
        static const int _pwmMinimum = 98;
        static const int _pwmMaximum = 200;

        static const int _minPos = 0;
        static const int _maxPos = 5250;

        static const float _mmWidth = 230.0;

    public:
        Axis_Y() : Axis_DCEncoder(
                Y_MOTOR, _pwmMinimum, _pwmMaximum,
                YENC_A, YENC_B,
                YSTP_MIN, YSTP_MAX,
                _mmWidth, _minPos, _maxPos)
        {
        }
};


#endif /* AXIS_Y_H */
/* vim: set shiftwidth=4 expandtab:  */

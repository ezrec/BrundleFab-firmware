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

#ifndef AXIS_E_H
#define AXIS_E_H

#include "Axis_AFStepper.h"

class Axis_E : public Axis_AFStepper {
    private:
        static const int _adaMotor = 2; /* 1 = M1/M2, 2 = M3/M4 */
        static const int _pinStopMin = ESTP_MIN;

    public:
        Axis_E() : Axis_AFStepper(2, ESTP_MIN, -1)
        {
        }
};

#endif /* AXIS_E_H */

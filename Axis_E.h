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
    public:
        Axis_E() : Axis_AFStepper(E_MOTOR, ESTP_MIN, ESTP_MAX)
        {
        }
};

#endif /* AXIS_E_H */

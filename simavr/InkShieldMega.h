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

#ifndef SIMAVR_INKSHIELDMEGA_H
#define SIMAVR_INKSHIELDMEGA_H

class InkShieldA0A3 {
    private:
        int _pulse_pin;

    public:
        InkShieldA0A3(int pin)
        {
            _pulse_pin = pin;
        }

        void spray_ink(int pattern)
        {
        }
};

#endif /* SIMAVR_INKSHIELDMEGA_H */
/* vim: set shiftwidth=4 expandtab:  */

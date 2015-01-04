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

#ifndef TOOLINK_H
#define TOOLINK_H

#include <InkShieldMega.h>

#include "Tool.h"

#include "pinout.h"

#define INK_PATTERN_SIZE        (8 * 96)

extern INKSHIELD_CLASS InkShield_Black;

class ToolInk : public Tool {
    private:
        uint16_t _pattern[INK_PATTERN_SIZE];
        int _len;
        float _origin[AXIS_MAX];
        float _inc[AXIS_MAX];
    public:
        virtual void pattern(const float axis[AXIS_MAX],
                             const float dim[AXIS_MAX],
                             const void *buff, size_t len)
        {
            int i;
            const uint8_t *byte = (const uint8_t *)buff;

            len >>= 1;
            if (len > INK_PATTERN_SIZE)
                len = INK_PATTERN_SIZE;
            
            for (i = 0; i < (int)len; i++, byte+=2)
                _pattern[i] = ((uint16_t)byte[0] << 8) | byte[1];

            for (; i < INK_PATTERN_SIZE; i++)
                _pattern[i] = 0;

            for (i = 0; i < AXIS_MAX; i++) {
                _origin[i] = axis[i];
                _inc[i] = dim[i] / _len;
            }

            _len = len;
        }

        virtual void update(const float axis[AXIS_MAX])
        {
            uint16_t mask;
            int index;

            if (!active())
                return;

            index = (int)((axis[AXIS_Y] - _origin[AXIS_Y]) * _inc[AXIS_Y]) % _len;
            mask = _pattern[index];
            
            InkShield_Black.spray_ink(mask);
        }
};

#endif /* TOOLINK_H */
/* vim: set shiftwidth=4 expandtab:  */

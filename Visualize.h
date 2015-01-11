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

#ifndef VISUALIZE_H
#define VISUALIZE_H

#include <Adafruit_GFX.h>

#include "Axis.h"

#define VC_INVISIBLE    -1
#define VC_AXIS         0
#define VC_MOVE         (AXIS_MAX + 0)
#define VC_TOOL         (AXIS_MAX + 1)
#define VC_FEED         (AXIS_MAX + 2)
#define VC_BACKGROUND   (AXIS_MAX + 3)
#define VC_BORDER       (AXIS_MAX + 4)

#define VC_MAX          (AXIS_MAX + 5)

struct point {
    int x, y;
};

class Visualize {
    private:
        Adafruit_GFX *_gfx;

        /* Location of visualization */
        int _top,_left,_width,_height;

        uint16_t _color[VC_MAX];

        struct {
            struct point point;
            float position[AXIS_MAX];
        } _cursor;

    public:
        Visualize(Adafruit_GFX *gfx, int width, int height,
                  int x = 0, int y = 0)
        {
            _gfx = gfx;
            _top = y + 1;
            _left = x + 1;
            _width = width - 2;
            _height = height - 2;

            for (int i = 0; i < VC_MAX; i++)
                _color[i] = 0xffff;
            _color[VC_BACKGROUND] = 0;
        }

        void begin()
        {
            _gfx->drawRect(_left-1, _top-1, _width+2, _height+2, _color[VC_BORDER]);
            clear();
        }

        void clear()
        {
            float zero[AXIS_MAX] = {};
            _gfx->fillRect(_left, _top, _width, _height, _color[VC_BACKGROUND]);

            for (int i = 0; i < AXIS_MAX; i++) {
                float pos[AXIS_MAX] = {};
                pos[i] = 1000.0f;
                cursor_to(zero);
                line_to(VC_AXIS + i, pos);
            }
        }

        void color_set(int color_ndx, uint16_t color)
        {
            _color[color_ndx] = color;
        }

        void render(struct gcode_block *blk);

        void cursor_to(const float *pos)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                _cursor.position[i] = pos[i];

            _flatten(_cursor.position, &_cursor.point);
        }

        void line_to(int color_ndx, const float *pos);
        void pixel_at(int color_ndx, const float *pos);

    private:
        void _flatten(const float *pos, struct point *pt);
        void _pixel2d_clipped(uint16_t color, const struct point *pt);
        void _line2d_clipped(const float *a_color, const struct point *a,
                             const float *b_color, const struct point *b);

};

#endif /* VISUALIZE_H */
/* vim: set shiftwidth=4 expandtab:  */

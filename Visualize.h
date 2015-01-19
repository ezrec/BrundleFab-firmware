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
#include "WindowGFX.h"

#define VC_INVISIBLE    -1
#define VC_AXIS         0
#define VC_MOVE         (AXIS_MAX + 0)
#define VC_TOOL         (AXIS_MAX + 1)
#define VC_FEED         (AXIS_MAX + 2)
#define VC_BACKGROUND   (AXIS_MAX + 3)

#define VC_MAX          (AXIS_MAX + 4)

struct point {
    int x, y;
};

class Visualize : public WindowGFX {
    private:
        float _scale, _max[AXIS_MAX];

        uint16_t _color[VC_MAX];

        struct {
            struct point point;
            float position[AXIS_MAX];
        } _cursor;

    public:
        Visualize(Adafruit_GFX *gfx, int width, int height,
                  int x = 0, int y = 0)
            : WindowGFX(gfx, width, height, x, y)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                _max[i] = 1000.0;

            for (int i = 0; i < VC_MAX; i++)
                _color[i] = 0xffff;
            _color[VC_BACKGROUND] = 0;
        }

        void clear()
        {
            float zero[AXIS_MAX] = {};
            fillScreen(_color[VC_BACKGROUND]);

            for (int i = 0; i < AXIS_MAX; i++) {
                float pos[AXIS_MAX] = {};
                pos[i] = _max[i];
                cursor_to(zero);
                line_to(VC_AXIS + i, pos);
            }

            cursor_to(0, 0, _max[AXIS_Z]);
            line_to(VC_AXIS + AXIS_Y, 0, _max[AXIS_Y], _max[AXIS_Z]);
            line_to(VC_AXIS + AXIS_Z, 0, _max[AXIS_Y], 0);
            line_to(VC_AXIS + AXIS_X, _max[AXIS_X], _max[AXIS_Y], 0);
            line_to(VC_AXIS + AXIS_Z, _max[AXIS_X], _max[AXIS_Y], _max[AXIS_Z]);
            line_to(VC_AXIS + AXIS_Y, _max[AXIS_X], 0, _max[AXIS_Z]);
            line_to(VC_AXIS + AXIS_Z, _max[AXIS_X], 0, 0);
            line_to(VC_AXIS + AXIS_Y, _max[AXIS_X], _max[AXIS_Y], 0);
            cursor_to(0, 0, _max[AXIS_Z]);
            line_to(VC_AXIS + AXIS_X, _max[AXIS_X], 0, _max[AXIS_Z]);
        }

        void clear(float scale)
        {
            _scale = scale;
            clear();
        }

        void clear(float x_mm, float y_mm, float z_mm)
        {
            /* Bounding box for the build volume */
            float scale_y = (float)height()/(z_mm + y_mm/4.0);
            float scale_x = (float)width()/(x_mm + y_mm/4.0);

            _max[AXIS_X] = x_mm;
            _max[AXIS_Y] = y_mm;
            _max[AXIS_Z] = z_mm;

            clear(scale_x > scale_y ? scale_y : scale_x);
        }

        void color_set(int color_ndx, uint16_t color)
        {
            _color[color_ndx] = color;
        }

        void cursor_to(const float *pos)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                _cursor.position[i] = pos[i];

            _flatten(_cursor.position, &_cursor.point);
        }

        void line_to(int color_ndx, const float *pos);

        void pixel_at(int color_ndx, const float *pos);

        void cursor_to(float x, float y, float z)
        {
            float pos[AXIS_MAX] = { x, y, z };
            cursor_to(pos);
        }

        void line_to(int color_ndx, float x, float y, float z)
        {
            float pos[AXIS_MAX] = { x, y, z };
            line_to(color_ndx, pos);
        }

        void pixel_at(int color_ndx, float x, float y, float z)
        {
            float pos[AXIS_MAX] = { x, y, z };
            pixel_at(color_ndx, pos);
        }

    private:
        void _flatten(const float *pos, struct point *pt)
        {
            pt->x = (pos[AXIS_X] + pos[AXIS_Y]/4.0) * _scale + 1;
            pt->y = (height() - 1) - (pos[AXIS_Z] + pos[AXIS_Y]/4) * _scale - 1;
        }

        void _pixel2d_clipped(uint16_t color, const struct point *pt);
        void _line2d_clipped(const float *a_color, const struct point *a,
                             const float *b_color, const struct point *b);

};

#endif /* VISUALIZE_H */
/* vim: set shiftwidth=4 expandtab:  */

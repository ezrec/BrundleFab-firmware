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

#include "Visualize.h"

void Visualize::_pixel2d_clipped(uint16_t color, const struct point *pt)
{
    int x,y;

    x = pt->x;
    y = pt->y;

    if (x < 0 || x >= _width)
        return;

    if (y < 0 || y >= _height)
        return;

    _gfx->drawPixel(x + _left, y + _top, color);
}

// From WikiPedia:
// http://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000
 
// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (0, 0), and (_width, _height)
static int ComputeOutCode(int x, int y, int w, int h)
{
    int code;
 
    code = INSIDE;          // initialised as being inside of clip window
 
    if (x < 0)           // to the left of clip window
        code |= LEFT;
    else if (x > w)      // to the right of clip window
        code |= RIGHT;
    if (y < 0)           // below the clip window
        code |= BOTTOM;
    else if (y > h)      // above the clip window
        code |= TOP;
 
    return code;
}

static uint16_t r5g6b5(const float *color_a, const float *cinc, int pixel)
{
    return (((int)((color_a[0] + cinc[0] * pixel) * 31) & 0x1f) << 11) |
           (((int)((color_a[1] + cinc[1] * pixel) * 63) & 0x3f) << 5) |
           (((int)((color_a[2] + cinc[2] * pixel) * 31) & 0x1f) << 0);
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
// diagonal from (0, 0) to (_width, _height).
void Visualize::_line2d_clipped(const float *color_a, const struct point *a,
                                const float *color_b, const struct point *b)
{
    int x0, y0, x1, y1;

    x0 = a->x;
    y0 = a->y;
    x1 = b->x;
    y1 = b->y;

    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    int outcode0 = ComputeOutCode(x0, y0, _width, _height);
    int outcode1 = ComputeOutCode(x1, y1, _width, _height);
    bool accept = false;
 
    while (true) {
        if (!(outcode0 | outcode1)) { // Bitwise OR is 0. Trivially accept and get out of loop
            accept = true;
            break;
        } else if (outcode0 & outcode1) { // Bitwise AND is not 0. Trivially reject and get out of loop
            break;
                } else {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            int x = 0, y = 0;
 
            // At least one endpoint is outside the clip rectangle; pick it.
            int outcodeOut = outcode0 ? outcode0 : outcode1;
 
            // Now find the intersection point;
            // use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
            if (outcodeOut & TOP) {           // point is above the clip rectangle
                x = x0 + (x1 - x0) * (_height - y0) / (y1 - y0);
                y = _height;
            } else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
                x = x0 + (x1 - x0) * ( - y0) / (y1 - y0);
                y = 0;
            } else if (outcodeOut & RIGHT) {  // point is to the right of clip rectangle
                y = y0 + (y1 - y0) * (_width - x0) / (x1 - x0);
                x = _width;
            } else if (outcodeOut & LEFT) {   // point is to the left of clip rectangle
                y = y0 + (y1 - y0) * ( - x0) / (x1 - x0);
                x = 0;
            }
 
            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = ComputeOutCode(x0, y0, _width, _height);
            } else {
                x1 = x;
                y1 = y;
                outcode1 = ComputeOutCode(x1, y1, _width, _height);
            }
        }
    }

    if (!accept)
        return;

// From http://www.roguebasin.com/index.php?title=Bresenham%27s_Line_Algorithm
    int delta_x(x1 - x0);
    // if x0 == x1, then it does not matter what we set here
    signed char const ix((delta_x > 0) - (delta_x < 0));
    delta_x = abs(delta_x) << 1;
 
    int delta_y(y1 - y0);
    // if y0 == y1, then it does not matter what we set here
    signed char const iy((delta_y > 0) - (delta_y < 0));
    delta_y = abs(delta_y) << 1;

    float dist = sqrt((x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0));
    float cinc[3];
    int pixel = 0;

    for (int i = 0; i < 3; i++)
        cinc[i] = (color_b[i] - color_a[i])/dist;
 
    _gfx->drawPixel(_left + x0, _top + y0, r5g6b5(color_a, cinc, pixel++));
 
    if (delta_x >= delta_y)
    {
        // error may go below zero
        int error(delta_y - (delta_x >> 1));
 
        while (x0 != x1)
        {
            if ((error >= 0) && (error || (ix > 0)))
            {
                error -= delta_x;
                y0 += iy;
            }
            // else do nothing
 
            error += delta_y;
            x0 += ix;
 
            _gfx->drawPixel(_left + x0, _top + y0, r5g6b5(color_a, cinc, pixel++));
        }
    }
    else
    {
        // error may go below zero
        int error(delta_x - (delta_y >> 1));
 
        while (y0 != y1)
        {
            if ((error >= 0) && (error || (iy > 0)))
            {
                error -= delta_y;
                x0 += ix;
            }
            // else do nothing
 
            error += delta_x;
            y0 += iy;
 
            _gfx->drawPixel(_left + x0, _top + y0, r5g6b5(color_a, cinc, pixel++));
        }
    }
}

void Visualize::_flatten(const float *pos, struct point *pt)
{
    pt->x = (pos[AXIS_X] + pos[AXIS_Y]/4.0) * _scale;
    pt->y = (_height - 1) - (pos[AXIS_Z] + pos[AXIS_Y]/4) * _scale;
}

static void fcolor(float *c, uint16_t r5g6b5, float y)
{
    float dim = (250.0 - y) / 250.0;
    c[0] = ((r5g6b5 >> 11) & 0x1f) / 31.0 * dim;
    c[1] = ((r5g6b5 >>  5) & 0x3f) / 63.0 * dim;
    c[2] = ((r5g6b5 >>  0) & 0x1f) / 31.0 * dim;
}

void Visualize::line_to(int ndx, const float *pos)
{
    struct point loc;

    _flatten(pos, &loc);

    if (ndx >= 0 && ndx < VC_MAX) {
        float c1[3], c2[3];
        fcolor(c1, _color[ndx], _cursor.position[AXIS_Y]);
        fcolor(c2, _color[ndx], pos[AXIS_Y]);
        _line2d_clipped(c1, &_cursor.point, c2, &loc);
    }

    for (int i = 0; i < AXIS_MAX; i++)
        _cursor.position[i] = pos[i];
    _cursor.point = loc;
}

void Visualize::pixel_at(int ndx, const float *pos)
{
    struct point loc;

    if (ndx < 0 || ndx >= VC_MAX)
        return;

    _flatten(pos, &loc);
    _pixel2d_clipped(_color[ndx], &loc);
}

/* vim: set shiftwidth=4 expandtab:  */

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

#ifndef WINDOWGFX_H
#define WINDOWGFX_H

#include <Adafruit_GFX.h>

class WindowGFX : public Adafruit_GFX {
    private:
        Adafruit_GFX *_gfx;
        int16_t _w, _h;
        int16_t _x, _y;
        int8_t _rot;

    public:
        WindowGFX(Adafruit_GFX *gfx, int16_t w, int16_t h, int x = 0, int y = 0)
            : Adafruit_GFX(w, h)
        {
            _gfx = gfx;
            _w = w;
            _h = h;
            _x = x;
            _y = y;
        }

        /* Standard Adafruit_GFX character cell size is 6x8 */
        int16_t cols()
        {
            return width() / 6;
        }

        int16_t rows()
        {
            return height() / 8;
        }

        void setTextCursor(int16_t col, int16_t row)
        {
            setCursor(col * 6, row * 8);
        }

        /* Adafruit_GFX overrides */
        virtual void drawPixel(int16_t x, int16_t y, uint16_t color)
        {
            _rotate_xy(&x, &y, x, y);
            _gfx->drawPixel(x, y, color);
        }

        virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
        {
            _rotate_xy(&x0, &y0);
            _rotate_xy(&x1, &y1);
            _gfx->drawLine(_x + x0, _y + y0,
                           _x + x1, _y + y1, color);
        }

        virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
        {
            int16_t nw, nh;

            _rotate_xy(&x, &y);
            _rotate_wh(&nw, &nh, h, h);

            if (getRotation() & 1)
                _gfx->drawFastHLine(x, y, nw, color);
            else
                _gfx->drawFastVLine(x, y, nh, color);
        }

        virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
        {
            int16_t nw, nh;

            _rotate_xy(&x, &y);
            _rotate_wh(&nw, &nh, w, w);

            if (getRotation() & 1)
                _gfx->drawFastVLine(x, y, nw, color);
            else
                _gfx->drawFastHLine(x, y, nw, color);
        }

        virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        {
            _rotate_xy(&x, &y);
            _rotate_wh(&w, &h);

            _gfx->drawRect(x, y, w, h, color);
        }

        virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        {
            _rotate_xy(&x, &y);
            _rotate_wh(&w, &h);

            _gfx->fillRect(x, y, w, h, color);
        }

        virtual void fillScreen(uint16_t color)
        {
            _gfx->fillRect(_x, _y, _w, _h, color);
        }

        virtual void invertDisplay(boolean inv)
        {
            _gfx->invertDisplay(inv);
        }

    private:
        void _rotate_xy(int16_t *x, int16_t *y, int16_t ox, int16_t oy)
        {
            int16_t nx = _x;
            int16_t ny = _y;

            switch (getRotation()) {
            case 0: /* North */
                nx += ox;
                ny += oy;
                break;
            case 1: /* West */
                nx += (_w - 1) - oy;
                ny += ox;
                break;
            case 2: /* South */
                nx += (_w - 1) - ox;
                ny += (_h - 1) - oy;
                break;
            case 3: /* East */
                nx += oy;
                ny += (_h - 1) - ox;
                break;
            }

            *x = nx;
            *y = ny;
        }

        void _rotate_xy(int16_t *x, int16_t *y)
        {
            _rotate_xy(x, y, *x, *y);
        }

        void _rotate_wh(int16_t *w, int16_t *h, int16_t ow, int16_t oh)
        {
            switch (getRotation()) {
            case 0:
                *w = ow;
                *h = oh;
                break;
            case 1:
                *w = -oh;
                *h = ow;
                break;
            case 2:
                *w = -ow;
                *h = -oh;
                break;
            case 3:
                *w = oh;
                *h = -ow;
                break;
            }
        }

        void _rotate_wh(int16_t *w, int16_t *h)
        {
            _rotate_wh(w, h, *w, *h);
        }
};

#endif /* WINDOWGFX_H */
/* vim: set shiftwidth=4 expandtab:  */

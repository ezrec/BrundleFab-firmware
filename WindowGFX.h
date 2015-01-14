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
        int16_t _rows, _cols;

    public:
        WindowGFX(Adafruit_GFX *gfx, int16_t w, int16_t h, int x = 0, int y = 0)
            : Adafruit_GFX(w, h)
        {
            _gfx = gfx;
            _w = w;
            _h = h;
            _x = x;
            _y = y;

            /* Standard Adafruit_GFX character cell size is 6x8 */
            _cols = w / 6;
            _rows = h / 8;
        }
        
        int16_t cols()
        {
            return _cols;
        }

        int16_t rows()
        {
            return _rows;
        }

        void setTextCursor(int16_t col, int16_t row)
        {
            setCursor(_x + col * 6, _y + row * 8);
        }

        /* Adafruit_GFX overrides */
        virtual void drawPixel(int16_t x, int16_t y, uint16_t color)
        {
            _gfx->drawPixel(_x + x, _y + y, color);
        }

        virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
        {
            _gfx->drawLine(_x + x0, _y + y0,
                           _x + x1, _y + y1, color);
        }

        virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
        {
            _gfx->drawFastVLine(_x + x, _y + y, h, color);
        }

        virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
        {
            _gfx->drawFastHLine(_x + x, _y + y, w, color);
        }

        virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        {
            _gfx->drawRect(_x + x, _y + y, w, h, color);
        }

        virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        {
            _gfx->fillRect(_x + x, _y + y, w, h, color);
        }

        virtual void fillScreen(uint16_t color)
        {
            _gfx->fillRect(_x, _y, _w, _h, color);
        }

        virtual void invertDisplay(boolean inv)
        {
            _gfx->invertDisplay(inv);
        }
};

#endif /* WINDOWGFX_H */
/* vim: set shiftwidth=4 expandtab:  */

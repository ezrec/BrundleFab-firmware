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

#ifndef SIMAVR_ADAFRUIT_ST7735_H
#define SIMAVR_ADAFRUIT_ST7735_H

#include <Adafruit_GFX.h>

#include <SDL.h>

// some flags for initR() :(
#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB   0x2

#define INITR_18GREENTAB    INITR_GREENTAB
#define INITR_18REDTAB      INITR_REDTAB
#define INITR_18BLACKTAB    INITR_BLACKTAB
#define INITR_144GREENTAB   0x1

#define ST7735_TFTWIDTH  128
// for 1.44" display
#define ST7735_TFTHEIGHT_144 128
// for 1.8" display
#define ST7735_TFTHEIGHT_18  160

// Color definitions: r:5,g:6,b:5
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0  
#define ST7735_WHITE   0xFFFF

class Adafruit_ST7735 : public Adafruit_GFX {
    private:
        SDL_Surface *_surface;
        int _lock;

        void _pixel_lock()
        {
            if (_lock++ == 0)
                SDL_LockSurface(_surface);
        }

        void _pixel_unlock()
        {
            if (--_lock == 0) {
                SDL_UnlockSurface(_surface);
                SDL_Flip(_surface);
            }
        }

    public:
        Adafruit_ST7735(int pin_cs, int pin_dc, int pin_rst)
            : Adafruit_GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT_18)
        {
            _lock = 0;
        }
        ~Adafruit_ST7735()
        {
        }

        void initR(uint8_t options = INITR_GREENTAB)
        {
            _surface = SDL_SetVideoMode(
                      ST7735_TFTWIDTH * 2, ST7735_TFTHEIGHT_18 * 2,
                      32, SDL_SWSURFACE);
            SDL_WM_SetCaption("BrundleFab: TFT", NULL);
            SDL_FillRect(_surface, NULL, SDL_MapRGB(_surface->format, 0xff, 0xff, 0xff));
            SDL_Flip(_surface);
        }

        void drawPixel(int16_t x, int16_t y, uint16_t color)
        {
            Uint32 pixel;
            if (x < 0 || x >= WIDTH || y <0 || y >= HEIGHT)
                return;
            
            pixel = SDL_MapRGB(_surface->format, 
                    ((color >> 11) << 3) & 0xff,
                    ((color >>  5) << 2) & 0xff,
                    ((color >>  0) << 3) & 0xff);

            x *= 2;
            y *= 2;
            _pixel_lock();
            ((Uint32 *)_surface->pixels)[y * _surface->w + x] = pixel;
            ((Uint32 *)_surface->pixels)[y * _surface->w + x + 1] = pixel;
            ((Uint32 *)_surface->pixels)[(y + 1) * _surface->w + x] = pixel;
            ((Uint32 *)_surface->pixels)[(y + 1)* _surface->w + x + 1] = pixel;
            _pixel_unlock();
        }

        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
        {
            _pixel_lock();
            Adafruit_GFX::drawLine(x0, y0, x1, y1, color);
            _pixel_unlock();
        }

        void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
        {
            _pixel_lock();
            Adafruit_GFX::drawFastVLine(x, y, h, color);
            _pixel_unlock();
        }

        void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
        {
            _pixel_lock();
            Adafruit_GFX::drawFastHLine(x, y, w, color);
            _pixel_unlock();
        }

        void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        {
            _pixel_lock();
            Adafruit_GFX::drawRect(x, y, w, h, color);
            _pixel_unlock();
        }

        virtual size_t write(uint8_t c)
        {
            size_t res;
            _pixel_lock();
            res = Adafruit_GFX::write(c);
            _pixel_unlock();
            return res;
        }


        void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        {
            Uint32 pixel = SDL_MapRGB(_surface->format, 
                    ((color >> 11) << 3) & 0xff,
                    ((color >>  5) << 2) & 0xff,
                    ((color >>  0) << 3) & 0xff);
            SDL_Rect r;

            r.x = x * 2;
            r.y = y * 2;
            r.w = w * 2;
            r.h = h * 2;
            _pixel_lock();
            SDL_FillRect(_surface, &r, pixel);
            _pixel_unlock();
        }
};

#endif /* SIMAVR_ADAFRUIT_ST7735_H */
/* vim: set shiftwidth=4 expandtab:  */

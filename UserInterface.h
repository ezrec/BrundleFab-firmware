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

#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "Axis.h"
#include "Adafruit_GFX.h"
#include "CNC.h"

enum ui_key {
    UI_KEY_NONE = 0,
    UI_KEY_SELECT,
    UI_KEY_UP, UI_KEY_DOWN,
    UI_KEY_LEFT, UI_KEY_RIGHT
};

#define UI_STATUS_MAX   64

#define UI_COLOR_BACKGROUND   0
#define UI_COLOR_TEXT         1
#define UI_COLOR_STATUS       2
#define UI_COLOR_MAX          3

#define UI_SWITCH_OPTIONAL_STOP 0

#define UI_BUTTON_CYCLE_START   0

class UserInterface;

class Menu {
    public:
        Menu() { }
        virtual void begin(UserInterface *ui) = 0;
        virtual Menu *update(UserInterface *ui, unsigned long now, enum ui_key key = UI_KEY_NONE) = 0;
};

class MenuMain : public Menu {
    private:
        struct {
            bool enable;
            bool blink;
            unsigned long time;
        } _status;
    public:
        virtual void begin(UserInterface *ui);
        virtual Menu *update(UserInterface *ui, unsigned long now, enum ui_key key = UI_KEY_NONE);
};

extern MenuMain UserInterfaceMenuMain;

class UserInterface : public Adafruit_GFX {
    private:
        Adafruit_GFX *_gfx;
        CNC *_cnc;
        
        int _width, _height, _top, _left;
        uint16_t _cnc_switch_mask;
        uint16_t _cnc_button_mask;

        int _rows, _cols;

        uint16_t _color[UI_COLOR_MAX];

        unsigned long _update_time;

        char _status[UI_STATUS_MAX];

        bool _paused;

        Menu *_menu;

    public:
        UserInterface(CNC *cnc, Adafruit_GFX *gfx, int w, int h, int left, int top) : Adafruit_GFX(w, h)
        {
            _cnc = cnc;
            _top = top;
            _left = left;
            _gfx = gfx;

            /* Standard Adafruit_GFX character cell size is 6x8 */
            _cols = width() / 6;
            _rows = height() / 8;

            _color[UI_COLOR_BACKGROUND] = 0;
            _color[UI_COLOR_TEXT] = 0xffff;
            _color[UI_COLOR_STATUS] = 0xffff;
        }

        void color_set(int nsel, uint16_t color)
        {
            _color[nsel] = color;
        }

        uint16_t color(int nsel)
        {
            return _color[nsel];
        }

        int rows()
        {
            return _rows;
        }

        int cols()
        {
            return _cols;
        }

        CNC *cnc()
        {
            return _cnc;
        }

        void begin()
        {
            _menu = &UserInterfaceMenuMain;
            _menu->begin(this);
            _update_time = millis();
        }

        void clear(const char *title);

        void status_set(const char *message)
        {
            if (!message || message[0] == 0) {
                _status[0] = 0;
            } else {
                strncpy(_status, message, UI_STATUS_MAX);
            }
        }
        
        const char *status()
        {
            return _status[0] ? _status : NULL;
        }

        void update(enum ui_key key = UI_KEY_NONE)
        {
            unsigned long now = millis();
            Menu *prev = _menu;

            if (key != UI_KEY_NONE || now > _update_time) {
                _menu = _menu->update(this, now, key);
                if (_menu != prev)
                    _menu->begin(this);
                _update_time = now + 500;
            }
        }

        void on_pause()
        {
            _paused = true;
        }

        void on_run()
        {
            _paused = false;
        }

        bool cnc_button(int cnc_button)
        {
            uint16_t mask = (1 << cnc_button);
            bool pressed;

            pressed = (_cnc_button_mask & mask) ? true : false;
                
            _cnc_button_mask &= ~mask;

            return pressed;
        }

        void cnc_button_set(int cnc_button)
        {
            uint16_t mask = (1 << cnc_button);

            _cnc_button_mask |= mask;
        }

        bool cnc_switch(int cnc_switch)
        {
            return (_cnc_switch_mask & (1 << cnc_switch)) ? true : false;
        }

        void cnc_switch_set(int cnc_switch, bool enabled = true)
        {
            uint16_t mask = (1 << cnc_switch);
            if (enabled)
                _cnc_switch_mask |= mask;
            else
                _cnc_switch_mask &= ~mask;
        }

        /* Adafruit_GFX overrides */
        virtual void drawPixel(int16_t x, int16_t y, uint16_t color)
        {
            _gfx->drawPixel(_left + x, _top + y, color);
        }

        virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
        {
            _gfx->drawLine(_left + x0, _top + y0,
                           _left + x1, _top + y1, color);
        }

        virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
        {
            _gfx->drawFastVLine(_left + x, _top + y, h, color);
        }

        virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
        {
            _gfx->drawFastHLine(_left + x, _top + y, w, color);
        }

        virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        {
            _gfx->drawRect(_left + x, _top + y, w, h, color);
        }

        virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
        {
            _gfx->fillRect(_left + x, _top + y, w, h, color);
        }

        virtual void fillScreen(uint16_t color)
        {
            _gfx->fillRect(_left, _top, _width, _height, color);
        }

        virtual void invertDisplay(boolean inv)
        {
            _gfx->invertDisplay(inv);
        }
};

#endif /* USERINTERFACE_H */
/* vim: set shiftwidth=4 expandtab:  */

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
#include "WindowGFX.h"
#include "CNC.h"

enum ui_key {
    UI_KEY_NONE = 0,
    UI_KEY_SELECT,
    UI_KEY_UP, UI_KEY_DOWN,
    UI_KEY_LEFT, UI_KEY_RIGHT
};

#define UI_COLOR_BACKGROUND     0
#define UI_COLOR_TEXT           1
#define UI_COLOR_STATUS         2
#define UI_COLOR_MAX            3

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

class UserInterface : public WindowGFX {
    private:
        CNC *_cnc;
        
        uint16_t _color[UI_COLOR_MAX];

        unsigned long _update_time;

        Menu *_menu;

    public:
        UserInterface(CNC *cnc, Adafruit_GFX *gfx, int w, int h, int left, int top) : WindowGFX(gfx, w, h, left, top)
        {
            _cnc = cnc;

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

        CNC *cnc()
        {
            return _cnc;
        }

        void begin()
        {
            _menu = &UserInterfaceMenuMain;
            _menu->begin(this);
            _menu->update(this, 0);
            _update_time = millis();
        }

        void clear(const char *title, const char *subtitle = NULL);

        bool update(enum ui_key key = UI_KEY_NONE)
        {
            unsigned long now = millis();
            Menu *prev = _menu;

            if (key != UI_KEY_NONE || now > _update_time) {
                _menu = _menu->update(this, now, key);
                if (_menu != prev) {
                    _menu->begin(this);
                    _menu->update(this, 0);
                }
                _update_time = now + 500;
            }

            return (_menu != &UserInterfaceMenuMain);
        }

        void setTextCursor(int16_t col, int16_t row)
        {
            WindowGFX::setTextCursor(col, row + 1);
        }

        int16_t rows()
        {
            return WindowGFX::rows() - 1;
        }
};

#endif /* USERINTERFACE_H */
/* vim: set shiftwidth=4 expandtab:  */

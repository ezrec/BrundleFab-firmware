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

#define UI_STATUS_MAX           32
#define UI_MESSAGE_MAX          32

#define UI_COLOR_BACKGROUND     0
#define UI_COLOR_TEXT           1
#define UI_COLOR_STATUS         2
#define UI_COLOR_MAX            3

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

class UserInterface : public WindowGFX {
    private:
        CNC *_cnc;
        
        uint16_t _cnc_switch_mask;
        uint16_t _cnc_button_mask;

        uint16_t _color[UI_COLOR_MAX];

        unsigned long _update_time;

        char _status[UI_STATUS_MAX];
        char _message[UI_MESSAGE_MAX];
        bool _message_updated;

        bool _paused;

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

        void message_set(const char *message)
        {
            if (!message || message[0] == 0) {
                _message[0] = 0;
            } else {
                strncpy(_message, message, UI_STATUS_MAX);
            }
            _message_updated = true;
        }

        const char *message(bool *updated = NULL)
        {
            if (updated) {
                *updated = _message_updated;
                _message_updated = false;
            }

            return _message[0] ? _message : NULL;
        }

         void update(enum ui_key key = UI_KEY_NONE)
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

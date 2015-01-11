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
    UI_KEY_SELECT,
    UI_KEY_UP, UI_KEY_DOWN,
    UI_KEY_LEFT, UI_KEY_RIGHT
};

#define UI_STATUS_MAX   64

#define UC_BACKGROUND   0
#define UC_TEXT         1
#define UC_STATUS       2

#define UC_MAX          3

class UserInterface {
    private:
        char _status[UI_STATUS_MAX];
        Adafruit_GFX *_gfx;
        CNC *_cnc;
        
        int _width, _height, _top, _left;

        int _rows, _cols;

        uint16_t _color[UC_MAX];

        unsigned long _update_time;

        bool _blink;

    public:
        UserInterface(CNC *cnc, Adafruit_GFX *gfx, int w, int h, int left, int top)
        {
            _cnc = cnc;
            _width = w;
            _height = h;
            _top = top;
            _left = left;
            _gfx = gfx;

            /* Standard Adafruit_GFX character cell size is 6x8 */
            _cols = _width / 6;
            _rows = _height / 8;

            _color[UC_BACKGROUND] = 0;
            _color[UC_TEXT] = 0xffff;
            _color[UC_STATUS] = 0xffff;
        }

        void color_set(int nsel, uint16_t color)
        {
            _color[nsel] = color;
        }

        void begin()
        {
            _gfx->fillRect(_left, _top, _width, _height, _color[UC_BACKGROUND]);

            _gfx->setTextWrap(false);
            _gfx->setTextColor(_color[UC_TEXT], _color[UC_BACKGROUND]);
            if (_cols < 12) {
                _gfx->setCursor(_left, _top);
            } else {
                _gfx->setCursor((_width - 12*6)/2 + _left, _top);
            }
            _gfx->print(" BrundleFab ");

            _update_time = millis();
        }

        void status(const char *message)
        {
            if (!message || message[0] == 0) {
                _status[0] = 0;
                _gfx->fillRect(_left, _top + 8, _width, 8, _color[UC_BACKGROUND]);
            } else {
                strncpy(_status, message, UI_STATUS_MAX);
                _blink = true;
            }
        }

        void update()
        {
            unsigned long now = millis();

            if (millis() < _update_time)
                return;
            
            if (_status[0]) {
                uint16_t c1, c2;
                if (_blink) {
                    c1 = _color[UC_STATUS];
                    c2 = _color[UC_BACKGROUND];
                } else {
                    c1 = _color[UC_BACKGROUND];
                    c2 = _color[UC_STATUS];
                }
                _gfx->fillRect(_left, _top + 8, _width, 8, c2);
                _gfx->setCursor(_left, _top + 8);
                _gfx->setTextWrap(false);
                _gfx->setTextColor(c1, c2);
                _gfx->print(_status);
                _blink = !_blink;
            }

            float pos[AXIS_MAX];
            int tool = _cnc->toolhead()->selected();

            for (int i = 0; i < AXIS_MAX; i++) {
                Axis *axis = _cnc->axis(i);
                pos[i] = axis->position_get_mm();
                _gfx->setCursor(_left + (1 + 1 + 1 + 3 + 1 + 2 + 1) * 6 * (i & 1), _top + 8 * (3 + (i>>1)));
                _gfx->setTextColor(_color[UC_TEXT], _color[UC_BACKGROUND]);
                _gfx->print("XYZE"[i]); _gfx->print(": ");
                _gfx->setTextColor(axis->motor_active() ? _color[UC_STATUS] : _color[UC_TEXT], _color[UC_BACKGROUND]);
                _gfx->print(pos[i], (pos[i] < 0) ? 2 : 3);
            }

            _gfx->setCursor(_left + 2 + (_gfx->width()-6*3)/2, _top + 8 * 5 + 2);
            _gfx->setTextColor(tool ? _color[UC_STATUS] : _color[UC_TEXT], _color[UC_BACKGROUND]);
            _gfx->print("T");_gfx->print(tool);

            _update_time = now + 500;
        }

};

#endif /* USERINTERFACE_H */
/* vim: set shiftwidth=4 expandtab:  */

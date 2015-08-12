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

#include "SD.h"

#include "UserInterface.h"

static MenuMain UserInterfaceMenuMain;

static const float menu_axis_incr[] = {
    -100.0,
    -10.0,
    -1.0,
    -0.1,
    0,
    0.1,
    1.0,
    10.0,
    100.0
};

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

static void axis_report(UserInterface *ui, int col, int row, int sel_axis = -1)
{
    uint16_t bg, fg, st;
    float pos[AXIS_MAX];

    bg = ui->color(UI_COLOR_BACKGROUND);
    fg = ui->color(UI_COLOR_TEXT);
    st = ui->color(UI_COLOR_STATUS);

    int tool = ui->cnc()->toolhead()->selected();
    ui->setTextCursor(col, row);
    ui->setTextColor(tool ? st : fg, bg);
    ui->print("T");
    ui->print(tool);
    ui->print("  ");

    for (int i = 0; i < AXIS_MAX; i++) {
        Axis *axis = ui->cnc()->axis(i);
        bool selected = sel_axis == i;
        pos[i] = axis->position_get();
        ui->setTextCursor(col + 1 + (1 + 1 + 1 + 3 + 1 + 2 + 1) * (i >> 1), row + 1 + (i & 1));
        ui->setTextColor(selected ? st : fg, bg);
        ui->print("XYZE"[i]); ui->print(":");
        if (sel_axis < 0)
            ui->setTextColor(axis->motor_active() ? st : fg, bg);
        else
            ui->setTextColor(selected ? bg : fg, selected ? fg : bg);

        ui->print(" ");
        ui->print(pos[i], 2);
        ui->print(" ");
    }
}

class MenuAxis : public Menu {
    private:
        int _axis;
        uint16_t _incr;
        enum { SEL_AXIS = 0, SEL_INCR } _sel;
    public:
        void begin(UserInterface *ui)
        {
            ui->clear("Manual Move");
            _axis = 0;
            _incr = ARRAY_SIZE(menu_axis_incr)/2;
            _sel = SEL_INCR;
        }

        /* Manual Move 
         * Axis: x Incr: nn
         * X: 00000 Y: 0000
         * Z: 00000 E: 0000
         */
        Menu *update(UserInterface *ui, unsigned long now, enum ui_key key)
        {
            uint16_t bg, fg, st;

            bg = ui->color(UI_COLOR_BACKGROUND);
            fg = ui->color(UI_COLOR_TEXT);
            st = ui->color(UI_COLOR_STATUS);

            ui->setTextCursor(0, 0);

            if (_sel == SEL_AXIS) {
                if (key == UI_KEY_DOWN) {
                    _axis++;
                    if (_axis >= AXIS_MAX)
                        _axis = 0;
                } else if (key == UI_KEY_UP) {
                    _axis--;
                    if (_axis < 0)
                        _axis = AXIS_MAX-1;
                } else if (key == UI_KEY_RIGHT)
                    _sel = SEL_INCR;
            } else if (_sel == SEL_INCR) {
                if (key == UI_KEY_UP && _incr < (ARRAY_SIZE(menu_axis_incr)-1))
                    _incr++;
                else if (key == UI_KEY_DOWN && _incr > 0)
                    _incr--;
                else if (key == UI_KEY_LEFT)
                    _sel = SEL_AXIS;
                else if (key == UI_KEY_RIGHT)
                    return &UserInterfaceMenuMain;
            }

            if (key == UI_KEY_SELECT) {
                if (menu_axis_incr[_incr] == 0)
                    ui->cnc()->axis(_axis)->home();
                else {
                    float pos = ui->cnc()->axis(_axis)->target_get();
                    pos += menu_axis_incr[_incr];
                    ui->cnc()->axis(_axis)->target_set(pos);
                }
            }

            ui->setTextWrap(false);
            ui->setTextColor(fg, bg);
            ui->print("Axis:");

            if (_sel == SEL_AXIS)
                ui->setTextColor(fg, st);
            else
                ui->setTextColor(st, bg);
            ui->print(" ");
            ui->print("XYZE"[_axis]);
            ui->print(" ");

            ui->setTextColor(fg, bg);
            ui->print(" Incr:");
            if (_sel == SEL_INCR)
                ui->setTextColor(fg, st);
            else
                ui->setTextColor(st, bg);
            ui->print(" ");
            if (menu_axis_incr[_incr] == 0)
                ui->print("HOME");
            else
                ui->print(menu_axis_incr[_incr], 2);
            ui->print(" ");
            ui->setTextColor(fg, bg);
            ui->print("    ");

            axis_report(ui, 0, 1, _axis);
            return this;
        }
};

static MenuAxis UserInterfaceMenuAxis;

#if ENABLE_SD
class MenuSD : public Menu {
    private:
        File _file;
        uint16_t _sel, _ith, _max;

    public:
        void begin(UserInterface *ui)
        {
            begin(ui, NULL);
        }

        void begin(UserInterface *ui, File *file)
        {
            if (_file)
                _file.close();
            if (file)
                _file = *file;
            else
                _file = SD.open("");

            _ith = 0;
            _max = ~0;
            _sel = 0;
        }

        Menu *update(UserInterface *ui, unsigned long now, enum ui_key key)
        {
            char *name;

            if (!_file)
                return &UserInterfaceMenuMain;

            /* Don't refresh the screen unless we have a key,
             * or the special 'now = 0'
             */
            if (now && key == UI_KEY_NONE)
                return this;

            name = _file.name();
            ui->clear("SD: ", name[0] ? name : "/");

            if (_file.isDirectory())
                return _update_dir(ui, now, key);
            else
                return _update_file(ui, now, key);
        }

    private:
        /* SD: /dir
         *  fname1
         * /dirname
         *  fname2
         *  fname3
         */
        Menu *_update_dir(UserInterface *ui, unsigned long now, enum ui_key key)
        {
            uint16_t fg, bg;
            uint16_t rows = ui->rows();
            File tmp;

            switch (key) {
            case UI_KEY_UP:
                if (_ith > 0) {
                    _ith--;
                    _sel--;
                } else if (_sel > 0) {
                    _sel--;
                }
                break;
            case UI_KEY_DOWN:
                if (_ith < _max) {
                    _ith++;
                    _sel++;
                } else if (_sel < _max) {
                    _sel++;
                }
                break;
            case UI_KEY_LEFT:
                if (_ith < rows) {
                    if (_file.name()[0] == 0) {
                        _file.close();
                        return &UserInterfaceMenuMain;
                    }
                    begin(ui);
                    update(ui, 0, UI_KEY_NONE);
                    return this;
                } else {
                    _ith -= rows;
                    _sel += rows;
                }
                break;
            case UI_KEY_RIGHT:
                if ((_ith + rows) < _max) {
                    _ith += rows;
                    _sel += rows;
                }
                break;
            case UI_KEY_SELECT:
                _file.rewindDirectory();
                for (uint16_t i = 0; i < _sel; i++) {
                    tmp = _file.openNextFile();
                    tmp.close();
                }
                tmp = _file.openNextFile();
                begin(ui, &tmp);
                update(ui, 0, UI_KEY_NONE);
                return this;
            default:
                break;
            }

            _file.rewindDirectory();
            for (uint16_t i = 0; i < _ith; i++) {
                tmp = _file.openNextFile();
                if (!tmp) {
                    _max = i;
                    _ith = (i - 1);
                    break;
                }
                tmp.close();
            }

            bg = ui->color(UI_COLOR_BACKGROUND);
            fg = ui->color(UI_COLOR_TEXT);

            ui->setTextWrap(false);
            for (uint16_t i = 0; i < (_max - _ith) && i < rows; i++) {
                tmp = _file.openNextFile();
                if (!tmp)
                    break;
                if ((_ith + i) == _sel)
                    ui->setTextColor(bg, fg);
                else
                    ui->setTextColor(fg, bg);
                ui->setTextCursor(0, i);
                if (tmp.isDirectory())
                    ui->print("/");
                else
                    ui->print(" ");
                ui->print(tmp.name());
                tmp.close();
            }

            return this;
        }

        /* SD: /dir/file
         *  [..]
         *  Print
         *  Delete
         */
        Menu *_update_file(UserInterface *ui, unsigned long now, enum ui_key key)
        {
            uint32_t size = _file.size();
            int block = ui->rows() * ui->cols();
            int c;

            switch (key) {
            case UI_KEY_LEFT:
                begin(ui);
                update(ui, 0, UI_KEY_NONE);
                return this;
            case UI_KEY_UP:
                if (_ith > 0)
                    _ith--;
                break;
            case UI_KEY_DOWN:
                if (_ith < (size + block - 1) / block)
                    _ith++;
                break;
            case UI_KEY_SELECT:
                ui->cnc()->program_set(&_file);
                return &UserInterfaceMenuMain;
            default:
                break;
            }

            ui->setTextCursor(0, 0);
            ui->setTextWrap(true);
            _file.seek(block * _ith);
            for (int i = 0; i < block; i++) {
                c = _file.read();
                if ( c < 0)
                    break;
                if (c == '\r') c = '.';
                if (c == '\n') c = '\\';
                ui->print((char)c);
            }

            return this;
        }
};

static MenuSD UserInterfaceMenuSD;
#endif /* ENABLE_SD */

void UserInterface::begin()
{
    _menu = &UserInterfaceMenuMain;
    _menu->begin(this);
    _menu->update(this, 0);
    _update_time = millis();
}

bool UserInterface::update(enum ui_key key)
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

void UserInterface::clear(const char *title, const char *subtitle)
{
    uint16_t bg, fg;
    int tlen = strlen(title);
    int slen = subtitle ? strlen(subtitle) : 0;

    bg = color(UI_COLOR_BACKGROUND);
    fg = color(UI_COLOR_TEXT);

    fillScreen(bg);
    fillRect(0, 0, width(), 8, fg);

    setTextWrap(false);
    setTextColor(bg, fg);

    if (slen || (cols() < tlen)) {
        setCursor(0, 0);
    } else {
        setCursor((width() - tlen*6)/2, 0);
    }

    print(title);
    if (slen)
        print(subtitle);
}

void MenuMain::begin(UserInterface *ui)
{
    ui->clear("BrundleFab");
}

Menu *MenuMain::update(UserInterface *ui, unsigned long now, enum ui_key key)
{
    uint16_t bg, fg, st;
    const char *status;

    if (key != UI_KEY_NONE) {
        switch (key) {
        case UI_KEY_SELECT:
            ui->cnc()->button_set(CNC_BUTTON_CYCLE_START);
            break;
#if ENABLE_SD
        case UI_KEY_RIGHT:
            return &UserInterfaceMenuSD;
#endif
        case UI_KEY_LEFT:
            return &UserInterfaceMenuAxis;
        default:
            break;
        }
    }

    bg = ui->color(UI_COLOR_BACKGROUND);
    fg = ui->color(UI_COLOR_TEXT);
    st = ui->color(UI_COLOR_STATUS);
  
    bool updated;
    const char *message = ui->cnc()->message_get(&updated);

    status = ui->cnc()->status_get();
    if (!_status.enable && status) {
        _status.enable = true;
        _status.time = now;
        _status.blink = false;
    } else if (_status.enable && !status) {
        ui->fillRect(0, 8, ui->width(), 8, bg);
        _status.enable = false;
        updated = true;
    }
    
    ui->setTextWrap(false);
    if (_status.enable && _status.time <= now) {
        uint16_t c1, c2;
        if (_status.blink) {
            c1 = st;
            c2 = bg;
        } else {
            c1 = bg;
            c2 = st;
        }
        ui->fillRect(0, 8, ui->width(), 8, c2);
        ui->setTextCursor(0, 0);
        ui->setTextColor(c1, c2);
        ui->print(status);
        _status.blink = !_status.blink;
        _status.time = now + 500;
    }

    if (!now || updated) {
        ui->fillRect(0, 8, ui->width(), 8, bg);
        if (message) {
            ui->setTextCursor(0, 0);
            ui->setTextColor(fg, bg);
            ui->print(message);
        }
    }

    axis_report(ui, 0, 1);

    return this;
}

/* vim: set shiftwidth=4 expandtab:  */

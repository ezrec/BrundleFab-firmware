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

#include "UserInterface.h"

static const float menu_axis_incr[] = {
    -100.0,
    -10.0,
    -1.0,
    -0.1,
    0.1,
    1.0,
    10.0,
    100.0
};

static void axis_report(UserInterface *ui, int x, int y, int sel_axis = -1)
{
    uint16_t bg, fg, st;
    float pos[AXIS_MAX];

    bg = ui->color(UI_COLOR_BACKGROUND);
    fg = ui->color(UI_COLOR_TEXT);
    st = ui->color(UI_COLOR_STATUS);

    int tool = ui->cnc()->toolhead()->selected();
    ui->setCursor(x, y);
    ui->setTextColor(tool ? st : fg, bg);
    ui->print("T");
    ui->print(tool);
    ui->print("  ");

    for (int i = 0; i < AXIS_MAX; i++) {
        Axis *axis = ui->cnc()->axis(i);
        bool selected = sel_axis == i;
        pos[i] = axis->position_get_mm();
        ui->setCursor(x + 6 * (1 + (1 + 1 + 1 + 3 + 1 + 2 + 1) * (i >> 1)), y + 8 * (1 + (i & 1)));
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
        int _incr;
        enum { SEL_AXIS = 0, SEL_INCR } _sel;
    public:
        void begin(UserInterface *ui)
        {
            ui->clear("Manual Move");
            _axis = 0;
            _incr = 5;
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

            ui->setCursor(0, 8);

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
                else if (key == UI_KEY_LEFT)
                    return &UserInterfaceMenuMain;
            } else if (_sel == SEL_INCR) {
                if (key == UI_KEY_UP && _incr < (int)(sizeof(menu_axis_incr)/sizeof(menu_axis_incr[0])-1))
                    _incr++;
                else if (key == UI_KEY_DOWN && _incr > 0)
                    _incr--;
                else if (key == UI_KEY_LEFT)
                    _sel = SEL_AXIS;
            }

            if (key == UI_KEY_SELECT) {
                ui->cnc()->axis(_axis)->target_move_mm(menu_axis_incr[_incr]);
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
            ui->print(menu_axis_incr[_incr], 2);
            ui->print(" ");
            ui->setTextColor(fg, bg);
            ui->print("    ");

            axis_report(ui, 0, 2*8, _axis);
            return this;
        }
};

MenuAxis UserInterfaceMenuAxis;

void UserInterface::clear(const char *title)
{
    uint16_t bg, fg;
    int tlen = strlen(title);

    bg = color(UI_COLOR_BACKGROUND);
    fg = color(UI_COLOR_TEXT);

    fillScreen(bg);
    fillRect(0, 0, width(), 8, fg);

    setTextWrap(false);
    setTextColor(bg, fg);

    if (cols() < tlen) {
        setCursor(0, 0);
    } else {
        setCursor((width() - tlen*6)/2, 0);
    }

    print(title);
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
            ui->cnc_button_set(UI_BUTTON_CYCLE_START);
            break;
        case UI_KEY_RIGHT:
            return &UserInterfaceMenuAxis;
        default:
            break;
        }
    }

    bg = ui->color(UI_COLOR_BACKGROUND);
    fg = ui->color(UI_COLOR_TEXT);
    st = ui->color(UI_COLOR_STATUS);
  
    bool updated;
    const char *message = ui->message(&updated);

    status = ui->status();
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
        ui->setCursor(0, 8);
        ui->setTextColor(c1, c2);
        ui->print(status);
        _status.blink = !_status.blink;
        _status.time = now + 500;
    }

    if (!now || updated) {
        ui->fillRect(0, 8, ui->width(), 8, bg);
        if (message) {
            ui->setCursor(0, 8);
            ui->setTextColor(fg, bg);
            ui->print(message);
        }
    }

    axis_report(ui, 0, 2*8);

    return this;
}

MenuMain UserInterfaceMenuMain;

/* vim: set shiftwidth=4 expandtab:  */

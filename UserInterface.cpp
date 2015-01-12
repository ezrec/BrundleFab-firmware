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

void MainMenu::begin(UserInterface *ui)
{
    uint16_t bg, fg;

    bg = ui->color(UI_COLOR_BACKGROUND);
    fg = ui->color(UI_COLOR_TEXT);

    ui->fillScreen(bg);
    ui->setTextWrap(false);
    ui->setTextColor(fg, bg);
    if (ui->cols() < 12) {
        ui->setCursor(0, 0);
    } else {
        ui->setCursor((ui->width() - 12*6)/2, 0);
    }
    ui->print(" BrundleFab ");
}

Menu *MainMenu::update(UserInterface *ui, unsigned long now, enum ui_key key)
{
    uint16_t fg, bg, st;
    const char *status;

    if (key != UI_KEY_NONE) {
        if (key == UI_KEY_SELECT)
            ui->cnc_button_set(UI_BUTTON_CYCLE_START);
    }

    bg = ui->color(UI_COLOR_BACKGROUND);
    fg = ui->color(UI_COLOR_TEXT);
    st = ui->color(UI_COLOR_STATUS);
  
    status = ui->status();
    if (!_status.enable && status) {
        _status.enable = true;
        _status.time = now;
        _status.blink = false;
    } else if (_status.enable && !status) {
        ui->fillRect(0, 8, ui->width(), 8, bg);
        _status.enable = false;
    }
    
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
        ui->setTextWrap(false);
        ui->setTextColor(c1, c2);
        ui->print(status);
        _status.blink = !_status.blink;
        _status.time = now + 500;
    }

    float pos[AXIS_MAX];
    int tool = ui->cnc()->toolhead()->selected();

    for (int i = 0; i < AXIS_MAX; i++) {
        Axis *axis = ui->cnc()->axis(i);
        pos[i] = axis->position_get_mm();
        ui->setCursor((1 + 1 + 1 + 3 + 1 + 2 + 1) * 6 * (i & 1), 8 * (3 + (i>>1)));
        ui->setTextColor(fg, bg);
        ui->print("XYZE"[i]); ui->print(": ");
        ui->setTextColor(axis->motor_active() ? st : fg, bg);
        ui->print(pos[i], (pos[i] < 0) ? 2 : 3);
    }

    ui->setCursor(2 + (ui->width()-6*3)/2, 8 * 5 + 2);
    ui->setTextColor(tool ? st : fg, bg);
    ui->print("T");ui->print(tool);

    return this;
}

MainMenu UserInterfaceMainMenu;

/* vim: set shiftwidth=4 expandtab:  */

/*
 * Copyright (C) 2014, Netronome, Inc.
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

#include <Arduino.h>
#include <Wire.h>
#include <SD.h>

#include <Adafruit_ST7735.h>

#include "GCode.h"

#include "Axis.h"
#include "Axis_X.h"
#include "Axis_Y.h"
#include "Axis_Z.h"
#include "Axis_E.h"

#include "ToolHead.h"
#include "ToolInk.h"
#include "ToolFuser.h"

#include "Visualize.h"

#include "Adafruit_Joystick.h"

Adafruit_MotorShield AFMS;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

ToolHead tools;
ToolInk toolInk_Black;
ToolFuser toolFuser = ToolFuser(FUSER_ENABLE);
#ifdef AXIS_NULL
Axis axisX;
Axis axisY;
Axis axisZ;
Axis axisE;
#else
Axis_X axisX;
Axis_Y axisY;
Axis_Z axisZ;
Axis_E axisE;
#endif

Visualize vis = Visualize(&tft, ST7735_TFTWIDTH, ST7735_TFTHEIGHT_18 - 5*8,
                                0, 5 * 8);

CNC cnc = CNC(&axisX, &axisY, &axisZ, &axisE, &tools);

UserInterface ui = UserInterface(&cnc, &tft, ST7735_TFTWIDTH, 5*8, 0, 0);

GCode gcode = GCode(&Serial, &cnc, &ui, &vis);

Adafruit_Joystick joy = Adafruit_Joystick(JOY_PIN);

void setup()
{
    Serial.begin(115200);
    AFMS.begin(1000);
    SD.begin(SD_CS);

    toolInk_Black.begin();
    toolFuser.begin();

    tools.attach(TOOL_INK_BLACK, &toolInk_Black);
    tools.attach(TOOL_FUSER, &toolFuser);
    tools.begin();

    tft.initR(TFT_INITR);

    ui.color_set(UI_COLOR_BACKGROUND, ST7735_WHITE);
    ui.color_set(UI_COLOR_TEXT, ST7735_BLACK);
    ui.color_set(UI_COLOR_STATUS, ST7735_RED);
    ui.begin();

    vis.color_set(VC_AXIS + AXIS_X, ST7735_RED);
    vis.color_set(VC_AXIS + AXIS_Y, ST7735_GREEN);
    vis.color_set(VC_AXIS + AXIS_Z, ST7735_BLUE);
    vis.color_set(VC_MOVE, ST7735_CYAN);
    vis.color_set(VC_FEED, ST7735_YELLOW);
    vis.color_set(VC_TOOL, ST7735_WHITE);
    vis.clear(175, 260, 175);

    axisX.begin();
    axisY.begin();
    axisZ.begin();
    axisE.begin();

    gcode.begin();
}

static unsigned long next_update = millis();

static enum ui_key keymap(int joy)
{
    static int last_joy = AFJOYSTICK_NONE;

    if (joy == last_joy)
        return UI_KEY_NONE;

    last_joy = joy;

    switch (joy) {
    case AFJOYSTICK_DOWN:   return UI_KEY_DOWN;
    case AFJOYSTICK_UP:     return UI_KEY_UP;
    case AFJOYSTICK_LEFT:   return UI_KEY_LEFT;
    case AFJOYSTICK_RIGHT:  return UI_KEY_RIGHT;
    case AFJOYSTICK_SELECT: return UI_KEY_SELECT;
    }

    return UI_KEY_NONE;
}

void loop()
{
    enum ui_key key;

    gcode.update();

    key = keymap(joy.read());
    ui.update(key);
}

/* vim: set shiftwidth=4 expandtab:  */

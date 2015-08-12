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

#include "config.h"

#include <Arduino.h>
#include <Wire.h>
#if ENABLE_SD
#include <SD.h>
#endif

#include "GCode.h"
#include "Axis.h"
#include "ToolHead.h"

#include "pinout.h"

#if ENABLE_UI
#include <Adafruit_ST7735.h>
#include "Adafruit_Joystick.h"
#include "UserInterface.h"
#include "Visualize.h"
#endif

#include "Axis_DCEncoder.h"
#include "Axis_AF1Stepper.h"
#include "Axis_AF2Stepper.h"
#include "Axis_A4988.h"

#include "InkBar.h"
#include "ToolFuser.h"

#if ENABLE_AXIS_X
X_MOTOR(axisX);
#else
Axis axisX;
#endif

#if ENABLE_AXIS_Y
Y_MOTOR(axisY);
#else
Axis axisY;
Tool toolInk_Black;
#endif

#if ENABLE_AXIS_Z
Z_MOTOR(axisZ);
#else
Axis axisZ;
#endif

#if ENABLE_AXIS_E
E_MOTOR(axisE);
#else
Axis axisE;
#endif

#if ENABLE_TOOL_FUSER
FUSER(toolFuser);
#else
Tool toolFuser;
#endif

Tool toolRecoat;

ToolHead tools;
CNC cnc = CNC(&axisX, &axisY, &axisZ, &axisE, &tools);

#if ENABLE_UI
Adafruit_Joystick joy = Adafruit_Joystick(JOY_PIN);

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

Visualize vis = Visualize(&tft, ST7735_TFTWIDTH, ST7735_TFTHEIGHT_18 - 5*8,
                                0, 5 * 8);

UserInterface ui = UserInterface(&cnc, &tft, ST7735_TFTWIDTH, 5*8, 0, 0);

GCode gcode = GCode(&Serial, &cnc, &vis);
#else
GCode gcode = GCode(&Serial, &cnc);
#endif

void setup()
{
    Serial.begin(SERIAL_SPEED);
#if ENABLE_SD
    SD.begin(SD_CS);
#endif

#if ENABLE_AXIS_Y
    tools.attach(TOOL_INK_BLACK, (Tool *)&axisY);
#else
    tools.attach(TOOL_INK_BLACK, &toolInk_Black);
#endif
    tools.attach(TOOL_FUSER, &toolFuser);
    tools.attach(TOOL_RECOAT, &toolRecoat);
    toolFuser.begin();
    tools.begin();

#if ENABLE_UI
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
    vis.origin(198, 0, 0);
#endif

    axisX.begin();
    axisY.begin();
    axisZ.begin();
    axisE.begin();

    gcode.begin();

#if ENABLE_SD
    cnc.begin("start.gco");
#else
    cnc.begin();
#endif
}

#if ENABLE_UI
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
#endif

void loop()
{
    unsigned long us_now = micros();
#if ENABLE_UI
    enum ui_key key;
    bool cnc_active, ui_active;

    cnc_active = cnc.update(us_now);

    if (!cnc_active) {
        key = keymap(joy.read());

        ui_active = ui.update(key);
    } else {
        ui_active = false;
    }

    if (!ui_active)
        gcode.update(cnc_active);
#else
    gcode.update(cnc.update(us_now));
#endif
}

/* vim: set shiftwidth=4 expandtab:  */

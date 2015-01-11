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

Adafruit_MotorShield AFMS;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#ifndef AXIS_NULL
#define AXIS_NULL
#endif

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

GCode gcode = GCode(&Serial,
                    &axisX, &axisY, &axisZ, &axisE,
                    &tools, &vis);

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
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
    tft.setCursor((ST7735_TFTWIDTH - 12*6)/2, 0);
    tft.print(" BrundleFab ");
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextWrap(true);

    vis.color_set(VC_AXIS + AXIS_X, ST7735_RED);
    vis.color_set(VC_AXIS + AXIS_Y, ST7735_GREEN);
    vis.color_set(VC_AXIS + AXIS_Z, ST7735_BLUE);
    vis.color_set(VC_MOVE, ST7735_CYAN);
    vis.color_set(VC_FEED, ST7735_YELLOW);
    vis.color_set(VC_TOOL, ST7735_WHITE);
    vis.begin();

    axisX.begin();
    axisY.begin();
    axisZ.begin();
    axisE.begin();

    gcode.begin();
}

static unsigned long next_update = millis();

void loop()
{
    gcode.update();

    if (next_update < millis()) {
        next_update = millis() + 1000;
        float pos[AXIS_MAX];
        int tool = tools.selected();

        for (int i = 0; i < AXIS_MAX; i++) {
            Axis *axis = gcode.axis(i);
            pos[i] = axis->position_get_mm();
            tft.setCursor((1 + 1 + 1 + 3 + 1 + 2 + 1) * 6 * (i & 1), 8 * (3 + (i>>1)));
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.print("XYZE"[i]); tft.print(": ");
            tft.setTextColor(axis->motor_active() ? ST7735_RED : ST7735_WHITE, ST7735_BLACK);
            tft.print((pos[i] < -99.99) ? -99 : (pos[i] > 999.99 ? 999 : ((int)pos[i])));
            tft.print(".");
            tft.print((pos[i] < -99.99) ? 99 : (pos[i] > 999.99 ? 99 : ((int)(pos[i]*100) % 100)));
            tft.print(" ");
        }
        tft.setCursor(2, 8 * 5 + 2);
        tft.setTextColor(tool ? ST7735_RED : ST7735_WHITE, ST7735_BLACK);
        tft.print("T: ");tft.print(tool);
    }
}

/* vim: set shiftwidth=4 expandtab:  */

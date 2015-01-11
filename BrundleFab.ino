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

//#include "MenuGFX.h"

Adafruit_MotorShield AFMS;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

GCode gcode;

ToolHead tools;
ToolInk toolInk_Black;
ToolFuser toolFuser = ToolFuser(FUSER_ENABLE);
Axis_X axisX;
Axis_Y axisY;
Axis_Z axisZ;
Axis_E axisE;

Axis *axis[AXIS_MAX];

void setup()
{
    Serial.begin(115200);
    AFMS.begin(1000);
    SD.begin(SD_CS);

    tools.attach(TOOL_INK_BLACK, &toolInk_Black);
    tools.attach(TOOL_FUSER, &toolFuser);
    tools.begin();

    tft.initR(TFT_INITR);
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextWrap(true);
    tft.print("BrundleFab");

    axis[AXIS_X] = &axisX;
    axis[AXIS_Y] = &axisY;
    axis[AXIS_Z] = &axisZ;
    axis[AXIS_E] = &axisE;

    for (int i = 0; i < AXIS_MAX; i++) {
        axis[i]->begin();
    }

    gcode.begin(&Serial,
                axis[AXIS_X],
                axis[AXIS_Y],
                axis[AXIS_Z],
                axis[AXIS_E],
                &tools);
}

static unsigned long next_update = millis();
static float position[AXIS_MAX];

void loop()
{
    bool motion = false;

    gcode.update();

    for (int i = 0; i < AXIS_MAX; i++) {
        motion |= axis[i]->update();
    }

    if (motion) {
        tools.update();
        return;
    }

    if (next_update < millis()) {
        next_update = millis() + 250;
        float pos[AXIS_MAX];
        int height = tft.height();
        int tool = tools.selected();

        for (int i = 0; i < AXIS_MAX; i++) {
            pos[i] = axis[i]->position_get_mm();
            tft.setCursor((1 + 1 + 1 + 3 + 1 + 2 + 1) * 6 * (i & 1), 8 * (3 + (i>>1)));
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.print("XYZE"[i]); tft.print(": ");
            tft.setTextColor(axis[i]->motor_active() ? ST7735_RED : ST7735_WHITE, ST7735_BLACK);
            tft.print((pos[i] < -99.99) ? -99 : (pos[i] > 999.99 ? 999 : ((int)pos[i])));
            tft.print(".");
            tft.print((pos[i] < -99.99) ? 99 : (pos[i] > 999.99 ? 99 : ((int)(pos[i]*100) % 100)));
            tft.print(" ");
        }
        tft.println();
        tft.setTextColor(tool ? ST7735_RED : ST7735_WHITE, ST7735_BLACK);
        tft.print("T: ");tft.print(tool);

        if (pos[AXIS_X] != position[AXIS_X]) {
            tft.drawPixel(position[AXIS_X], height-1, ST7735_YELLOW);
            tft.drawPixel(pos[AXIS_X], height-1, ST7735_RED);
            position[AXIS_X] = pos[AXIS_X];
        }

        if (pos[AXIS_Y] != position[AXIS_Y]) {
            tft.drawPixel(position[AXIS_Y]/4, height - (position[AXIS_Y]/4 + 1), ST7735_YELLOW);
            tft.drawPixel(pos[AXIS_Y]/4, height - (pos[AXIS_Y]/4 + 1), ST7735_GREEN);
            position[AXIS_Y] = pos[AXIS_Y];
        }

        if (pos[AXIS_Z] != position[AXIS_Z]) {
            tft.drawPixel(0, height - (position[AXIS_Z] + 1), ST7735_YELLOW);
            tft.drawPixel(0, height - (pos[AXIS_Z] + 1), ST7735_BLUE);
            position[AXIS_Z] = pos[AXIS_Z];
        }

        if (tools.active()) {
            uint16_t color = ST7735_WHITE - ((1 << 11) | (2 << 5) | 1) * (int)pos[AXIS_Y]/2;
            tft.drawPixel(pos[AXIS_X] + pos[AXIS_Y]/4, height - (pos[AXIS_Z] + pos[AXIS_Y]/4 + 1), color);
        }
    }
}

/* vim: set shiftwidth=4 expandtab:  */

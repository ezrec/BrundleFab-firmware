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
#include <Adafruit_ST7735.h>
#include <SD.h>

#include "GCode.h"

#include "Axis.h"
#include "Axis_X.h"
#include "Axis_Y.h"
#include "Axis_Z.h"
#include "Axis_E.h"

#include "ToolHead.h"
#include "ToolInk.h"

INKSHIELD_CLASS InkShield_Black(INKSHIELD_PULSE);

Adafruit_MotorShield AFMS;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

GCode gcode;

ToolHead tools;
ToolInk toolInk_Black;
Tool toolHeater;
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

    tools.attach(1, &toolInk_Black);
    tools.attach(20, &toolHeater);
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

void loop()
{
    bool motion = false;

    gcode.update();

    for (int i = 0; i < AXIS_MAX; i++) {
        motion |= axis[i]->update();
    }

    if (motion)
        tools.update();
}

/* vim: set shiftwidth=4 expandtab:  */

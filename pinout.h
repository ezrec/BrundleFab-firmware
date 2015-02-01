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

#ifndef PINOUT_H
#define PINOUT_H

#define TOOL_INK_BLACK          1
#define TOOL_FUSER              20

#define TFT_DC                  45      /* D45 */
#define TFT_RST                 47      /* D47 */
#define TFT_CS                  49      /* D49 */

#define TFT_INITR               INITR_BLACKTAB

#define JOY_PIN                 4       /* A4 */

#define SD_CS                   53      /* D53 aka SS_PIN */

#define X_MOTOR                 3       /* AFMotor M3 */
#define XENC_A                  18      /* Encoder A */
#define XENC_B                  19      /* Encoder B */
#define XSTP_MIN                35      /* Endstop (minimum) */
#define XSTP_MAX                33      /* Encstop (maximum) */

#define Z_MOTOR                 2       /* Adafruit_MotorShield M2 */
#define ZSTP_MIN                -1      /* Endstop (Minimim) */
#define ZSTP_MAX                39      /* Endstop (Maximim) */

#define E_MOTOR                 1       /* Adafruit_MotorShield M1 */
#define ESTP_MIN                37      /* Endstop (Minimum) */
#define ESTP_MAX                -1      /* Endstop (Maximim) */

#define FUSER_ENABLE            43      /* Heater enable */

#endif /* PINOUT_H */
/* vim: set shiftwidth=4 expandtab:  */

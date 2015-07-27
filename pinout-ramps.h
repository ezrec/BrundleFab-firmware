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

#ifndef PINOUT_RAMPS_H
#define PINOUT_RAMPS_H

#define TFT_CS                  49      /* D49 */
#define TFT_DC                  32      /* D32 */
#define TFT_RST                 47      /* D47 */

#define TFT_INITR               INITR_BLACKTAB

#define JOY_PIN                 A12     /* A12 */

#define SD_CS                   53      /* D53 aka SS_PIN */

#define X_STEP                  54      /* A0 */
#define X_DIR                   55      /* A1 */
#define X_ENABLE                38      /* D38 */
#define X_STP_MIN               3       /* Endstop (minimum) */
#define X_STP_MAX               2       /* Endstop (maximum) */
#define X_TURN_STEPS            200
#define X_TURN_MM               4.0
#define X_MICROSTEP             16
#define X_MOTOR(name)           Axis_A4988 name(X_ENABLE, X_STEP, X_DIR, \
                                           X_STP_MIN, X_STP_MAX, \
					   X_FEED_MAX, \
                                           X_MM_MAX, X_MICROSTEP, \
                                           X_TURN_STEPS, X_TURN_MM)

/* Y is driven by the inkbar - 9", 96 DPI */
#define Y_DPMM                  (96.0 / 25.4)
#define Y_MOTOR(name)           InkBar name(&Serial2,  0, Y_MM_MAX, Y_DPMM)

#define Z_STEP                  46
#define Z_DIR                   48
#define Z_ENABLE                62
#define Z_STP_MIN               -1      /* Endstop (Minimim) */
#define Z_STP_MAX               19      /* Endstop (Maximim) */
#define Z_TURN_STEPS            200
#define Z_TURN_MM               4.0
#define Z_MICROSTEP             16
#define Z_MOTOR(name)           Axis_A4988 name(Z_ENABLE, Z_STEP, Z_DIR, \
                                           Z_STP_MIN, Z_STP_MAX, \
					   Z_FEED_MAX, \
                                           Z_MM_MAX, Z_MICROSTEP, \
                                           Z_TURN_STEPS, Z_TURN_MM)

/* We use the RAMPS Y axis for E */
#define E_STEP                  60 
#define E_DIR                   61 
#define E_ENABLE                56
#define E_STP_MIN               14      /* Endstop (Minimum) */
#define E_STP_MAX               -1      /* Endstop (Maximim) */
#define E_TURN_STEPS            200
#define E_TURN_MM               4.0
#define E_MICROSTEP             16
#define E_MOTOR(name)           Axis_A4988 name(E_ENABLE, E_STEP, E_DIR, \
                                           E_STP_MIN, E_STP_MAX, \
					   E_FEED_MAX, \
                                           E_MM_MAX, E_MICROSTEP, \
                                           E_TURN_STEPS, E_TURN_MM)

#define FUSER_ENABLE            8       /* Heater enable */

#endif /* PINOUT_RAMPS_H */
/* vim: set shiftwidth=4 expandtab:  */

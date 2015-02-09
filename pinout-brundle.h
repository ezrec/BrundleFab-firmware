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

#ifndef PINOUT_BRUNDLE_H
#define PINOUT_BRUNDLE_H

#define TOOL_INK_BLACK          1
#define TOOL_FUSER              20

#define TFT_DC                  45      /* D45 */
#define TFT_RST                 47      /* D47 */
#define TFT_CS                  49      /* D49 */

#define TFT_INITR               INITR_BLACKTAB

#define JOY_PIN                 4       /* A4 */

#define SD_CS                   53      /* D53 aka SS_PIN */

#define X_STP_MIN               35      /* Endstop (minimum) */
#define X_STP_MAX               33      /* Endstop (maximum) */
#define X_TURN_STEPS            200     /* Steps/full rotation */
#define X_TURN_MM               76.0    /* mm/full rotation */
#define X_MOTOR(name)           Axis_AF1Stepper name(2,  X_STP_MIN, X_STP_MAX, \
                                               X_MM_MAX, \
                                               X_TURN_STEPS, X_TURN_MM)

/* Y is driven by the inkbar  - 9", 96 DPI */
#define Y_MOTOR(name)           InkBar name(&Serial3,  0, 25.4 * 9.0, 96.0 / 25.4)

#define Z_STP_MIN               -1      /* Endstop (Minimim) */
#define Z_STP_MAX               39      /* Endstop (Maximim) */
#define Z_TURN_STEPS            200     /* Steps/full rotation */
#define Z_TURN_MM               4.0     /* mm/full rotataton */
#define Z_MOTOR(name)           Axis_AF2Stepper name(2, Z_STP_MIN, Z_STP_MAX, \
                                                Z_MM_MAX, \
                                                Z_TURN_STEPS, Z_TURN_MM)

#define E_STP_MIN                37      /* Endstop (Minimum) */
#define E_STP_MAX                -1      /* Endstop (Maximim) */
#define E_TURN_STEPS            200     /* Steps/full rotation */
#define E_TURN_MM               4.0     /* mm/full rotataton */
#define E_MOTOR(name)           Axis_AF2Stepper name(1, E_STP_MIN, E_STP_MAX, \
                                                E_MM_MAX, \
                                                E_TURN_STEPS, E_TURN_MM)

#define FUSER_ENABLE            43      /* Heater enable */

#endif /* PINOUT_BRUNDLE_H */
/* vim: set shiftwidth=4 expandtab:  */

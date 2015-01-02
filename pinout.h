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

#define INKSHIELD_PULSE         2       /* D2 */
#define INKSHIELD_CLASS         InkShieldA0A3

#define TFT_RST                 45      /* D45 */
#define TFT_RS                  47      /* D47 */
#define TFT_CS                  49      /* D49 */

#define TFT_INITR               INITR_GREENTAB

#define SD_CS                   53      /* D53 aka SS_PIN */

#define XENC_A                  19      /* D19 */
#define XENC_B                  15      /* D15 */
#define XSTP_MAX                22      /* D22 */

#define YENC_A                  18      /* D19 */
#define YENC_B                  14      /* D15 */

#define ESTP_MIN                16      /* D16 */
#define ZSTP_MAX                17      /* D16 */

#endif /* PINOUT_H */
/* vim: set shiftwidth=4 expandtab:  */

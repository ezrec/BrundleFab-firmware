/*
 * Copyright (C) 2015, Netronome Systems, Inc.
 * All right reserved.
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#define ENABLE_CNC              1
#define ENABLE_UI               1
#define ENABLE_SD               1

#define SERIAL_SPEED            115200
//#define SERIAL_SPEED            1000000 /* 1Mbit */

#define X_MM_MAX                650.0
#define Y_MM_MAX                229.0
#define Z_MM_MAX                200.0
#define E_MM_MAX                225.0

#define X_FEED_MAX              2000    /* mm/minute */
#define Y_FEED_MAX              20000   /* mm/minute */
#define Z_FEED_MAX              2000    /* mm/minute */
#define E_FEED_MAX              2000    /* mm/minute */

#define ARRAY_SIZE(x)           (sizeof(x)/sizeof((x)[0]))

#endif /* CONFIG_H */
/* vim: set shiftwidth=4 expandtab:  */

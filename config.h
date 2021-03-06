/*
 * Copyright (C) 2015, Netronome Systems, Inc.
 * All right reserved.
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#define ENABLE_UI               1
#define ENABLE_SD               1
#define ENABLE_AXIS_X           1
#define ENABLE_AXIS_Y           1
#define ENABLE_AXIS_Z           1
#define ENABLE_AXIS_E           1
#define ENABLE_TOOL_FUSER       1

#define SERIAL_SPEED            115200

#define X_MM_MAX                650.0
#define Y_MM_MAX                229.0
#define Z_MM_MAX                140.0
#define E_MM_MAX                140.0

#define X_FEED_MAX              2000    /* mm/minute */
#define Y_FEED_MAX              20000   /* mm/minute */
#define Z_FEED_MAX              2000    /* mm/minute */
#define E_FEED_MAX              2000    /* mm/minute */

#define ARRAY_SIZE(x)           (sizeof(x)/sizeof((x)[0]))

#endif /* CONFIG_H */
/* vim: set shiftwidth=4 expandtab:  */

/*
 * Copyright (C) 2015, Netronome Systems, Inc.
 * All right reserved.
 *
 */

#include "Axis_AF2Stepper.h"

Adafruit_MotorShield AFMS;

void Axis_AF2Stepper::begin(void)
{
    static bool AFMS_init = false;

    if (!AFMS_init) {
        AFMS.begin(1000);
        AFMS_init = true;
    }

    Axis_Stepper::begin();
}

/* vim: set shiftwidth=4 expandtab:  */

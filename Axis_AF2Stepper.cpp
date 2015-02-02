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

    _motor->setSpeed(100);       /* 100 RPM */
    if (_pinStopMin >= 0)
        pinMode(_pinStopMin, INPUT_PULLUP);
    if (_pinStopMax >= 0)
        pinMode(_pinStopMax, INPUT_PULLUP);
    Axis::begin();
}

/* vim: set shiftwidth=4 expandtab:  */

/*
 * Copyright (C) 2014, Jason S. McMullan
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

#ifndef AXIS_AFSTEPPER_H
#define AXIS_AFSTEPPER_H

#include <Wire.h>
#include <Adafruit_MotorShield.h>

#include "Axis_Stepper.h"

extern Adafruit_MotorShield AFMS;

class Axis_AF2Stepper : public Axis_Stepper {
    private:
        Adafruit_StepperMotor *_motor;

    public:
        Axis_AF2Stepper(int af_motor, int pinStopMin, int pinStopMax,
                        float maxPosMM,
                        unsigned int stepsPerRotation, 
                        float mmPerRotation)
            : Axis_Stepper(pinStopMin, pinStopMax, maxPosMM,
                           MICROSTEPS, stepsPerRotation, mmPerRotation)
        {
            _motor = AFMS.getStepper(stepsPerRotation, af_motor);
        }

        virtual void begin(void);

        virtual void motor_enable(bool enabled = true)
        {
            if (!enabled)
                _motor->release();
            Axis_Stepper::motor_enable(enabled);
        }

        virtual int step(int32_t steps)
        {
            uint8_t dir;
            int neg = 1;

            if (!steps)
                return 0;

            if (steps < 0) {
                dir = BACKWARD;
                steps = -steps;
                neg = -1;
            } else {
                dir = FORWARD;
            }

            if (steps > MICROSTEPS * 2) {
                _motor->onestep(dir, DOUBLE);
                return neg * (MICROSTEPS * 2);
            }

            if (steps > MICROSTEPS) {
                _motor->onestep(dir, SINGLE);
                return neg * (MICROSTEPS * 2);
            }

            _motor->onestep(dir, MICROSTEP);
            return neg;
        }
};


#endif /* AXIS_AF2STEPPER_H */
/* vim: set shiftwidth=4 expandtab:  */

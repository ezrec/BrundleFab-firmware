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

#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>
#include <math.h>

#define AXIS_MAX        4
#define AXIS_X          0
#define AXIS_Y          1
#define AXIS_Z          2
#define AXIS_E          3

class Axis {
    private:
        bool _enabled, _updated;
        int _pinStopMin;
        int _pinStopMax;

        unsigned int _velocityMax;
    protected:
        struct {
            float mm;
            unsigned long ms;
            unsigned int velocity;      /* mm/minute */
        } _target;

    public:
        enum axis_stop_e {
            STOP_NONE = 0,
            STOP_MIN,
            STOP_MIN_SWITCH,    /* Ignores soft trip */
            STOP_MAX,
            STOP_MAX_SWITCH,    /* Ignores soft trip */
        };

        /* NOTE: This class assumes that all endstops are
         *       normally closed switches to ground.
         */
        Axis(int pinStopMin = -1, int pinStopMax = -1, unsigned int mm_per_minute_max = 0)
        {
            _velocityMax = mm_per_minute_max ? mm_per_minute_max : 2000;
            _pinStopMin = pinStopMin;
            _pinStopMax = pinStopMax;
        }

        virtual void begin()
        {
            if (_pinStopMin >= 0)
                pinMode(_pinStopMin, INPUT_PULLUP);
            if (_pinStopMax >= 0)
                pinMode(_pinStopMax, INPUT_PULLUP);
            motor_enable(false);
        }

        bool endstop(enum axis_stop_e select, bool *is_physical = NULL)
        {
            int pin = -1;
            bool tripped = false;

            switch (select) {
            case STOP_NONE:
                break;
            case STOP_MIN:
                tripped = position_get() <= position_min();
                /* FALLTHROUGH */
            case STOP_MIN_SWITCH:
                if (_pinStopMin >= 0)
                    pin = _pinStopMin;
                break;
            case STOP_MAX:
                tripped = position_get() >= position_max();
                /* FALLTHROUGH */
            case STOP_MAX_SWITCH:
                if (_pinStopMax >= 0)
                    pin = _pinStopMax;
                break;
            }

            if (is_physical)
                *is_physical = (pin < 0) ? false : true;

            if (tripped)
                return true;

            /* If we have a pin, if we don't read a zero,
             * then the switch is triggered, or the wire is pulled.
             */
            if (pin >= 0)
                return digitalRead(pin) == 1;

            return false;
        }

        virtual void home(float mm = 0.0)
        {
            target_set(mm);
        }

        virtual bool update(unsigned long ms_now)
        {
            if (!_updated) {
                _updated = true;
                return true;
            } else {
                return false;
            }
        }

        virtual void motor_enable(bool enabled = true)
        {
            if (enabled)
                _enabled = true;
            else {
               _enabled = false;
            }
        }

        virtual bool motor_enabled() { return _enabled; }
        virtual bool motor_active() { return false; }

        virtual void target_set(float mm, unsigned long ms = 0)
        {
            unsigned long mm_per_minute;
            float mm_distance = fabs(mm - _target.mm);

            if (ms > 0) {
                mm_per_minute = mm_distance * 60000 / ms;
                if (mm_per_minute < 1)
                    mm_per_minute = _velocityMax;
                else if (mm_per_minute  > _velocityMax)
                    mm_per_minute = _velocityMax;
            } else {
                mm_per_minute = _velocityMax;
            }

            _target.velocity = mm_per_minute;
            _target.ms = millis() + mm_distance * mm_per_minute / 60 / 1000;
            _target.mm = mm;
            _updated = false;
        }

        virtual float target_get(unsigned long *ms_left = NULL)
        {
            unsigned long now = millis();

            if (ms_left)
                *ms_left = (now <= _target.ms) ? (_target.ms - now) : 0;

            return _target.mm;
        }

        virtual float position_min(void) { return 0.0; }
        virtual float position_max(void) { return 200.0; }

        virtual float position_get() { return _target.mm; }
};

#endif /* AXIS_H */
/* vim: set shiftwidth=4 expandtab:  */

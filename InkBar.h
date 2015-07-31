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

#ifndef INKBAR_H
#define INKBAR_H

/* It's a Tool! It's an Axis! It's a sealing wax! */
#include "Tool.h"
#include "Axis.h"

/**
 * Printhead
 */
#include "BrundleInk.h"

class InkBar : public Tool, public Axis {
    private:
        static const int DEBUG = 0;
	static const int TIMEOUT_MS = 100;
        BrundleInk _ink;
        float _mm_min, _mm_max;
        float _dotlines_per_mm;
        int32_t _dotline;
        uint16_t _pattern, _sprays;
        unsigned long _next_status, _next_motor;

	enum inkbar_state {
	    STATE_IDLE = 0,
	    STATE_HOME,
	    STATE_INK_FORWARD,
	    STATE_INK_REVERSE,
	    STATE_INK_CLEAR,
	} _state;

    public:
        InkBar(HardwareSerial *io, float mm_min, float mm_max, float dotlines_per_mm) :
            _ink(io)
        {
            _mm_min = mm_min;
            _mm_max = mm_max;
            _dotlines_per_mm = dotlines_per_mm;
	    _state = STATE_IDLE;
	    _sprays = 4;
        }

        void begin()
        {
            _ink.begin();
            _next_status = millis() + 100;
            _ink.cmd('s', _sprays-1);
        }

        /* Tool specific functions */

        void parm(enum parm_e p, float val)
        {
            switch (p) {
            case Tool::PARM_P:
if (DEBUG) {
    Serial.print("parm: Ink ");Serial.println((uint16_t)val, BIN);
}
                _pattern = (uint16_t)val;
                break;
            case Tool::PARM_S:
                _sprays = (val < 2) ? 2 : val;
if (DEBUG) {
    Serial.print("parm: Spray ");Serial.println(_sprays);
}
                _ink.cmd('s', _sprays-1);
                break;
            default:
                break;
            }
        }

        virtual bool update()
        {
            enum inkbar_state in_state = _state;
            bool motor_timeout = millis() > _next_motor;

            if (_ink.busy()) {
                if (!_ink.recv())
                    return true;
                _next_status = millis() + 100;
            }

            switch (_state) {
            case STATE_IDLE:
                    break;
            case STATE_HOME:
                    if (motor_timeout || !_ink.motor_on()) {
                        _state = STATE_IDLE;
                        _dotline = 0;
                    }
                    break;
            case STATE_INK_FORWARD:
                    if (motor_timeout || !_ink.motor_on()) {
                        _state = STATE_INK_REVERSE;
                        _ink.send('j');
                        _next_motor = millis() + (_sprays + 1) * 1000;
                    }
                    break;
            case STATE_INK_REVERSE:
                    if (motor_timeout || !_ink.motor_on()) {
                        _state = STATE_INK_CLEAR;
                        _ink.send('k');
                    }
                    break;
            case STATE_INK_CLEAR:
                    _dotline = 0;
                    _state = STATE_IDLE;
                    break;
            }

if (DEBUG && in_state != _state) {
    Serial.print("MODE: ");Serial.print(in_state);
    Serial.print(" => ");Serial.println(_state);
}

            if (!_ink.busy() && (millis() > _next_status)) {
                _ink.send('?');
            }

            return motor_active();
        }

        /* Axis commands */
        virtual bool motor_active()
        {
            return (_state != STATE_IDLE);
        }

        virtual void home(float mm )
        {
if (DEBUG) {
	Serial.print("home: ");
	Serial.println(mm);
}
            while (_ink.busy())
                _ink.recv();

            _ink.send('h');
	    _state = STATE_HOME;

            Axis::home(mm);
        }

        virtual void target_set(float mm, unsigned long ms)
        {
            int32_t pos = mm * _dotlines_per_mm;

            /* Moving backwards? Ink the bar... */
            if (pos < _dotline) {
if (DEBUG) Serial.println("target_set: Inking");
                while (_ink.busy())
                    _ink.recv();
                _ink.send('i');
                _state = STATE_INK_FORWARD;
                _next_motor = millis() + (_sprays + 1) * 1000;
if (DEBUG) Serial.println("MODE: 0 => 2");
            } else if (_dotline != pos) {
if (DEBUG) Serial.print("target_set: Repeat ");
if (DEBUG) Serial.println(pos - _dotline);
                while (_ink.busy())
                    _ink.recv();
                _ink.cmd('l',_pattern);
                if ((pos - _dotline) > 1)
                    _ink.cmd('r', (pos - _dotline) - 1);
                _dotline = pos;
            }

            Axis::target_set(mm, ms);
        }
};

#endif /* INKBAR_H */
/* vim: set shiftwidth=4 expandtab:  */

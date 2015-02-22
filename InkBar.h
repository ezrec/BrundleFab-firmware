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
#define STATUS_MOTOR_ON         (1 << 0)
#define STATUS_MOTOR_STALL      (1 << 1)
#define STATUS_MOTOR_MIN        (1 << 2)
#define STATUS_MOTOR_MAX        (1 << 3)
#define STATUS_HEATER_ON        (1 << 4)
#define STATUS_HEATER_STABLE    (1 << 5)
#define STATUS_INK_ON           (1 << 6)
#define STATUS_INK_EMPTY        (1 << 7)

class InkBar : public Tool, public Axis {
    private:
        static const int DEBUG = 0;
        HardwareSerial *_io;
        float _mm_min, _mm_max;
        float _dotlines_per_mm;
        int32_t _dotline;
        bool _inking;
        uint16_t _pattern;

        struct {
            uint8_t state;
            uint8_t sprays;
            uint16_t space;
            uint8_t temp;
        } _status;

        struct {
            bool waiting;
            char buff[16];
            int pos;
        } _response;

    public:
        InkBar(HardwareSerial *io, float mm_min, float mm_max, float dotlines_per_mm)
        {
            _io = io;
            _mm_min = mm_min;
            _mm_max = mm_max;
            _dotlines_per_mm = dotlines_per_mm;
        }


        void begin()
        {
            bool done = false;

            _io->begin(115200);

            /* Attempt to communicate with the device */
            do {
                unsigned long timeout = millis() + 2000;
                _response.waiting = false;
                _send('?');
                while (millis() < timeout) {
                    if (_recv()) {
                        done = true;
                        break;
                    }
                }
            } while (!done);

            _dotline = 0;
        }

        /* Tool specific functions */

        void parm(enum parm_e p, float val)
        {
            uint16_t sprays;

            switch (p) {
            case Tool::PARM_P:
if (DEBUG) {
    Serial.print("parm: Ink ");Serial.println((uint16_t)val, BIN);
}
                _pattern = (uint16_t)val;
                break;
            case Tool::PARM_S:
                /* Convert from dots/mm to sprays/dotline */
                sprays = val / _dotlines_per_mm;
if (DEBUG) {
    Serial.print("parm: Spray ");Serial.println(sprays);
}
                _send('s', sprays);
                break;
            default:
                break;
            }
        }

        virtual bool update()
        {
            if (_recv()) {
if (DEBUG) if (_status.state != 4) { Serial.print("status: 0x");Serial.println(_status.state, HEX); }
                if (_status.state & STATUS_INK_ON) {
                    _inking = true;
                }
                if (_inking & !(_status.state & STATUS_INK_ON)) {
                    /* When we're done inking, home the bar */
                    _inking = false;
if (DEBUG) Serial.println("target_set: Homing");
                    _send('h');
                    _dotline = 0;
                }
            }

            if (!_response.waiting)
                _send('?');

            return motor_active();
        }

        /* Axis commands */
        virtual bool motor_active()
        {
            return (_status.state & STATUS_MOTOR_ON) ? true : false;
        }

        virtual void home(int32_t pos)
        {
            _send('h');
            _dotline = 0;

            Axis::home(pos);
        }

        virtual void target_set(float mm, unsigned long ms)
        {
            int32_t pos = mm * _dotlines_per_mm;

            /* Moving backwards? Ink the bar... */
            if (pos < _dotline) {
if (DEBUG) Serial.println("target_set: Inking");
                _send('i');
                _status.state |= 1;
                _inking = true;
            } else if (_dotline != pos) {
if (DEBUG) Serial.print("target_set: Repeat ");
if (DEBUG) Serial.println(pos - _dotline);
                _send('l',_pattern);
                if ((pos - _dotline) > 1)
                    _send('r', (pos - _dotline) - 1);
                _dotline = pos;
            }

            Axis::target_set(mm, ms);
        }

    private:
        bool _send(char cmd, uint16_t val = 0)
        {
            _recv_flush();

if (DEBUG) if (cmd != '?') {
    Serial.print("TX: ");
    Serial.print(cmd);
    Serial.println(val, HEX);
}
            _io->print(cmd);
            _io->println(val, HEX);
            _response.waiting = true;
            return true;
        }

        void _recv_flush()
        {
            if (!_response.waiting)
                return;

            while (!_recv());
        }

        bool _recv()
        {
            if (!_response.waiting)
                return false;

            if (!_io->available())
                return false;

            char c = _io->read();
if (DEBUG) {
    if (c >= ' ') Serial.print(c);
    if (c == '\r') { Serial.println(); }
    if (c == '\n') { Serial.print("RX: "); }
}
            if (c == '\n')
                return false;

            if (c == '\r') {
                unsigned s, i, n, t;
                _response.buff[_response.pos] = 0;
                int rc;

                rc = sscanf(_response.buff, "ok %x %x %x %x", &s, &i, &n, &t);
                _response.pos = 0;
if (DEBUG || n < 10) {
    Serial.print("rc: ");Serial.print(rc);
    Serial.print(" s: 0x");Serial.print(s, HEX);
    Serial.print(" i: ");Serial.print(i, DEC);
    Serial.print(" n: ");Serial.print(n, DEC);
    Serial.print(" t: ");Serial.print(t*2, DEC);
    Serial.println("C");
}
                if (rc != 4) {
                    return false;
                }
                _status.state = s;
                _status.sprays = i;
                _status.space = n;
                _status.temp = t;
                _response.waiting = false;

                return true;
            }

            if (_response.pos < (int)(ARRAY_SIZE(_response.buff)-1))
                _response.buff[_response.pos++] = c;

            return false;
        }

};

#endif /* INKBAR_H */
/* vim: set shiftwidth=4 expandtab:  */

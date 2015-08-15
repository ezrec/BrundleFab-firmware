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

#ifndef BRUNDLEINK_H
#define BRUNDLEINK_H

/**
 * BrundleInk interface
 */
#define STATUS_MOTOR_ON         (1 << 0)
#define STATUS_MOTOR_STALL      (1 << 1)
#define STATUS_MOTOR_MIN        (1 << 2)
#define STATUS_MOTOR_MAX        (1 << 3)
#define STATUS_HEATER_ON        (1 << 4)
#define STATUS_HEATER_HOT       (1 << 5)
#define STATUS_INK_ON           (1 << 6)
#define STATUS_INK_EMPTY        (1 << 7)

class BrundleInk {
    private:
        static const int DEBUG = 0;
        static const int TIMEOUT_MS = 2000;
        HardwareSerial *_io;
        uint16_t _line_no;

        struct {
            uint8_t state;
            uint8_t sprays;
            uint16_t space;
            uint16_t line;
            uint16_t position;
        } _status;

        struct {
            char cmd;
            uint16_t val;
            unsigned long resend;
            bool waiting;
            char buff[32];
            int  pos;
        } _response;

    public:
        BrundleInk(HardwareSerial *io)
        {
            _io = io;
        }

        void begin()
        {
            uint16_t line_no = millis() & 0xfff;

            _io->begin(115200);

if (DEBUG) {
    Serial.print(">> SYNC?\n");
}
            /* Attempt to communicate with the device */
            do {
                unsigned long timeout = millis() + 100;

                line_no = (line_no + 1) & 0xfff;
                send('n',line_no);
                while (!recv() && (millis() < timeout));
            } while (_status.line != line_no);

            _response.waiting = false;
if (DEBUG) {
    Serial.print(">> SYNC!\n");
}
        }

        // Status reporting
        bool motor_on()
        {
            return _status.state & STATUS_MOTOR_ON;
        }

        // Command protocol
        bool send(char cmd, uint16_t val = 0)
        {
if (DEBUG) {
    Serial.print("TX: ");
    Serial.print(cmd);
    Serial.println(val, HEX);
}

if (DEBUG && motor_on() && cmd != '?') {
    Serial.println("**** OH NO! THE MOTOR IS ON! ****");
    for (;;);
}

            if (_response.waiting) {
if (DEBUG) {
    Serial.println(">> SEQUENCE ERROR: _send");
}
                return false;
            }

            if (cmd == 'n')
                _line_no = val;

            _io->print(cmd);
            _io->println(val, HEX);
            _response.waiting = true;
            _response.resend = millis() + 100;
            _response.cmd = cmd;
            _response.val = val;

            return true;
        }

        bool busy()
        {
            if (!_response.waiting)
                return false;

            return true;
        }

        bool recv()
        {
            if (!_response.waiting) {
if (DEBUG) {
    Serial.println(">> SEQUENCE ERROR: _recv");
}
                return false;
            }

            if (!_io->available()) {
                if (millis() > _response.resend) {
if (DEBUG) {
    Serial.print("RESEND: ");
}
                    _response.waiting = false;
                    send(_response.cmd, _response.val);
                }

                return false;
            }

            char c = _io->read();
if (DEBUG > 1) {
    if (c >= ' ') Serial.print(c);
    if (c == '\r') { Serial.println(); }
    if (c == '\n') { Serial.print("RX: "); }
}
            if (c == '\r')
                return false;

            if (c != '\n') {
                if (_response.pos < (int)(ARRAY_SIZE(_response.buff)-1))
                    _response.buff[_response.pos++] = c;
                return false;
            }

            unsigned s, i, n, l, p;
            _response.buff[_response.pos] = 0;
            int rc;

            if (strncmp(_response.buff, "ok ", 3) == 0) {
                _response.waiting = false;
            } else {
if (DEBUG) {
    Serial.print(">> UNEXPECTED: _recv: '");
    Serial.print((const char *)_response.buff);
    Serial.println("'");
}
            }

            rc = sscanf(_response.buff, "ok %x %x %x %x %x", &s, &i, &n, &l, &p);
            _response.pos = 0;

if (DEBUG) {
    Serial.println((const char *)_response.buff);
    Serial.print("rc:");Serial.print(rc);
    if (rc > 0) { Serial.print(" s:");Serial.print(s, HEX); }
    if (rc > 1) { Serial.print(" i:");Serial.print(i, DEC); }
    if (rc > 2) { Serial.print(" n:");Serial.print(n, DEC); }
    if (rc > 3) { Serial.print(" l:");Serial.print(l, DEC); }
    if (rc > 4) { Serial.print(" p:");Serial.print(p, DEC); }
    Serial.println();
}
            if (rc == 5 && l == _line_no) {
                _status.state = s;
                _status.sprays = i;
                _status.space = n;
                _status.line = l;
                _status.position = p;

                _line_no = (_line_no + 1) & 0xfff;
            } else {
if (DEBUG) {
    Serial.print(">> CORRUPTED: _recv: (");Serial.print(rc);Serial.print(") '");
    Serial.print((const char *)_response.buff);
    Serial.println("'");
    for(;;);
}
            }

            return true;
        }

        bool cmd(char c, uint16_t val = 0)
        {
            while (busy())
                recv();

            if (!send(c, val))
                return false;

if (DEBUG) {
    Serial.println(">> WAIITING..");
}
            while (!recv());
if (DEBUG) {
    Serial.println(">> .. DONE");
}

            return true;
        }
};

#endif /* BRUNDLEINK_H */
/* vim: set shiftwidth=4 expandtab:  */

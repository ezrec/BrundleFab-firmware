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
        static const int DEBUG = 1;

        struct {
            bool waiting;
        } _response;

    public:
        BrundleInk(HardwareSerial *io)
        {
        }

        void begin()
        {
if (DEBUG) {
    Serial.print(">> SYNC?\n");
}
if (DEBUG) {
    Serial.print(">> SYNC!\n");
}
        }

        // Status reporting
        bool motor_on()
        {
            return false;
        }

        // Command protocol
        bool send(char cmd, uint16_t val = 0)
        {
if (DEBUG) {
    Serial.print("TX: ");
    Serial.print(cmd);
    Serial.println(val, HEX);
}

            if (_response.waiting) {
if (DEBUG) {
    Serial.println(">> SEQUENCE ERROR: _send");
}
                return false;
            }

            _response.waiting = true;

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

            _response.waiting = false;

            return true;
        }

        bool cmd(char c, uint16_t val = 0)
        {
            while (busy())
                recv();

            if (!send(c, val))
                return false;

            while (!recv());

            return true;
        }
};

#endif /* BRUNDLEINK_H */
/* vim: set shiftwidth=4 expandtab:  */

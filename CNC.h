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

#ifndef CNC_H
#define CNC_H

#include "Axis.h"
#include "ToolHead.h"

#define CNC_STATUS_MAX           32
#define CNC_MESSAGE_MAX          32

#define CNC_SWITCH_OPTIONAL_STOP 0

#define CNC_BUTTON_CYCLE_START   0

class CNC {
    private:
        Axis *_axis[AXIS_MAX];
        ToolHead *_toolhead;

        uint16_t _switch_mask;
        uint16_t _button_mask;

        char _status[CNC_STATUS_MAX];
        char _message[CNC_MESSAGE_MAX];
        bool _message_updated;

        File _program;

    public:
        CNC(Axis *x, Axis *y, Axis *z, Axis *e, ToolHead *t)
        {
            _axis[AXIS_X] = x;
            _axis[AXIS_Y] = y;
            _axis[AXIS_Z] = z;
            _axis[AXIS_E] = e;
            _toolhead = t;
        }

        void begin(const char *filename = NULL)
        {
            program_set(filename);
        }

        bool program_set(const char *filename)
        {
            if (_program)
                _program.close();

            if (!filename)
                return false;

            _program = SD.open(filename);

            if (_program) {
                status_set(NULL);
                message_set(_program.name());
            }

            return _program;
        }

        bool program_set(File *program)
        {
            if (_program)
                _program.close();

            _program = *program;
            *program = File();
                   
            if (_program) {
                status_set(NULL);
                message_set(_program.name());
            }

            return _program;
        }

        File *program()
        {
            return &_program;
        }

        Axis *axis(int n)
        {
            return _axis[n];
        }

        ToolHead *toolhead()
        {
            return _toolhead;
        }

        void motor_enable(bool enabled = true)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_enable(enabled);
        }

        void motor_halt()
        {
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_halt();
        }

        void motor_disable()
        {
            motor_enable(false);
        }

        void target_get_mm(float *pos)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                pos[i] = _axis[i]->target_get_mm();
        }

        void sleep()
        {
            motor_halt();
        }

        void stop()
        {
            motor_disable();
            _toolhead->stop();
        }

        void status_set(const char *message)
        {
            if (!message || message[0] == 0) {
                _status[0] = 0;
            } else {
                strncpy(_status, message, CNC_STATUS_MAX);
            }
        }
        
        const char *status_get()
        {
            return _status[0] ? _status : NULL;
        }

        void message_set(const char *message)
        {
            if (!message || message[0] == 0) {
                _message[0] = 0;
            } else {
                strncpy(_message, message, CNC_STATUS_MAX);
            }
            _message_updated = true;
        }

        const char *message_get(bool *updated = NULL)
        {
            if (updated) {
                *updated = _message_updated;
                _message_updated = false;
            }

            return _message[0] ? _message : NULL;
        }

        bool button_get(int button)
        {
            uint16_t mask = (1 << button);
            bool pressed;

            pressed = (_button_mask & mask) ? true : false;
                
            _button_mask &= ~mask;

            return pressed;
        }

        void button_set(int button)
        {
            uint16_t mask = (1 << button);

            _button_mask |= mask;
        }

        bool switch_get(int sw)
        {
            return (_switch_mask & (1 << sw)) ? true : false;
        }

        void switch_set(int sw, bool enabled = true)
        {
            uint16_t mask = (1 << sw);
            if (enabled)
                _switch_mask |= mask;
            else
                _switch_mask &= ~mask;
        }

        bool update()
        {
            bool motion = false;

            for (int i = 0; i < AXIS_MAX; i++)
                motion |= _axis[i]->update();

            if (motion)
                _toolhead->update();

            return motion;
        }
};

#endif /* CNC_H */
/* vim: set shiftwidth=4 expandtab:  */

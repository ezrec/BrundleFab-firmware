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

#include "config.h"

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
        float _pos[AXIS_MAX];

        Stream *_serial[4];

#if ENABLE_SD
        File _program;
#endif

    public:
        CNC(Axis *x, Axis *y, Axis *z, Axis *e, ToolHead *t)
        {
            _axis[AXIS_X] = x;
            _axis[AXIS_Y] = y;
            _axis[AXIS_Z] = z;
            _axis[AXIS_E] = e;
            _toolhead = t;
        }

        void begin()
        {
        }

        void serial_set(int id, Stream *stream)
        {
            if (id < 0 || id >= (int)ARRAY_SIZE(_serial))
                return;

            _serial[id] = stream;
        }

        Stream *serial_get(int id = 0)
        {
            if (id < 0 || id >= (int)ARRAY_SIZE(_serial))
                return NULL;

            return _serial[id];
        }


        void target_move(float *pos, uint8_t axis_mask, unsigned long ms = 0)
        {
            const float *offset = tool()->offset_is();

            for (int i = 0; i < AXIS_MAX; i++) {
                if (axis_mask & (1 << i))
                    _pos[i] += pos[i];
                _axis[i]->target_set(_pos[i] - offset[i], ms);
            }
        }

        void target_set(float *pos, uint8_t axis_mask, unsigned long ms = 0)
        {
            const float *offset = tool()->offset_is();

            for (int i = 0; i < AXIS_MAX; i++) {
                if (axis_mask & (1 << i))
                    _pos[i] = pos[i];
                _axis[i]->target_set(_pos[i] - offset[i], ms);
            }
        }

        void target_move_rate(float *pos, uint8_t axis_mask, float feed_rate)
        {
            float dist = 0.0;

            for (int i = 0; i < AXIS_MAX; i++) {
                if (axis_mask & (1 << i))
                    dist += pos[i] * pos[i];
            }

            unsigned long ms = sqrt(dist) / feed_rate * 60000.0;

            target_move(pos, axis_mask, ms);
        }

        void target_set_rate(float *pos, uint8_t axis_mask, float feed_rate)
        {
            float dist = 0.0;

            for (int i = 0; i < AXIS_MAX; i++) {
                if (axis_mask & (1 << i)) {
                    float delta = pos[i] - _pos[i];
                    dist += delta * delta;
                }
            }

            unsigned long ms = sqrt(dist) / feed_rate * 60000.0;

            target_set(pos, axis_mask, ms);
        }

        void home(uint8_t axis_mask = 0xff)
        {
            for (int i = 0; i < AXIS_MAX; i++) {
                if (axis_mask & (1 << i))
                    _axis[i]->home();
            }
        }

        void target_get(float *pos)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                pos[i] = _pos[i];
        }

        void position_get(float *pos)
        {
            const float *offset = tool()->offset_is();

            for (int i = 0; i < AXIS_MAX; i++)
                pos[i] = _axis[i]->position_get() + offset[i];
        }


#if ENABLE_SD
        void begin(const char *filename)
        {
            program_set(filename);
            begin();
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
#endif

        ToolHead *toolhead()
        {
            return _toolhead;
        }

        Tool *tool(int tool_id = -1)
        {
            return _toolhead->tool(tool_id);
        }

        bool axis_active(int axis = -1)
        {
            bool active = false;

            if (axis < 0) {
                for (int i = 0; i < AXIS_MAX; i++)
                    active |= _axis[i]->motor_active();
            } else {
                active = _axis[axis]->motor_active();
            }

            return active;
        }

        bool axis_endstop(int axis, enum Axis::axis_stop_e stop, bool *is_phys = NULL)
        {
            return _axis[axis]->endstop(stop, is_phys);
        }

        void axis_motor(bool enabled, uint8_t axis_mask = 0xff)
        {
            for (int i = 0; i < AXIS_MAX; i++)
                if ((1 << i) && axis_mask)
                    _axis[i]->motor_enable(enabled);
        }

        void axis_enable(uint8_t axis_mask = 0xff)
        {
            axis_motor(true, axis_mask);
        }

        void axis_disable(uint8_t axis_mask = 0xff)
        {
            axis_motor(false, axis_mask);
        }

        void stop()
        {
            axis_disable();
            _toolhead->tool()->stop();
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

        bool tool_offset_set(int tool_id, float *pos, uint8_t axis_mask)
        {
            Tool *t = tool(tool_id);

            if (!t)
                return false;

            t->offset_set(pos, axis_mask);

            return true;
        }

        bool update(unsigned long us_now)
        {
            bool motion = false;

            for (int i = 0; i < AXIS_MAX; i++)
                motion |= _axis[i]->update(us_now);

            if (motion)
                tool()->update(us_now);

            return motion;
        }
};

#endif /* CNC_H */
/* vim: set shiftwidth=4 expandtab:  */

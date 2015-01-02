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

#ifndef GCODE_H
#define GCODE_H

#include "Axis.h"
#include "Tool.h"

#define GCODE_LINE_MAX  128
#define GCODE_QUEUE_MAX 4

struct gcode_parameter {
    char label;
    float value;
};

struct gcode_block {
    struct gcode_block *next;

    bool buffered;
    char code;
    int  cmd;
    int  num;
    int32_t axis[AXIS_MAX];
    float i;        /* x center of arc */
    float j;        /* y center of arc */
    float k;        /* z center of arc */
    float f;        /* feed rate */
    float p;        /* parameter */
    float q;        /* parameter */
    float r;        /* parameter */
    float s;        /* parameter */
};

struct gcode_line {
    uint8_t len;
    uint8_t cs;
    char buff[GCODE_LINE_MAX];
};

class GCode {
    private:
        Axis *_axis[AXIS_MAX];
        Tool *_tool;
        Stream *_stream;

        struct gcode_line _line;
        struct {
            struct gcode_block ring[GCODE_QUEUE_MAX];
            struct gcode_block *free;
            struct gcode_block *active;
            struct gcode_block *pending, **pending_tail;
        } _block;
        enum { ABSOLUTE = 0, RELATIVE } _positioning;
        float _units_to_mm;
        enum { MODE_SLEEP = 0, MODE_STOP, MODE_ON } _mode;
    public:
        GCode() { }
        ~GCode() { }

        void begin(Stream *s, Axis *x, Axis *y, Axis *z, Axis *e, Tool *t)
        {
            _positioning = ABSOLUTE;
            _units_to_mm = 1.0;
            _stream = s;
            _tool = t;
            _axis[AXIS_X] = x;
            _axis[AXIS_Y] = y;
            _axis[AXIS_Z] = z;
            _axis[AXIS_E] = e;

            _stream->println("start");

            for (int i = 0; i < GCODE_QUEUE_MAX - 1; i++) {
                _block.ring[i].next = &_block.ring[i+1];
            }
            _block.free = &_block.ring[0];
            _block.pending_tail = &_block.pending;

            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_disable();
        }

        void update();
    private:
        void _block_do(struct gcode_block *blk);
        bool _line_parse(struct gcode_line *line, struct gcode_block *blk);
};

#endif /* GCODE_H */
/* vim: set shiftwidth=4 expandtab:  */

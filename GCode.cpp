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

#include <HardwareSerial.h>
#include <ctype.h>

#include <Wire.h>

#include "GCode.h"

static void _line_reset(struct gcode_line *line)
{
    line->len = 0;
}

static bool _line_update(struct gcode_line *line, char c)
{
    if (line->len >= (GCODE_LINE_MAX - 1))
        return false;

    if (line->len == 0 &&  isspace(c))
        return false;

    line->buff[line->len++] = c;

    if (c == '\n' || c == '\r') {
        line->buff[line->len] = 0;
        return true;
    }

    return false;
}

/*  false - resend
 *  true - do
 */
bool GCode::_line_parse(struct gcode_line *line, struct gcode_block *blk)
{
    char *cp, *is_cs = NULL;
    uint8_t cs = 0;
    enum { INVALID, INTEGER, FLOAT, FLOAT_FRAC } mode = INVALID;
    union {
        float *fptr;
        int *iptr;
    } data;
    int i, ipart = 0, fpart = 0, fbase = 0;
    int neg = 1;
    static struct gcode_block state;
    float axis[AXIS_MAX];

    data.fptr = NULL;
    state.code = state.cmd = 0;

    for (i = 0; i < AXIS_MAX; i++)
        axis[i] = (_positioning == RELATIVE) ? 0 : state.axis[i];

    for (cp = line->buff; *cp; cp++) {
        if (*cp == ';')
            break;

        if (*cp == '*') {
            is_cs = (cp+1);
            break;
        }

        cs ^= *cp;
        if (mode == FLOAT && *cp == '.') {
            fbase = 0;
            fpart = 0;
            mode = FLOAT_FRAC;
            continue;
        }
       
        if (*cp == '-') {
            neg *= -1;
            continue;
        }

        if (isdigit(*cp)) {
            if (mode == INTEGER || mode == FLOAT) {
                ipart *= 10;
                ipart += *cp - '0';
            } else if (mode == FLOAT_FRAC) {
                fpart *= 10;
                fbase *= 10;
                fpart += *cp - '0';
            }
            continue;
        }

        switch (mode) {
        case INTEGER:
            *data.iptr = neg * ipart;
            mode = INVALID;
            ipart = 0;
            break;
        case FLOAT:
            *data.fptr = neg * (float)ipart;
            mode = INVALID;
            ipart = 0;
            break;
        case FLOAT_FRAC:
            *data.fptr = neg * (float)ipart + ((float)fpart / (float)fbase);
            mode = INVALID;
            ipart = 0;
            break;
        case INVALID:
            break;
        }
                
        if (isspace(*cp))
            continue;

        switch (*cp) {
        case 'N':
            mode = INTEGER;
            data.iptr = &state.num;
            break;
        case 'G':
        case 'M':
        case 'T':
            mode = INTEGER;
            state.code = *cp;
            data.iptr = &state.cmd;
            break;
        case 'I':
            mode = FLOAT;
            data.fptr = &state.i;
            break;
        case 'J':
            mode = FLOAT;
            data.fptr = &state.j;
            break;
        case 'K':
            mode = FLOAT;
            data.fptr = &state.k;
            break;
        case 'P':
            mode = FLOAT;
            data.fptr = &state.p;
            break;
        case 'Q':
            mode = FLOAT;
            data.fptr = &state.q;
            break;
        case 'R':
            mode = FLOAT;
            data.fptr = &state.r;
            break;
        case 'S':
            mode = FLOAT;
            data.fptr = &state.s;
            break;
        case 'X':
            mode = FLOAT;
            data.fptr = &axis[AXIS_X];
            break;
        case 'Y':
            mode = FLOAT;
            data.fptr = &axis[AXIS_Y];
            break;
        case 'Z':
            mode = FLOAT;
            data.fptr = &axis[AXIS_Z];
            break;
        case 'E':
            mode = FLOAT;
            data.fptr = &axis[AXIS_E];
            break;
        case 'F':
            mode = FLOAT;
            data.fptr = &state.f;
        default:
            mode = INVALID;
            break;
        }

        if (mode == INTEGER) {
            *data.iptr = 0;
            neg = 1;
        }
        if (mode == FLOAT) {
            *data.fptr = 0.0;
            neg = 1;
        }
    }

    if (is_cs) {
        int xcs = 0;
        while (*is_cs && isdigit(*is_cs) && xcs < 256) {
            xcs *= 10;
            xcs += *(is_cs++) - '0';
        }
        if (xcs != cs) {
            state.cmd = state.code = 0;
            return false;
        }
    }

    /* Determine if this is a queued block */
    if (state.code == 'G' &&
            (state.cmd == 0 ||   /* G0  - Uncontrolled move */
             state.cmd == 1 ||   /* G1  - Linear move */
             state.cmd == 2 ||   /* G2  - Arc Clockwise */
             state.cmd == 3 ||   /* G3  - Arc Counter-Clockwise */
             state.cmd == 28 ||  /* G28 - Move to origin */
             state.cmd == 29 ||  /* G29 - Detailed Z-probe */
             state.cmd == 30 ||  /* G30 - Single Z-probe */
             state.cmd == 31 ||  /* G31 - Report Current Probe Status */
             state.cmd == 32)) {  /* G32 - Probe Z and caclulate Z plane */
        state.buffered = true;
    } else {
        state.buffered = false;
    }

    for (i = 0; i < AXIS_MAX; i++)
        state.axis[i] = axis[i] * _units_to_mm * _axis[i]->mm_to_position();

    state.next = blk->next;
    *blk = state;

    return true;
}

void GCode::_block_do(struct gcode_block *blk)
{
    float dist, time;

    switch (blk->code) {
    case 'T':
        _tool->stop();
        _tool->select(blk->cmd);
        _tool->parm(blk->p, blk->q, blk->r, blk->s);
        _tool->start();
        break;
    case 'G':
        switch (blk->cmd) {
        case 0: /* G0 - Uncontrolled move */
        case 1: /* G1 - Controlled move */
            Serial.print("// G0 X:");Serial.print(blk->axis[AXIS_X]);
            Serial.print(" Y:");Serial.print(blk->axis[AXIS_Y]);
            Serial.print(" Z:");Serial.print(blk->axis[AXIS_Z]);
            Serial.print(" E:");Serial.print(blk->axis[AXIS_E]);
            Serial.print(" F:");Serial.print(blk->f);

            dist = 0.0;
            for (int i = 0; i < AXIS_MAX; i++) {
                float delta;
                int32_t pdelta = blk->axis[i];

                switch (_positioning) {
                case ABSOLUTE: pdelta -= _axis[i]->target_get();
                case RELATIVE: break;
                }

                delta = pdelta /  _axis[i]->mm_to_position();
                dist  += delta * delta;
            }

            time = sqrt(dist) / blk->f;
            Serial.print(" t:");Serial.print(time);

            for (int i = 0; i < AXIS_MAX; i++) {
                switch (_positioning) {
                case ABSOLUTE: _axis[i]->target_set(blk->axis[i], time); break;
                case RELATIVE: _axis[i]->target_move(blk->axis[i], time); break;
                }
            }
            Serial.print(" C: X:");Serial.print(_axis[AXIS_X]->target_get());
            Serial.print(" Y:");Serial.print(_axis[AXIS_Y]->target_get());
            Serial.print(" Z:");Serial.print(_axis[AXIS_Z]->target_get());
            Serial.print(" E:");Serial.print(_axis[AXIS_E]->target_get());
            Serial.println();
            break;
        case 20: /* G20 - Set units to inches */
            _units_to_mm = 25.4;
            break;
        case 21: /* G21 - Set units to mm */
            _units_to_mm = 1.0;
            break;
        case 28: /* G28 - Re-home */
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->home();
            break;
        case 90: /* G90 - Set to absolute positioning */
            _positioning = ABSOLUTE;
            break;
        case 91: /* G91 - Set to Relative position */
            _positioning = RELATIVE;
            break;
        case 92: /* G92 - Set position */
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->position_set(blk->axis[i]);
        default:
            break;
        }
        break;
    case 'M':
        switch (blk->cmd) {
        case 0: /* M0 - Stop */
            _mode = MODE_STOP;
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_disable();
            break;
        case 1: /* M1 - Sleep */
            _mode = MODE_SLEEP;
            _tool->stop();
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_disable();
            break;
        case 17: /* M17 - Enable motors */
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_enable();
            break;
        case 18: /* M18 - Disable motors */
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_disable();
            break;
        case 114: /* M114 - Get current position */
            _stream->print(" C: X:");
            _stream->print((float)_axis[AXIS_X]->position_get()/
                                  _axis[AXIS_X]->mm_to_position()/
                                  _units_to_mm);
            _stream->print(" Y:");
            _stream->print((float)_axis[AXIS_Y]->position_get()/
                                  _axis[AXIS_Y]->mm_to_position()/
                                  _units_to_mm);
            _stream->print(" Z:");
            _stream->print((float)_axis[AXIS_Z]->position_get()/
                                  _axis[AXIS_Z]->mm_to_position()/
                                  _units_to_mm);
            _stream->print(" E:");
            _stream->print((float)_axis[AXIS_E]->position_get()/
                                  _axis[AXIS_E]->mm_to_position()/
                                  _units_to_mm);
            break;
        case 115: /* M115 - Get firmware version */
            _stream->print(" FIRMWARE_NAME:BrundleFab");
            break;
        case 124: /* M124 - Immediate motor stop */
            _tool->stop();
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_halt();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void GCode::update()
{
    struct gcode_block *blk;
    bool busy = false;

    for (int i = 0; i < AXIS_MAX; i++)
        busy |= _axis[i]->motor_active();

    /* If the axes are idle, then the current active block is done */
    if (!busy) {
        if (_block.active) {
Serial.print("// Done ");Serial.print(_block.active->code);Serial.println(_block.active->cmd);
            _block.active->next = _block.free;
            _block.free = _block.active;
            _block.active = NULL;
        }
        /* New pending block? */
        if (_block.pending) {
            _block.active = _block.pending;
            _block.pending = _block.pending->next;
            if (_block.pending == NULL)
                _block.pending_tail = &_block.pending;
Serial.print("// Execute ");Serial.print(_block.active->code);Serial.println(_block.active->cmd);
            if (!_block.active->buffered)
                _stream->print("ok");
            _block_do(_block.active);
            if (!_block.active->buffered)
                _stream->println();
        }
    }

    blk = _block.free;
    if (!blk) {
Serial.println("// No free blocks");
        return;
    }

    if (!_stream->available())
        return;

    char c = _stream->read();
Serial.print(c);
if (c == '\r') Serial.print('\n');
    if (!_line_update(&_line, c))
        return;

    if (_mode == MODE_STOP)
        _stream->println("!!");

    /* We have a line! */
    if (!_line_parse(&_line, blk)) {
        _stream->print("rs");
        _stream->println(blk->num);
    } else {
        /* Special case: M112 Emergency stop */
        if (blk->code == 'M' && blk->cmd == 112) {
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_disable();
            _mode = MODE_STOP;
            _stream->println("ok");
            return;
        }

        if (_mode == MODE_SLEEP) {
Serial.println("// Waking up...");
            for (int i = 0; i < AXIS_MAX; i++)
                _axis[i]->motor_enable();
            _mode = MODE_ON;
        }

        _block.free = blk->next;
        blk->next = NULL;
        *_block.pending_tail = blk;
        _block.pending_tail = &blk->next;
        if (blk->buffered)
            _stream->println("ok");
    }

    _line_reset(&_line);

    return;
}

/* vim: set shiftwidth=4 expandtab:  */

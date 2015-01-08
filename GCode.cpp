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

    if (c == '\n' || c == '\r') {
        line->buff[line->len] = 0;
        return true;
    }

    line->buff[line->len++] = c;

    return false;
}

/*  false - resend
 *  true - do
 */
bool GCode::_line_parse(struct gcode_line *line, struct gcode_block *blk)
{
    char *cp, *is_cs = NULL;
    uint8_t cs = 0;
    enum { INVALID, INTEGER, FLOAT, FLOAT_FRAC, FILENAME } mode = INVALID;
    union {
        float *fptr;
        int *iptr;
    } data;
    int i, ipart = 0, fpart = 0, fbase = 0;
    int neg = 1;
    struct gcode_block state = { 0 };

    data.fptr = NULL;

    for (cp = line->buff; *cp; cp++) {
        if (*cp == ';')
            break;

        if (*cp == '*') {
            is_cs = (cp+1);
            break;
        }

        cs ^= *cp;

        if (mode == FILENAME) {
            if (isspace(*cp)) {
                if (ipart > 0) {
                    state.filename[ipart] = 0;
                    mode = INVALID;
                    ipart = 0;
                } else
                    continue;
            }
            if (ipart < (int)sizeof(state.filename)-1) {
                state.filename[ipart++] = *cp;
                continue;
            }
        } else {
            if (mode == FLOAT && *cp == '.') {
                fbase = 1;
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
        }

        switch (mode) {
        case INTEGER:
            *data.iptr = neg * ipart;
            break;
        case FLOAT:
            *data.fptr = neg * (float)ipart;
            break;
        case FLOAT_FRAC:
            *data.fptr = neg * (float)ipart + ((float)fpart / (float)fbase);
            break;
        case FILENAME:
            state.filename[ipart] = 0;
            break;
        case INVALID:
            break;
        }

        mode = INVALID;
        ipart= 0;

        if (state.code == 'M' && (
                    state.cmd == 23 ||
                    state.cmd == 28 ||
                    state.cmd == 29 ||
                    state.cmd == 30 ||
                    state.cmd == 32 ||
                    state.cmd == 36)) {
            if (!(state.update_mask & GCODE_UPDATE_FILENAME)) {
                state.update_mask |= GCODE_UPDATE_FILENAME;
                mode = FILENAME;
                ipart = 0;
                continue;
            }
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
            state.update_mask |= GCODE_UPDATE_I;
            break;
        case 'J':
            mode = FLOAT;
            data.fptr = &state.j;
            state.update_mask |= GCODE_UPDATE_J;
            break;
        case 'K':
            mode = FLOAT;
            data.fptr = &state.k;
            state.update_mask |= GCODE_UPDATE_K;
            break;
        case 'P':
            mode = FLOAT;
            data.fptr = &state.p;
            state.update_mask |= GCODE_UPDATE_P;
            break;
        case 'Q':
            mode = FLOAT;
            data.fptr = &state.q;
            state.update_mask |= GCODE_UPDATE_Q;
            break;
        case 'R':
            mode = FLOAT;
            data.fptr = &state.r;
            state.update_mask |= GCODE_UPDATE_R;
            break;
        case 'S':
            mode = FLOAT;
            data.fptr = &state.s;
            state.update_mask |= GCODE_UPDATE_S;
            break;
        case 'X':
            mode = FLOAT;
            data.fptr = &state.axis[AXIS_X];
            state.update_mask |= GCODE_UPDATE_AXIS(AXIS_X);
            break;
        case 'Y':
            mode = FLOAT;
            data.fptr = &state.axis[AXIS_Y];
            state.update_mask |= GCODE_UPDATE_AXIS(AXIS_Y);
            break;
        case 'Z':
            mode = FLOAT;
            data.fptr = &state.axis[AXIS_Z];
            state.update_mask |= GCODE_UPDATE_AXIS(AXIS_Z);
            break;
        case 'E':
            mode = FLOAT;
            data.fptr = &state.axis[AXIS_E];
            state.update_mask |= GCODE_UPDATE_AXIS(AXIS_E);
            break;
        case 'F':
            mode = FLOAT;
            data.fptr = &state.f;
            state.update_mask |= GCODE_UPDATE_F;
            break;
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

    switch (mode) {
    case INTEGER:
        *data.iptr = neg * ipart;
        break;
    case FLOAT:
        *data.fptr = neg * (float)ipart;
        break;
    case FLOAT_FRAC:
        *data.fptr = neg * (float)ipart + ((float)fpart / (float)fbase);
        break;
    case FILENAME:
        state.filename[ipart] = 0;
        break;
    case INVALID:
        break;
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
    } else if (state.code == 'T') {
        state.buffered = true;
    } else {
        state.buffered = false;
    }

    for (i = 0; i < AXIS_MAX; i++) {
        if (state.update_mask & GCODE_UPDATE_AXIS(i))
            state.axis[i] *= _units_to_mm;
    }
    if (state.update_mask & GCODE_UPDATE_F)
        state.f *= _units_to_mm;

    state.next = blk->next;
    *blk = state;

    _line_reset(line);

    return true;
}

void GCode::_block_do(struct gcode_block *blk)
{
    float dist, time;
    File tmp_file;

    switch (blk->code) {
    case 'T':
        _debug->print("// T");_debug->print(blk->cmd);
        _tool->stop();
        _tool->select(blk->cmd);
        if (blk->update_mask & GCODE_UPDATE_P) {
            _tool->parm(Tool::TOOL_PARM_P, blk->p);
            _debug->print(" P:");_debug->print(blk->p);
        }
        if (blk->update_mask & GCODE_UPDATE_Q) {
            _tool->parm(Tool::TOOL_PARM_Q, blk->q);
            _debug->print(" Q:");_debug->print(blk->q);
        }
        if (blk->update_mask & GCODE_UPDATE_R) {
            _tool->parm(Tool::TOOL_PARM_R, blk->r);
            _debug->print(" R:");_debug->print(blk->r);
        }
        if (blk->update_mask & GCODE_UPDATE_S) {
            _tool->parm(Tool::TOOL_PARM_S, blk->s);
            _debug->print(" S:");_debug->print(blk->s);
        }
        _tool->start();
        _debug->println();
        break;
    case 'G':
        switch (blk->cmd) {
        case 0: /* G0 - Uncontrolled move */
        case 1: /* G1 - Controlled move */
            _debug->print("// G");_debug->print(blk->cmd);
            for (int i = 0; i < AXIS_MAX; i++) {
                if (!(blk->update_mask & GCODE_UPDATE_AXIS(i)))
                    continue;

                _debug->print(" ");
                _debug->print("XYZE"[i]);
                _debug->print(":");
                _debug->print(blk->axis[i]);
            }
            if (blk->update_mask & GCODE_UPDATE_F) {
                _debug->print(" F:");_debug->print(blk->f);
                _feed_rate = blk->f * _units_to_mm;
            }

            dist = 0.0;
            for (int i = 0; i < AXIS_MAX; i++) {
                if (!(blk->update_mask & GCODE_UPDATE_AXIS(i)))
                    continue;

                float delta = blk->axis[i];

                switch (_positioning) {
                case ABSOLUTE: delta -= _offset[i] + _axis[i]->target_get_mm();
                case RELATIVE: break;
                }

                dist += delta * delta;
            }

            time = sqrt(dist) / _feed_rate;
            _debug->print(" t:");_debug->print(time * 60);

            for (int i = 0; i < AXIS_MAX; i++) {
                if (!(blk->update_mask & GCODE_UPDATE_AXIS(i)))
                    continue;

                switch (_positioning) {
                case ABSOLUTE: _axis[i]->target_set_mm(blk->axis[i] + _offset[i], time); break;
                case RELATIVE: _axis[i]->target_move_mm(blk->axis[i], time); break;
                }
            }
            _debug->print(" C: X:");_debug->print(_axis[AXIS_X]->target_get_mm());
            _debug->print(" Y:");_debug->print(_axis[AXIS_Y]->target_get_mm());
            _debug->print(" Z:");_debug->print(_axis[AXIS_Z]->target_get_mm());
            _debug->print(" E:");_debug->print(_axis[AXIS_E]->target_get_mm());
            _debug->println();
            break;
        case 20: /* G20 - Set units to inches */
            _units_to_mm = 25.4;
            break;
        case 21: /* G21 - Set units to mm */
            _units_to_mm = 1.0;
            break;
        case 28: /* G28 - Re-home */
            for (int i = 0; i < AXIS_MAX; i++) {
                if (blk->update_mask & GCODE_UPDATE_AXIS(i))
                    _axis[i]->home_mm(blk->axis[i]);
            }
            break;
        case 90: /* G90 - Set to absolute positioning */
            _positioning = ABSOLUTE;
            break;
        case 91: /* G91 - Set to Relative position */
            _positioning = RELATIVE;
            break;
        case 92: /* G92 - Set position */
            for (int i = 0; i < AXIS_MAX; i++) {
                if (blk->update_mask & GCODE_UPDATE_AXIS(i))
                    _offset[i] = (_axis[i]->position_get_mm() + blk->axis[i]);
            }
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
        case 23: /* M23 - Select SD file */
            _file = SD.open(blk->filename);
            break;
        case 24: /* M24 - Start SD print */
            _file_enable = true;
            break;
        case 25: /* M25 - Pause SD print */
            _file_enable = false;
            break;
        case 26: /* M26 - Set SD position */
            if (_file)
                _file.seek((uint32_t)blk->s);
            break;
        case 30: /* M30 - Delete file from SD */
            SD.remove(blk->filename);
            break;
        case 32: /* M32 - Select SD file, and print */
            _file = SD.open(blk->filename);
            _file_enable = true;
            break;
        case 36: /* M36 - Return file information */
            tmp_file = SD.open(blk->filename);
            _stream->print(" {\"err\":");
            if (tmp_file) {
                _stream->print("0,\"size\":");
                _stream->print(tmp_file.size());
                _stream->print("}");
                tmp_file.close();
            } else {
                _stream->print("1}");
            }
            break;
        case 111: /* M111 - Set debug */
            if (blk->update_mask & GCODE_UPDATE_S) {
                int s = (int)blk->s;
                if (s & DEBUG_ECHO)
                    _debug = _stream;
                else
                    _debug = &_null;
            }
            break;
        case 114: /* M114 - Get current position */
            _stream->print(" C: X:");
            _stream->print((float)_axis[AXIS_X]->position_get_mm()/
                                  _units_to_mm);
            _stream->print(" Y:");
            _stream->print((float)_axis[AXIS_Y]->position_get_mm()/
                                  _units_to_mm);
            _stream->print(" Z:");
            _stream->print((float)_axis[AXIS_Z]->position_get_mm()/
                                  _units_to_mm);
            _stream->print(" E:");
            _stream->print((float)_axis[AXIS_E]->position_get_mm()/
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
            _block_do(_block.active);
        }
    }

    blk = _block.free;
    if (!blk) {
        return;
    }

    /* Serial input is of higher priority than SD input */
    if (_stream->available()) {
        char c = _stream->read();
        if (_stream_line.len == 0)
            _debug->print("// ");
        _debug->print(c);
        if (c == '\r')
            _debug->print('\n');
        if (_line_update(&_stream_line, c)) {
            if (_mode == MODE_STOP) {
                _stream->println("!!");
            } else if (_line_parse(&_stream_line, blk)) {
                _stream->print("ok");
                _process_block(blk);
                _stream->println();
                return;
            } else {
                _stream->print("rs");
                _stream->println(blk->num);
            }
        }
    }

    if (_file_enable && _file && _file.available()) {
        char c = _file.read();
        if (_line_update(&_file_line, c) &&
            _line_parse(&_file_line, blk)) {
            _process_block(blk);
            return;
        }
    }

    return;
}

void GCode::_process_block(struct gcode_block *blk)
{
    /* Special case: M112 Emergency stop */
    if (blk->code == 'M' && blk->cmd == 112) {
        for (int i = 0; i < AXIS_MAX; i++)
            _axis[i]->motor_disable();
        _mode = MODE_STOP;
        return;
    }

    if (_mode == MODE_SLEEP) {
        for (int i = 0; i < AXIS_MAX; i++)
            _axis[i]->motor_enable();
        _mode = MODE_ON;
    }

    if (blk->buffered) {
        _block.free = blk->next;
        blk->next = NULL;
        *_block.pending_tail = blk;
        _block.pending_tail = &blk->next;
    } else {
        _block_do(blk);
    }
}

/* vim: set shiftwidth=4 expandtab:  */

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
    line->buff[line->len] = 0;
    line->len = 0;
}

static bool _line_update(struct gcode_line *line, char c)
{
    if (c == '\n' || c == '\r')
        return true;

    if (line->len >= GCODE_LINE_MAX)
        return false;

    if (line->buff[line->len] != ';') {
        line->buff[line->len] = c;
        if (c != ';')
            line->len++;
    }

    return false;
}

/*  false - resend
 *  true - do
 */
bool GCode::_line_parse(struct gcode_line *line, struct gcode_block *blk)
{
    uint8_t cs = 0;
    enum { INVALID, INTEGER, FLOAT, FLOAT_FRAC, STRING } mode = INVALID;
    union {
        float *fptr;
        int *iptr;
    } data;
    int is_cs = 0, i, ipart = 0, fpart = 0, fbase = 0;
    int neg = 1;
    struct gcode_block state = { 0 };

    data.fptr = NULL;

    for (i = 0; i < line->len; i++) {
        char c = line->buff[i];

        if (c == '*') {
            is_cs = i + 1;
            break;
        }

        cs ^= c;

        if (mode == STRING) {
            if (isspace(c) && ipart == 0)
                continue;

            if (ipart < (int)sizeof(state.string)-1) {
                state.string[ipart++] = c;
                continue;
            }
        } else {
            if (mode == FLOAT && c == '.') {
                fbase = 1;
                fpart = 0;
                mode = FLOAT_FRAC;
                continue;
            }
           
            if (c == '-') {
                neg *= -1;
                continue;
            }

            if (isdigit(c)) {
                if (mode == INTEGER || mode == FLOAT) {
                    ipart *= 10;
                    ipart += c - '0';
                } else if (mode == FLOAT_FRAC) {
                    fpart *= 10;
                    fbase *= 10;
                    fpart += c - '0';
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
        case STRING:
            state.string[ipart] = 0;
            break;
        case INVALID:
            break;
        }

        mode = INVALID;
        ipart= 0;

        if (state.code == 'M' && (
                    state.cmd == 20 ||
                    state.cmd == 23 ||
                    state.cmd == 28 ||
                    state.cmd == 29 ||
                    state.cmd == 30 ||
                    state.cmd == 32 ||
                    state.cmd == 36 ||
                    state.cmd == 117 ||
                    state.cmd == 490 ||
                    state.cmd == 491 ||
                    state.cmd == 492 ||
                    state.cmd == 493 )) {
            if (!(state.update_mask & GCODE_UPDATE_STRING)) {
                state.update_mask |= GCODE_UPDATE_STRING;
                mode = STRING;
                ipart = 0;
                continue;
            }
        }
                
        if (isspace(c))
            continue;

        switch (c) {
        case 'N':
            mode = INTEGER;
            data.iptr = &state.num;
            break;
        case 'G':
        case 'M':
        case 'T':
            mode = INTEGER;
            state.code = c;
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
    case STRING:
        state.string[ipart] = 0;
        break;
    case INVALID:
        break;
    }

    if (is_cs > 0) {
        int xcs = 0;
        while ((is_cs < line->len) && isdigit(line->buff[is_cs]) && xcs < 256) {
            xcs *= 10;
            xcs += line->buff[is_cs++] - '0';
        }
        if (xcs != cs) {
            state.cmd = state.code = 0;
            _line_reset(line);
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
        if (state.update_mask & GCODE_UPDATE_AXIS(i)) {
            state.axis[i] *= _units_to_mm;
            if (_positioning == ABSOLUTE)
                state.axis[i] += _offset[i];
        }
    }
    if (state.update_mask & GCODE_UPDATE_F)
        state.f *= _units_to_mm;

    if (state.update_mask & GCODE_UPDATE_STRING) {
        if (state.string[0] == 0)
            state.update_mask &= ~GCODE_UPDATE_STRING;
    }

    state.next = blk->next;
    *blk = state;

    if (_debug != &_null && blk->code) {
        _debug->print("// ");
        _debug->print(blk->code);_debug->print(blk->cmd);
        for (int i = 0; i < AXIS_MAX; i++) {
            if (blk->update_mask & GCODE_UPDATE_AXIS(i)) {
                _debug->print(" ");
                _debug->print("XYZE"[i]);
                _debug->print(blk->axis[i]);
            }
        }
        if (blk->update_mask & GCODE_UPDATE_F) {
            _debug->print(" F");_debug->print(blk->f);
        }
        if (blk->update_mask & GCODE_UPDATE_I) {
            _debug->print(" I");_debug->print(blk->i);
        }
        if (blk->update_mask & GCODE_UPDATE_J) {
            _debug->print(" J");_debug->print(blk->j);
        }
        if (blk->update_mask & GCODE_UPDATE_K) {
            _debug->print(" K");_debug->print(blk->k);
        }
        if (blk->update_mask & GCODE_UPDATE_P) {
            _debug->print(" P");_debug->print(blk->p);
        }
        if (blk->update_mask & GCODE_UPDATE_Q) {
            _debug->print(" Q");_debug->print(blk->q);
        }
        if (blk->update_mask & GCODE_UPDATE_R) {
            _debug->print(" R");_debug->print(blk->r);
        }
        if (blk->update_mask & GCODE_UPDATE_S) {
            _debug->print(" S");_debug->print(blk->s);
        }
        if (blk->update_mask & GCODE_UPDATE_STRING) {
            _debug->print(" ");_debug->print(blk->string);
        }
        _debug->println();
    }

    _line_reset(line);

    return true;
}

void GCode::_block_do(struct gcode_block *blk)
{
    Stream *out = blk->io->out;
    ToolHead *th;
#if ENABLE_SD
    File tmp_file, *program;

    program = _cnc->program();
#endif

    switch (blk->code) {
    case 'T':
        th = _cnc->toolhead();

        th->stop();
        th->select(blk->cmd);
        if (blk->update_mask & GCODE_UPDATE_P)
            th->parm(Tool::PARM_P, blk->p);
        if (blk->update_mask & GCODE_UPDATE_Q)
            th->parm(Tool::PARM_Q, blk->q);
        if (blk->update_mask & GCODE_UPDATE_R)
            th->parm(Tool::PARM_R, blk->r);
        if (blk->update_mask & GCODE_UPDATE_S)
            th->parm(Tool::PARM_S, blk->s);
        th->start();
        break;
    case 'G':
        switch (blk->cmd) {
        case 0: /* G0 - Uncontrolled move */
            switch (_positioning) {
            case ABSOLUTE:
                _cnc->target_set(blk->axis, blk->update_mask);
                break;
            case RELATIVE:
                _cnc->target_move(blk->axis, blk->update_mask);
                break;
            }

            break;
        case 1: /* G1 - Controlled move */
            if (blk->update_mask & GCODE_UPDATE_F) {
                float rate = blk->f * _units_to_mm;
                if (rate > 10.0)
                    _feed_rate = rate;
            }

            switch (_positioning) {
            case ABSOLUTE:
                _cnc->target_set_rate(blk->axis, blk->update_mask, _feed_rate);
                break;
            case RELATIVE:
                _cnc->target_move_rate(blk->axis, blk->update_mask, _feed_rate);
                break;
            }

#if ENABLE_UI
            if (_vis) {
                float pos[AXIS_MAX];

                _cnc->target_get(pos);

                int color;
                int tool = _cnc->toolhead()->selected();
                /* Non-build tools are invisible.
                 * If we aren't extruding, use the tool color
                 */
                if (tool < 1 || tool > 16)
                    color = VC_INVISIBLE;
                else if (blk->update_mask & GCODE_UPDATE_AXIS(AXIS_E))
                    color = (blk->cmd == 0) ? VC_MOVE : VC_FEED;
                else
                    color = VC_TOOL;
                _vis->line_to(color, pos);
            }
#endif
            break;
        case 20: /* G20 - Set units to inches */
            _units_to_mm = 25.4;
            break;
        case 21: /* G21 - Set units to mm */
            _units_to_mm = 1.0;
            break;
        case 28: /* G28 - Re-home */
#if ENABLE_UI
            if (_vis) {
                float pos[AXIS_MAX];
                for (int i = 0; i < AXIS_MAX; i++) {
                    if (blk->update_mask & GCODE_UPDATE_AXIS(i))
                        pos[i] = blk->axis[i];
                    else
                        pos[i] = _cnc->axis(i)->target_get();
                }
                _vis->cursor_to(pos);
            }
#endif
            for (int i = 0; i < AXIS_MAX; i++) {
                if (blk->update_mask & GCODE_UPDATE_AXIS(i))
                    _cnc->axis(i)->home(blk->axis[i]);
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
                    _offset[i] = (_cnc->axis(i)->position_get() + blk->axis[i]);
            }
        default:
            break;
        }
        break;
    case 'M':
        switch (blk->cmd) {
        case 0: /* M0 - Stop, regardless of optional stop switch */
            _pause(blk->io, false);
            break;
        case 1: /* M1 - Stop, only if optional stop switch set */
            _pause(blk->io, true);
            break;
        case 17: /* M17 - Enable motors */
            _cnc->motor_enable();
            break;
        case 18: /* M18 - Disable motors */
            _cnc->motor_disable();
            break;
#if ENABLE_SD
        case 20: /* M20 - List SD files */
            out->print(" Files: {");
            if (blk->update_mask & GCODE_UPDATE_STRING)
                tmp_file = SD.open(blk->string);
            else
                tmp_file = SD.open("/");
            if (tmp_file) {
                for (;;) {
                    File entry = tmp_file.openNextFile();
                    if (!entry)
                        break;

                    if (strcmp(entry.name(), ".") == 0)
                        continue;
                    if (strcmp(entry.name(), "..") == 0)
                        continue;
                    out->print(entry.name());
                    out->print(",");
                }
            }
            out->print("}");
            break;
        case 23: /* M23 - Select SD file */
            file_select(blk->string, true);
            break;
        case 24: /* M24 - Start SD print */
            file_start();
            break;
        case 25: /* M25 - Pause SD print */
            file_stop();
            break;
        case 26: /* M26 - Set SD position */
            if (*program)
                program->seek((uint32_t)blk->s);
            break;
        case 27: /* M27 - Show SD position */
            if (*program) {
                out->print(" SD printing byte ");
                out->print(program->position());
                out->print("/");
                out->print(program->size());
            } else {
                out->print(" Not SD printing");
            }
            break;
        case 30: /* M30 - Delete file from SD */
            SD.remove(blk->string);
            break;
        case 32: /* M32 - Select SD file, and print */
            file_select(blk->string, true);
            break;
        case 36: /* M36 - Return file information */
            tmp_file = SD.open(blk->string);
            out->print(" {\"err\":");
            if (tmp_file) {
                out->print("0,\"size\":");
                out->print(tmp_file.size());
                out->print("}");
                tmp_file.close();
            } else {
                out->print("1}");
            }
            break;
#endif /* ENABLE_SD */
        case 111: /* M111 - Set debug */
            if (blk->update_mask & GCODE_UPDATE_S) {
                int s = (int)blk->s;
                if (s & DEBUG_ECHO)
                    _debug = out;
                else
                    _debug = &_null;
            }
            break;
        case 114: /* M114 - Get current position */
            out->print(" C: X:");
            out->print((float)_cnc->axis(AXIS_X)->position_get()/
                                  _units_to_mm);
            out->print(" Y:");
            out->print((float)_cnc->axis(AXIS_Y)->position_get()/
                                  _units_to_mm);
            out->print(" Z:");
            out->print((float)_cnc->axis(AXIS_Z)->position_get()/
                                  _units_to_mm);
            out->print(" E:");
            out->print((float)_cnc->axis(AXIS_E)->position_get()/
                                  _units_to_mm);
            break;
        case 115: /* M115 - Get firmware version */
            out->print(" FIRMWARE_NAME:BrundleFab");
            break;
        case 117:
            out->print(" ");
            out->print(blk->string);
            _cnc->status_set(blk->string);
            break;
        case 119: /* M119 - Report endstop status */
            for (int i = 0; i < AXIS_MAX; i++) {
                bool state, exists;
                state = _cnc->axis(i)->endstop(Axis::STOP_MIN, &exists);
                if (exists) {
                    out->print(" ");out->print("xyze"[i]);
                    out->print("_min:");out->print(state ? "TRIGGERED" : "open");
                }
                state = _cnc->axis(i)->endstop(Axis::STOP_MAX, &exists);
                if (exists) {
                    out->print(" ");out->print("xyze"[i]);
                    out->print("_max:");out->print(state ? "TRIGGERED" : "open");
                }
            }
            break;
        case 124: /* M124 - Immediate motor stop */
            _cnc->stop();
            break;
        case 490: /* M490 - Send message to serial bus 0 */
        case 491: /* M491 - Send message to serial bus 1 */
        case 492: /* M492 - Send message to serial bus 2 */
        case 493: /* M493 - Send message to serial bus 3 */
            Stream *s;
            s = _cnc->serial_get(blk->cmd - 490);
            if (s) {
                s->write(blk->string);
                out->print(' ');
                while (s->available()) {
                    char c = s->read();
                    if (c == '\n')
                        continue;
                    if (c == '\r')
                        c = '.';
                    out->print(c);
                }
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void GCode::update(bool cnc_active)
{
    /* If the axes are idle, then the current active block is done */
    if (!cnc_active) {
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

    /* Serial input is of higher priority than SD input */
    _process_io(&_console);

#if ENABLE_SD
    _process_io(&_program);
#endif
}

void GCode::_process_io(struct gcode_io *io)
{
    struct gcode_block *blk;
    
    /* If paused, wait for the Cycle Start button to be pressed
     */
    if (!_enabled(io)) {
        if (_cnc->button_get(CNC_BUTTON_CYCLE_START)) {
            _start(io);
        } else {
            return;
        }
    }

    blk = _block.free;

    if (!blk)
        return;

    if (io->in->available()) {
        char c;
        
        if (io->line.len == 0)
            _debug->print("// ");

        c = io->in->read();

        if (c == '\n' || c == '\r')
            _debug->println();
        else
            _debug->print(c);

        if (_line_update(&io->line, c)) {
            if (_halted) {
                io->out->println("!!");
            } else if (_line_parse(&io->line, blk)) {
                io->out->print("ok");

                blk->io = io;

                _process_block(blk);

                io->out->println();
            } else {

                io->out->print("rs");

                io->out->println(blk->num);
            }
        }
    }
}

void GCode::_process_block(struct gcode_block *blk)
{
    /* Special case: M112 Emergency stop */
    if (blk->code == 'M' && blk->cmd == 112) {
        _cnc->motor_disable();
        _halted = true;
        return;
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

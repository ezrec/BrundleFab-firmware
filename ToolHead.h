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

#ifndef TOOLHEAD_H
#define TOOLHEAD_H

#include "Tool.h"

#define TOOL_NONE       0

#define MAX_TOOLS       (16 + 1)

class ToolHead : Tool {
    private:
        int _id;
        Tool *_tool;            /* Current tool */
        const float *_offset;   /* Current offset */

        int _tools;
        struct {
            int id;
            Tool *tool;
            float offset[AXIS_MAX];
        } _map[MAX_TOOLS];

        Tool _tool_null;

    public:
        ToolHead() :
            _tool_null()
        {
            _map[0].id = 0;
            _map[0].tool = &_tool_null;
            _tools = 1;
            _id = _map[0].id;
            _tool = _map[0].tool;
        }

        virtual bool attach(int id, Tool *tool)
        {
            if (_tools >= MAX_TOOLS || id <= 0)
                return false;

            for (int i = 0; i < _tools; i++) {
                if (_map[i].id == id) {
                    _map[_tools].tool = tool;
                    return true;
                }
            }

            _map[_tools].id = id;
            _map[_tools].tool = tool;
            _tools++;
            return true;
        }

        virtual void begin()
        {
            select(0);
        }

        virtual bool select(int tool)
        {
            for (int i = 0; i < _tools; i++) {
                if (_map[i].id == tool) {
                    _id = _map[i].id;
                    _tool = _map[i].tool;
                    _offset = &_map[i].offset[0];
                    return true;
                }
            }

            return false;
        }

        virtual int selected()
        {
            return _id;
        }

        virtual void start(void)
        {
            _tool->start();
        }

        virtual void stop(void)
        {
            _tool->stop();
        }

        virtual bool active(void)
        {
            return _tool->active();
        }

        virtual bool update()
        {
            return _tool->update();
        }

        virtual void parm_set(enum parm_e p, float val = 0.0)
        {
            _tool->parm_set(p, val);
        }

        virtual float parm_get(enum parm_e p)
        {
            return _tool->parm_get(p);
        }

        virtual bool offset_set(int tool, float *pos, uint8_t axis_mask)
        {
            for (int i = 0; i < _tools; i++) {
                if (_map[i].id == tool) {
                    for (int j = 0; j < AXIS_MAX; j++) {
                        if ((1 << j) && axis_mask)
                            _map[i].offset[j] = pos[j];
                    }
                    return true;
                }
            }

            return false;
        }

        virtual const float *offset_is()
        {
            return _offset;
        }
};

#endif /* TOOLHEAD_H */
/* vim: set shiftwidth=4 expandtab:  */

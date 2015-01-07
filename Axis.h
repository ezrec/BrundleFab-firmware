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

#ifndef AXIS_H
#define AXIS_H

#include <stdint.h>
#include <math.h>

#define AXIS_MAX        4
#define AXIS_X          0
#define AXIS_Y          1
#define AXIS_Z          2
#define AXIS_E          3

class Axis {
  private:
    int32_t _target, _velocity;
    bool _enabled, _valid;

  public:
    Axis() { _enabled = false; _valid = false; _target = 0; _velocity = 0; }
    ~Axis() {}

    virtual const float mm_to_position() { return 1.0; }

    virtual void begin()
    {
        motor_disable();
    }

    virtual void home(int32_t pos = 0)
    {
      _valid = true;
      target_set(pos);
    }
    virtual bool update() { return false; }

    virtual void motor_enable() { _enabled = true; }
    virtual void motor_disable() { motor_halt(); _enabled = false; _valid = false; }
    virtual bool motor_enabled() { return _enabled; }

    virtual bool motor_active() { return false; }
    virtual void motor_halt() { }

    /* Velocity is in position units / minute */
    virtual void velocity_set(int32_t vel) { _velocity = vel; }
    virtual int32_t velocity_get() { return _velocity; }

    virtual void target_set(int32_t pos, float time_min = 0.0)
    {
        if (time_min > 0.0)
            velocity_set(fabsf((pos - _target) / time_min));

        _target = pos;
    }
    virtual int32_t target_get(void) { return _target; }

    virtual const int32_t position_min(void) { return 0; }
    virtual const int32_t position_max(void) { return 0; }

    virtual bool position_valid() { return _valid; } 
    virtual int32_t position_get() { return 0; }

    virtual inline void home_mm(float pos_mm)
    {
        return home(pos_mm * mm_to_position());
    }
    virtual inline float target_get_mm()
    {
        return target_get() / mm_to_position();
    }
    virtual inline void target_set_mm(float pos_mm, float time_min = 0.0)
    {
        target_set(pos_mm * mm_to_position(), time_min);
    }
    virtual inline void target_move(int32_t pos, float time_min = 0.0)
    {
        target_set(target_get() + pos, time_min);
    }
    virtual inline void target_move_mm(float pos_mm, float time_min = 0.0)
    {
        target_move(pos_mm * mm_to_position(), time_min);
    }
    virtual inline float position_get_mm()
    {
        return position_get() / mm_to_position();
    }
};

#endif /* AXIS_H */
/* vim: set shiftwidth=4 expandtab:  */

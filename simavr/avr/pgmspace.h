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

#ifndef SIMAVR_AVR_PGMSPACE_H
#define SIMAVR_AVR_PGMSPACE_H

#define PROGMEM

char * ltoa (long val, char *s, int radix);
char * utoa (unsigned int val, char *s, int radix);
char * ultoa (unsigned long val, char *s, int radix);
char * itoa (int val, char *s, int radix);

static inline unsigned char pgm_read_byte(const char *c)
{
    return *c;
}

#endif /* SIMAVR_AVR_PGMSPACE_H */
/* vim: set shiftwidth=4 expandtab:  */

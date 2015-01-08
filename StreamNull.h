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

#ifndef STREAMNULL_H
#define STREAMNULL_H

#include <Stream.h>

class StreamNull : public Stream {
    public:
    virtual int available() { return false; }
    virtual int read() { return -1; }
    virtual int peek() { return -1; }
    virtual void flush() { }
    virtual size_t write(uint8_t) { return 1; }
};


#endif /* STREAMNULL_H */
/* vim: set shiftwidth=4 expandtab:  */

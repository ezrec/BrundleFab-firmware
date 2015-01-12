/*
  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
*/

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "Stream.h"

class HardwareSerial : public Stream
{
  private:
    int _io;
    int _c;
    struct termios _term;
  public:
    HardwareSerial()
    {
      _io = -1;
      _c = -1;
    }
    void begin(unsigned long baud_rate, uint8_t unit = 0)
    {
      _io = ::open("/dev/tty", O_RDWR | O_NONBLOCK);
      if (_io >= 0) {
        struct termios nterm;
        ::tcgetattr(_io, &_term);
        nterm = _term;
        ::cfmakeraw(&nterm);
        ::tcsetattr(_io, TCSANOW, &nterm);
      }

      _c = -1;
    }
    void end()
    {
      if (_io >= 0) {
        ::tcsetattr(_io, TCSANOW, &_term);
        ::close(_io);
      }
      _io = -1;
    }
    virtual int available(void)
    {
      char c;
      int err;

      if (_c >= 0)
        return true;

      err = ::read(_io, &c, 1);
      if (err < 0)
        return false;

      _c = c;
      return true;
    }
    virtual int peek(void)
    {
      if (!available())
        return -1;
      return _c;
    }
    virtual int read(void)
    {
      int c;

      c = peek();
      if (_c >= 0)
        _c = -1;

      return c;
    }
    virtual void flush(void) { }
    virtual size_t write(uint8_t c)
    {
      return ::write(_io, &c, 1);
    }
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return (_io >= 0) ? true : false; }
};

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

extern HardwareSerial Serial;

extern void serialEventRun(void) __attribute__((weak));

#endif
/* vim: set shiftwidth=2 expandtab: */

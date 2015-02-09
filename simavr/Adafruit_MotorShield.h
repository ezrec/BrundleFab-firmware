#ifndef _Adafruit_MotorShield_h_
#define _Adafruit_MotorShield_h_

#include <inttypes.h>

#include "Arduino.h"
#include "Encoder.h"
#include "pinout.h"

//#define MOTORDEBUG

#define MICROSTEPS 16         // 8 or 16

#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR4_A 0
#define MOTOR4_B 6
#define MOTOR3_A 5
#define MOTOR3_B 7

#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

#define SINGLE 1
#define DOUBLE 2
#define INTERLEAVE 3
#define MICROSTEP 4

class Adafruit_MotorShield;

class Adafruit_DCMotor
{
 public:
  Adafruit_DCMotor(void) {}
  friend class Adafruit_MotorShield;
  void run(uint8_t dir)
  {
    encoder_dir(_encoder, dir);
  }
  void setSpeed(uint8_t pwm)
  {
    encoder_speed(_encoder, pwm);
  }
  
 private:
  int _encoder;
};

class Adafruit_StepperMotor {
 public:
  Adafruit_StepperMotor(void) {}
  friend class Adafruit_MotorShield;

  void step(uint16_t steps, uint8_t dir,  uint8_t style = SINGLE)
  {
    switch (style) {
    case SINGLE: steps *= MICROSTEPS; break;
    case DOUBLE: steps *= MICROSTEPS/2; break;
    default: break;
    }

    if (dir == FORWARD) {
      if (steppingcounter + steps >= _limit)
        steppingcounter = _limit;
      else
        steppingcounter += steps;
    } else if (dir == BACKWARD) {
      if (steppingcounter < steps)
        steppingcounter = 0;
      else
        steppingcounter -= steps;
    }

    digitalWrite(_pinStopMin, (steppingcounter == 0) ? 1 : 0);

    digitalWrite(_pinStopMax, (steppingcounter == _limit) ? 1 : 0);
  }

  void setSpeed(uint16_t) {}
  uint8_t onestep(uint8_t dir, uint8_t style)
  { return (style == SINGLE) ? MICROSTEPS : 
           (style == DOUBLE) ? MICROSTEPS/2 :
           1;
  }
  void release(void) {}
  uint32_t usperstep, steppingcounter;

  private:
    int _pinStopMin;
    int _pinStopMax;
    uint32_t _limit;
};

class Adafruit_MotorShield
{
public:
  Adafruit_MotorShield(uint8_t addr = 0x60) {}
  friend class Adafruit_DCMotor;
  void begin(uint16_t freq = 1600) {}

  void setPWM(uint8_t pin, uint16_t val) {}
    void setPin(uint8_t pin, boolean val) {}
    Adafruit_DCMotor *getMotor(uint8_t n)
    {
      return &dcmotors[n];
    }
    Adafruit_StepperMotor *getStepper(uint16_t steps, uint8_t n)
    {
      Adafruit_StepperMotor *st;

      if (n <= 0 || n > 2)
        return NULL;
      st = &steppers[n-1];

      if (n == 2) {
        st->_pinStopMin = -1;
        st->_pinStopMax = Z_STP_MAX;
        st->_limit = 11500 * MICROSTEPS;
      }
      if (n == 1) {
        st->_pinStopMin = E_STP_MIN;
        st->_pinStopMax = -1;
        st->_limit = 11500 * MICROSTEPS;
      }
      return st;
    }
 private:
    uint8_t _addr;
    uint16_t _freq;
    Adafruit_DCMotor dcmotors[4];
    Adafruit_StepperMotor steppers[2];
};

#endif

/* vim: set shiftwidth=2 expandtab: */

// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#ifndef _AFMotor_h_
#define _AFMotor_h_

#include "Encoder.h"

    //#define MOTORDEBUG 1

#define MICROSTEPS 16                       // 8 or 16

#define DC_MOTOR_PWM_RATE 0

// Constants that the user passes in to the motor calls
#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

// Constants that the user passes in to the stepper calls
#define SINGLE 1
#define DOUBLE 2
#define INTERLEAVE 3
#define MICROSTEP 4

/*
#define LATCH 4
#define LATCH_DDR DDRB
#define LATCH_PORT PORTB

#define CLK_PORT PORTD
#define CLK_DDR DDRD
#define CLK 4

#define ENABLE_PORT PORTD
#define ENABLE_DDR DDRD
#define ENABLE 7

#define SER 0
#define SER_DDR DDRB
#define SER_PORT PORTB
*/

// Arduino pin names for interface to 74HCT595 latch
#define MOTORLATCH 12
#define MOTORCLK 4
#define MOTORENABLE 7
#define MOTORDATA 8

class AFMotorController
{
  public:
    AFMotorController(void) {}
    void enable(void) {}
    friend class AF_DCMotor;
    void latch_tx(void) {}
    uint8_t TimerInitalized;
};

class AF_DCMotor
{
 public:
  AF_DCMotor(uint8_t motornum, uint8_t freq = DC_MOTOR_PWM_RATE) {
    if (motornum == X_MOTOR) {
      _encoder = XENC_A;
      encoder_pinstop(_encoder, 11500, XSTP_MIN, XSTP_MAX);
    } else if (motornum == Y_MOTOR) {
      _encoder = YENC_A;
      encoder_pinstop(_encoder, 5250, YSTP_MIN, YSTP_MAX);
    } else {
      _encoder = 0;
    }
  }
  void run(uint8_t dir) { encoder_dir(_encoder, dir); }
  void setSpeed(uint8_t pwm)
  {
     if (pwm != _pwm) {
       encoder_speed(_encoder, (pwm >= 96) ? (((float)pwm - 96.0)/(200.0-96.0)) : 0.0);
     }
     _pwm = pwm;
  }

 private:
  int _encoder;
  uint8_t _pwm;
};

class AF_Stepper {
 public:
  AF_Stepper(uint16_t, uint8_t) {}
  void step(uint16_t steps, uint8_t dir,  uint8_t style = SINGLE) {}
  void setSpeed(uint16_t) {}
  uint8_t onestep(uint8_t dir, uint8_t style)
  { return (style == SINGLE) ? MICROSTEPS : 
           (style == DOUBLE) ? MICROSTEPS/2 :
           1;
  }
  void release(void) {}
  uint16_t revsteps; // # steps per revolution
  uint8_t steppernum;
  uint32_t usperstep, steppingcounter;
 private:
  uint8_t currentstep;

};

#endif

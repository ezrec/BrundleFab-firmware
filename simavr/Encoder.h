#ifndef Encoder_h_
#define Encoder_h_

#include <stdint.h>

#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

void encoder_pinstop(int n, uint32_t range, int pin_min, int pin_max);
void encoder_speed(int encoder, uint8_t pwm);
void encoder_dir(int encoder, uint8_t dir);
void encoder_reset(int n);
void encoder_set(int n, int32_t pos);
int32_t encoder_get(int n);

class Encoder
{
private:
	int _pinA, _pinB;

public:
	Encoder(uint8_t pin1, uint8_t pin2) {
		_pinA = pin1;
		_pinB = pin2;
		encoder_reset(_pinA);
	}

	inline int32_t read() {
		return encoder_get(_pinA);
	}
	inline void write(int32_t p) {
		encoder_set(_pinA, p);
	}
};

#endif

#ifndef Encoder_h_
#define Encoder_h_

class Encoder
{
private:
	int32_t _position;

public:
	Encoder(uint8_t pin1, uint8_t pin2) {
	}

	inline int32_t read() {
		return _position;
	}
	inline void write(int32_t p) {
		_position = p;
	}
};

#endif

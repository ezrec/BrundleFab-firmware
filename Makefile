# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

# For the RAMPS 1.4 electronics
SHIELD_TAG   = ramps
# For the BrundleFab protoboard
#SHIELD_TAG   = brundle

BOARD_TAG    = mega2560
ISP_PORT = /dev/ttyACM0
MONITOR_PORT = /dev/ttyACM0

MONITOR_BAUDRATE = 115200
ARDMK_DIR=/usr/share/arduino

CXXFLAGS = -DMICROSTEPS=16 -Wno-deprecated-declarations

ARDUINO_LIBS = Adafruit_MotorShield \
	       Adafruit_MotorShield/utility \
	       AFMotor \
	       Encoder \
	       Adafruit_ST7735 \
	       Adafruit_GFX \
	       SPI \
	       SD \
	       Wire

-include $(ARDMK_DIR)/Arduino.mk

EXTRA_FLAGS = -Wall -Werror -DSHIELD_$(SHIELD_TAG)
# !!! Important. You have to use make ispload to upload when using ISP programmer

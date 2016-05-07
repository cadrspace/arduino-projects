BOARD_TAG = mega
BOARD_SUB = atmega2560
ARDUINO_LIBS = Adafruit_CC3000_Library SPI WiFi

CXXFLAGS = \
	-I. \
	-I/usr/avr/include

CFLAGS=${CXXFLAGS} -I/opt/arduino/hardware/arduino/avr/cores/arduino

SOURCES = $(ls -tp *.ino | grep -v /$ | head -1) \
	UDPServer.cpp coap.cpp endpoints.cpp

include /opt/arduino/Arduino.mk

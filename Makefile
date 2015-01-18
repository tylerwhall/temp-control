CC = msp430-elf-gcc
LDFLAGS = -Wl,-Map=$@.map

MCU = msp430g2231
CFLAGS = -mmcu=$(MCU)

all: main

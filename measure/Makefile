# References : ../examples/autotest-validate/Makefile
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror -I/lib/x86_64-linux-gnu/include
LDFLAGS ?=  -lrt
TARGET ?= bme280_measure

.PHONY: all clean

all: $(TARGET)

bme280_measure: bme280_measure.c
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o $@ $< -lsqlite3

default: all

clean:
	rm -f $(TARGET)

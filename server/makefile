CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror -I/lib/x86_64-linux-gnu/include
LDFLAGS ?= -lpthread -lrt
TARGET ?= aesdsocket

.PHONY: all clean

all: $(TARGET)

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) $(LDFLAGS) -pthread -o $@ $< -lsqlite3

default: all

clean:
	rm -f $(TARGET)

CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror
LDFLAGS ?= -lpthread -lrt -lsqlite3
TARGET ?= aesdsocket

.PHONY: all clean

all: $(TARGET)

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) -pthread -o $@ $< $(LDFLAGS)

default: all

clean:
	rm -f $(TARGET)

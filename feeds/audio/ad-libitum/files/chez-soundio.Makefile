CC ?= cc
CFLAGS ?=
LDFLAGS ?=
SCHEMEH ?=
PREFIX ?= /usr/local
SHARED_FLAGS ?= -shared -fPIC

SOUNDIO_LIBS = ../libbridge.so

all: $(SOUNDIO_LIBS)

../libbridge.so: bridge.c
	$(CC) -O3 $(SHARED_FLAGS) -I$(SCHEMEH) -I$(PREFIX)/include \
	    $(CFLAGS) -o $@ $< -L$(PREFIX)/lib $(LDFLAGS) -lsoundio

clean:
	rm -f $(SOUNDIO_LIBS)

.PHONY: all clean

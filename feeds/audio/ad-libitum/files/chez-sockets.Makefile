CC ?= cc
CFLAGS ?=
LDFLAGS ?=
SCHEMEH ?=
SHARED_FLAGS ?= -shared -fPIC

SOCKET_LIBS = ../sockets-stub.so ../socket-ffi-values.so

all: $(SOCKET_LIBS)

../sockets-stub.so: sockets-stub.c
	$(CC) -O3 $(SHARED_FLAGS) -I$(SCHEMEH) $(CFLAGS) $(LDFLAGS) -o $@ $<

../socket-ffi-values.so: socket-ffi-values.c
	$(CC) -O3 $(SHARED_FLAGS) -I$(SCHEMEH) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(SOCKET_LIBS)

.PHONY: all clean

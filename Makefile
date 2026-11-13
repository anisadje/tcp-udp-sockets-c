CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGETS = client_tcp server_tcp client_udp server_udp

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean

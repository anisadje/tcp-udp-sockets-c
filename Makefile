CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGETS = client_tcp serveur_tcp client_udp serveur_udp

all: $(TARGETS)

client_tcp: src/client_tcp.c
	$(CC) $(CFLAGS) -o $@ $<

serveur_tcp: src/serveur_tcp.c
	$(CC) $(CFLAGS) -o $@ $<

client_udp: src/client_udp.c
	$(CC) $(CFLAGS) -o $@ $<

serveur_udp: src/serveur_udp.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
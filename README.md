# TCP & UDP Socket Programming in C

> Client/server communication in C with POSIX sockets, comparing **TCP** (connection-oriented) and **UDP** (connectionless).

---

## Description

This project covers the basics of **network programming in C** through two client/server architectures:

| Protocol | Type | Reliability | Typical use |
|----------|------|-------------|-------------|
| **TCP** | Connection-oriented | Guaranteed delivery | Web, email, file transfer |
| **UDP** | Connectionless | Not guaranteed | Streaming, online games, DNS |

### Features

- **TCP server**: accepts incoming connections, handles each client in a **child process** (`fork`) and answers the message it receives
- **TCP client**: connects to the server, sends a message and waits for the answer (5 s timeout)
- **UDP server**: receives datagrams and sends them back (echo)
- **UDP client**: sends a message without establishing a connection first

---

## Project structure

```
.
├── client_tcp.c      # TCP client with timeout
├── server_tcp.c      # Multi-process TCP server (fork)
├── client_udp.c      # UDP client
├── server_udp.c      # UDP echo server
├── Makefile
└── README.md
```

---

## Build

### With Make (recommended)

```bash
make          # builds all the executables
make clean    # removes the binaries
```

### Manually

```bash
gcc -Wall -Wextra -std=c99 -o client_tcp client_tcp.c
gcc -Wall -Wextra -std=c99 -o server_tcp server_tcp.c
gcc -Wall -Wextra -std=c99 -o client_udp client_udp.c
gcc -Wall -Wextra -std=c99 -o server_udp server_udp.c
```

---

## Usage

### TCP

**Terminal 1, start the server:**
```bash
./server_tcp
# TCP server listening on port 9600...
# [child 4242] Client connected: 127.0.0.1:58180
# [child 4242] Message received: "Hello!"
```

**Terminal 2, start the client:**
```bash
./client_tcp localhost
# Connected to server localhost on port 9600.
# Enter your message: Hello!
# Response: Server: message received!
```

### UDP

**Terminal 1, start the server:**
```bash
./server_udp
# UDP server listening on port 9600...
# [127.0.0.1:41234] Message received (6 bytes): "Hello!"
```

**Terminal 2, start the client:**
```bash
./client_udp localhost
# Enter your message: Hello!
# Message sent (6 bytes).
```

> **Note:** the default port is `9600`. Change the `PORT` constant in the source files to use another one.

---

## Concepts

### Life cycle of a TCP server socket
```
socket() → bind() → listen() → accept() → read()/write() → close()
```

### Life cycle of a UDP server socket
```
socket() → bind() → recvfrom()/sendto() → close()
```

### Zombie processes (TCP)
The TCP server uses `sigaction(SIGCHLD, ...)` with `waitpid()` so that finished child processes do not pile up as zombies while clients are handled concurrently.

---

## Topics covered

- POSIX sockets (`AF_INET`, `SOCK_STREAM`, `SOCK_DGRAM`)
- `sockaddr_in` and `hostent` structures
- `bind`, `listen`, `accept`, `connect`
- `read`/`write` vs `sendto`/`recvfrom`
- Signal handling (`SIGCHLD`) and child processes (`fork`)
- Receive timeout (`SO_RCVTIMEO`)
- Byte order conversion (`htons`, `htonl`, `ntohs`)

---

## Environment

- **Language:** C (C99)
- **OS:** Linux / macOS (POSIX)
- **Compiler:** GCC
- **Headers:** `<sys/socket.h>`, `<netinet/in.h>`, `<netdb.h>`

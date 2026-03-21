# TCP & UDP Socket Programming in C

> Implémentation d'une communication client/serveur en C avec les sockets POSIX — comparaison des protocoles **TCP** (orienté connexion) et **UDP** (sans connexion).

---

## Description

Ce projet illustre les fondamentaux de la **programmation réseau en C** à travers deux architectures client/serveur :

| Protocole | Type | Fiabilité | Usage typique |
|-----------|------|-----------|---------------|
| **TCP** | Orienté connexion | Garantie de livraison | Web, email, transfert de fichiers |
| **UDP** | Sans connexion | Non garantie | Streaming, jeux en ligne, DNS |

### Fonctionnalités

- **Serveur TCP** : accepte des connexions entrantes, traite chaque client dans un **processus fils** (`fork`), répond au message reçu
- **Client TCP** : établit une connexion, envoie un message, reçoit la réponse (timeout de 5s)
- **Serveur UDP** : reçoit des datagrammes et renvoie un écho
- **Client UDP** : envoie un message sans établissement de connexion préalable

---

## Architecture

```
.
├── src/
│   ├── client_tcp.c      # Client TCP avec timeout
│   ├── serveur_tcp.c     # Serveur TCP multi-processus (fork)
│   ├── client_udp.c      # Client UDP
│   └── serveur_udp.c     # Serveur UDP avec écho
├── Makefile
└── README.md
```

---

## Compilation

### Avec Make (recommandé)

```bash
make          # Compile tous les exécutables
make clean    # Supprime les binaires
```

### Manuellement

```bash
gcc -Wall -Wextra -o client_tcp  src/client_tcp.c
gcc -Wall -Wextra -o serveur_tcp src/serveur_tcp.c
gcc -Wall -Wextra -o client_udp  src/client_udp.c
gcc -Wall -Wextra -o serveur_udp src/serveur_udp.c
```

---

## Utilisation

### TCP

**Terminal 1 — Lancer le serveur :**
```bash
./serveur_tcp
# Serveur TCP en attente sur le port 9600...
```

**Terminal 2 — Lancer le client :**
```bash
./client_tcp localhost
# Connecté au serveur sur le port 9600
# Entrez votre message : Bonjour !
# Serveur : Message bien reçu !
```

### UDP

**Terminal 1 — Lancer le serveur :**
```bash
./serveur_udp
# Serveur UDP en attente sur le port 9600...
```

**Terminal 2 — Lancer le client :**
```bash
./client_udp localhost
# Entrez votre message : Bonjour !
# Message envoyé avec succès.
```

> **Note :** Par défaut, le port utilisé est `9600`. Modifiez la constante `PORT` dans les fichiers source pour en changer.

---

## Concepts illustrés

### Cycle de vie d'un socket TCP (serveur)
```
socket() → bind() → listen() → accept() → read()/write() → close()
```

### Cycle de vie d'un socket UDP (serveur)
```
socket() → bind() → recvfrom()/sendto() → close()
```

### Gestion des processus zombies (TCP)
Le serveur TCP utilise `sigaction(SIGCHLD, ...)` avec `waitpid()` pour éviter l'accumulation de processus zombies lors du traitement concurrent des clients.

---

## Notions abordées

- Sockets POSIX (`AF_INET`, `SOCK_STREAM`, `SOCK_DGRAM`)
- Structures `sockaddr_in`, `hostent`
- Fonctions `bind`, `listen`, `accept`, `connect`
- Fonctions `send`/`recv` vs `sendto`/`recvfrom`
- Gestion des signaux (`SIGCHLD`) et processus fils (`fork`)
- Timeout sur réception (`SO_RCVTIMEO`)
- Conversion d'ordre d'octets (`htons`, `htonl`, `ntohl`)

---

## Environnement

- **Langage :** C (standard C99)
- **OS :** Linux / macOS (POSIX)
- **Compilateur :** GCC
- **Bibliothèques :** `<sys/socket.h>`, `<netinet/in.h>`, `<netdb.h>`

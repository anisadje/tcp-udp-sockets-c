/*
 * client_tcp.c — Client TCP
 *
 * Établit une connexion avec un serveur TCP, envoie un message saisi
 * par l'utilisateur et affiche la réponse reçue.
 *
 * Usage : ./client_tcp <hostname>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT    9600
#define BUF_SIZE 256

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage : %s <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 1. Création du socket TCP                                           */
    /* ------------------------------------------------------------------ */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Timeout de réception : 5 secondes (évite un blocage si le serveur
     * ne répond pas) */
    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* ------------------------------------------------------------------ */
    /* 2. Résolution du nom d'hôte                                        */
    /* ------------------------------------------------------------------ */
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Erreur : hôte « %s » introuvable.\n", argv[1]);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 3. Remplissage de la structure d'adresse du serveur                */
    /* ------------------------------------------------------------------ */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    /* ------------------------------------------------------------------ */
    /* 4. Connexion au serveur (3-way handshake TCP)                      */
    /* ------------------------------------------------------------------ */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Connecté au serveur %s sur le port %d.\n", argv[1], PORT);

    /* ------------------------------------------------------------------ */
    /* 5. Saisie et envoi du message                                       */
    /* ------------------------------------------------------------------ */
    char buffer[BUF_SIZE];
    printf("Entrez votre message : ");
    fflush(stdout);

    memset(buffer, 0, BUF_SIZE);
    if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
        close(sockfd);
        return 0;
    }

    ssize_t sent = write(sockfd, buffer, strlen(buffer));
    if (sent < 0) {
        perror("write");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 6. Réception de la réponse du serveur                              */
    /* ------------------------------------------------------------------ */
    memset(buffer, 0, BUF_SIZE);
    ssize_t n = read(sockfd, buffer, BUF_SIZE - 1);
    if (n < 0) {
        perror("read");
    } else if (n == 0) {
        printf("Le serveur a fermé la connexion.\n");
    } else {
        printf("Réponse : %s", buffer);
    }

    /* ------------------------------------------------------------------ */
    /* 7. Fermeture du socket                                              */
    /* ------------------------------------------------------------------ */
    close(sockfd);
    return 0;
}
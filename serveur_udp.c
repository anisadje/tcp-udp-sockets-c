/*
 * serveur_udp.c — Serveur UDP avec écho
 *
 * Reçoit des datagrammes UDP et renvoie chaque message à l'expéditeur
 * (comportement écho).  Contrairement à TCP, aucune connexion n'est
 * établie : chaque datagramme est traité indépendamment.
 *
 * Usage : ./serveur_udp
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT     9600
#define BUF_SIZE 1024

int main(void)
{
    /* ------------------------------------------------------------------ */
    /* 1. Création du socket UDP                                           */
    /* ------------------------------------------------------------------ */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 2. Configuration de l'adresse locale                               */
    /* ------------------------------------------------------------------ */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* ------------------------------------------------------------------ */
    /* 3. Association du socket au port local (bind)                      */
    /* ------------------------------------------------------------------ */
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur UDP en attente sur le port %d...\n", PORT);

    /* ------------------------------------------------------------------ */
    /* 4. Boucle de réception                                             */
    /* ------------------------------------------------------------------ */
    char buffer[BUF_SIZE];

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        memset(buffer, 0, BUF_SIZE);

        /* recvfrom() récupère le datagramme ET l'adresse de l'expéditeur */
        ssize_t n = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0,
                             (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        printf("[%s:%d] Message reçu (%zd oct.) : « %s »\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               n, buffer);

        /* Écho : renvoie le message à l'expéditeur */
        if (sendto(sockfd, buffer, (size_t)n, 0,
                   (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("sendto");
        }
    }

    close(sockfd);
    return 0;
}
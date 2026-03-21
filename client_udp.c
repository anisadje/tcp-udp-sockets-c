/*
 * client_udp.c — Client UDP
 *
 * Envoie un message saisi par l'utilisateur à un serveur UDP.
 * Contrairement à TCP, aucune connexion n'est établie : le datagramme
 * est envoyé directement avec sendto().
 *
 * Usage : ./client_udp <hostname>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT     9600
#define BUF_SIZE 256

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage : %s <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 1. Création du socket UDP                                           */
    /* ------------------------------------------------------------------ */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

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
    /* 4. Saisie du message                                               */
    /* ------------------------------------------------------------------ */
    char buffer[BUF_SIZE];
    printf("Entrez votre message : ");
    fflush(stdout);

    if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
        close(sockfd);
        return 0;
    }

    /* Suppression du '\n' final */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[--len] = '\0';
    }

    /* ------------------------------------------------------------------ */
    /* 5. Envoi du datagramme (sendto — pas de connexion préalable)       */
    /* ------------------------------------------------------------------ */
    ssize_t sent = sendto(sockfd, buffer, len, 0,
                          (struct sockaddr *)&server_addr,
                          sizeof(server_addr));
    if (sent < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Message envoyé (%zd octets).\n", sent);

    /* ------------------------------------------------------------------ */
    /* 6. Fermeture du socket                                              */
    /* ------------------------------------------------------------------ */
    close(sockfd);
    return 0;
}
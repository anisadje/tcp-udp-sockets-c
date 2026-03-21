/*
 * serveur_tcp.c — Serveur TCP multi-processus
 *
 * Écoute sur un port fixe, accepte les connexions entrantes et traite
 * chaque client dans un processus fils dédié (fork).
 * Les processus zombies sont nettoyés via SIGCHLD / waitpid.
 *
 * Usage : ./serveur_tcp
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT     9600
#define BUF_SIZE 256
#define BACKLOG  5

/* ------------------------------------------------------------------ */
/* Gestionnaire SIGCHLD : récupère les processus fils terminés        */
/* ------------------------------------------------------------------ */
static void reap_children(int sig)
{
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

/* ------------------------------------------------------------------ */
/* Traitement d'un client (exécuté dans le processus fils)            */
/* ------------------------------------------------------------------ */
static void handle_client(int newsockfd, struct sockaddr_in *client_addr)
{
    char buffer[BUF_SIZE];

    printf("[fils %d] Client connecté : %s:%d\n",
           getpid(),
           inet_ntoa(client_addr->sin_addr),
           ntohs(client_addr->sin_port));

    memset(buffer, 0, BUF_SIZE);
    ssize_t n = read(newsockfd, buffer, BUF_SIZE - 1);
    if (n > 0) {
        /* Supprime le '\n' final pour un affichage plus propre */
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("[fils %d] Message reçu : « %s »\n", getpid(), buffer);

        const char *response = "Serveur : message bien reçu !\n";
        if (write(newsockfd, response, strlen(response)) < 0)
            perror("write");
    } else if (n == 0) {
        printf("[fils %d] Connexion fermée par le client.\n", getpid());
    } else {
        perror("read");
    }
}

/* ------------------------------------------------------------------ */
/* Point d'entrée                                                      */
/* ------------------------------------------------------------------ */
int main(void)
{
    /* -- Gestion des zombies ---------------------------------------- */
    struct sigaction sa;
    sa.sa_handler = reap_children;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    /* -- Création du socket d'écoute --------------------------------- */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Permet de réutiliser le port immédiatement après un redémarrage */
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* -- Adresse locale ---------------------------------------------- */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, BACKLOG) < 0) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur TCP en attente sur le port %d...\n", PORT);

    /* -- Boucle principale ------------------------------------------- */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int newsockfd = accept(sockfd,
                               (struct sockaddr *)&client_addr,
                               &client_len);
        if (newsockfd < 0) {
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(newsockfd);
            continue;
        }

        if (pid == 0) {
            /* Processus fils */
            close(sockfd);
            handle_client(newsockfd, &client_addr);
            close(newsockfd);
            exit(EXIT_SUCCESS);
        }

        /* Processus père : ferme la copie du socket client */
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}
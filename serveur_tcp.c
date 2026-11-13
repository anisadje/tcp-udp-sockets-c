/*
 * server_tcp.c — Multi-process TCP server
 *
 * Listens on a fixed port, accepts incoming connections and handles
 * each client in a dedicated child process (fork).
 * Zombie processes are cleaned up with SIGCHLD / waitpid.
 *
 * Usage: ./server_tcp
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
/* SIGCHLD handler: reaps finished child processes                    */
/* ------------------------------------------------------------------ */
static void reap_children(int sig)
{
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

/* ------------------------------------------------------------------ */
/* Handles one client (runs in the child process)                     */
/* ------------------------------------------------------------------ */
static void handle_client(int newsockfd, struct sockaddr_in *client_addr)
{
    char buffer[BUF_SIZE];

    printf("[child %d] Client connected: %s:%d\n",
           getpid(),
           inet_ntoa(client_addr->sin_addr),
           ntohs(client_addr->sin_port));

    memset(buffer, 0, BUF_SIZE);
    ssize_t n = read(newsockfd, buffer, BUF_SIZE - 1);
    if (n > 0) {
        /* Remove the trailing '\n' for a cleaner output */
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("[child %d] Message received: \"%s\"\n", getpid(), buffer);

        const char *response = "Server: message received!\n";
        if (write(newsockfd, response, strlen(response)) < 0)
            perror("write");
    } else if (n == 0) {
        printf("[child %d] Connection closed by the client.\n", getpid());
    } else {
        perror("read");
    }
}

/* ------------------------------------------------------------------ */
/* Entry point                                                        */
/* ------------------------------------------------------------------ */
int main(void)
{
    /* -- Zombie handling --------------------------------------------- */
    struct sigaction sa;
    sa.sa_handler = reap_children;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);

    /* -- Listening socket -------------------------------------------- */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Allows the port to be reused right after a restart */
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* -- Local address ----------------------------------------------- */
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

    printf("TCP server listening on port %d...\n", PORT);

    /* -- Main loop --------------------------------------------------- */
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
            /* Child process */
            close(sockfd);
            handle_client(newsockfd, &client_addr);
            close(newsockfd);
            exit(EXIT_SUCCESS);
        }

        /* Parent process: closes its copy of the client socket */
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}

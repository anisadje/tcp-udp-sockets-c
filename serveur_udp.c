/*
 * server_udp.c — UDP echo server
 *
 * Receives UDP datagrams and sends each message back to its sender
 * (echo).  Unlike TCP, no connection is established: each datagram
 * is handled independently.
 *
 * Usage: ./server_udp
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
    /* 1. Create the UDP socket                                           */
    /* ------------------------------------------------------------------ */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 2. Set up the local address                                        */
    /* ------------------------------------------------------------------ */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* ------------------------------------------------------------------ */
    /* 3. Bind the socket to the local port                               */
    /* ------------------------------------------------------------------ */
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP server listening on port %d...\n", PORT);

    /* ------------------------------------------------------------------ */
    /* 4. Receive loop                                                    */
    /* ------------------------------------------------------------------ */
    char buffer[BUF_SIZE];

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        memset(buffer, 0, BUF_SIZE);

        /* recvfrom() gets the datagram AND the sender's address */
        ssize_t n = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0,
                             (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        printf("[%s:%d] Message received (%zd bytes): \"%s\"\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               n, buffer);

        /* Echo: send the message back to the sender */
        if (sendto(sockfd, buffer, (size_t)n, 0,
                   (struct sockaddr *)&client_addr, client_len) < 0) {
            perror("sendto");
        }
    }

    close(sockfd);
    return 0;
}

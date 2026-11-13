/*
 * client_udp.c — UDP client
 *
 * Sends a message typed by the user to a UDP server.
 * Unlike TCP, no connection is established: the datagram is sent
 * directly with sendto().
 *
 * Usage: ./client_udp <hostname>
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
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 1. Create the UDP socket                                           */
    /* ------------------------------------------------------------------ */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 2. Resolve the host name                                           */
    /* ------------------------------------------------------------------ */
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error: host \"%s\" not found.\n", argv[1]);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 3. Fill in the server address                                      */
    /* ------------------------------------------------------------------ */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    /* ------------------------------------------------------------------ */
    /* 4. Read the message                                                */
    /* ------------------------------------------------------------------ */
    char buffer[BUF_SIZE];
    printf("Enter your message: ");
    fflush(stdout);

    if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
        close(sockfd);
        return 0;
    }

    /* Remove the trailing '\n' */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[--len] = '\0';
    }

    /* ------------------------------------------------------------------ */
    /* 5. Send the datagram (sendto, no connection needed)                */
    /* ------------------------------------------------------------------ */
    ssize_t sent = sendto(sockfd, buffer, len, 0,
                          (struct sockaddr *)&server_addr,
                          sizeof(server_addr));
    if (sent < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Message sent (%zd bytes).\n", sent);

    /* ------------------------------------------------------------------ */
    /* 6. Close the socket                                                */
    /* ------------------------------------------------------------------ */
    close(sockfd);
    return 0;
}

/*
 * client_tcp.c — TCP client
 *
 * Connects to a TCP server, sends a message typed by the user
 * and prints the response.
 *
 * Usage: ./client_tcp <hostname>
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
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* ------------------------------------------------------------------ */
    /* 1. Create the TCP socket                                           */
    /* ------------------------------------------------------------------ */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Receive timeout: 5 seconds (avoids blocking forever if the server
     * does not answer) */
    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

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
    /* 4. Connect to the server (TCP 3-way handshake)                     */
    /* ------------------------------------------------------------------ */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server %s on port %d.\n", argv[1], PORT);

    /* ------------------------------------------------------------------ */
    /* 5. Read and send the message                                       */
    /* ------------------------------------------------------------------ */
    char buffer[BUF_SIZE];
    printf("Enter your message: ");
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
    /* 6. Receive the server response                                     */
    /* ------------------------------------------------------------------ */
    memset(buffer, 0, BUF_SIZE);
    ssize_t n = read(sockfd, buffer, BUF_SIZE - 1);
    if (n < 0) {
        perror("read");
    } else if (n == 0) {
        printf("The server closed the connection.\n");
    } else {
        printf("Response: %s", buffer);
    }

    /* ------------------------------------------------------------------ */
    /* 7. Close the socket                                                */
    /* ------------------------------------------------------------------ */
    close(sockfd);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_address = argv[1];
    int udp_socket;
    struct sockaddr_storage server_addr;
    char buffer[BUFFER_SIZE];
    socklen_t server_addr_len;

    // Create UDP socket
    udp_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow dual-stack (IPv4 and IPv6)
    int dual_stack = 0;
    if (setsockopt(udp_socket, IPPROTO_IPV6, IPV6_V6ONLY, &dual_stack, sizeof(dual_stack)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Resolve server address
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server_address, NULL, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Copy resolved address to server_addr
    memcpy(&server_addr, res->ai_addr, res->ai_addrlen);
    server_addr_len = res->ai_addrlen;
    freeaddrinfo(res);

    // Set server port for both IPv4 and IPv6
    if (server_addr.ss_family == AF_INET) {
        ((struct sockaddr_in *)&server_addr)->sin_port = htons(SERVER_PORT);
    } else if (server_addr.ss_family == AF_INET6) {
        ((struct sockaddr_in6 *)&server_addr)->sin6_port = htons(SERVER_PORT);
    }

    printf("Enter username to get the refuge summary: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

    // Send message to server
    if (sendto(udp_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, server_addr_len) == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    // Receive response from server
    int len = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&server_addr, &server_addr_len);
    if (len == -1) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    buffer[len] = '\0';

    printf("Server response: %s\n", buffer);

    close(udp_socket);
    return 0;
}


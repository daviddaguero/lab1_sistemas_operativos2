#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "cJSON.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void trim_whitespace(char *str);

int main() {
    // Variables
    int client_socket;
    struct sockaddr_in server_addr;
    char username[BUFFER_SIZE], password[BUFFER_SIZE], message[BUFFER_SIZE];
    int bytes_sent, bytes_received;

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating client socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address (localhost)
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // Prompt user for username and password
    printf("Enter username: ");
    fgets(username, BUFFER_SIZE, stdin);
    printf("Enter password: ");
    fgets(password, BUFFER_SIZE, stdin);

    // Trim whitespace characters    
    trim_whitespace(username); 
    trim_whitespace(password);

    // Send username and password to server
    if (send(client_socket, username, strlen(username), 0) == -1) {
        perror("Error sending username to server");
        exit(EXIT_FAILURE);
    }
    if (send(client_socket, password, strlen(password), 0) == -1) {
        perror("Error sending password to server");
        exit(EXIT_FAILURE);
    }

    // Check if username and password are ubuntu
    if (strcmp(username, "ubuntu") == 0 && strcmp(password, "ubuntu") == 0) {
        // Create cJSON object
        cJSON *root = cJSON_CreateObject();
        cJSON *food = cJSON_CreateObject();
        cJSON *medicine = cJSON_CreateObject();

        // Prompt user for values
        printf("Enter meat amount: ");
        fgets(message, BUFFER_SIZE, stdin);
        cJSON_AddNumberToObject(food, "meat", atoi(message));

        printf("Enter vegetables amount: ");
        fgets(message, BUFFER_SIZE, stdin);
        cJSON_AddNumberToObject(food, "vegetables", atoi(message));

        printf("Enter fruits amount: ");
        fgets(message, BUFFER_SIZE, stdin);
        cJSON_AddNumberToObject(food, "fruits", atoi(message));

        printf("Enter water amount: ");
        fgets(message, BUFFER_SIZE, stdin);
        cJSON_AddNumberToObject(food, "water", atoi(message));

        printf("Enter antibiotics amount: ");
        fgets(message, BUFFER_SIZE, stdin);
        cJSON_AddNumberToObject(medicine, "antibiotics", atoi(message));

        printf("Enter analgesics amount: ");
        fgets(message, BUFFER_SIZE, stdin);
        cJSON_AddNumberToObject(medicine, "analgesics", atoi(message));

        printf("Enter bandages amount: ");
        fgets(message, BUFFER_SIZE, stdin);
        cJSON_AddNumberToObject(medicine, "bandages", atoi(message));

        // Add objects to root
        cJSON_AddStringToObject(root, "hostname", "ubuntu");
        cJSON_AddItemToObject(root, "food", food);
        cJSON_AddItemToObject(root, "medicine", medicine);

        // Convert cJSON to string
        char *json_str = cJSON_Print(root);

        // Send JSON string to server
        if (send(client_socket, json_str, strlen(json_str), 0) == -1) {
            perror("Error sending JSON to server");
            exit(EXIT_FAILURE);
        }

        // Free cJSON object and JSON string
        cJSON_Delete(root);
        free(json_str);
    }

    // Receive confirmation from server
    if ((bytes_received = recv(client_socket, message, BUFFER_SIZE, 0)) == -1) {
        perror("Error receiving confirmation from server");
        exit(EXIT_FAILURE);
    }
    message[bytes_received] = '\0';
    printf("Server response: %s\n", message);

    // Close client socket
    close(client_socket);

    return 0;
}

// Trim whitespace characters from the end of a string
void trim_whitespace(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

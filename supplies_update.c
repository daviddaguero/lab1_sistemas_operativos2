#include "supplies_update.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "cJSON.h"

#define BUFFER_SIZE 1024

void supplies_update(int client_socket, cJSON* root) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Receive update from ubuntu client
    if ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) == -1) {
        perror("Error receiving update message from client");
        exit(EXIT_FAILURE);
    }

    printf("Update received:\n%s\n", buffer);

    // Parse JSON data received from client
    cJSON *json_data = cJSON_Parse(buffer);
    if (json_data == NULL) {
        perror("Error parsing JSON data from client");
        exit(EXIT_FAILURE);
    }

    // Extract food and medicine objects from JSON data
    cJSON *food = cJSON_GetObjectItem(json_data, "food");
    cJSON *medicine = cJSON_GetObjectItem(json_data, "medicine");

    // Update JSON root object with new food and medicine values
    cJSON *supplies = cJSON_GetObjectItem(root, "supplies");
    cJSON_ReplaceItemInObject(supplies, "food", food);
    cJSON_ReplaceItemInObject(supplies, "medicine", medicine);

    // Convert cJSON structure to a string
    char *updated_json_str = cJSON_Print(root);

    // Write updated JSON string to file
    FILE *file = fopen("refuge_summary.json", "w");
    if (file == NULL) {
        perror("Error opening JSON file");
        exit(EXIT_FAILURE);
    }
    flock(fileno(file), LOCK_EX); // Lock access to the file
    fputs(updated_json_str, file);
    flock(fileno(file), LOCK_UN); // Unlock access to the file
    fclose(file);

    // Free memory
    cJSON_Delete(json_data);
    free(updated_json_str);

    printf("Process %d: Updated supplies data\n", getpid());

    // Send confirmation to client
    if (send(client_socket, "Update received successfully.\n", 30, 0) == -1) {
        perror("Error sending confirmation to client");
        exit(EXIT_FAILURE);
    }
}


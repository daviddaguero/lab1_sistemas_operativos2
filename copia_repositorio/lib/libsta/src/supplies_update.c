#include "supplies_update.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "cJSON.h"

#define BUFFER_SIZE 1024

void supplies_update(int client_socket) {
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

    FILE *file = fopen("refuge_summary.json", "r+");
    if (file == NULL) {
        printf("Error opening refuge_summary.json\n");
        return;
    }

    // Lock access to the file
    flock(fileno(file), LOCK_EX);

    // Read JSON content from the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char *json_content = (char *)malloc(file_size + 1);
    fread(json_content, 1, file_size, file);

    // Parse JSON content
    cJSON *root = cJSON_Parse(json_content);
    free(json_content);

    // Update JSON root object with new food and medicine values
    cJSON *supplies = cJSON_GetObjectItem(root, "supplies");
    cJSON_ReplaceItemInObject(supplies, "food", food);
    cJSON_ReplaceItemInObject(supplies, "medicine", medicine);

    // Convert cJSON structure to a string
    char *updated_json_str = cJSON_Print(root);

    // Truncate the file and write updated JSON content
    fseek(file, 0, SEEK_SET);
    fwrite(updated_json_str, strlen(updated_json_str), 1, file);
    fflush(file);

    // Unlock access to the file
    flock(fileno(file), LOCK_UN);

    fclose(file);

    // Free memory
    cJSON_Delete(json_data);
    //free(updated_json_str);
    // cJSON_Delete(root);

    printf("Process %d: Updated supplies data\n", getpid());

    // Send confirmation to client
    if (send(client_socket, "Update received successfully.\n", 30, 0) == -1) {
        printf("Error sending confirmation to client");
        exit(EXIT_FAILURE);
    }
}


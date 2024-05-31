#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "emergency.h"
#include "cJSON.h"

int generate_random_time(int min_seconds, int max_seconds) {
    // Seed for random number generation
    srand(time(NULL));
    max_seconds = max_seconds - 1; // We need to decrease this value to do the calculation correctly.
    // Generate a random number within the range of min_seconds to max_seconds
    int random_time = (rand() % max_seconds) + min_seconds;
    return random_time;
}

int send_emergency_notification(int fd[]) {
    // Minimum and maximum time between emergencies
    int min_seconds = 30;
    int max_seconds = 120;
    int random_time = generate_random_time(min_seconds, max_seconds);
    sleep(random_time); // Simulate time between emergencies

    close(fd[0]); // Close the read end of the pipe in the child
    // Generate random message
    const char *messages[] = {"POWER OUTAGES", "OTHER CRITICAL SITUATIONS"};
    const int num_messages = sizeof(messages) / sizeof(messages[0]);
    const char *message = messages[rand() % num_messages];
    
    // Send message to parent through pipe
    ssize_t bytes_written = write(fd[1], message, strlen(message));
    if (bytes_written == -1) {
        perror("Error sending message to parent through pipe");
        return 1;
    }
    close(fd[1]); // Close the write end of the pipe in the child
    kill(getppid(), SIGUSR1); // Notify the server
    return 0;
}

int update_refuge_summary_emergency(const char *current_time, const char *emergency_message, cJSON* root) {
    // Update the emergency section in the JSON
    cJSON *emergency = cJSON_GetObjectItem(root, "emergency");
    cJSON_AddItemToObject(emergency, "last_keepalived", cJSON_CreateString(current_time));
    cJSON_AddItemToObject(emergency, "last_event", cJSON_CreateString(emergency_message));

    // Convert cJSON structure to a string
    char *updated_json_str = cJSON_Print(root);

    // Write updated JSON content to the file
    FILE *json_file = fopen("refuge_summary.json", "w");
    if (json_file == NULL) {
        perror("Error opening JSON file");
        return 1;
    }
    fputs(updated_json_str, json_file);
    fclose(json_file);

    // Clean up
    free(updated_json_str);
    return 0;
}

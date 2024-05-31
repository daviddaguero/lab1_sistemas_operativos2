#include <sys/file.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
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
    // close(fd[1]); // Close the write end of the pipe in the child
    kill(getppid(), SIGUSR1); // Notify the server
    return 0;
}

int update_refuge_summary_emergency(const char *current_time, const char *emergency_message) {
    FILE *file = fopen("refuge_summary.json", "r+");
    if (file == NULL) {
        printf("Error opening refuge_summary.json\n");
        return 1;
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
    if (root == NULL) {
        printf("Error parsing JSON content\n");
        fclose(file);
        flock(fileno(file), LOCK_UN);
        return 1;
    }

    // Update the emergency section in the JSON
    cJSON *emergency = cJSON_GetObjectItem(root, "emergency");
    if (emergency == NULL) {
        printf("Error finding emergency section in JSON\n");
        cJSON_Delete(root);
        fclose(file);
        flock(fileno(file), LOCK_UN);
        return 1;
    }

    // Delete the existing "last_event" item if it exists
    cJSON_DeleteItemFromObject(emergency, "last_event");

    // Create a new "last_event" item with the cleaned emergency message
    cJSON_AddItemToObject(emergency, "last_event", cJSON_CreateString(emergency_message));
    cJSON_ReplaceItemInObject(emergency, "last_keepalived", cJSON_CreateString(current_time));

    // Convert cJSON structure to a string
    char *updated_json_str = cJSON_Print(root);

    // Truncate the file and write updated JSON content
    fseek(file, 0, SEEK_SET);
    fwrite(updated_json_str, strlen(updated_json_str), 1, file);
    fflush(file);
    ftruncate(fileno(file), strlen(updated_json_str)); // Truncate the file to the new length

    // Unlock access to the file
    flock(fileno(file), LOCK_UN);

    fclose(file);

    // Clean up
    cJSON_Delete(root);
    free(updated_json_str);
    return 0;
}



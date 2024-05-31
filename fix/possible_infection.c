#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include "possible_infection.h"
#include "cJSON.h"

double read_temperature() {
    double min_temperature = 36.0; // Minimum temperature for a healthy person
    double max_temperature = 40.0; // Maximum temperature. This guy is probably infected
    srand(time(NULL)); // Seed to generate random temperaturesbased on the current time
    return min_temperature + ((double)rand() / RAND_MAX) * (max_temperature - min_temperature);
}

int check_infected(double temperature) {
    double treshold_infected = 37.5; // If the temperature exceeds this value, the person is infected 
    if(temperature > treshold_infected) {
        return 1;    
    } else {
        return 0;
    }
}

int increment_infected_people(char *current_entry) {
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

    // Determine the entry to update based on the detected infection
    const char *entry_to_update = current_entry;
    cJSON *entry_value = cJSON_GetObjectItemCaseSensitive(root->child, entry_to_update);
    if (entry_value == NULL) {
        printf("Error: Entry not found in JSON\n");
        cJSON_Delete(root);
        fclose(file);
        return 1;
    }

    // Increment the entry value
    int new_value = entry_value->valueint + 1;
    cJSON_SetIntValue(entry_value, new_value);

    // Convert cJSON structure back to string
    char *updated_json_str = cJSON_Print(root);

    // Truncate the file and write updated JSON content
    fseek(file, 0, SEEK_SET);
    fwrite(updated_json_str, strlen(updated_json_str), 1, file);
    fflush(file);

    // Clean up
    free(updated_json_str);
    cJSON_Delete(root);

    // Unlock access to the file
    flock(fileno(file), LOCK_UN);

    fclose(file);

    return 0;
}

int possible_infection_log(char *current_entry, double temperature, int infected, char *time_str) {
    // Open the log file
    FILE *log_file = fopen("infected.log", "a"); // File pointer for the log file
    if (log_file == NULL) {
        printf("The log file could not be open.\n");
        return 1;
    }
    if(infected) {
        fprintf(log_file, "%s, %s, %.2f°C. HERE QUARANTINE!!!\n", time_str, current_entry, temperature);
        //printf("%s, %s, %.2f°C. HERE QUARANTINE!!!\n", time_str, current_entry, temperature);
    } else {
        fprintf(log_file, "%s, %s, %.2f°C\n", time_str, current_entry, temperature);
        //printf("%s, %s, %.2f°C\n", time_str, current_entry, temperature);
    }
    fclose(log_file); // Close the log file before exiting
    return 0;
}

#ifndef EMERGENCY_LIB_H
#define EMERGENCY_LIB_H

#include <unistd.h>
#include <signal.h>
#include "cJSON.h"

/**
 * send_emergency_notification
 * 
 * This function simulates sending an emergency notification to a parent process through a pipe.
 * It generates a random time delay between emergencies and sends one of this messages to the parent process: "POWER OUTAGES", "OTHER CRITICAL SITUATIONS"
 * 
 * @param fd An array of file descriptors representing the pipe for communication with the parent process.
 * @return Returns 0 upon successful notification, or 1 if an error occurs during the notification process.
 */
int send_emergency_notification(int fd[]);

/**
 * generate_random_time
 * 
 * This function generates a random time delay between emergencies within a specified range.
 * 
 * @param min_seconds The minimum time delay between emergencies (in seconds).
 * @param max_seconds The maximum time delay between emergencies (in seconds).
 * @return The randomly generated time delay between emergencies.
 */
int generate_random_time(int min_seconds, int max_seconds);

/**
 * update_refuge_summary_emergency
 * 
 * This function updates the emergency section in the refuge summary JSON with the current time and emergency message.
 * It then writes the updated JSON content to the file "refuge_summary.json".
 * 
 * @param current_time The current time string to be added to the "last_keepalived" field in the emergency section.
 * @param emergency_message The message describing the emergency event to be added to the "last_event" field in the emergency section.
 * @param root The root cJSON object representing the refuge summary JSON.
 * @return Returns 0 upon successful update and writing to the JSON file, or 1 if an error occurs during the process.
 */
int update_refuge_summary_emergency(const char *current_time, const char *emergency_message, cJSON* root);

#endif /* EMERGENCY_LIB_H */


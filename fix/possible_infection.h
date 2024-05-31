#ifndef POSSIBLE_INFECTION_H
#define POSSIBLE_INFECTION_H

#pragma once

/**
 * read_temperature
 * 
 * This function generates a random temperature between 36 and 40 °C. It simulates temperatures read in the survivors.
 * 
 * @return The generated temperature.
 */
double read_temperature();

/**
 * check_infected
 * 
 * This function checks if a given temperature indicates an infection.
 * 
 * @param temperature The temperature to be checked.
 * @return 1 if the temperature indicates an infection, 0 otherwise.
 */
int check_infected(double temperature);

/**
 * increment_infected_people
 * 
 * This function increments the number of infected people for a specific entry in the "refuge_summary.json" file.
 * It locks the file to ensure safe concurrent access, reads and parses the JSON content, updates the specified entry,
 * and writes the updated JSON content back to the file.
 * 
 * @param current_entry The entry in the JSON file to be updated.
 * @return 0 on success, 1 on error (e.g., file opening failure, JSON parsing error, entry not found).
 */
int increment_infected_people(char *current_entry);

/**
 * possible_infection_log
 * 
 * This function logs the temperature readings of an entry to the "infected.log" file.
 * It writes a log entry with the current time, entry name and temperature read.
 * 
 * @param current_entry The entry being logged.
 * @param temperature The temperature of the entry.
 * @param infected The infection status (1 if infected, 0 otherwise).
 * @param time_str The current time as a string.
 * @return 0 on success, 1 if the log file could not be opened.
 */
int possible_infection_log(char *current_entry, double temperature, int infected, char *time_str);

#endif


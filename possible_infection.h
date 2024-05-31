#ifndef POSSIBLE_INFECTION_H
#define POSSIBLE_INFECTION_H

#pragma once

/**
 * read_temperature
 * 
 * This function generates a random temperature between 36 and 40 °C. It simulates temperatures read in the survivors
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

int increment_infected_people(char *current_entry);

int possible_infection_log(char *current_entry, double temperature, int infected, const char *date_format);

int possible_infection(const char *date_format);

#endif


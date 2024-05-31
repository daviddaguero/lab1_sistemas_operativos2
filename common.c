#include <time.h>

#define BUFFER_SIZE 1024

char* get_current_time_str(const char *date_format) {
    time_t current_time;
    struct tm *time_info;
    time(&current_time);
    time_info = localtime(&current_time);
    static char time_str[BUFFER_SIZE]; // Static to prevent memory from being lost when exiting the function
    strftime(time_str, sizeof(time_str), date_format, time_info);
    return time_str;
}

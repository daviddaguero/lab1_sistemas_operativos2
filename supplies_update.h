#ifndef SUPPLIES_UPDATE_H
#define SUPPLIES_UPDATE_H
#include "cJSON.h"
#include <sys/file.h>

void supplies_update(int client_socket, cJSON* root);

#endif


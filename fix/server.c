#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <errno.h>
#include "cJSON.h"
#include <sys/file.h>
#include "supplies_update.h"
#include <time.h>
#include "possible_infection.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "emergency.h"

// Define a structure to store client information
typedef struct {
    int client_socket; // File descriptor of the client socket
    pid_t child_pid;   // PID of the child process handling the client
} ClientInfo;

#define TCP_PORT 8080
#define UDP_PORT 12345
#define MAX_PENDING_CONNECTIONS 10
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024
#define MESSAGE_SIZE BUFFER_SIZE*2
#define TRUE 1
#define FALSE 0
#define SHM_SIZE sizeof(ClientInfo) * MAX_CLIENTS

// Function prototypes
int communication_with_client(int client_socket);
void possible_infection_handler();
void emergency_notification_handler();
void sigint_handler(int sig);
int create_refuge_summary();
char* get_current_time_str();
int supplies_update_log(const char *protocol);
int connection_log(const char *protocol, const char *type);
void sigusr1_handler(int sig);
void end_gracefully();
void udp_connections();
int infected_log(char *time_str, char *current_entry);
void set_signal_handlers();
ClientInfo* initialize_clients_array();
int config_tcp_socket(struct sockaddr_in6 server_addr);
int save_client_socket_in_clients_array(int client_socket);
void max_clients_reached();
void remove_client();

int server_socket, shmid;
int fd[2]; // For pipe communication
const char *date_format = "%Y-%m-%d %H:%M:%S";
ClientInfo *clients; // Array to save the file descriptors for each client connected and the children's pid that is serving that client
sem_t *mutex; // Semaphore to control access to shared memory
char *semaphore_name;
int pid_emergency, pid_udp, udp_socket, pid, client_socket;

int main() {
    // Variables
    int pid_possible_infection;
    struct sockaddr_in6 server_addr;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if(create_refuge_summary()) {
        perror("Error creating the refuge summary");
        end_gracefully();
        exit(EXIT_FAILURE);
    }    

    set_signal_handlers();
  
    printf("Server starting...\n");

    // Create and initialize the semaphore
    semaphore_name = "/my_semaphore";
    mutex = sem_open(semaphore_name, O_CREAT | O_EXCL, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        end_gracefully();
        exit(EXIT_FAILURE);
    }

    pid_possible_infection = fork();
    if(pid_possible_infection == 0) { //Child process to execute possible_infection module
        printf("Process %d: Module possible_infection\n", getpid());
        while(1) {
            char *entries[] = {"north_entry", "east_entry", "west_entry", "south_entry"};
            int amount_of_entries = 4;
            char *current_entry;
            double temperature;
            int infected;
            for(int i=0; i<amount_of_entries; i++) {
                current_entry = entries[i];
                temperature = read_temperature();
                infected = check_infected(temperature);
                char *time_str = get_current_time_str(date_format);    
                if(possible_infection_log(current_entry, temperature, infected, time_str)) {
                    perror("Process %d: Module possible_infection. Error writing the log");
                    end_gracefully();
                    exit(EXIT_FAILURE);
                }
                if(infected) {
                    // printf("Process %d: Infected person detected with %.2f°C in %s\n", getpid(), temperature, current_entry);
                    if(increment_infected_people(current_entry)) {
                        perror("Error incrementing the number of infected people in the refuge summary");
                        end_gracefully();
                        exit(EXIT_FAILURE);
                    }
                    if(infected_log(time_str, current_entry)) {
                        perror("Error writing the refuge log with the new infected");
                        end_gracefully();
                        exit(EXIT_FAILURE);
                    }
                }
                sleep(5); //Wait to simulate temperature readings. It doesn't happen instantly
            }
        }
        return 0;
    }

    // Create the pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        end_gracefully();
        exit(EXIT_FAILURE);
    }

    pid_emergency = fork();
    if(pid_emergency == 0) { //Child process
        // Funciones de emergency
        printf("Process %d: Module emergency\n", getpid());
        while(1) {
            if(send_emergency_notification(fd)) {
                // error
                end_gracefully();
                exit(EXIT_FAILURE);
            }
        }
        return 0;
    }

    // Fork to manage udp connections
    pid_udp = fork();
    if (pid_udp == 0) { // Child process for udp communications
        printf("Process %d: UDP connections\n", getpid());
        udp_connections();
        exit(EXIT_SUCCESS);
    } else if (pid_udp < 0) {
        perror("Error forking UDP process");
        end_gracefully();
        exit(EXIT_FAILURE);
    }  

    clients = initialize_clients_array();

    if(config_tcp_socket(server_addr)) {
        end_gracefully();
        exit(EXIT_FAILURE);    
    }

    // Main server loop
    while (1) {
        printf("MAIN SERVER PROCESS %d: Waiting for connections\n", getpid());
        printf("MAIN SERVER PROCESS %d: Press CTRL + C to quit the server\n", getpid());

        // Accept incoming connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            end_gracefully();
            exit(EXIT_FAILURE);
        }
        int position_clients_array = save_client_socket_in_clients_array(client_socket);
        if (position_clients_array == MAX_CLIENTS) {
            // Maximum amount of clients reached
            max_clients_reached();
            continue; // Skip forking and continue with the next iteration
        } 
               
        // Fork to handle client communication
        pid = fork();
        if (pid == 0) { // Child process
            close(server_socket); // Close unused server socket in child process
            clients[position_clients_array].child_pid = getpid();
            printf("Process %d: I saved my pid in the position %d of the clients array. Now clients[%d].child_pid = %d\n", getpid(), position_clients_array, position_clients_array, clients[position_clients_array].child_pid);
            if(communication_with_client(client_socket)) {
                perror("Error in communication_with_client\n");
                end_gracefully();
                exit(EXIT_FAILURE);          
            }
            // Search the client file descriptor in the clients array
            printf("Process %d: Searching the file descriptor of the client %d in the clients array\n", getpid(), client_socket);
            remove_client();
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("Error forking process");
            end_gracefully();
            exit(EXIT_FAILURE);
        } else { // Parent process
            //wait(NULL); // Wait for child process to terminate
        }
    }
    return 0;
}

// Function to manage communication with a client
int communication_with_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char username[BUFFER_SIZE], password[BUFFER_SIZE];
    int bytes_received;
    char *protocol = "TCP";

    printf("Process %d: Communication started\n", getpid());
    if(connection_log(protocol, "new")) {
        return 1;
    }
    printf("Process %d: Waiting for username and password\n", getpid());

    // Receive JSON data from client
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving JSON data from client");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received data
    printf("Process %d: client data received: %s\n", getpid(), buffer);

    // Parse JSON data to extract username and password
    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL) {
        perror("Error parsing JSON data");
        end_gracefully();
        exit(EXIT_FAILURE);
    }

    cJSON *username_json = cJSON_GetObjectItem(root, "username");
    cJSON *password_json = cJSON_GetObjectItem(root, "password");

    if (username_json == NULL || password_json == NULL || !cJSON_IsString(username_json) || !cJSON_IsString(password_json)) {
        perror("Error extracting username or password from JSON data");
        cJSON_Delete(root);
        end_gracefully();
        exit(EXIT_FAILURE);
    }

    strcpy(username, username_json->valuestring);
    strcpy(password, password_json->valuestring);

    // printf("Process %d: Username received: %s\n", getpid(), username);
    // printf("Process %d: Password received: %s\n", getpid(), password);

    cJSON_Delete(root);

    // Check if username and password are ubuntu
    if (strcmp(username, "ubuntu") == 0 && strcmp(password, "ubuntu") == 0) {
        printf("Process %d: Ubuntu client identified\n", getpid());
        // Call function to handle supplies update
        supplies_update(client_socket);
        supplies_update_log(protocol);
    } else {
        printf("Process %d: Normal client. Sending JSON...\n", getpid());
        // Send JSON file to client
        FILE *json_file = fopen("refuge_summary.json", "r");
        if (json_file == NULL) {
            perror("Error opening JSON file");
            end_gracefully();
            exit(EXIT_FAILURE);
        }
        fseek(json_file, 0, SEEK_END);
        long file_size = ftell(json_file);
        rewind(json_file);

        // Allocate memory for file content
        char *json_content = (char *)malloc(file_size + 1);
        if (json_content == NULL) {
            perror("Error allocating memory for JSON content");
            end_gracefully();
            exit(EXIT_FAILURE);
        }
        // Read file content into buffer
        if (fread(json_content, 1, file_size, json_file) != file_size) {
            perror("Error reading JSON file");
            end_gracefully();
            exit(EXIT_FAILURE);
        }
        fclose(json_file);

        // Send JSON content to client
        if (send(client_socket, json_content, file_size, 0) == -1) {
            perror("Error sending JSON to client");
            end_gracefully();
            exit(EXIT_FAILURE);
        }
        printf("Process %d: JSON sent to client. Finishing...\n", getpid());

        // Free allocated memory
        free(json_content);
    }
    // Close client socket
    close(client_socket);
    if(connection_log(protocol, "close")) {
        return 1;
    }
    return 0;
}

// Signal handler for SIGINT (Control+C)
void sigint_handler(int sig) {
    printf("\nProcess %d: Caught SIGINT signal. Exiting...\n", getpid());
    end_gracefully();
}

int create_refuge_summary() {
    // printf("I am process: %d and I will write the file\n", getpid());
    // Open the file in write mode
    FILE *file = fopen("refuge_summary.json", "w");
    if (file == NULL) {
        printf("The file could not be open.\n");
        end_gracefully();
        return 1;
    }
    // Block acces to the file
    flock(fileno(file), LOCK_EX);

    // Create cJSON objects for each section
    cJSON *root = cJSON_CreateObject();
    cJSON *alerts = cJSON_CreateObject();
    cJSON *north_entry = cJSON_CreateNumber(0);
    cJSON *east_entry = cJSON_CreateNumber(0);
    cJSON *west_entry = cJSON_CreateNumber(0);
    cJSON *south_entry = cJSON_CreateNumber(0);
    cJSON *supplies = cJSON_CreateObject();
    cJSON *food = cJSON_CreateObject();
    cJSON *medicine = cJSON_CreateObject();
    cJSON *emergency = cJSON_CreateObject();
    cJSON *last_keepalived = cJSON_CreateObject();
    cJSON *last_event = cJSON_CreateObject();

    // Add the entries to the alerts object
    cJSON_AddItemToObject(alerts, "north_entry", north_entry);
    cJSON_AddItemToObject(alerts, "east_entry", east_entry);
    cJSON_AddItemToObject(alerts, "west_entry", west_entry);
    cJSON_AddItemToObject(alerts, "south_entry", south_entry);

    // Add food object and medicine object to the supplies object
    cJSON_AddItemToObject(supplies, "food", food);
    cJSON_AddItemToObject(supplies, "medicine", medicine);

    // Add alerts object, supplies object and emergency object to the root object
    cJSON_AddItemToObject(root, "alerts", alerts);
    cJSON_AddItemToObject(root, "supplies", supplies);
    cJSON_AddItemToObject(root, "emergency", emergency);

    cJSON_AddItemToObject(emergency, "last_keepalived", last_keepalived);
    cJSON_AddItemToObject(emergency, "last_event", last_event);

    // Convert cJSON structure to a string
    char *json_str = cJSON_Print(root);
    if (file) {
        // Check if the file is empty
        fseek(file, 0, SEEK_END);
        if (ftell(file) == 0) {
            fputs(json_str, file);
        }
    }
    fclose(file);

    // Unlock access to the file
    flock(fileno(file), LOCK_UN);

    cJSON_Delete(root);
    free(json_str);
}

int supplies_update_log(const char *protocol) {
    char *time_str = get_current_time_str(date_format);    
    // Open the log file
    FILE *log_file = fopen("refuge.log", "a");
    if (log_file == NULL) {
        printf("The log file could not be open.\n");
        return 1;
    }
    fprintf(log_file, "%s, Update of supplies from client (%s)\n", time_str, protocol);
    return 0;
}

char* get_current_time_str() {
    time_t current_time;
    struct tm *time_info;
    time(&current_time);
    time_info = localtime(&current_time);
    static char time_str[BUFFER_SIZE]; // Static to prevent memory from being lost when exiting the function
    strftime(time_str, sizeof(time_str), date_format, time_info);
    return time_str;
}

int connection_log(const char *protocol, const char *type) {
    char *time_str = get_current_time_str(date_format);

    FILE *log_file = fopen("refuge.log", "a");
    if (log_file == NULL) {
        printf("The log file could not be open.\n");
        return 1;
    }

    if (strcmp(type, "new") == 0) {
        fprintf(log_file, "%s, Connection from new client (%s)\n", time_str, protocol);
    } else if (strcmp(type, "close") == 0) {
        fprintf(log_file, "%s, Connection close from client (%s)\n", time_str, protocol);
    } else {
        printf("Invalid type. Expected new or close. New for new connections and close for closed connections");
        fclose(log_file);
        return 1;
    }

    fclose(log_file);
    return 0;
}

void sigusr1_handler(int sig) {
    char buffer[BUFFER_SIZE];
    close(fd[1]); // Close the write end of the pipe in the parent
    // Receive message from child
    ssize_t bytes_received = read(fd[0], buffer, BUFFER_SIZE);
    if (bytes_received == -1) {
        perror("Error receiving message from child");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0'; // Null-terminate the received message
    printf("MAIN SERVER PROCESS %d: Received emergency message: %s\n", getpid(), buffer);
    
    // Close connections with clients and send message to clients
    char emergency_message[MESSAGE_SIZE];
    sprintf(emergency_message, "EMERGENCY!!! %s\n", buffer);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].client_socket > 0) {
            send(clients[i].client_socket, emergency_message, strlen(emergency_message), 0);
            printf("Sending emergency message to client %d\n", clients[i].client_socket);
            clients[i].client_socket = 0;
            clients[i].child_pid = 0;
            printf("MAIN SERVER PROCESS %d: Now clients[%d].client_socket = %d and clients[%d].child_pid = %d\n", getpid(), i, clients[i].client_socket, i,
                    clients[i].child_pid);
            close(clients[i].client_socket); // Close the connection with the client in the parent
            // if(connection_log(protocol, "close")) {
                // exit(EXIT_FAILURE);
            // }
        }
    }

    // Write to refuge.log
    char *time_str = get_current_time_str(date_format);
    FILE *log_file = fopen("refuge.log", "a");
    if (log_file == NULL) {
        perror("The log file could not be open.");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "EMERGENCY!!! %s, %s. Emergency notification sent to all connected clients.\n", time_str, buffer);
    fclose(log_file);

    if(update_refuge_summary_emergency(time_str, emergency_message)) {
        perror("Error updating the refuge summary.");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
}

void end_gracefully() {
    // Detach the shared memory segment
    shmdt(clients);
    // Remove the shared memory segment
    shmctl(shmid, IPC_RMID, NULL);
    // Close the semaphore
    sem_close(mutex);
    sem_unlink(semaphore_name);
    
    if(pid_emergency == 0) {
        close(fd[1]); // Close the write end of the pipe in the child
    }
    if(pid_udp == 0) {
        close(udp_socket);
    }
    if(pid == 0) {
        close(client_socket); // close client socket in the children
    } else {
        close(server_socket); // close server socket in the parent
    }
    exit(EXIT_SUCCESS);
}

void udp_connections() {
    struct sockaddr_in6 server_addr;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create UDP socket
    udp_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        perror("Error creating UDP socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure for IPv6
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any; // Bind to any available IPv6 address
    server_addr.sin6_port = htons(UDP_PORT);

    // Bind UDP socket to the specified address and port
    if (bind(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding UDP socket");
        exit(EXIT_FAILURE);
    }

    // printf("Process %d: UDP Server started. Waiting for UDP connections...\n", getpid());

    // Main UDP server loop
    while (1) {
        // Receive message from client
        int bytes_received = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytes_received == -1) {
            perror("Error receiving UDP message");
            continue;
        }

        if(connection_log("UDP", "new")) {
            perror("Error writing the log (new udp connection)");
            continue;
        }

        // Convert client address to string
        char client_ip[INET6_ADDRSTRLEN];
        if (client_addr.ss_family == AF_INET6) {
            struct sockaddr_in6 *addr_ipv6 = (struct sockaddr_in6 *)&client_addr;
            inet_ntop(AF_INET6, &(addr_ipv6->sin6_addr), client_ip, INET6_ADDRSTRLEN);
        } else if (client_addr.ss_family == AF_INET) {
            struct sockaddr_in *addr_ipv4 = (struct sockaddr_in *)&client_addr;
            inet_ntop(AF_INET, &(addr_ipv4->sin_addr), client_ip, INET_ADDRSTRLEN);
        } else {
            strcpy(client_ip, "Unknown");
        }

        // Print received message
        buffer[bytes_received] = '\0';
        printf("Process %d: Received username from UDP client: %s\n", getpid(), buffer);

        // Enviar el resumen del refugio al cliente
        FILE *json_file = fopen("refuge_summary.json", "r");
        if (json_file == NULL) {
            perror("Error opening JSON file");
            continue; // No se puede abrir el archivo JSON, continuar con la siguiente iteración del bucle
        }

        // Leer el contenido del archivo JSON en un búfer
        fseek(json_file, 0, SEEK_END);
        long file_size = ftell(json_file);
        rewind(json_file);
        if (fread(buffer, 1, file_size, json_file) != file_size) {
            perror("Error reading JSON file");
            fclose(json_file);
            continue; // Error al leer el archivo JSON, continuar con la siguiente iteración del bucle
        }
        fclose(json_file);

        // Enviar el contenido JSON al cliente
        if (sendto(udp_socket, buffer, file_size, 0, (struct sockaddr *)&client_addr, client_addr_len) == -1) {
            perror("Error sending JSON to client");
            continue; // Error al enviar el JSON al cliente, continuar con la siguiente iteración del bucle
        }
        printf("Sent JSON to client %s\n", client_ip);
    }
}

int infected_log(char *time_str, char *current_entry) {
    // Open the log file
    FILE *log_file = fopen("refuge.log", "a"); // File pointer for the log file
    if (log_file == NULL) {
        printf("The log file could not be open.\n");
        return 1;
    }
    fprintf(log_file, "%s, Alert of possible infection in %s.\n", time_str, current_entry);
    fclose(log_file); // Close the log file before exiting
    return 0;
}

void set_signal_handlers() {
    // Set up signal handler for SIGINT (Control+C)
    signal(SIGINT, sigint_handler);
    // Set up signal handler for SIGUSR1
    signal(SIGUSR1, sigusr1_handler);
}

ClientInfo* initialize_clients_array() {
    // I use shared memory because I need that the parent and the childreen have access to the same clients array
    // Create a shared memory identifier
    key_t key = ftok("shmfile",65);
    // Create a shared memory segment
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    // Attach the shared memory segment
    clients = (ClientInfo*) shmat(shmid, NULL, 0);

    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].client_socket = 0; // Indicates a free space in the array
        clients[i].child_pid = 0;
    }
    return clients;
}

int config_tcp_socket(struct sockaddr_in6 server_addr) {
    // Create server socket for IPv6
    server_socket = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating server socket");
        return 1;
    }

    // This will allow us to reutilize the address when the program ends
    int opt = 1; // enabled
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Error setting SO_REUSEADDR option");
        return 1;
    }

    opt = 0;
    // Allow the socket to accept both IPv4 and IPv6 connections
    if (setsockopt(server_socket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) == -1) {
        perror("Error setting IPV6_V6ONLY option");
        return 1;
    }

    // Initialize server address structure for IPv6
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any; // Bind to any available IPv6 address
    server_addr.sin6_port = htons(TCP_PORT);

    // Bind server socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding server socket");
        return 1;
    }
    // Listen for incoming connections
    if (listen(server_socket, MAX_PENDING_CONNECTIONS) == -1) {
        perror("Error listening for connections");
        return 1;
    }
    return 0;
}

int save_client_socket_in_clients_array(int client_socket) {
    int i;
    // Search for an empty space in the clients array
    for (i = 0; i < MAX_CLIENTS; i++) {
        // Access the clients array within the critical region protected by the semaphore
        sem_wait(mutex);
        printf("MAIN SERVER PROCESS %d: clients[%d].client_socket = %d and clients[%d].child_pid = %d\n", getpid(), i, clients[i].client_socket, i,
                clients[i].child_pid);
       if (clients[i].client_socket == 0) {
            clients[i].client_socket = client_socket;
            printf("MAIN SERVER PROCESS %d: Client socket %d saved in position %d: clients[%d].client_socket = %d\n", getpid(), client_socket, i, i,           
                        clients[i].client_socket);
            sem_post(mutex);                
            return i;
       }
       sem_post(mutex);
    }
    return i;
}

void max_clients_reached() {
    int bytes_received;
    char aux_buffer[BUFFER_SIZE]; // Aux buffer to receive the username and password from the client before sending the message
    bytes_received = recv(client_socket, aux_buffer, BUFFER_SIZE, 0); // Receive the username and the password
    char *max_clients_msg = "Maximum number of clients reached. Please try again later.\n";
    send(client_socket, max_clients_msg, strlen(max_clients_msg), 0);
    close(client_socket);
}

void remove_client() {
    for (int i=0; i<MAX_CLIENTS;i++) {
        sem_wait(mutex);
        if(clients[i].client_socket == client_socket) {
            printf("Process %d: I found the client %d in the position %d. clients[%d].client_socket = %d\n", getpid(), client_socket, i, i,
                            clients[i].client_socket);
            printf("Process %d: The pid of the child that served this client is: %d\n", getpid(), clients[i].child_pid);
            clients[i].client_socket = 0; // Delete the client socket to make this space available for another client 
            clients[i].child_pid = 0; // Delete the child's pid that was serving this client
            printf("Process %d: Now clients[%d].client_socket = %d and clients[%d].child_pid = %d\n", getpid(), i, clients[i].client_socket, i,
                            clients[i].child_pid);
            sem_post(mutex);
            break;               
        }
        sem_post(mutex);
    }
}

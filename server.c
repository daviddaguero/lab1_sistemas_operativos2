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
#include "common.h"

// Define a structure to store client information
typedef struct {
    int client_socket; // File descriptor of the client socket
    pid_t child_pid;   // PID of the child process handling the client
} ClientInfo;

#define PORT 8080
#define MAX_PENDING_CONNECTIONS 10
#define MAX_CLIENTS 100
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
cJSON* create_refuge_summary(FILE *file);
int supplies_update_log(const char *protocol);
int connection_log(const char *protocol, const char *type);
void sigusr1_handler(int sig);
void end_gracefully();
void sigterm_handler(int sig);
//void free_space_children_array();

int server_socket, shmid;
FILE *file = NULL;
FILE *log_file = NULL; // File pointer for the log file
cJSON *root;
char *json_str;
int fd[2]; // For pipe communication
const char *date_format = "%Y-%m-%d %H:%M:%S";
ClientInfo *clients; // Array to save the file descriptors for each client connected and the children's pid that is serving that client
sem_t *mutex; // Semaphore to control access to shared memory
char *semaphore_name;
pid_t children[MAX_CLIENTS]; // Array to store child process IDs
int num_children = 0; // Counter for the number of child processes
char *protocol = "TCP";

int main() {
    // Variables
    int client_socket, pid, pid_emergency, pid_possible_infection;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    //socklen_t client_addr_len;

    // Open the file in write mode
    file = fopen("refuge_summary.json", "w");
    if (file == NULL) {
        printf("The file could not be open.\n");
        end_gracefully();
        return 1;
    }
    // Block acces to the file
    flock(fileno(file), LOCK_EX);
    root = create_refuge_summary(file);
    // Unlock access to the file
    flock(fileno(file), LOCK_UN);

    // Set up signal handler for SIGINT (Control+C)
    signal(SIGINT, sigint_handler);
    // Set up signal handler for SIGUSR1
    signal(SIGUSR1, sigusr1_handler);
    // Set up signal handler for SIGTERM in child processes
    signal(SIGTERM, sigterm_handler);
  
    printf("Server starting...\n");

    // Create and initialize the semaphore
    semaphore_name = "/my__semaphore";
    mutex = sem_open(semaphore_name, O_CREAT | O_EXCL, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        end_gracefully();
        exit(EXIT_FAILURE);
    }

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating server socket");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // This will allow us to reutilize the address when the program ends
    int opt = 1; // enabled
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Error setting SO_REUSEADDR option");
        end_gracefully();
        exit(EXIT_FAILURE);
    }

    // Bind server socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding server socket");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
    // Listen for incoming connections
    if (listen(server_socket, MAX_PENDING_CONNECTIONS) == -1) {
        perror("Error listening for connections");
        end_gracefully();
        exit(EXIT_FAILURE);
    }

    pid_possible_infection = fork();
    if(pid_possible_infection == 0) { //Child process to execute possible_infection module
        printf("Process %d: Module possible_infection\n", getpid());
        while(1) {
            if(possible_infection(date_format)) {
                perror("Error in module possible_infection");
                end_gracefully();
                exit(EXIT_FAILURE);
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
        int i;
        // Search for an empty space in the clients array
        for (i = 0; i < MAX_CLIENTS; i++) {
        // Access the clients array within the critical region protected by the semaphore
        sem_wait(mutex);
        // printf("MAIN SERVER PROCESS %d: clients[%d].client_socket = %d and clients[%d].child_pid = %d\n", getpid(), i, clients[i].client_socket, i,
                // clients[i].child_pid);
            if (clients[i].client_socket == 0) {
                clients[i].client_socket = client_socket;
                // printf("MAIN SERVER PROCESS %d: Client socket %d saved in position %d: clients[%d].client_socket = %d\n", getpid(), client_socket, i, i,           
                        // clients[i].client_socket);
                sem_post(mutex);                
                break;
            }
        sem_post(mutex);
        }
        if (i == MAX_CLIENTS) {
            // Maximum amount of clients reached
            int bytes_received;
            char aux_buffer[BUFFER_SIZE]; // Aux buffer to receive the username and password from the client before sending the message
            bytes_received = recv(client_socket, aux_buffer, BUFFER_SIZE, 0); // Receive the username
            bytes_received = recv(client_socket, aux_buffer, BUFFER_SIZE, 0); // Overwrite the username because I don't need it 
            char *max_clients_msg = "Maximum number of clients reached. Please try again later.\n";
            send(client_socket, max_clients_msg, strlen(max_clients_msg), 0);
            close(client_socket);
            continue; // Skip forking and continue with the next iteration
        }        

        // Fork to handle client communication
        pid = fork();
        if (pid == 0) { // Child process
            close(server_socket); // Close unused server socket in child process
            clients[i].child_pid = getpid();
            // printf("Process %d: I saved my pid in the position %d of the clients array. Now clients[%d].child_pid = %d\n", getpid(), i, i, clients[i].child_pid);
            if(communication_with_client(client_socket)) {
                perror("Error in communication_with_client\n");
                end_gracefully();
                exit(EXIT_FAILURE);          
            }
            // Search the client file descriptor in the clients array
            // printf("Process %d: Searching the file descriptor of the client %d in the clients array\n", getpid(), client_socket);
            for (int i=0; i<MAX_CLIENTS;i++) {
                sem_wait(mutex);
                if(clients[i].client_socket == client_socket) {
                    // printf("Process %d: I found the client %d in the position %d. clients[%d].client_socket = %d\n", getpid(), client_socket, i, i,
                            // clients[i].client_socket);
                    // printf("Process %d: The pid of the child that served this client is: %d\n", getpid(), clients[i].child_pid);
                    clients[i].client_socket = 0; // Delete the client socket to make this space available for another client 
                    clients[i].child_pid = 0; // Delete the child's pid that was serving this client
                    // printf("Process %d: Now clients[%d].client_socket = %d and clients[%d].child_pid = %d\n", getpid(), i, clients[i].client_socket, i,
                            // clients[i].child_pid);
                    sem_post(mutex);
                    break;               
                }
                sem_post(mutex);
            }
            //free_space_children_array();
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("Error forking process");
            end_gracefully();
            exit(EXIT_FAILURE);
        } else { // Parent process
            //children[num_children] = pid; // Save child PID in the array
            //num_children++;
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

    printf("Process %d: Communication started\n", getpid());
    if(connection_log(protocol, "new")) {
        return 1;
    }
    printf("Process %d: Waiting for username and password\n", getpid());

    // Receive username from client
    bytes_received = recv(client_socket, username, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving username from client");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
    username[bytes_received] = '\0'; // Null-terminate the received data
    printf("Process %d: Username received: %s\n", getpid(), username);

    // Receive password from client
    bytes_received = recv(client_socket, password, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Error receiving password from client");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
    password[bytes_received] = '\0'; // Null-terminate the received data
    printf("Process %d: Password received: %s\n", getpid(), password);

    // Check if username and password are ubuntu
    if (strcmp(username, "ubuntu") == 0 && strcmp(password, "ubuntu") == 0) {
        printf("Process %d: Ubuntu client identified\n", getpid());
        // Call function to handle supplies update
        supplies_update(client_socket, root);
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

cJSON* create_refuge_summary(FILE *file) {
    printf("I am process: %d and I will write the file\n", getpid());
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

    // Convert cJSON structure to a string
    json_str = cJSON_Print(root);
    if (file) {
        // Check if the file is empty
        fseek(file, 0, SEEK_END);
        if (ftell(file) == 0) {
            fputs(json_str, file);
        }
    }
    fclose(file);
    return root;
}

int supplies_update_log(const char *protocol) {
    char *time_str = get_current_time_str(date_format);    
    // Open the log file
    log_file = fopen("refuge.log", "a");
    if (log_file == NULL) {
        printf("The log file could not be open.\n");
        return 1;
    }
    fprintf(log_file, "%s, Update of supplies from client (%s)\n", time_str, protocol);
    return 0;
}

int connection_log(const char *protocol, const char *type) {
    char *time_str = get_current_time_str(date_format);    
    // Open the log file
    log_file = fopen("refuge.log", "a");
    if (log_file == NULL) {
        printf("The log file could not be open.\n");
        return 1;
    }
    if(strcmp(type, "new") == 0) {
        fprintf(log_file, "%s, Connection from new client (%s)\n", time_str, protocol);
        return 0;
    } else if(strcmp(type, "close") == 0){
        fprintf(log_file, "%s, Connection close from client (%s)\n", time_str, protocol);
        return 0;
    } else {
        printf("Invalid type. Expected new or close. New for new connections and close for closed connections");
        return 1;
    }
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
            close(clients[i].client_socket); // Close the connection with the client
            //kill(clients[i].child_pid, SIGTERM); // Send SIGTERM to each child process
            clients[i].client_socket = 0;
            clients[i].child_pid = 0;
            printf("MAIN SERVER PROCESS %d: Now clients[%d].client_socket = %d and clients[%d].child_pid = %d\n", getpid(), i, clients[i].client_socket, i,
                    clients[i].child_pid);
            if(connection_log(protocol, "close")) {
                exit(EXIT_FAILURE);
            }
        }
    }

    // Write to refuge.log
    char *time_str = get_current_time_str(date_format);
    log_file = fopen("refuge.log", "a");
    if (log_file == NULL) {
        perror("The log file could not be open.");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "EMERGENCY!!! %s, %s. Emergency notification sent to all connected clients.\n", time_str, buffer);
    fclose(log_file);

    if(update_refuge_summary_emergency(time_str, emergency_message, root)) {
        perror("Error updating the refuge summary.");
        end_gracefully();
        exit(EXIT_FAILURE);
    }
}

void end_gracefully() {
    close(server_socket); // Close server socket to release the port
    cJSON_Delete(root);
    free(json_str);
    // Detach the shared memory segment
    shmdt(clients);
    // Remove the shared memory segment
    shmctl(shmid, IPC_RMID, NULL);
    // Close the semaphore
    sem_close(mutex);
    sem_unlink(semaphore_name);
    exit(EXIT_SUCCESS);
}

// Function to handle SIGTERM signal in child processes
void sigterm_handler(int sig) {
    printf("Process %d received SIGTERM. Ending...\n", getpid());
    //close(clients[i].client_socket);
    end_gracefully();
    exit(EXIT_SUCCESS);
}


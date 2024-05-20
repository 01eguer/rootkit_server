#pragma once
#include "options.h"
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
// #include <arpa/inet.h>



// typedef struct {
//     struct in_addr ip_address;       // filter
//     uint16_t port;                        // filter
//     char command[32];                // command
//     char output[BUFFER_SIZE];        // output
// } ARHashTable;


typedef struct {
    uint32_t id;
    bool active;
    int fd_sender;

    uint32_t ip;    // FILTER
    uint32_t mask;  // FILTER
    uint16_t port;  // FILTER

    uint64_t data_size;
    uint64_t bytes_left;
} DSHashTable;

typedef struct {
    int fd[MAX_CLIENTS];
    int thread[MAX_CLIENTS];
    bool active[MAX_CLIENTS];
    bool blocked[MAX_ADMINS];
    time_t last_request_time;
    int request_count;
} FDClientsHashTable;

typedef struct {
    int fd[MAX_ADMINS];
    int thread[MAX_CLIENTS];
    bool active[MAX_ADMINS];
    bool blocked[MAX_ADMINS];
} FDAdminsHashTable;


extern FDClientsHashTable *clients[NUM_THREADS];
extern FDAdminsHashTable *admins[NUM_THREADS];
extern DSHashTable *data_streams[MAX_CLIENTS];

extern int client_count[NUM_THREADS];
extern pthread_mutex_t mutex_client_count;

// extern ARHashTable admin_request;
void client_data_manage(int *client_fd, char *buffer, int *bytes_received, int *thread_id);
void data_stream_route(char *buffer, int *thread_id);
void send_to_all_clients(uint32_t data_stream_id, char *buffer, int *thread_id, int offset);

void register_client(int fd, int thread);
void unregister_client(int fd, int thread );
void register_admin(int fd, int thread);
void unregister_admin(int fd, int thread);
bool admin_exists(int fd, int thread);

void register_data_stream(uint32_t id, int fd_sender, uint32_t ip, uint32_t mask, uint16_t port, uint64_t data_size);
void unregister_data_stream(uint32_t id);
void init_data_streams_hash_table();

void init_clients_hash_table(int thread);
void init_admins_hash_table(int thread);
int hash(int fd);




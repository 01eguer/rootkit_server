#include "poll/poll.h"
// #include "server/options.h"
// #include "net/socket.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "server/options.h"
#include "threading/threading.h"



FDClientsHashTable *clients[NUM_THREADS];
FDAdminsHashTable *admins[NUM_THREADS];
DSHashTable *data_streams[MAX_CLIENTS];

int client_count[NUM_THREADS] = {0};

bool thread_action_done[NUM_THREADS] = {false};

pthread_barrier_t barrier; // Declare a barrier variable
pthread_barrier_t barrier_msg;
pthread_mutex_t mutex_client_count = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_thread_action_done = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_accept = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_recv = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[NUM_THREADS];




void sigpipe_handler() {
    printf("ERROR: SIGPIPE\n");
}


int main(int argc, char *argv[]) {
    // ------------------------ TESTING ------------------------
    int opt;
    int port = -1; // default port number
    // Parsing command-line options
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'h':
                fprintf(stderr, "Usage: %s [-p|--port port]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    // If the port is not provided
    if (port == -1) {
        fprintf(stderr, "Port number is missing.\n");
        exit(EXIT_FAILURE);
    }
    // ---------------------------------------------------------

    // char buffer[BUFFER_SIZE];

    // Create server socket
    int server_fd = create_socket(port, "0.0.0.0");

    signal(SIGPIPE, sigpipe_handler);


    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    pthread_barrier_init(&barrier_msg, NULL, NUM_THREADS);

    pthread_mutex_init(&mutex_client_count, NULL);
    pthread_mutex_init(&mutex_thread_action_done, NULL);
    pthread_mutex_init(&mutex_accept, NULL);
    pthread_mutex_init(&mutex_recv, NULL);     


    // Allocate memory for HashMaps
    for (int i = 0; i < NUM_THREADS; ++i) {
        clients[i] = (FDClientsHashTable*)malloc(sizeof(FDClientsHashTable));
        if (clients[i] == NULL) {
            // Handle allocation failure
            return 1;
        }
        admins[i] = (FDAdminsHashTable*)malloc(sizeof(FDAdminsHashTable));
        if (admins[i] == NULL) {
            // Handle allocation failure
            return 1;
        }
    }
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        data_streams[i] = (DSHashTable*)malloc(sizeof(DSHashTable));
        if (data_streams[i] == NULL) {
            // Handle allocation failure
            return 1;
        }
    }



    char a;


    
    
    for (int i = 0; i < NUM_THREADS; i++){
        
        start_epoll_loop_thread(&server_fd, i);
        printf("thread %i started\n",i);
        usleep(100);
        // scanf(" %c", &a);
    }


    char input[255];
    while (1){
        printf("[console]> ");
        fgets(&input, sizeof(input),stdin);
        if (input[0] == '\n') {

        } else if (input[0] == 'c') {
            for(int i = 0; i < NUM_THREADS; i++){
                printf("clients in thread %i: %i\n", i, client_count[i]);
            }
        } else if (input[0] == 't'){
            int total_client_count = 0;
            for (int i = 0; i < NUM_THREADS; i++){
                total_client_count += client_count[i];
            }
            printf("Total clients: %i\n", total_client_count);
        }
        input[0] = '\0';
        // input[0] = '\0';
        usleep(100);
    }

    
    // Wait for threads to finish
    for (int thread_id = 0; thread_id < NUM_THREADS; thread_id++) {
        if (pthread_join(threads[thread_id], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    pthread_barrier_destroy(&barrier);
    pthread_barrier_destroy(&barrier_msg);

    pthread_mutex_destroy(&mutex_client_count);
    pthread_mutex_destroy(&mutex_thread_action_done);
    pthread_mutex_destroy(&mutex_accept);
    pthread_mutex_destroy(&mutex_recv);

    // stop poll
    // socket_epoll_close(&server_fd, &epoll_fd);
    
    // Free memory for HashMaps
    for (int i = 0; i < NUM_THREADS; ++i) {
        free(clients[i]);
        free(admins[i]);
    }
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        free(data_streams[i]);
    }
    return 0;
}









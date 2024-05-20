
#pragma once
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "../net/socket.h"
#include "../server/options.h"





// void *client_connection_manage_wrapper(void *args);
void *epoll_loop_wrapper(void *args);

void start_epoll_loop_thread(int *server_fd, int thread_id);
// void start_data_connection_manage_thread(int *client_fd, char * buffer, int * bytes_received);
// void start_client_data_manage_thread(int *server_fd, struct epoll_event *event, int *epoll_fd, int *events_count, struct epoll_event *events);

extern int client_count[NUM_THREADS];
extern pthread_t threads[NUM_THREADS];


extern pthread_barrier_t barrier;
extern pthread_barrier_t barrier_msg;
extern pthread_mutex_t mutex_client_count;
extern pthread_mutex_t mutex_thread_action_done;
extern pthread_mutex_t mutex_accept;
extern pthread_mutex_t mutex_recv;

extern bool thread_action_done[NUM_THREADS];

int count_threads_with_clients();
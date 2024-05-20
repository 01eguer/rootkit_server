
#include "threading.h"
#include <sys/epoll.h>
#include "../poll/poll.h"
// #include "../net/socket.h"


void *epoll_loop_wrapper(void *args) {
    int *server_fd = ((int **)args)[0];
    // struct epoll_event *event = ((struct epoll_event **)args)[1];
    // int *epoll_fd = ((int **)args)[2];
    // int *events_count = ((int **)args)[3];
    // struct epoll_event *events = ((struct epoll_event **)args)[4];
    int thread_id = (int)(intptr_t)(((void **)args)[1]);

    epoll_loop(server_fd, &thread_id);
    return NULL;
}


void start_epoll_loop_thread(int *server_fd, int thread_id){
    // Create thread for client_connection_manage_wrapper || MANAGE CLIENT CONNECTIONS
    // int thread_id = CLIENT_CONNECTION_MANAGE_THREAD;
    static void *args[2]; // if not set  to static variables are destroyed when exiting function
    args[0] = (void *)server_fd;
    // args[1] = (void *)event;
    // args[2] = (void *)epoll_fd;
    // args[3] = (void *)events_count;
    // args[4] = (void *)events;
    args[1] = (void *)(intptr_t)thread_id;
    
    if (pthread_create(&threads[thread_id], NULL, epoll_loop_wrapper, args) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
}



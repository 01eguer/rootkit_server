#pragma once
#include "../server/options.h"
#include "../server/server.h"
#include <sys/epoll.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <pthread.h>
// #include <sys/socket.h>
// #include <stdbool.h>
// #include <stdlib.h>


// ulimit -n 999999 

void socket_epoll_init(int *server_fd, struct epoll_event *event, int *epoll_fd, int *events_count, struct epoll_event *events);
void epoll_loop(int *server_fd, int *thread_id);
void socket_epoll_close(int *server_fd, int *epoll_fd);





// extern int all_fds[MAX_CLIENTS];

// socket_epoll_loop -> clients_manage_thread  | not using anymore global variables for client count
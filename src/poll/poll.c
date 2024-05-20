#include "poll.h"
// #include "../net/socket.h"
#include "../threading/threading.h"
#include "../server/server.h"
// #include "../server/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void socket_epoll_init(int *server_fd, struct epoll_event *event, int *epoll_fd, int *events_count, struct epoll_event *events){
    events = malloc(MAX_CLIENTS * sizeof(*events));
    if (events == NULL) { // if error allocating memory
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *epoll_fd = epoll_create1(0);
    if (*epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    
    // Add server socket to epoll
    event->events = EPOLLIN;
    event->data.fd = *server_fd;
    if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, *server_fd, event) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    // printf("Server listening ...\n");
}



int count_threads_with_clients() {
    int count = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (client_count[i] > 0) {
            count++;
        }
    }
    return count;
}


bool all_thread_action_done() {
    for (int i = 0; i < sizeof(thread_action_done); i++) {
        if (!thread_action_done[i]) {
            return false;
        }
    }
    return true;
}




void epoll_loop(int *server_fd, int *thread_id) {
    struct epoll_event event;
    int epoll_fd, events_count;
    struct epoll_event *events = malloc(MAX_EVENTS * sizeof(struct epoll_event));
    // init poll
    socket_epoll_init(server_fd, &event, &epoll_fd, &events_count, events);

    for (int j = 0; j < client_count[*thread_id]; j++){
        printf("events[%i].data.fd: %i\n", j, events[j].data.fd);
    }
    // printf("hello from worker%i\n", *thread_id);
    // printf("clients in %i: %i\n", thread_id, client_count[thread_id]);
    // int thread_id = 0;



    // printf("tid: %d\n", (int)*thread_id);
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int threads_with_clients_count;




    init_clients_hash_table(*thread_id);
    init_admins_hash_table(*thread_id);

    time_t current_time;


    while (1) {
        // usleep( 1000);
        // pthread_barrier_wait(&barrier);
        // pthread_barrier_wait(&barrier);
        
        // data_stream_route(buffer, thread_id, &client_addr, &client_len);
        // pid_t pid = fork();
        // if (pid < 0) {
        //     perror("fork");
        //     exit(EXIT_FAILURE);
        // } else if (pid == 0) {
        //     // Child process
        //     data_stream_route(buffer, thread_id, &client_addr, &client_len);
        //     exit(EXIT_SUCCESS); // Terminate the child process after executing data_stream_route
        // }

        // timeout.tv_sec = timeout_seconds;
        // timeout.tv_usec = 0;

        // if (setsockopt(events[i].data.fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        //     perror("setsockopt");
        //     exit(EXIT_FAILURE);
        // }




        client_fd = -1;
        pthread_barrier_wait(&barrier);
        // atomic_store(&connection_accepted, 0);
        events_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);
        // usleep(1000);
        if (events_count < 0){
            perror("epoll_wait");
        }

        // printf("out of barrier t%i\n",*thread_id);
        // pthread_barrier_wait(&barrier);
        for (int i = 0; i < events_count; ++i) {
            // New connection
            if (events[i].data.fd == *server_fd ) {
                // printf("tid: %i is trying to accept\n", *thread_id);
                // pthread_barrier_wait(&barrier);
                // pthread_mutex_lock(&mutex_accept);
                if (client_count[*thread_id] < MAX_CLIENTS) {

                    // printf("tid: %i is trying to accept\n", *thread_id);
                

                    pthread_mutex_lock(&mutex_accept);
                    client_fd = accept(events[i].data.fd, (struct sockaddr *)&client_addr, &client_len);
                    pthread_mutex_unlock(&mutex_accept);
       

                    if (client_fd < 0) {
                        close(client_fd);
                        // perror("accept");
                        continue;
                    }
                    
                // pthread_mutex_unlock(&mutex_accept);
                    
                } else{
                    continue;
                }

                if (client_count[*thread_id] < MAX_CLIENTS) {
                    printf("[+] New connection from %s:%d with fd %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);

                    // Add new client to epoll
                    event.events = EPOLLIN;
                    event.data.fd = client_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                        perror("epoll_ctl");
                        close(client_fd);
                        continue;
                    }
                    register_client(client_fd, *thread_id);
                    pthread_mutex_lock(&mutex_client_count);
                    client_count[*thread_id]++;
                    pthread_mutex_unlock(&mutex_client_count);
                    
                } else {
                    close(client_fd);
                    printf("[!] Client limit reached\n");
                }

                
            } else { // Data from existing connection
                // pthread_barrier_wait(&barrier);
                // printf("tid: %i is trying to recv\n", *thread_id);
                buffer[0] = '\0';
                pthread_mutex_unlock(&mutex_recv);
                bytes_received = recv(events[i].data.fd, buffer, BUFFER_SIZE, 0);
                pthread_mutex_unlock(&mutex_recv);
                if (bytes_received <= 0) {
                    if (bytes_received == 0) {
                        
                        // Close client connection
                        close(events[i].data.fd);

                        if (admin_exists(events[i].data.fd, *thread_id)){
                            unregister_admin(events[i].data.fd, *thread_id);
                            printf("[-] Admin disconnected\n");
                        } else{
                            pthread_mutex_lock(&mutex_client_count);
                            client_count[*thread_id]--;
                            pthread_mutex_unlock(&mutex_client_count);
                            unregister_client(events[i].data.fd, *thread_id);
                            printf("[-] Client disconnected\n");
                        }


                    } else {
                        perror("recv");
                    }
                } else {
                    client_data_manage(&events[i].data.fd, &buffer, &bytes_received, thread_id);
                    pthread_barrier_wait(&barrier);
                }
                
            }

        }



    }
}





// }



void socket_epoll_close(int *server_fd, int *epoll_fd){
    close(*server_fd);
    close(*epoll_fd);
}

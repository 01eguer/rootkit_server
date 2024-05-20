#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>




void encrypt_decrypt(char *message);
int create_socket(int port, char *ip);
// int accept_new_connection(int server_fd, int epoll_fd, struct epoll_event *event);


// will be called by clients_manage_thread
// void handle_client_data(int client_fd);


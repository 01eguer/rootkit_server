#include "socket.h"
// #include "../poll/poll.h"



int create_socket(int port, char *ip) {
    int server_fd;
    struct sockaddr_in server_addr;

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind socket to address
    server_addr.sin_family = AF_INET; // ipv4
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if (port == 0){  // If the port is 0 get the actual port number assigned by the OS
        socklen_t len = sizeof(server_addr);
        if (getsockname(server_fd, (struct sockaddr *)&server_addr, &len) == -1) {
            perror("getsockname");
            exit(EXIT_FAILURE);
        }

    }
    // Print IP:PORT where is running the server
    printf("[*] Socket bind in %s:%d\n", ip, ntohs(server_addr.sin_port));


    // int timeout_seconds = 1; // Set your desired timeout value in seconds

    // // Set socket timeout
    // struct timeval timeout;
    // timeout.tv_sec = timeout_seconds;
    // timeout.tv_usec = 0;

    // if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }


        int flags;

    if ((flags = fcntl(server_fd, F_GETFL, 0)) == -1)
        flags = 0;

    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    return server_fd;
}
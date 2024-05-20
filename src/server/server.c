#include "options.h"
#include "protocol.h"
#include "poll.h"
// #include "../net/socket.h"
// #include "../threading/threading.h"
#include "server.h"
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>
#include "../server/protocol.h"
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>
// #include <unistd.h>
// #include <time.h>




void init_clients_hash_table(int thread) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[thread]->fd[i] = -1; // Initialize file descriptor to -1 (unused)
        clients[thread]->active[i] = false; // Initialize active flag to false
    }
}

void init_admins_hash_table(int thread) {
    for (int i = 0; i < MAX_ADMINS; i++) {
        admins[thread]->fd[i] = -1; // Initialize file descriptor to -1 (unused)
        admins[thread]->active[i] = false; // Initialize active flag to false
    }
}

void init_data_streams_hash_table() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        data_streams[i]->id = 0;
        data_streams[i]->active = false;
    }
}

int hash(int fd) {
    return fd % MAX_CLIENTS;
}

void register_client(int fd, int thread) {
    int index = hash(fd);
    while (clients[thread]->active[index]) { // if in use
        index = (index + 1) % MAX_CLIENTS; // Linear probing
    }
    clients[thread]->fd[index] = fd;
    clients[thread]->active[index] = true;
}

void register_admin(int fd, int thread) {
    int index = hash(fd);
    while (admins[thread]->active[index]) { // if in use
        index = (index + 1) % MAX_ADMINS; // Linear probing
    }
    admins[thread]->fd[index] = fd;
    admins[thread]->active[index] = true;
}

bool admin_exists(int fd, int thread) {
    int index = hash(fd);
    while (admins[thread]->active[index]) {
        if (admins[thread]->fd[index] == fd) {
            return true; // Admin exists
        }
        index = (index + 1) % MAX_ADMINS; // Linear probing
    }
    return false;
}

void register_data_stream(uint32_t id, int fd_sender, uint32_t ip, uint32_t mask, uint16_t port, uint64_t data_size) {
    int index = hash(id);
    while (data_streams[index]->active) { // Check if in use
        index = (index + 1) % MAX_CLIENTS; // Linear probing
    }
    data_streams[index]->id = id;
    data_streams[index]->fd_sender = fd_sender;
    data_streams[index]->ip = ip; // destination
    data_streams[index]->mask = mask; // destination
    data_streams[index]->port = port; // destination
    data_streams[index]->active = true;
    data_streams[index]->bytes_left = data_size;
}

void unregister_client(int fd, int thread) {
    int index = hash(fd);
    while (clients[thread]->fd[index] != fd) {
        index = (index + 1) % MAX_CLIENTS; // Linear probing
    }
    clients[thread]->active[index] = false;
}

void unregister_admin(int fd, int thread) {
    int index = hash(fd);
    while (admins[thread]->fd[index] != fd) {
        index = (index + 1) % MAX_CLIENTS; // Linear probing
    }
    admins[thread]->active[index] = false;
}

void unregister_data_stream(uint32_t id) {
    int index = hash(id);
    while (data_streams[index]->active && data_streams[index]->id != id) { // Find the data stream
        index = (index + 1) % MAX_CLIENTS; // Linear probing
    }
    if (data_streams[index]->id == id) { // If found
        data_streams[index]->id = 0;
        data_streams[index]->fd_sender = 0;
        data_streams[index]->ip = INADDR_NONE; // destination
        data_streams[index]->mask = INADDR_ANY; // destination
        data_streams[index]->port = 0; // destination
        data_streams[index]->active = false;
        data_streams[index]->bytes_left = 0;
    }
}


void client_data_manage(int *client_fd, char *buffer, int *bytes_received, int *thread_id) {
    
    // printf("-> %i\n", buffer[BUFFER_SIZE]);
    char command = buffer[0];
    if (command == (char)COMMAND_SENDINGFROMCLIENT) {
        
        uint32_t data_stream_id;
        char data[BUFFER_SIZE-5];

        memset(data, 0, BUFFER_SIZE-5);
        decapsulate_transfer_data_cmd(buffer, (uint32_t *)&data_stream_id, data);

        int index = hash(data_stream_id);
        while (data_streams[index]->id != data_stream_id) { // Check if in use
            index = (index + 1) % MAX_CLIENTS; // Linear probing
        }
        
        encapsulate_transfer_data_cmd(buffer, command, *client_fd, data, 0);  // sends the fd because is unique and client can use it as an id to reorder the packets
        // --- sending TRANSFER_CMD from client to admin ---
        send(data_streams[index]->fd_sender, buffer, BUFFER_SIZE, 0);
        
    } else if (command == (char)STATUS_FROMADMINSENDED) {
        uint32_t data_stream_id = get_data_stream_id(buffer);
        printf("--- SENDING TO ALL CLIENTS\n");
        send_to_all_clients(data_stream_id, buffer, thread_id, 0); // TODO: this has to run in background

        
    } else if (command == (char)STATUS_FROMCLIENTSENDED) {
        uint32_t data_stream_id = get_data_stream_id(buffer);
        int index = hash(data_stream_id);
        while (data_streams[index]->id != data_stream_id) { // Check if in use
            index = (index + 1) % MAX_CLIENTS; // Linear probing
        }
        // --- sending TRANSFER_CMD from client to admin ---
        send(data_streams[index]->fd_sender, buffer, BUFFER_SIZE, 0);
        
    } else if (command == COMMAND_STARTRECEIVINGFROMCLIENT) {
        uint32_t data_stream_id = get_data_stream_id(buffer);
        int index = hash(data_stream_id);
        while (data_streams[index]->id != data_stream_id) { // Check if in use
            index = (index + 1) % MAX_CLIENTS; // Linear probing
        }
        // printf("data stream id: %i", data_streams[index]->id);
        // printf("admin fd: %i", data_streams[index]->fd_sender);
        // --- sending TRANSFER_CMD from client to admin ---
        send(data_streams[index]->fd_sender, buffer, 5, 0);
        // printf("sending TRANSFER_CMD from client to admin\n");s
    } else if (command == COMMAND_RECEIVINGFROMCLIENT) {
        uint32_t data_stream_id = get_data_stream_id(buffer);
        int index = hash(data_stream_id);
        while (data_streams[index]->id != data_stream_id) { // Check if in use
            index = (index + 1) % MAX_CLIENTS; // Linear probing
        }
        // --- sending TRANSFER_CMD from client to admin ---
        send(data_streams[index]->fd_sender, buffer, 5, 0);
        // printf("sending TRANSFER_CMD from client to admin\n");
        // printf("buffer ---> %s | strlen buffer: %lu\n", buffer, strlen(buffer));
        
    } else if (command == COMMAND_SENDINGFROMADMIN) {
        uint32_t data_stream_id = get_data_stream_id(buffer);
        int index = hash(data_stream_id);
        while (data_streams[index]->id != data_stream_id) { // Check if in use
            index = (index + 1) % MAX_CLIENTS; // Linear probing
        }

        if (data_streams[index]->bytes_left > 0) {
            int offset = data_streams[index]->data_size - data_streams[index]->data_size;
            if (data_streams[index]->bytes_left - BUFFER_SIZE-5 < 0) { // chunk of data can be smaller than data bytes space in protocol
                data_streams[index]->bytes_left = 0;
            } else {
                data_streams[index]->bytes_left -= BUFFER_SIZE-5;
            }
            // --- sending TRANSFER_DATA_CMD from server to all clients ---
            // printf("offset %i\n", offset);
            send_to_all_clients(data_stream_id, buffer, thread_id, offset); // TODO: this has to run in background
            // printf("buffer -> %s\n", &buffer[5]);
        }
        

    } else if (command == COMMAND_LOGIN) { // LOGIN
        if (check_token(buffer)){
            printf("[+] Admin logged in\n");
            send(*client_fd, (char[]){STATUS_CORRECTPASS}, 1, 0);
            unregister_client(*client_fd, *thread_id);
            pthread_mutex_lock(&mutex_client_count);
            client_count[*thread_id]--;
            pthread_mutex_unlock(&mutex_client_count);

            // register_client(*client_fd, *thread_id);
            register_admin(*client_fd, *thread_id);
        } else {
            printf("[!] Wrong password\n");
            send(*client_fd, (char[]){ERROR_INCORRECTPASS}, 1, 0);
        }
    } else if (command == COMMAND_PING) {
        send(*client_fd, (char[]){COMMAND_PING}, 1, 0);
    } else if (command == COMMAND_EXECCMD || 
    command == COMMAND_EXECBIN || 
    command == COMMAND_EXECASM || 
    command == COMMAND_DEVREAD || 
    command == COMMAND_DEVWRITE || 
    command == COMMAND_DEVSHOW || 
    command == COMMAND_FILEREAD || 
    command == COMMAND_FILEWRITE) {
        uint32_t ip;
        uint32_t mask;
        uint16_t port;
        bool output;
        uint64_t data_size;
        decapsulate_run_cmd_admin(buffer, &ip, &mask, &port, &output, &data_size);

        printf("ip: %d mask: %d port: %d output: %d bytes_left: %lu\n", ip, mask, port, output, data_size);
        // for (int i = 0; i < BUFFER_SIZE; i++){
        //     if (i == 0 || i == 1 || i == 5 || i == 9 || i == 11 || i == 12  || i == 20){
        //         printf("|");
        //     }
        //     printf("%X", (uint32_t)buffer[i]);
        // }
        if (validate_mask(mask)){  // Checks if mask is valid and if its not it sends an error code to the client (ERROR_INVALIDPARAMETERS)
            uint32_t data_stream_id = (uint32_t)rand();
            register_data_stream(data_stream_id, *client_fd, ip, mask, port, data_size);
            printf("data_stream_id -> %u\n", data_stream_id);

            // --- sends TRANSFER_CMD to admin ---
            encapsulate_transfer_cmd(buffer, COMMAND_SETDATASTREAMID, data_stream_id);
            send(*client_fd, buffer, 5, 0);




            // --- sends RUN_CMD_CLIENT to all clients ---
            encapsulate_run_cmd_client(buffer, command, data_stream_id, output, data_size);
            send_to_all_clients(data_stream_id, buffer, thread_id, 0); // TODO: this has to run in background





        } else {
            send(*client_fd, (char[]){ERROR_INVALIDPARAMETERS}, 1, 0);
        }
       
    }
}


void send_to_all_clients(uint32_t data_stream_id, char *buffer, int *thread_id, int offset) {
    // socklen_t client_len = sizeof(*client_addr);
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    // Get filter
    int index = hash(data_stream_id);
    while (data_streams[index]->id != data_stream_id) { // Check if in use
        index = (index + 1) % MAX_CLIENTS; // Linear probing
    }
    if (offset > 0) {
        uint32_t data_stream_id;
        char *data;
        decapsulate_transfer_data_cmd(buffer, &data_stream_id, data);
        encapsulate_transfer_data_cmd(buffer, buffer[0], data_stream_id, data, offset);
    }
    // Get all receivers fd and send to all receivers
    for (int t = 0; t < NUM_THREADS; t++) {
        for (int i = 0; i < MAX_CLIENTS; i++){

            if (!clients[t]->active[i]){
                continue;
            }
            if (getpeername(clients[t]->fd[i], (struct sockaddr *)&client_addr, &client_len) == -1) {
                // perror("getpeername");
                continue;
            }
            // char ip_address[INET_ADDRSTRLEN];
            // if (inet_ntop(AF_INET, &(client_addr.sin_addr), ip_address, INET_ADDRSTRLEN) == NULL) {
            //     perror("inet_ntop");
            //     break;
            // }
            // printf("Peer address: %i, filter address: %i\n", client_addr.sin_addr.s_addr, data_streams[index]->ip);
            // printf("Peer address masked: %i, filter address masked: %i\n", client_addr.sin_addr.s_addr & data_streams[index]->mask, data_streams[index]->ip & data_streams[index]->mask);
            if (((client_addr.sin_addr.s_addr) & data_streams[index]->mask) == (data_streams[index]->ip & data_streams[index]->mask) &&
                (client_addr.sin_port == data_streams[index]->port || data_streams[index]->port == 0 )) { // if is in the filter
                printf("client found | thread: %i fd: %i\n", t,i);
                send(clients[t]->fd[i], buffer, BUFFER_SIZE, 0);
                printf("SENDING TO ALL CLIENTS\n");
            }
        }
    }

}



// void data_stream_route(char *buffer, int *thread_id) {


//     if (buffer[0] != COMMAND_RECEIVINGFROMCLIENT || buffer[0] != COMMAND_SENDINGFROMADMIN) {
//         return;
//     }

//     uint32_t data_stream_id;
//     char data[BUFFER_SIZE-5];
//     uint32_t ip;
//     uint16_t port;
    

//     if (buffer[0] == COMMAND_SENDINGFROMADMIN) { //  here I get data_streamid + data // SEND ADMIN RUN_CMD_ADMIN / TRANSFER_DATA_CMD -> CLIENTS
//         // decapsulate_transfer_data_cmd(buffer, &data_stream_id, data);
//         // send_to_all_clients(data_stream_id, data, client_addr, client_len, thread_id); // this send actual chunk to all clients
//     } else if (buffer[0] == COMMAND_RECEIVINGFROMCLIENT) {  // SEND CLIENT TRANSFER_CMD -> ADMIN
//         data_stream_id = get_data_stream_id(buffer);
//         // Get sender fd
//         int index = hash(data_stream_id);
//         while (data_streams[index]->id != data_stream_id) { // Check if in use
//             index = (index + 1) % MAX_CLIENTS; // Linear probing
//         }
//         send(data_streams[index]->fd_sender, buffer, 1, 0); // only sending command (client dont need data_stream_id)
//     }



// //     if (client.command[0] != '\0'){
// //         if (client.command[0] == 'b' && !thread_action_done[*thread_id]){
// //             // printf("[%i]\tclient count: %i/%i\n", *thread_id, client_count[*thread_id], MAX_CLIENTS);
// //             for (int j = 0; j < MAX_CLIENTS; j++){
// //                 if (!clients.active[j]){
// //                     continue;
// //                 }
// //                 // send(clients_thread.fd[j], "test", sizeof("test"), 0);
// //                 //

// //                 if (getpeername(clients_thread.fd[j], (struct sockaddr *)&client_addr, &client_len) == -1) {
// //                     perror("getpeername");
// //                     // handle error
// //                 } 
// //                 char ip[INET_ADDRSTRLEN];
// //                 inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN);
// //                 char port[8];
// //                 current_time = time(NULL);
// //                 char ip_port[INET_ADDRSTRLEN + 10 + sizeof(current_time)]; // Make space for IP and port
                
// //                 sprintf(ip_port, "%s:%s - %ld\n", ip, port, (long)current_time);
// //                 // printf("[%i]sending to ->%i\n", *thread_id, j);
// //                 if (send(clients_thread.fd[j], ip_port, sizeof(ip_port), 0) == -1){
// //                     perror("send");
// //                 }
// //             }
// //         }


// //         if (!thread_action_done[*thread_id]){
// //             pthread_mutex_lock(&mutex_thread_action_done);
// //             thread_action_done[*thread_id] = true;
// //             // printf("[%i]a\n", *thread_id);
// //             pthread_mutex_unlock(&mutex_thread_action_done);
// //         }

// //         // printf("[%i]b \t %d\n", *thread_id, all_thread_action_done());
// //         do {
// //             // printf("[%i]bb \t %d\n", *thread_id, all_thread_action_done());
// //             pthread_barrier_wait(&barrier);
            
// //         } while (!all_thread_action_done());
// //         // pthread_barrier_wait(&barrier);

// //         if (all_thread_action_done()){
// //             // printf("[%i]c\n",*thread_id);
// //             pthread_barrier_wait(&barrier_msg);
// //             if (client.command[0] != '\0'){ // this is faster than using mutex and every thread changing his part
// //                 // for (int i = 0; i < MAX_CLIENTS; i++) {
// //                 //     thread_action_done[i] = 0;
// //                 // }
// //                 client.command[0] = '\0';
// //                 // printf("seting client.command[0] = '\\0' \n");
// //             }
// //             pthread_mutex_lock(&mutex_thread_action_done);
// //             thread_action_done[*thread_id] = false;
// //             pthread_mutex_unlock(&mutex_thread_action_done);
// //         }
        
// //     }
// }

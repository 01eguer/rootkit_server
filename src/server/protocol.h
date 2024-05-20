#pragma once
#include "options.h"
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


// ---- SESSION COMMANDS ---- [TOTAL: 15]
#define COMMAND_LOGIN 0x01
// 0x02 - 0x0F <- RESERVED FOR FUTURE USE

// ---- STATUS COMMANDS ---- [TOTAL: 16]
#define COMMAND_PING 0x10
// 0x13 - 0x1F <- RESERVED FOR FUTURE USE

// ---- CONNECTION/INSTALLATION COMMANDS ---- [TOTAL: 16]
#define COMMAND_DISCONECT 0x20
#define COMMAND_DESTROY 0x21
// 0x21 - 0x2F <- RESERVED FOR FUTURE USE

// ----  COMMANDS ---- [TOTAL: 128]
// RUN
#define COMMAND_EXECCMD 0x30 // run command
#define COMMAND_EXECBIN 0x31 // run binary code
#define COMMAND_EXECASM 0x32 // run assembly
#define COMMAND_DEVREAD 0x33 // read device
#define COMMAND_DEVWRITE 0x34 // write device
#define COMMAND_DEVSHOW 0x35  // show devices
#define COMMAND_FILEREAD 0x36  // read file
#define COMMAND_FILEWRITE 0x37  // write file

// TRANSFER
#define COMMAND_SETDATASTREAMID 0x38
#define COMMAND_STARTRECEIVINGFROMCLIENT 0x39
#define COMMAND_RECEIVINGFROMCLIENT 0x3A
#define COMMAND_SENDINGFROMADMIN 0x3B

#define COMMAND_SENDINGFROMCLIENT 0x3C
#define COMMAND_RECEIVINGFROMADMIN 0x3D
// 0x37- 0xAF <- RESERVED FOR FUTURE USE

// ---- ERROR CODES ---- [TOTAL: 40]
#define ERROR_UNKNOWN 0xB0
#define ERROR_INCORRECTPASS 0xB1
#define ERROR_RATELIMIT 0xB2
#define ERROR_MAXCLIENTREACHED 0xB4
#define ERROR_INVALIDPARAMETERS 0xB5
#define ERROR_SENDINGCLIENT 0xB6
// 0xB2 - 0xD8 <- RESERVED FOR FUTURE USE

// ---- STATUS CODES ---- [TOTAL: 39]
#define STATUS_OK 0xD9
#define STATUS_CORRECTPASS 0xDA
#define STATUS_FROMADMINSENDED 0xDB
#define STATUS_FROMCLIENTSENDED 0xDC
#define STATUS_NOOUTPUT 0xDD
// 0xB2 - 0xD8 <- RESERVED FOR FUTURE USE



// DATA STRUCTURES

// FUNCTIONS FOR CHECKING PROTOCOL
bool check_password(char *buffer);
bool check_token(char *buffer);

// ENCAPSULATE DECAPSULATE DATA
void encapsulate_run_cmd_admin(char *buffer, char command, uint32_t ip, uint32_t mask, uint16_t port, bool output, uint64_t data_size);
void decapsulate_run_cmd_admin(char *buffer, uint32_t *ip, uint32_t *mask, uint16_t *port, bool *output, uint64_t *data_size);

void encapsulate_run_cmd_client(char *buffer, char command, uint32_t data_stream_id, bool output, uint64_t data_size);
void decapsulate_run_cmd_client(char *buffer, uint32_t *data_stream_id, bool *output, uint64_t *data_size);



void encapsulate_transfer_cmd(char *buffer, char command, uint32_t data_stream_id);
// void decapsulate_transfer_cmd(char *buffer, uint32_t data_stream_id);
uint32_t get_data_stream_id(char *buffer); // better this

void encapsulate_transfer_cmd_larger(char *buffer, char command, uint64_t total_client_count);
uint64_t get_total_client_count(char *buffer); // better this

void encapsulate_transfer_data_cmd(char *buffer, char command, uint32_t data_stream_id, char *data, int offset);
void decapsulate_transfer_data_cmd(char *buffer, uint32_t *data_stream_id, char *data);

void encapsulate_transfer_data_tid_cmd(char *buffer, char command, uint32_t data_stream_id, uint16_t thread_id, char *data);
void decapsulate_transfer_data_tid_cmd(char *buffer, uint32_t *data_stream_id, uint16_t *thread_id, char *data);

bool validate_mask(uint32_t mask);

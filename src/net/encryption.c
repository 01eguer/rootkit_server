
#include "key.h"

// #include <stdio.h>



void encrypt_decrypt(char *message){
    // const int message_len = sizeof(message)/(int)sizeof(message[0]); // get array len
    int message_len = 0;
    while (message[message_len] != '\0') {
        message_len++;
    }
    // printf("message_len -> %i , %i / %i \n", message_len, (int)sizeof(*message), (int)sizeof(message[0]));
    int j = 0;
    int i;
    for (i =0 ;i <= message_len-1; i++){
        if (i >= (key_len + (key_len * j))){
            j += 1;
        }
        message[i] = message[i] ^ key[i - (key_len * j)];
        // printf("i: %i, j: %i \t if: %i \n", i, j, ((key_len * j))); // debug
    }
}

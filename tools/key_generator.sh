#!/bin/bash

# Define the number of bytes in the key
key_len=1024

# Generate random bytes for the key
random_key=$(openssl rand -hex $((key_len/2)))

# Print the key in the desired format
echo "unsigned char key[] = {"
count=0
for ((i=0; i<${#random_key}; i+=2)); do
    printf "  0x%s%s" "${random_key:$i:2}"
    if [ $((i+2)) -lt ${#random_key} ]; then
        echo -n ","
    fi
    ((count++))
    if [ $count -eq 12 ]; then
        echo
        count=0
    fi
done
echo "};"
echo "unsigned int key_len = $key_len;"


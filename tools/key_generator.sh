#!/bin/bash

# Check if the length of the key is provided as a parameter
if [ "$#" -ne 1 ]; then
	echo "Usage: $0 <key_length>"
	exit 1
fi

# Generate binary file with random data
head -c "$1" /dev/urandom > key.bin


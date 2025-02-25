#!/bin/bash

# Ensure at least two arguments are provided
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <N> <command> [args...]"
    exit 1
fi

# Extract the number of times to run the command
N=$1
shift  # Remove the first argument (N)

# Run the command N times
for ((i = 1; i <= N; i++)); do
    "$@" "$i"
done

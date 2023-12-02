#!/bin/bash

# Check if an output path was provided as the first argument
if [ -z "$1" ]; then
    echo "Usage: $0 <output_path>"
    exit 1
fi

# Specify the range and quantity of random numbers to generate
min=1
max=100
count=10

# Specify the file to save the random numbers
output_file=$1

# Clear or create the file
> "$output_file"

# Generate unique random numbers and write them to the file
while [ $(wc -l < "$output_file") -lt $count ]; do
    random_number=$((min + RANDOM % (max - min + 1)))

    # Check if the random number already exists in the file
    if ! grep -q "$random_number" "$output_file"; then
        echo "$random_number" >> "$output_file"
    fi
done

echo "Generated unique random numbers have been saved to the file $output_file."

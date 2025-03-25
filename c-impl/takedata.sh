#!/bin/bash

# Hardcoded list of files
FILES=("timing_frames.csv" timing_hit.csv "timing_vec.csv" "timing_scal.csv" "timing_ins.csv" "timing_opt.csv" "settings.tex")

# Check if the destination directory is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <destination_directory>"
    exit 1
fi

# Temporary file to store all numbers from the second column in the .csv files
tmp_file="/tmp/second_column_values.txt"
> "$tmp_file"  # Clear the file if it exists
> "settings.tex"
# Extract the second column from each file and append to tmp_file
for file in "${FILES[@]}"; do
    if [[ -f "$file" ]]; then
        awk -F',' 'NR>1 {print $2}' "$file" >> "$tmp_file"
    else
        echo "Warning: File $file not found. Skipping."
    fi
done

# Find the maximum value
max_value=$(sort -nr "$tmp_file" | awk "NR==15")

# Output the result in the required format
echo "\\newcommand{\\customymin}{0} \\newcommand{\\customymax}{$(echo $max_value + $max_value/10 | bc)}" > settings.tex

echo "Maximum value found: $max_value"



DEST_DIR="$1"

# Create the destination directory if it doesn't exist
mkdir -p "$DEST_DIR"

# Move each file if it exists
for FILE in "${FILES[@]}"; do
    if [ -e "$FILE" ]; then
        cp "$FILE" "$DEST_DIR"
        echo "Copied $FILE to $DEST_DIR"
    else
        echo -e "\e[31mSkipping\e[0m $FILE (does not exist)"
    fi
done

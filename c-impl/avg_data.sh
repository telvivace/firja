#!/bin/bash
set -o errexit
set -o nounset
set -o pipefail

if [ -z "$1" ]; then
    echo "Usage: $0 <source_directory>"
    exit 1
fi
# Ensure at least one CSV file exists
shopt -s nullglob
tmpdir=$(mktemp -d)
trap 'rm -rf -- "$tmpdir"' EXIT
files=("$1"/timing_*.csv)
#nfiles=$(ls timing_*.csv | wc -l)
nfiles=${#files[@]}

if [ ${#files[@]} -eq 0 ]; then
    echo "No CSV files found!" >&2
    exit 1
fi

# Extract all second columns and merge them
# but this fails since paste does not know how to deal with array output apparently
# paste -d' ' <(cut -d, -f2 "${files[@]}") | tail -n +2 | sed 's/\s/,/g' > merged.csv

# Extract all second columns and merge them (ignoring first line and re-adding commas)
paste -d, "$1"/timing_*.csv | tail -n +2 | cut -d, -f$(seq -s, 2 2 "$nfiles") > "$tmpdir"/merged.csv

# Compute the average for each row
awk -F, '{sum=0; for(i=1; i<=NF; i++) sum+=$i; print sum/NF}' "$tmpdir"/merged.csv > "$tmpdir"/averaged.csv

# Attach the first column from the first file
paste -d, <(cut -d, -f1 "${files[0]}" | tail -n +2) "$tmpdir"/averaged.csv | sed 's/\s/,/g' > "$tmpdir"/final_output.csv

# Concatenate the first line (column names) from the first file
cat <(head -n 1 "${files[0]}") "$tmpdir"/final_output.csv > "$1"/final_output.csv

echo "Averaged data from $nfiles files saved to $1/final_output.csv"

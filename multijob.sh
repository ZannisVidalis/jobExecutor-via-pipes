#!/bin/bash

#Process each input file
for file in "$@"; do
    #Read each line from the file
    while IFS= read -r line || [ -n "$line" ]; do
        #Execute the command from the line
        $line
    done < "$file"
done

#!/bin/bash 
./p2 $1 2>/dev/null > ./test-files/tmp.txt

h1=$(head -1 ./test-files/tmp.txt)
h2=$(tail -1 ./test-files/tmp.txt)

if [ $(wc -l < ./test-files/tmp.txt) -eq 2 ]; then 
    if [ $h1 = $h2 ]; then 
        echo 1
        exit 
    fi 
fi 
echo 0

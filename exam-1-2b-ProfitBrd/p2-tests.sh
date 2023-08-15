#!/usr/bin/env bash

# function to clean up files and make executables
remake () {
    # echo -e "\ old files and making executables"
    make -s clean
    make -s >/dev/null 2>&1
}


echo -e "To remove colour from tests, set COLOUR to 1 in sh file\n"
COLOUR=0
if [[ COLOUR -eq 0 ]]; then
    ORANGE='\033[0;33m'
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    NC='\033[0m'
else
    ORANGE='\033[0m'
    GREEN='\033[0m'
    RED='\033[0m'
    NC='\033[0m'
fi

SCORE=0

echo -e "\nStart testing"
remake
echo -e "\nTesting :: Compilation\n"
echo -e "  ${GREEN}Passed${NC}"
SCORE=$(($SCORE+10))

remake
echo -e "\nTesting :: Pipe creation\n"
if [ $(grep -c "pipe(" p2.cpp) -ge 2 ]; then 
    echo -e "  ${GREEN}Passed${NC}"
    SCORE=$(($SCORE+10))
else 
    echo -e "  ${RED}Failed${NC}"
fi 

echo -e "\nTesting :: Using 2x read\n"
if [ $(grep -c "read(" p2.cpp) -ge 2 ]; then 
    echo -e "  ${GREEN}Passed${NC}"
    SCORE=$(($SCORE+5))
else 
    echo -e "  ${RED}Failed${NC}"
fi 

echo -e "\nTesting :: Using 2x write\n"
if [ $(grep -c "write(" p2.cpp) -ge 2 ]; then 
    echo -e "  ${GREEN}Passed${NC}"
    SCORE=$(($SCORE+5))
else 
    echo -e "  ${RED}Failed${NC}"
fi 


remake
echo -e "\nTesting :: Correct output for message = hello world - ./p2 \"hello world\"\n"
ret=$(./test-files/t1.sh "hello world")
if [ $ret -eq 1 ]; then
    echo -e "  ${GREEN}Passed${NC}"
    SCORE=$(($SCORE+20))
else
    echo -e "  ${RED}Failed${NC}"
fi

# print score and delete executable
echo -e "\nSCORE: ${SCORE}/50\n"
make -s clean

exit 0
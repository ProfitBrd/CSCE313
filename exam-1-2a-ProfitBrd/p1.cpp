#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <bits/stdc++.h>

int main(int argc, char** argv) {
    int n = 1, opt;
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
        case 'n':
            n = atoi(optarg);
            break;
        }
    }

    /*
    1. TODO: fork n child processes and run p1-helper on each using execvp
        > note: we need to synchronize the child processes to print in the desired order
        > note: sleep(..) does not guarantee desired synchronization
        > note: check "raise" system call
        > note: check "WUNTRACED" flag for "waitpid"
    */
    std::vector<pid_t> pidList;
    for (int i = 0; i < n; i++){
        pid_t pid = fork();
        if (pid == 0){
            raise(SIGSTOP);
            char *args[] = {const_cast<char *>("./p1-helper"), nullptr};
            if (execvp(args[0], args) < 0) //overwrites entirety of below file
            {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
        else{
            pidList.push_back(pid);
        }
    }
   
    
    /* 
    2. TODO: print children pids 
    */
    for (long unsigned int i = 0; i < pidList.size()-1; i++){
        std::cout << pidList[i] << " ";
    }
    std::cout << pidList[pidList.size()-1] << std::endl;


    fflush(stdout);             // DO NOT REMOVE: ensures the first line prints all pids

    
    /* 
    3. TODO: signal children with the reverse order of pids 
        > note: take a look at "kill" system call 
    */
    sort(pidList.begin(), pidList.end(), std::greater<pid_t>());
    for (auto pid : pidList){
        kill(pid, SIGCONT);
        waitpid(pid, NULL, WUNTRACED);
    }

    printf("Parent: exiting\n");

    return 0;
}
/****************
LE2: Basic Shell
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <sys/wait.h> // wait
#include "Tokenizer.h"
#include <iostream>
using namespace std;

int main () {
    const int READ_END = 0;
    const int WRITE_END = 1;
    pid_t pid;
    
    int OG_READ_END = dup(READ_END);
    int OG_WRITE_END = dup(WRITE_END);
    string input;
    while(true){
        std::cout << "Provide Commands: ";
        getline(cin, input);

        //if input is exit (case invariant), break out of loop
        if (input == "exit") break;

        Tokenizer token(input);

        //for each command in token.commands
        for (unsigned long int i = 0; i < token.commands.size(); i++){


            // Create pipe
            int fd[2];
            pipe(fd);
            pid = fork();

            

            if (pid == 0){
                if (i < token.commands.size()-1){ //if last one in the list we need to see it on the console redirect output to write end of pipe
                    dup2(fd[WRITE_END],STDOUT_FILENO); //redirect output of execvp to the write end of the pipe
                }
                close(fd[READ_END]);
                vector<char*> commands;
                for (auto const& a : token.commands[i]->args){
                    commands.push_back(const_cast<char*>(a.c_str()));
                }
                commands.push_back(NULL);
                if (execvp(commands[0], commands.data()) < 0) //overwrites entirety of below file
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }

            else if(pid > 0){
                //redirect SHELL(parent) input to read end of pipe
                dup2(fd[READ_END],STDIN_FILENO);
                //close write end of pipe
                close(fd[WRITE_END]);

                if(i == token.commands.size()-1){
                    wait(0); // wait until last command finishes
                }
            }  
        }
    }
    dup2(STDIN_FILENO, OG_READ_END); //restoring stdin
    dup2(STDOUT_FILENO, OG_WRITE_END); //restoring stdout 
    
    //left is old, right is new, refer to left side instead of right side ------------------------------------

}  
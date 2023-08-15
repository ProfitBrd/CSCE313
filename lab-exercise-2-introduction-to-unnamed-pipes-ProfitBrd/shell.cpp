/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h> //add ---------------------------
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <cstring>
#include <string>
using namespace std;


int main () {
    const int READ_END = 0;
    const int WRITE_END = 1;
    pid_t pid;
    pid_t pid2;
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // Create pipe
    int fd[2];
    pipe(fd);
    pid = fork();

    

    if (pid == 0){
        
        dup2(fd[WRITE_END],STDOUT_FILENO); //redirect output of file to the write end of the pipe
        close(fd[READ_END]);
        if (execvp(cmd1[0], cmd1) < 0) //overwrites entirety of below file
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    else if(pid > 0){
        
        pid2 = fork();
        if (pid2 == 0){
            dup2(fd[READ_END],STDIN_FILENO); //redirect the pipe output to our STDIN, or input
            close(fd[WRITE_END]); //don't need write end
            if (execvp(cmd2[0], cmd2) < 0) //overwrites entirety of below file
            {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0){ // Reset the input and output file descriptors of the parent.
            close(fd[WRITE_END]);
            close(fd[READ_END]);
            waitpid(pid2, NULL, 0);
        }
    
    }  
}  

    

    // Create child to run first command
    // In child, redirect output to write end of pipe
    // Close the read end of the pipe on the child side.
    // In child, execute the command

    // Create another child to run second command
    // In child, redirect input to the read end of the pipe
    // Close the write end of the pipe on the child side.
    // Execute the second command.

    

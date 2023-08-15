#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
char *custom_env[] = {"USER=Pikachu", "PATH=/tmp", NULL};

int main(void)
{
    const int READ_END = 0;
    const int WRITE_END = 1;

    pid_t pid;
    pid_t pid2;
    pid = fork();


    if (pid == 0){
        printf("Running child process PID: %d\n", getpid());
        if (execle("/home/profitbrd/CSCE313/quiz-2-2-ProfitBrd/echoall", "echoall", "Bandicoot", "Pacman", NULL, custom_env) < 0) //don't need const chaqr because it's in C
        {
            perror("execle");
            exit(EXIT_FAILURE);
        }
    }
    
    else if(pid > 0){
        printf("Running parent process PID: %d\n\n", getpid());
        waitpid(pid, NULL, 0);
        
        pid2 = fork();
        if (pid2 == 0){
            printf("Running child process PID: %d\n", getpid());
            if (execlp("./echoall", "echoall", "Spyro", NULL) < 0) //only needs relative path b/c searching in PATH
            {
                perror("execlp");
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0){
            waitpid(pid2, NULL, 0); //wait for pid2 to complete
        }

    
    }  
    exit(0);
}
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#define MAX_MESSAGE 256

long long unsigned int hash(int seed, char* buf, int nbytes) {
    long long unsigned int H = seed; 
    for (int i = 0; i < nbytes; ++i) 
        H = H * 2 * buf[i] + 1;
    return H;
}

int main(int argc, char** argv) {
    if (argc < 1){
        exit(0);
    }

	const int READ_END = 0;
    const int WRITE_END = 1;
    
	//Create Pipes
    int fd[2];
    pipe(fd);
    int fd2[2];
    pipe(fd2);
    int pid = fork();

    if (pid == 0) {
        // Reads from parent
        char buf[MAX_MESSAGE];
        read(fd[READ_END], buf, sizeof(buf)); //redirect the pipe output to our STDIN, or input
        close(fd[READ_END]);
        
        // get the current PID to use as seed for the hash process
        pid_t currentPID = getpid();
        long long unsigned int h = hash(currentPID, buf, sizeof(buf));
        
        // Sends the hash to the parent, of type void* so we need to use a reference
        write(fd2[WRITE_END], &h, sizeof(&h));
        close(fd2[READ_END]);
    }
    else {
        // Writes to first argument to the child
        write(fd[WRITE_END], argv[1], sizeof(argv));
        close(fd[READ_END]);
        
        // Gets the hash from child
        long long unsigned int h = 0; //child hash
        char buf[MAX_MESSAGE];
        read(fd2[READ_END], &h, sizeof(buf));
        close(fd2[WRITE_END]);
        
        // Calculates hash again on the parent side
        long long unsigned int hrecv = 0; //parent hash
        hrecv = hash(h, buf, sizeof(buf));
        
		// print hashes; DO NOT change
        printf("%llX\n", h);
        printf("%llX\n", hrecv);
    }

    return 0;
}
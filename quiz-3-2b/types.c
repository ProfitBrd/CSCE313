#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    //if command input incorrect
    if (argc < 2){
        printf("usage: path1 [path2 ...]\n");
        exit(0);
    }

    //if command input correct
    struct stat fileStat;
    int i = 1;
    while ( i < (argc)){
        if (lstat(argv[i], &fileStat) < 0){
            printf("%s: path error\n", argv[i]);
            i = i + 1;
            continue;
        }
        
        
        char path[256];
        int fileType = fileStat.st_mode;
        if (S_ISREG(fileType)){
            getcwd(path, 256);
            printf("%s : regular file\n", argv[i]);
        }
        if (S_ISDIR(fileType)){
            getcwd(path, 256);
            printf("%s : directory\n", argv[i]);
        }
        if (S_ISBLK(fileType)){
            getcwd(path, 256);
            printf("%s : block\n", argv[i]);
        }
        if (S_ISCHR(fileType)){
            getcwd(path, 256);
            printf("%s : character special file\n", argv[i]);
        }
        if (S_ISFIFO(fileType)){
            getcwd(path, 256);
            printf("%s : FIFO\n", argv[i]);
        }
        if (S_ISLNK(fileType)){
            getcwd(path, 256);
            printf("%s : symbolic links\n", argv[i]);
        }
        if (S_ISSOCK(fileType)){
            getcwd(path, 256);
            printf("%s : socket\n", argv[i]);
        }
        i = i + 1;
        
    }
    
    exit(0);
}
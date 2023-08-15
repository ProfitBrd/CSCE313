#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{

    //if command input incorrect
    if (argc != 2){
        printf("usage: filestats directory_name\n");
        exit(0);
    }

    DIR *dp;
    if ((dp = opendir(argv[1])) == NULL){
        printf("can’t open %s\n", argv[1]);
        exit(0);
    }

    //if command input correct
    struct dirent *dirp;
    struct stat fileStat;
    chdir(argv[1]);
    while ( (dirp = readdir(dp)) != NULL){
        stat(dirp->d_name, &fileStat);
        
        printf("File Name: %s\n", dirp->d_name);
        printf("inode number: %d\n", (int)dirp->d_ino);
        printf("Number of Links: %d\n", (int)fileStat.st_nlink);
        printf("User ID of owner: %d\n", (int)fileStat.st_uid);
        printf("Group ID of owner: %d\n", (int)fileStat.st_gid);
        printf("Size in Bytes: %d\n", (int)fileStat.st_size);
        printf("Last Access: %s", ctime(&fileStat.st_atime));
        printf("Last Modification: %s", ctime(&fileStat.st_mtime));
        printf("Last status change: %s", ctime(&fileStat.st_ctime));
        printf("Number of disk blocks allocated: %ld\n", fileStat.st_blocks);
        printf("Access mode in octal: %d\n", fileStat.st_mode);

        printf("File Permissions: ");
        printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
        printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
        printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
        printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
        printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
        printf("\n\n");
    }
    closedir(dp);
    exit(0);
        /*
            stat(dir->d_name, &fileStat);
            
            printf("File Name: %s\n", dir->d_name);
            printf("inode number: %d\n", (int)dir->d_ino);
            printf("Number of Links: %d\n", (int)fileStat.st_nlink);
            printf("User ID of owner: %d\n", (int)fileStat.st_uid);
            printf("Group ID of owner: \t%d\n", (int)fileStat.st_gid);
            printf("File inode: \t\t%d\n", (int)fileStat.st_ino);

            printf("File Permissions: \t");
            printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf("\n\n");
            */

            
            // lstat(dir->d_name, &inf);
            // printf("File Name: %s\n", dir->d_name);
            // printf("inode number: %d\n", (int)dir->d_ino);
            // printf("Number of Links: \t%d\n", (int)inf.st_nlink);//fileStat.st_nlink)
            // printf("Size of file: \t%d\n", (int)inf.st_size);
            // printf("\n");
            
            

            // printf("File Size: \t\t%d bytes\n", ); //fileStat.st_size);
            // printf("Number of Links: \t%d\n", dir->d_off);//fileStat.st_nlink);
            // printf("File inode: \t\t%d\n",fileStat.st_ino);
}
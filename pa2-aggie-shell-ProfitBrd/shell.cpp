#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <filesystem>
#include <chrono>
//#include <ctime>

#include <vector>
#include <string>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define WHITE "\033[1;37m"
#define NC "\033[0m"

using namespace std;
const int READ_END = 0;
const int WRITE_END = 1;

int main()
{
    auto currPath = std::filesystem::current_path();
    string previousDirectory = currPath;
    string currentDirectory = currPath;

    int OG_READ_END = dup(READ_END);
    int OG_WRITE_END = dup(WRITE_END);
    vector<pid_t> backgroundProcesses;
    // erasing all the background processes that we let run without waiting for

    for (;;)
    {
        for (int i = (int)backgroundProcesses.size() - 1; i >= 0; i--)
        {
            int status;
            waitpid(backgroundProcesses[i], &status, WNOHANG);
        }

        // TERMINAL BASICS------------------
        auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
        string currentTime = ctime(&timenow);
        currentTime.erase(currentTime.length() - 1); // erase newline

        string pathForTerminal = std::filesystem::current_path();

        cout << YELLOW << "Shell$" << BLUE << " ~" << BLUE << getenv("USER") << "~ " << GREEN << currentTime << " " << WHITE << pathForTerminal << NC << " ";

        // USER INPUT-----------------
        string input;
        getline(cin, input);

        if (input == "")
        {
            continue;
        }
        if (input == "exit")
        { // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl
                 << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer token(input);
        if (token.hasError())
        { // continue to next prompt if input had an error
            continue;
        }

        if (token.commands[0]->args[0] == "cd")
        {
            if (token.commands[0]->args[1] == "-")
            {
                cout << "Going to Previous Location" << endl;
                chdir(previousDirectory.c_str());
                previousDirectory = currentDirectory;
                currentDirectory = std::filesystem::current_path();
                continue;
            }
            else
            { // they must want to go back a directory
                cout << "going up a directory" << endl;
                chdir(token.commands[0]->args[1].c_str());
                previousDirectory = currentDirectory;
                currentDirectory = std::filesystem::current_path(); //---------------need this or back doesn't work
                continue;
            }
        }

        pid_t pid;

        // Runs 1 time for each command
        for (unsigned long int i = 0; i < token.commands.size(); i++)
        {
            // Create pipe
            int fd[2];
            pipe(fd);
            pid = fork();

            // if child
            if (pid == 0)
            {
                if (i < token.commands.size() - 1)
                {                                       // if last one in the list we need to see it on the console redirect output to write end of pipe
                    dup2(fd[WRITE_END], STDOUT_FILENO); // redirect output of execvp to the write end of the pipe
                }
                if (token.commands[i]->hasOutput())
                {
                    // removing semicolon from the back of an out_file
                    string temp = token.commands[i]->out_file;
                    if (temp[temp.size() - 1] == ';')
                    {
                        temp = temp.substr(0, temp.size() - 1);
                    }

                    int filedescriptor = open((temp.c_str()), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    dup2(filedescriptor, STDOUT_FILENO);
                    close(filedescriptor);
                }
                if (token.commands[i]->hasInput())
                {
                    int filedescriptor = open((token.commands[i]->in_file.c_str()), O_RDONLY, S_IRUSR | S_IWUSR);
                    dup2(filedescriptor, STDIN_FILENO);
                    close(filedescriptor);
                }

                close(fd[READ_END]);
                vector<char *> commands;
                for (auto const &a : token.commands[i]->args)
                {
                    commands.push_back(const_cast<char *>(a.c_str()));
                }
                commands.push_back(NULL);
                if (execvp(commands[0], commands.data()) < 0) // overwrites entirety of below file
                {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }

            else if (pid > 0)
            {
                // redirect SHELL(parent) input to read end of pipe
                dup2(fd[READ_END], STDIN_FILENO);
                close(fd[WRITE_END]);


                if (!(token.commands[i]->isBackground()))
                {                          // if you want it to run in background, don't make the program wait
                    waitpid(pid, NULL, 0); // wait until each child process finishes in order
                }
                else
                {
                    backgroundProcesses.push_back(pid);
                }


            }
            close(fd[READ_END]);
            close(fd[WRITE_END]);
        }

        // reset both input and output after messing around w/ redirecting to READ and WRITE ends of the pipe
        dup2(OG_READ_END, STDIN_FILENO);
        dup2(OG_WRITE_END, STDOUT_FILENO);
    }
    dup2(STDIN_FILENO, OG_READ_END);   // restoring stdin
    dup2(STDOUT_FILENO, OG_WRITE_END); // restoring stdout
}

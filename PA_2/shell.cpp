#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    for (;;) {
        // need date/time, username, and absolute path to current dir
        cout << YELLOW << "Shell$" << NC << " ";

        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }
        if (tknr.commands.empty()) { //check if there's any commands
                continue;
        }
        int number_cmds = tknr.commands.size();
        std::vector<pid_t> child_pids;

        std::vector<int[2]> pipes(number_cmds - 1); // create a vector of pipes

        for (int i = 0; i < number_cmds - 1; ++i) {
            if (pipe(pipes[i]) < 0) {
                perror("Pipe failed");
                exit(1);
            }
        }
        for (int i = 0; i < number_cmds; ++i) { // for every command, we make a vector of arguments for it
            std::vector<char*> args;
            for (const std::string& s : tknr.commands[i]->args) { //this actually propagates the args vector
                args.push_back(const_cast<char*>(s.c_str()));
            }
            args.push_back(nullptr); // add a nullptr to the end
            pid_t pid = fork();
            if (pid < 0) {
                perror("Fork failed");
                exit(1);
            }
            if (pid == 0) { //child?
                if (i > 0) { // if it isnt the very first command then we connect stdin to the last pipe
                    dup2(pipes[i-1][0], 0);
                }
                if (i < number_cmds -1) { //if we're at the end of the commands, get the write end of the pipe to stdout
                    dup2(pipes[i][1], 1);
                }
                for (int j = 0; j < number_cmds-1; ++j) { //close our pipes
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                if (execvp(args[0], args.data()) <0) { //execute the commands
                    perror("Exec failed");
                    exit(2);
                }
            } 
            else {
                child_pids.push_back(pid);
            }
        }
        for (int j = 0; j < number_cmds-1; ++j) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }
        int status = 0;
        for (pid_t pid : child_pids) {
            waitpid(pid, &status, 0);
        }
    }
}

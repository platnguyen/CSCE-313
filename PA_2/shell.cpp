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

        for (auto& cmd : tknr.commands) { //loops through all commands
            std::vector<char*> args; //argument 
            for (const std::string& s : cmd->args) {
                args.push_back(const_cast<char*>(s.c_str()));
            }
            args.push_back(nullptr);
            pid_t pid = fork();
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }

            if (pid == 0) {  // if child, exec to run command
                
                if (execvp(args[0], args.data()) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }
            }
            else {  // if parent, wait for child to finish
               
		int status = 0;
		waitpid(pid, &status, 0);
            }
        }

    }

}

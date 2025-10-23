#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <pwd.h>
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
    // Get the old working directory 
    char initial_pwd_buf[1024];
    string old_pwd;
    if (getcwd(initial_pwd_buf, sizeof(initial_pwd_buf)) != NULL) {
        old_pwd = initial_pwd_buf;
    } else {
        // If getcwd fails (which is rare), just start with an empty string
        old_pwd = "";
        perror("getcwd"); // Print an error but continue
    }
    for (;;) {
        // Get the data and time
        time_t now = time(0);
        tm *ltm = localtime(&now);
        char time_buf[80];
        strftime(time_buf, sizeof(time_buf), "%b %d %H:%M:%S", ltm);
        string time_str = time_buf;

        
        // Get the username
        struct passwd *pw = getpwuid(getuid());
        string username = (pw != NULL) ? pw->pw_name : "user";
        
        // get the current directory
        char cwd_buf[1024]; // A buffer to hold the path
        string current_dir = (getcwd(cwd_buf, sizeof(cwd_buf)) != NULL) ? cwd_buf : "?";

        // Print everything together
        // Format: [Date] [Username]:[Directory]$
        cout << time_str << " " << GREEN << username << NC << ":" << BLUE << current_dir << NC << "$ ";
        

        // get user inputted command
        string input;
        getline(cin, input);
        
        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }
        //Split up the input by ampersand to not mess with tokenizer
        std::vector<string> split_ampersand;
        size_t start_split = 0;
        size_t end_split = input.find("&&");
        while (end_split != string::npos) {
            split_ampersand.push_back(input.substr(start_split, end_split - start_split));
            start_split = end_split + 2; 
            end_split = input.find("&&", start_split);
        }
        split_ampersand.push_back(input.substr(start_split));
        bool prev_success = true; //used to break out if the last command failed
        int status = 0;
        for (const string& cmd_set : split_ampersand) { //loop through the split vector
            if (!prev_success) {
                break;
            }

            // specific fix for the -d issue
            std::string fixed_cmd_set = cmd_set;
            size_t pos = 0;
            while ((pos = fixed_cmd_set.find("-d'", pos)) != std::string::npos) {
                fixed_cmd_set.insert(pos + 2, " "); // insert space between -d and '
                pos += 3; // move past inserted space
            }
            while ((pos = fixed_cmd_set.find("-d\"", pos)) != std::string::npos) {
                fixed_cmd_set.insert(pos + 2, " "); // insert space between -d and "
                pos += 3;
            }
            Tokenizer tknr(fixed_cmd_set);



            //Tokenizer tknr(cmd_set); //tokenize the user input after it got split
            if (tknr.hasError()) {  // continue to next prompt if input had an error
                continue;
            }
            if (tknr.commands.empty()) { //check if there's any commands
                continue;
            }

            if (tknr.commands.size() == 1 && tknr.commands[0]->args[0] == "cd") {//if we're only doing cd
                string new_dir;
                
                // just cd, go home
                if (tknr.commands[0]->args.size() < 2) { 
                    const char* home = getenv("HOME");
                    if (home) {
                        new_dir = home;
                    }
                } 
                // cd - go to previous working directory
                else if (tknr.commands[0]->args[1] == "-") {
                    if (old_pwd.empty()) {
                        cerr << "shell: cd: OLDPWD not set" << endl;
                        prev_success = false; // Mark as failed
                        continue; // Skip to next job
                    }
                    new_dir = old_pwd;
                }
                // cd to a patch
                else {
                    new_dir = tknr.commands[0]->args[1];
                }

                // save directory and complete moving to new directory
                if (!new_dir.empty()) {
                    char cwd_buf[1024];
                    string current_pwd;
                    if (getcwd(cwd_buf, sizeof(cwd_buf)) != NULL) {
                        current_pwd = cwd_buf;
                    } else {
                        current_pwd = ""; // Set to empty on failure
                        perror("getcwd");
                    }

                    if (chdir(new_dir.c_str()) != 0) {
                        perror("shell: cd"); // Print error
                        prev_success = false; // Mark as failed
                    } else {
                        // Only update old_pwd on success
                        old_pwd = current_pwd;
                        prev_success = true; // Mark as success
                    }
                }
                
                // 'cd' command is done, skip the forking/piping logic
                continue; 
            }
            int number_cmds = tknr.commands.size();
            std::vector<pid_t> child_pids;
            pid_t prev_pid = -1;

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
                
                if (args.empty() || args[0] == nullptr) {
                    continue;
                }

                if (string(args[0]) == "exit") {  // print exit message and break out of infinite loop
                    cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
                    exit(0);
                }

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

                    auto& cmd = tknr.commands[i];
                    if (cmd->hasInput()) {
                        int in_fd = open(cmd->in_file.c_str(), O_RDONLY);
                        if (in_fd < 0) {
                            perror("Opening in_file failed");
                            exit(2);
                        }
                        dup2(in_fd, 0);
                        close(in_fd);
                    }
                    if (cmd->hasOutput()) {
                        int out_fd = open(cmd->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (out_fd < 0) {
                            perror("Opening out_file failed");
                            exit(2);
                        }
                        dup2(out_fd, 1);
                        close(out_fd);
                    }

                    for (int j = 0; j < number_cmds-1; ++j) { //close our pipes in child
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
                    prev_pid = pid;
                }
            }
            for (int j = 0; j < number_cmds-1; ++j) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            if (prev_pid != -1) { //wait for children
                waitpid(prev_pid, &status, 0);
                for (pid_t pid : child_pids) {
                    if (pid != prev_pid) {
                        waitpid(pid, nullptr, 0);
                    }
                }
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) { //check if the last command succeeded or not
                    prev_success = true;
                } else {
                    prev_success = false;
                }
            }
        }
        
    }
}

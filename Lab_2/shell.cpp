/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <iostream>
#include <sys/wait.h>

using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};
    // Create pipe
    int fds[2];
    // If either pipe() or fork() returns -1, print that it failed
    if (pipe(fds) == -1) {
	cerr << "Pipe failed\n";
        return 1;
    }

    // Creates child to run first command
    pid_t pid_1 = fork();
    if (pid_1 == -1) {
	cerr << "Fork failed\n";
	return 1;
    }
    if (pid_1 == 0) {
        // Redirects output to the write end of the pipe
        dup2(fds[1], STDOUT_FILENO);
        // Closes the read end of the pipe (unused)
        close(fds[0]);
        // Closes the write end of the pipe
        close(fds[1]);
        // Executes the first command
        execvp(cmd1[0],cmd1);
        // Prints an error message if exec fails
        cerr << "Exec failed\n";
        return 1;
    }
    else {
	wait(nullptr);
    }
    // Make another child for the second command
    pid_t pid_2 = fork();
    if (pid_2 == -1) {
	cerr << "Fork failed\n";
	return 1;
    }
    if (pid_2 == 0) {
        // Redirects input to the read end of the pipe
        dup2(fds[0], STDIN_FILENO);
        // Closes the write end of the pipe (unused)
        close(fds[1]);
        // Closes the read end of the pipe
        close(fds[0]);
        // Executes the second command.
        execvp(cmd2[0],cmd2);
        cerr << "Exec failed\n";
        return 1;
    }
    // Closes both ends of the pipe for parent
    close(fds[0]);
    close(fds[1]);
    // Waits for child of the specific pid
    waitpid(pid_1, nullptr, 0);
    waitpid(pid_2, nullptr, 0);
}

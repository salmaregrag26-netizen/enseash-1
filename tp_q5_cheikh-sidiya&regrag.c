#include <unistd.h>  
#include <string.h>    
#include <stdlib.h>     
#include <sys/wait.h>  
#include <stdio.h>     
#include <signal.h>     
#include <time.h>       

#define BUFSIZE 1000    // Maximum command size

void writeRet(char *str, int code, int time_ms) {
    char buf[BUFSIZE];

    // Build string: "code|time_ms"
    sprintf(buf, "%d|%dms] ", code, time_ms);

    // Append result to the prompt
    strcat(str, buf);

    // Display the prompt
    write(STDOUT_FILENO, str, strlen(str));
}

/*
 * This function executes a command entered by the user
 * and measures its execution time
 */
void Commande(char *cmd, ssize_t n) {
    int pid, status;
    char msg[BUFSIZE];
    struct timespec start, end;
    int time_exe_ms;

    // Remove the newline character at the end of the command
    cmd[n - 1] = '\0';

    // If the user types "exit" or sends Cmd, quit the shell
    if (strcmp(cmd, "exit") == 0 || n == 0) {
        write(STDOUT_FILENO, "Bye bye...\n", strlen("Bye bye...\n"));
        exit(EXIT_SUCCESS);
    }

    // Get start time
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create a new process
    pid = fork();

    if (pid == 0) {
        // Child process: execute the command
        execlp(cmd, cmd, NULL);

        // If exec fails, display error and exit
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    // Parent process waits for the child
    wait(&status);

    // Get end time
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Compute execution time in milliseconds
    // We compute the elapsed time by subtracting the start and end timestamps and converting seconds and nanoseconds into milliseconds 
    time_exe_ms = (end.tv_sec - start.tv_sec) * 1000
                + (end.tv_nsec - start.tv_nsec) / 1000000;

    // Command terminated normally
    if (WIFEXITED(status)) {
        strcpy(msg, "enseash [exit:");
        writeRet(msg, WEXITSTATUS(status), time_exe_ms);
    }
    // Command terminated by a signal
    else if (WIFSIGNALED(status)) {
        strcpy(msg, "enseash [sign:");
        writeRet(msg, WTERMSIG(status), time_exe_ms);
    }
}

int main() {
    const char *mess =
        "Bienvenue dans le Shell ENSEA.\n"
        "Pour quitter, tapez 'exit'.\n"
        "enseash % ";

    char cmd[BUFSIZE];
    ssize_t n;

    // Display welcome message
    write(STDOUT_FILENO, mess, strlen(mess));

    // Main shell loop
    while (1) {
        // Read command from standard input
        n = read(STDIN_FILENO, cmd, BUFSIZE);

        // Execute the command and measure its execution time
        Commande(cmd, n);

        write(STDOUT_FILENO, "% ", strlen("% "));
    }
}

/*
 * shell.c
 *
 * A simple shell interface program written in C.
 *
 * Nicholas Zajkeskovic
 * October 6, 2024
 * COMPSCI 3SH3
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80 // Maximum length of a command
#define MAX_HISTORY 5 // Maximum number of commands stored in history

char history[MAX_HISTORY][MAX_LINE]; // Array to store the command history
int history_length = 0; // Number of commands in history

// Adds a command to the history array
void add_command(char *arg) {
    // Do not add 'history' command itself to the history list
    if (strcmp(arg, "history") == 0) return; 

    // If history is full, shift commands to the left to make space
    if (history_length >= MAX_HISTORY) {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strcpy(history[MAX_HISTORY - 1], arg);
    } else {
        // Add the command to the history and increase history length
        strcpy(history[history_length], arg);
        history_length++;
    }
};

// Displays the last 5 commands entered in reverse order
void show_history(void) {
    for (int i = history_length - 1; i >= 0; i--) {
        printf("%d %s\n", i + 1, history[i]);
    }
};

// Executes the most recent command in history
void execute() {
    // Check if there are any commands in history
    if (history_length == 0) {
        printf("No commands in history.\n");
        return;
    }    
    // Create arguments for shell execution
    char *args[] = {"/bin/sh", "-c", history[history_length - 1], NULL};
    pid_t pid = fork();

    if (pid == 0) { 
        execvp(args[0], args);
        perror("Failed to execute command");
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    }
};

int main(void) {
    char args[MAX_LINE]; // Buffer to hold the user's command
    int should_run = 1; // Control flag to determine when to exit
    pid_t pid;

    while (should_run) {
        printf("osh> ");
        fflush(stdout);

        // Read user input
        if (fgets(args, MAX_LINE, stdin) == NULL) {
            perror("Failed to read user input");
            continue;
        }

        // Remove newline character from the command string
        args[strcspn(args, "\n")] = '\0';

        // If the user types 'exit', terminate the shell
        if (strcmp(args, "exit") == 0) {
            should_run = 0;
            break;
        }

        // If the user enters 'history', display command history
        if (strcmp(args, "history") == 0) {
            show_history();
            continue;
        }

        // If the user enters '!!', execute the last command in history
        if (strcmp(args, "!!") == 0) {
            execute();
            continue;
        }

        add_command(args); // Store the current command in history

        // Flag that checks if the command should run in the background
        int flag = 0;
        if (args[strlen(args) - 1] == '&') {
            flag = 1;
        }

        // Fork a child process to run the command
        pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            // Use a different variable to store arguments for command execution
            char *cmd_args[] = {"/bin/sh", "-c", args, NULL};
            if (execvp(cmd_args[0], cmd_args) == -1) {
                perror("Command execution failed");
            }
            exit(1);
        } else { 
            // If the command is not running in the background, wait for the child process
            if (!flag) {
                wait(NULL);
            }
        }
    }

    return 0;
};
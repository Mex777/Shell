#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <signal.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_BOLD "\x1b[1m"

#define MAX_SIZE 128
#define MAX_PIPES 10

static jmp_buf env;

// handle the SIGINT signal (Ctrl+C)
void handle_suspend(int signo) {
    printf("\n");
    siglongjmp(env, 42);
}

// for now splits the input by space
// puts the first token in the command pointer
// puts the other tokens in the args pointer array
// the args array will end with NULL
// returns the number of arguments the command has
int parse(char *input, char *command, char *args[16]) {
    char *token = strtok(input, " ");
    strcpy(command, token);

    int argCounter = 0;
    ++argCounter;
    while (token != NULL) {
        token = strtok(NULL, " ");
        if (token != NULL) {
            args[argCounter] = token;
            ++argCounter;
        }
    }

    args[argCounter] = NULL;

    return argCounter;
}

char *strip(char *str, char element) {
    int index = 0;
    while (index < strlen(str) && str[index] == element) {
        ++index;
    }

    // strips front
    for (int i = 0; i < strlen(str) - index + 1; ++i) {
        str[i] = str[i + index];
    }

    // strips back
    index = strlen(str) - 1;
    while (str[index] == element && index >= 0) {
        str[index] = '\0';
    }

    return str;
}

// checks for implementation in bin folder
// otherwise looks for implementation in PATH
void exec(char *command, char *args[16], int argc) {
    strip(command, ' ');
    for (int i = 1; i < argc; ++i) {
        strip(args[i], ' ');
    }

    char path[MAX_SIZE] = "../bin/";
    strcat(path, command);


    args[0] = command;
    if (execvp(path, args) == -1 && execvp(command, args) == -1) {
        perror("COMMAND");
    }
}

void exec_pipes(char *commands[MAX_PIPES], int num_pipes) {
    int pipefds[MAX_PIPES - 1][2];

    for (int i = 0; i < num_pipes - 1; ++i) {
        if (pipe(pipefds[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_pipes; ++i) {
        pid_t currPid = fork();

        if (currPid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (currPid == 0) {
            // redirect stdout to the write end of the next pipe
            if (i < num_pipes - 1) {
                dup2(pipefds[i][1], STDOUT_FILENO);
            }

            // redirect stdin to the read end of the previous pipe
            if (i > 0) {
                dup2(pipefds[i - 1][0], STDIN_FILENO);
            }

            // close all pipe ends in the child
            for (int j = 0; j < num_pipes - 1; ++j) {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            // execute the command
            char *command = commands[i];
            char *args[16];
            strip(command, ' ');
            int argc = parse(command, command, args);
            exec(command, args, argc);
            exit(1);
        }
    }

    // close all pipe ends in the parent
    for (int i = 0; i < num_pipes - 1; ++i) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    // wait for all child processes after they have been created
    for (int i = 0; i < num_pipes; ++i) {
        wait(NULL);
    }
}

int main() {
    char input[MAX_SIZE];
    char command[MAX_SIZE], *args[16];

    char hostname[MAX_SIZE];
    gethostname(hostname, sizeof(hostname));

    signal(SIGINT, handle_suspend);

    int totalBackground = 0;
    while (true) {
        sigsetjmp(env, 1);
        printf(ANSI_BOLD ANSI_COLOR_GREEN "%s@%s" ANSI_COLOR_RESET ":" ANSI_BOLD ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "$ ", getlogin() , hostname, getcwd(NULL, MAX_SIZE));

        fgets(input, MAX_SIZE, stdin);
        // ignores the case when the input is empty
        if (strcmp(input, "\n") == 0) {
            continue;
        }

        // removes "\n" from end of the line
        input[strlen(input) - 1] = '\0';
        strip(input, ' ');

        bool runInBackground = false;
        if (input[strlen(input) - 1] == '&') {
            runInBackground = true;
            ++totalBackground;
            input[strlen(input) - 1] = '\0';
        }

        // pipe operator logic
        if (strstr(input, "|") != NULL) {
            int num_pipes = 0;
            char *commands[MAX_PIPES];

            // split the input into commands based on pipe operator "|"
            char *token = strtok(input, "|");
            while (token != NULL && num_pipes < MAX_PIPES) {
                commands[num_pipes++] = strip(token, ' ');
                token = strtok(NULL, "|");
            }

            exec_pipes(commands, num_pipes);
            continue;
        }

        int argc = parse(input, command, args);

        if (strcmp(command, "cd") == 0) {
            if (chdir(args[1]) == -1) {
                perror("CD");
            }

            continue;
        }

        if (strcmp(command, "exit") == 0) {
            exit(0);
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(-1);
        }

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            exec(command, args, argc);
            exit(0);
        } else {
            if (runInBackground == false) {
                waitpid(pid, NULL, 0);
            } else {
                printf("[%d] %d\n", totalBackground, pid);
            }
        }
    }
    
    return 0;
}
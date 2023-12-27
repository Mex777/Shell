#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 128

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

// checks for implementation in bin folder
// otherwise looks for implementation in PATH
void exec(char *command, char *args[16], int argc) {
    char path[MAX_SIZE] = "../bin/";
    strcat(path, command);

    args[0] = command;
    if (execvp(path, args) == -1) {
        perror("COMMAND");
    }
}

int main() {
    char input[MAX_SIZE];
    char command[MAX_SIZE], *args[16];
    while (1) {
        fgets(input, MAX_SIZE, stdin);
        // ignores the case when the input is empty
        if (strcmp(input, "\n") == 0) {
            continue;
        }

        // removes "\n" from end of the line
        input[strlen(input) - 1] = '\0';

        int argc = parse(input, command, args);
        if (vfork() == 0) {
            exec(command, args, argc);
        }
    }

    return 0;
}

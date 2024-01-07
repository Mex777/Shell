#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/termios.h>

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

int loadingHistory(const char *historyPath, char lines[50][MAX_SIZE]) {
    FILE *file = fopen(historyPath, "r");
    if (file == NULL) {
        perror("Error opening history file");
        exit(-1);
    }

    char line[MAX_SIZE];
    int count = 0;
    int i = 0;
    int lastLines = 50;

    while (fgets(line, sizeof(line), file) != NULL) {
        count++;
    }

    fseek(file, 0, SEEK_SET);
    if (count > 50){
        for (i = 0; i < count - 50; i++){
            fgets(line, sizeof(line), file);
        }
    }

    if (count < lastLines){
        lastLines = count;
    }
    i=0;
    while (fgets(line, sizeof(line), file) != NULL && i < 50 && i < count) {
        if (line != "\n") {
            line[strlen(line) - 1] = '\0';
            strcpy(lines[lastLines - i - 1], line);
            i++;
        }
    }

    fclose(file);
    return lastLines;
}

void addToHistoryFile(const char *historyPath, char *command) {
    FILE *file = fopen(historyPath, "a");
    if (file == NULL) {
        perror("Error opening history file");
        exit(-1);
    }
    fprintf(file, "%s\n", command);
    fclose(file);
}

void setTerminalMode() {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &new_termios); // gets the current attributes of the terminal and stores them in new_termios
    new_termios.c_lflag &= ~(ICANON | ECHO); // turns of canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios); // sets the modified attributes
}

int main() {
    char command[MAX_SIZE], *args[16];

    setTerminalMode();

    char historyPath[MAX_SIZE];
    snprintf(historyPath, sizeof(historyPath), "%s/history.txt", getcwd(NULL, MAX_SIZE));
    char history[50][MAX_SIZE];
    int nrCommands = loadingHistory(historyPath, history); // the number of commands in the history array

    char hostname[MAX_SIZE];
    gethostname(hostname, sizeof(hostname));

    int totalBackground = 0;
    while (true) {
        char input[MAX_SIZE] = ""; 
        int inputLength = 0;
        int cursorPosition = 0;
        int currentCommand = -1; // in the history array
        int eraserCount = 0;

        while(1){
            int URILength = strlen(getlogin()) + strlen("@") + strlen(hostname) + strlen(":") + strlen(getcwd(NULL, MAX_SIZE)) + strlen("$ ");
            printf("\r" ANSI_BOLD ANSI_COLOR_GREEN "%s@%s" ANSI_COLOR_RESET ":" ANSI_BOLD ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "$ %-*s\033[%dG", getlogin(), hostname, getcwd(NULL, MAX_SIZE), inputLength + 1, input, cursorPosition + URILength + 1);
            // "\r" -> carriage return,  resets printing to the beginning of the line (overwriting what was already printed)
            // "%-*s" -> positions the input to the left ("-") of a width ("*") equal to inputLength + 1 ("s") 
            // "\033[%dG" -> Positions the cursor at position %d 
            if (eraserCount != 0) {
                for (int i = 0; i < eraserCount; i++){
                    printf(" ");
                }
                printf("\033[%dG", cursorPosition + URILength + 1);
                eraserCount = 0;
            } // for deleting the old input (what remains of it after reprtinting)

            char c = getchar();

            if (c == 27) {
                c = getchar();
                c = getchar();

                if (c == 'A') { // Up arrow
                    if (currentCommand < nrCommands - 1) {
                        currentCommand++;
                        if (strlen(history[currentCommand]) < inputLength) {
                            eraserCount = inputLength - strlen(history[currentCommand]);
                        }
                        strncpy(input, history[currentCommand], MAX_SIZE);
                        input[MAX_SIZE] = '\0';
                        inputLength = strlen(input);
                        cursorPosition = strlen(input);
                    }
                } else if (c == 'B') { // Down arrow
                    if (currentCommand > 0) {
                        currentCommand--;
                        if (strlen(history[currentCommand]) < inputLength ) {
                            eraserCount = inputLength - strlen(history[currentCommand]);
                        }
                        strncpy(input, history[currentCommand], MAX_SIZE);
                        input[MAX_SIZE] = '\0';
                        inputLength = strlen(input);
                        cursorPosition = strlen(input);
                    } else {
                        eraserCount = inputLength;
                        strncpy(input, "", MAX_SIZE);
                        inputLength = 0;
                        cursorPosition = 0;
                        if (currentCommand == 0){
                            currentCommand--;
                        }
                    }
                } else if (c == 'C') { // Right arrow
                    if (cursorPosition < inputLength) {
                        cursorPosition++;
                    }
                } else if (c == 'D') { // Left arrow
                    if (cursorPosition > 0) {
                        cursorPosition--;
                    }
                }
            } else if (c == 127 || c == 8) { // Backspace
                if (cursorPosition > 0) {
                    memmove(&input[cursorPosition - 1], &input[cursorPosition], inputLength - cursorPosition + 1);
                    cursorPosition--;
                    inputLength--;
                }
            } else if (c == '\n') { // Enter
                input[cursorPosition++] = c;
                printf("\n");
                break;
            } else if (inputLength < MAX_SIZE && c >= 32 && c <= 126) { // Restul caracterelor
                for (int i = inputLength; i > cursorPosition; i--){
                    input[i]=input[i - 1];
                }
                input[cursorPosition++] = c;
                inputLength++;
            }
        }

        // fgets(input, MAX_SIZE, stdin);
        // ignores the case when the input is empty
        if (strcmp(input, "\n") == 0) {
            continue;
        }

        // removes "\n" from end of the line
        input[strlen(input) - 1] = '\0';
        strip(input, ' ');

        addToHistoryFile(historyPath, input);
        for (int i = nrCommands - 1; i >= 0; i--){
            strcpy(history[i + 1], history[i]);
        }
        strcpy(history[0], input);
        if (nrCommands < 50){
            nrCommands++;
        }

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

        pid_t pid = vfork();

        if (pid < 0) {
            perror("fork");
            exit(-1);
        }

        if (pid == 0) {
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
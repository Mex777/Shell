#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("The directory name should be an argument\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        if (mkdir(argv[i], 0777) == -1) {
            printf("Couldn't create directory %s\n", argv[i]);
            exit(1);
        }

        printf("Directory created successfully\n");
    }

    return 0;
}
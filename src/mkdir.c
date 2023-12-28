#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv) {
	if (argc != 2) {
        printf("The directory name should be an argument\n");
        exit(1);
    }

    if (mkdir(argv[1], 0777) == -1) {
		printf("Couldn't create directory %s\n", argv[1]);
		exit(1);
	}

    printf("Directory created successfully\n");

    return 0;
}
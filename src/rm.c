#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("The file should be an argument\n");
        exit(1);
    }

    if (remove(argv[1])) {
        printf("Couldn't remove the file\n");
		exit(1);
	}

    printf("File removed\n");

    return 0;
}
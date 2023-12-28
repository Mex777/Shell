#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Directory should be an argument\n");
        exit(1);
    }

    if (rmdir(argv[1]) == -1) {
		printf("Couldn't remove the directory\n");
		exit(1);
	}

    printf("Removed directory\n");
    
    return 0;
}
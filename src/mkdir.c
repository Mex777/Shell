#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Eroare la crearea directorului\n");
        exit(1);
    }

    if (mkdir(argv[1], 0777) == -1) {
		printf("Eroare la mkdir\n");
		exit(1);
	}

    printf("Director creat\n");

    return 0;
}
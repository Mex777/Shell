#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Eroare la stergerea directorului\n");
        exit(1);
    }

    if(rmdir(argv[1]) == -1) {
		printf("Eroare la stergerea directorului\n");
		exit(1);
	}
    printf("Director sters\n");

    return 0;
}
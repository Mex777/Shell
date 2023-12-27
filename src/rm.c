#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Eroare la stergerea fisierului\n");
        exit(1);
    }

    if(remove(argv[1])) {
        printf("Eroare la stergerea fisierului\n");
		exit(1);
	}

    printf("Fisier sters\n");

    return 0;
}
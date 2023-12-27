#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("Eroare la copierea fisierului\n");
        exit(1);
    }

    char c;
	FILE *f1, *f2;
	
	if((f1 = fopen(argv[1], "r")) == NULL) {
		printf("Eroare la deschiderea fisierului\n");
		exit(1);
	}

	f2 = fopen(argv[2], "w");
	c = fgetc(f1);

	while(c != EOF) {
		fputc(c, f2);
		c = fgetc(f1);
	}

	fclose(f1);
	fclose(f2);

	return 0;
}
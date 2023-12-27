#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Eroare la schimbarea directorului\n");
        exit(1);
    }

	if(chdir(argv[1]) != 0) {
		printf("Eroare la schimbarea directorului\n");
        exit(1);
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Eroare la obtinerea directorului curent");
        exit(1);
    }

    printf("Directorul curent este: %s\n", cwd);

	return 0;
}
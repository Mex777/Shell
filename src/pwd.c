#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {

    char buffer[1024];
	if(getcwd(buffer, sizeof(buffer))==NULL)
        {
            printf("Eroare la getcwd\n");
            exit(1);
        }

    printf("%s\n", buffer);

    return 0;
}
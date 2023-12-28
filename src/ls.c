#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

int main(int argc, char **argv) {
	DIR *dir;
	struct dirent *entry;

	if (argc == 1) {
		char buffer[1024];
		if (getcwd(buffer, sizeof(buffer)) == NULL) {
			printf("Couldn't get the directory\n");
			exit(1);
		}

		dir = opendir(buffer);
		if (dir == NULL) {
            printf("Couldn't open the directory\n");
			exit(1);
		}

		while ((entry = readdir(dir)) != NULL) {
			printf("%s\n", entry->d_name);
		}
		
		closedir(dir);
	}

	else 
		for (int i = 1; i < argc; i++) {
			dir = opendir(argv[i]);

			if (dir == NULL) {
			printf("Couldn't open the directory\n");
			exit(1);
			}

			while ((entry = readdir(dir)) != NULL) {
			printf("%s\n", entry->d_name);
			}

			printf("\n");
			closedir(dir);
		}

	return 0;
}
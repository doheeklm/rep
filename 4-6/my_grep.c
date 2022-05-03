#include <stdio.h>
#include <fcntl.h> //open()
#include <unistd.h> //access()
#include <string.h> //strstr()

enum { BUFSIZE = 255 };

int main(int argc, char *argv[])
{
	const char* search = argv[1];
	const char* path = argv[2];

	if (argc != 3) {
		printf("인자를 잘 못 입력하였습니다.\n");
		return 0;
	}

	if (access(path, F_OK) == -1) {
		printf("/bin/grep: %s: No such file or directory\n", path);
		return 0;
	}

	FILE* fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		perror("error\n");
		return 0;
	}

	char buf[BUFSIZE];
	memset(buf, 0, sizeof(buf));
	char* temp;
	int line = 0;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		line++;
		temp = strstr(buf, search);		
		if (temp != NULL) {
			printf("%d: %s", line, buf);
		}
	}

	if (fclose(fp) != 0) {
		perror("close error\n");
		return 0;
	}

	return 0;
}

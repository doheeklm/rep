#include <stdio.h>
#include <fcntl.h> //open()
#include <unistd.h> //access()
#include <string.h> //strstr()
#include <unistd.h> //fork()
#include <wait.h> //wait() waitpid()
#include <sys/stat.h> //stat()
#include <stdlib.h> //exit()

enum { BUFSIZE = 255, LINES_PER_CHILD = 20 };

int main(int argc, char *argv[])
{
	const char *search = argv[1];
	const char *path = argv[2];
	const char *tempPath = "./temp.txt";

	if (access(path, F_OK) == -1) {
		printf("%s is not existed\n", path);
		return 0;
	}

	char buffer[BUFSIZE];
	int i = 0;
	int count = 0;
	int nChild = 0;
	int status = 0;
	pid_t pid;

	do {	
		pid = fork();

		if (pid == 0) {
			printf("Child %d\n", getpid());
			
			FILE* fp;
			fp = fopen(path, "r");
			if (fp == NULL) {
				perror("fopen()");
			}

			struct stat fileInfo;
			if (stat(path, &fileInfo) != 0) {
				perror("stat()");
			}

			off_t fileSize;
			fileSize = fileInfo.st_size;
			
			char c;
			int lines = 0;
			while ((c = fgetc(fp) != EOF)) {
				if (c == '\n') {
					lines++;
				}
			}
			nChild = lines / LINES_PER_CHILD; //자식 프로세스 갯수
			
			for (i = 0; i < LINES_PER_CHILD; i++) {
				memset(buffer, 0, sizeof(buffer));
				fseek(fp, /* offset */,SEEK_SET);
				fgets(buffer, sizeof(buffer), fp);

				printf("fp: %p ", fp);
				printf("ftell(fp): %ld ", ftell(fp));
				printf("%s", buffer);
			}

			//	if (strstr(buffer, search) != NULL) {
				//	fprintf(fp_search, "%d: %s", lineCheck + (i * 10), buffer);
			//	}

			exit(0);
		}
		else if (pid > 0) {
			printf("Parent %d\n", getpid());

			if (waitpid(pid, &status, 0) == -1) {
				perror("waitpid");
			}
			if (WIFEXITED(status) != 0) {
				printf("< %d 정상종료 >\n", pid[i]);
			}
		}
		else if (pid == -1) {
			perror("fork()");
		}

		count++;
		printf("count %d\n", count);
	} while (count >= nChild); // 수정

	fseek(fp_search, 0, SEEK_SET);
	printf("\nCurrent PID %d\n", getpid());

	char buf[BUFSIZE];
	int rep = 0;
	while (feof(fp_search) == 0) {
		memset(buf, 0, sizeof(buf));
		fread(buf, sizeof(buf), 1, fp_search);
		printf("%s", buf);
		rep++;
	}

	printf("rep %d\n", rep);
	
	struct stat fileInfo2;
	if (stat(tempPath, &fileInfo2) != 0) {
		perror("stat()");
	}
	
	if (fileInfo2.st_size == 0) {
		printf("\"%s\" not found in %s\n", search, path);
	}

	execl("/bin/rm", "rm", tempPath, NULL);

	if (fclose(fp) != 0) {
		perror("fclose()");
	}

	if (fclose(fp_search) != 0) {
		perror("fclose()");
	}

	return 0;
}

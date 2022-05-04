#include <stdio.h>
#include <fcntl.h> //open()
#include <unistd.h> //access()
#include <string.h> //strstr()
#include <unistd.h> //fork()
#include <wait.h> //wait() waitpid()
#include <sys/stat.h> //stat()
#include <stdlib.h> //exit()

enum { BUFSIZE = 255, LINES = 10 };

int main(int argc, char *argv[])
{
	printf("[ 현재 실행 중인 PID %d ]\n", getpid());

	const char *search = argv[1];
	const char *path = argv[2];
	const char *tempPath = "./search.txt";

	if (access(path, F_OK) == -1) {
		printf("%s is not existed\n", path);
		return 0;
	}

	char buffer[BUFSIZE];
	memset(buffer, 0, sizeof(buffer));

	FILE* fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		perror("fopen()");
	}

	struct stat fileInfo;
	if (stat(path, &fileInfo) != 0) {
		printf("can't read file info\n");
		return 0;
	}
	off_t fileSize;
	fileSize = fileInfo.st_size;
	printf("[ %s 파일 사이즈 %ld Byte ]\n", path, fileSize);
	
	FILE* fp_search;
	fp_search = fopen(tempPath, "w+");
	if (fp_search == NULL) {
		perror("fopen()");
	}
	
	char c;
	int line = 0;
	while ((c = fgetc(fp)) != EOF) {
		if (c == '\n') {
			line++;
		}
	}
	printf("[ 라인 갯수 %d ]\n", line);
	fseek(fp, 0, SEEK_SET);

	int count;
	count = (line / LINES) + 1; //LINES 줄마다 프로세스 1개
	pid_t pids[count];
	printf("[ 자식 프로세스 갯수 %d ]\n", count);
	printf("[ 프로세스당 읽어들이는 라인 갯수 %d]\n", LINES);

	int i;
	int lineCheck = 0;
	int status = 0;

	char buf[300];

	for (i = 0; i < count; i++) {
		pids[i] = fork();
		
		if (pids[i] == -1) {
			perror("fork()");
		}
		else if (pids[i] == 0) { //자식 프로세스
			printf("자식[%d], getpid %d\n", i, getpid());

			while (fgets(buffer, sizeof(buffer), fp) != NULL) {
				printf("%p %d %s", fp, ftell(fp), buffer);
				lineCheck++;
				if (lineCheck >= LINES) {
					break;
				}
					
				//확인
				if (strstr(buffer, search) != NULL) {
					fprintf(fp_search, "%d: %s", lineCheck, buffer);
				}
			}

			exit(0);
		}
		else { //부모 프로세스
				wait(NULL);		
				
				printf("자식[%d] 종료, getpid %d\n", i, getpid());

		}
	}

	if (fclose(fp) != 0) {
		perror("fclose()");
	}

	if (fclose(fp_search) != 0) {
		perror("fclose()");
	}

	return 0;
}

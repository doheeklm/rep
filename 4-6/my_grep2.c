#include <stdio.h>
#include <fcntl.h> //open()
#include <unistd.h> //access()
#include <string.h> //strstr()
#include <unistd.h> //fork()
#include <wait.h> //wait() waitpid()
#include <sys/stat.h> //stat()
#include <stdlib.h> //exit()

enum { BUFSIZE = 255 };

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

	int loop = 10;

	int count;
	count = (line / loop) + 1; //LINES 줄마다 프로세스 1개
	printf("[ 자식 프로세스 갯수 %d ]\n", count);
	printf("[ 프로세스당 읽어들이는 라인 갯수 %d]\n", loop);

	int i = 0;
	int lineCheck = 0;

	int status = 0;
	pid_t pid[count];

	for (i = 0; i < count; i++) {
		pid[i] = fork();

		if (pid[i] == 0) {
			printf("Child %d\n", getpid());
			
			while(fgets(buffer, sizeof(buffer), fp) != NULL) {
				//printf("fp: %p ", fp); //주소
				//printf("ftell(fp): %ld ", ftell(fp)); //스트림의 위치 지정자의 현재 위치
				//printf("%s", buffer);
				lineCheck++;
				
				if (lineCheck >= loop) {
					break;
				}

				if (strstr(buffer, search) != NULL) {
					fprintf(fp_search, "%d: %s", lineCheck + (i * 10), buffer);
				}
			}

			exit(0);
		}
		else if (pid[i] > 0) {
			printf("Parent %d\n", getpid());

			if (waitpid(pid[i], &status, 0) == -1) {
				perror("waitpid");
				exit(1);
			}
			if (WIFEXITED(status) != 0) {
				printf("pid %d 정상종료\n", pid[i]);
			}
		}
		else if (pid[i] == -1) {
			perror("fork()");
			exit(1);
		}
	}

	char buf[BUFSIZE];
	
	fseek(fp_search, 0, SEEK_SET);
	
	printf("\ncurrent %d\n", getpid());
	
	while (feof(fp_search) == 0) {
		memset(buf, 0, sizeof(buf));
		fread(buf, sizeof(buf), 1, fp_search);
		printf("%s", buf);
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

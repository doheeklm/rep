#include <stdio.h>
#include <fcntl.h> //open()
#include <unistd.h> //access()
#include <string.h> //strstr()
#include <unistd.h> //fork() unlink()
#include <wait.h> //wait() waitpid()
#include <sys/stat.h> //stat()
#include <stdlib.h> //exit()

enum { LINES_PER_CHILD = 20, BUFSIZE = 255 };

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("인자를 잘 못 입력하였습니다\n");
		return 0;
	}

	const char *search = argv[1];
	const char *path = argv[2];

	if (access(path, F_OK) == -1) {
		printf("%s is not existed\n", path);
		return 0;
	}

	/* 라인 갯수 및 자식 프로세스 갯수 확인 위해 파일 열기 */ 
	FILE* fp_init;
	fp_init = fopen(path, "r");
	if (fp_init == NULL) {
		perror("fopen()");
		exit(EXIT_FAILURE);
	}

	char buffer[BUFSIZE];
	memset(buffer, 0, sizeof(buffer));

	/* 라인 갯수 확인 */
	char c;
	int line = 0;
	while ((c = fgetc(fp_init)) != EOF) {
		if (c == '\n') {
			line++;
		}
	}
	printf("[%d 라인 갯수]\n", line);
	fseek(fp_init, 0, SEEK_SET);

	/* 자식 프로세스 갯수 */
	int nChild = 0;
	nChild = (line / LINES_PER_CHILD) + 1;
	printf("[%d 자식 프로세스 갯수]\n", nChild);

	int a = 0;
	int cnt = 0;
	int point[nChild];

	/* 파일포인터 위치 확인 */
	while ((fgets(buffer, sizeof(buffer), fp_init)) != NULL) {
		if (a == 0) { //0줄일때
			point[cnt] = 0;	
			printf("%d줄 파일포인터 %d\n", a, point[cnt]);
			cnt++;
		}
		else { //19, 39, 59
			if (a % LINES_PER_CHILD == LINES_PER_CHILD - 1) {
				point[cnt] = (int)ftell(fp_init);
				printf("%d줄 파일포인터 %d\n", a, point[cnt]);
				cnt++;
			}
		}
		a++;
	}

	/* 파일 닫기 */
	if (fclose(fp_init) != 0) {
		perror("fclose()");
		exit(EXIT_FAILURE);
	}
	
 	char filename[50];
	pid_t pid[nChild];

	int i = 0;
	for (i = 0; i < nChild; i++) {
		pid[i] = fork();

		if (pid[i] == 0) { /* 자식 프로세스 */
			printf("[%d 자식 프로세스]\n", getpid());
			
			int lineCheck = 1;
			
			FILE* fp;
			fp = fopen(path, "r");
			if (fp == NULL) {
				perror("fopen()");
				exit(EXIT_FAILURE);
			}

			/* 파일 포인터 위치 */
			fseek(fp, point[i], SEEK_SET);

			FILE* fp_temp;
			snprintf(filename, sizeof(filename), "./temp%d.txt", i);
			fp_temp = fopen(filename, "w+");
			if (fp_temp == NULL) {
				perror("fopen()");
				exit(EXIT_FAILURE);
			}

			while (fgets(buffer, sizeof(buffer), fp) != NULL) {
				/* 20줄 이상 읽으면 반복문 중지 */
				if (lineCheck > LINES_PER_CHILD) {
					break;
				}
				
				/* 문자열 검색 후 저장 */
				if (strstr(buffer, search) != NULL) {
					fprintf(fp_temp, "%d:%s", lineCheck + (i * LINES_PER_CHILD) , buffer);
				}

				lineCheck++;
			}

			if (fclose(fp) != 0) {
				perror("fclose()");
				exit(EXIT_FAILURE);
			}
			if (fclose(fp_temp) != 0) {
				perror("fclose()");
				exit(EXIT_FAILURE);
			}

			exit(0);
		}
		else if (pid[i] > 0) {
			
		}
		else if (pid[i] == -1) {
			perror("fork()");
			exit(EXIT_FAILURE);
		}
	}

	printf("[%d 프로세스]\n", getpid());

	/* 자식 프로세스 종료 회수 */
	int status = 0;
	for (i = 0; i < nChild; i++) {
		if (waitpid(pid[i], &status, 0) == -1) {
			perror("waitpid()");
			exit(EXIT_FAILURE);
		}
		if (WIFEXITED(status) != 0) {
			printf("[%d 자식 프로세스 정상 종료]\n", pid[i]);
		}
	}

	printf("\n");

	int size = 0;
	int total = 0;

	/* 임시 파일들 읽기 및 삭제 */
	for (i = 0; i < nChild; i++) {
		FILE* fp_final;
		snprintf(filename, sizeof(filename), "./temp%d.txt", i);
		fp_final = fopen(filename, "r");
		if (fp_final == NULL) {
			perror("fopen()");
			exit(EXIT_FAILURE);
		}

		struct stat s2;
		if (stat(filename, &s2) == -1) {
			perror("stat()");
			exit(EXIT_FAILURE);
		}
		
		size = (int)s2.st_size;
		printf("file size %d\n", size);
		
		total += size;

		while (fgets(buffer, sizeof(buffer), fp_final) != NULL) {
			printf("%s", buffer);	
		}

		if (fclose(fp_final) != 0) {
			perror("fclose()");
			exit(EXIT_FAILURE);
		}

		unlink(filename);
	}

	if (total == 0) {
		printf("\"%s\" not found in %s\n", search, path);
	}

	return 0;
}

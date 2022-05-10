#include <stdio.h>
#include <fcntl.h> //open()
#include <unistd.h> //access()
#include <string.h> //strstr()
#include <unistd.h> //fork() unlink()
#include <wait.h> //wait() waitpid()
#include <sys/stat.h> //stat()
#include <stdlib.h> //exit()

enum { LINES_PER_CHILD = 10, BUFSIZE = 255 };

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
	}

	/* 라인 갯수 확인 */
	char c;
	int line = 0;
	while ((c = fgetc(fp_init)) != EOF) {
		if (c == '\n') {
			line++;
		}
	}
	printf("[%d 라인 갯수]\n", line);

	/* 파일 닫기 */
	if (fclose(fp_init) != 0) {
		perror("fclose()");
	}
	
	/* 자식 프로세스 갯수 */
	int nChild = 0;
	nChild = (line / LINES_PER_CHILD) + 1;
	printf("[%d 자식 프로세스 갯수]\n", nChild);

	char filename[100];
	char buffer[BUFSIZE];
	pid_t pid[nChild];

	int i = 0;
	for (i = 0; i < nChild; i++) {
		pid[i] = fork();

		if (pid[i] == 0) { /* 자식 프로세스 */
			printf("[%d 자식 프로세스]\n", getpid());

			int lineCheck = 0;

			FILE* fp;
			fp = fopen(path, "r");
			if (fp == NULL) {
				perror("fopen()");
			}

			//fseek(fp, i * LINES_PER_CHILD, SEEK_SET);
			//fseek의 offset을 바이트로 계산해야하는데...

			FILE* fp_temp;
			snprintf(filename, sizeof(filename), "./temp%d.txt", i);
			fp_temp = fopen(filename, "w+");
			if (fp_temp == NULL) {
				perror("fopen()");
			}

			memset(buffer, 0, sizeof(buffer));
			while (fgets(buffer, sizeof(buffer), fp) != NULL) {
				printf("fp: %p ", fp);
				printf("ftell(fp): %ld ", ftell(fp));
				printf("%s", buffer);

				/* 10줄 이상 읽으면 반복문 중지 */
				if (lineCheck >= LINES_PER_CHILD) {
					break;
				}

				/* 문자열 검색결과  임시 파일에 저장 */
				if (strstr(buffer, search) != NULL) {
					fprintf(fp_temp, "%d: %s", lineCheck + (i * LINES_PER_CHILD), buffer);
				}

				lineCheck++;
			}

			if (fclose(fp) != 0) {
				perror("fclose()");
			}
			if (fclose(fp_temp) != 0) {
				perror("fclose()");
			}

			exit(0);
		}
		else if (pid[i] > 0) {
			
		}
		else if (pid[i] == -1) {
			perror("fork()");
		}
	}

	printf("[%d 프로세스]\n", getpid());

/*	int status = 0;
	while (wait(&status) != -1) {
		if (WIFEXITED(status) != 0) {
			printf("정상 종료\n");
		}
	};*/

	/* 자식 프로세스 종료 회수 */
	int status = 0;
	for (i = 0; i < nChild; i++) {
		if (waitpid(pid[i], &status, 0) == -1) {
			perror("waitpid()");
		}
		if (WIFEXITED(status) != 0) {
			printf("[%d 자식 프로세스  정상 종료]\n", pid[i]);
		}
	}

	/* 임시 파일들 읽기 및 삭제 */
	for (i = 0; i < nChild; i++) {
		FILE* fp_final;
		snprintf(filename, sizeof(filename), "./temp%d.txt", i);
		fp_final = fopen(filename, "r");
		if (fp_final == NULL) {
			perror("fopen()");
		}

		while (feof(fp_final) == 0) {
			memset(buffer, 0, sizeof(buffer));
			fread(buffer, sizeof(buffer), 1, fp_final);
			printf("%s", buffer);
		}

		if (fclose(fp_final) != 0) {
			perror("fclose()");
		}

		unlink(filename);
	}

	return 0;
}

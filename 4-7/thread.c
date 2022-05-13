#include <stdio.h>
#include <unistd.h> //access()
#include <pthread.h> //pthread_create(), pthread_join()
#include <stdlib.h> //exit()
#include <sys/stat.h> //stat()
#include <string.h> //strstr()
#include <errno.h>

enum { LINES_PER_CHILD = 20, BUFSIZE = 255 };

char* search;
char* path;

typedef struct Info
{
	int cnt;
	int ptr;
} Info;

void *t_function(void *data)
{
	Info *temp = (Info *)data;

	char filename[50];
	snprintf(filename, sizeof(filename), "./temp%d.txt", temp->cnt);

	printf("Thread #%d\n", temp->cnt);
	printf("Searching %s in %s [%d~]\n", search, path, temp->ptr);
	printf("Saving in %s..\n", filename);
	printf("=============================\n");

	FILE* fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		perror("fopen()");
		exit(EXIT_FAILURE);
	}
	fseek(fp, temp->ptr, SEEK_SET);

	FILE* fp_temp;
	fp_temp = fopen(filename, "w+");
	if (fp_temp == NULL) {
		perror("fopen()");
		exit(EXIT_FAILURE);
	}

	char buf[BUFSIZE];
	int lineCheck = 1;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (lineCheck > LINES_PER_CHILD) {
			break;
		}
		if (strstr(buf, search) != NULL) {
			fprintf(fp_temp, "%d:%s", lineCheck + (temp->cnt * LINES_PER_CHILD), buf);
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

	pthread_exit((void *)&temp->cnt);
	
	//return NULL;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("인자를 잘 못 입력하였습니다.\n");
		return 0;
	}

	if (access(argv[2], F_OK) == -1) {
		printf("%s is not existed\n", argv[2]);
		return 0;
	}

	/* 인자로 받은 문자열 & 파일 */
	search = argv[1];
	path = argv[2];

	/* 라인 갯수, 스레드 갯수, 파일의 포인터 위치 확인 위해 파일 열기 */
	FILE* fp_init;
	fp_init = fopen(argv[2], "r");
	if (fp_init == NULL) {
		perror("fopen()");
		exit(EXIT_FAILURE);
	}

	/* 라인 갯수 */
	char c;
	int line = 0;
	while ((c = fgetc(fp_init)) != EOF) {
		if (c == '\n') {
			line++;
		}
	}
	//printf("[%d 라인 갯수]\n", line);

	/* 파일 돌렸으니 위치 맨앞으로 변경 */
	fseek(fp_init, 0, SEEK_SET);

	/* 스레드 갯수 */
	int nThread = 0;
	nThread = (line / LINES_PER_CHILD) + 1;
	//printf("[%d 스레드 갯수]\n", nThread);

	int i = 0;
	int a = 0;
	int b = 0;
	int point[nThread]; //포인터 위치

	char buffer[BUFSIZE];
	memset(buffer, 0, sizeof(buffer));

	/* 라인 n줄(LINES_PER_CHILD)마다 포인터 위치 확인, point[]에 저장 */
	while (fgets(buffer, sizeof(buffer), fp_init) != NULL) {
		if (a == 0) {
			point[b] = 0;
			//printf("%d줄 파일포인터 %d\n", a, point[b]);
			b++;
		}
		else {
			if (a % LINES_PER_CHILD == LINES_PER_CHILD - 1) {
				point[b] = (int)ftell(fp_init);
				//printf("%d줄 파일포인터 %d\n", a, point[b]);
				b++;
			}
		}
		a++;
	}

	if (fclose(fp_init) != 0) {
		perror("fclose()");
		exit(EXIT_FAILURE);
	}

	/* 스레드 */
	pthread_t p_thread[nThread];

	/* 몇번째 스레드인지, 스레드 내 파일 위치 정보 */
	Info info[nThread];

	/* 스레드 nThread개 생성 */
	for (i = 0; i < nThread; i++) {
		info[i].cnt = i;
		info[i].ptr = point[i];

		if (pthread_create(&p_thread[i], NULL, t_function, (void *)&info[i]) != 0) {
			perror("pthread_create()");
			exit(EXIT_FAILURE);
		}
	}

	int status;
	/* 각 스레드 종료를 기다림 */
	for (i = 0; i < nThread; i++) {
		if (pthread_join(p_thread[i], (void *)&status) != 0) {
			perror("pthread_join()");
			exit(EXIT_FAILURE);
		}

		printf("Thread #%d [%d]\n", i, status);
	}
	
	char name[50];
	int size = 0;
	int total = 0;

	/* 최종 결과 화면 출력 및 임시 파일들 삭제 */
	for (i = 0; i < nThread; i++) {
		snprintf(name, sizeof(name), "./temp%d.txt", i);
		
		FILE* fp_final;
		fp_final = fopen(name, "r");
		if (fp_final == NULL) {
			perror("fopen()");
			exit(EXIT_FAILURE);
		}

		struct stat s2;
		if (stat(name, &s2) == -1) {
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

		unlink(name);
	}

	printf("total %d\n", total);
	if (total == 0) {
		printf("\"%s\" not found in %s\n", search, path);
	}

	return 0; 
}

#include <stdio.h>
#include <unistd.h> //access()
#include <pthread.h> //pthread..
#include <stdlib.h> //exit()
#include <string.h>
#include <errno.h>

enum { LINES_PER_CHILD = 20, BUFSIZE = 255 };

char* path;
char* search;
int i;

void *t_function(void *data)
{
	/*	int j = 0;
	while (j < 3) {
		printf("no.%d 쓰레드 : %d\n", info.num, j);
		j++;
		sleep(1);
	}*/
 
	i = *((int *)data);

	FILE* fp;
	fp = fopen(path, "r'");
	if (fp == NULL) {
		perror("fopen()");
	}
		
	fseek(fp, 0, SEEK_SET);

	char filename[50];
	snprintf(filename, sizeof(filename), "./temp%d.txt", 0);

	FILE* fp_temp;
	fp_temp = fopen(filename, "w+");
	if (fp_temp == NULL) {
		perror("fopen()");
	}

	char buf[BUFSIZE];

	int lineCheck = 1;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (lineCheck > LINES_PER_CHILD) {
			break;
		}
		if (strstr(buf, search) != NULL) {			
			printf("%d:", lineCheck + (i * LINES_PER_CHILD));
			printf("%s", buf);
			fprintf(fp_temp, "%d:%s", lineCheck + (i * LINES_PER_CHILD), buf);
		}
		lineCheck++;
	}

	if (fclose(fp) != 0) {
		perror("fclose()");
	}
	if (fclose(fp_temp) != 0) {
		perror("fclose()");
	}

	return NULL;
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

	search = argv[1];
	path = argv[2];

	FILE* fp_init;
	fp_init = fopen(argv[2], "r");
	if (fp_init == NULL) {
		perror("fopen()");
	}

	char c;
	int line = 0;
	while ((c = fgetc(fp_init)) != EOF) {
		if (c == '\n') {
			line++;
		}
	}
	printf("[%d 라인 갯수]\n", line);
	fseek(fp_init, 0, SEEK_SET);

	int nThread = 0;
	nThread = (line / LINES_PER_CHILD) + 1;
	printf("[%d Thread 갯수]\n", nThread);

	int a = 0;
	int b = 0;
	int point[nThread];

	char buffer[BUFSIZE];
	memset(buffer, 0, sizeof(buffer));

	while (fgets(buffer, sizeof(buffer), fp_init) != NULL) {
		if (a == 0) {
			point[b] = 0;
			printf("%d줄 파일포인터 %d\n", a, point[b]);
			b++;
		}
		else {
			if (a % LINES_PER_CHILD == LINES_PER_CHILD - 1) {
				point[b] = (int)ftell(fp_init);
				printf("%d줄 파일포인터 %d\n", a, point[b]);
				b++;
			}
		}
		a++;
	}

	if (fclose(fp_init) != 0) {
		perror("fclose()");
	}

	pthread_t p_thread[nThread];

	printf("Create Thread\n");

	for (i = 0; i < nThread; i++) {
		if (pthread_create(&p_thread[i], NULL, t_function, (void *)&i) != 0) {
			perror("pthread_create()");
			exit(EXIT_FAILURE);
		}
	}


	/*	for (i = 0; i < nThread; i++) {
		//예외처리
		pthread_detach(p_thread[i]);
		pause();
	}
*/

	int j;
	for (j = 0; j < nThread; j++) {
		pthread_join(p_thread[j], NULL);
	}

//	for (i = 0; i < nThread; i++) {
//		if (pthread_join(p_thread[i], (void **)&status) == 0) {

//		}
//		printf("%d status %d\n", i, status);
//	}

	return 0; 
}

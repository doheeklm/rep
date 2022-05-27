#include <stdio.h>
#include <pthread.h> //pthread_create() pthread_join()
#include <semaphore.h> 
#include <errno.h> //errno
#include <string.h> //strcmp()
#include <stdlib.h> //malloc() free()
#include <stdio_ext.h> //__fpurge()
#include <unistd.h> //sleep()

pthread_mutex_t mutex;

#define snprintf_nowarnings(...) (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

enum { EMPTY = 0, NOT_EMPTY = -1,
		ENQ = 0, ENQ_FAIL = -1,
		DEQ = 0, DEQ_FAIL = -1 };

typedef struct _INFO {
	char name[13];
	char phone[14];
	char address[151];
	struct _INFO *next;
} Info;

typedef struct _QUEUE {
	Info *front;
	Info *rear;
	int count;
} Queue;

Queue q;

void ClearStdin(char* c);
void Init(Queue *q);
int Enqueue(Queue *q, Info info);
int Dequeue(Queue *q, Info *info);
int isEmpty(Queue *q);
void *FileWrite(void *data);

int main()
{
	Init(&q);

	pthread_t thread_write;

	if (pthread_mutex_init(&mutex, NULL) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	if (pthread_create(&thread_write, NULL, FileWrite, (void *)&q) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	Info info;

	while (1) {
		memset(&info, 0, sizeof(info));
		char temp[200];

		printf("Name: ");
		if (fgets(temp, sizeof(temp), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
		
		if (strcmp("exit\n", temp) == 0) {
			printf("입력을 종료합니다.\n");
			break;
		}
		
		ClearStdin(temp);
		snprintf_nowarnings(info.name, 13, "%s", temp);

		printf("Phone Num: ");
		if (fgets(temp, sizeof(temp), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		ClearStdin(temp);
		snprintf_nowarnings(info.phone, 14, "%s", temp);
		
		if ((info.phone[3] != '-') || (info.phone[8] != '-')) {
			printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
			printf("이름을 다시 입력 받습니다.\n");
			continue;
		}
		
		printf("Address: ");
		if (fgets(temp, sizeof(temp), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		ClearStdin(temp);
		snprintf_nowarnings(info.address, 151, "%s", temp);

		if (pthread_mutex_lock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		if (Enqueue(&q, info) == ENQ_FAIL) {
			if (pthread_mutex_unlock(&mutex) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				break;
			}
			break;
		}

		if (pthread_mutex_unlock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
	}

	if (pthread_join(thread_write, (void **)NULL) != 0) {
		if (pthread_mutex_destroy(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}
	
	if (pthread_mutex_destroy(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	return 0;
}

void ClearStdin(char* c)
{
	if (c == NULL) {
		return;
	}
	if (c[strlen(c) - 1] == '\n') {
		c[strlen(c) - 1] = '\0';
	}
	__fpurge(stdin);
}

void Init(Queue *q)
{
	q->front = NULL;
	q->rear = NULL;
	q->count = 0;
}

int Enqueue(Queue *q, Info info)
{
	Info *i = (Info *)malloc(sizeof(Info));
	if (i == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		return ENQ_FAIL;
	}

	strncpy(i->name, info.name, sizeof(info.name));
	strncpy(i->phone, info.phone, sizeof(info.phone));
	strncpy(i->address, info.address, sizeof(info.address));
	i->next = NULL;

	if (isEmpty(q) == EMPTY) {
		q->front = i;
		q->rear = i;
	}
	else {
		q->rear->next = i;
		q->rear = i;
	}

	printf("Current Q count[%d]\nQ->FRONT[%s|%s|%s]\nQ->REAR[%s|%s|%s]\n",
			q->count,
			q->front->name, q->front->phone, q->front->address,
			q->rear->name, q->rear->phone, q->rear->address);

	q->count++;

	return ENQ;
}

int Dequeue(Queue *q, Info *info)
{
	if (isEmpty(q) == EMPTY) {
		return DEQ_FAIL;
	}

	Info *temp = q->front;
	q->front = q->front->next;

	strncpy(info->name, temp->name, sizeof(temp->name));
	strncpy(info->phone, temp->phone, sizeof(temp->phone));
	strncpy(info->address, temp->address, sizeof(temp->address));
	info->next = NULL;

	free(temp);
	q->count--;

	return DEQ;
}

int isEmpty(Queue *q)
{
	if (q->count == 0) {
		return EMPTY;
	}
	else {
		return NOT_EMPTY;
	}
}

void *FileWrite(void *data)
{
	while (1) {
		Info *temp_info = (Info *)malloc(sizeof(Info));
		if (temp_info == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		if (pthread_mutex_lock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		if (Dequeue(&q, temp_info) == DEQ_FAIL) {
			if (pthread_mutex_unlock(&mutex) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				break;
			}
			sleep(5);
			continue;
		}

		if (pthread_mutex_unlock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		FILE* fp = NULL;
		fp = fopen("./address.txt", "a+");
		if (fp == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		if (fwrite(temp_info, sizeof(Info), 1, fp) != 1) {
			if (fclose(fp) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				break;
			}
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		printf("[...Writing File...]\n");

		free(temp_info);

		if (fclose(fp) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
	}

	return NULL;
}

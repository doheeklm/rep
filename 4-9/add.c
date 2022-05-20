#include <stdio.h>
#include <pthread.h> //pthread_create() pthread_join()
#include <semaphore.h> 
#include <errno.h> //errno
#include <string.h> //strcmp()
#include <stdlib.h> //malloc() free()

#define snprintf_nowarnings(...) (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

enum { Q_SIZE = 2, EMPTY = 0, FULL = 0, FALSE = -1 };

typedef struct _INFO {
	char name[13];
	char phone[14];
	char address[151];
} Info;

typedef struct _QUEUE {
	int front;
	int rear;
	Info *info;
} Queue;

void Init(Queue *q);
void Enqueue(Queue *q, Info temp);
Info Dequeue(Queue *q);
int isEmpty(Queue *q);
void *FileWrite(void *data);

int main()
{	
	Queue q;
	Init(&q);
	
	Info info;

	while (1) {
		memset(&info, 0, sizeof(info));

		char temp[200];

		printf("Name: ");
		if (fgets(temp, sizeof(temp), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			//thread join
			//return
		}
	
		if (strcmp("exit\n", temp) == 0) {
			printf("입력을 종료합니다.\n");
			break;
		}

		size_t i = 0;
		size_t j = 0;
		for (i = 0; i < sizeof(temp); i++) {
			if (temp[i] != ' ') {
				temp[j] = temp[i];
				j++;
			}
		}
		snprintf_nowarnings(info.name, 13, "%s", temp);

		printf("Phone Num: ");
		if (fgets(temp, sizeof(temp), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			//thread join
			//return
		}
		snprintf_nowarnings(info.phone, 14, "%s", temp);
		
		if ((info.phone[3] != '-') || (info.phone[8] != '-')) {
			printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
			printf("이름을 다시 입력 받습니다.\n");
			continue;
		}
		
		printf("Address: ");
		if (fgets(temp, sizeof(temp), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			//thread join
			//return
		}
		snprintf_nowarnings(info.address, 151, "%s", temp);

		Enqueue(&q, info);
	}

	pthread_t thread_write;

	if (pthread_create(&thread_write, NULL, FileWrite, (void *)NULL) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	if (pthread_join(thread_write, (void **)NULL) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	return 0;
}

void Init(Queue *q)
{
	q->front = 0;
	q->rear = 0;
	
	q->info = (Info *)malloc(sizeof(Info *) * Q_SIZE);
	if (q->info == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		//thread join
		//return
	}
}

void Enqueue(Queue *q, Info temp)
{
	q->rear = (q->rear + 1) % Q_SIZE;
	q->info[q->rear] = temp;
}

Info Dequeue(Queue *q)
{
	if (isEmpty(q) == EMPTY) {
		return NULL;
	}
	else {
		q->front = (q->front + 1) % Q_SIZE;
	}
}

int isEmpty(Queue *q)
{
	if (q->front == q->rear) {
		return EMPTY;
	}
	else {
		return FALSE;
	}
}

void *FileWrite(void *data)
{
	//lock 걸기
	//Dequeue한 값을 파일에 Write
	//lock 풀기

	int i = 0;
	while (1) {
		printf("%d\n", i);

		if (i == 10) {
			printf("탈출\n");
			break;
		}

		i++;
	}

	return NULL;
}


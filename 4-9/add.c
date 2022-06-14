#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>	
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <unistd.h>

pthread_mutex_t mutex;

static sem_t sem;

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
int endFlag = 1;

const char* str_exit = "exit";			//Name에 exit을 입력받으면 프로그램 종료
void ClearStdin(char* c);				//버퍼 삭제
void Init(Queue *q);					//큐 초기화
int Enqueue(Queue *q, Info *pInfo);		//스레드1 내부에서 호출 (메인)
int Dequeue(Queue *q, Info **ppInfo);	//스레드2 내부에서 호출
int isEmpty(Queue *q);					//비어있는 큐 확인
void cleanup_handler(void *arg);
void *fWrite(void *data);				//스레드2에서 호출하는 함수

int main()
{
	pthread_t tWrite;

	Init(&q); //큐 초기화

	Info *pInfo = NULL;

	if (sem_init(&sem, 0, 0) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		return 0;
	}

	if (pthread_mutex_init(&mutex, NULL) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT0;
	}
	
	if (pthread_create(&tWrite, NULL, fWrite, (void *)NULL) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT1;
	}
	
	while (1) {	
		pInfo = (Info *)malloc(sizeof(Info));
		if (pInfo == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT2;
		}
		
		do {
			memset(pInfo, 0, sizeof(Info));

			printf("Name: ");
			if (fgets(pInfo->name, sizeof(pInfo->name), stdin) == NULL) {
				fprintf(stderr, "errno[%d]", errno);
				goto EXIT2;
			}
			ClearStdin(pInfo->name);

			if (strcmp(str_exit, pInfo->name) == 0) {
				printf("입력을 종료합니다.\n");
				endFlag = 0;
				if (sem_post(&sem) != 0) {
					fprintf(stderr, "errno[%d]", errno);
				}
				goto EXIT2;
			}

			printf("Phone Num: ");
			if (fgets(pInfo->phone, sizeof(pInfo->phone), stdin) == NULL) {
				fprintf(stderr, "errno[%d]", errno);
				goto EXIT2;
			}
			ClearStdin(pInfo->phone);

			if ((pInfo->phone[3] != '-') || (pInfo->phone[8] != '-')) {
				printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
				printf("처음으로 돌아갑니다.\n");
			}
		} while ((pInfo->phone[3] != '-') || (pInfo->phone[8] != '-'));

		printf("Address: ");
		if (fgets(pInfo->address, sizeof(pInfo->address), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT2;
		}
		ClearStdin(pInfo->address);

		if (Enqueue(&q, pInfo) == ENQ_FAIL) {
			printf("Enqueue Fail\n");
			goto EXIT2;
		}

		if (sem_post(&sem) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT2;
		}
	}

EXIT2:
	if (pInfo != NULL) {
		free(pInfo);
		printf("pInfo free 완료\n");
	}

	if (pthread_join(tWrite, NULL) != 0) {
		fprintf(stderr, "errno[%d]", errno);
	}
	else {
		printf("[thread 종료 대기]\n");
	}
	
EXIT1:
	if (pthread_mutex_destroy(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
	}

EXIT0:
	if (sem_destroy(&sem) != 0) {
		fprintf(stderr, "errno[%d]", errno);
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

int Enqueue(Queue *q, Info *pInfo)
{
	if (pthread_mutex_lock(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		return ENQ_FAIL;
	}

	//큐가 비어있을 땐, front와 rear 동일하게 정보 입력
	if (isEmpty(q) == EMPTY) {
		q->front = pInfo;
		q->rear = pInfo;
	}
	else { //큐가 비어있지 않을 땐, next를 가리키는 포인터에 정보 입력
		//큐가 하나 증가하기 때문에 rear은 rear의 next로 변경함 
		q->rear->next = pInfo;
		q->rear = q->rear->next;
	}

	(q->count)++;

	if (pthread_mutex_unlock(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		return ENQ_FAIL;
	}

	return ENQ;
}

int Dequeue(Queue *q, Info **ppInfo)
{
	if (pthread_mutex_lock(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		return DEQ_FAIL;
	}

	if (ppInfo != NULL) {
		//큐의 맨앞 원소를 넣어줌
		*ppInfo = q->front;
		
		//큐의 맨앞은 큐의 맨앞다음으로 바꿈
		q->front = q->front->next;
		
		//큐 갯수 1감소
		(q->count)--;
	}

	if (pthread_mutex_unlock(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		return DEQ_FAIL;
	}

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

void *fWrite(void *data)
{
	while (1) {
		//세마포어 값 1 감소 (스레드2 대기상태)
		if (sem_wait(&sem) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}		

		if (endFlag == 0) {
			pthread_exit((void *)0);
		}

		Info *tempInfo = NULL;
		if (Dequeue(&q, &tempInfo) == DEQ_FAIL) {
			printf("Dequeue Fail\n");
			break;
		}

		if (tempInfo != NULL) {	
			FILE* fp = NULL;
			fp = fopen("./address.txt", "a");
			if (fp == NULL) {
				fprintf(stderr, "errno[%d]", errno);
				continue;
			}

			//파일에 구조체 통으로 입력하기
			if (fwrite(tempInfo, sizeof(Info), 1, fp) != 1) {
				fprintf(stderr, "errno[%d]", errno);
			}

			//파일 닫기
			if (fclose(fp) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				break;
			}
		}
	}
	
	return NULL;
}

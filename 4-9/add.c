#include <stdio.h>
#include <pthread.h>	//pthread_create() pthread_join() pthread_mutex_init() pthread_mutex_destroy()
						//pthread_mutex_lock() //pthread_mutex_unlock()
#include <semaphore.h>	//sem_init() sem_getvalue() sem_wait() sem_post() sem_destroy()
#include <errno.h>		//errno
#include <string.h>		//strcmp()
#include <stdlib.h>		//malloc() free()
#include <stdio_ext.h>	//__fpurge()
#include <unistd.h>		//sleep() for testing

pthread_mutex_t mutex;

static sem_t sem_one;
static sem_t sem_two;

enum { EMPTY = 0, NOT_EMPTY = -1,
		ENQ = 0, ENQ_FAIL = -1,
		DEQ = 0, DEQ_FAIL = -1 };

//주소록 정보
typedef struct _INFO {
	char name[13];
	char phone[14];
	char address[151];
	struct _INFO *next;
} Info;

//큐
typedef struct _QUEUE {
	Info *front;
	Info *rear;
	int count;
} Queue;

Queue q;
Info *pInfo = NULL;

const char* EXIT = "exit\n";			//Name에 exit을 입력받으면 프로그램 종료

void ClearStdin(char* c);				//버퍼 삭제
void Init(Queue *q);					//큐 초기화
int Enqueue(Queue *q, Info *pInfo);		//스레드1 내부에서 호출 (메인)
int Dequeue(Queue *q, Info **ppInfo);	//스레드2 내부에서 호출
int isEmpty(Queue *q);					//비어있는 큐 확인
void *fWrite(void *data);				//스레드2에서 호출하는 함수

int main()
{
	pthread_t tWrite;

	Init(&q);

	//세마포어 초기화
	if (sem_init(&sem_one, 0, 0) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}
	if (sem_init(&sem_two, 0, 1) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	//뮤텍스 초기화
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}
	
	//스레드2 생성 (인자로 포인터 변수 pInfo를 넘겨줌)
	if (pthread_create(&tWrite, NULL, fWrite, (void *)pInfo) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	while (1) {	
		//sem_two -1
		if (sem_wait(&sem_two) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
		
		pInfo = (Info *)malloc(sizeof(Info));
		if (pInfo == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
		memset(pInfo, 0, sizeof(Info));

		//이름 입력받기
		printf("Name: ");
		if (fgets(pInfo->name, sizeof(pInfo->name), stdin) == NULL) {
			free(pInfo);
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
	
		if (strcmp(EXIT, pInfo->name) == 0) {
			printf("입력을 종료합니다.\n");
			break;
		}
		ClearStdin(pInfo->name);

		//전화번호 입력받기
		printf("Phone Num: ");
		if (fgets(pInfo->phone, sizeof(pInfo->phone), stdin) == NULL) {
			free(pInfo);
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
		if ((pInfo->phone[3] != '-') || (pInfo->phone[8] != '-')) {
			printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
			printf("처음으로 돌아갑니다.\n");
			//sem_one +1
			if (sem_post(&sem_one) != 0) {
				free(pInfo);
				fprintf(stderr, "errno[%d]", errno);
				break;
			}
			free(pInfo);
			continue;
		}
		ClearStdin(pInfo->phone);

		//주소 입력받기
		printf("Address: ");
		if (fgets(pInfo->address, sizeof(pInfo->address), stdin) == NULL) {
			free(pInfo);
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
		ClearStdin(pInfo->address);

		//Enqueue - 큐에 주소록 정보 저장
		if (Enqueue(&q, pInfo) == ENQ_FAIL) {
			printf("Enqueue Fail\n");
			break;
		}
		printf("<T1> [%s][%s][%s]\n", pInfo->name, pInfo->phone, pInfo->address);
	
		//sem_one +1
		if (sem_post(&sem_one) != 0) {
			free(pInfo);
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
	}

	//스레드2 종료 기다리는 중
	if (pthread_join(tWrite, (void **)NULL) != 0) {
		if (pthread_mutex_destroy(&mutex) != 0) {
			if (sem_destroy(&sem_one) != 0) {
				if (sem_destroy(&sem_two) != 0) {
					free(pInfo);
					fprintf(stderr, "errno[%d]", errno);
					exit(EXIT_FAILURE);
				}
				free(pInfo);
				fprintf(stderr, "errno[%d]", errno);
				exit(EXIT_FAILURE);
			}
			free(pInfo);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
		free(pInfo);
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	//뮤텍스 소멸
	if (pthread_mutex_destroy(&mutex) != 0) {
		if (sem_destroy(&sem_one) != 0) {
			if (sem_destroy(&sem_two) != 0) {
				free(pInfo);
				fprintf(stderr, "errno[%d]", errno);
				exit(EXIT_FAILURE);
			}
			free(pInfo);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
		free(pInfo);
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	//세마포어 제거
	if (sem_destroy(&sem_one) != 0) {
		if (sem_destroy(&sem_two) != 0) {
			free(pInfo);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}	
		free(pInfo);
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

int Enqueue(Queue *q, Info *pInfo)
{
	//뮤텍스 락
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

	//큐 갯수 1증가
	(q->count)++;

	//뮤텍스 언락
	if (pthread_mutex_unlock(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		return ENQ_FAIL;
	}

	return ENQ;
}

int Dequeue(Queue *q, Info **ppInfo)
{
	//뮤텍스 락
	if (pthread_mutex_lock(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		return DEQ_FAIL;
	}

	Info **pptemp = ppInfo;
	if (pptemp != NULL) {
		*pptemp = q->front;
		q->front = q->front->next;
		(q->count)--;
	}

	//뮤텍스 언락
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
		//sem_one -1
		//스레드 대기상태 진입
		if (sem_wait(&sem_one) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		//스레드 함수 fWrite에 받은 인자 data는 전역변수 Info *pInfo와 동일함
		Info *ptrInfo = (Info *)data;

		//Dequeue 함수의 인자로 ptrInfo의 주소를 넘겨줌
		if (Dequeue(&q, &ptrInfo) == DEQ_FAIL) {
			printf("Dequeue Fail\n");
			return NULL;
		}

		if (ptrInfo != NULL) {	
			//파일 열기
			//a : (쓰기 전용) 파일이 없으면 생성하고, 있으면 파일 포인터가 파일 끝에 위치함
			FILE* fp = NULL;
			fp = fopen("./address.txt", "a");
			if (fp == NULL) {
				free(ptrInfo);
				fprintf(stderr, "errno[%d]", errno);
				break;
			}

			//파일에 구조체 통으로 입력하기
			if (fwrite(ptrInfo, sizeof(Info), 1, fp) != 1) {
				if (fclose(fp) != 0) {
					free(ptrInfo);
					fprintf(stderr, "errno[%d]", errno);
					break;		
				}
					free(ptrInfo);
				fprintf(stderr, "errno[%d]", errno);
				break;
			}

			//파일 닫기
			if (fclose(fp) != 0) {
				free(ptrInfo);
				fprintf(stderr, "errno[%d]", errno);
				break;
			}

			printf("<T2> [%s][%s][%s]\n", ptrInfo->name, ptrInfo->phone, ptrInfo->address);
			free(ptrInfo);
	
			//sem_two +1
			if (sem_post(&sem_two) != 0) {
				free(pInfo);
				fprintf(stderr, "errno[%d]", errno);
				break;
			}		
		}

	} //while

	return NULL;
}

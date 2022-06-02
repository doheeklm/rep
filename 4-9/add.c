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

static sem_t sem;

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

	Init(&q);

	//세마포어 초기화
	if (sem_init(&sem, 0, 0) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	//뮤텍스 초기화
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		if (sem_destroy(&sem) != 0) {
			fprintf(stderr, "errno[%d]", errno);
		}
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}
	
	//스레드2 생성 (인자로 포인터 변수 pInfo를 넘겨줌)
	if (pthread_create(&tWrite, NULL, fWrite, (void *)pInfo) != 0) {
		if (pthread_mutex_destroy(&mutex) != 0) {
			if (sem_destroy(&sem) != 0) {
				fprintf(stderr, "errno[%d]", errno);
			}
			fprintf(stderr, "errno[%d]", errno);
		}
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	while (1) {	
		//pInfo 동적할당
		pInfo = (Info *)malloc(sizeof(Info));
		if (pInfo == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT;
		}
		
		do {
			//pInfo 초기화
			memset(pInfo, 0, sizeof(Info));

			//이름 입력받기
			printf("Name: ");
			if (fgets(pInfo->name, sizeof(pInfo->name), stdin) == NULL) {
				fprintf(stderr, "errno[%d]", errno);
				goto EXIT;
			}
			ClearStdin(pInfo->name);

			//exit 입력받으면 프로그램 종료
			if (strcmp(str_exit, pInfo->name) == 0) {
				printf("입력을 종료합니다.\n");
				goto EXIT;
			}

			//전화번호 입력받기
			printf("Phone Num: ");
			if (fgets(pInfo->phone, sizeof(pInfo->phone), stdin) == NULL) {
				free(pInfo);
				fprintf(stderr, "errno[%d]", errno);
				goto EXIT;
			}
			ClearStdin(pInfo->phone);

			//입력한 전화번호 형태가 잘못된 경우, 에러 문구를 출력하고 이름부터 다시 입력 받음
			if ((pInfo->phone[3] != '-') || (pInfo->phone[8] != '-')) {
				printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
				printf("처음으로 돌아갑니다.\n");
			}
		} while ((pInfo->phone[3] != '-') || (pInfo->phone[8] != '-'));

		//주소 입력받기
		printf("Address: ");
		if (fgets(pInfo->address, sizeof(pInfo->address), stdin) == NULL) {
			free(pInfo);
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT;
		}
		ClearStdin(pInfo->address);

		//Enqueue - 큐에 주소록 정보 저장
		if (Enqueue(&q, pInfo) == ENQ_FAIL) {
			printf("Enqueue Fail\n");
			goto EXIT;
		}
		printf("<T1> [%s][%s][%s]\n", pInfo->name, pInfo->phone, pInfo->address);

		//세마포어 값 1 증가
		if (sem_post(&sem) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT;
		}
	}

EXIT:
	//세마포어 값 1 증가 안 한 상태라서 스레드2는 계속 대기중..
	//스레드2에서 자원 회수할건 pInfo 밖에 없음

	//join하기 전에 "exit"을 pInfo->name에 입력하고, 스레드2 내에서 break하는 방법??

	free(pInfo);

	//스레드 취소 요청
	if (pthread_cancel(tWrite) != 0) {
		fprintf(stderr, "errno[%d]", errno);
	}
	
	//뮤텍스 소멸
	if (pthread_mutex_destroy(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
	}

	//세마포어 제거
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

	//개행문자 널문자로 변환
	if (c[strlen(c) - 1] == '\n') {
		c[strlen(c) - 1] = '\0';
	}

	//버퍼 비우기
	__fpurge(stdin);
}

void Init(Queue *q)
{
	//큐 초기화
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

	if (ppInfo != NULL) {
		//큐의 맨앞 원소를 넣어줌
		*ppInfo = q->front;
		
		//큐의 맨앞은 큐의 맨앞다음으로 바꿈
		q->front = q->front->next;
		
		//큐 갯수 1감소
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
		//세마포어 값 1 감소 (스레드2 대기상태)
		if (sem_wait(&sem) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}		

		//스레드 함수 fWrite에 받은 인자 data는 전역변수 Info *pInfo와 동일함
		Info *ptrInfo = (Info *)data;

		//Dequeue 함수의 인자로 ptrInfo의 주소를 넘겨줌
		if (Dequeue(&q, &ptrInfo) == DEQ_FAIL) {
			printf("Dequeue Fail\n");
			break;
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
		}
	}
	
	return NULL;
}

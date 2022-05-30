#include <stdio.h>
#include <pthread.h> //pthread_create() pthread_join()
#include <semaphore.h> 
#include <errno.h> //errno
#include <string.h> //strcmp()
#include <stdlib.h> //malloc() free()
#include <stdio_ext.h> //__fpurge()
#include <unistd.h> //sleep()

pthread_mutex_t mutex;

// GCC에서 형식 잘림 경고를 피하는 방법
#define snprintf_nowarnings(...) (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

enum { EMPTY = 0, NOT_EMPTY = -1,
		ENQ = 0, ENQ_FAIL = -1,
		DEQ = 0, DEQ_EMPTY = -1, DEQ_EXIT = -2 };

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

//전역으로 선언
Queue q;

//문자열 입력받을 임시버퍼 (exit을 스레드 함수에 전달)
char temp[200];
const char* EXIT = "exit\n";

void ClearStdin(char* c);			//버퍼 삭제
void Init(Queue *q);				//큐 초기화
int Enqueue(Queue *q, Info info);	//스레드1 내부에서 호출 (메인)
int Dequeue(Queue *q, Info *info);	//스레드2 내부에서 호출
int isEmpty(Queue *q);
void *FileWrite(void *data);		//스레드2에서 호출하는 함수

int main()
{
	Init(&q);

	pthread_t thread_write;

	//뮤텍스 초기화
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	//스레드2 생성
	if (pthread_create(&thread_write, NULL, FileWrite, (void *)temp) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	Info info;

	while (1) {
		memset(&info, 0, sizeof(info));

		//이름 입력받기
		printf("Name: ");
		if (fgets(temp, sizeof(temp), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
		if (strcmp(EXIT, temp) == 0) {
			printf("입력을 종료합니다.\n");
			break;
		}
		ClearStdin(temp);
		snprintf_nowarnings(info.name, 13, "%s", temp);

		//전화번호 입력받기
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
		
		//주소 입력받기
		printf("Address: ");
		if (fgets(temp, sizeof(temp), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
		ClearStdin(temp);
		snprintf_nowarnings(info.address, 151, "%s", temp);

		//뮤텍스 락
		if (pthread_mutex_lock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		//큐에 주소록 정보 저장
		if (Enqueue(&q, info) == ENQ_FAIL) {
			if (pthread_mutex_unlock(&mutex) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				break;
			}
			break;
		}

		//뮤텍스 언락
		if (pthread_mutex_unlock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
	}

	//스레드2 종료 대기
	if (pthread_join(thread_write, (void **)NULL) != 0) {
		if (pthread_mutex_destroy(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	//뮤텍스 소멸
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
	//큐에 넣어줄 Info 동적할당
	Info *i = (Info *)malloc(sizeof(Info));
	if (i == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		return ENQ_FAIL;
	}

	//입력받은 정보들 복사
	strncpy(i->name, info.name, sizeof(info.name));
	strncpy(i->phone, info.phone, sizeof(info.phone));
	strncpy(i->address, info.address, sizeof(info.address));
	i->next = NULL;

	//큐가 비어있을 땐, front와 rear 동일하게 정보 입력
	if (isEmpty(q) == EMPTY) {
		q->front = i;
		q->rear = i;
	}
	else { //큐가 비어있지 않을 땐, next를 가리키는 포인터에 정보 입력
		//큐가 하나 증가하기 때문에 rear은 rear의 next로 변경함 
		q->rear->next = i;
		q->rear = i; //q->rear = q->rear->next;
	}

	//확인용 (스레드2에서 실시간으로 디큐하기 때문에 대부분 count는 0으로 확인됨)
	printf("Q->COUNT[%d]\nQ->FRONT[%s|%s|%s]\nQ->REAR[%s|%s|%s]\n",
			q->count, q->front->name, q->front->phone, q->front->address,
			q->rear->name, q->rear->phone, q->rear->address);

	//큐 갯수 1증가
	q->count++;

	return ENQ;
}

int Dequeue(Queue *q, Info *info)
{
	if (isEmpty(q) == EMPTY) {
		return DEQ_EMPTY;
	}

	//아래에서 q->front->next가 필요하기 때문에 임시로 temp 포인터 변수를 선언함
	Info *temp = q->front;

	//인자로 받은 Info *info에 복사된 정보를 입력함 
	strncpy(info->name, temp->name, sizeof(temp->name));
	strncpy(info->phone, temp->phone, sizeof(temp->phone));
	strncpy(info->address, temp->address, sizeof(temp->address));
	info->next = NULL;

	//큐 원소 하나를 삭제하기 때문에 front는 front의 next로 변경함
	q->front = q->front->next;

	//임시 포인터 변수는 해제함
	free(temp);

	//큐 갯수 1감소
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
	//data는 exit 입력 확인용
	char *temp_char = (char *)data;

	while (1) {
		//exit 입력 받으면 스레드 종료
		 if (strcmp(temp_char, EXIT) == 0) {
			printf("스레드를 종료합니다.\n");
			pthread_exit(NULL);
		}

		//Dequeue 함수에 인자로 넘겨주기 위해 동적으로 할당함
		Info *temp_info = (Info *)malloc(sizeof(Info));
		if (temp_info == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		//뮤텍스 락
		if (pthread_mutex_lock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		//큐가 비어있으면, 뮤텍스 언락 + 할당해제 + 파일 write 안하고 continue
		//큐가 비어있지 않으면, temp_info에 주소록 정보 입력됨
		if (Dequeue(&q, temp_info) == DEQ_EMPTY) {
			if (pthread_mutex_unlock(&mutex) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				break;
			}	
			free(temp_info);
			continue;
		}
	
		//뮤텍스 언락
		if (pthread_mutex_unlock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		//파일 열기
		//a : (쓰기 전용) 파일이 없으면 생성하고, 있으면 파일 포인터가 파일 끝에 위치함
		FILE* fp = NULL;
		fp = fopen("./address.txt", "a");
		if (fp == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		//파일에 구조체 통으로 입력하기
		if (fwrite(temp_info, sizeof(Info), 1, fp) != 1) {
			if (fclose(fp) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				break;
			}
			free(temp_info);
			fprintf(stderr, "errno[%d]", errno);
			break;
		}

		free(temp_info);

		//파일 닫기
		if (fclose(fp) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			break;
		}
	}

	return NULL;
}

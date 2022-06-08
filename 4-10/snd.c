/* snd.c */
#include <stdio.h>
#include <string.h> //strcmp()
#include <errno.h> //errno
#include <stdlib.h> //exit()
#include <sys/types.h> //msgget() msgsnd() msgrcv() msgctl() ftok()
#include <sys/ipc.h> //msgget() msgsnd() msgrcv() msgctl() ftok()
#include <sys/msg.h> //msgget() msgsnd() msgrcv() msgctl()
#include <pthread.h> //pthread_mutex_init() pthread_mutex_lock() pthread_mutex_unlock() pthread_mutex_destroy()
#include <semaphore.h> //sem_init() sem_getvalue() sem_wait() sem_post() sem_destroy()
#include <stdlib.h> //malloc() free()
#include <stdio_ext.h> //__fpurge()
#include <unistd.h> //getpid()

pthread_mutex_t mutex; //뮤텍스 객체
static sem_t sem; //세마포어 정수 변수

typedef struct _MSG {
	/* 메시지 타입 */
	long mtype;
	/* 메시지 데이터 */
	char name[13]; //이름 한글 최대 4자
	char phone[14]; //전화번호 xxx-xxxx-xxxx
	char address[151]; //주소 한글 최대 50자
} Msg;

const char* str_exit = "exit"; //Name에 exit을 입력 받으면 프로그램 종료
void ClearStdin(char* c); //버퍼 삭제

int main()
{
	Msg msg;
//	msg.mtype = getpid();

	int msg_size = 0;
	msg_size = sizeof(msg) - sizeof(msg.mtype);

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

	do {
		//뮤텍스 락
		if (pthread_mutex_lock(&mutex) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT;
		}
	
		memset(&msg, 0, sizeof(msg));

		//이름 입력받기
		printf("Name: ");
		if(fgets(msg.name, sizeof(msg.name), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT;
		}
		ClearStdin(msg.name);

		//exit 입력받으면 프로그램 종료
		if (strcmp(str_exit, msg.name) == 0) {
			printf("입력을 종료합니다.\n");
			goto EXIT;
		}

		//전화번호 입력받기
		printf("Phone Num: ");
		if (fgets(msg.phone, sizeof(msg.phone), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT;
		}
		ClearStdin(msg.phone);

		//입력한 전화번호 형태가 잘못된 경우, 에러 문구를 출력하고 이름부터 다시 입력 받음
		if ((msg.phone[3] != '-') || (msg.phone[8] != '-')) {
			printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
			printf("처음으로 돌아갑니다.\n");
		}
	} while ((msg.phone[3] != '-') || (msg.phone[8] != '-'));

	//주소 입력받기
	printf("Address: ");
	if (fgets(msg.address, sizeof(msg.address), stdin) == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}
	ClearStdin(msg.address);
	
	//뮤텍스 언락
	if (pthread_mutex_unlock(&mutex) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}

	//키 생성
	key_t key = 0;
	if ((key = ftok(".", 'A')) == -1) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	int qid = 0; //메세지 큐 식별자
	qid = msgget(key, IPC_CREAT | 0666);

	if (msgsnd(qid, (void *)&msg, msg_size, IPC_NOWAIT) == -1) {
		fprintf(stderr, "errno[%d]", errno);
	}

	//세마포어 값 1 증가
	if (sem_post(&sem) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}

EXIT:
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

	//개행문자를 널문자로 변환
	if (c[strlen(c) - 1] == '\n') {
		c[strlen(c) - 1] = '\0';
	}

	//버퍼 비우기
	__fpurge(stdin);
}

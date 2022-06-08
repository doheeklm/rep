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

//pthread_mutex_t mutex; //뮤텍스 객체 변수
//static sem_t sem; //세마포어 객체 변수

//enum { EMPTY = 0, NOT_EMPTY = -1 };

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
	memset(&msg, 0, sizeof(msg));

	msg.mtype = getpid();

	int msg_size = 0;
	msg_size = sizeof(msg) - sizeof(msg.mtype);

	//이름 입력받기
	printf("Name: ");
	if(fgets(msg.name, sizeof(msg.name), stdin) == NULL) {
		fprintf(stderr, "errno[%d]", errno);
	}
	ClearStdin(msg.name);

	//전화번호 입력받기
	printf("Phone Num: ");
	if (fgets(msg.phone, sizeof(msg.phone), stdin) == NULL) {
		fprintf(stderr, "errno[%d]", errno);
	}
	ClearStdin(msg.phone);

	//주소 입력받기
	printf("Address: ");
	if (fgets(msg.address, sizeof(msg.address), stdin) == NULL) {
		fprintf(stderr, "errno[%d]", errno);
	}
	ClearStdin(msg.address);
	
	//키 생성
	key_t key = 0;
	if ((key = ftok(".", 'A')) == -1) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	int qid = 0;
	qid = msgget(key, IPC_CREAT | 0666);

	if (msgsnd(qid, (void *)&msg, msg_size, IPC_NOWAIT) == -1) {
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

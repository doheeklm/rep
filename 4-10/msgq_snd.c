/* msgq_snd.c */
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

	do {
		//이름 입력받기
		printf("Name: ");
		if(fgets(msg.name, sizeof(msg.name), stdin) == NULL) {
			fprintf(stderr, "msg.name/errno[%d]", errno);
			goto EXIT;
		}
		ClearStdin(msg.name);

		//전화번호 입력받기
		printf("Phone Num: ");
		if (fgets(msg.phone, sizeof(msg.phone), stdin) == NULL) {
			fprintf(stderr, "msg.phone/errno[%d]", errno);
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
		fprintf(stderr, "msg.address/errno[%d]", errno);
		goto EXIT;
	}
	ClearStdin(msg.address);

	//키 생성
	key_t key = 0;
	if ((key = ftok(".", 'A')) == -1) {
		fprintf(stderr, "ftok/errno[%d]", errno);
		goto EXIT;
	}

	int qid = 0;
	qid = msgget(key, IPC_CREAT | 0666);
	if (qid == -1) {
		fprintf(stderr, "msgget/errno[%d]", errno);
		goto EXIT;
	}

	if (msgsnd(qid, (void *)&msg, msg_size, IPC_NOWAIT) == -1) {
		fprintf(stderr, "msgsnd/errno[%d]", errno);
		goto EXIT;
	}

EXIT:
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

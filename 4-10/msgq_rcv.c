/* msgq_rcv.c */
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
#include <unistd.h> //getpid() fork()
#include <wait.h> //wait() waitpid()
#include <sys/stat.h> //stat()

typedef struct _MSG {
	/* 메시지 타입 */
	long mtype;
	/* 메시지 데이터 */
	char name[13]; //이름 한글 최대 4자
	char phone[14]; //전화번호 xxx-xxxx-xxxx
	char address[151]; //주소 한글 최대 50자
} Msg;

int main()
{
	printf("\n[rcv process]\n");
	
	Msg msg;
	memset(&msg, 0, sizeof(msg));

	int msg_size = 0;
	msg_size = sizeof(msg) - sizeof(msg.mtype);
	//msg.mtype = getpid();

	//키 생성
	key_t key = 0;
	if ((key = ftok(".", 'A')) == -1) {
		fprintf(stderr, "ftok/errno[%d]", errno);
		goto EXIT;
	}
	else {
		printf("key[%d]\n", key);
	}

	//메시지 큐 식별자
	int qid = 0;
	qid = msgget(key, IPC_CREAT | 0666);
	if (qid == -1) {
		fprintf(stderr, "msgget/errno[%d]", errno);
		goto EXIT;
	}

	struct msqid_ds buf;
	if (msgctl(qid, IPC_STAT, &buf) == -1) {
		fprintf(stderr, "msgctl/errno[%d]", errno);
		goto EXIT;
	}

	int nMsg = 0;
	nMsg = buf.msg_qnum;
while (1) {
	if (nMsg != 0) { //메세지 큐 안에 메시지 갯수가 0이 아닐 때
		ssize_t nbytes = 0;
		nbytes = msgrcv(qid, (void *)&msg, msg_size, 0, IPC_NOWAIT);
		//4번째 인자 0 : msgtype 메시지 큐에서 첫번째 메세지를 수신
		if (nbytes == -1) {
			fprintf(stderr, "msgrcv/errno[%d]", errno);
			goto EXIT;
		}
		
		msg.mtype = 0; //0으로 설정해야 파일 write할때 NULL이 들어감

		FILE* fp = NULL;
		fp = fopen("./address_msgq.txt", "a");
		if (fp == NULL) {
			fprintf(stderr, "fopen/errno[%d]", errno);
			goto EXIT;
		}
	
		if (fwrite(&msg, sizeof(msg), 1, fp) != 1) {
			fprintf(stderr, "fwrite/errno[%d]", errno);
		}

		if (fclose(fp) != 0) {
			fprintf(stderr, "fclose/errno[%d]", errno);
			goto EXIT;
		}

		break;
	}
}
	
	//메시지 큐를 제거하고 관련 데이터 구조체를 제거한다
	if (msgctl(qid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "msgctl/errno[%d]", errno);
		goto EXIT;
	}

EXIT:
	
	return 0;
}

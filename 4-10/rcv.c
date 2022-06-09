/* rcv.c */
#include <stdio.h>
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
#include <string.h> //memset()

typedef struct {
	/* 메시지 타입 */
	long mtype;
	/* 메시지 데이터 */
	char name[13]; //이름 한글 최대 4자
	char phone[14]; //전화번호 xxx-xxxx-xxxx
	char address[151]; //주소 한글 최대 50자
} Msg;

int main()
{
	Msg msg;
	memset(&msg, 0, sizeof(msg));

	int msg_size = 0;
	msg_size = sizeof(msg) - sizeof(msg.mtype);
	//msg.mtype = getpid();

	//키 생성
	key_t key = 0;
	if ((key = ftok(".", 'A')) == -1) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}
	else {
		printf("key[%d]\n", key);
	}

	int qid = 0; //메시지 큐 식별자
	qid = msgget(key, IPC_CREAT | 0666);

	ssize_t nbytes = 0;
	nbytes = msgrcv(qid, (void *)&msg, msg_size, 0, IPC_NOWAIT); //0 => msgtype? PID?

	if (nbytes > 0) {
		printf("SUCCESS: message bytes[%ld]\n", nbytes);
	}
	else {
		printf("FAIL: message bytes[%ld]\n", nbytes);
		goto EXIT;
	}

	struct msqid_ds buf;
	
	//현재 메시지 큐의 정보를 buf로 지정한 메모리에 저장함
	if (msgctl(qid, IPC_STAT, &buf) == -1) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;	
	}

	printf("msgrcv가 성공하면 메시지 갯수는 1 감소함:msg_qnum값[%ld]\n", buf.msg_qnum);
	msg.mtype = 0;

	FILE* fp = NULL;
	fp = fopen("./add.txt", "a");
	if (fp == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}

	if (fwrite(&msg, sizeof(msg), 1, fp) != 1) {
		fprintf(stderr, "errno[%d]", errno);
	}

	if (fclose(fp) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}

	//메시지 큐를 제거하고 관련 데이터 구조체를 제거한다
	if (msgctl(qid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXit;
	}

EXIT:
	return 0;
}

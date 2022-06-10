/* msgq_rcv.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct _MSG {
	long mtype;
	char name[13];
	char phone[14];
	char address[151];
} Msg;

int main()
{
	Msg msg;

	int msg_size = 0;
	msg_size = sizeof(msg) - sizeof(msg.mtype);
	
	key_t key = 0;
	if ((key = ftok(".", 'A')) == -1) {
		fprintf(stderr, "ftok/errno[%d]", errno);
		return 0;
	}

	int qid = 0;
	qid = msgget(key, IPC_CREAT | 0666);
	if (qid == -1) {
		fprintf(stderr, "msgget/errno[%d]", errno);
		return 0;
	}

	struct msqid_ds buf; //메시지 큐 정보
	pid_t pidSnd = 0;
	ssize_t nBytes = 0;
	int nMsg = 0;

	struct stat st;
	char* str;

	while (1) {
		memset(&msg, 0, sizeof(msg));
		if (msgctl(qid, IPC_STAT, &buf) == -1) {
			fprintf(stderr, "msgctl/errno[%d]", errno);
			return 0;
		}
		nMsg = buf.msg_qnum;

		nBytes = msgrcv(qid, (void *)&msg, msg_size, 0, 0);
		if (nbytes == -1) {
			fprintf(stderr, "msgrcv/errno[%d]", errno);
			break;
		}
		msg.mtype = 0; //0으로 설정해야 파일 write할때 NULL이 들어감

		FILE* fp = NULL;
		fp = fopen("./address_msgq.txt", "a");
		if (fp == NULL) {
			fprintf(stderr, "fopen/errno[%d]", errno);
			break;
		}

		if (fwrite(&msg, sizeof(msg), 1, fp) != 1) {
			fprintf(stderr, "fwrite/errno[%d]", errno);
		}
	
		if (fclose(fp) != 0) {
			fprintf(stderr, "fclose/errno[%d]", errno);
			break;
		}

		pidSnd = buf.msg_lspid;
		sprintf(str, "/proc/%d", (int)pidSnd); 
		if (stat(str, &st) == -1 && errno == ENOENT) {
			printf("exit process\n");
			break;
		}
	}

	if (msgctl(qid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "msgctl/errno[%d]", errno);
	}

	return 0;
}

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

	ssize_t nBytes = 0;

	while (1) {
		memset(&msg, 0, sizeof(msg));
		msg.mtype = 3;

		nBytes = msgrcv(qid, (void *)&msg, msg_size, msg.mtype, 0);
		if (nBytes == -1) {
			if (errno == EIDRM) { //errno43
				printf("메시지 큐를 삭제했으므로 종료합니다.\n");
				return 0;
			} 
			fprintf(stderr, "msgrcv/errno[%d]", errno);
			break;
		}

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
	}

	return 0;
}

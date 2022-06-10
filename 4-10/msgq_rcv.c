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
	memset(&msg, 0, sizeof(msg));

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

	struct msqid_ds buf;
	pid_t pidSnd = 0;

	ssize_t nBytes = 0;
	int nMsg = 0;

	struct stat st;
	char* str;

	while (1) {
		//0: 메세지가 올때까지 기다린다
		nBytes = msgrcv(qid, (void *)&msg, msg_size, (long)pidSnd, 0);
		if (nBytes == -1) {
			if (errno == EIDRM) { //errno43
				printf("메시지 큐를 삭제했으므로 종료합니다.\n");
				return 0;
			} 
			fprintf(stderr, "msgrcv/errno[%d]", errno);
			break;
		}

		//메시지 큐 정보를 buf에 저장
		if (msgctl(qid, IPC_STAT, &buf) == -1) {
			fprintf(stderr, "msgctl/errno[%d]", errno);
			return 0;
		}

		nMsg = buf.msg_qnum; //메시지 갯수
		if (nMsg == 0) { //메시지를 receive 했다면, 메시지 큐의 갯수는 0
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

		pidSnd = buf.msg_lspid; //최근 msgsnd한 pid
		printf("pidSnd[%d]\n", pidSnd);


	}

	return 0;
}

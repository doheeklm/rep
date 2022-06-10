/* msgq_snd.c */
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

const char* str_exit = "exit";
void ClearStdin(char* c);

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

	while (1) {
		memset(&msg, 0, sizeof(msg));

		msg.mtype = getpid();
		printf("mtype[%ld]\n", msg.mtype);

		do {
			printf("Name: ");
			if(fgets(msg.name, sizeof(msg.name), stdin) == NULL) {
				fprintf(stderr, "msg.name/errno[%d]", errno);
				goto EXIT;
			}
			ClearStdin(msg.name);

			if (strcmp(str_exit,msg.name) == 0) {
				return 0;
			}
			
			printf("Phone Num: ");
			if (fgets(msg.phone, sizeof(msg.phone), stdin) == NULL) {
				fprintf(stderr, "msg.phone/errno[%d]", errno);
				goto EXIT;
		 	}
 			ClearStdin(msg.phone);

			if ((msg.phone[3] != '-') || (msg.phone[8] != '-')) {
				printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
				printf("처음으로 돌아갑니다.\n");
			}
		} while ((msg.phone[3] != '-') || (msg.phone[8] != '-'));

		printf("Address: ");
		if (fgets(msg.address, sizeof(msg.address), stdin) == NULL) {
			fprintf(stderr, "msg.address/errno[%d]", errno);
			goto EXIT;
		}
		ClearStdin(msg.address);

		if (msgsnd(qid, (void *)&msg, msg_size, 0) == -1) {
			fprintf(stderr, "msgsnd/errno[%d]", errno);
			goto EXIT;
		}
	}

EXIT:
	if (msgctl(qid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "msgctl/errno[%d]", errno);
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

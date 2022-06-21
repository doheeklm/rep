/* tcp_client.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h> //socket() connect() send() send()
#include <sys/socket.h> //socket() connect() inet_addr() send()
#include <unistd.h> //close()
#include <arpa/inet.h> //htons() htonl()

#define PORT 7777

typedef struct _Data {
	char name[13];
	char phone[14];
	char address[151];
} Data; 

const char* str_exit = "exit\n";
void ClearStdin(char* c);

int main()
{
	Data data;

	int cSockFd = 0;
	ssize_t snd = 0;
	struct sockaddr_in sAddr;
	socklen_t sAddrSize = sizeof(sAddr);

	//소켓 생성
	if ((cSockFd = (socket(PF_INET, SOCK_STREAM, 0))) == -1) {
		fprintf(stderr, "socket|errno[%d]", errno);
		return 0;
	}

	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(PORT);
	if (sAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]", errno);
		goto EXIT;
	}
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (sAddr.sin_addr.s_addr == -1) {
		fprintf(stderr, "htonl|errno[%d]", errno);
		goto EXIT;
	}

	//서버로 연결 시도
	if (connect(cSockFd, (struct sockaddr*)&sAddr, sAddrSize) == -1) {
		fprintf(stderr, "connect|errno[%d]", errno);
		goto EXIT;
	}
		
	while (1) {
		memset(&data, 0, sizeof(data));

		do {
			printf("Name: ");
			if (fgets(data.name, sizeof(data.name), stdin) == NULL) {
				fprintf(stderr, "fgets|errno[%d]", errno);
				goto EXIT;
			}
			ClearStdin(data.name);

			if (strcmp(str_exit, data.name) == 0) {
				goto EXIT;
			}

			printf("Phone Num: ");
			if (fgets(data.phone, sizeof(data.phone), stdin) == NULL) {
				fprintf(stderr, "fgets|errno[%d]", errno);
				goto EXIT;
			}
			ClearStdin(data.phone);

			if ((data.phone[3] != '-') || (data.phone[8] != '-')) {
				printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
				printf("처음으로 돌아갑니다.\n");
			}
		} while ((data.phone[3] != '-') || (data.phone[8] != '-'));

		printf("Address: ");
		if (fgets(data.address, sizeof(data.address), stdin) == NULL) {
			fprintf(stderr, "fgets|errno[%d]", errno);
			goto EXIT;
		}
		ClearStdin(data.address);

		printf("%s/%s/%s", data.name, data.phone, data.address);

		snd = send(cSockFd, &data, sizeof(data), 0);
		if (snd == -1) {
			fprintf(stderr, "send|errno[%d]", errno);
			goto EXIT;
		}
		printf("send[%ld]\n", snd);
	}

EXIT:
	if (shutdown(cSockFd, SHUT_RDWR) == -1) {
		fprintf(stderr, "shutdown|errno[%d]", errno);
	}

	if (close(cSockFd) == -1) {
		fprintf(stderr, "close|errno[%d]", errno);	
	}
	
	return 0;
}

void ClearStdin(char* c)
{
	if (c == NULL) {
		return;
	}

	if (c[strlen(c) - 1] == '\n') {
		c[strlen(c) - 1] == '\0';
	}

	__fpurge(stdin);
}

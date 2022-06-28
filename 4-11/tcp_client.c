/* tcp_client.c */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7777

typedef struct _Data {
	char name[13];
	char phone[14];
	char address[151];
} Data; 

const char* str_exit = "exit";
void ClearStdin(char* c);

int main()
{
	Data data;

	int cSockFd = 0;
	struct sockaddr_in sAddr;
	socklen_t sAddrSize = sizeof(sAddr);

	cSockFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (cSockFd == -1) {
		fprintf(stderr, "socket|errno[%d]\n", errno);
		return 0;
	}

	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(PORT);
	if (sAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]\n", errno);
		goto EXIT;
	}

	//sAddr.sin_addr.s_addr = inet_addr("170.20.234.142");
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (sAddr.sin_addr.s_addr == -1) {
		fprintf(stderr, "inet_addr|errno[%d]\n", errno);
		goto EXIT;
	}

	if (connect(cSockFd, (struct sockaddr*)&sAddr, sAddrSize) == -1) {
		fprintf(stderr, "connect|errno[%d]\n", errno);
		goto EXIT;
	}

	printf("cSockFd[%d]\n", cSockFd);	
	printf("입력해야할 바이트 수[%ld]\n", sizeof(data));

	while (1) {
		memset(&data, 0, sizeof(data));

		do {
			printf("Name: ");
			if (fgets(data.name, sizeof(data.name), stdin) == NULL) {
				fprintf(stderr, "fgets|errno[%d]\n", errno);
				goto EXIT;
			}
			ClearStdin(data.name);
			
			if (strcmp(str_exit, data.name) == 0) {
				goto EXIT;
			}

			printf("Phone Num: ");
			if (fgets(data.phone, sizeof(data.phone), stdin) == NULL) {
				fprintf(stderr, "fgets|errno[%d]\n", errno);
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
			fprintf(stderr, "fgets|errno[%d]\n", errno);
			goto EXIT;
		}
		ClearStdin(data.address);

		ssize_t wr = 0;
		ssize_t totalWr = 0;

		while (1) {
			wr = write(cSockFd, &data + totalWr, sizeof(data) - totalWr);
			if (wr == -1) {
				fprintf(stderr, "write|errno[%d]\n", errno);
				goto EXIT;
			}
			printf("write[%ld]\n", wr);
			
			totalWr += wr;
			printf("totalWr[%ld]\n", totalWr);
			
			if (totalWr == sizeof(data)) {
				printf("totalWr[%ld]:데이터 송신\n", totalWr);
				break;
			}
		}
	}

EXIT:
	if (close(cSockFd) == -1) {
		fprintf(stderr, "close|errno[%d]\n", errno);	
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

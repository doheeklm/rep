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

const char* str_exit = "exit";
void ClearStdin(char* c);

int main()
{
	Data data;

	int cSockFd = 0;
	struct sockaddr_in sAddr;
	socklen_t sAddrSize = sizeof(sAddr);

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
	
	//[O] TODO INADDR_ANY 사용하지 않고 ifconfig 통해 IP주소 확인 후 변환
	//sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	const char* addr = "127.0.0.1";
	unsigned int conv_addr = inet_addr(addr);
	if (conv_addr == -1) {
		fprintf(stderr, "inet_addr|errno[%d]", errno);
			goto EXIT;
	}
	sAddr.sin_addr.s_addr = conv_addr;

	//---->[errno 98]:address already in use
	const int flag = 1;
	int SetSockOpt = setsockopt(cSockFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
	//setsockopt(소켓번호, 옵션의 종류, 설정을 위한 소켓 옵션의 번호, 설정값이 저장된 주소값, 그 주소값 버퍼의 크기)
	if (SetSockOpt == -1) {
		fprintf(stderr, "setsockopt|errno[%d]", errno);
		goto EXIT;
	}

	//[O] TODO connect 이후 sockfd 값 표기
	if (connect(cSockFd, (struct sockaddr*)&sAddr, sAddrSize) == -1) {
		fprintf(stderr, "connect|errno[%d]", errno);
		goto EXIT;
	}
	printf("cSockFd[%d]\n", cSockFd);
	
	ssize_t wr = 0;

	printf("입력해야할 바이트 수[%ld]\n", sizeof(data));

	while (1) {
		memset(&data, 0, sizeof(data));

		//[X] TODO write
		do {
			printf("Name: ");
			if (fgets(data.name, sizeof(data.name), stdin) == NULL) {
				fprintf(stderr, "fgets|errno[%d]", errno);
				goto EXIT;
			}
			ClearStdin(data.name);
			//[O] TODO 이름 버퍼 수정 - 오타였음..
			
			if (strcmp(str_exit, data.name) == 0) {
				goto EXIT;
				//[O] TODO 서버에서 종료처리 - 읽은 데이터가 0일 때 종료
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

		printf("%s|%s|%s\n", data.name, data.phone, data.address);

		wr = write(cSockFd, &data, sizeof(data));
		if (wr == -1) {
			fprintf(stderr, "write|errno[%d]", errno);
			goto EXIT;
		}
		printf("write[%ld]\n", wr);
	}

EXIT:
	//[O] TODO shutdown 제거

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
		c[strlen(c) - 1] = '\0';
	}

	__fpurge(stdin);
}

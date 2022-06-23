/* tcp_server.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h> //socket() bind() listen() accept() recv()
#include <sys/socket.h> //socket() bind() listen() accept() recv()
#include <unistd.h> //close() fcntl()
#include <fcntl.h> //fcntl()
#include <arpa/inet.h> //htons() htonl()

#define PORT 7777
#define MAX_PENDING 5

typedef struct _DATA {
	char name[13];
	char phone[14];
	char address[151];
} Data;

int main()
{
	Data data;

	int sSockFd;
	int cSockFd;
	int Bind;
	
	struct sockaddr_in sAddr;
	struct sockaddr_in cAddr; 
	
	socklen_t sAddrSize = 0;
	socklen_t cAddrSize = 0;

	sAddrSize = sizeof(sAddr);
	cAddrSize = sizeof(cAddr);

	sSockFd = socket(PF_INET, SOCK_STREAM, 0);
	if (sSockFd == -1) {
		fprintf(stderr, "socket|errno[%d]\n", errno);
		return 0;
	}

	memset(&sAddr, 0, sAddrSize);
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(PORT); //포트 번호 바이트오더링
	if (sAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]\n", errno);
		goto EXIT;
	}

	//[O] TODO INADDR_ANY 사용하지 않고  ifconfig 통해 내 IP주소 확인하기
	//const char* addr = "170.20.225.235";
	//unsigned int conv_addr = inet_addr(addr);
	//if (conv_addr == -1) {
	//	fprintf(stderr, "inet_addr|errno[%d]", errno);
	//	goto EXIT;
	//}
	//sAddr.sin_addr.s_addr = conv_addr;
	//---->[errno 99]:cannot assign requested address
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (sAddr.sin_addr.s_addr == -1) {
		fprintf(stderr, "htonl|errno[%d]\n", errno);
		goto EXIT;
	}
	
	Bind = bind(sSockFd, (struct sockaddr*)&sAddr, sAddrSize);
	if (Bind == -1) {
		fprintf(stderr, "bind|errno[%d]\n", errno);
		goto EXIT;
	}

	if (listen(sSockFd, MAX_PENDING) == -1) {
		fprintf(stderr, "listen|errno[%d]\n", errno);
		goto EXIT;
	}

	//[O] TODO accept 이후 sockfd 값 표기
	if ((cSockFd = accept(sSockFd, (struct sockaddr*)&cAddr, &cAddrSize)) == -1) {
		fprintf(stderr, "accept|errno[%d]\n", errno);
		goto EXIT;
	}
	printf("cSockFd[%d]\n", cSockFd);

	printf("입력받아야 할 바이트 수[%ld]\n", sizeof(data));

	while (1) {
		memset(&data, 0, sizeof(data));

		ssize_t rd = 0;
		ssize_t totalRd = 0;

		//[O] TODO read
		while (1) {
			rd = read(cSockFd, &data + totalRd, sizeof(data) - totalRd);
			if (rd == -1) {
				fprintf(stderr, "read|errno[%d]\n", errno);
				goto EXIT;
			}
			else if (rd > 0 && rd < sizeof(data)) {
				printf("read[%ld]\n", rd);
				totalRd += rd;
				printf("totalRd[%ld]\n", totalRd);
				continue;
			}
			else if (rd == sizeof(data)) {
				printf("read[%ld]:데이터 수신|파일 작성중\n", rd);
				break;
			}
			else if (rd == 0) {
				printf("read[exit]:프로그램 종료\n");
				goto EXIT;
			}
		}

		FILE* fp = NULL;
		fp = fopen("./address_tcp.txt", "a");
		if (fp == NULL) {
			fprintf(stderr, "fopen|errno[%d]\n", errno);
			goto EXIT;
		}
		
		//(데이터가 char 아니고 int였다면 바이트 오더링:ntohl/ltohn 필요)
		if (fwrite(&data, sizeof(data), 1, fp) != 1) {
			fprintf(stderr, "fwrite|errno[%d]\n", errno);
		}
		if (fclose(fp) != 0) {
			fprintf(stderr, "fclose|errno[%d]\n", errno);
			goto EXIT;
		}

	}

EXIT:
	//[O] TODO shutdown 제거

	if (close(cSockFd) == -1) {
		fprintf(stderr, "close|errno[%d]\n", errno);
	}

	return 0;
}

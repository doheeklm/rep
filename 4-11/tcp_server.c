/* tcp_server.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h> //socket() bind() listen() accept() recv()
#include <sys/socket.h> //socket() bind() listen() accept() recv()
#include <unistd.h> //close()
#include <arpa/inet.h> //htons() htonl()

#define PORT 7777 //포트 번호
#define MAX_PENDING 5 //연결요청 대기 큐의 크기

typedef struct _DATA {
	char name[13];
	char phone[14];
	char address[151];
} Data;

int main()
{
	Data data;

	int sSockFd; //서버 소켓
	int cSockFd; //클라이언트 소켓
	int Bind; //bind 성공여부
	
	struct sockaddr_in sAddr; //서버의 주소 정보 
	struct sockaddr_in cAddr; //클라이언트의 주소 정보 
	
	socklen_t sAddrSize = 0;
	socklen_t cAddrSize = 0;

	sAddrSize = sizeof(sAddr);
	cAddrSize = sizeof(cAddr);

	sSockFd = socket(PF_INET, SOCK_STREAM, 0);
	if (sSockFd == -1) {
		fprintf(stderr, "socket|errno[%d]", errno);
		return 0;
	}

	memset(&sAddr, 0, sAddrSize);
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(PORT);
	if (sAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]", errno);
		goto EXIT;
	}

	//TODO ifconfig 내 IP주소로..
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (sAddr.sin_addr.s_addr == -1) {
		fprintf(stderr, "htonl|errno[%d]", errno);
		goto EXIT;
	}

	Bind = bind(sSockFd, (struct sockaddr*)&sAddr, sAddrSize);
	if (Bind == -1) {
		fprintf(stderr, "bind|errno[%d]", errno);
		goto EXIT;
	}

	if (listen(sSockFd, MAX_PENDING) == -1) {
		fprintf(stderr, "listen|errno[%d]", errno);
		goto EXIT;
	}

	//TODO accept 이후 sockfd 값 표기
	if ((cSockFd = accept(sSockFd, (struct sockaddr*)&cAddr, &cAddrSize)) == -1) {
		fprintf(stderr, "accept|errno[%d]", errno);
		goto EXIT;
	}

	ssize_t rd = 0;

	while (1) {
		memset(&data, 0, sizeof(data));

		//TODO 기다리는 로직
		//원하는 바이트 수를 읽을 때까지
		//버퍼 옮겨가기(포인터 이동)
		rd = read(cSockFd, &data, sizeof(data));
		if (rd == -1) {
			fprintf(stderr, "read|errno[%d]", errno);
			goto EXIT;
		}
		printf("read[%ld]\n", rd);
		
		FILE* fp = NULL;
		fp = fopen("./address_tcp.txt", "a");
		if (fp == NULL) {
			fprintf(stderr, "fopen|errno[%d]", errno);
			goto EXIT;
		}
		
		//(데이터가 char 아니고 int였다면 바이트 오더링:ntohl/ltohn 필요)
		if (fwrite(&data, sizeof(data), 1, fp) != 1) {
			fprintf(stderr, "fwrite|errno[%d]", errno);
		}

		if (fclose(fp) != 0) {
			fprintf(stderr, "fclose|errno[%d]", errno);
			goto EXIT;
		}
	}

EXIT:
	//TODO shutdown 제거
	if (shutdown(cSockFd, SHUT_RDWR) == -1) {
		fprintf(stderr, "shutdown|errno[%d]", errno);
	}

	if (close(cSockFd) == -1) {
		fprintf(stderr, "close|errno[%d]", errno);
	}

	return 0;
}

/* tcp_server.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h> //socket() bind() listen() accept() 
#include <sys/socket.h> //socket() bind() listen() accept()
#include <unistd.h> //read() close()
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

	sSockFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sSockFd == -1) {
		fprintf(stderr, "socket|errno[%d]\n", errno);
		return 0;
	}

	memset(&sAddr, 0, sAddrSize);
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(PORT);
	if (sAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]\n", errno);
		goto EXIT;
	}

	//sAddr.sin_addr.s_addr = inet_addr("172.20.234.142");
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (sAddr.sin_addr.s_addr == -1) {
		fprintf(stderr, "inet_addr|errno[%d]\n", errno);
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

	if ((cSockFd = accept(sSockFd, (struct sockaddr*)&cAddr, &cAddrSize)) == -1) {
		fprintf(stderr, "accept|errno[%d]\n", errno);
		goto EXIT;
	}
	printf("sSockFd[%d] cSockFd[%d]\n", sSockFd, cSockFd);

	printf("입력받아야 할 바이트 수[%ld]\n", sizeof(data));

	while (1) {
		memset(&data, 0, sizeof(data));

		ssize_t rd = 0;
		ssize_t totalRd = 0;

		while (1) {
			rd = read(cSockFd, &data + totalRd, sizeof(data) - totalRd);
			if (rd == -1) {
				fprintf(stderr, "read|errno[%d]\n", errno);
				goto EXIT;
			}
			printf("read[%ld]\n", rd);

			totalRd += rd;
			printf("totalRd[%ld]\n", totalRd);

			if (totalRd == sizeof(data)) {
				printf("totalRd[%ld]:데이터 수신|파일 작성중\n", totalRd);
				break;
			}
			else if (totalRd == 0) {
				printf("totalRd[%ld]:프로그램 종료\n", totalRd);
				goto EXIT;
			}
		}

		FILE* fp = NULL;
		fp = fopen("./address_tcp.txt", "a");
		if (fp == NULL) {
			fprintf(stderr, "fopen|errno[%d]\n", errno);
			goto EXIT;
		}
		
		if (fwrite(&data, sizeof(data), 1, fp) != 1) {
			fprintf(stderr, "fwrite|errno[%d]\n", errno);
		}
		if (fclose(fp) != 0) {
			fprintf(stderr, "fclose|errno[%d]\n", errno);
			goto EXIT;
		}

	}

EXIT:
	if (close(cSockFd) == -1) {
		fprintf(stderr, "close|errno[%d]\n", errno);
	}

	return 0;
}

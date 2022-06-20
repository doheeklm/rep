/* tcp_server.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h> //socket() bind() listen() accept()
#include <sys/socket.h> //socket() bind() listen() accept()
#include <unistd.h> //read() write() close()
#include <arpa/inet.h>

#define PORT 7777
#define MAXPENDING 5

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
	
	struct sockaddr_in sAddr;
	struct sockaddr_in cAddr;
	
	socklen_t sAddrSize = 0;
	socklen_t cAddrSize = 0;

	sAddrSize = sizeof(sAddr);
	cAddrSize = sizeof(cAddr);

	if ((sSockFd == socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket()_errno[%d]:%s\n", errno, strerror(errno));
		return 0;
	}

	if (setsockopt(sSockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET; //IPv4 인터넷 프로토콜
	sAddr.sin_port = htons(PORT); //사용할 PORT 번호
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sSockFd, (struct sockaddr*)&sAddr, sAddrSize) == -1) {
		fprintf(stderr, "bind()_errno[%d]:%s\n", errno, strerror(errno));
		return 0;
	}

	if (listen(sSockFd, MAXPENDING) == -1) {
		fprintf(stderr, "listen()_errno[%d]:%s\n", errno, strerror(errno));
		return 0;
	}
	
	printf("waiting connection from client\n");
	if ((cSockFd = accept(sSockFd, (struct sockaddr*)&cAddr, &cAddrSize)) == -1) {
		fprintf(stderr, "accept/errno[%d]\n", errno);
		return 0;
	}	
	printf("connected with client\n");

	read(cSockFd, )
/*
	FILE* fp = NULL;
	fp = fopen("./address_tcp.txt", "a");
	if (fp == NULL) {
		fprintf(stderr, "fopen/errno[%d]\n", errno);
		//return
	}

	if (fwrite(&data, sizeof(data), 1, fp) != 1) {
		fprintf(stderr, "fwrite/errno[%d]\n", errno);
	}

	if (fclose(fp) != 0) {
		fprintf(stderr, "fclose/errno[%d]\n", errno)l
		//return
	}
*/

	close(cSockFd);
	close(sSockFd);

	return 0;
}

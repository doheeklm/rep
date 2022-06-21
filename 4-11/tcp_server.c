/* tcp_server.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h> //socket() bind() listen() accept()
#include <sys/socket.h> //socket() bind() listen() accept()
#include <unistd.h> //read() write() close()
#include <arpa/inet.h>

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

	//소켓 생성
	sSockFd = socket(PF_INET, SOCK_STREAM, 0);
	if (sSockFd == -1) {
		fprintf(stderr, "socket|errno[%d]", errno);
		return 0;
	}

	memset(&sAddr, 0, sAddrSize);
	sAddr.sin_family = AF_INET; //IPv4 인터넷 프로토콜
	sAddr.sin_port = htons(PORT); //사용할 포트 저장 
	if (sAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]", errno);
		goto EXIT;
	}
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY); //IP주소 자동으로 할당
	if (sAddr.sin_addr.s_addr == -1) {
		fprintf(stderr, "htonl|errno[%d]", errno);
		goto EXIT;
	}
	//INADDR_ANY :
	//IP주소가 하나인 컴퓨터에서는 현재 컴퓨터의 IP주소를 소켓에 할당하며,
	//Multi-homed(IP가 2개 이상)인 경우, 어떤 IP로 데이터가 들어오든지
	//포트 번호만 맞으면 해당 포트로 데이터를 전달해주도록 하는 상수. 주로 서버 측에서 사용됨

	//생성한 소켓을 서버 소켓으로 등록함 (성공하면 서버 소켓에 sAddr(주소 정보)가 할당됨)
	Bind = bind(sSockFd, (struct sockaddr*)&sAddr, sAddrSize);
	if (Bind == -1) {
		fprintf(stderr, "bind|errno[%d]", errno);
		goto EXIT;
	}

	//서버 소켓을 통해 클라이언트의 접속 요청을 확인하도록 설정함
	if (listen(sSockFd, MAX_PENDING) == -1) {
		fprintf(stderr, "listen|errno[%d]", errno);
		goto EXIT;
	}

	//클라이언트 접속 요청 대기 및 허락함 & 클라이언트와 통신을 위해 새 소켓 생성함
	if ((cSockFd = accept(sSockFd, (struct sockaddr*)&cAddr, &cAddrSize)) == -1) {
		fprintf(stderr, "accept|errno[%d]", errno);
		goto EXIT;
	}

	ssize_t rd;

	while (1) {
		memset(&data, 0, sizeof(data));

		rd = read(cSockFd, &data, sizeof(data));
		printf("rd[%ld]\n", rd);

		if (rd == -1) {
			fprintf(stderr, "read|errno[%d]", errno);
			goto EXIT;
		}
		else if (rd != 0) {
			FILE* fp = NULL;
			fp = fopen("./address_tcp.txt", "a");
			if (fp == NULL) {
				fprintf(stderr, "fopen|errno[%d]", errno);
				goto EXIT;
			}

			if (fwrite(&data, sizeof(data), 1, fp) != 1) {
				fprintf(stderr, "fwrite|errno[%d]", errno);
			}

			if (fclose(fp) != 0) {
				fprintf(stderr, "fclose|errno[%d]", errno);
				goto EXIT;
			}
		}
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

/* epoll_tcp_server.c */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

typedef struct _AddrBook {
	char szName[13];
	char szPhone[14];
	char szAddress[151];
} ADDRBOOK;

int main()
{
	int i;
	int nPort;
	int nMaxPending;
	int nMaxConnecting;
	int nListenFd;
	int nClientFd;
	int nEpollFd;
	char szFileName[255]; //"./szFileName.txt"

	struct sockaddr_in sAddr;
	struct sockaddr_in sClientAddr;
	socklen_t tAddrSize;
	socklen_t tClientAddrSize;

	struct epoll_event sEvent;
	struct epoll_event* sPtrEvents;
	int nTimeout;
	int nFdCount;
	epoll_data_t* tData;

	ADDRBOOK sAddrBook;
	int nRead;
	int nTotalRead;
	FILE* pFile;

	nListenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nListenFd == -1) {
		fprintf(stderr, "socket|errno[%d]\n", errno);
		//return
	}
	
	nEpollFd = epoll_create(10);
	if (nEpollFd == -1) {
		fprintf(stderr, "epoll_create|errno[%d]\n", errno);
		//return
	}
	
	tAddrSize = sizeof(sAddr);
	tClientAddrSize = sizeof(sClientAddr);

	nPort = 7000;
	memset(&sAddr, 0, tAddrSize);
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(nPort);
	if (sAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]\n", errno);
		//return
	}
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (sAddr.sin_addr.s_addr == -1) {
		fprint(fstderr, "htonl|errno[%d]\n", errno);
		//return
	}

	if (bind(nListenFd, &sAddr, tAddrSize) == -1) {
		fprintf(stderr, "bind|errno[%d]\n", errno);
		//return
	}
	
	nMaxPending = 5;
	if (listen(nListenFd, nMaxPending) == -1) {
		fprintf(stderr, "listen|errno[%d]\n", errno);
		//return
	}

	/* 서버쪽 소켓에 epoll 세팅 구조체 */
	/* 리슨fd를 epoll에 등록함*/
	sEvent.events = EPOLLIN;
	sEvent.data.fd = nListenFd;

	/* 관심 있는 fd 추가 */
	if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, nListenFd, &sEvent) == -1) {
		fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
		//return
	}

	/* 클라이언트 소켓들을 관리하기 위한 구조체 (최대 5개 동적할당)  */
	nMaxConnecting = 5;
	nTimeout = -1; //sPtrEvents[5] array TODO
	sPtrEvents = (struct epoll_event*)malloc(sizeof(struct epoll_event) * nMaxConnecting);
	if (sPtrEvents == NULL) {
		fprintf(stderr, "malloc|errno[%d]\n", errno);
		//return
	}

	while (1) {
		nFdCount = epoll_wait(nEpollFd, sPtrEvents, nMaxConnecting, nTimeout);
		if (nFdCount == -1) {
			fprintf(stderr, "epoll_wait|errno[%d]\n", errno);
			//return
		}

		for (i = 0; i < nFdCount; i++) {
			if (sPtrEvents[i].data.fd == nListenFd) {
				//int fd array (linked list)
				nClientFd = accept(nListenFd, &sClientAddr, tClientAddrSize);
				if (nClientFd == -1) {
					fprintf(stderr, "accept|errno[%d]\n", errno);
					//return

					//
				}
//5 max  +1 close   5 -> 4 . +1 new
			}
			else {

				//if ------
				//send 0 1 2

//cFd 데이터
//nonblocking read ... clientFd 
//				filename 

//------------------
//filename
//5개초과
			}
		}
	}
	

	//free
}

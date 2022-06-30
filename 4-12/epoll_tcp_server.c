/* epoll_tcp_server.c */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <stdlib.h>

typedef struct _AddrBook {
	char szName[13];
	char szPhone[14];
	char szAddress[151];
} ADDRBOOK;

typedef struct _Client {
	int nFd;
	int nReceive;
} CLIENT;

const char* szExit = "exit";

int main()
{
	int i = 0;
	int j = 0;
	int nIndex = 0;
	int nPort = 7000;
	int nMaxPending = 5;
	int nMaxConnecting = 5;
	int nListenFd = 0;
	int nClientFd = 0;
	int nEpollFd = 0;
	int nFdCount = 0;
	int nTimeout = -1;
	int nAccept = 0;
	char szFileName[255]; //filename
	char szBuffer[261]; //./filename.txt
//TODO size
	struct sockaddr_in tAddr;
	struct sockaddr_in tClientAddr;
	socklen_t nAddrSize;
	socklen_t nClientAddrSize;

	nAddrSize = sizeof(tAddr);
	nClientAddrSize = sizeof(tClientAddr);
	memset(&tAddr, 0, nAddrSize);
	memset(&tClientAddr, 0, nClientAddrSize);

	struct epoll_event tEvent; //서버쪽 소켓에  epoll 세팅 구조체
	struct epoll_event* ptEvents; // 클라이언트 소켓들을 관리하기 위한 구조체

	ptEvents = (struct epoll_event*)malloc(sizeof(struct epoll_event) * nMaxConnecting);
	if (ptEvents == NULL) {
		fprintf(stderr, "malloc|errno[%d]\n", errno);
		return 0;
	}

	ADDRBOOK tAddrBook;
	CLIENT* tClient;

	int nRead = 0;
	int nTotalRead = 0;
	FILE* pFile = NULL;
	memset(&szFileName, 0, sizeof(szFileName));
	memset(&szBuffer, 0, sizeof(szBuffer));

	nListenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nListenFd == -1) {
		fprintf(stderr, "socket|errno[%d]\n", errno);
		return 0;
	}
	
	nEpollFd = epoll_create(100);
	if (nEpollFd == -1) {
		fprintf(stderr, "epoll_create|errno[%d]\n", errno);
		goto EXIT;
	}

	tAddr.sin_family = AF_INET;
	tAddr.sin_port = htons(nPort);
	if (tAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]\n", errno);
		goto EXIT;
	}
	tAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (tAddr.sin_addr.s_addr == -1) {
		fprintf(stderr, "htonl|errno[%d]\n", errno);
		goto EXIT;
	}
	
	const int flag = 1;
	if (setsockopt(nListenFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1) {
		fprintf(stderr, "setsockopt|errno[%d]\n", errno);
		goto EXIT;
	}

	if (bind(nListenFd, (struct sockaddr*)&tAddr, nAddrSize) == -1) {
		fprintf(stderr, "bind|errno[%d]\n", errno);
		goto EXIT;
	}
	
	if (listen(nListenFd, nMaxPending) == -1) {
		fprintf(stderr, "listen|errno[%d]\n", errno);
		goto EXIT;
	}

	tEvent.events = EPOLLIN;
	tEvent.data.fd = nListenFd;
	if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, nListenFd, &tEvent) == -1) {
		fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
		goto EXIT;
	}

	while (1) {
		printf("1\n");
		nFdCount = epoll_wait(nEpollFd, ptEvents, nMaxConnecting, nTimeout);
		if (nFdCount == -1) {
			fprintf(stderr, "epoll_wait|errno[%d]\n", errno);
			goto EXIT;
		}

		for (i = 0; i < nFdCount; i++) {
			if (ptEvents[i].events & EPOLLIN) { //TODO events가 EPOLLIN인지 EPOLLERR인지 확인
				if (ptEvents[i].data.fd == nListenFd) {					
					nClientFd = accept(nListenFd, (struct sockaddr*)&tClientAddr, &nClientAddrSize);
					if (nClientFd == -1) {
						fprintf(stderr, "accept|errno[%d]\n", errno);
						goto EXIT;
					}
					printf("Accept: nClientFd[%d]\n", nClientFd);

//TODO nAccept 5 close(fd) nClient

					tClient = (struct _Client*)malloc(sizeof(struct _Client));
					tClient->nFd = nClientFd;

					tEvent.events = EPOLLIN;
					tEvent.data.ptr = tClient;
//TODO tEvent.data.fd = tClient->nFd;
					if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, nClientFd, &tEvent) == -1) {
						fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
						goto EXIT;
					}
				}
				else {
					tClient = ptEvents[i].data.ptr;

					if (tClient->nReceive == 0) {
						nRead = read(tClient->nFd, &szFileName, sizeof(szFileName));
						if (nRead == -1) {
							fprintf(stderr, "read|errno[%d]\n", errno);
							goto EXIT;
						}
						snprintf(szBuffer, sizeof(szBuffer), "./%s.txt", szFileName);

						tClient->nReceive = 1;
						printf("=====================================\n변화 감지된 Client: nFd[%d]\n파일명 전송받음: nReceive[%d]\n파일 경로: szBuffer[%s]\n=====================================\n", tClient->nFd, tClient->nReceive, szBuffer);
						goto CONTINUE; //TODO continue
					}
					else if (tClient->nReceive == 1)
					{
						while (1)
						{
							//TODO init -> 함수로
							nRead = read(tClient->nFd, &tAddrBook + nTotalRead, sizeof(tAddrBook) - nTotalRead); //TODO totalread
							
							if (nRead == -1)
							{
								fprintf(stderr, "read|errno[%d]\n", errno);
								goto EXIT;
							}

							if (strcmp(szExit, tAddrBook.szName) == 0)
							{
								printf("=====================================\n변화 감지된 Client: nFd[%d]\nexit를 전송받음: nReceive[%d]\n관찰대상에서 삭제\n=====================================\n", tClient->nFd, tClient->nReceive);

								if (epoll_ctl(nEpollFd, EPOLL_CTL_DEL, tClient->nFd, NULL) == -1)
								{
									fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
								}
								if (close(tClient->nFd) == -1)
								{
									fprintf(stderr, "close|errno[%d]\n", errno);
									goto EXIT;
								}
								
								free(tClient);
								//TODO nAccpet -1
								//	goto CONTINUE;
							}

							nTotalRead += nRead;
							
							if (nTotalRead == sizeof(tAddrBook))
							{
								printf("=====================================\n변화 감지된 Client: nFd[%d]\n주소록 정보 전송받음: nTotalRead[%d]\n=====================================\n", tClient->nFd, nTotalRead);
								goto WRITE;
							}
						}
WRITE:
						pFile = fopen(szBuffer, "a");
						if (pFile == NULL) {
							fprintf(stderr, "fopen|errno[%d]\n", errno);
							goto EXIT;
						}
						if (fwrite(&tAddrBook, sizeof(tAddrBook), 1, pFile) != 1) {
							fprintf(stderr, "fwrite|errno[%d]\n", errno);
						}
						if (fclose(pFile) != 0) {
							fprintf(stderr, "fclose|errno[%d]\n", errno);
							goto EXIT;
						}

						goto CONTINUE;
					}
				}
			}
			else if (ptEvents[i].events & EPOLLERR) {
				fprintf(stderr, "EPOLLERR|errno[%d]\n", errno);
				goto EXIT; //TODO close client  4 -> nAccept
			}
		}

CONTINUE:
	printf("\n");
	
	}

EXIT:
	free(tClient);
	free(ptEvents);

	if (close(nListenFd) == -1) {
		fprintf(stderr, "close|errno[%d]\n", errno);
	}
	
	return 0;
}

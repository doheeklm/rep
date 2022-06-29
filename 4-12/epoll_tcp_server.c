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

const char* szExit = "exit";

int main()
{
	int i = 0;
	int nPort = 7000;
	int nMaxPending = 5;
	int nMaxConnecting = 5;
	int nListenFd = 0;
	int nClientFd = 0;
	int nEpollFd = 0;
	int nFdCount = 0;
	int nTimeout = -1;
	int nAccept = 0;
	int nReceive = 0;
	char szFileName[255]; //filename
	char szBuffer[261]; //./filename.txt

	struct sockaddr_in tAddr;
	struct sockaddr_in tClientAddr;
	socklen_t nAddrSize;
	socklen_t nClientAddrSize;

	struct epoll_event tEvent; //서버쪽 소켓에 epoll 세팅 구조체
	struct epoll_event atEvents[nMaxConnecting]; // 클라이언트 소켓들을 관리하기 위한 구조체

	ADDRBOOK tAddrBook;
	int nRead = 0;
	int nTotalRead = 0;;
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
	
	nAddrSize = sizeof(tAddr);
	nClientAddrSize = sizeof(tClientAddr);

	tEvent.events = EPOLLIN;
	tEvent.data.fd = nListenFd;

	memset(&tAddr, 0, nAddrSize);
	tAddr.sin_family = AF_INET;
	tAddr.sin_port = htons(nPort);
	if (tAddr.sin_port == -1) {
		fprintf(stderr, "htons|errno[%d]\n", errno);
		//return
	}
	tAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (tAddr.sin_addr.s_addr == -1) {
		fprintf(stderr, "htonl|errno[%d]\n", errno);
		//return
	}

	if (bind(nListenFd, (struct sockaddr*)&tAddr, nAddrSize) == -1) {
		fprintf(stderr, "bind|errno[%d]\n", errno);
		//return
	}
	
	if (listen(nListenFd, nMaxPending) == -1) {
		fprintf(stderr, "listen|errno[%d]\n", errno);
		//return
	}

	tEvent.events = EPOLLIN;
	tEvent.data.fd = nListenFd;
	if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, nListenFd, &tEvent) == -1) {
		fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
		//return
	}

	while (1) {
		nFdCount = epoll_wait(nEpollFd, atEvents, nMaxConnecting, nTimeout);
		if (nFdCount == -1) {
			fprintf(stderr, "epoll_wait|errno[%d]\n", errno);
			//return
		}

		for (i = 0; i < nFdCount; i++) {
			if (atEvents[i].data.fd == nListenFd) {
				//int fd array (Use linked list to check which one is added and deleted)
				if (nAccept == 5) {
					printf("동시접속 가능한 Client의 수가 5개를 초과하였습니다.\n");
					goto CONTINUE;
				}

				nClientFd = accept(nListenFd, (struct sockaddr*)&tClientAddr, &nClientAddrSize);
				if (nClientFd == -1) {
					fprintf(stderr, "accept|errno[%d]\n", errno);
					//return
				}
				nAccept++;

				tEvent.events = EPOLLIN;
				tEvent.data.fd = nClientFd;
				if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, nClientFd, &tEvent) == -1) {
					fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
					//return
				}

				printf("=====================================\n연결된 Client: nClientFd[%d]\n현재 동시접속 Client 수: nAccept[%d]\n======================================\n", nClientFd, nAccept);
			}
			else { 
				nClientFd = atEvents[i].data.fd;
				
				if (nReceive == 0) {
					nRead = read(nClientFd, &szFileName, sizeof(szFileName));
					if (nRead == -1) {
						fprintf(stderr, "read|errno[%d]\n", errno);
						//return
					}
					snprintf(szBuffer, sizeof(szBuffer), "./%s.txt", szFileName);
					nReceive++;
					
					printf("=====================================\n변화 감지된 Client: nClientFd[%d]\n파일명 전송받음: nReceive[%d]\n파일 경로: szBuffer[%s]\n=====================================\n", nClientFd, nReceive, szBuffer);
				}
				else if (nReceive == 1) {
					while (1) {
						nRead = read(nClientFd, &tAddrBook, sizeof(tAddrBook));
						if (nRead == -1) {
							fprintf(stderr, "read|errno[%d]\n", errno);
						}
						printf("nRead[%d]\n", nRead);
					
						if (strcmp(szExit, tAddrBook.szName) == 0) {
							printf("=====================================\n변화 감지된 Client: nClientFd[%d]\nexit를 전송받음: 관찰대상에서 삭제\n=====================================\n", nClientFd);
						
							if (epoll_ctl(nEpollFd, EPOLL_CTL_DEL, nClientFd, NULL) == -1) {
								fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
								//return
							}
							if (close(nClientFd) == -1) {
								fprintf(stderr, "close|errno[%d]\n", errno);
							}
							goto CONTINUE;
						}

						nTotalRead += nRead;
						if (nTotalRead == sizeof(tAddrBook)) {
							printf("=====================================\n변화 감지된 Client: nClientFd[%d]\nnTotalRead[%d]: 데이터 수신|파일 작성중\n=====================================\n", nClientFd, nTotalRead);
							break;
						}
						else if (nTotalRead == 0) {
							printf("=====================================\n	변화 감지된 Client: nClientFd[%d]\nnTotalRead[%d]: 읽을 데이터가 없음\n	=====================================\n", nClientFd, nTotalRead);
							goto CONTINUE;
						}
					}

					FILE* fp = NULL;
					fp = fopen(szBuffer, "a");
					if (fp == NULL) {
						fprintf(stderr, "fopen|errno[%d]\n", errno);
						goto EXIT;
					}
					if (fwrite(&tAddrBook, sizeof(tAddrBook), 1, fp) != 1) {
						fprintf(stderr, "fwrite|errno[%d]\n", errno);
					}
					if (fclose(fp) != 0) {
						fprintf(stderr, "fclose|errno[%d]\n", errno);
						goto EXIT;
					}

				}//else if
			}//else
		}//for
	
CONTINUE:
	printf("epoll_wait()로 돌아갑니다.\n");

	}//while

EXIT:
	if (close(nListenFd) == -1) {
		fprintf(stderr, "close|errno[%d]\n", errno);
		//return
	}
	
	return 0;
}

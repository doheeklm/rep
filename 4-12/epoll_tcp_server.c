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

#define READ_FAIL -1
#define READ_EOF 0

typedef struct _AddrBook
{
	char szName[13];
	char szPhone[14];
	char szAddress[151];
} ADDRBOOK;

typedef struct _Client
{
	int nFd;
	int nReceive;
	char szFilePath[261];
} CLIENT;

const char* szExit = "exit";
int MyRead(int fd, void* buf, int nSize);

int main()
{
	int i = 0;
	int j = 0;
	int k = 0;
	int nIndex = 0;
	int nPort = 7000;
	int nMaxPending = 5;
	int nMaxConnecting = 5;
	int nListenFd = 0;
	int nClientFd = 0;
	int nEpollFd = 0;
	int nFdCount = 0;
	int nTimeout = -1;
	int nClient = 0;
	char szFileName[255];

	struct sockaddr_in tAddr;
	struct sockaddr_in tClientAddr;
	socklen_t nAddrSize;
	socklen_t nClientAddrSize;
	
	nAddrSize = sizeof(tAddr);
	nClientAddrSize = sizeof(tClientAddr);
	memset(&tAddr, 0, nAddrSize);
	memset(&tClientAddr, 0, nClientAddrSize);

	ADDRBOOK tAddrBook;
	CLIENT* tClient;
	tClient = (CLIENT*)malloc(sizeof(CLIENT) * nMaxConnecting);
	if (tClient == NULL)
	{
		fprintf(stderr, "malloc|errno[%d]\n", errno);
		return 0;
	}

	struct epoll_event tEvent;
	struct epoll_event* ptEvents;
	ptEvents = (struct epoll_event*)malloc(sizeof(struct epoll_event) * nMaxConnecting);
	if (ptEvents == NULL)
	{
		fprintf(stderr, "malloc|errno[%d]\n", errno);
		return 0;
	}
	
	FILE* pFile = NULL;

	nListenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nListenFd == -1)
	{
		fprintf(stderr, "socket|errno[%d]\n", errno);
		return 0;
	}
	
	nEpollFd = epoll_create(100);
	if (nEpollFd == -1)
	{
		fprintf(stderr, "epoll_create|errno[%d]\n", errno);
		goto EXIT;
	}

	tAddr.sin_family = AF_INET;
	tAddr.sin_port = htons(nPort);
	if (tAddr.sin_port == -1)
	{
		fprintf(stderr, "htons|errno[%d]\n", errno);
		goto EXIT;
	}
	tAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (tAddr.sin_addr.s_addr == -1)
	{
		fprintf(stderr, "htonl|errno[%d]\n", errno);
		goto EXIT;
	}
	
	const int flag = 1;
	if (setsockopt(nListenFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1)
	{
		fprintf(stderr, "setsockopt|errno[%d]\n", errno);
		goto EXIT;
	}

	if (bind(nListenFd, (struct sockaddr*)&tAddr, nAddrSize) == -1)
	{
		fprintf(stderr, "bind|errno[%d]\n", errno);
		goto EXIT;
	}
	
	if (listen(nListenFd, nMaxPending) == -1)
	{
		fprintf(stderr, "listen|errno[%d]\n", errno);
		goto EXIT;
	}

	tEvent.events = EPOLLIN;
	tEvent.data.fd = nListenFd;
	if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, nListenFd, &tEvent) == -1)
	{
		fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
		goto EXIT;
	}

	while (1)
	{
		nFdCount = epoll_wait(nEpollFd, ptEvents, nMaxConnecting, nTimeout);
		if (nFdCount == -1)
		{
			fprintf(stderr, "epoll_wait|errno[%d]\n", errno);
			goto EXIT;
		}

		for (i = 0; i < nFdCount; i++)
		{
			if (ptEvents[i].events & EPOLLIN)
			{
				if (ptEvents[i].data.fd == nListenFd)
				{
					nClientFd = accept(nListenFd, (struct sockaddr*)&tClientAddr, &nClientAddrSize);
					if (nClientFd == -1)
					{
						fprintf(stderr, "accept|errno[%d]\n", errno);
						goto EXIT;
					}

					if (nClient == 5) {
						if (close(nClientFd) == -1)
						{
							fprintf(stderr, "close|errno[%d]\n", errno);
							goto EXIT;
						}
						printf("=====================================\nClient 동시 접속 수가 5개를 초과하여 연결을 끊습니다.\n=====================================\n");
						goto WHILE;
					}
					//TODO Q. 서버 측에서 초과된 연결을 끊었는데, 클라이언트가 응답받지 못함

					for (k =  0; k < nMaxConnecting; k++)
					{
						if (tClient[k].nFd == 0)
						{
							tClient[k].nFd = nClientFd;
							nIndex = k;
							break;
						}
					}

					tEvent.events = EPOLLIN;
					tEvent.data.fd = tClient[nIndex].nFd;
				
					if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, tClient[nIndex].nFd, &tEvent) == -1)
					{
						fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
						goto EXIT;
					}

					nClient++;
					printf("=====================================\n동시 접속 중인 Client 수: %d\n=====================================\n", nClient);
				}
				else
				{
					for (j = 0; j < nMaxConnecting; j++) //j < nClient
					{
						if (ptEvents[i].data.fd == tClient[j].nFd)
						{
							tClient[j].nFd = ptEvents[i].data.fd;

							if (tClient[j].nReceive == 0) {
								memset(&szFileName, 0, sizeof(szFileName));
								memset(&tClient[j].szFilePath, 0, sizeof(tClient[j].szFilePath));
					
								if (MyRead(tClient[j].nFd, &szFileName, sizeof(szFileName)) == READ_FAIL)
								{
									goto EXIT;
								}

								snprintf(tClient[j].szFilePath, sizeof(tClient[j].szFilePath), "./%s.txt", szFileName);

								tClient[j].nReceive = 1;
								printf("=====================================\n변화 감지된 Client: #%d\n전송받은 파일 경로: %s\n=====================================\n", tClient[j].nFd, tClient[j].szFilePath);
							}
							else if (tClient[j].nReceive == 1)
							{
								if (MyRead(tClient[j].nFd, &tAddrBook, sizeof(tAddrBook)) == READ_FAIL)
								{
									goto EXIT;
								}
								
								if (strcmp(szExit, tAddrBook.szName) == 0)
								{
									printf("=====================================\n변화 감지된 Client: #%d\n관찰대상에서 삭제\n=====================================\n", tClient[j].nFd);

									if (epoll_ctl(nEpollFd, EPOLL_CTL_DEL, tClient[j].nFd, NULL) == -1)
									{
										fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
									}
									if (close(tClient[j].nFd) == -1)
									{
										fprintf(stderr, "close|errno[%d]\n", errno);
										goto EXIT;
									}

									nClient--;
									memset(&tClient[j].nFd, 0, sizeof(tClient[j].nFd));
									memset(&tClient[j].nReceive, 0, sizeof(tClient[j].nReceive));
									memset(&tClient[j].szFilePath, 0, sizeof(tClient[j].szFilePath));	
									printf("=====================================\n동시 접속 중인 Client 수: %d\n=====================================\n", nClient);

									if (nClient == 0)
									{
										printf("서버를 종료합니다.\n");
										goto EXIT;
									}

									break;
								}

								pFile = fopen(tClient[j].szFilePath, "a");
								if (pFile == NULL)
								{
									fprintf(stderr, "fopen|errno[%d]\n", errno);
									goto EXIT;
								}
								if (fwrite(&tAddrBook, sizeof(tAddrBook), 1, pFile) != 1)
								{
									fprintf(stderr, "fwrite|errno[%d]\n", errno);
								}
								if (fclose(pFile) != 0)
								{
									fprintf(stderr, "fclose|errno[%d]\n", errno);
									goto EXIT;
								}

							}//else if (tClient->nReceive == 1)

						}//if (tClient[j]->nFd == ptEvents[i].data.fd)

					}//for (j = 0; j < nClient; j++)
CONT:
					printf("goto CONT\n");

				}//else

			}//if EPOLLIN
			else if (ptEvents[i].events & EPOLLERR)
			{
				fprintf(stderr, "EPOLLERR|errno[%d]\n", errno);
				goto EXIT;
			}

		}//for(i = 0; i < nFdCount; i++)

WHILE:
		printf("goto WHILE\n");

	}//while(1)

EXIT:
	free(tClient);
	free(ptEvents);

	if (close(nListenFd) == -1)
	{
		fprintf(stderr, "close|errno[%d]\n", errno);
	}

	return 0;
}

int MyRead(int fd, void* buf, int nSize)
{
	int nRead = 0;
	int nTotal = 0;

	while (1)
	{
 		nRead = read(fd, buf + nTotal, nSize - nTotal);
 		if (nRead == -1)
 		{
 			fprintf(stderr, "read|errno[%d]\n", errno);
			return READ_FAIL;
		}

		nTotal += nRead;
		if (nTotal == nSize)
		{
			printf("=====================================\n변화 감지된 Client: #%d\nRead 성공\n=====================================\n", fd);
			return nTotal;
		}
		else if (nTotal == 0)
		{
			return READ_EOF;
		}
	}
}


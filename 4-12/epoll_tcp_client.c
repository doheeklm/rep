/* epoll_tcp_client.c */

#include <stdio.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#define WRITE_FAIL -1
#define WRITE_EOF 0

typedef struct _AddrBook {
	char szName[13];
	char szPhone[14];
	char szAddress[151];
} ADDRBOOK;

const char* szExit = "exit";
void ClearStdin(char* c);
int MyWrite(int fd, void* buf, int nSize);

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("하나의 파일명을 입력해주세요.\n");
		return 0;
	}

	int nPort = 0; //연동된 Port 번호
	int nFd = 0; //Client 소켓 Fd
	char szFileName[255]; //인자로 입력받을 주소록 파일명

	if (sizeof(argv[1]) > sizeof(szFileName))
	{
		printf("파일명이 너무 깁니다.\n");
		return 0;
	}

	nPort = 7000;
	nFd = 0;
	memset(&szFileName, 0, sizeof(szFileName));
	strncpy(szFileName, argv[1], sizeof(argv[1]));
	szFileName[sizeof(szFileName) - 1] = '\0';
	
	struct sockaddr_in tAddr; //소켓에 설정할 주소 정보

	socklen_t nAddrSize;
	nAddrSize = sizeof(tAddr);
	
	ADDRBOOK tAddrBook; //주소록 정보 구조체 변수
	
	nFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nFd == -1)
	{
		fprintf(stderr, "socket|errno[%d]\n", errno);
		return 0;
	}

	memset(&tAddr, 0, nAddrSize);
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

	if (connect(nFd, (struct sockaddr*)&tAddr, nAddrSize) == -1)
	{
		fprintf(stderr, "connect|errno[%d]\n", errno);
		goto EXIT;
	}
	printf("nFd[%d]\n", nFd);

	if (MyWrite(nFd, &szFileName, sizeof(szFileName)) == WRITE_FAIL)
	{
		goto EXIT;
	}
	printf("=====================================\n파일명을 전송했습니다: %s\n=====================================\n", szFileName);

	while(1)
	{
		do {
			memset(&tAddrBook, 0, sizeof(tAddrBook));

			printf("Name: ");
			if (fgets(tAddrBook.szName, sizeof(tAddrBook.szName), stdin) == NULL)
			{
				fprintf(stderr, "fgets|errno[%d]\n", errno);
				goto EXIT;
			}
			ClearStdin(tAddrBook.szName);
			
			if (strcmp(szExit, tAddrBook.szName) == 0)
			{
				if (MyWrite(nFd, &tAddrBook, sizeof(tAddrBook)) == WRITE_FAIL)
				{
					goto EXIT;
				}
				goto EXIT;	
			}

			printf("Phone Num: ");
			if (fgets(tAddrBook.szPhone, sizeof(tAddrBook.szPhone), stdin) == NULL)
			{
				fprintf(stderr, "fgets|errno[%d]\n", errno);
				goto EXIT;
			}
			ClearStdin(tAddrBook.szPhone);
			
			if ((tAddrBook.szPhone[3] != '-') || (tAddrBook.szPhone[8] != '-'))
			{
				printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
			}
		} while ((tAddrBook.szPhone[3] != '-') || (tAddrBook.szPhone[8] != '-'));

		printf("Address: ");
		if (fgets(tAddrBook.szAddress, sizeof(tAddrBook.szAddress), stdin) == NULL)
		{
			fprintf(stderr, "fgets|errno[%d]\n", errno);
			goto EXIT;
		}
		ClearStdin(tAddrBook.szAddress);

		if (MyWrite(nFd, &tAddrBook, sizeof(tAddrBook)) == WRITE_FAIL)
		{
			goto EXIT;
		}
	}

EXIT:
	if (close(nFd) == -1)
	{
		fprintf(stderr, "close|errno[%d]\n", errno);
	}

	return 0;
}

void ClearStdin(char* c)
{
	if (c == NULL)
	{
		return;
	}

	if (c[strlen(c) - 1] == '\n')
	{
		c[strlen(c) - 1] = '\0';
	}

	__fpurge(stdin);
}

int MyWrite(int fd, void* buf, int nSize)
{
	int nWrite = 0;
	int nTotal = 0;

	while (1)
	{
		nWrite = write(fd, buf + nTotal, nSize - nTotal);
		if (nWrite == -1)
		{
			fprintf(stderr, "write|errno[%d]\n", errno);
			return WRITE_FAIL;
		}
	
		nTotal += nWrite;
		if (nTotal == nSize)
		{
			printf("=====================================\nWrite 성공\n=====================================\n");
			return nTotal;
		}
		else if (nTotal == 0)
		{
			return WRITE_EOF;
		}
	} 
}

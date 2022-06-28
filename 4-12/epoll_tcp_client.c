/* epoll_tcp_client.c */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

typedef struct _AddrBook {
	char szName[13];
	char szPhone[14];
	char szAddress[151];
} ADDRBOOK;

const char* szExit = "exit";
void ClearStdin(char* c);

int main(int argc, char* argv[])
{
	int nPort;
	char szFileName[255];
	int nFd;

	struct sockaddr_in sAddr;
	socklent_t tAddrSize;
	
	ADDRBOOK sAddrBook;
	int nWrite;
	int nTotalWrite;


}

void ClearStdin(char* c)
{
	if (c == NULL) {
		return;
	}

	if (c[strlen(c) - 1] == '\n') {
		c[strlen(c) - 1] = '\0';
	}

	__fpurge(stdin);
}

/* tcp_client.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h> //socket() connect()
#include <sys/socket.h> //socket() connect() inet_addr()
#include <unistd.h> //read() write() close()
#include <arpa/inet.h> //htons() inet_addr()
#include <netinet/in.h> //inet_addr()

#define PORT 7777
#define MAX_PENDING 5

typedef struct _Data {
	char name[13];
	char phone[14];
	char address[151];
} Data; 

void ClearStdin(char* c);

int main(int argc, char *argv[])
{
//	const char* str_exit = "exit";
	
//	Data data;
	
	int sockfd = 0;

	struct sockaddr_in sAddr;
	socklen_t sAddrSize = 0;
	sAddrSize = sizeof(sAddr);

	if ((sockfd = (socket(PF_INET, SOCK_STREAM, 0))) == -1) {
		fprintf(stderr, "socket/errno[%d]", errno);
		//return
	}

	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = inet_addr(argv[1]);
	sAddr.sin_port = htons(PORT);

	if (connect(sockfd, (struct sockaddr*)&sAddr, sAddrSize) == -1) {
		fprintf(stderr, "connect/errno[%d]", errno);
		//return
	}
	
	char msg[30];
	read(sockfd, msg, sizeof(msg) - 1);
	printf("Msg: %s\n", msg);

	close(sockfd);

	return 0;
}

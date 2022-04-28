#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler()
{
	static int count = 1;

	switch (count) {
		case 1:
			{
				printf("count %d\n", count);
				printf("SIGUSR1 first\n");
			}
			break;
		case 2:
			{
				printf("count %d\n", count);
				printf("SIGUSR1 second\n");
			}
			break;
		case 3:
			{
				printf("count %d\n", count);
				printf("exit the program\n");
				exit(1);
			}
			break;
		default:
			break;
	}

	count++;
}

int main()
{
	/* SIGTERM 수신해도 프로그램은 종료하지 않음 */
	if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
		perror("error\n");
		return 0;
	}
	
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGUSR1);

	act.sa_sigaction = handler;
	act.sa_flags = SA_SIGINFO;

	sigaction(SIGUSR1, &act, NULL);

	while (1) {
		raise(SIGUSR1);
		sleep(10);
	}

	return 0;
}

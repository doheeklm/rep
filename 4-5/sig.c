#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handler()
{
	static int count = 1;
	printf("count %d\n", count);

	if (count == 1) {
		printf("SIGUSR1 first\n");
	}
	else if (count == 2) {
		printf("SIGUSR1 second\n");
	}
	else {
		printf("exit the program\n");
		exit(1);
	}

	count++;
}

int main()
{
	struct sigaction act;
	
	act.sa_handler = SIG_IGN;
	
	sigemptyset(&act.sa_mask);
	
	int signo[2] = { SIGTERM, SIGINT };
	
	for (size_t i = 0; i < sizeof(signo); i++) {
	
		sigaddset(&act.sa_mask, signo[i]);
	
		if (sigismember(&act.sa_mask, signo[i]) == 1) {
			printf("signo[%d] 포함\n", signo[i]);
		
			if (sigaction(signo[i], &act, NULL) != 0) {
				perror("sigaction error\n");
				return 0;
			}
		}
	}
	
	if (signal(SIGUSR1, handler) == SIG_ERR) {
		perror("error\n");
		return 0;
	}

	while (1) {
		raise(SIGUSR1);
		sleep(10);
	}

	return 0;
}

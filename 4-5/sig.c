#include <stdio.h>
#include <stdlib.h> //exit()
#include <signal.h> //sig-
#include <unistd.h> //pause()

volatile sig_atomic_t flag = 0;

void log_message()
{
	if (flag == 1) {
		printf("SIGUSR1 first\n");
	}
	else if (flag == 2) {
		printf("SIGUSR1 second\n");
	}
	else if (flag == 3){
		printf("exit the program\n");
	}
}

void sigusr_handler(int signo)
{
	if (signo == SIGUSR1) {
		flag++;
	}
}

int main()
{
	struct sigaction act;
	act.sa_handler = sigusr_handler;
	act.sa_flags = 0;

	if (sigemptyset(&act.sa_mask) != 0)
	{	
		perror("error\n");
		return 0;
	}
	
	//본인 포함 시그널 인터럽트 실패
	if (sigaddset(&act.sa_mask, SIGUSR1) != 0)
	{	
		perror("error\n");
		return 0;
	}

	if (sigaction(SIGUSR1, &act, NULL) != 0) {
		perror("error\n");
		return 0;
	}

	if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
		perror("error\n");
		return 0;
	}

	while (flag >= 0 && flag <= 2) {
		pause();
		log_message();
	}

	return 0;
}

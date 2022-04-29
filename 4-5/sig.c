#include <stdio.h>
#include <stdlib.h> //exit()
#include <signal.h> //sig
#include <unistd.h> //pause()

void handler(int signo, siginfo_t *info, void *ucontext)
{
	static int count = 1;
	printf("count %d\n", count);
	printf("시그널 넘버 %d\n", info->si_signo);
	printf("에러 넘버 %d\n", info->si_errno);
	printf("시그널 발생 이유 %d\n", info->si_code);

	if (signo == SIGUSR1) {

		if (count == 1) {
			printf("SIGUSR1 first\n");
		}
		else if (count == 2) {
			printf("SIGUSR1 second\n");
		}
		else {
			printf("exit\n");
			exit(1);
		}

		count++;
	}

	sleep(10); //이 사이에 다른 시그널들 처리되는지 확인
	//이 함수 끝나자마자 봉쇄된 시그널이 처리됨
}

int main()
{
	if (signal(SIGTERM, SIG_IGN) == SIG_ERR) {
		perror("error\n");
		return 0;
	}

	struct sigaction act;
	act.sa_flags = SA_SIGINFO; //시그널 정보들
	act.sa_sigaction = handler;

	//(sigset_t sa_mask; 시그널을 처리하는 동안 차단할 시그널의 집합)
	if (sigfillset(&act.sa_mask) == -1) //모든 시그널 봉쇄됨
	{	
		perror("error\n");
		return 0;
	}	//시그널 핸들러 실행이 완료될 때까지 모든 시그널의 처리가 미뤄짐

	if (sigaction(SIGUSR1, &act, NULL) == -1) {
		perror("error\n");
		return 0;
	}

	int j = 0;

	while (1) {
		printf("%d\n", j);
		sleep(1);
		j++;

		if (j > 10) {
			printf("j > 10\n");
			sigdelset(&act.sa_mask, SIGINT); //함수핸들러 sleep때 sigint 바로 적용됨

			break;
		}
	}

	int k = 0;
	while (1) {
		printf("%d\n", k);
		sleep(1);
		k++;
	}

	return 0;
}

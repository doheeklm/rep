#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> //exit() malloc() free()
#include <errno.h> //errno
#include <string.h> //strerror_r()
#include <unistd.h> //sleep()

enum { ERR_BUFSIZE = 64, NUM_THREAD = 5, MAX_BALANCE = 10000,
		OUT_OF_RANGE = 0, IN_RANGE = -1 };

char errmsg[ERR_BUFSIZE];

static int balance;
static pthread_rwlock_t rwlock;

typedef struct DATA{
	int cnt;
	int time;
	int amount;
} DATA;

int OffLimits()
{
	if (balance < MAX_BALANCE) {
		return IN_RANGE;
	}
	
	return OUT_OF_RANGE;
}

void *Balance(void *data)
{
	DATA d = *(DATA *)data;
	
	while (balance < MAX_BALANCE) {
		if (OffLimits() == OUT_OF_RANGE) {
			exit(EXIT_SUCCESS);
		}

#if DEBUG
		printf("[ Thread #%d ]\n", d.cnt);
#endif

		if (d.cnt == 3 || d.cnt == 4) { //잔고 증가&감소시키는 스레드 2개
			
			if (pthread_rwlock_wrlock(&rwlock) != 0) {
				if (strerror_r(errno, errmsg, ERR_BUFSIZE) != 0) {
					fprintf(stderr, "errno[%d] : %s\n", errno, errmsg);
					exit(EXIT_FAILURE);
				}	
			}

			balance += d.amount;

			if (pthread_rwlock_unlock(&rwlock) != 0) {
				if (strerror_r(errno, errmsg, ERR_BUFSIZE) != 0) {
					fprintf(stderr, "errno[%d] : %s\n", errno, errmsg);
					exit(EXIT_FAILURE);
				}
			}
		}
		else { //잔고 출력하는 스레드 3개
			if (pthread_rwlock_rdlock(&rwlock) != 0) {
				if (strerror_r(errno, errmsg, ERR_BUFSIZE) != 0) {
					fprintf(stderr, "errno[%d] : %s\n", errno, errmsg);
					exit(EXIT_FAILURE);
				}
			}
			printf("%d원\n", balance);
			
			if (pthread_rwlock_unlock(&rwlock) != 0) {
				if (strerror_r(errno, errmsg, ERR_BUFSIZE) != 0) {
					fprintf(stderr, "errno[%d] : %s\n", errno, errmsg);
					exit(EXIT_FAILURE);
				}
			}
		}
		
		sleep(d.time);
	}

	return NULL;
}

int main()
{
	size_t i;
	balance = 0;

	pthread_t p_thread[NUM_THREAD];

	DATA *data;
	data = (struct DATA *)malloc(sizeof(struct DATA) * NUM_THREAD);

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		if (strerror_r(errno, errmsg, ERR_BUFSIZE) != 0) {
			fprintf(stderr, "errno[%d] : %s\n", errno, errmsg);
			exit(EXIT_FAILURE);
		}		
	}

	for (i = 0; i < NUM_THREAD; i++) {
		data[i].cnt = i;
		data[i].time = 1;
		data[i].amount = 0;

		if (i == 3) { //1초에 1000원씩 증가
			data[i].amount = 1000;
		}
		else if (i == 4) { //2초에 500원씩 감소
			data[i].time = 2;
			data[i].amount = -500;
		}
		
		if (pthread_create(&p_thread[i], NULL, Balance, (void *)&data[i]) != 0) {
			if (strerror_r(errno, errmsg, ERR_BUFSIZE) != 0) {
				fprintf(stderr, "errno[%d] : %s\n", errno, errmsg);
				exit(EXIT_FAILURE);
			}		
		}
	}

	for (i = 0; i < NUM_THREAD; i++) {
		if (pthread_join(p_thread[i], (void **)NULL) != 0) {
			if (strerror_r(errno, errmsg, ERR_BUFSIZE) != 0) {
				fprintf(stderr, "errno[%d] : %s\n", errno, errmsg);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		if (strerror_r(errno, errmsg, ERR_BUFSIZE) != 0) {
			fprintf(stderr, "errno[%d] : %s\n", errno, errmsg);
			exit(EXIT_FAILURE);
		}
	}

	free(data);
	
	return 0;
}

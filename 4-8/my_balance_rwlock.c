/* rwlock ver. */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> //exit() malloc() free()
#include <errno.h> //errno
#include <string.h> //strerror_r()
#include <time.h> //nanosleep() clock_gettime()

enum { ERR_BUFSIZE = 64, NUM_CHECK = 3, NUM_UPDATE = 2,
		MAX_BALANCE = 10000};

static int balance;
static pthread_rwlock_t rwlock;

typedef struct DATA {
	int count; //스레드 번호
	int amount; //증감할 돈
} DATA;

void *Check(void *data);
void *Update(void *data);

int main()
{
	size_t i;
	balance = 0;

	pthread_t th_check[NUM_CHECK]; //잔고 출력하는 스레드
	pthread_t th_update[NUM_UPDATE]; //잔고 업데이트하는 스레드

	DATA *data_ch;
	data_ch = (struct DATA *)malloc(sizeof(struct DATA) * NUM_CHECK);
	if (data_ch == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	DATA *data_up;
	data_up = (struct DATA *)malloc(sizeof(struct DATA) * NUM_UPDATE);
	if (data_up == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		free(data_ch);
		free(data_up);
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < NUM_CHECK; i++) {
		data_ch[i].count = i; //스레드 0, 1, 2번
		data_ch[i].amount = 0;
		
		if (pthread_create(&th_check[i], NULL, Check, (void *)&data_ch[i]) != 0) {
			free(data_ch);
			free(data_up);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < NUM_UPDATE; i++) {
		data_up[i].count = i + NUM_CHECK; //스레드 3, 4번
		
		if (i == 0) {
			data_up[i].amount = 1000; //1초에 1000원씩 증가
		}
		else if (i == 1) {
			data_up[i].amount = -500; //2초에 500원씩 감소
		}

		if (pthread_create(&th_update[i], NULL, Update, (void *)&data_up[i]) != 0) {
			free(data_ch);
			free(data_up);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < NUM_CHECK; i++) {
		if (pthread_join(th_check[i], (void **)NULL) != 0) {
			free(data_ch);
			free(data_up);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
	}	

	for (i = 0; i < NUM_UPDATE; i++) {
		if (pthread_join(th_update[i], (void **)NULL) != 0) {
			free(data_ch);
			free(data_up);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
	}
	
	if (pthread_rwlock_destroy(&rwlock) != 0) {
		free(data_ch);
		free(data_up);
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	free(data_ch);
	free(data_up);
		
	return 0;
}

void *Check(void *data)
{
	long deltaTime = 0;

	struct timespec start = { 0, 0 };
	struct timespec end = { 0, 0 };
	struct timespec sleep = { 0, 010000000 }; //sec:0, nsec:01 => 0.01초

	DATA d = *(DATA *)data;

	while (1) {
		if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		//////////* READ lock *//////////
		if (pthread_rwlock_rdlock(&rwlock) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		if (balance >= MAX_BALANCE) {
			if (pthread_rwlock_unlock(&rwlock) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}
			return NULL;
		}

		printf("스레드[%d] %d원\n", d.count, balance);

		if (pthread_rwlock_unlock(&rwlock) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}
		///////////////////////////////////
		
		while (1) {
			if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}

			deltaTime = end.tv_sec - start.tv_sec;
			//printf("delta %ld\n", deltaTime);
			if (deltaTime >= 1) {
				break;
			}

			if (nanosleep(&sleep, NULL) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}
		}
	}

	return NULL;
}

void *Update(void *data)
{
	int deltaTime = 0;

	struct timespec start = { 0, 0 };
	struct timespec end = { 0, 0 };
	struct timespec sleep = { 0, 010000000 }; //sec:0, nsec:01 => 0.01초

	DATA d = *(DATA *)data;

	while (1) {
		if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		//////////* WRITE lock *//////////
		if (pthread_rwlock_wrlock(&rwlock) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		if (balance >= MAX_BALANCE) {
			if (pthread_rwlock_unlock(&rwlock) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}
			return NULL;
		}

		balance += d.amount;

		if (d.amount > 0) {
			printf("스레드[%d] +%d\n", d.count, d.amount);
		}
		else if (d.amount < 0) {
			printf("스레드[%d] %d\n", d.count, d.amount);
		}

		if (pthread_rwlock_unlock(&rwlock) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}
		///////////////////////////////////

		while (1) {
			if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}

			deltaTime = end.tv_sec - start.tv_sec;
			//printf("delta %ld\n", deltaTime);

			if (d.amount < 0 && deltaTime >= 2) {
				break;
			}
			else if (d.amount > 0 && deltaTime >= 1) {
				break;
			}

			if (nanosleep(&sleep, NULL) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}
		}

	}

	return NULL;
}

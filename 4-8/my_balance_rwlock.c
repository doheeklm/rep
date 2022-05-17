#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> //exit() malloc() free()
#include <errno.h> //errno
#include <string.h> //strerror_r()
#include <unistd.h> //sleep()

enum { ERR_BUFSIZE = 64, NUM_THREAD = 5, MAX_BALANCE = 10000,
		OUT_OF_RANGE = 0, IN_RANGE = -1 };

static int balance;
static pthread_rwlock_t rwlock;

typedef struct DATA{
	int count;
	int time;
	int amount;
} DATA;

int OffLimits()
{
	if (balance < MAX_BALANCE) {
		return IN_RANGE; //-1
	}
	
	return OUT_OF_RANGE; //0
}

void *Balance(void *data)
{
	DATA d = *(DATA *)data;
	
	while (balance < MAX_BALANCE) {
		if (d.count == 3 || d.count == 4) { //wrlock
			if (pthread_rwlock_wrlock(&rwlock) != 0) {
				fprintf(stderr, "errno[%d]", errno);
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
		}
		else { //rdlock
			if (pthread_rwlock_rdlock(&rwlock) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}

			printf("스레드[%d] %d원\n", d.count, balance);
			
			if (pthread_rwlock_unlock(&rwlock) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}
		}
		
		sleep(d.time);
	}

	if (OffLimits() == OUT_OF_RANGE) {
		return NULL;
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
		free(data);
		fprintf(stderr, "errno[%d]\n", errno);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < NUM_THREAD; i++) {
		data[i].count = i;
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
			free(data);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < NUM_THREAD; i++) {
		if (pthread_join(p_thread[i], (void **)NULL) != 0) {
			free(data);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		free(data);
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	free(data);
	
	return 0;
}

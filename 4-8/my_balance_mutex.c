#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> //exit() malloc() free()
#include <errno.h> //errno
#include <string.h> //strerror_r()
#include <unistd.h> //sleep()
#include <time.h> //nanosleep() clock_gettime()

enum { ERR_BUFSIZE = 64, NUM_CHECK = 3, NUM_UPDATE = 2,
	MAX_BALANCE = 10000, OUT_OF_RANGE = 0, IN_RANGE = -1 };

static int balance;
static pthread_mutex_t mutex;

typedef struct DATA{
	int count;
	int time;
	int amount;
} DATA;

void *Check(void *data);
void *Update(void *data);
int BalanceRange();

int main()
{
	size_t i;
	balance = 0;

	pthread_t th_check[NUM_CHECK];
	pthread_t th_update[NUM_UPDATE];

	DATA *data_ch;
	data_ch = (struct DATA *)malloc(sizeof(struct DATA) * NUM_CHECK);
	DATA *data_up;
	data_up = (struct DATA *)malloc(sizeof(struct DATA) * NUM_UPDATE);

	if (pthread_mutex_init(&mutex, NULL) != 0) {
		free(data_ch);
		free(data_up);
		fprintf(stderr, "errno[%d]", errno);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < NUM_CHECK; i++) {
		data_ch[i].count = i;
		data_ch[i].time = 1;
		data_ch[i].amount = 0;
		
		if (pthread_create(&th_check[i], NULL, Check, (void *)&data_ch[i]) != 0) {
			free(data_ch);
			free(data_up);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < NUM_UPDATE; i++) {
		data_up[i].count = i + NUM_CHECK;
		
		if (i == 0) {
			data_up[i].time = 1;
			data_up[i].amount = 1000;
		}
		else if (i == 1) {
			data_up[i].time = 2;
			data_up[i].amount = -500;
		}

		if (pthread_create(&th_update[i], NULL, Update, (void *)&data_up[i]) != 0) {
			free(data_ch);
			free(data_up);
			fprintf(stderr, "errno[%d]", errno);
			exit(EXIT_FAILURE);
		}
	}

	//메인 스레드는 은행 잔고가 10,000원 이상이 되면 모든 스레드를 종료하고 프로그램을 종료함
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

	if (pthread_mutex_destroy(&mutex) != 0) {
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
	int delta = 0;

	struct timespec start = {0, 0};
	struct timespec end = {0, 0};

	DATA d = *(DATA *)data;
		
	while (balance < MAX_BALANCE) {
		if (clock_gettime(CLOCK_REALTIME, &start) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		struct timespec reqtime;
		reqtime.tv_sec = 0;
		reqtime.tv_nsec = 999999999;
		if (nanosleep(&reqtime, NULL) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		if (clock_gettime(CLOCK_REALTIME, &end) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		delta = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1000000000);
		
		if (BalanceRange() == OUT_OF_RANGE) {
			return NULL;
		}

		if (delta >= 1) {
			if (pthread_mutex_lock(&mutex) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}
			
			printf("스레드[%d] %d원\n", d.count, balance);

			if (pthread_mutex_unlock(&mutex) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}		
		}

	//	if (d.count == 2) {
	//		sleep(20);	
	//	}
	//	else {
	//		sleep(d.time);
	//	}

	}

	return NULL;	
}

void *Update(void *data)
{
	int delta = 0;

	struct timespec start = {0, 0};
	struct timespec end = {0, 0};

	DATA d = *(DATA *)data;
	
	while (balance < MAX_BALANCE) {
		if (clock_gettime(CLOCK_REALTIME, &start) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}
	
		struct timespec reqtime;
		if (d.amount > 0) {
			reqtime.tv_sec = 0;
		}
		else if (d.amount < 0) {
			reqtime.tv_sec = 1;
		}
		reqtime.tv_nsec = 999999999;
		if (nanosleep(&reqtime, NULL) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		if (clock_gettime(CLOCK_REALTIME, &end) != 0) {
			fprintf(stderr, "errno[%d]", errno);
			return NULL;
		}

		delta = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1000000000);

		if (BalanceRange() == OUT_OF_RANGE) {
			return NULL;
		}

		if (d.amount > 0 && delta >= 1) {
			if (pthread_mutex_lock(&mutex) != 0) {
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
	
			if (pthread_mutex_unlock(&mutex) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}
		}
		else if (d.amount < 0 && delta >= 2) {
			if (pthread_mutex_lock(&mutex) != 0) {
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
	
			if (pthread_mutex_unlock(&mutex) != 0) {
				fprintf(stderr, "errno[%d]", errno);
				return NULL;
			}
		}

//		sleep(d.time);
	}

	return NULL;
}

int BalanceRange()
{
	if (balance < MAX_BALANCE) {
		return IN_RANGE; //-1
	}
	return OUT_OF_RANGE; //0
}

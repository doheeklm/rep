/* shm_rcv.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#define KEY_NUM 777
#define SEM_NAME "pSem"

typedef struct _SHM {
	char name[13];
	char phone[14];
	char address[151];
} Shm;

int main()
{
	sem_t* pSem;

	if ((pSem = sem_open(SEM_NAME, O_CREAT, 0777, 0)) == SEM_FAILED) {
		fprintf(stderr, "sem_open/errno[%d]", errno);
		return 0;
	}

	int shmid = 0;
	shmid = shmget((key_t)KEY_NUM, sizeof(Shm), IPC_CREAT | 0666);
	if (shmid == -1) {
		fprintf(stderr, "shmget/errno[%d]", errno);
		goto EXIT;
	}
	
	void* shared_memory = NULL;
	struct shmid_ds buf;

	while (1) {
		if (sem_wait(pSem) == -1) {
			fprintf(stderr, "semwait/errno[%d]", errno);	
			goto EXIT;
		}
		
		if (shmctl(shmid, IPC_STAT, &buf) == -1) {
			printf("공유메모리 제거됨 - 프로세스 종료\n");
			return 0;
		}

		if (shmctl(shmid, SHM_LOCK, 0) == -1) {
			fprintf(stderr, "shmctl_lock/errno[%d]", errno);
			goto EXIT;
		}
	
		if ((shared_memory = shmat(shmid, (void *)0, 0)) == NULL) {
			fprintf(stderr, "shmat/errno[%d]", errno);
			goto EXIT;
		}
		
		FILE* fp = NULL;
		fp = fopen("./address_shm.txt", "a");
		if (fp == NULL) {
			fprintf(stderr, "fopen/errno[%d]", errno);
			continue;
		}

		if (fwrite((Shm *)shared_memory, sizeof(Shm), 1, fp) != 1) {
			fprintf(stderr, "fwrite/errno[%d]", errno);
		}

		if (fclose(fp) != 0) {
			fprintf(stderr, "fclose/errno[%d]", errno);
			break;
		}

		if (shmctl(shmid, SHM_UNLOCK, 0) == -1) {
			fprintf(stderr, "shmctl_lock/errno[%d]", errno);
			goto EXIT;
		}

		if (shmdt(shared_memory) == -1) {
			fprintf(stderr, "shmdt/errno[%d]", errno);
			goto EXIT;
		}
	}

EXIT:
	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "shmctl_rm/errno[%d]", errno);
	}

	return 0;
}

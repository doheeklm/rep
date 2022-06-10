/* shm_rcv.c */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
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
		return 0; //return 처리
	}

	int shmid = 0;
	shmid = shmget((key_t)KEY_NUM, sizeof(Shm), IPC_CREAT | 0666);
	if (shmid == -1) {
		fprintf(stderr, "shmget/errno[%d]", errno);
		goto EXIT;
	}
	
	Shm* shared_memory;
	memset(shared_memory, 0, sizeof(Shm));

	while (1) {
		if (sem_wait(pSem) == -1) {
			fprintf(stderr, "semwait[%d]", errno);	
		}

		if ((shared_memory = shmat(shmid, (void *)0, 0)) == NULL) {
			fprintf(stderr, "shmat/errno[%d]", errno);
		}

		if (shmctl(shmid, SHM_LOCK, 0) == -1) {
			fprintf(stderr, "shmctl_lock/errno[%d]", errno);
			goto EXIT;
		}
		
 		FILE* fp = NULL;
		fp = fopen("./address_shm.txt", "a");
		if (fp == NULL) {
			fprintf(stderr, "fopen/errno[%d]", errno);
			break;
		}

		if (fwrite(shared_memory, sizeof(Shm), 1, fp) != 1) {
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
	}

EXIT:
	return 0;
}

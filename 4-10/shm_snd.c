/* shm_snd.c */
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

const char* str_exit = "exit";
void ClearStdin(char* c);

int main()
{
	sem_t* pSem;

	Shm shm;
	
	int shmid = 0;
	shmid = shmget((key_t)KEY_NUM, sizeof(Shm), IPC_CREAT | 0666);
	if (shmid == -1) {
		fprintf(stderr, "shmget/errno[%d]", errno);
		return 0;
	}

	void* shared_memory = NULL;
	
	if ((pSem = sem_open(SEM_NAME, O_CREAT, 0777, 0)) == SEM_FAILED) {
		fprintf(stderr, "sem_open/errno[%d]", errno);
		return 0;  //dev/sem
	}

	while (1) {
		memset(&shm, 0, sizeof(shm));

		do {
			printf("Name: ");
			if(fgets(shm.name, sizeof(shm.name), stdin) == NULL) {
				fprintf(stderr, "shm.name/errno[%d]", errno);
				goto EXIT;
			}
			ClearStdin(shm.name);

			if (strcmp(str_exit, shm.name) == 0) {
				if (sem_post(pSem) == -1) {
					fprintf(stderr, "sempost/errno[%d]", errno);
					goto EXIT;
				}
				goto EXIT;
			}

			printf("Phone Num: ");
			if (fgets(shm.phone, sizeof(shm.phone), stdin) == NULL) {
				fprintf(stderr, "shm.phone/errno[%d]", errno);
				goto EXIT;
	 		}
 			ClearStdin(shm.phone);

			if ((shm.phone[3] != '-') || (shm.phone[8] != '-')) {
				printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
				printf("처음으로 돌아갑니다.\n");
			}
		} while ((shm.phone[3] != '-') || (shm.phone[8] != '-'));

		printf("Address: ");
		if (fgets(shm.address, sizeof(shm.address), stdin) == NULL) {
			fprintf(stderr, "shm.address/errno[%d]", errno);
			goto EXIT;
		}
		ClearStdin(shm.address);
		
		if (shmctl(shmid, SHM_LOCK, 0) == -1) {
			fprintf(stderr, "shmctl_lock/errno[%d]", errno);
			goto EXIT;
		}
		
		if ((shared_memory = shmat(shmid, (void *)NULL, 0)) == NULL) {
			fprintf(stderr, "shmat/errno[%d]", errno);
			goto EXIT;
		}

		memcpy(shared_memory, &shm, sizeof(shm));

		if (shmctl(shmid, SHM_UNLOCK, 0) == -1) {
			fprintf(stderr, "shmctl_lock/errno[%d]", errno);
			goto EXIT;
		}

		if (sem_post(pSem) == -1) {
			fprintf(stderr, "sempost/errno[%d]", errno);
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

	if (sem_destroy(pSem) == -1) {
		fprintf(stderr, "sem_destroy/errno[%d]", errno);
	}

	return 0;
}

void ClearStdin(char* c)
{
	if (c == NULL) {
		return;
	}

	if (c[strlen(c) - 1] == '\n') {
		c[strlen(c) - 1] = '\0';
	}

	__fpurge(stdin);
}

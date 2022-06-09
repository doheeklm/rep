/* shm_rcv.c */
#include <stdio.h>
#include <string.h> //strcmp()
#include <errno.h> //errno
#include <stdlib.h> //exit()
#include <sys/ipc.h> //shmget() shmctl() ftok()
#include <sys/types.h> //shmat() shmdt() ftok()
#include <sys/shm.h> //shmget() shmat() shmdt() shmctl()
#include <pthread.h> //pthread_mutex_init() pthread_mutex_lock() pthread_mutex_unlock() pthread_mutex_destroy()
#include <semaphore.h> //sem_init() sem_getvalue() sem_wait() sem_post() sem_destroy()
#include <stdlib.h> //malloc() free()
#include <stdio_ext.h> //__fpurge()
#include <unistd.h> //getpid()

#define KEY_NUM 777

typedef struct _SHM {
	char name[13]; //이름 한글 최대 4자
	char phone[14]; //전화번호 xxx-xxxx-xxxx
	char address[151]; //주소 한글 최대 50자
} Shm;

int main()
{
	Shm shm;
	memset(&shm, 0, sizeof(shm));

	//키 생성
//	key_t key = 0;
//	if ((key = ftok(".", 'B')) == -1) {
//		fprintf(stderr, "ftok/errno[%d]", errno);
//		goto EXIT;
//	}

	//공유 메모리 생성
	int shmid = 0;
	shmid = shmget((key_t)KEY_NUM, sizeof(shm), IPC_CREAT | 0666);
	if (shmid == -1) {
		fprintf(stderr, "shmget/errno[%d]", errno);
		goto EXIT;
	}
	
	//프로세스에서 공유 메모리 분리
/*	if (shmdt(&shm) == -1) {
		fprintf(stderr, "shmdt/errno[%d]", errno);
		goto EXIT;
	}

	struct shmid_ds buf;
	//공유 메모리 영역 제어
	if (shmctl(shmid, IPC_STAT, &buf) == -1) {
		fprintf(stderr, "shmctl/errno[%d]", errno);
		goto EXIT;
	}
*/
	//파일 열기
	FILE* fp = NULL;
	fp = fopen("./address_shm.txt", "a");
	if (fp == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}

	if (fwrite(&shm, sizeof(shm), 1, fp) != 1) {
		fprintf(stderr, "errno[%d]", errno);
	}

	if (fclose(fp) != 0) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}

EXIT:
	return 0;
}

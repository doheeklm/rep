/* shm_snd.c */
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

typedef struct _SHM {
	char name[13]; //이름 한글 최대 4자
	char phone[14]; //전화번호 xxx-xxxx-xxxx
	char address[151]; //주소 한글 최대 50자
} Shm;

const char* str_exit = "exit"; //Name에 exit을 입력 받으면 프로그램 종료
void ClearStdin(char* c); //버퍼 삭제

int main()
{
	Shm shm;
	memset(&shm, 0, sizeof(shm));

	//키 생성
	key_t key = 0;
	if ((key = ftok(".", 'B')) == -1) {
		fprintf(stderr, "errno[%d]", errno);
		//return
	}

	//공유 메모리 생성
	int shmid = 0;
	shmid = shmget(key, sizeof(shm), IPC_CREATE | IPC_EXCL | 0666);
	if (shmid == -1) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}

	do {
		//이름 입력받기
		printf("Name: ");
		if(fgets(shm.name, sizeof(shm.name), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT;
		}
		ClearStdin(shm.name);

		//전화번호 입력받기
		printf("Phone Num: ");
		if (fgets(shm.phone, sizeof(shm.phone), stdin) == NULL) {
			fprintf(stderr, "errno[%d]", errno);
			goto EXIT;
	 	}
 		ClearStdin(shm.phone);

		//입력한 전화번호 형태가 잘못된 경우, 에러 문구를 출력하고 이름부터 다시 입력 받음
		if ((shm.phone[3] != '-') || (shm.phone[8] != '-')) {
			printf("전화번호를 xxx-xxxx-xxxx 형태로 입력해주세요.\n");
			printf("처음으로 돌아갑니다.\n");
		}
	} while ((shm.phone[3] != '-') || (shm.phone[8] != '-'));

	//주소 입력받기
	printf("Address: ");
	if (fgets(shm.address, sizeof(shm.address), stdin) == NULL) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}
	ClearStdin(shm.address);

	if (shmat(shmid, &shm, SHM_RDONLY) == -1) {
		fprintf(stderr, "errno[%d]", errno);
		goto EXIT;
	}

EXIT:
	return 0;
}

void ClearStdin(char* c)
{
	if (c == NULL) {
		return;
	}

	//개행문자를 널문자로 변환
	if (c[strlen(c) - 1] == '\n') {
		c[strlen(c) - 1] = '\0';
	}

	//버퍼 비우기
	__fpurge(stdin);
}

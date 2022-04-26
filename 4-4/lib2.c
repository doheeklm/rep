#include <stdio.h>
#include <stdlib.h> //malloc() free()
#include <errno.h> //errno
#include <string.h> //strerror() strlen()
#include <fcntl.h> //open()
#include <unistd.h> //read() write() lseek() close()
#include <stdio_ext.h> //__fpurge
#include <math.h> //pow()

enum { MAX = 50, nSub = 8 };

typedef struct Info {
	int id;
	char name[13];
	unsigned char sub;
} Info;

const char* Subject[nSub] =
{ "컴퓨터개론", "이산수학", "C언어", "JAVA초급",
	"리눅스구조", "자료구조", "컴파일러", "네트워크개론" };

void PrintInfo(Info* info);

void ClearStdin(char* str)
{
	if (str == NULL) {
		return;
	}
	if (str[strlen(str) - 1] == '\n') {
		str[strlen(str) - 1] = '\0';
	}
	__fpurge(stdin);
}

int main()
{
	const char* path = "./f.txt";
	
	int fd = 0;
	FILE* fp;

	size_t rd = 0;
	size_t wr = 0;

	int count = 0;

	int select = 0;
	int backToMenu = 0;

	int tempID = 0;
	int deleteID = 0;
	
	char temp[200]; //과목명 입력받는 임시변수
	memset(temp, 0, sizeof(temp));
	int option = 0; //비트

	char search[13];
	memset(search, 0, sizeof(search));

	Info info[MAX];
	memset(info, 0, sizeof(info));

	fd = open(path, O_RDWR | O_CREAT, 0777);
	if (fd == -1) {
		fprintf(stderr, "OPEN errno[%d] : %s\n", errno, strerror(errno));
		return 0;
	}

	fp = fdopen(fd, "r+");
	if (fp == NULL) {
		fprintf(stderr, "fp errno[%d] : %s\n", errno, strerror(errno));
		return 0;
	}

	rd = fread(info, sizeof(info), 1, fp); //== fread(info, sizeof(*info), MAX, fp)
	printf("read %ld\n", rd);

	for (int i = 0; i < MAX; i++) {
		if (info[i].id == '\0') {
			printf("info[%d] 비어있음\n",i);
			count = i;
			break;
		}
	}

	printf("현재 저장된 정보 %d개\n", count);

	do {
		backToMenu = 0;
        
		printf("======================\n");
		printf("1. 수강 신청 정보 입력\n");
		printf("2. 수강 신청 정보 출력\n");
		printf("3. 종료\n");
		printf("4. 삭제\n");
		printf("======================\n");
		printf("select num: ");
		scanf("%d", &select);
		while (getchar() != '\n') {};
		if (select != 1 && select != 2 && select != 3 && select != 4) {
			printf("숫자를 다시 입력해주세요.\n");
		}
	
		switch (select) {
			case 1:
				{
					if (count >= MAX) {
						printf("최대 입력 갯수를 초과했습니다. 메뉴로 돌아갑니다.\n");
						break;
					}
					
					printf("아이디: ");
					scanf("%d", &tempID);
					while (getchar() != '\n') {};
					if (tempID < 1 || tempID > 1000) {
						printf("아이디의 범위는 1부터 1000입니다. 메뉴로 돌아갑니다.\n");
						break;
					}

					for (int i = 0; i < MAX; i++) {
						if (info[i].id == tempID) {
							printf("아이디가 중복되었습니다. 메뉴로 돌아갑니다.\n");
							backToMenu = 1;
							break;
						}
					}
					if (backToMenu == 1) {
						break;
					}

					info[count].id = tempID;			

					printf("이름: ");
					fgets(info[count].name, sizeof(info[count].name), stdin);
					ClearStdin(info[count].name);

					printf("수강 신청 과목: ");
					fgets(temp, sizeof(temp), stdin);
					ClearStdin(temp);
					int j = 0;
					for (size_t i = 0; i < sizeof(temp); i++) {
						if (temp[i] != ' ') {
							temp[j++] = temp[i];
						}
					}
					temp[j] = '\0';

					info[count].sub = 0; //0000 0000

					char* cut = strtok(temp, ",");
					while (cut != NULL) {
						printf("cut = %s\n", cut);
						for (int i = 0; i < nSub; i++) {
							if (strcmp(Subject[i], cut) == 0) {
								option = (int)pow(2, i); //option 2^3 = 8 ==> 0000 1000
								info[count].sub |= option;
								backToMenu = 0;
								break;
							}
							else {
								backToMenu = 1;
							}
						}
						cut = strtok(NULL, ",");
					}
	
					if (backToMenu == 1) {
						printf("존재하지 않은 과목명을 입력했습니다. 메뉴로 돌아갑니다.\n");
						memset(&info[count], 0, sizeof(info[count]));
						break;
					}
					else {
						printf("info[%d] 입력\n", count);
						count++;

						if (fseek(fp, 0L, SEEK_SET) != 0) {
							fprintf(stderr, "fseek errno[%d] : %s\n", errno, strerror(errno));
							return 0;
						}

						wr = fwrite(info, sizeof(info), 1, fp); //== fwrite(info, sizeof(*info), MAX, fp)
						printf("write %ld\n", wr);
					
						if (wr != 1) {
							fprintf(stderr, "fwrite errno[%d] : %s\n", errno, strerror(errno));
							return 0;
						}
						else {
							fflush(fp);
						}
					}
				}
				break;
			case 2:
				{
					printf("이름 검색: ");
					fgets(search, sizeof(search), stdin);
					ClearStdin(search);

					for (int i = 0; i < count; i++) {
						if (strcmp(search, info[i].name) == 0) {
							PrintInfo(&info[i]);
						}
						else if (strcmp(search, "all") == 0) {
							PrintInfo(&info[i]);
						}
					}
				}
				break;
			case 3:
				{
					printf("종료합니다.\n");
					
					/*
					if (fseek(fp, 0L, SEEK_SET) != 0) {
						fprintf(stderr, "fseek errno[%d] : %s\n", errno, strerror(errno));
						return 0;
					}

					wr = fwrite(info, sizeof(info), 1, fp); //== fwrite(info, sizeof(*info), MAX, fp)
					printf("wr %ld\n", wr);
					
					if (wr != 1) {
						fprintf(stderr, "fwrite errno[%d] : %s\n", errno, strerror(errno));
						return 0;
					}
					*/

					if (fclose(fp) != 0) {
						fprintf(stderr, "CLOSE errno[%d] : %s\n", errno, strerror(errno));
						return 0;
					}
				}
				break;
			case 4:
				{
					printf("삭제할 아이디: ");
					scanf("%d", &deleteID);
					while (getchar() != '\n') {};

					for (int i = 0; i < MAX; i++) {
						if (info[i].id == deleteID) {
							printf("info[%d] 삭제\n", i);
							if (i == count - 1) {
								memset(&info[i], 0, sizeof(info[i]));
							}
							else {
								for (int j = i; j < MAX; j++) {
									info[j].id = info[j + 1].id;
									strcpy(info[j].name, info[j + 1].name);
									info[j].sub = info[j + 1].sub;
								}
								memset(&info[count - 1], 0, sizeof(info[count - 1]));
							}
							count--;
							break;
						}
					}
				}
				break;
		}
	} while (select != 3);

	return 0;
}

void PrintInfo(Info* info)
{
	size_t length = strlen(info->name);

	printf("ID: ");
	if (info->id < 10) {
		printf("   ");
	}
	else if (info->id < 100) {
		printf("  ");
	}
	else if (info->id < 1000) {
		printf(" ");
	}

	printf("%d, 이름: %s", info->id, info->name);
	
	if (length == 3) {
		printf("      ");
	}
	else if (length == 6) {
		printf("    ");
	}
	else if (length == 9) {
		printf("  ");
	}

	printf(", 수강신청 과목: ");

	for (int i = 0; i < nSub; i++) {
		if (info->sub & (int)pow(2, i)) {
			printf("%s ", Subject[i]);
		}
	}
	printf("\n");
}

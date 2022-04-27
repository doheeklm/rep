#include <stdio.h>
#include <stdlib.h> //malloc() free()
#include <errno.h>
#include <sys/types.h> //open()
#include <sys/stat.h> //open()
#include <fcntl.h> //open()
#include <unistd.h> //close()
#include <string.h> //strcmp() memset()

enum { CLOSED = 1 };

void Usage()
{
	printf("Usage: c [-f] src_file dest_file\n");
	printf("          -f: dest_file 존재시 src_file 내용으로 바꿈\n");
}

int CLOSE(FILE* stream)
{
	if (fclose(stream) != 0) {
		fprintf(stderr, "fclose errno[%d] : %s\n", errno, strerror(errno));
		//errno
		return 0;
	}
    
	return CLOSED;
}

int main(int argc, char* argv[])
{
	/* 인자의 숫자가 잘못되면 사용법 출력 */
	if (argc != 3 && argc != 4) {
		Usage();
		return 0;
	}

	/* 인자의 숫자가 4개인데, -f가 아닌 다른 것이 입력되었을때 사용법 출력 */
	if (argc == 4 && (strcmp(argv[1], "-f") != 0)) {
		Usage();
		return 0;
	}

	/* -f 입력 되었을때 */
	int f = 0;
	if (strcmp(argv[1], "-f") == 0) {
		f = 1;
	}
   
	/* src 파일 존재 여부 확인 */
	const char* path_src = argv[1 + f];
	if (access(path_src, F_OK) == -1) {	
		fprintf(stderr, "access src errno[%d] : %s\n", errno, strerror(errno));
		if (errno == ENOENT) {
			printf("존재하는 파일이 없습니다.\n");
			return 0;
		}
		else {
			return 0;
		}
	}

	int fd_src = open(path_src, O_RDONLY);
	if (fd_src == -1) {
		fprintf(stderr, "fd_src errno[%d] : %s\n", errno, strerror(errno));
		//errno
		return 0;
	}

	/* 읽기 전용으로 src파일 fopen */
	FILE* fp_src = fdopen(fd_src, "r");
	if (fp_src == NULL) {
		fprintf(stderr, "fp_src errno[%d] : %s\n", errno, strerror(errno));
		//errno EINVAL
		return 0;
	}

	/* dest파일이 존재할 때의 에러 처리를 위해 open(), fdopen() 사용 */
	const char* path_dest = argv[2 + f];
	int fd_dest;
	FILE* fp_dest;

	if (f == 0) {
		fd_dest = open(path_dest, O_CREAT | O_EXCL | O_WRONLY);
		if (fd_dest == -1) {	
			fprintf(stderr, "fd_dest errno[%d] : %s\n", errno, strerror(errno));
			//errno
			return 0;
		}

		/* fdopen() */
		fp_dest = fdopen(fd_dest, "w");
	}
	else if (f == 1) {
		/* fopen() : 해당 파일이 존재하면 해당 파일의 내용을 삭제함 */
		fp_dest = fopen(path_dest, "w");

		if (fp_dest == NULL) {
			fprintf(stderr, "fp_dest errno[%d] : %s\n", errno, strerror(errno));
			//errno
			return 0;
		}
	}
    
	char buffer[1000];

	while (1) {
		memset(buffer, 0, sizeof(buffer));

		size_t rd = fread(buffer, sizeof(buffer), 1, fp_src);
		size_t wr = fwrite(buffer, sizeof(buffer), 1, fp_dest);

		if (rd != 1) {
			fprintf(stderr, "fread errno[%d] : %s\n", errno, strerror(errno));
			//errno EISDIR
			return 0;
		}
		
		if (wr != 1) {
			fprintf(stderr, "fwrite errno[%d] : %s\n", errno, strerror(errno));
			//errno
			return 0;
		}
	}
	
	if (CLOSE(fp_src)) {
		CLOSE(fp_dest);
	}

	printf("Copy success\n");

	return 0;
} 

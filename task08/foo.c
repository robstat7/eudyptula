/*
 * Program to demonstrate simultaneous
 * writing and reading the foo file.
 */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

void *write_foo(void *filed)
{
	int *fd;
	const char *wdata = "Kernel development is cool!\n";
	size_t count;

	fd = (int *)filed;
	count = strlen(wdata);
	if (write(*fd, wdata, count) == -1)
		printf("Could not write foo!\n");
	else
		printf("root: wrote foo!\n");

	pthread_exit(NULL);
}

void *read_foo(void *filed)
{
	int i;
	int *fd;
	ssize_t bytes;
	static const size_t page_size = 4096;
	char rdata[page_size];

	fd = (int *)filed;
	bytes = read(*fd, &rdata, page_size);
	if (bytes == -1) {
		printf("Could not read foo!\n");
	} else {
		printf("root: read foo!\n");
		for (i = 0; i < bytes; i++)
			printf("%c", rdata[i]);
	}

	pthread_exit(NULL);
}

int main(void)
{
	int fd;
	int i;
	int error;
	const char *filename = "/sys/kernel/debug/eudyptula/foo";
	pthread_t t1, t2;

	fd = open(filename, O_RDWR, 0600);
	if (fd == -1) {
		printf("Cannot open %s.\n", filename);
		return 1;
	}

	error = pthread_create(&t1, NULL, write_foo, (void *) &fd);
	if (error) {
		printf("pthread_create failed! Error code is %d\n", error);
		return 1;
	}
	error = pthread_create(&t2, NULL, read_foo, (void *) &fd);
	if (error) {
		printf("pthread_create failed! Error code is %d\n", error);
		return 1;
	}

	pthread_exit(NULL);

	if (close(fd) == -1) {
		printf("Cannot close %s.\n", filename);
		return 1;
	}
	return 0;
}

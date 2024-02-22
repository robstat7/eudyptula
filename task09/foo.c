/*
 * Program to demonstrate concurrent
 * writing and reading of the 'foo' file.
 */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

const char *filename = "/sys/kernel/eudyptula/foo";

int open_file(int oflag)
{
	int fd;

	fd = open(filename, oflag, 0600);
	if (fd == -1) {
		printf("Cannot open %s.\n", filename);
		exit(1);
	}
	return fd;
}

void close_file(int fd)
{
	if (close(fd) == -1) {
		printf("Cannot close %s.\n", filename);
		exit(1);
	}
}

void *write_foo(void *arg)
{
	int fd;
	const char *wdata = "Kernel development is cool!";
	size_t count;

	fd = open_file(O_WRONLY);
	count = strlen(wdata);
	if (write(fd, wdata, count) == -1)
		printf("Could not write foo!\n");
	else
		printf("root: wrote foo!\n");

	close_file(fd);
	pthread_exit(NULL);
}

void *read_foo(void *arg)
{
	int i;
	int fd;
	ssize_t bytes;
	static const size_t page_size = 4096;
	char rdata[page_size];

	fd = open_file(O_RDONLY);
	bytes = read(fd, &rdata, page_size);
	if (bytes == -1) {
		printf("Could not read foo!\n");
	} else {
		printf("root: read foo!\n");
		for (i = 0; i < bytes; i++)
			printf("%c", rdata[i]);
		printf("\n");
	}

	close_file(fd);
	pthread_exit(NULL);
}

int main(void)
{
	int i;
	int error;
	pthread_t t1, t2;

	error = pthread_create(&t1, NULL, write_foo, NULL);
	if (error) {
		printf("pthread_create failed! Error code is %d\n", error);
		return 1;
	}
	error = pthread_create(&t2, NULL, read_foo, NULL);
	if (error) {
		printf("pthread_create failed! Error code is %d\n", error);
		return 1;
	}

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return 0;
}

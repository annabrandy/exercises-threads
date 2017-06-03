// with pthread_mutex

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <pthread.h>

#define AMOUNT 4
#define FIFO "fifo"

int fd;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* function(void* voidptr) {

	unsigned int sum = 0, received = 0;

	while(1) {

		pthread_mutex_lock(&mutex);
		if(read(fd, &received, sizeof(received)) < 0)
			return NULL;
		pthread_mutex_unlock(&mutex);

		sum += received;
		if(received == 0) {
			printf("sum: %u\n", sum);
			return NULL;
		}

	}

}

int main() {

	mkfifo(FIFO, 0666);
	fd = open(FIFO, O_RDWR);
	pthread_t threads[AMOUNT];
	
	for(int i=0; i<AMOUNT; i++)
		pthread_create(threads+i, NULL, &function, NULL);
	
	unsigned int number = 1;

	for(int i=0; i<100000; i++) {
		//pthread_mutex_lock(&mutex);
		write(fd, &number, sizeof(number));
		//pthread_mutex_unlock(&mutex);
		if(i>=99995)
			number = 0;
	}

	for(int i=0; i<AMOUNT; i++) {
		pthread_join(threads[i], NULL);
	}

}








// with pthread_mutex
// phuong@dps.uibk.ac.at
	// write that we are from juanjo

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <pthread.h>

#define AMOUNT 4
#define SIGNALS 100000
#define FIFO "fifoo"

int fd;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* function(void* voidptr) {

	unsigned int sum = 0, received = 0;

	for(;;) {

		pthread_mutex_lock(&mutex); 
		pthread_cond_wait(&cond, &mutex);
		if(read(fd, &received, sizeof(received)) < 0)
			return NULL;
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);

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
	pthread_cond_signal(&cond);

	for(int i=0; i<SIGNALS; i++) {
		if(i>=SIGNALS-AMOUNT)
			number = 0;
		write(fd, &number, sizeof(number));
	}

	for(int i=0; i<AMOUNT; i++)
		pthread_join(threads[i], NULL);

}








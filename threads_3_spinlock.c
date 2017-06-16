// with pthread_spin_lock

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <pthread.h>

#define AMOUNT 4
#define FIFO "fifo"

int fd;
pthread_spinlock_t lock;

void* function(void* voidptr) {

	unsigned int sum = 0, received = 0;

	while(1) {

		pthread_spin_lock(&lock);
		if(read(fd, &received, sizeof(received)) < 0)
			return NULL;
		pthread_spin_unlock(&lock);

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

	pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);
	
	for(int i=0; i<AMOUNT; i++)
		pthread_create(threads+i, NULL, &function, NULL);
	
	unsigned int number = 1;

	for(int i=0; i<100000; i++) {
		write(fd, &number, sizeof(number));
		if(i>=99995)
			number = 0;
	}

	for(int i=0; i<AMOUNT; i++)
		pthread_join(threads[i], NULL);

}








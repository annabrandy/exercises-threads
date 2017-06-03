#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define AMOUNT 10

void exit_handler(void *voidptr) {

	int *ptr = (int *)voidptr;
	int fd = *ptr;
	close(fd);

}

void* thread_function(void *numberptr) {

	unsigned int random;
	int fd, *iptr;
	pid_t tid;
	char cnumber;

	random = rand()%3;	/* wait for 0 to 2 seconds */
	sleep(random);

	iptr = (int *)numberptr;	/* create char from numberptr */
	cnumber = 48+(*iptr);
	char filename[] = {'t', 'h', 'r', 'e', 'a', 'd', cnumber, '.', 't', 'x', 't'};

	pthread_cleanup_push(&exit_handler, &fd);	/* install exit_handler */

	if((fd = open(filename, O_RDWR|O_CREAT))<0) {	/* creat and open file */
		perror("open file");
		return NULL;
	}
		
	tid = syscall(SYS_gettid); 	/* "undefined reference to gettid" -> syscall used */
	
	dprintf(fd, "%d", tid);	/* write to fd with dprintf */

	pthread_cleanup_pop(1);	/* deinstall and execute exit_handler */

}

int main() {
	
	int random;
	int number[AMOUNT];
	pthread_t threads[AMOUNT];

	for(int i=0; i<AMOUNT; i++) {
		number[i] = i;
		pthread_create(threads+i, NULL, &thread_function, number+i);
	}

	for(int i=0; i<AMOUNT; i++) {
		random = rand()%2;
		if(random)
			pthread_cancel(threads[i]);
	}

	for(int i=0; i<AMOUNT; i++) 
		pthread_join(threads[i], NULL);

}













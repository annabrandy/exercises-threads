#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define AMOUNT 10000

/** DYNAMIC */
pthread_mutex_t mutex;
pthread_spinlock_t lock;
pthread_cond_t cond;
pthread_mutex_t mutex_cond;

/** STATIC */
/*
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_spinlock_t lock;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;
*/

// cond: lock mutex - wait cond (unlocks mutex, waits for cond, locks mutex) - unlock mutex - signal cond



/** could for example close a fd opened in thread_function, print some information, unlock access control ...
	automatically executed when thread was canceled/interrupted 
	or pthread_cleanup_pop(1) called, otherwise ignored */
void exit_handler(void *voidptr) {

	int argument = *((int *)voidptr);	/* cast information from pthread_cleanup_push to int */
	/*pthread_mutex_unlock(&mutex);
	pthread_spin_unlock(&lock);
	pthread_mutex_unlock(&mutex_cond);
	pthread_cond_signal(&cond);*/

}



/** launched when thread was created with pthread_created and that funcion as argument
    not synchron as processes, but asynchron -> no exec needed
    can receive a void pointer from pthread_create, has to be casted */
void* thread_function(void *voidptr) {

	int argument = *((int *)voidptr);	/* cast information from pthread_create to int */

	/* process starts exit_handler function if interrupted, otherwise it would exit normally */
	pthread_cleanup_push(&exit_handler, &argument);	/* install exit_handler */

	pid_t tid = syscall(SYS_gettid); 	/* "undefined reference to gettid" -> syscall used */
	pthread_t id = pthread_self();

	pthread_mutex_lock(&mutex);							//printf("locked mutex\n");
	/* or: pthread_mutex_trylock(&mutex) -> returns immediately when already locked */
	// do something critical here (read from memory, queue ...)
	pthread_mutex_unlock(&mutex);						//printf("unlocked mutex\n");

	pthread_spin_trylock(&lock); 						//printf("locked spin\n");
	/* or: pthread_spin_lock(&lock); */
	// do something critical here (read from memory, queue ...)
	pthread_spin_unlock(&lock); 						//printf("unlocked lock\n");

	pthread_mutex_lock(&mutex_cond); 					//printf("locked mutex_cond\n");
	pthread_cond_wait(&cond, &mutex_cond); 				//printf("locked cond\n");
	// do something critical here (read from memory, queue ...)
	pthread_mutex_unlock(&mutex_cond); 					//printf("unlocked mutex_cond\n");
	pthread_cond_signal(&cond); 						//printf("unlocked cond\n");
	// pthread_cond_broadcast(&cond); // signals all threads waiting for cond

	/* works like stack, installed first -> deinstalled last */
	/* deinstalling it was necessery for bugfixing */
	/* argument not 0: deinstall and execute. argument 0: just deinstall */
	pthread_cleanup_pop(0);	/* deinstall and execute (last installed) exit_handler */

	/* argument is send to thread calling join */
	pthread_exit((void*)0);

}



/** what the main-thread can do: create, cancel and wait for threads
    thread id can be stored when calling pthread_create (pthread_t *)
    thread id (pthread_t) is needed for cancel thread and join (wait for thread) */
int main() {
	
	int number[AMOUNT];
	pthread_t threads[AMOUNT];

	/* beacause dynamic:  mutex and cond initializable with NULL or attribute */
	pthread_mutex_init(&mutex, NULL); 
	pthread_mutex_init(&mutex_cond, NULL);
	pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE);
	// or: pthread_spin_init(&lock, PTHREAD_PROCESS_SHARED); // can be accessed from threads of other processes
	pthread_cond_init(&cond, NULL);


	for(int i=0; i<AMOUNT; i++) {	/* spawn AMOUNT threads and store id */
		number[i] = i;
		pthread_create(threads+i, NULL, &thread_function, number+i);
	}

	//printf("main thread wakes up one thread waiting for cond\n");
	pthread_cond_signal(&cond);

//	for(int i=0; i<AMOUNT; i++)		/* cancel threads */ 
//		if(pthread_cancel(threads[i]) != 0)
//			perror("pthread_cancel");
		/* can also be done with pthread_testcancel, where cancel states 
		or types can be defined -> immediately, after sth happens ... */

	for(int i=0; i<AMOUNT; i++) 	/* wait for all threads */
		/* stores return value of thread int second arument void **retval */
		/* if thread not already called pthread_detach(pthread_t thread) */
		if(pthread_join(threads[i], NULL) != 0)	
			perror("waiing for thread failed");


	/* because dynamic: */
	pthread_mutex_destroy(&mutex);
	pthread_spin_destroy(&lock);
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex_cond);


}






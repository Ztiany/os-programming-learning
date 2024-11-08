/*
 ============================================================================

 Author      : Ztiany
 Description : Linux线程：线程控制

 ============================================================================
 */

/*
具体参考：https://akaedu.github.io/book/ch35s02.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *thr_fn1(void *arg)
{
	printf("thread 1 returning\n");
	return (void *)1;
}

void *thr_fn2(void *arg)
{
	printf("thread 2 exiting\n");
	pthread_exit((void *)2);
}

void *thr_fn3(void *arg)
{
	while(1) {
		printf("thread 3 writing\n");
		sleep(1);
	}
}

int main(void)
{
	pthread_t   tid;
	void        *tret;

	pthread_create(&tid, NULL, thr_fn1, NULL);
	pthread_join(tid, &tret);
	printf("thread 1 exit code %d\n", (int)tret);

	pthread_create(&tid, NULL, thr_fn2, NULL);
	pthread_join(tid, &tret);
	printf("thread 2 exit code %d\n", (int)tret);

	pthread_create(&tid, NULL, thr_fn3, NULL);
	sleep(3);
	pthread_cancel(tid);
	pthread_join(tid, &tret);
	printf("thread 3 exit code %d\n", (int)tret);

	return 0;
}
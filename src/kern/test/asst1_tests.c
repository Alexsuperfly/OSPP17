//100% actual GARBAGE
//if it doesnt compile which it will not
//comment all of it out dawg

#include <synch.h>
#include <thread.h>
#include <wchan.h>
#include <spinlock.h>
#include <lib.h>
#include <test.h>


static struct lock *mylock;
static struct cv *mycv;
static struct semaphore *sem, *sem2;



static unsigned long myval;

///////////////////////////////////////////////////////////////////////////////
/*
	thread_fork entrypoint - cv
*/
static int entrypoint(void *data1, unsigned long data2){
	(void)data1;
	lock_acquire(mylock);
	V(sem);
	while(myval != data2);
		cv_wait(mycv, mylock);
	V(sem);
	lock_release(mylock);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/*
	CV tests
*/

// cv_create
static void test_cv_create(){
	kprintf("begin test_cv_create\n");
    if (mycv == NULL) {
        mycv = cv_create("mycv");
        if (mycv == NULL)
            panic("FAIL test_cv_create\n");
    }
	kprintf("end test_cv_create\n");
}

// cv_wait
static void test_cv_wait(){
	kprintf("begin test_cv_wait\n");
	
	test_lock_create();
	test_cv_create();
	sem = sem_create("sem", 0);
	
	int h = 0;
	h = thread_fork("testCVwait", NULL, entrypoint, NULL, 1);
	if(h)
		panic("FAIL test_cv_wait, thread_fork\n");
	
	P(sem);
	
	lock_acquire(mylock);
	myval++;		// wake thread
	cv_signal(mycv, mylock);
	lock_release(mylock);
	
	P(sem);
	
	kprintf("end test_cv_wait\n");
}

// cv _signal
static void test_cv_signal(){
	kprintf("begin test_cv_signal\n");
	
	test_lock_create();
	test_cv_create();
	sem = sem_create("sem", 0);
	
	int h = 0;
	h = thread_fork("testCVsignal1", NULL, entrypoint, NULL, 1);
	if(h)
		panic("FAIL test_cv_signal, thread_fork\n");
	
	h = thread_fork("testCVsignal2", NULL, entrypoint, NULL, 1);
	if(h)
		panic("FAIL test_cv_signal, thread_fork\n");
	
	
	P(sem);
	P(sem);
	// increment once to wake one
	
	lock_acquire(mylock);
	myval++;
	cv_signal(mycv, mylock);
	lock_release(mylock);
	
	P(sem);
	
	lock_acquire(mylock);
	cv_signal(mycv, mylock);
	lock_release(mylock);
	
	P(sem);
	
	kprintf("end test_cv_signal\n");
}

// cv_broadcast
static void test_cv_broadcast(){
	kprintf("begin test_cv_broadcast\n");
	
	test_lock_create();
	test_cv_create();
	sem = sem_create("sem", 0);
	
	int h;
	h = thread_fork("testCVbroadcast1", NULL, entrypoint, NULL, 1);
	if(h)
		panic("FAIL test_cv_broadcast, thread_fork\n");
	
	h = thread_fork("testCVbroadcast2", NULL, entrypoint, NULL, 1);
	if(h)
		panic("FAIL test_cv_broadcast, thread_fork\n");
	h = thread_fork("testCVbroadcast3", NULL, entrypoint, NULL, 1);
	if(h)
		panic("FAIL test_cv_broadcast, thread_fork\n");
	
	P(sem);
	P(sem);
	P(sem);
	
	// same deal
	
	lock_acquire(mylock);
	myval++;
	cv_broadcast(mycv, mylock);
	lock_release(mylock);
	
	P(sem);
	P(sem);
	
	lock_acquire(mylock);
	myval++;
	cv_broadcast(mycv, mylock);
	lock_release(mylock);
	
	P(sem);
	
	kprintf("end test_cv_broadcast\n");
}

///////////////////////////////////////////////////////////////////////////////
/*
	Lock tests
*/
// lock_create
static void test_lock_create(){
	kprintf("begin test_lock_create\n");
	if (mylock == NULL) {
        mylock = lock_create("mylock");
		if (mylock == NULL)
            panic("FAIL test_lock_create\n");
    }
	
	kprintf("begin test_lock_create\n");
}

/*
	thread_fork entrypoint - lock
*/
static int entrypoint_lock(void *data1, unsigned long data2){
	(void)data1;
	lock_acquire(mylock);
	
	if(!lock_do_i_hold(mylock)){
		kprintf("FAIL test_lock_holder - should hold lock, but don't\n");
		lock_release(mylock);
	}
	
	lock_release(mylock);
	
	
	V(sem2);
	return 0;
}

static void test_lock_holder(){
	kprintf("begin test_lock_holder\n");
	
	test_lock_create();
	sem2 = sem_create("sem2", 0);

	
	int h;
	h = thread_fork("testLockHolder", NULL, entrypoint_lock, NULL, 1);
	if(h)
		panic("FAIL test_lock_holder, thread_fork\n");
	
	P(sem2);
	
	kprintf("end test_lock_holder\n");
}

///////////////////////////////////////////////////////////////////////////////
/*
	thread_join tests
*/


static int threadstuff(void *data1, unsigned long data2){
	(void)data1;
	kprintf("thread # %ld\n" data2);
	return 100+data2;
}

static void test_thread_join(){
	struct thread *threads[10];
	int hold;
	
	for(int i = 0; i < 10; i++)
		thread_forking("t", NULL, &threadstuff, NULL, i, &(threads[i]));
	
	for(int i = 0; i < 10; i++)
		hold = thread_join(threads[i]);
	
}

///////////////////////////////////////////////////////////////////////////////
/*
	call me to run tests
*/
int asst1_tests(){
	// cv
	test_cv_create();
	test_cv_wait();
	test_cv_signal();
	test_cv_broadcast();
	
	// lock
	test_lock_create();
	test_lock_holder();
	
	// thread_join
	test_thread_join();
	
	return 0;
}
















///////////////////////////////////////////////////////////////////////////////

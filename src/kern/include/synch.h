#ifndef _SYNCH_H_
#define _SYNCH_H_

/*
 * Header file for synchronization primitives.
 */
#include <spinlock.h>

/*
 * Dijkstra-style semaphore.
 * The name field is for easier debugging. A copy of the name is made
 * internally.
 */
struct semaphore {
        char *sem_name;
	struct wchan *sem_wchan;
	struct spinlock sem_lock;
        volatile unsigned sem_count;
};

struct semaphore *sem_create(const char *name, unsigned initial_count);
void sem_destroy(struct semaphore *);

/*
 * Operations (both atomic):
 *     P (proberen): decrement count. If the count is 0, block until
 *                   the count is 1 again before decrementing.
 *     V (verhogen): increment count.
 */
void P(struct semaphore *);
void V(struct semaphore *);


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/*
 * Simple lock for mutual exclusion.
 *
 * When the lock is created, no thread should be holding it. Likewise,
 * when the lock is destroyed, no thread should be holding it.
 *
 * The name field is for easier debugging. A copy of the name is
 * (should be) made internally.
 */
 ////////////////////////////////////////////////////////////
struct lock {
        char *lk_name;
        HANGMAN_LOCKABLE(lk_hangman);   /* Deadlock detector hook. */

		// begin add stuff //
		struct wchan *lk_wchan;
		struct spinlock lk_lock;
		struct thread *lk_holder;
		// end add stuff //
		
};
////////////////////////////////////////////////////////////
struct lock *lock_create(const char *name);
void lock_destroy(struct lock *);
////////////////////////////////////////////////////////////
/*
 * Operations:
 *    lock_acquire - Get the lock. Only one thread can hold the lock at the
 *                   same time.
 *    lock_release - Free the lock. Only the thread holding the lock may do
 *                   this.
 *    lock_do_i_hold - Return true if the current thread holds the lock;
 *                   false otherwise.
 *
 * These operations must be atomic. You get to write them.
 */
void lock_acquire(struct lock *);
void lock_release(struct lock *);
bool lock_do_i_hold(struct lock *);
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


/*
 * Condition variable.
 *
 * Note that the "variable" is a bit of a misnomer: a CV is normally used
 * to wait until a variable meets a particular condition, but there's no
 * actual variable, as such, in the CV.
 *
 * These CVs are expected to support Mesa semantics, that is, no
 * guarantees are made about scheduling.
 *
 * The name field is for easier debugging. A copy of the name is
 * (should be) made internally.
 */

struct cv {
        char *cv_name;
        // begin add stuff //
		struct wchan *cv_wchan;
		struct spinlock cv_lock;
		// end add stuff //
};

struct cv *cv_create(const char *name);
void cv_destroy(struct cv *);

/*
 * Operations:
 *    cv_wait      - Release the supplied lock, go to sleep, and, after
 *                   waking up again, re-acquire the lock.
 *    cv_signal    - Wake up one thread that's sleeping on this CV.
 *    cv_broadcast - Wake up all threads sleeping on this CV.
 *
 * For all three operations, the current thread must hold the lock passed
 * in. Note that under normal circumstances the same lock should be used
 * on all operations with any particular CV.
 *
 * These operations must be atomic. You get to write them.
 */
void cv_wait(struct cv *cv, struct lock *lock);
void cv_signal(struct cv *cv, struct lock *lock);
void cv_broadcast(struct cv *cv, struct lock *lock);


#endif /* _SYNCH_H_ */

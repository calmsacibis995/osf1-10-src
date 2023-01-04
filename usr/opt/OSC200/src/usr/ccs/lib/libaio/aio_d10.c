/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: aio_d10.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/08/28 14:35:25 $";
#endif

/*
 * change history
 *
 *	7-Feb-1992     Paula Long, Rob Haydt
 *	Fixed queue_io so that requests are queued in the order in which
 *	they are requested (within a priority range).
 *
 *	6-jan-1992     Paula Long
 *	Backed out 2-dec-1991 changes.	The problem has been fixed in the
 *	library.
 *
 *	2-dec-1991	Paula Long, Jeff Denham
 *	Add workarounds for bad interactions between AIO
 *	completion signals and DECthreads. Now, before the pthreads
 *	library initializes, we save user signal handler state and
 *	replace it after the first pthreads call. Also, we have our
 *	own jacket around sigwait() in which we save and replace
 *	handlers in the same way.
 *
 *	6-jun-1991	Rob Haydt
 *	Move an aiocb into the thread structure (AKA: a_close) for close.
 *	Move the file close into aio_close, use the aiocb in the thread
 *	structure, and don't wait for the thread to finish.  In the thread,
 *	don't issue outstanding reads/writes if a close has been issued.
 *	Return ECANCELED instead.
 *
 *	15-may-1991	Rob Haydt
 *	Add comments, set errno properly, and make aio_suspend return
 *	0 instead of an index into the aiocbp list.
 */

/*
 * THIS IS ALL-IMPORTANT: Defining POSIX_4D10 allows source-level
 * compatibility for AIO applications coded to POSIX P1003.4/Draft 10
 * but compiled on a Draft 11 system (as in DEC OSF/1 V2.0).
 */

#define POSIX_4D10

/*
 * Include files
 */
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>

#define AIO_SUSPEND aio_wait	/* redefines a pointer used in aiodef.h */

#include <aio.h>		/* you got the right one baby, UH UGH! */

#undef close

#define AIO_THREAD_EXIT 0
#define AIO_THREAD_ACTIVE 1
#define AIO_THREAD_CLOSE 2

/*
 * SEEK_IO
 *	returns the failure status of the seek or the completion status
 *	of the read/write.  A seek is only performed if necessary.
 *
 *	The macro translates into:
 *	    if seek-not-needed OR (seek-succeeds)
 *	       return read/write-result
 *	    else return -1
 * args:
 *	func - read or write
 *	fd - file descriptor
 *	aiocb - pointer to aio control block
 */
#define SEEK_IO(func, fd, aiocb) \
   ((((aiocb)->aio_whence==SEEK_CUR)&&((aiocb)->aio_offset==0)) \
    || (lseek(fd,(aiocb)->aio_offset,(aiocb)->aio_whence) != -1)) \
    ? func(fd, (void *)(aiocb)->aio_buf, (aiocb)->aio_nbytes) : -1

/*
 * allocate a thread block
 */
#define malloc_thread() \
  ((struct aio_thread *)(malloc(sizeof(struct aio_thread))))

/*
 * if_then_else(exp, true, false)
 *	if the expression is true, produce the true value,
 *	else produce the false value
 */
#define if_then_else(exp, t_exp, f_exp) ((exp)?(t_exp):(f_exp))

/*
 *
 * INSERT_TAIL
 *   push a new entry onto the end of a doubley linked list
 *
 * args:
 *   st_name - structure name
 *		The structure has two fields, last and next, which contain
 *		the links in the chain.
 *   entry - pointer to the new entry to add
 *   head - pointer to the head of the list
 */
#define INSERT_TAIL(st_name, entry, head) \
  {(entry)->st_name.last=(head)->st_name.last;(entry)->st_name.next=head;\
   (head)->st_name.last->st_name.next=(head)->st_name.last=entry;}

/*
 * REMOVE_ENTRY
 *	Remove the specified entry from a list by adjusting the last and
 *	next entries in the neighboring list entries.
 *
 * args:
 *   st_name - structure name (see INSERT_TAIL)
 *   entry - pointer to entry to remove
 */
#define REMOVE_LIST(st_name, entry) \
  {(entry)->st_name.last->st_name.next=(entry)->st_name.next; \
   (entry)->st_name.next->st_name.last=(entry)->st_name.last;}

#define NOT_NULL(a) ((int)(a))
   

/*
 * local definitions
 */

#define LIO_SUSPEND 7		/* used in completion code */
#define LIO_CLOSE 8		/* asynch. close */

/*
 * the synchronization structure is the basic data structure
 * for local mutexes (i.e. locks), semaphores, and flags
 */
struct aio_sync {		/* synchronization structure */
	int status;			/* status */
	int waiters;			/* number of waiters */
	pthread_cond_t cv;		/* condition variable */
};

/*
 * there is one thread for each file descriptor
 */
struct aio_thread {
	int fd;			/* file descriptor */
	int status;			/* thread status */
	pthread_t thread;		/* thread */
	struct aio_sync lock;		/* aio_thread lock */
	struct aio_sync sem;		/* semaphore for outstanding i/o requests */
	struct aiocb	*io_list,	/* outstanding i/o list */
			*active;	/* the current i/o request on the thread */
	struct aiocb a_close;		/* reserved for the close request */
};

/*
 * there is one suspend structure for each entry in a suspend list
 */
struct aio_wait {
	int index;			/* index into the suspend aiocbp list */
	struct aiocb *aiocbp;		/* and a pointer to the appropriate aiocb */
	struct {
		struct aio_wait *next,*last;	/* linked list of waiters */
	} list;
	struct aio_suspend_base *base;/* pointer to the suspend structures */
};

/*
 * there is one suspend_base structure for each suspend call.  The suspend
 * structure contains (a pointer to) the array of suspend structures.
 */
struct aio_suspend_base {
	int return_value;		/* index of the aiocbp which completed the
					 *   suspend request */
	int count;			/* number of aiocb's inlist */
	int not_done;			/* number of incomplete AIOCB's */
	int type;			/* LIO_{WAIT,ASYNC,NOWAIT,SUSPEND} */
	struct aio_wait *list;	/* pointer to suspend list */
	struct aio_sync flag;		/* completion flag */
	struct sigevent sig;		/* ASYNC completion signal */
};

/*
 * Local Data Structures
 */
static struct aio_thread **aio_threads;
static int aio_init_done = 0;
static pthread_mutex_t aio_mutex;	/* used to syncrhronize local
					 * mutexes, semaphores, and flags */
static struct aio_sync aio_control,	/* lock all local data structures */
	aio_completion_mutex;		/* lock completion/suspend processing */

/*
 * synchronization primitives
 *
 * High level mutexes and semaphores are implemented using a
 * pthread-mutex, aio_mutex, and condition variables.
 */
static void
aio_lock_init(struct aio_sync *as)
{
	pthread_cond_init(&as->cv, pthread_condattr_default);
	as->status = as->waiters = 0;
}

static int
aio_lock_delete(struct aio_sync *as)
{
	pthread_cond_destroy(&as->cv);
	as->status = -1;
	return 0;
}

/*
 * High level mutex
 */

static void
aio_mutex_lock(struct aio_sync *as)
{
	pthread_mutex_lock(&aio_mutex);
	as->waiters++;
	while ((as->status != 0) && (pthread_cond_wait(&as->cv,&aio_mutex)==0))
		;
	as->waiters--;
	as->status = 1;
	pthread_mutex_unlock(&aio_mutex);
}

static void
aio_mutex_unlock(struct aio_sync *as)
 {
	register int yield;

	pthread_mutex_lock(&aio_mutex);
	as->status = 0;
	if (yield = as->waiters)
		pthread_cond_signal(&as->cv);
	pthread_mutex_unlock(&aio_mutex);
}

/*
 * Counting semaphores
 *    status represents the number of times the semaphore has been signalled.
 *    waiters represents the number of waiters.
 *
 *    Note: a wait only occurs if status is 0 (the semphore hasn't been
 *	    signalled).
 *	    a cond_signal only occurs if the semaphore is unsignalled (status
 *	    isn't 0) and there is a waiter (waiters > 0).
 */
static int
aio_sem_wait(struct aio_sync *sem)
{
	pthread_mutex_lock(&aio_mutex);
	sem->waiters++;
	while ((sem->status == 0) &&
		(pthread_cond_wait(&sem->cv,&aio_mutex) == 0))
		;
	sem->waiters--;
	sem->status--;
	return(pthread_mutex_unlock(&aio_mutex));
}

static void
aio_sem_signal(struct aio_sync *sem)
{
	register int yield;

	pthread_mutex_lock(&aio_mutex);
	if (yield = ((sem->status++ == 0) && (sem->waiters != 0)))
		pthread_cond_signal(&sem->cv);
	pthread_mutex_unlock(&aio_mutex);
}

/*
 * flags
 *   status is the flag level
 *   waiters is the number of waiters
 *
 *   Note: cond_broadcast is only called if there are any waiters
 */
static void
aio_flag_wait(struct aio_sync *flag)
{
	pthread_mutex_lock(&aio_mutex);
	flag->waiters++;
	while ((flag->status == 0) &&
		(pthread_cond_wait(&flag->cv, &aio_mutex) == 0))
		;
	flag->waiters--;
	pthread_mutex_unlock(&aio_mutex);
}

static void
aio_flag_set(struct aio_sync *flag, int val)
{
	register int yield = 0;

	pthread_mutex_lock(&aio_mutex);
	flag->status = val;
	if (yield = (val && (flag->waiters != 0)))
		pthread_cond_broadcast(&flag->cv);
	pthread_mutex_unlock(&aio_mutex);
}

/*
 * macros for setting and clearing a flag
 */
#define aio_flag_raise(f) aio_flag_set(f, 1)
#define aio_flag_clear(f) aio_flag_set(f, 0)

/*
 * initialization routine
 *	called by pthread_once.
 *
 * allocate the thread array with enough pointers for every possible
 * file descriptor.  Initialize aio_mutex and aio_control then create
 * the completion mutex.
 */
static int aio_init()
{
	int count, i;

	count = sysconf(_SC_OPEN_MAX);
	aio_threads = (struct aio_thread **)
			(calloc((sizeof *aio_threads), count));
	if (aio_threads == NULL)
		return(EAGAIN);
	for (i = 0; i<count; i++)
		aio_threads[i] = (struct aio_thread *)NULL;
	pthread_mutex_init(&aio_mutex, pthread_mutexattr_default);
	pthread_cond_init(&aio_control.cv, pthread_condattr_default);
	aio_control.status = aio_control.waiters = 0;
	aio_lock_init(&aio_completion_mutex);
	aio_init_done = 1;
	return 0;
}

/*
 * aio_complete
 *   Wake up or signal any threads waiting for an i/o control block.
 *
 * Input:
 *	a - pointer to aiocb
 *	io_result - result value (returned by aio_return())
 *	io_errno - errno value (returned by aio_error())
 */
static void
aio_complete(struct aiocb *a, int io_result, int io_errno)
{
	struct aio_wait *s;
	struct sigevent sig;
	int flag;

	a->aio_handle_body.aio_result = io_result;
	a->aio_handle_body.aio_errno = io_errno;

	if ((flag = a->aio_flag&AIO_EVENT) != 0)
		sig = a->aio_event;
	/* get completion mutex */
	aio_mutex_lock(&aio_completion_mutex);
	s = a->aio_suspended;
	/*
	 * Wander through the list for any requests waiting for completion
	 * of this aiocb
	 */
	while (s != NULL) {
		register struct aio_wait *next = s->list.next;

		REMOVE_LIST(list, s);
		s->list.next = s->list.last = s;
		switch(s->base->type) {
			case LIO_SUSPEND:
			/*
			 * suspend completes when any aio request in the
			 * list completes.
			 */
				s->base->return_value = s->index;
				aio_flag_raise(&s->base->flag);
				break;
			case LIO_WAIT:
			/*
			 * wait completes only when all the aio's in the
			 * listio list complete. not_done is the count of how
			 * many haven't completed.
			 */
				if (--s->base->not_done == 0)
					aio_flag_raise(&s->base->flag);
				break;
			case LIO_ASYNC:
			/*
			 * A user specified signal is generated when all the
			 * aio's in listio complete.  not_done is the count
			 * of how many haven't completed.
			 */
				if (--s->base->not_done == 0) {
					kill(getpid(), s->base->sig.sevt_signo);
					free(s->base->list);
					free (s->base);
				}
				break;
			default:
				break;
		}

		if (s == next)
			s = a->aio_suspended = NULL;
		else
			s = next;
	}

	/*
	 * Release the completion mutex.  If the aiocb had an event associated
	 * with it, raise the corresponding signal.
	 */
	aio_mutex_unlock(&aio_completion_mutex);
	if ((flag != 0) && (io_errno != ECANCELED))
		kill(getpid(), sig.sevt_signo);
}

/*
 * aio_thread_main
 *	an aio thread is created for a file descriptor when the first
 *	aio request is made.  the thread is created by get_thread().
 *	The thread raises its priority and goes into a loop waiting
 *	for requests.  When a request is made, the aiocb is queued to
 *	the thread structure and a counting semaphore is incremented.
 *	Incrementing the semaphore unwaits the thread, which remove the
 *	next aiocb from the queue and processes it.  After processing,
 *	the thread calls aio_complete with the aiocb, the error value,
 *	and the read/write value.  The thread then processes the next
 *	request.
 *
 *	When aio_thread_main processes a close request, it causes the
 *	thread to exit the loop.  After exiting, the thread deallocates
 *	the thread resources.
 *
 * Input:
 *	fd - the file descriptor number
 */
static void
aio_thread_main(int fd)
{
	register struct aio_thread *at;
	int io_result, io_errno;

	pthread_setprio(pthread_self(), pthread_getprio(pthread_self())+1);
	at = aio_threads[fd];

	/*
	 * Loop until the thread is ready to exit
	 */
	while (at->status != AIO_THREAD_EXIT) {
		register struct aiocb *a;

	/*
	 * wait for the counting semaphore
	 * and then lock the thread mutex so the thread structure can
	 * be manipulated.	Set the active pointer to the next entry
	 * at the head of the list.
	 */
		aio_sem_wait(&at->sem);
		aio_mutex_lock(&at->lock);
		if (NOT_NULL(a = at->io_list))
			at->io_list = a->aio_next;
		at->active = a;
		aio_mutex_unlock(&at->lock);
		if (a == NULL)	 /* i.e. if any operations have been cancelled */
			continue;
		/*
		 * process the aio control block
		 */
		switch(a->aio_opcode) {
			case LIO_READ:
				if (at->status != AIO_THREAD_CLOSE) {
			/*
			 * as long as the file descriptor hasn't been closed,
			 * try the i/o.
			 */
					io_result = SEEK_IO(read, a->aio_fildes, a);
					io_errno = if_then_else((io_result == -1), errno, 0);
					aio_complete(a, io_result, io_errno);
				} else {
			/*
			 * the file descriptor has been closed; return
			 * cancelled status.
			 */
					io_result = -1;
					io_errno = ECANCELED;
				}
				break;

			case LIO_WRITE:
				if (at->status != AIO_THREAD_CLOSE) {
				/*
				 * as long as the file descriptor hasn't been closed, try the i/o.
				 */
					io_result = SEEK_IO(write, a->aio_fildes, a);
					io_errno = if_then_else((io_result == -1), errno, 0);
					aio_complete(a, io_result, io_errno);
				} else {
				/*
				 * the file descriptor has been closed;
				 * return cancelled status.
				 */
					io_result = -1;
					io_errno = ECANCELED;
				}
				break;

			case LIO_CLOSE:
				at->status = AIO_THREAD_EXIT;	/* make the thread exit */
				io_result = 0;
				io_errno = 0;
				break;

			case LIO_NOP:
			default:
				break;
		}

		/*
		 * Don't take out a lock because this routine is the only
		 * writer and the only reader, acancel, just checks for NULL.
		 */
		at->active = NULL;
	}

	/*
	 * The file descriptor has been closed, free the thread resources.
	 */
	aio_lock_delete(&at->lock);
	aio_lock_delete(&at->sem);
	free(at);
}

/*
 * get_thread
 *	Get the thread data for a file descriptor and lock it.
 *	Create the file descriptor if it doesn't already exist.
 *
 * input:
 *	fd - the file descriptor
 *
 * output:
 *	**t - where to return the pointer to the thread structure
 *
 * NOTE:
 *	this routine locks the thread.	the caller must unlock it.
 */
static int
get_thread(int fd, struct aio_thread **t)
{
	register struct aio_thread *at;

	if (!aio_init_done) {
		static pthread_once_t aio_once = pthread_once_init;
		pthread_once(&aio_once, (void *)aio_init);
	}

	/*
	 * If the thread doesn't exist, create it.
	 */
	aio_mutex_lock(&aio_control);
	if ((at = aio_threads[fd]) == NULL) {

	/*
	 * create a thread data structure, condition variable, etc.
	 * lock the thread data structure, drop the main lock,
	 * create the thread
	 */
		at = aio_threads[fd] = malloc_thread();
		aio_lock_init(&at->lock);
		aio_mutex_lock(&at->lock);
		aio_mutex_unlock(&aio_control);
		aio_lock_init(&at->sem);
		at->status = AIO_THREAD_ACTIVE;
		at->active = at->io_list = (struct aiocb *)NULL;
		if (pthread_create(&at->thread, pthread_attr_default, 
				(void *)aio_thread_main, (void *)fd) != 0) {
			/*
			 * if the thread_create fails, unlock it and delete
			 * the resources.
			 */
			aio_threads[fd] = NULL;
			aio_mutex_unlock(&at->lock);
			aio_lock_delete(&at->lock);
			free(at);
			*t = 0;
			return(EAGAIN);
		}
	} else {
		/*
		 * lock the thread and unlock the control mutex.
		 */
		aio_mutex_lock(&at->lock);
		if (at->status != AIO_THREAD_ACTIVE) {
			aio_mutex_unlock(&at->lock);
			aio_mutex_unlock(&aio_control);
			return(EAGAIN);
		}
		aio_mutex_unlock(&aio_control);
	}
	*t = at;
	return 0;
}

/*
 * queue_io
 *	link the i/o request into the queue and kick the semaphore.
 *	the queue is scanned and the i/o request is positioned in the
 *	list according to io->aio_reqprio.
 *
 * input:
 *	io - pointer to aiocb
 *	t - pointer to locked thread
 *
 * Note:
 *	this routine assume that the caller is holding the thread lock.
 */
static void
queue_io(struct aiocb *io, struct aio_thread *t)
 {
	struct aiocb *l;

	/*
	 * Special case if it's queued to the head of the list
	 */
	if (((l = t->io_list) == NULL) || (io->aio_reqprio > l->aio_reqprio)) {
		io->aio_next = t->io_list;
		t->io_list = io;
	} else {

	/*
	 * It's going to be inserted somewhere in the list.  Scan the list
	 * until the priority of the next request in the list, is lower than
	 * the priority of the new request.
	 */
		while ((l->aio_next != NULL) && 
			(io->aio_reqprio <= l->aio_next->aio_reqprio))
			l = l->aio_next;
		io->aio_next = l->aio_next;
		l->aio_next = io;
	}

	/*
	 * signal the semaphore
	 */
	aio_sem_signal(&t->sem);
}

/*
 * queue_to_thread
 *	Initialize the handle fields, get the thread, queue the io request,
 *	unlock the thread, and return.
 *
 * input:
 *	fd - file descriptor
 *	io_block - pointer to aiocb
 *
 * note:
 *	get_thread locks the thread structure and this routine unlocks it.
 */
static int
queue_to_thread(int fd, struct aiocb *io_block)
{
	int st;
	struct aio_thread *at;

	/*
	 * Lookup the thread and lock it.
	 */
	io_block->aio_handle_body.aio_errno = EINPROG;
	io_block->aio_handle_body.aio_result = -1;
	io_block->aio_suspended = NULL;
	if ((st = get_thread(fd,&at)) != 0)
		return(st);
	queue_io(io_block, at);
	aio_mutex_unlock(&at->lock);
	return 0;
}

/*
 * aio_close
 *	If there isn't an aio thread, just do a normal close.  If there is
 *	an aio thread, create a close request with the lowest priority, and
 *	queue it to the thread.	 Also, mark the thread structure and remove
 *	it from the thread array so no more i/o will be queued to it.
 *
 * input:
 *	fd - file descriptor
 */
aio_close_D10(int fildes)
{
	int ret;
	struct aio_thread *at;

	/*
	 * Do a normal close if no aio has been done on the file descriptor
	 */
	if (!aio_init_done)
		return(close(fildes));
	aio_mutex_lock(&aio_control);
	if ((at = aio_threads[fildes]) == NULL) {
		aio_mutex_unlock(&aio_control);
		return(close(fildes));
	}

	/*
	 * New entries into get_thread are blocked.  Wait for any thread holding the
	 * thread lock to release it. Then:
	 *	o  remove the thread entry from the array so no more entries
	 *	   are queued to it
	 *	o  set the status to THREAD_CLOSE, so the thread will exit
	 *	   after it closes the file
	 *	o  release the control mutex
	 */
	aio_mutex_lock(&at->lock);
	aio_threads[fildes]=NULL;
	at->status = AIO_THREAD_CLOSE;
	aio_mutex_unlock(&aio_control);
	ret = close(fildes);

	/*
	 * set up the "close" aiocb as the lowest priority request, queue
	 * it to the thread, and release the thread lock.
	 */
	at->a_close.aio_opcode = LIO_CLOSE;
	at->a_close.aio_fildes = fildes;
	at->a_close.aio_reqprio = AIO_PRIO_MIN;
	at->a_close.aio_flag = 0;
	at->a_close.aio_handle = &at->a_close.aio_handle_body;
	at->a_close.aio_handle_body.aio_errno = EINPROG;
	at->a_close.aio_handle_body.aio_result = -1;
	at->a_close.aio_suspended = NULL;
	queue_io(&at->a_close, at);
	aio_mutex_unlock(&at->lock);

	/*
	 * Wait for the close to complete.  The thread cleans up after itself.
	 */
	return(ret);
}

/*
 * the aread and awrite functions
 */  
/*
 * aio_read
 *
 * input:
 *	fildes - file descriptor
 *	aiocbp - pointer to aiocb structure
 */
aio_read_D10(int fildes, struct aiocb *aiocbp)
 {
	register int result;

	/* fill io_block for read
	 * queue to thread
	 * return status
	 */
	aiocbp->aio_opcode = LIO_READ;
	aiocbp->aio_fildes = fildes;
	aiocbp->aio_handle = &aiocbp->aio_handle_body;
	result = queue_to_thread(fildes, aiocbp);
	if (result) {
		seterrno(result);
		result = -1;
	}
	return result;
}

/*
 * aio_write
 *
 * input:
 *	fildes - file descriptor
 *	aiocbp - pointer to aiocb structure
 */
aio_write_D10(int fildes, struct aiocb *aiocbp)
{
	register int result;

	/* fill io_block for write
	 * queue to thread
	 * return status
	 */
	aiocbp->aio_opcode = LIO_WRITE;
	aiocbp->aio_fildes = fildes;
	aiocbp->aio_handle = &aiocbp->aio_handle_body;
	result = queue_to_thread(fildes, aiocbp);
	if (result) {
		seterrno(result);
		result = -1;
	}
	return result;
}

/*
 * aio_suspend
 *	Wait for completion of any one of a list of aio requests.
 *	Lock the completion mutex and scan the list to see if any
 *	have already completed (so that there isn't a wait).  If none
 *	have completed, create suspend list of wait stuctures (one
 *	for each aiocb in the list) and link each wait structure to
 *	it's corresponding aiocb.  Create a completion event, unlock
 *	the completion mutex, and wait for the completion event.
 *
 * input:
 *	cnt - number of entries in the list
 *	aiocbp - pointer to the list of aiocbp's
 */
aio_suspend_D10(const int cnt, struct aiocb *aiocbp[])
{
	int i;
	struct aio_suspend_base s_list;
	struct aio_wait wait1;

	aio_mutex_lock(&aio_completion_mutex);
	/*
	 * scan the list and if any have already completed, just return
	 */
	for (i = 0; i < cnt; i++) {
		if (aiocbp[i] &&
			    (aiocbp[i]->aio_handle_body.aio_errno != EINPROG)) {
			aio_mutex_unlock(&aio_completion_mutex);
			return 0;
		}
	}

	/*
	 * optimize for the case of a single aio request in the list
	 */
	if (cnt > 1) {
		s_list.list = (struct aio_wait*)
			(calloc(cnt, sizeof (struct aio_wait)));
		if (s_list.list == NULL) {
			aio_mutex_unlock(&aio_completion_mutex);
			seterrno(EAGAIN);
			return -1;
		}
	} else
		s_list.list = &wait1;

	s_list.type = LIO_SUSPEND;

	/*
	 * initialize each wait structure and link it to the aiocb (remember
	 * there could be more than one wait structure on a single aiocb)
	 */
	for (i = 0; i < cnt; i++) {
		register struct aio_wait *w = &s_list.list[i];

		if ((w->aiocbp = aiocbp[i]) != NULL) {
			if (aiocbp[i]->aio_suspended == NULL)
				aiocbp[i]->aio_suspended = w->list.next =
					w->list.last = w;
			else
				INSERT_TAIL(list, w, aiocbp[i]->aio_suspended)
			w->index = i;
			w->base = &s_list;
		}
	}

	/*
	 * create the completion flag, unlock the global completion mutex, and
	 * wait for the io completion flag.  Then grab the completion mutex
	 * again.
	 */
	aio_lock_init(&s_list.flag);
	aio_mutex_unlock(&aio_completion_mutex);
	aio_flag_wait(&s_list.flag);
	aio_mutex_lock(&aio_completion_mutex);

	/*
	 * Now, remove each wait structure from the list on the aiocb and
	 * finally release the completion mutex.
	 */
	for (i = 0; i<cnt; i++)
		if (aiocbp[i])
			REMOVE_LIST(list, &s_list.list[i])
	aio_mutex_unlock(&aio_completion_mutex);

	/*
	 * if the structure was allocated, free it.
	 */
	if (cnt > 1)
		free(s_list.list);

	return 0;
}

/*
 * lio_listio
 *	Fire off a list of aio requests.  The operation varies according
 *	to the mode parameter.
 *
 * input:
 *	mode - ASYNC, WAIT, NOWAIT
 *	list - array of LIOCB pointers
 *	nent - number of entries in the list
 *	event - event to assert after all i/o's complete if ASYNC is specified
 */
typedef struct liocb *liocbp;

int
lio_listio_D10(int mode, liocbp list[], int nent, struct sigevent *event)
 {
	int l_i;
	struct aio_suspend_base *s_b;

	/* validate mode */
	switch(mode) {
		default:
			seterrno(EINVAL);
			return -1;

		case LIO_NOWAIT:
		case LIO_ASYNC:
		case LIO_WAIT:
			break;
	}

	if (mode != LIO_NOWAIT) {
		/*
		 * for ASYNC and WAIT a suspend list is created with one wait
		 * structure for each LIOCBP
		 */
		if ((s_b = (struct aio_suspend_base *) 
				malloc(sizeof(struct aio_wait))) == 0) {
			seterrno(EAGAIN);
			return -1;
		}

		if ((s_b->list = (struct aio_wait *) 
				calloc(nent, sizeof(struct aio_wait))) == 0) {
			free(s_b);
			seterrno(EAGAIN);
			return -1;
		}

		if (mode == LIO_WAIT)		/* initialize the flag */
			aio_lock_init(&s_b->flag);
		else /*if (mode == LIO_ASYNC)*/
			s_b->sig = *event;			/* copy the event structure */
		s_b->type = mode;
		s_b->count = nent;
		s_b->not_done = nent;
	}

	/*
	 * Process the list by queueing all the i/o requests and initializing
	 * the suspend list.	NOWAIT is checked in several places.
	 */
	for (l_i = 0; l_i < nent; l_i++) {
		struct liocb *l = list[l_i];

		if (l->lio_opcode != LIO_NOP) {
			switch(l->lio_opcode) {
				default:
					if (mode != LIO_NOWAIT) {
						free(s_b->list);
						free(s_b);
					}
					seterrno(EINVAL);
					return -1;
				case LIO_READ:
				case LIO_WRITE:
					break;
			}

			if (mode != LIO_NOWAIT) {
				s_b->list[l_i].aiocbp = &l->lio_aiocb;
				s_b->list[l_i].index = l_i;
				s_b->list[l_i].base = s_b;
			}

			l->lio_aiocb.aio_opcode = l->lio_opcode;
			l->lio_aiocb.aio_fildes = l->lio_fildes;
			l->lio_aiocb.aio_handle = &l->lio_aiocb.aio_handle_body;
			queue_to_thread(l->lio_fildes, &l->lio_aiocb);
		} else if (mode != LIO_NOWAIT) {
				/*
				 * LIO_NOP's don't count
				 */
				s_b->list[l_i].aiocbp = (struct aiocb *)NULL;
				s_b->not_done--;
			}
	}

	if (mode != LIO_NOWAIT) {

	/*
	 * For ASYNCH and WAIT, the wait entry must be linked to the
	 * corresponding aiocb.	 Block completions while linking the
	 * suspend list to the aiocb's. Check for completion first.
	 */
		aio_mutex_lock(&aio_completion_mutex);
		for (l_i = 0; l_i < nent; l_i++) {
			struct aio_wait *l = &s_b->list[l_i];

			if (l->aiocbp == NULL)
				continue;
			if (l->aiocbp->aio_handle_body.aio_errno == EINPROG) {
				if (l->aiocbp->aio_suspended == NULL)
					l->aiocbp->aio_suspended = 
						l->list.next = l->list.last = l;
				else
					INSERT_TAIL(list, l, l->aiocbp->aio_suspended)
			} else
				if (--s_b->not_done == 0)
					break;
		}
		aio_mutex_unlock(&aio_completion_mutex);

		if (mode == LIO_WAIT) {
		/*
		 * wait for completion of all i/o requests and then free the
		 * event and the memory.
		 */
			if (s_b->not_done) /* if nothing was queued, the flag won't be set */
				aio_flag_wait(&s_b->flag);
			aio_lock_delete(&s_b->flag);
			free(s_b->list);
			free(s_b);
		}
	}
	return(0);
}

/*
 * aio_cancel
 *	try to cancel an i/o request.  Lock the thread and scan the
 *	request list.  If the aiocb matches, remove it from the list
 *	and call aio_complete.
 *
 * input:
 *	fildes - file descriptor
 *	aiocbp - pointer to the aiocb to cancel or NULL to cancel all
 */
aio_cancel_D10(int fildes, struct aiocb *aiocbp)
{
	struct aio_thread *at;
	register struct aiocb *a, *prev, *next;
	int ret_val;

	if (get_thread(fildes, &at) != 0)
		return AIO_NOTCANCELED;
	a = at->io_list;
	if (a == NULL) {
		aio_mutex_unlock(&at->lock);
		return AIO_ALLDONE;
	}

	/*
	 * If an aiocb pointer is provided...
	 * Attempt to remove an individual entry from the list.
	 * If we don't find it, attempt to use the aiocb pointer
	 * to check the status of the "in progress" aio. If its
	 * status is not EINPROGRESS, we'll return ALLDONE.
	 */
	if (aiocbp) {
		ret_val = AIO_NOTCANCELED;
		prev = NULL;
		while(a) {
			next = a->aio_next;
			if (a == aiocbp) {
				aio_complete(a, -1, ECANCELED);
				if (prev)
					prev->aio_next = next;
				else
					at->io_list = next;
				ret_val = AIO_CANCELED;
				break;
			} else
				prev = a;	
			a = next;
		}
		/* Unlock mutex before touching suspect aiocbp */
		aio_mutex_unlock(&at->lock);
		if ((ret_val == AIO_NOTCANCELED)
			&& (aiocbp->aio_handle_body.aio_errno != EINPROG))
				ret_val = AIO_ALLDONE;
		return ret_val;
	}

	/*
	 * Else, remove all entries from the list...
	 * We can't account for entries that have already been removed,
	 * so we can't know to return NOTCANCELED for them! The caller
	 * may perceive this as meaning all aio's on the file descriptor
	 * were canceled when they haven't been. Nothing realistic to be
	 * done about this.
	 */
	ret_val = AIO_CANCELED;
	while (a) {
		next = a->aio_next;
		aio_complete(a, -1, ECANCELED);
		a = next;
	}
	if (at->active)
		ret_val = AIO_NOTCANCELED;
	at->io_list = NULL;
	aio_mutex_unlock(&at->lock);
	return ret_val;
}

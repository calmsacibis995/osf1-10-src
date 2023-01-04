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
static char	*sccsid = "@(#)$RCSfile: aio.c,v $ $Revision: 4.2.13.3 $ (DEC) $Date: 1993/06/04 19:58:04 $";
#endif 
/*
 *
 * change history
 *
 *      7-Feb-1992     Paula Long, Rob Haydt
 *      Fixed queue_io so that requests are queued in the order in which
 *      they are requested (within a priority range).
 *
 *      6-jan-1992     Paula Long
 *      Backed out 2-dec-1991 changes.  The problem has been fixed in the
 *      library.  
 *
 *	2-dec-1991	Paula Long, Jeff Denham
 *	Add workarounds for bad interractions between AIO
 *	completion signals and DECthreads. Now, before the pthreads
 *	library initializes, we save user signal handler state and
 *	replace it after the first pthreads call. Also, we have our
 *	own jacket around sigwait() in which we save and replace
 *	handlers in the same way.
 *
 *	6-jun-1991
 *	Move an aiocb into the thread structure (AKA: a_close) for close.
 *	Move the file close into aio_close, use the aiocb in the thread
 *	structure, and don't wait for the thread to finish.  In the thread,
 *	don't issue outstanding reads/writes if a close has been issued.
 *	Return ECANCELED instead.
 *
 *	15-may-1991
 *	Add comments, set errno properly, and make aio_suspend return
 *	0 instead of an index into the aiocbp list.
 */

/*
 * Include files
 */

#include <aio.h>
/*
 * Undo redefine of close() for the user in <aio.h>.
 */
#undef close
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>


#define AIO_THREAD_EXIT 	0
#define AIO_THREAD_ACTIVE 	1
#define AIO_THREAD_CLOSE 	2

#define AIO_IN_DRIVER		((struct aio_thread *)-1)

/*
 * aio_key_to_addr(): validate and translate aio_key and return address
 * If the key is bad, the aiorb array has not mapped, or the key doesn't
 * match the key in the aiorb, return NULL address; otherwise, return
 * address of the appropriate result block.
 */
#define aio_key_to_addr(key)					\
	(((key) < 0) || (AIO_GET_IDX((key)) >= aio_max) ||	\
		(aiorb == NULL) || 				\
		aiorb[AIO_GET_IDX(key)].rb_key != (key)) ?	\
		NULL : &aiorb[AIO_GET_IDX((key))]

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
 *	aiocb - pointer to aio control block
 *	at - point to aio_thread structure
 */
#define SEEK_IO(func, aiocb, at)					\
   ((at)->no_seek ||	(aiocb)->aio_offset == AIO_SEEK_CUR ||		\
    (lseek((aiocb)->aio_fildes, (aiocb)->aio_offset, SEEK_SET) != -1))	\
    ? func((aiocb)->aio_fildes,	(void *)(aiocb)->aio_buf,		\
		(aiocb)->aio_nbytes) : -1

/*
 * allocate a thread block
 */
#define malloc_thread() \
  ((struct aio_thread *)(malloc(sizeof(struct aio_thread))))

/*
 * if_then_else(exp,true,false)
 *	if the expression is true, produce the true value,
 *	else produce the false value
 */
#define if_then_else(exp, t_exp, f_exp) ((exp)?(t_exp):(f_exp))

/*
 * local definitions
 */

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
	int fd;				/* file descriptor */
	int no_seek;			/* do an lseek? */
	int status;			/* thread status */
	pthread_t thread;		/* thread */
	struct aio_sync lock;		/* aio_thread lock */
	struct aio_sync sem;		/* semaphore for outstanding i/o requests */
	struct aiocb *io_list,		/* outstanding i/o list */
		     *active,		/* the current i/o request on the thread */
		     *tail;		/* point to last entry in list */
	struct aiocb a_close;		/* reserved for the close request */
};

/*
 * Local Data Structures
 */
static aio_result_t aiorb = NULL;	/* array of result blocks */
static int aio_max = 0;			/* # elements in array */
static struct aio_thread **aio_threads;
static int aio_init_done = 0;
static pthread_mutex_t aio_mutex;	/* used to syncrhronize local
					 * mutexes, semaphores, and flags */
static struct aio_sync aio_control;	/* lock all local data structures */

#ifdef AIO_THREADS_SWITCH
static int threads_only = 0;
#endif


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
	while ((as->status != 0) &&
		(pthread_cond_wait(&as->cv,&aio_mutex) == 0))
		; /* NULL statement */
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
		; /* NULL statement */
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
 * initialization routine
 *	called once by aio_read(), aio_write(), aio_cancel()
 *
 * call aio_init() to map in the global result block array.
 * allocate the thread array with enough pointers for every possible
 * file descriptor.  Initialize aio_mutex and aio_control.
 */
static int libaio_init()
{
	int count, i;
#ifdef AIO_THREADS_SWITCH
	char *threads_only_env;
#endif

	i = aio_init(&aiorb, &aio_max);
	if (i)
		return(EAGAIN);
	count = sysconf(_SC_OPEN_MAX);
	aio_threads = (struct aio_thread **)(calloc((sizeof(*aio_threads)), count));
	if (aio_threads == NULL)
		return(EAGAIN);
	for (i = 0; i < count; i++)
		aio_threads[i] = (struct aio_thread *)0;
	pthread_mutex_init(&aio_mutex, pthread_mutexattr_default);
	pthread_cond_init(&aio_control.cv, pthread_condattr_default);
	aio_control.status = aio_control.waiters = 0;
	aio_init_done = 1;
#ifdef AIO_THREADS_SWITCH
	threads_only_env = getenv("AIO_THREADS_ONLY");
	threads_only = threads_only_env != NULL;
#endif
	return 0;
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
 *	the thread calls aio_transfer_done with the aiocb, the error value,
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
		 * be manipulated.  Set the active pointer to the next entry
		 * at the head of the list.
		 */
		aio_sem_wait(&at->sem);
		aio_mutex_lock(&at->lock);
		if ((a = at->io_list) != NULL)
			at->io_list = a->aio_next;
		if (at->io_list == NULL)
			at->tail = NULL;
		at->active = a;
		aio_mutex_unlock(&at->lock);
		if (a == NULL)	/* i.e. if any operations have been cancelled */
			continue;

		/*
		 * process the aio control block
		 */
		switch (a->aio_lio_opcode) {
		case LIO_READ:
			if (at->status != AIO_THREAD_CLOSE) {
				/*
				 * as long as the file descriptor hasn't
				 * been closed, try the i/o.
				 */
				io_result = SEEK_IO(read, a, at);
				io_errno = if_then_else((io_result == -1),
						errno, 0);
			} else {
				/*
				 * the file descriptor has been closed;
				 * return cancelled status.
				 */
				io_result = -1;
				io_errno = ECANCELED;
			}
			aio_transfer_done(a->aio_key, io_result, io_errno);
			break;

		case LIO_WRITE:
			if (at->status != AIO_THREAD_CLOSE) {
				/*
				 * as long as the file descriptor hasn't been
				 * closed, try the i/o.
				 */
				io_result = SEEK_IO(write, a, at);
				io_errno = if_then_else((io_result == -1),
						errno, 0);
			} else {
				/*
				 * the file descriptor has been closed;
				 * return cancelled status.
				 */
				io_result = -1;
				io_errno = ECANCELED;
			}
			aio_transfer_done(a->aio_key, io_result, io_errno);
			break;

		case LIO_CLOSE:
			at->status = AIO_THREAD_EXIT; /* make the thread exit*/
			io_result = 0;
			io_errno = 0;
			break;

		case LIO_NOP:
		default:
			break;
		}

		/*
		 * Don't take out a lock because this routine is the only
		 * writer and  the only reader, acancel, just checks for NULL.
		 */
		at->active = NULL;
	} /* while */

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
	register int flags;

	aio_mutex_lock(&aio_control);

	at = aio_threads[fd];

	if (at == AIO_IN_DRIVER) {
		/*
		 * This is an error: a previous aio in the kernel
		 * succeeded, but now we're here because one failed.
		 * The only thing to do is return EAGAIN here.
		 */
		aio_mutex_unlock(&aio_control);
		*t = at;
		return EAGAIN;
	} else if (at == NULL) {
		/*
		 * The thread doesn't exist, create it.
		 * Create a thread data structure, condition variable...
		 * Lock the thread data structure, drop the main lock,
		 * create the thread.
		 */
		at = aio_threads[fd] = malloc_thread();
		aio_lock_init(&at->lock);
		aio_mutex_lock(&at->lock);
		aio_mutex_unlock(&aio_control);
		aio_lock_init(&at->sem);
		at->status = AIO_THREAD_ACTIVE;
		at->active = at->io_list = at->tail = (struct aiocb *)NULL;
		flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1) {
			aio_threads[fd] = NULL;
			aio_mutex_unlock(&at->lock);
			aio_lock_delete(&at->lock);
			free(at);
			*t = 0;
			return(EBADF);
		}
		/*
		 * If the file is open for append or it is open
		 * on a non-seeking device, set the no_seek flag
		 * so that the SEEK_IO() macro skips the seek.
		 */
		at->no_seek = ((flags & O_APPEND) == O_APPEND)
			|| (lseek(fd, 0, SEEK_CUR) == -1);
		if ((pthread_create(&at->thread, pthread_attr_default,
			(void *)aio_thread_main, (void *)fd)) != 0) {
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
		 * The thread exists, so post the aio.
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
 *	Link the i/o request into the queue and kick the semaphore.
 *	Since we're not doing D11 prioritized I/O, put it at the tail
 *	of the queue.
 *
 * input:
 *	io - pointer to aiocb
 *	t - pointer to locked thread
 *
 * Note:
 *	this routine assume that the caller is holding the thread lock.
 */
static void
queue_io(struct aiocb *io, struct aio_thread *at)
{
	struct aiocb *l;

	/*
	 * Special case if it's queued to the head of the list
	 */
	if ((l = at->io_list) == NULL) {
		io->aio_next = at->io_list;
		at->io_list = io;
		at->tail = io;
	} else {
		/*
		 * It's going to be inserted at the end of the list.
		 */
		io->aio_next = at->tail->aio_next;
		at->tail->aio_next = io;
		at->tail = io;
	}

	/*
	 * signal the semaphore
	 */
	aio_sem_signal(&at->sem);
}

/*
 * queue_to_thread
 *	Initialize the handle fields, get the thread, queue the io request,
 *	unlock the thread, and return.
 *
 * input:
 *	io_block - pointer to aiocb
 *
 * note:
 *	get_thread locks the thread structure and this routine unlocks it.
 */
static int
queue_to_thread(aiocb_t *io_block)
{
	int st;
	struct aio_thread *at;

	/*
	 * Lookup the thread and lock it.
	 */
	if ((st = get_thread(io_block->aio_fildes, &at)) != 0)
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
int
aio_close(int fildes)
{
	int ret;
	struct aio_thread *at;

	/*
	 * Do a normal close if no aio has been done on the file descriptor
	 */
	if (!aio_init_done)
		return(close(fildes));

	if (aio_threads[fildes] == AIO_IN_DRIVER) {
		aio_threads[fildes] = NULL;
		return(close(fildes));
	}		

	aio_mutex_lock(&aio_control);
	if ((at = aio_threads[fildes]) == NULL) {
		aio_mutex_unlock(&aio_control);
		return(close(fildes));
	}
	/*
	 * New entries into get_thread are blocked.  Wait for any thread
	 * holding the thread lock to release it. Then:
	 *	o remove the thread entry from the array so no more entries
	 *	  are queued to it
	 *	o set the status to THREAD_CLOSE, so the thread will exit
	 *	  after it closes the file
	 *	o release the control mutex
	 */
	aio_mutex_lock(&at->lock);
	aio_threads[fildes] = NULL;
	at->status = AIO_THREAD_CLOSE;
	aio_mutex_unlock(&aio_control);
	ret = close(fildes);

	/*
	 * set up the "close" aiocb, queue it to the thread,
	 * and release the thread lock.
	 */
	at->a_close.aio_lio_opcode = LIO_CLOSE;
	at->a_close.aio_fildes = fildes;
	queue_io(&at->a_close, at);
	aio_mutex_unlock(&at->lock);

	/*
	 * Don't wait for the close to complete.  The thread cleans
	 * up after itself.
	 */
	return(ret);
}

/*
 * aio_error()
 *	return the current errno/status from the correct result block
 */
int
aio_error(aiocb_t *aiocbp)
{
	register aio_result_t rb;

	rb = aio_key_to_addr(aiocbp->aio_key);
	if (rb)
		return(rb->rb_errno);
	else {
		seterrno(EINVAL);
		return -1;
	}
}
/*
 * aio_return()
 *	return the status and allow system to reclaim resources.
 */
int
aio_return(aiocb_t *aiocbp)
{
	return(aio_done(aiocbp->aio_key));
}

/*
 * aio_read
 *	pass request to kernel for driver AIO
 *	if that fails, queue the read to a thread
 * input:
 *	aiocbp - pointer to aiocb structure
 */
int
aio_read(aiocb_t *aiocbp)
{
	register int result, in_driver = 0, sig;
	register struct aio_thread *at;
	register union sigval sigval;
	int tty;

	if (!aio_init_done) {
		result = libaio_init();
		if (result) {
			seterrno(result);
			return -1;
		}
	}

	at = aio_threads[aiocbp->aio_fildes];

	/*
	 * If no thread exists for this file descriptor, or if we
	 * already know that driver is handling the aio, set the flag
	 * to tell aio_transfer() to pass the operation to the driver.
	 * Because we don't kernel aio going to tty's, we do a handrolled
	 * call to isatty() to prevent the kernel from ever attempting
	 * aio to a tty.
	 */
#ifdef AIO_THREADS_SWITCH
	if (threads_only)
		in_driver = 0;
	else
#endif
	if (at == AIO_IN_DRIVER)
		in_driver = 1;
	else
		if (at == NULL)
			in_driver = ioctl(aiocbp->aio_fildes, TIOCGETD, &tty)
					== -1;

	aiocbp->aio_lio_opcode = LIO_READ;
	sig = aiocbp->aio_sigevent.sigev_signo;
	sigval = aiocbp->aio_sigevent.sigev_value;

	switch (aio_transfer(aiocbp, in_driver, aiocbp->aio_fildes,
			sig, sigval)) {
		case 0:
			result = queue_to_thread(aiocbp);
			if (result) {
				/*
				 * If the attempt to queue fails, we need
				 * to say this transfer is done and give
				 * back the result block the call to
				 * aio_transfer() allocated.
				 */
				aio_transfer_done(aiocbp->aio_key, -1, EIO);
				aio_done(aiocbp->aio_key);
				seterrno(result);
				result = -1;
			}
			return result;
		case EAIO:
			aio_threads[aiocbp->aio_fildes] = AIO_IN_DRIVER;
			return 0;
		default:
			return -1;
	}
/* NOTREACHED */
}

/*
 * aio_write
 *	pass request to kernel for driver AIO
 *	if that fails because the driver doesn't suppport AIO, queue
 *	the write to a thread
 * input:
 *	aiocbp - pointer to aiocb structure
 */
int
aio_write(aiocb_t *aiocbp)
{
	register int result, in_driver = 0, sig;
	register struct aio_thread *at;
	register union sigval sigval;
	int tty;

	if (!aio_init_done) {
		result = libaio_init();
		if (result) {
			seterrno(result);
			return -1;
		}
	}

	at = aio_threads[aiocbp->aio_fildes];

	/*
	 * If no thread exists for this file descriptor, or if we
	 * already know that driver is handling the aio, set the flag
	 * to tell aio_transfer() to pass the operation to the driver.
	 * Because we don't kernel aio going to tty's, we do a handrolled
	 * call to isatty() to prevent the kernel from ever attempting
	 * aio to a tty.
	 */
#ifdef AIO_THREADS_SWITCH
	if (threads_only)
		in_driver = 0;
	else
#endif
	if (at == AIO_IN_DRIVER)
		in_driver = 1;
	else
		if (at == NULL)
			in_driver = ioctl(aiocbp->aio_fildes, TIOCGETD, &tty)
					== -1;

	aiocbp->aio_lio_opcode = LIO_WRITE;
	sig = aiocbp->aio_sigevent.sigev_signo;
	sigval = aiocbp->aio_sigevent.sigev_value;

	switch (aio_transfer(aiocbp, in_driver, aiocbp->aio_fildes,
			sig, sigval)) {
		case 0:
			result = queue_to_thread(aiocbp);
			if (result) {
				/*
				 * If the attempt to queue fails, we need
				 * to say this transfer is done and give
				 * back the result block the call to
				 * aio_transfer() allocated.
				 */
				aio_transfer_done(aiocbp->aio_key, -1, EIO);
				aio_done(aiocbp->aio_key);
				seterrno(result);
				result = -1;
			}
			return result;
		case EAIO:
			aio_threads[aiocbp->aio_fildes] = AIO_IN_DRIVER;
			return 0;
		default:
			return -1;
	}
/* NOTREACHED */
}

/*
 * aio_suspend
 *	Wait for completion of any one of a list of aio requests.
 *
 * input:
 *	cnt - number of entries in the list
 *	aiocbp - pointer to the list of aiocbp's
 */
int
aio_suspend(int cnt, const aiocb_t *aiocbp[])
{
	register int i, result, count;
	aio_key_t keys[AIO_LISTIO_MAX];
	register aio_key_t *kp;
	register aiocb_t *a;

	if ((cnt > AIO_LISTIO_MAX) || (cnt <= 0)) {
		seterrno(EINVAL);
		return -1;
	}

	for (kp = keys, i = 0, count = 0; i < cnt; i++)
		if (aiocbp[i]) {
			*kp++ = aiocbp[i]->aio_key;
			count++;
		}

	if (count)
		return(aio_wait(count, keys, LIO_SUSPEND, 0, 0L));
	else
		return 0;
}

/*
 * lio_listio
 *	Fire off a list of aio requests.  The operation varies according
 *	to the mode parameter.
 *
 * input:
 *	mode - LIO_WAIT, LIO_NOWAIT
 *	list - array of AIOCB pointers
 *	nent - number of entries in the list
 *	sigevent - signal to assert after all i/o's complete for LIO_NOWAIT
 */

int
lio_listio(int mode, aiocb_t *list[], int nent, struct sigevent *sigevent)
{
	register int i, j, result;
	aio_key_t 	keys[AIO_LISTIO_MAX];
	register 	aio_key_t *kp;
	register 	aiocb_t *a;
	register int 	sig;
	sigval_t 	sigval;

	if (nent > AIO_LISTIO_MAX) {
		seterrno(EINVAL);
		return -1;
	}

	/* validate mode */
	switch (mode) {
		default:
			seterrno(EINVAL);
			return -1;
		case LIO_NOWAIT:
		case LIO_WAIT:
			break;
	}

	for (kp = keys, i = 0, j = 0; i < nent; i++) {
		a = list[i];
		switch (a->aio_lio_opcode) {
			case LIO_READ:
				result = aio_read(a);
				*kp++ = a->aio_key;
				j++;
				break;
			case LIO_WRITE:
				result = aio_write(a);
				*kp++ = a->aio_key;
				j++;
				break;
			case LIO_NOP:
				continue;
			default:
				seterrno(EINVAL);
				return(-1);
		}
		if (result) {
			if (errno == EAGAIN)
				return -1;
			else {
				seterrno(EIO);
				return -1;
			}
		}
	}

	if (sigevent) {
		sig = sigevent->sigev_signo;
		sigval = sigevent->sigev_value;
	} else {
		sig = 0;
		sigval.sival_ptr = NULL;
	}

	if (j)
		return(aio_wait(j, keys, mode, sig, sigval));
	else
		return 0;
}

/*
 * aio_cancel
 *	try to cancel an i/o request. If i/o on the file descriptor
 *	is handled in the kernel, scan the result blocks. Otherwise,
 *	scan the list of queued aiocbs being processed by a pthread.
 *
 * input:
 *	fildes - file descriptor
 *	aiocbp - pointer to the aiocb to cancel or NULL to cancel all
 */
int
aio_cancel(int fildes, struct aiocb *aiocbp)
{
	struct aio_thread *at;
	register struct aiocb *a, *prev, *next;
	int ret_val;

	if (!aio_init_done) {
		libaio_init();
		seterrno(EINVAL);
		return (-1);
	}

	/*
	 * Check whether I/O on this fildes is in the driver.
	 * If so, determine whether we need to return ALLDONE.
	 * If not, return NOTCANCELED.
	 */
	if (aio_threads[fildes] == AIO_IN_DRIVER) {
		if (aiocbp != NULL) {
			/*
			 * We have an aiocb pointer. We could die
			 * on this, but try to get its status.
			 */
			if (aio_error(aiocbp) != EINPROGRESS)
				return AIO_ALLDONE;
			else
				return AIO_NOTCANCELED;
		} else {
			/*
			 * Only the fildes was specified, so we
			 * need to scan the aio database, look
			 * for aio's that we own, and determine
			 * their status. Note that we can't cancel
			 * them; rather, we can only determine their
			 * state at the time of the cancelation request.
			 */
			register int alldone = 1, error;
			register aio_result_t rbp;
			register pid_t pid = getpid();
			for (rbp = aiorb; rbp < (aiorb + aio_max); rbp++) {
				if ((rbp->rb_pid == pid) && (rbp->rb_fd == fildes)
					&& (rbp->rb_driver) &&
					((error = rbp->rb_errno) != EINVAL))
					if (error == EINPROGRESS) {
						alldone = 0;
						break;
					}
			}
			if (alldone)
				return AIO_ALLDONE;
			else
				return AIO_NOTCANCELED;
		}
	}

	/*
	 * The fildes is handled in the library. The removal of
	 * one aiocb is handled separately from the removal of
	 * all to simplify the determination of the correct
	 * return status.
	 */
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
				aio_transfer_done(a->aio_key, -1, ECANCELED);
				if (prev)
					prev->aio_next = next;
				else
					at->io_list = next;
				ret_val = AIO_CANCELED;
				if (next == NULL)
					at->tail = prev;
				break;
			} else
				prev = a;	
			a = next;
		}
		/* Unlock mutex before touching suspect aiocbp */
		aio_mutex_unlock(&at->lock);
		if ((ret_val == AIO_NOTCANCELED)
			&& (aio_error(aiocbp) != EINPROGRESS))
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
		aio_transfer_done(a->aio_key, -1, ECANCELED);
		a = next;
	}
	if (at->active)
		ret_val = AIO_NOTCANCELED;
	at->io_list = at->tail = NULL;
	aio_mutex_unlock(&at->lock);
	return ret_val;
}

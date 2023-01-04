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
/*
 * @(#)$RCSfile: sysaio.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/07/08 10:39:49 $
 */

#ifndef _SYS_SYSAIO_H_
#define _SYS_SYSAIO_H_

#include <standards.h>

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/limits.h>
#include <sys/buf.h>
#ifdef _KERNEL
#include <kern/thread.h>
#include <kern/lock.h>
#include <sys/ioctl.h>
#endif

/*
 * define the AIO/LIO constants
 */

#define AIO_CANCELED 	0	/* all operations cancelled */
#define AIO_NOTCANCELED 1	/* not all operations cancelled */
#define AIO_ALLDONE 	2	/* all operations were completed */

#define LIO_WAIT 	1	/* wait for all operations to complete */
#define LIO_SUSPEND 	2	/* wait for any to complete, no signal */
#define LIO_NOWAIT 	3	/* don't wait for completion, signal */

#define LIO_READ 	4
#define LIO_WRITE 	5
#define LIO_NOP 	0

#define	AIO_SEEK_CUR	(-1)

/*
 * Macros to pack and unpack aio key values
 */
#define AIO_SET_IDX(key, idx) (key) = \
	((int)((unsigned)(idx) & 0xFFFF0000) | (short)(idx))
#define AIO_SET_SEQ(key, seq) (key) = \
	((int)((unsigned)(key) & 0x0000FFFF) | ((short)(seq) << 16))
#define AIO_GET_IDX(key) 	((int)(0x0000FFFF & (unsigned)(key)))
#define AIO_GET_SEQ(key)	((int)((0xFFFF0000 & (unsigned)(key)) >> 16))

/*
 * aio key type
 */
typedef unsigned int aio_key_t;	/* key/id for an aio */

/*
 * aiocb: represents one AIO request at the user level
 */
struct aiocb {
	int 		aio_fildes;
	off_t 		aio_offset;
	volatile void 	*aio_buf;
	size_t 		aio_nbytes;
	int  		aio_reqprio;
	struct sigevent aio_sigevent;
	int 		aio_lio_opcode;
  	/* implementation extensions */
  	struct aiocb 	*aio_next;
	aio_key_t	aio_key;	
	int		aio_reserved[8];
};

typedef struct aiocb aiocb_t;

/*
 * aio_result_block:
 *	the list of result blocks is double mapped between user and
 *	kernel space. There is one result block for each outstanding
 *	aio request. The block is allocated by aio_read/write and
 *	released by aio_return.
 */
struct aio_result_block {
	int			rb_idx;
	volatile aio_key_t	rb_key;
	int			rb_fd;
	pid_t			rb_pid;
	volatile int		rb_errno;
	volatile size_t		rb_result;
	sigevent_t 		rb_sigevent;
	int			rb_driver;
#ifdef _KERNEL
        struct proc		*rb_proc;
	struct aio_test		*rb_test_list;
	struct aio_buf		*rb_buf;
#else
        void			*rb_proc;
	void			*rb_test_list;
	void			*rb_buf;
#endif
};

typedef struct aio_result_block *aio_result_t;

#ifdef _KERNEL
/*
 * Internal work routines.
 */
extern void aio_next_done();
extern void aio_test_done();
extern void aio_remove_tests();
extern void aio_unwait_signal();
extern void aio_sysinit();
extern aio_result_t aio_alloc();
extern int aio_dealloc();
extern int aio_mapfunc();
extern int aio_complete();
extern int aio_ioctl();

/*
 * aio_buf: represents a user buffer passed to driver aio
 */
struct aio_buf {
	struct buf 	ab_buf;
	long 		ab_requested;
	aio_key_t  	ab_key;
};

typedef struct aio_buf aio_buf_t;

/*
 * aio_test
 *	There is one test block for each aiocb in aio_suspend and lio_listio
 *	(aio_wait). Each test block is backlinked to its corresponding
 *	result_block.  The test block is dequeued when the test completes.
 */
struct aio_test {
	aio_result_t	at_rb;		/* pointer to result block */
  	struct aio_test	*at_next,	/* queue links to result block */
			*at_last;
	struct aio_test_hdr *at_thdr; 	/* test header */
};

typedef struct aio_test aio_test_t;


/*
 * aio_test_hdr
 *	there is one test header allocated for each call to aio_suspend
 *	or lio_listio.  the test header contains the test blocks, the 
 *	sigevent structure, a count of the tests needed for completion,
 *	and a count of the tests in the list.  the block is deallocated
 *	when the test has completed.
 */
struct aio_test_hdr {
	int thdr_flags;			/* ALL/ANY, SIGNAL, EVENT */
	int thdr_notdone;		/* count of tests to complete */
  	int thdr_count;			/* count of tests */
	thread_t thdr_thread;		/* thread to awaken */
	struct proc *thdr_proc;		/* proc to signal */
	sigevent_t thdr_sigevent;	/* completion signal */
  	struct aio_test thdr_list[AIO_LISTIO_MAX]; /* list of tests */
};

typedef struct aio_test_hdr aio_thdr_t;

/*
 * AIO ioctl commands.
 */
#define AIOCTL_CMDRW 	1
#define AIOCTL_CMDCAN	2
/*
 * Read/Write command. aiocb is read only.
 */
#define AIOCTLRW	_IOW('A', AIOCTL_CMDRW, struct aiocb)
/*
 * Cancel command. aiocb is read only.
 */
#define AIOCTLCAN	_IOW('A', AIOCTL_CMDCAN, struct aiocb)

#endif /* _KERNEL */

#endif /* _SYS_SYSAIO_H_ */

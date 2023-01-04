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
 * @(#)$RCSfile: siginfo.h,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1994/01/06 14:24:08 $"
 */

#ifndef _SYS_SIGINFO_H
#define _SYS_SIGINFO_H

#ifndef ASSEMBLER
#if	defined(_KERNEL)
#include <kern/queue.h>
#endif	/* _KERNEL */


/*
 * Max siginfo size -- 128 bytes altogether (used to pad siginfo _sifields)
 */
#define SI_MAX_SIZE	(128)
#define SI_PAD_SIZE	(SI_MAX_SIZE - (sizeof(k_siginfo_t) - (3*sizeof(int))))

/*
 * Define kernel version of siginfo with no padding for growth.
 * Copy into user siginfo structure when copying into user space.
 * (This is defined before siginfo_t so sizeof(k_siginfo_t) will work
 * in siginfo_t below.)
 */
struct k_siginfo {
	int si_signo;	/* signal number */
	int si_errno;	/* error number */
	int si_code;	/* signal code */

	union {
		/* kill() or SIGCHLD */
		struct {
			pid_t	_pid;			/* child's pid */
			union {
				/* kill() */
				struct {
					uid_t	_uid;	/* sender's uid */
				} _kill;

				/* SIGCHLD */
				struct {
					int	_status; /* exitcode/signal */
					clock_t _utime;
					clock_t _stime;
				} _sigchld;
			} _pinfo;
		} _sigproc;

		/* SIGILL, SIGFPE, SIGSEGV, SIGBUS */
		struct {
			caddr_t	_addr;	/* faulting instruction/memory ref. */
		} _sigfault;


		/* SIGPOLL */
		struct {
			long	_band;	/* POLL_IN, POLL_OUT, POLL_MSG */
			/* fd not currently available for SIGPOLL */
			int	_fd;	/* file descriptor */
		} _sigpoll;
	} _sifields;
};

typedef struct k_siginfo k_siginfo_t;

struct siginfo {
	int si_signo;	/* signal number */
	int si_errno;	/* error number */
	int si_code;	/* signal code */

	union {
		char _sipad[SI_PAD_SIZE]; /* reserve space for new fields */

		/* kill() or SIGCHLD */
		struct {
			pid_t	_pid;			/* sender's pid */
			union {
				/* kill() */
				struct {
					uid_t	_uid;	/* sender's uid */
				} _kill;

				/* SIGCHLD */
				struct {
					int	_status; /* exitcode/signal */
					clock_t _utime;
					clock_t _stime;
				} _sigchld;
			} _pinfo;
		} _sigproc;

		/* SIGILL, SIGFPE, SIGSEGV, SIGBUS */
		struct {
			caddr_t	_addr;	/* faulting instruction/memory ref. */
		} _sigfault;


		/* SIGPOLL */
		struct {
			long	_band;	/* POLL_IN, POLL_OUT, POLL_MSG */
			/* fd not currently available for SIGPOLL */
			int	_fd;	/* file descriptor */
		} _sigpoll;
	} _sifields;
};

typedef struct siginfo siginfo_t;


/*
 * This is how users expect to access these fields.
 */
#define si_pid		_sifields._sigproc._pid
#define si_uid		_sifields._sigproc._pinfo._kill._uid
#define si_status	_sifields._sigproc._pinfo._sigchld._status
#define si_utime	_sifields._sigproc._pinfo._sigchld._utime
#define si_stime	_sifields._sigproc._pinfo._sigchld._stime
#define si_addr		_sifields._sigfault._addr
#define si_band		_sifields._sigpoll._band
#define si_fd		_sifields._sigpoll._fd

/**************************************************************************
			si_code values
**************************************************************************/

/* negative si_codes are reserved for user defined signals */
#define SI_USER		0
#define SI_FROMUSER(siptr)	((siptr)->si_code <= 0)
#define SI_FROMKERNEL(siptr)	((siptr)->si_code > 0)


/*
 * SIGILL si_codes 
 */
#define ILL_ILLOPC	1	/* illegal opcode */
#define ILL_ILLOPN	2	/* illegal operand */
#define ILL_ILLADR	3	/* illegal addressing mode */
#define ILL_ILLTRP	4	/* illegal trap */
#define ILL_PRVOPC	5	/* privileged opcode */
#define ILL_PRVREG	6	/* privileged register */
#define ILL_COPROC	7	/* coprocessor error */
#define ILL_BADSTK	8	/* internal stack error */
#define NSIGILL		8

/*
 * SIGFPE si_codes
 */
#define FPE_INTDIV	1	/* integer divide by zero */
#define FPE_INTOVF	2	/* integer overflow */
#define FPE_FLTDIV	3	/* floating point divide by zero */
#define FPE_FLTOVF	4	/* floating point overflow */
#define FPE_FLTUND	5	/* floating point underflow */
#define FPE_FLTRES	6	/* floating point inexact result */
#define FPE_FLTINV	7	/* invalid floating point operation */
#define FPE_FLTSUB	8	/* subscript out of range */
#define FPE_FLTCPL	9	/* complete */
#define NSIGFPE		9

/*
 * SIGSEGV si_codes
 */
#define SEGV_MAPERR	1	/* address not mapped to object */
#define SEGV_ACCERR	2	/* invalid permissions for mapped object */
#define NSIGSEGV	2

/* 
 * SIGBUS si_codes
 */
#define BUS_ADRALN	1	/* invalid address alignment */
#define BUS_ADRERR	2	/* non-existent physical address */
#define BUS_OBJERR	3	/* object specific hardware error */
#define NSIGBUS		3

/*
 * SIGTRAP si_codes
 */
#define TRAP_BRKPT	1	/* process breakpoint */
#define TRAP_TRACE	2	/* process trace trap */
#define NSIGTRAP	2

/*
 * SIGCHLD si_codes
 */
#define CLD_EXITED	1	/* child has exited */
#define CLD_KILLED	2	/* child was killed */
#define CLD_DUMPED	3	/* child terminated abnormally */
#define CLD_TRAPPED	4	/* traced child has trapped */
#define CLD_STOPPED	5	/* child has stopped */
#define CLD_CONTINUED	6	/* stopped child has continued */
#define NSIGCLD		6

/*
 * SIGPOLL si_codes
 */
#define POLL_IN		1	/* data input available */
#define POLL_OUT	2	/* output buffers available */
#define POLL_MSG	3	/* input message available */
#define POLL_ERR	4	/* I/O error */
#define POLL_PRI	5	/* high priority input available */
#define POLL_HUP	6	/* device disconnected */
#define NSIGPOLL	6

/**************************************************************************
			end si_code values
**************************************************************************/


#ifdef	_KERNEL

/*
 * Structure for queuing siginfo data.
 */

struct sigqueue_struct {
	k_siginfo_t		siginfo;
	queue_chain_t		sigqueue_list;
};

typedef struct sigqueue_struct *sigqueue_t;

/*
 * kernel macros and routines
 */
#define sigq_find_sig(q, sig) \
	sigq_find_next_sig(q, (sigqueue_t) queue_first(q), sig)
#define sigq_dequeue(q,qp) \
	queue_remove(q, qp, sigqueue_t, sigqueue_list)
#define sigq_enqueue_head(q,qp) \
	queue_enter_first( q, qp, sigqueue_t, sigqueue_list)
#define sigq_enqueue_tail(q,qp) \
	queue_enter(q, qp, sigqueue_t, sigqueue_list)

#define ALL_SIGQ_SIGS	-1

extern struct zone *sigqueue_zone;
extern siginfo_t zero_siginfo;
extern k_siginfo_t zero_ksiginfo;

/*
 * requeue sigqueue and signal if another hasn't been queued
 * since.
 */
#define SIGQ_REQUEUE(xq, xqp)						\
MACRO_BEGIN								\
	if (xqp) {							\
		if (sigq_find_sig(xq, xqp->siginfo.si_signo)) {		\
			zfree(sigqueue_zone, (vm_offset_t) xqp);	\
		} else {						\
			sigq_enqueue_head(xq, xqp);			\
		}							\
		xqp = 0;						\
	}								\
MACRO_END

extern int atintr_level;
/* Keep track of attempts to allocate from interrupt level */
extern int sigalloc_atintr;

#define SIGQ_ALLOC()	(sigqueue_t)					\
	(atintr_level ? sigalloc_atintr++, NULL : zalloc(sigqueue_zone))

#define SIGQ_FREE(xqp)							\
MACRO_BEGIN								\
	if(xqp) {							\
		zfree(sigqueue_zone, (vm_offset_t) xqp);		\
		xqp = 0;						\
	}								\
MACRO_END

/*
 * Look for another siginfo for this type signal...
 * reset bit in pending mask if found.
 *
 * This should never happen, because not
 * supporting multiple signal instances
 * right now, so a second sigqueue is never
 * queued if pending bit is set.
 *
 * if (xqp && sigq_find_next_sig(xq, xtmpq, xsig)) {
 *	if (threadmask & sigmask(xsig))
 *		p->p_sig |= mask;
 *	else
 *		u.u_sig |= mask;
 * }
 */
#define SIGQ_DEQUEUE( xq, xsig, xqp )					\
MACRO_BEGIN								\
	xqp = sigq_find_sig(xq, xsig);					\
	if (xqp)							\
		sigq_dequeue(xq, xqp);					\
MACRO_END

_BEGIN_CPLUSPLUS
extern sigqueue_t sigq_find_next_sig __((queue_t, sigqueue_t, int));
extern int sigq_remove_all __((queue_t, int));
extern int sigq_only_one __((queue_t, int));
_END_CPLUSPLUS

#if SIG_DEBUG
extern int sigq_alloc_count;
#endif	/* SIG_DEBUG */

#endif /* _KERNEL */

#endif /* ASSEMBLER */
#endif	/* _SYS_SIGINFO_H */

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
 *	@(#)$RCSfile: wait.h,v $ $Revision: 4.2.14.5 $ (DEC) $Date: 1994/01/06 14:24:11 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * NAME:  Waitpid, wait, and wait3 system call constants and definitions 
 *
 * ORIGIN: BSD, IBM
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

#ifndef _SYS_WAIT_H_
#define _SYS_WAIT_H_

#include <standards.h>
#include <sys/types.h>

/*
 * This file holds definitions relevent to the waitpid(), wait(), and wait3()
 * system calls.  The rusage option is only available with the wait3() call.
 * The options field in wait3() and waitpid() determines the behavior of the
 * call, while the process ID field in the waitpid() call determines which
 * group of children to search.
 */

/*
 * POSIX requires that certain values be included in wait.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */
#ifdef _POSIX_SOURCE

/*
 * If the user defines _BSD, they are obviously not looking for
 * POSIX definitions with respect to wait, so give 'em the BSD
 * interface.
 *
 */
#ifndef _BSD				/* POSIX definition of wait() */
#ifdef _NO_PROTO
extern pid_t wait();
#else
_BEGIN_CPLUSPLUS
extern pid_t wait(int *);
_END_CPLUSPLUS

#endif /* _NO_PROTO */
#endif /* _BSD */

/* 
 * waitpid is not in "old" BSD so that goes here 
 */

#ifdef _NO_PROTO
extern pid_t waitpid();
#else
_BEGIN_CPLUSPLUS
extern pid_t waitpid(pid_t , int *, int );
_END_CPLUSPLUS
#endif /* _NO_PROTO */


/*
 * The option field for wait3() and waitpid() is defined as follows:
 * WNOHANG causes the wait to not hang if there are no stopped or terminated
 * processes, rather returning an error indication in this case (pid==0).
 * WUNTRACED indicates that the caller should receive status about untraced
 * children which stop due to signals.  If children are stopped and a wait
 * without this option is done, it is as though they were still running...
 * nothing about them is returned.
 *
 */
#define WNOHANG		0x1	/* dont hang in wait			 */
#define WUNTRACED	0x2	/* tell about stopped, untraced children */


/*
 * Stopped process status.  Returned only for traced children unless requested
 * with the WUNTRACED option bit.  Lower byte gives the reason, next byte is
 * the last signal received, i.e. p->p_cursig.
 */
#define	_WSTOPPED	0177	/* bit set if stopped		*/

/*
 * MACRO defines for application interfacing to waitpid(), wait(), and wait3()
 */
#ifdef _BSD
#define	_W_INT(w)	(*(int *)&(w))	/* convert union wait to int */
#else
#define	_W_INT(i)	(i)
#endif

#define	_WSTATUS(x)	(_W_INT(x) & _WSTOPPED)

#ifndef _OSF_SOURCE
/* evaluates to a non-zero value if status returned for a stopped child	*/
#define WIFSTOPPED(x)	(_WSTATUS(x) == _WSTOPPED)
#else
/* See _OSF_SOURCE section for SVID3 useable WIFSTOPPED */
#endif

/* evaluates to the number of the signal that caused the child to stop	*/
#define WSTOPSIG(x)	(WIFSTOPPED(x) ? ((_W_INT(x) >> 8) & 0177) : -1)
/* evaluates to a non-zero value if status returned for normal termination */
#define WIFEXITED(x)	(_WSTATUS(x) == 0)
/* evaluates to the low-order 8 bits of the child exit status	*/
#define WEXITSTATUS(x)	(WIFEXITED(x) ? ((_W_INT(x) >> 8) & 0377) : -1)
/* evaluates to a non-zero value if status returned for abnormal termination */
#define WIFSIGNALED(x)	(_WSTATUS(x) != _WSTOPPED && _WSTATUS(x) != 0)
/* evaluates to the number of the signal that caused the child to terminate */
#define WTERMSIG(x)	(WIFSIGNALED(x) ? _WSTATUS(x) : -1)

#endif /* _POSIX_SOURCE */


#ifdef _OSF_SOURCE

#include <sys/param.h>

/*
 * Option for wait3(), mach extention.
 *
 */
#define WLOGINDEV       0x8000  /* tell about login device for child */


/* define for BSD or SVR4 compatibility	*/

#define WCOREFLAG       0200

#define WCOREDUMP(x)    ((x) & WCOREFLAG)

#define WCONTINUED      0x4    /* wait for processes continued SVID3 */
#define WNOWAIT         0x8    /* non destructive form of wait SVID3 */
#define	WTRAPPED	0x10
#define WEXITED		0x20	/* process exited */

#ifndef _BSD
/*
 * BSD uses this for something else. Don't really need unless compiling
 * a really old SVRn program.
 */
#define	WSTOPPED	WUNTRACED	/* old name for WUNTRACED */
#endif

/*
 * Evaluates to a non-zero value if status returned for continued child
 * value for continued processes (SVID3)
 */
#define _W_CONTINUED	0377	
#define WIFCONTINUED(x) ((_W_INT(x) & _W_CONTINUED) == _W_CONTINUED)

/*
 * Old macros which test _WSTATUS(x) != _WSTOPPED will fail when x is
 * set to _W_CONTINUED, which is what we want.  This is because a wait
 * returning because of a SIGCONT is in the same category as a wait
 * returning for a SIGSTOP as far as SVR4 is concerned -- both are
 * controlled by SA_NOCLDSTOP, etc.  A wait() only returns because of
 * a SIGCONT if the wait()ing process is in the SVR4 habitat.
 *
 * The WIFSTOPPED(x) macro has been changed so that it DOESN'T just use
 * _WSTATUS(x) in the the test to check specifically for STOP signals. 
 * the 8th bit must NOT be set for WIFSTOPPED(x) to succeed
 * NOTE: See above of _POSIX_SOURCE and _XOPEN_SOURCE defintion.
 */
#define WIFSTOPPED(x)	( (_WSTATUS(x) == _WSTOPPED) && !WIFCONTINUED(x) )

#ifdef _KERNEL
#define	WSIGINFO	0x40

/* return exit value if CLD_EXITED, otherwise signal */
#define WAITID_STATUS(x, y) \
	((x == CLD_EXITED) ? (_W_INT(y) >> 8) : (_W_INT(y) &~ WCOREFLAG))
#endif	/* _KERNEL */

/* define for BSD compatibility	*/

#ifdef _BSD

#define W_EXITCODE(ret, sig)    ((ret) << 8 | (sig))
#define W_STOPCODE(sig)		((sig) << 8 | _WSTOPPED)

#define	WSTOPPED	_WSTOPPED
#define W_STOPPED	_WSTOPPED

/*
 * Tokens for special values of the "pid" parameter to wait4.
 */
#define WAIT_ANY	(-1)    /* any process */
#define WAIT_MYPGRP     0       /* any process in my process group */

/*
 * Use of this union is deprecated
 */
union wait
{
	int	w_status;		/* used in syscall		*/

	struct				/* terminated process status	*/
	{
#if     BYTE_ORDER == BIG_ENDIAN
		unsigned short  w_PAD16;
                unsigned        w_Retcode:8;    /* exit code if w_termsig==0 */
                unsigned        w_Coredump:1;   /* core dump indicator */
                unsigned        w_Termsig:7;    /* termination signal */
#else
                unsigned 	w_Termsig:7;    /* termination signal */
                unsigned 	w_Coredump:1;   /* core dump indicator */
                unsigned 	w_Retcode:8;    /* exit code if w_termsig==0 */
                unsigned short  w_PAD16;
#endif
	} w_T;
	/*
         * Stopped process status.  Returned
         * only for traced children unless requested
         * with the WUNTRACED option bit.
         */
        struct {
#if     BYTE_ORDER == BIG_ENDIAN
                unsigned short  w_PAD16;
                unsigned        w_Stopsig:8;    /* signal that stopped us */
                unsigned        w_Stopval:8;    /* == _WSTOPPED if stopped */
#else
                unsigned	w_Stopval:8;    /* == _WSTOPPED if stopped */
                unsigned	w_Stopsig:8;    /* signal that stopped us */
                unsigned short  w_PAD16;
#endif
	} w_S;
};

#define	w_termsig	w_T.w_Termsig
#define w_coredump	w_T.w_Coredump
#define w_retcode	w_T.w_Retcode
#define w_stopval	w_S.w_Stopval
#define w_stopsig	w_S.w_Stopsig

#ifdef _NO_PROTO
extern int wait();
extern int wait3();
#else
_BEGIN_CPLUSPLUS
extern int wait(union wait *);
#include <sys/resource.h>   /* contains rusage struct for wait3 */
extern int wait3(union wait *, int, struct rusage *);
_END_CPLUSPLUS

#endif  /* !_NO_PROTO */

#endif /* _BSD */

#include <sys/procset.h>
#include <sys/siginfo.h>

#ifdef _NO_PROTO
extern int waitid();

#else

_BEGIN_CPLUSPLUS
extern int waitid(idtype_t, id_t, siginfo_t *, int);
_END_CPLUSPLUS
#endif /* _NO_PROTO */

#endif /* _OSF_SOURCE */

#endif /* _SYS_WAIT_H_ */

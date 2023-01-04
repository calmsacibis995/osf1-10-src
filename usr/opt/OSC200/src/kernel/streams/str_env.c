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
static char *rcsid = "@(#)$RCSfile: str_env.c,v $ $Revision: 4.2.8.7 $ (DEC) $Date: 1994/01/21 19:13:17 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>
#include <sys/cmn_err.h>
#include <sys/syslog.h>

#include <net/netisr.h>
#include <kern/sched_prim.h>	/* for assert_wait, etc */

/*
 * Some environment routines which are specified by
 * the STREAMS definition, but not available in OSF/1.
 * These come "without warranty", since the documentation
 * in the STREAMS programmer's guide and the System V
 * reference code leave some questions open. The first
 * usage in connection with software which is ported
 * from System V.3 will show whether we guessed right.
 * The open issues are pointed out at each routine.
 */

void
streams_delay(ticks)
int	ticks;
{
	assert_wait((vm_offset_t)0,FALSE);
	thread_set_timeout(ticks);
	thread_block();
}

/*
 * cmn_err - generalized interface to printf and panic
 *
 * The definition of cmn_err() has been derived from the AT&T SVR4 STREAMS
 * Programmer's Guide, as published by Prentice Hall. Refer to that document
 * for further information.
 *
 * According to the DKI/DDI spec if the first character is a '!' then the
 * output goes to the kernel buffer only.  If the first character of
 * fmt is a '^' then the output goes to the console only.
 */

void
cmn_err(level, fmt, ARGS)
        int     level;
        char    *fmt;
        long    ARGS;
{
        int     to_console;
        int     to_buffer;
        int     dont_panic;
	int 	log_level;
        char *  prefix;

        if ( fmt == NULL || fmt[0] == 0 ) {
                cmn_err(CE_CONT, "cmn_err called with NULL message\n");
                return;
        }

        switch ( fmt[0] ) {
        case '!':
                to_console = FALSE;
                to_buffer = TRUE;
                fmt++;
                break;
        case '^':
                to_console = TRUE;
                to_buffer = FALSE;
                fmt++;
                break;
        default:
                to_console = TRUE;
                to_buffer = TRUE;
                break;
        }

        switch ( level ) {
        default: /* unknown level -> print extra warning */
                cmn_err(CE_CONT, "cmn_err called with bogus level paramter\n");
                /* falls through! */
        case CE_CONT:
                prefix = NULL;
                dont_panic = TRUE;
		log_level = LOG_DEBUG;
                break;
        case CE_NOTE:
                prefix = "NOTE";
                dont_panic = TRUE;
		log_level = LOG_NOTICE;
                break;
        case CE_WARN:
                prefix = "WARNING";
                dont_panic = TRUE;
		log_level = LOG_WARNING;
                break;
        case CE_PANIC:
                prefix = "PANIC";
                dont_panic = FALSE;
		log_level = LOG_EMERG;
                break;
        }

        if ( prefix && to_console)
                printf("%s: ",prefix);
        if ( to_console )
                printf(fmt, ARGS);
        if ( to_buffer && !to_console)
		log(log_level,fmt,ARGS);
        if ( prefix && to_console)
                printf("\n");
        if ( !dont_panic )
                panic("cmn_err: CE_PANIC");
}

/*
 * Resolution of name conflicts which can't be handled by the C preprocessor
 *
 * This passage must be compliant with the definitions in the "conflicts"
 * section of str_config.h!
 */

time_t
streams_time()
{
	struct timeval now;

	microtime(&now);
	return(now.tv_sec);
}

extern int tick;

time_t
streams_lbolt()
{
	struct timeval now;
	long sec, usec;

	microtime(&now);
	sec  = now.tv_sec -  boottime.tv_sec;
	usec = now.tv_usec - boottime.tv_usec;
	if (usec < 0) {
		--sec;
		usec += (1000*1000);
	}
	/* Return ticks since boot. */
	sec = ((sec * hz) + (((usec + tick - 1) * hz) / (1000*1000)));
	if (sec <= 0) sec = 1;
	return (time_t) sec;
}


/*
 *	Timeout handling
 *
 *	Source interface:
 *		id = timeout(func, arg, ticks)
 *		untimeout(id);
 *
 *	The C-Preprocessor maps
 *		timeout -> streams_timeout
 *		untimeout -> streams_untimeout
 *
 *	Intercepting these routines solves two problems
 *		- change of interfaces from V to OSF/1
 *		- correct synchronization in MP case
 *
 *	Interface change:
 *		- System V interface explained above
 *		- OSF/1 uses the (func, arg) pair as identifier,
 *		  i.e. those are the arguments to untimeout, and
 *		  timeout is a void function.
 *
 *	Data structure
 *		For our "callout" list, we use a chain of SQ's, headed
 *		by str_to_head and protected by the simple_lock str_to_lock.
 *		The SQ's get allocated dynamically, and removed after use.
 */

struct { SQP next, prev; } str_to_head;
int str_to_id = 0;
decl_simple_lock_data(static,str_to_lock)

void
str_to_init(void)
{
	extern task_t first_task;

	simple_lock_init(&str_to_lock);
	str_to_head.next = str_to_head.prev = (SQP)&str_to_head;
}


static void
str_timeout(sq)
	caddr_t	sq;
{
        void (*to_func)();
	caddr_t to_arg;
	SQP good;
	SQP find_to();
	int id;

	to_func = ((SQP)sq)->sq_entry;
	to_arg = ((SQP)sq)->sq_arg0;
	id = (int)((SQP)sq)->sq_arg1;
	good = find_to(id,1);
	if (good) {
		STR_FREE(sq, M_STRSQ);
		(*to_func)(to_arg);
	}
}

static SQP
find_to(id, remove)
	int id, remove;
{
	SQP	sq;
	SIMPLE_LOCK_DECL

	SIMPLE_LOCK(&str_to_lock);
	/* Check for presence timeout queues */
	for (sq = str_to_head.next;
	     sq != (SQP)&str_to_head;
	     sq = sq->sq_next)
		if ((int)sq->sq_arg1 == id) {
			if (remove) remque(sq);
			SIMPLE_UNLOCK(&str_to_lock);
			return sq;
		}
	SIMPLE_UNLOCK(&str_to_lock);
	return 0;
}

void
streams_untimeout(id)
	int	id;
{
	SQP	sq;

	if (sq = find_to(id, 0)) {
		/* Stop timeout, then remove entry (if still there) */
		untimeout(str_timeout, (caddr_t)sq);
		if (sq = find_to(id, 1))
			STR_FREE(sq, M_STRSQ);
	}
}

int
streams_timeout(func, arg, ticks)
	timeout_fcn_t	func;
	timeout_arg_t	arg;
	int	ticks;
{
	SQP	sq;
	int	retval;
	SIMPLE_LOCK_DECL

	STR_MALLOC(sq, SQP, sizeof *sq, M_STRSQ, M_NOWAIT);
	if (sq == 0)
		return 0;
	sq_init(sq);

	sq->sq_entry = (sq_entry_t)func;
	sq->sq_arg0  = arg;

	SIMPLE_LOCK(&str_to_lock);
	if ((retval = ++str_to_id) == 0)
		retval = ++str_to_id;
	sq->sq_arg1 = (void *)retval;
	insque(sq, str_to_head.next);
	SIMPLE_UNLOCK(&str_to_lock);

	timeout(str_timeout, (caddr_t)sq, ticks);

	return retval;
}

/*
 *	streams_mpsleep - MP safe version of mpsleep
 *
 *	We need this wrapper function for the actual
 *	sleep. It takes care of releasing and re-acquiring the locks under
 *	which the caller is running. There is quite a bit of knowledge about
 *	which locks can possibly be held at this time hard-coded in here,
 *	in order to avoid some expensive registry mechanism at each
 *	acquire and release operation.
 *
 *	The possible locks held are
 *	- this queue itself
 *		The sqh_owner identification which we find there, serves
 *		later to find out whether we own the other locks. So it'd
 *		better be intact!
 *
 *	- the queue above us
 *		This is usually the stream head. The only exception to the
 *		rule is re-open, which takes care of the stream head itself.
 *
 *	- the queue below us (if any)
 *
 *	SINCE THE INTRODUCTION OF SQLVL_QUEUE, we have to take care of
 *	each read and write queue separately. This doubles the number of
 *	potentially held locks...
 *
 *	- the mult_sqh lock
 *		This is a global lock, needed whenever we are working with
 *		more than one lock. A consistency check might be made here.
 *
 *	After having released all those locks, we might also insert another
 *	consistency check in order to assert that we are not holding any
 *	more locks...
 *		
 *	This routine may only be called during qi_qclose and qi_qopen
 *	routines and potentially on non-syncq streams, so we check the
 *	return of csq_which_q().
 */

#define	R	0	/* read */
#define	W	1	/* write */
#define	LR	2	/* lower read */
#define	LW	3	/* lower write */
#define	UR	4	/* upper read */
#define	UW	5	/* upper write */
#define	NQ	6

int
streams_mpsleep(chan, pri, mesg, tmo, lockp, flags)
	caddr_t	chan;
	int	pri;
	const char *mesg;
	int	tmo;
	void	*lockp;
	int	flags;
{
	int		retval;
	SQP		my_sq;
	queue_t *	my_qs[NQ];
	int		i = 0, have_mult_sqh = 0;

	if (my_qs[R] = csq_which_q()) {
		if (pri > PZERO)
			pri |= PCATCH;
		/*
		if (pri & PCATCH)
			pri |= PSUSP;
		*/
		if (chan)
			assert_wait((vm_offset_t)chan, (pri & PCATCH) != 0);
		my_sq = my_qs[R]->q_sqh.sqh_parent->sqh_owner;
		if (!(my_qs[R]->q_flag & QREADR)) {
			my_qs[W] = my_qs[R];
			my_qs[R] = OTHERQ(my_qs[W]);
		} else
			my_qs[W] = OTHERQ(my_qs[R]);
		my_qs[LR] = my_qs[R]->q_next;
		my_qs[UR] = backq(my_qs[R]);
		my_qs[LW] = backq(my_qs[W]);
		my_qs[UW] = my_qs[W]->q_next;
		if ( have_mult_sqh = (mult_sqh.sqh_owner == my_sq) )
			csq_release(&mult_sqh);
		for ( ; i < NQ; i++)
			if ( my_qs[i]
			&&   my_qs[i]->q_sqh.sqh_parent->sqh_owner == my_sq )
				csq_release(&my_qs[i]->q_sqh);
			else
				my_qs[i] = nilp(queue_t);
	} /* Else thread not in Streams context - no translation necessary */

	retval = mpsleep((caddr_t)0, pri, (char *)0, tmo, lockp, flags);

	if (have_mult_sqh)
		csq_acquire(&mult_sqh, my_sq);
	while (--i >= 0)
		if (my_qs[i])
			csq_acquire(&my_qs[i]->q_sqh, my_sq);
	return retval;
}

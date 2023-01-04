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
static char     *rcsid = "@(#)$RCSfile: kern_time.c,v $ $Revision: 4.4.19.2 $ (DEC) $Date: 1993/08/12 15:47:20 $";
#endif

/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0.1
 */
/*
 * kern_time.c
 *
 * Modification History:
 *
 * 04-Feb-92    Jeff Denham
 *	. Upgrade POSIX.4 behavior to Draft 11.
 *	. Add code and functions to perform POSIX.4 timer creation and
 *	  deletion in the kernel to support TIMER_MAX == 32 more
 *	  efficiently. Also provided compatibility code to support
 *	  D10 binaries.
 *
 * 19-Sep-91    Jeff Denham
 *	. Merged in OSF/1.0.1 fixes for BSD-style realtimer clear-on-exec.
 *	. To fix clearing of POSIX.4 timers on exec, added
 *	  clear_psx_timers() funtion for RT_TIMERS.
 *
 * 7-Jun-91 	Jeff Denham
 *      P1003.4/D10 requires that timeout values be rounded up to the next
 *      clock tick. It also requires that an abstime already passed result
 *	in timeout notification.
 *
 * 25-Apr-91 	Paula Long
 *      Changed max interval from a value to a constant in itimerfix();
 *
 * 24-Apr-91     Jeff Denham
 *	For P1003.4, add call to routine in kern_clock.c to adjust
 *	timers entered in the callout queue when settimeofday() is
 *	called.
 *
 * 4-Apr-91     Paula Long
 *      Add P1003.4 required extensions.
 *      Specifically <rt_timer.h> is now included and if RT_TIMER
 *      is defined the psx timer routines and usleep_thread is included.
 *
 */

/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <cputypes.h>
#include <sys/secdefines.h>

#include <sys/param.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>

#include <machine/reg.h>
#include <machine/cpu.h>
#include <sys/time.h>
#if	SEC_BASE
#include <sys/security.h>
#endif
#include <rt_timer.h>

#if RT_TIMER
#include <kern/zalloc.h>
#include <kern/queue.h>
#include <vm/vm_kern.h>
#endif

/* 
 * Time of day and interval timer support.
 *
 * These routines provide the kernel entry points to get and set
 * the time-of-day and per-process interval timers.  Subroutines
 * here provide support for adding and subtracting timeval structures
 * and decrementing interval timers, optionally reloading the interval
 * timers when they expire.
 */
extern int inaccinfinite;	/* Flag indicating inaccuarcy is infinite */

gettimeofday(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		struct	timeval *tp;
		struct	timezone *tzp;
	} *uap = (struct args *) args;
	struct timeval atv;
	struct timezone atz;
	int	error, s;

#ifdef	balance
	/*
	 * Return number of users to programs that limit # of users.
 	 * (value in R1 on return from gettimeofday()).
	 *
	 * DYNIX login uses this to tell if too many users logged in
	 * for a binary license.
	 */
	{
		extern unsigned sec0eaddr;
		retval[1] = sec0eaddr;
	}
#endif
	if (uap->tp) {
		microtime(&atv);
		if (error = copyout((caddr_t)&atv, (caddr_t)uap->tp,
			sizeof (atv)))
			return (error);
	}
	s = splhigh();
	TIME_READ_LOCK();
	atz = tz;
	TIME_READ_UNLOCK();
	splx(s);
	if (uap->tzp)
		return (copyout((caddr_t)&atz, (caddr_t)uap->tzp,
			sizeof(struct timezone)));
	return (0);
}

settimeofday(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		struct	timeval *tv;
		struct	timezone *tzp;
	} *uap = (struct args *) args;
	struct timeval atv;
	struct timezone atz;
	extern struct timezone *mach_tz;
	register int s;
	int error = 0;

	if (uap->tv) {
		if (error = copyin((caddr_t)uap->tv, (caddr_t)&atv,
			sizeof (struct timeval)))
			return (error);
#if	!AT386
		error = setthetime(&atv);
#endif
	}
	if (!error && uap->tzp) {
#if     SEC_BASE
		if (!privileged(SEC_SYSATTR, EPERM))
			return (EPERM);
#else
		if ((error = suser(u.u_cred, &u.u_acflag)))
			return (error);
#endif
		error = copyin((caddr_t)uap->tzp, (caddr_t)&atz,
			sizeof (atz));
		if (error == 0) {
			s = splhigh();
			TIME_WRITE_LOCK();
			tz = atz;
			if (mach_tz) *mach_tz = tz;
			TIME_WRITE_UNLOCK();
			splx(s);
		}
	}
#if	AT386
	/* Since rtc maintains local time, tz must be set first. */
	if (!error && uap->tv)
		return (setthetime(&atv));
#endif
	return (error);
}

setthetime(tv)
	struct timeval *tv;
{
	int error, s;

#if     SEC_BASE
	if (!privileged(SEC_SYSATTR, EPERM))
		return (EPERM);
#else  
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
/* WHAT DO WE DO ABOUT PENDING REAL-TIME TIMEOUTS??? */
	s = splhigh();
	TIME_WRITE_LOCK();
	inaccinfinite = 1;		/* Inaccuracy is infinite */
	boottime.tv_sec += tv->tv_sec - time.tv_sec;
#if	RT_TIMER
	(void) psx4_adjust_callout(tv);
#endif	RT_TIMER
	time = *tv;
	TIME_WRITE_UNLOCK();
	splx(s);
	resettodr();
	return (0);
}

extern	int tickadj;			/* "standard" clock skew, us./tick */
int	tickdelta;			/* current clock skew, us. per tick */
long	timedelta;			/* unapplied time correction, us. */
long	bigadj = 1000000;		/* use 10x skew above bigadj us. */
#ifndef multimax
int	doresettodr = 0; 
#endif

int
  adjtime (struct proc *p, void *args, long *retval)
{
	register struct args {
		struct timeval *delta;
		struct timeval *olddelta;
	} *uap = (struct args *) args;
	struct timeval atv, oatv;
	register long ndelta = 0;
	int error, s;
	
#if     SEC_BASE
	if (!privileged(SEC_SYSATTR, EPERM))
	        return (EPERM);
#else  
	if (error = suser(u.u_cred, &u.u_acflag))
	  return (error);
#endif  
	if (uap->delta) {
		if (error = copyin((caddr_t)uap->delta, (caddr_t)&atv, sizeof (struct timeval))) {
			return (error);
		}
		ndelta = atv.tv_sec * 1000000 + atv.tv_usec;
	}
	s = splhigh();
	TIME_WRITE_LOCK();

	if (uap->olddelta) {
		oatv.tv_sec = timedelta / 1000000;
		oatv.tv_usec = timedelta % 1000000;
	}
	if (uap->delta && ndelta) {
		inaccinfinite = 1;		/* Inaccuracy is infinite */
	 
		if ((ndelta > bigadj) || (ndelta < -bigadj)) {
			tickdelta = 10 * tickadj;
		} else {
			tickdelta = tickadj;
		}
	
		if (ndelta % tickdelta) {
			ndelta = ndelta / tickdelta * tickdelta;
		}
#if defined(PMAX) || defined(multimax)
#else
		if (ndelta)
			doresettodr = 1; 
		else if (timedelta && doresettodr) {
                	doresettodr = 0;
                	resettodr();
		}	
#endif 
		timedelta = ndelta;
	}
	TIME_WRITE_UNLOCK();
	splx(s);

	if (uap->olddelta) {
		if (error = copyout((caddr_t)&oatv, (caddr_t)uap->olddelta,sizeof(struct timeval))){
                        return (error);
		}
		return (0);
	}
}

/*
 * Get value of an interval timer.  The process virtual and
 * profiling virtual time timers are kept in the u. area, since
 * they can be swapped out.  These are kept internally in the
 * way they are specified externally: in time until they expire.
 *
 * The real time interval timer is kept in the process table slot
 * for the process, and its value (it_value) is kept as an
 * absolute time rather than as a delta, so that it is easy to keep
 * periodic real-time signals from drifting.
 *
 * Virtual time timers are processed in the hardclock() routine of
 * kern_clock.c.  The real time timer is processed by a timeout
 * routine, called from the softclock() routine.  Since a callout
 * may be delayed in real time due to interrupt processing in the system,
 * it is possible for the real time timeout routine (realitexpire, given below)
 * to be delayed in real time past when it is supposed to occur.  It
 * does not suffice, therefore, to reload the real timer .it_value from the
 * real time timers .it_interval.  Rather, we compute the next time in
 * absolute time the timer should go off.
 */
getitimer(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	which;			/* real type: int */
		struct	itimerval *itv;
	} *uap = (struct args *) args;
	struct itimerval aitv;
	int error, s;
	register unsigned int which = uap->which;

	if (which > 2)
		return (EINVAL);
	s = splhigh();

	if (which == ITIMER_REAL) {
		/*
		 * Convert from absoulte to relative time in .it_value
		 * part of real time timer.  If time for real time timer
		 * has passed return 0, else return difference between
		 * current time and time for the timer to go off.
		 */
		PROC_TIMER_LOCK(u.u_procp);
		aitv = u.u_procp->p_realtimer;
		PROC_TIMER_UNLOCK(u.u_procp);
		if (timerisset(&aitv.it_value)) {
			TIME_READ_LOCK();
			if (timercmp(&aitv.it_value, &time, <))
				timerclear(&aitv.it_value);
			else
				timevalsub(&aitv.it_value, &time);
			TIME_READ_UNLOCK();
		}
	} else {
		U_TIMER_LOCK();
		aitv = u.u_timer[which];
		U_TIMER_UNLOCK();
	}
	splx(s);
	return (copyout((caddr_t)&aitv, (caddr_t)uap->itv,
	    sizeof (struct itimerval)));
}


/*
 * ITIMER_REAL_COE HACK XXXXX	Undocumented extension.
 * This routines is called to clear an ITIMER_REAL upon exec.  Used
 * only by the user-space nano_timer implementation.
 * *** Remove this when nano_timers migrate into the kernel ***
 */
#define ITIMER_REAL_COE 0xDEADBEEF

int clear_p_realtimer(struct proc *p)
{
	struct itimerval aitv; 

	(void)untimeout(realitexpire, (caddr_t)p);
	timerclear(&aitv.it_interval);
	timerclear(&aitv.it_value);
	PROC_TIMER_LOCK(p);
	p->p_realtimer = aitv;
	p->p_realtimer_coe = 0;
	PROC_TIMER_UNLOCK(p);
	return(0);
}


setitimer(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		u_long	which;			/* real type: int */
		struct	itimerval *itv, *oitv;
	} *uap = (struct args *) args;
	struct itimerval aitv; 
	register struct itimerval *itvp;
	int s, error;
	int is_coe_hack = 0;		/* clear p_realtimer on exec */
	register unsigned int which = uap->which;

	/*
	 * ITIMER_REAL_COE HACK XXXXX	Undocumented extension
	 * supports nano_timer facility, which isn't inherited across exec,
	 * unlike setitimer.  HACK in common with user-space nano_timer code.
	 * *** Remove this when nano_timers migrate into the kernel ***
	 */
	if (which == ITIMER_REAL_COE)
		which = uap->which = ITIMER_REAL, is_coe_hack = 1;
	if (which > 2) 
		return (EINVAL);

	itvp = uap->itv;
	if (itvp && (error = copyin((caddr_t)itvp, (caddr_t)&aitv,
	    sizeof(struct itimerval))))
		return (error);
	if ((uap->itv = uap->oitv) && (error = getitimer(p, uap, retval)))
		return (error);
	if (itvp == 0)
		return (0);
	if (itimerfix(&aitv.it_value) || itimerfix(&aitv.it_interval))
		return (EINVAL);

	s = splhigh();
	if (which == ITIMER_REAL) {
		untimeout(realitexpire, (caddr_t)p);
		if (timerisset(&aitv.it_value)) {
			TIME_READ_LOCK();
			timevaladd(&aitv.it_value, &time);
			TIME_READ_UNLOCK();
			timeout(realitexpire, (caddr_t)p,hzto(&aitv.it_value));
		} else
			is_coe_hack = 0;
		PROC_TIMER_LOCK(p);
		p->p_realtimer = aitv;
		p->p_realtimer_coe = (short)is_coe_hack;
		PROC_TIMER_UNLOCK(p);
	} else {
		U_TIMER_LOCK();
		u.u_timer[which] = aitv;
		U_TIMER_UNLOCK();
	}
	splx(s);
	return (0);
}

/*
 * Real interval timer expired:
 * send process whose timer expired an alarm signal.
 * If time is not set up to reload, then just return.
 * Else compute next time timer should go off which is > current time.
 * This is where delay in processing this timeout causes multiple
 * SIGALRM calls to be compressed into one.
 *
 * Assumes called on the master processor!
 */
realitexpire(p)
	register struct proc *p;
{
	struct timeval	rit_value;
	int s;

	psignal(p, SIGALRM);
	s = splhigh();
	PROC_TIMER_LOCK(p);
	if (!timerisset(&p->p_realtimer.it_interval)) {
		timerclear(&p->p_realtimer.it_value);
		p->p_realtimer_coe = 0;	/* clear "clear on exec" */
		PROC_TIMER_UNLOCK(p);
		splx(s);
		return;
	}
	for (;;) {
		timevaladd(&p->p_realtimer.it_value,
		    &p->p_realtimer.it_interval);
		TIME_READ_LOCK();
		if (timercmp(&p->p_realtimer.it_value, &time, >)) {
			TIME_READ_UNLOCK();
			rit_value = p->p_realtimer.it_value;
			PROC_TIMER_UNLOCK(p);
			timeout(realitexpire, (caddr_t)p, hzto(&rit_value));
			splx(s);
			return;
		}
		TIME_READ_UNLOCK();
	}
	/* NOTREACHED */
}

/*
 * Check that a proposed value to load into the .it_value or
 * .it_interval part of an interval timer is acceptable, and
 * fix it to have at least minimal value (i.e. if it is less
 * than the resolution of the clock, round it up.)
 */
itimerfix(tv)
	struct timeval *tv;
{

	if (tv->tv_sec < 0 || tv->tv_sec > TOD_MAX_SECONDS ||
	    tv->tv_usec < 0 || tv->tv_usec >= 1000000)
		return (EINVAL);
	if (tv->tv_sec == 0 && tv->tv_usec != 0 && tv->tv_usec < tick)
		tv->tv_usec = tick;
	return (0);
}

/*
 * Decrement an interval timer by a specified number
 * of microseconds, which must be less than a second,
 * i.e. < 1000000.  If the timer expires, then reload
 * it.  In this case, carry over (usec - old value) to
 * reducint the value reloaded into the timer so that
 * the timer does not drift.  This routine assumes
 * that it is called in a context where the timers
 * on which it is operating cannot change in value.
 */
itimerdecr(itp, usec)
	register struct itimerval *itp;
	long usec;
{

	if (itp->it_value.tv_usec < usec) {
		if (itp->it_value.tv_sec == 0) {
			/* expired, and already in next interval */
			usec -= itp->it_value.tv_usec;
			goto expire;
		}
		itp->it_value.tv_usec += 1000000;
		itp->it_value.tv_sec--;
	}
	itp->it_value.tv_usec -= usec;
	usec = 0;
	if (timerisset(&itp->it_value))
		return (1);
	/* expired, exactly at end of interval */
expire:
	if (timerisset(&itp->it_interval)) {
		itp->it_value = itp->it_interval;
		itp->it_value.tv_usec -= usec;
		if (itp->it_value.tv_usec < 0) {
			itp->it_value.tv_usec += 1000000;
			itp->it_value.tv_sec--;
		}
	} else
		itp->it_value.tv_usec = 0;		/* sec is already 0 */
	return (0);
}

/*
 * Add and subtract routines for timevals.
 * N.B.: subtract routine doesn't deal with
 * results which are before the beginning,
 * it just gets very confused in this case.
 * Caveat emptor.
 */
timevaladd(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec += t2->tv_sec;
	t1->tv_usec += t2->tv_usec;
	timevalfix(t1);
}

timevalsub(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec -= t2->tv_sec;
	t1->tv_usec -= t2->tv_usec;
	timevalfix(t1);
}

timevalfix(t1)
	struct timeval *t1;
{

	if (t1->tv_usec < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000;
	}
#ifdef HW_DIVIDE
	/* If the Alpha had a hardwarde integer divide, we'd
	 * do it this way.
	 */
	if (t1->tv_usec >= 1000000) {
		t1->tv_sec  += t1->tv_usec / 1000000;
		t1->tv_usec %= 1000000;
	}
#else HW_DIVIDE
	/* Unfortunately, we have to hack in this loop because
	 * they tell me INTEGER DIVIDE is simulated in software.
	 */
	while (t1->tv_usec >= 1000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000;
	}
#endif HW_DIVIDE
}

#if RT
/*
 * suspend the execution of the
 * current thread until either
 * the specified time has expired
 * or the wait is interrupt
 * via a signal
 */
usleep_thread(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		struct	timeval *itv, *oitv;
	} *uap = (struct args *) args;
	struct timeval aitv; 
	register struct timeval *itvp, *oitvp;
	int s, error,tmp;
        struct timeval t_output={0,0};
	register thread_t	thread;                           /* psx4 */

	thread = current_thread();			/* get thread id */
	itvp = uap->itv;
        oitvp = uap->oitv;

        /*
         * do error checking input arguments
         */

	if (itvp == 0)
		return (0);


	if (error = copyin((caddr_t)itvp, (caddr_t)&aitv,
	    sizeof(struct timeval)))
		return (error);

    	if( oitvp && (error = copyout((caddr_t)&t_output, (caddr_t)oitvp,
	           sizeof (struct timeval))))
                   return(error);

        if (itimerfix(&aitv))
		return (EINVAL);

        
        /* 
         * a vaild sleep time was specified
         * block the thread waiting on a timeout interruptibly
         */
 
	if (timerisset(&aitv)) {
           s = splsched();
           thread_lock(thread);
           thread->time_remaining = 0;
           if (oitvp) 
               thread->psx4_sleep = TRUE;
           else
               thread->psx4_sleep = FALSE;
           thread_unlock(thread);
           splx(s);
           s = splhigh();
           TIME_READ_LOCK();
           timevaladd(&aitv.tv_sec, &time);
           TIME_READ_UNLOCK();
           splx(s);
           assert_wait((vm_offset_t)0, TRUE);     /* wait interruptible. */
           thread_set_timeout(hzto(&aitv.tv_sec)); /* queue timeout      */    
           thread_block();                         /* wait for timeout   */   
         }
         if (oitvp) {				 /* time remaining request */
            if ( tmp = thread->time_remaining) {
               t_output.tv_sec = tmp / hz;    /* convert from ticks     */
	       t_output.tv_usec = (tmp % hz) * tick;
               if(error = copyout((caddr_t)&t_output, (caddr_t)oitvp,
	           sizeof (struct timeval)))
                   return(error);
            }
         }     
       return(0); 
  }
#else
usleep_thread(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (ENOSYS);
}
#endif   /* RT */

#if RT_TIMER

int psx_driftrate = 0;		/* P1003.4 clock drift adjument        */

#define	TIMER_VALID 0xDEADBABEL

/*
 * Maximum expansion size of the realtime timer zone.
 */
#define RTTIMER_MAX_MEM		(nproc * sizeof(psx4_tblock_t))

/*
 * The amount of memory allocate each time the zone is expanded.
 * We can make this 4, but the zalloc allocation algorithm is
 * going try to optimize the number.
 */
#define RTTIMER_ALLOC_MEM	(4 * sizeof(psx4_tblock_t))


extern psx4_timer_t *psx4_timer_alloc(struct proc *, long *);
extern psx4_timer_t *psx4_tid_to_addr(struct proc *, long);
extern int 	     psx4_timer_dealloc(struct proc *, long);

struct zone *rttimer_zone;	/* memory zone of psx_tblock_t's */

/*
 * rttimer_init() -- called from init_main() to initialize the zone for
 * timer blocks (1 per proc)
 */
rttimer_init()
{
	extern struct zone	*rttimer_zone;

	/*
	 * Create the zone to hold real time signal event entries.
	 */
	rttimer_zone = zinit((vm_size_t) sizeof(psx4_tblock_t),
			     (vm_size_t) RTTIMER_MAX_MEM, 
			     (vm_size_t) RTTIMER_ALLOC_MEM,
			     "rttimer block zone");
			
}

/*
 * P1003.4 timer_create support routine
 *
 * The following system call creates a POSIX.4 for timer
 * in the proc structure.
 *
 * P1003.4 supports TIMER_MAX timers.
 *
 */
long
psx4_timer_create(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	clock_id;		/* real type: int */
		sigevent_t *evp;
	} *uap = (struct args *) args;
	struct itimerval aitv;
	int error, s;
	register int i;
	long timerid;
	psx4_timer_t *timerp;

	s = splhigh();
	PROC_TIMER_LOCK(p);

	/*
	 * Check to see whether timer block exists.
	 */
	if (p->p_psx4_timer == NULL) {
		vm_offset_t z;
		if ((z = zalloc(rttimer_zone)) == NULL) {
			PROC_TIMER_UNLOCK(p);
			splx(s);
			return (EAGAIN);
		}
		(vm_offset_t) p->p_psx4_timer = z;
		p->p_psx4_timer->psx4tb_free = 0;
		for (i = 0; i < TIMER_MAX; i++) {
			p->p_psx4_timer->psx4_timers[i].psx4t_tid = i;
			p->p_psx4_timer->psx4_timers[i].psx4t_idx = i + 1;
		}
	}

	timerp = psx4_timer_alloc(p, &timerid);
	if (timerp == NULL) {
		PROC_TIMER_UNLOCK(p);
		splx(s);
		return (EMTIMERS);
	}

	timerp->psx4t_active = FALSE;

	if (uap->evp) {
		sigevent_t sigevt;
		if (error = copyin((caddr_t)uap->evp, (caddr_t)&sigevt,
			sizeof (sigevent_t))) {
			psx4_timer_dealloc(p, timerid);
			PROC_TIMER_UNLOCK(p);
			splx(s);
			return(error);
		}
		timerp->psx4t_signo = sigevt.sigev_signo;
		timerp->psx4t_value = sigevt.sigev_value;
	} else {
		timerp->psx4t_signo = SIGALRM;
		timerp->psx4t_value.sival_ptr = NULL;
	}

#if RT_SIGNALS
	/* 
	 * Reserve rtsig_event_t entry for the timer.
	 */
#else
	PROC_TIMER_UNLOCK(p);
#endif

	*retval = timerid;
	splx(s);
	return (0);
}

/*
 * P1003.4 timer_delete routine.
 *
 * The following system call create a POSIX.4 for timer
 * in the proc structure.
 *
 * P1003.4 supports TIMER_MAX timers.
 *
 */
psx4_timer_delete(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long 	timerid;		/* real type: timer_t */
	} *uap = (struct args *) args;
	struct itimerval aitv;
	int error = 0, s;
	register psx4_timer_t *timerp;

	s = splhigh();
	PROC_TIMER_LOCK(p);

	if ((timerp = psx4_tid_to_addr(p, uap->timerid)) == NULL) {
		PROC_TIMER_UNLOCK(p);
		splx(s);
		return (EINVAL);
	}

	timerp->psx4t_active = FALSE;
	timerclear(&timerp->psx4t_timeval.it_value);
	timerclear(&timerp->psx4t_timeval.it_interval);
	untimeout(psx4_tod_expire, (caddr_t) timerp);
#if RT_SIGNALS
	/* 
	 * Unreserve rtsig_event_t entry for the timer.
	 */
#endif
	error = psx4_timer_dealloc(p, uap->timerid);
	PROC_TIMER_UNLOCK(p);
	splx(s);
	return(error);	
}

/*
 * P1003.4 timer_gettime support routine
 *
 * The following system call returns to the 
 * user the time remaining on the specified timer.
 *
 * P1003.4 supports TIMER_MAX timers.
 *
 */
psx4_get_todtimer(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long 	which;
		struct	itimerval *itv;
	} *uap = (struct args *) args;
	struct itimerval aitv;
	int error, s;
	psx4_timer_t *timerp;

	s = splhigh();
	PROC_TIMER_LOCK(p);
	if ((timerp = psx4_tid_to_addr(p, uap->which)) == NULL) {
		PROC_TIMER_UNLOCK(p);
		splx(s);
		return (EINVAL);
	}

	/*
	 * Timeouts are stored as absolute time values.	 
	 * The psx4_tod_gettimer call returns the interval remaining on 
	 * the timer.  The interval is calculated by taking the
	 * difference between the timeout value and the current time.
	 * If the timer is not set a zero is returned.
	 *
	 * PSX4 timers timeout values are adjusted when the
	 * system time is changed via the settimeofday() system call.
	 */

	aitv = timerp->psx4t_timeval; 
	PROC_TIMER_UNLOCK(p);
	if (timerisset(&aitv.it_value)) {	/* time remaining ? */
		TIME_READ_LOCK();
		if (timercmp(&aitv.it_value, &time, <))  
			timerclear(&aitv.it_value);
		else
			timevalsub(&aitv.it_value, &time);
		TIME_READ_UNLOCK();
	}
	splx(s);
	return (copyout((caddr_t)&aitv, (caddr_t)uap->itv,
	    sizeof (struct itimerval)));
}

/*
 * P1003.4 timer_settime support routine.
 *
 * The following system call enables  the specified 
 * P1003.4 REALTIME timer.  The current implementation
 * support TIMER_MAX timers per-process.  
 *
 * Timers can be either absolute (relative to EPOCH) 
 * or relative (relative to NOW).  Callers can also specify
 * the signal to raise when the timer has expired. 
 * 
 * 
 */
psx4_set_todtimer(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		which;		/* real type: 'timer_t' */
		struct itimerval *itv, *oitv;
		unsigned long	flags;
		long		signo;		/* real type: 'int' */
		sigval_t 	value;
	} *uap = (struct args *) args;
	struct itimerval aitv; 
	register struct itimerval *itvp;
	register psx4_timer_t *timerp;
	int s, error;
	int ticks;
	register long which = uap->which;
	register int signo = uap->signo;

	/*
	 * Valid Timer? 
	 * if not return error
	 */

	if ((timerp = psx4_tid_to_addr(p, which)) == NULL) {
#ifndef mips
		return EINVAL;
	}
#else	/* mips */
	/*
	 * BINARY COMPATIBILITY WITH DRAFT 10 on mips: strictly speaking, 
	 * we should return (EINVAL) here. But because a Draft 10
	 * binary won't have done an official timer_create() in
	 * the kernel, we're going to do it here for them. The
	 * only reason we can afford to take this chance is that
	 * we trust the library routine to throw us only timer IDs
	 * that it knows are valid. There is some chance of a "timer
	 * leak" here, but we know we've got 2 dozen+ timers to play with.
	 * On the down side, we're going to have to do most of this by
	 * hand.
	 */
		if (which < 0 || which >= TIMER_MAX)
			return(EINVAL);

		if (p->p_psx4_timer == NULL) {

			/*
			 * First, we need to bootstrap the timer structures
			 * with a call to psx4_timer_create(). Then we'll
			 * give back the allocated ID and force the
			 * correct timer to be allocated by hand.
			 */

			struct {
				long	cid;
				sigevent_t *evp;
			} fake_uap;
			fake_uap.cid = 1; /* CLOCK_REALTIME */
			fake_uap.evp = (struct sigevent *)0;
			if (error = psx4_timer_create(p, &fake_uap, retval)) {
				PROC_TIMER_UNLOCK(p);
				splx(s);
				return(EINVAL);
			}
			s = splhigh();
			PROC_TIMER_LOCK(p);
			(void) psx4_timer_dealloc(p, *retval);
			PROC_TIMER_UNLOCK(p);
			splx(s);
		}
		/*
		 * Now we're going to allocate timer uap->which
		 * by hand and hope to hell we're doing the right thing...
		 */
		s = splhigh();
		PROC_TIMER_LOCK(p);
		/* What follows is from psx4_timer_alloc()... */
		p->p_psx4_timer->psx4tb_free =
			p->p_psx4_timer->psx4_timers[which].psx4t_idx;
		p->p_psx4_timer->psx4_timers[which].psx4t_idx = TIMER_VALID;
		timerp = &p->p_psx4_timer->psx4_timers[which];
		/* What follows is from psx4_timer_create()... */
		timerp->psx4t_active = FALSE;
		timerp->psx4t_signo = signo;
		timerp->psx4t_value = uap->value;
		/* No need to reserve rtsig_event_t here */
		PROC_TIMER_UNLOCK(p);
		splx(s);
	}
#endif /* mips */

	itvp = uap->itv;

	/* 
	 * check access to input parameter
	 * return on error (no access)
	 */

	if (itvp && (error = copyin((caddr_t)itvp, (caddr_t)&aitv,
		sizeof(struct itimerval))))
		return (error);
	if ((uap->itv = uap->oitv) && (error = psx4_get_todtimer(p, uap, retval)))
		return (error);
	if (itvp == 0)
		return (0);

	/*
	 * if timeout value was specified as zero
	 *   cancel the current timer and return.
	 */

	if (!timerisset(&aitv.it_value)) {
		s = splhigh();
		PROC_TIMER_LOCK(p);
		timerp->psx4t_active = FALSE;
		timerclear(&timerp->psx4t_timeval.it_value);
		timerclear(&timerp->psx4t_timeval.it_interval);
		untimeout(psx4_tod_expire, (caddr_t) timerp);
		PROC_TIMER_UNLOCK(p);
		splx(s);
	} else {

		/*
		 * Grab the the proc and timer locks so no one can change
		 * timeout from under until the timers been armed.
		 */

		s = splhigh();
		PROC_TIMER_LOCK(p);
		TIME_READ_LOCK();

		/*
		 * Convert absolute timers to relative timers 
		 * to make life easier. Also, if the timeout time
		 * has already passed, set the timer to 0 for
		 * immediate expiration.
		 */
		if (uap->flags & TIMER_ABSTIME) {
			timevalsub(&aitv.it_value, &time);
		if (aitv.it_value.tv_sec < 0)
			aitv.it_value.tv_sec = aitv.it_value.tv_usec = 0;
		}

		/*
		 * POSIXIZE the timeout value
		 */
 
		if ((psx4_todtimer_fix(&aitv.it_value, &ticks)) ||
			(psx4_todtimer_fix(&aitv.it_interval, NULL))) {
			TIME_READ_UNLOCK();
			PROC_TIMER_UNLOCK(p);
			splx(s);
			return (EINVAL);
		}

		/*
		 * Cancel pending timeout if any
		 */

		if (timerp->psx4t_active)
			untimeout(psx4_tod_expire, (caddr_t) timerp);

		/*
		* Initialize Timer structure
		*/

		timevaladd(&aitv.it_value, &time);
		timerp->psx4t_active = TRUE;
		timerp->psx4t_timeval = aitv;
		timerp->psx4t_type = uap->flags;
		timerp->psx4t_p_proc = p;
		timerp->psx4t_overrun = -1;
		timeout(psx4_tod_expire, (caddr_t)timerp, ticks);
		TIME_READ_UNLOCK();
		PROC_TIMER_UNLOCK(p);
		splx(s);
	}
	return(0);
}

/*
 * POSIX 1003.4 REALTIME TIMER expiration:
 * Send the process whose timer expired the specified signal.
 * If time is not set up to reload, then just return.
 * Else compute next time timer should go off based on the current time
 * plus the specified interval.
 *
 * Assumes called on the master processor!  Locking probably doesn't work.
 *
 */

psx4_tod_expire(psx4_timer_t *timerp)
{
	register struct proc *p;
	struct timeval	rit_value;
	register int s, sig;

	sig = timerp->psx4t_signo;
	p = timerp->psx4t_p_proc;
	s = splhigh();
	PROC_TIMER_LOCK(p);

	/*
	 * if an interval wasn't specified or the
	 * timer has been canceled.  Clear the timevalues
	 * stored and return.
	 */
	if (!timerp->psx4t_active)
		goto psx4timer_out;

	if (!timerisset(&timerp->psx4t_timeval.it_interval)) {
#if RT_SIGNALS
		/* queue the signal */
#else
		psignal(p, sig);
#endif
		goto psx4timer_out;
	}

	/* 
	 * An interval was specified and the timer is active.
	 * Calculate the time until the next interval and queue
	 * another timeout.
	 */

#if RT_SIGNALS
	/* queue or bump overrun count */
#else
	psignal(p, sig);
#endif
	TIME_READ_LOCK();
	timerp->psx4t_timeval.it_value.tv_sec =
		timerp->psx4t_timeval.it_interval.tv_sec + time.tv_sec; 
	timerp->psx4t_timeval.it_value.tv_usec =
		timerp->psx4t_timeval.it_interval.tv_usec + time.tv_usec;
	TIME_READ_UNLOCK();
	rit_value = timerp->psx4t_timeval.it_value;
	timeout(psx4_tod_expire, (caddr_t)timerp, hzto(&rit_value));
	PROC_TIMER_UNLOCK(p);
	splx(s);
	return;

psx4timer_out:
	timerclear(&timerp->psx4t_timeval.it_value);
	timerclear(&timerp->psx4t_timeval.it_interval);
	timerp->psx4t_active = FALSE;
	PROC_TIMER_UNLOCK(p);
	splx(s);
	return;
}


/*
 * POSIX 1003.4 REALTIME TIMER validation and adjustment:
 *   Check that a proposed value to load into the .it_value or
 *   .it_interval part of an PSX4 timer is acceptable. Further,
 *   round the usec up to the next multiple of tick if it is
 *   not an integer multiple of the clock resolution.
 */
psx4_todtimer_fix(tv, ticks)
	struct timeval *tv;
	int *ticks;
{
	int ticktmp;

        /* 
         * Greater then maximum number of ticks, return an error
         */

	if ((tv->tv_sec < 0) || (tv->tv_sec + 1  > INT_MAX / hz) ||
	    (tv->tv_usec < 0) || (tv->tv_usec >= NSEC_PER_SEC))
		return (EINVAL);

	/*
	 * Round usecs up to next multiple of tick.
	 */

        if (tv->tv_usec > 0)
                tv->tv_usec = (tv->tv_usec + (tick - 1)) / tick * tick;  /* round up */
              
	/*
	 * Calculate the timeout in ticks.
	 */ 

	ticktmp = (tv->tv_sec * hz) + (((tv->tv_usec + tick - 1) * hz)
         		/ (1000*1000));

	/*
	 * Convert ticks back to a time value, effectively rounding
	 * up to the next multiple of the resolution, as required
	 * the P1003.4 D10 spec.
	 */

	tv->tv_sec = ticktmp / hz;
	tv->tv_usec = (ticktmp % hz) * tick;

	/*
	 * If ticks were requested, return them to caller.
	 */

	if (ticks)
		*ticks = ticktmp;

	return (0);
}

/*
 * POSIX 1003.4 REALTIME TIMER clear on exec()
 * Called from exec() to neutralize any POSIX.4
 * timers after they are inherited.
 */
int clear_psx4_timers(struct proc *p)
{
	int i, s;
	psx4_timer_t *tp;

	s = splhigh();
	PROC_TIMER_LOCK(p);
	
	if (p->p_psx4_timer == NULL)
		goto timer_clear_out;

	tp = p->p_psx4_timer->psx4_timers;
	for (i = 0; i < TIMER_MAX; i++, tp++)
		if (tp->psx4t_active) {
			tp->psx4t_active = 0;
			timerclear(&tp->psx4t_timeval.it_value);
			timerclear(&tp->psx4t_timeval.it_interval);
			untimeout(psx4_tod_expire, (caddr_t) tp);
		}

timer_clear_out:
	PROC_TIMER_UNLOCK(p);
	splx(s); 
	return(0);
}

/*
 * POSIX 1003.4 REALTIME TIMER clear on exit()
 * Called from exit() to neutralize any POSIX.4
 * timers and delete their storage.
 */
int exitrttimers(struct proc *p)
{
	int i, s;
	psx4_timer_t *tp;

	s = splhigh();
	PROC_TIMER_LOCK(p);

	/*
	 * If this proc never allocate timers, nothing to do.
	 */
	if (p->p_psx4_timer == NULL)
		goto timer_exit_out;

	/*
	 * Otherwise, walk the timer array and cancel any pending
	 * timeouts, then free the timer block.
	 */
	tp = p->p_psx4_timer->psx4_timers;
	for (i = 0; i < TIMER_MAX; i++, tp++)
		if (tp->psx4t_active)
			untimeout(psx4_tod_expire, (caddr_t) tp);
	zfree(rttimer_zone, (vm_offset_t) p->p_psx4_timer);

timer_exit_out:
	PROC_TIMER_UNLOCK(p);
	splx(s); 
	return(0);
}

/*
 * POSIX 1003.4 realtime timer allocation and decalltion routines:
 *	psx4_timer_t *psx4_tid_to_addr(p, tid): 
 *		translates timerid to addr; returns -1 if bad tid
 *	psx4_timer_t *psx4_timer_alloc(p, &tid):
 *		allocates timer entry, returned in tid;
 *		return timer addr; on failure, returns NULL, tid = -1
 *	int psx4_timer_dealloc(p, tid):
 *		deallocates tid; returns -1 is tid is already free
 */

psx4_timer_t *
psx4_tid_to_addr(struct proc *p, long tid)
{

	if (tid < 0 || tid >= TIMER_MAX || (p)->p_psx4_timer == NULL ||
		p->p_psx4_timer->psx4_timers[tid].psx4t_idx != TIMER_VALID)
		return NULL;
	else
		return p->p_psx4_timer->psx4_timers + tid;
}

psx4_timer_t *
psx4_timer_alloc(struct proc *p, long *r)
{
	register long i;

	if (p->p_psx4_timer->psx4tb_free < 0 ||
		p->p_psx4_timer->psx4tb_free >= TIMER_MAX || 
		p->p_psx4_timer == NULL) {
		*r = -1;
		return(NULL);
	}

	i = p->p_psx4_timer->psx4tb_free;
	p->p_psx4_timer->psx4tb_free =
		p->p_psx4_timer->psx4_timers[i].psx4t_idx;
	p->p_psx4_timer->psx4_timers[i].psx4t_idx = TIMER_VALID;
	*r = i;
	return(&p->p_psx4_timer->psx4_timers[i]);
}

int
psx4_timer_dealloc(struct proc *p, long tid)
{

	if (tid < 0 || tid >= TIMER_MAX || (p)->p_psx4_timer == NULL ||
		p->p_psx4_timer->psx4_timers[tid].psx4t_idx != TIMER_VALID)
		return(-1); 
	p->p_psx4_timer->psx4_timers[tid].psx4t_idx = 
		p->p_psx4_timer->psx4tb_free;
	p->p_psx4_timer->psx4tb_free = tid;
	return(0);
}

#else  /* stub entry points. */ 

psx4_set_todtimer(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (ENOSYS);
}

psx4_get_todtimer(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (ENOSYS);
}

int
psx4_timer_create(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (ENOSYS);
}

int
psx4_timer_delete(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (ENOSYS);
}

#endif /* RT_TIMER */

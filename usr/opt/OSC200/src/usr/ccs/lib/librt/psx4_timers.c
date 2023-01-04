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
static char	*sccsid = "@(#)$RCSfile: psx4_timers.c,v $ $Revision: 4.2.4.5 $ (DEC) $Date: 1992/11/06 13:48:35 $";
#endif
/*
 * @DIGITALCOPYRIGHT@
 */
/*****************************************************************************
 *++
 *
 * Facility:
 *
 *	ULTRIX POSIX 1003.4/D10 Clocks and Timers run-time library routines
 *
 * Abstract:
 *
 *	This module contains the routines to manipulate
 *	system-wide and per-process timers:
 *
 *	Clock Routines:
 *		clock_gettime()	clock_settime()	clock_getres()
 *		clock_getdrift() clock_setdrift()
 *
 *	Timer Routines:
 *		timer_create()	timer_delete()	timer_gettime()
 *		timer_settime	timer_getres()
 *
 *	Sleep Routines:
 *		nanosleep_getres() nanosleep()
 *
 *	Utility Routines:
 *		tod_timer_res()
 *
 * Author:
 *
 *
 *
 * Modified by:
 *
 *   modifier's name,	dd-mmm-yyyy,  VERSION: svv.u-ep
 *   03 - modification description
 *
 *   Jeff Denham,	04-Feb-1992,  VERSION: V1.1-02
 *   02 - Adapt 01 changes to do timer_create() and timer_delete()
 *	  in the kernel. This involved removed most of the timer creation
 *	  and validation code from the library.
 *
 *   01 - Paula Long	06-Jan-1992,	VERSION: V1.1-01
 *	  Update to D11 of the standard:
 *	  1. clock_gettimedrift() and clock_settimedrift() routines
 *	     named changed to clock_getdrift() and clock_setdrift().
 *	  2. Maxvalue argument was removed from clock_getres()
 *	     add a clock_getres_d10() for compatiblity
 *	  3. Noted obsolete routines (timer_getres(), nanosleep_getres().)
 *	  4. nanosleep() function no longer returns a 1 if the sleep was
 *	     interrupted (-1 and errno == EINTR).
 *
 * Notes --
 *	1. timer_getoverrun - is not currently implemented.
 *	2. locking right now is gross.	it's need to be done a lower
 *	   grandularity.  Still need the threads package to figure
 *	   out what the locks should look like.
 *--
 */
#undef POSIX_4D10
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>
#include <machine/machtime.h>
#include <sys/errno.h>

/*
 *
 */
#undef clock_gettimedrift
#undef clock_settimedrift
#undef nanosleep_getres
#undef timer_getres

/*
 * Constants and Macros
 */

#define SUCCESS	 0
#define FAILURE -1
#define TRUE  1
#define FALSE 0

#define	TIMER_LOCK()
#define	TIMER_UNLOCK()
#define	SETERR(err)	errno = err

#define PSX4_GETTIMEDRIFT 0
#define PSX4_SETTIMEDRIFT 1
#define INIT_TIMERID_TOD(clock_id, i)  	  \
{						  \
	timerid_available[i].allocated = TRUE;		\
	timerid_available[i].clock_id = clock_id;	\
	timerid_available[i].index = i;			\
	timerid_available[i].abs = 0;			\
	timerid_available[i].timer_set = psx4_set_todtimer;	\
	timerid_available[i].timer_get = psx4_get_todtimer;	\
	timerid_available[i].timer_del = psx4_timer_delete;	\
	timerid_available[i].timer_res = tod_timerres;		\
	timerid_available[i].scale_factor = NSEC_PER_USEC;	\
}

#define VALID_CLOCK_TYPE(timeraddr) (timeraddr->clock_id == CLOCK_REALTIME)

/*
 * Data Structure Definitions
 */

/*
 * Timerid structure.
 */

struct timer_id_struct {
  	int allocated;
  	int clock_id;
  	timer_t index;
  	int abs;
  	int signo;
	sigval_t value;
  	int scale_factor;
  	int (*timer_set)();
  	int (*timer_get)();
  	int (*timer_del)();
  	int (*timer_res)();
	};

/*
 * Static data
 */

static struct timer_id_struct timerid_available[TIMER_MAX];
static int timerid_avail_init = 0;

/*
 * Table of Contents
 */
int clock_gettime();		/* get the values of the specified clock	   */
int clock_settime();		/* set the time of the specified clock	   */
int clock_getres();		/* get the resolution of the specified clock */
int clock_getdrift();		/* get the clock drift rate		     */
int clock_setdrift();		/* set the clock drift rate		     */
timer_t timer_create();		/* create a per-process timer		   */
int timer_delete();		/* delete a per-process timer		   */
int timer_settime();		/* arm a per-process timer		   */
int timer_gettime();		/* get the value of a per-process timer	   */
int timer_getoverrun();		/* get overrun count on specified timer	   */
int nanosleep();		/* high resolution sleep			   */
int tod_timerres();		/* get the resolution of the CLOCK_REALTIME timer */

/*
 * Obsolete POSIX 1003.4/D10 functions:
 */
int timer_getres();		/* get resolution and maxval of timer        */
int nanosleep_getres();		/* get resolution supported by nanosleep()   */
int clock_getres_d10();		/* get clock res and maxvalue                */

/*
 * Library traps into syscalls:
 */
extern int psx4_set_todtimer();	/* gettimer syscall for CLOCK_REALTIME clock */
extern int psx4_get_todtimer();	/* settimer syscall for CLOCK_REALTIME clock */
extern int usleep_thread();	/* thread sleep syscall			     */
extern int psx4_time_drift();	/* set/get drift syscall		     */
extern timer_t psx4_timer_create(); /* timer creation syscall 		     */
extern int psx4_timer_delete(); /* timer deletion syscall 		     */

/*++
 *
 * clock_gettime()
 *
 * Functional description:
 *
 *	This routine returns the current time for the system-wide clock
 *	specified in the argument clock_type.  For the CLOCK_REALTIME clock,
 *	tp receives the number of seconds and nanoseconds since the Epoch.
 *
 *	Currently only the CLOCK_REALTIME clock_type is supported.
 *
 * Inputs:
 *
 *	clock_type - a system-wide clock, such as CLOCK_REALTIME
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	tp - current time, such as number of seconds and nanoseconds
 *	     since the Epoch for the clock_type CLOCK_REALTIME
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */
int clock_gettime(int clock_id, struct timespec *tp)
{
  if (!tp) {
    SETERR(EINVAL);
    return(FAILURE);
  }
  
  /*
   * Only clock type CLOCK_REALTIME is currectly supported.
   *
   * Note: gettimeofday() returns time in terms of seconds and
   *	 micro-seconds.	 The micro-seconds are converted to
   *	 nanoseconds as per the P1003.4/D10 standard
   */

  switch (clock_id) {
  case CLOCK_REALTIME:
    {
      struct timeval tval;
      if (gettimeofday(&tval, (struct timezone *)0) == SUCCESS) {
	tp->tv_sec = tval.tv_sec;
	tp->tv_nsec = tval.tv_usec * NSEC_PER_USEC;
	return(SUCCESS);
      }
      return(FAILURE);
    }
  default:
    SETERR(EINVAL);
    return(FAILURE);
  }
}

/*++
 *
 * clock_settime()
 *
 * Functional description:
 *
 *	This routine sets the time for the systemwide clock.  When
 *	clock_type is CLOCK_REALTIME, system time is set to the number of
 *	nanoseconds and seconds from the Epoch as specified in tp.
 *
 *	Currently only the CLOCK_REALTIME clock_type is supported.
 *
 * Inputs:
 *
 *	clock_type - system-wide clock, such as CLOCK_REALTIME
 *	tp - number of nanoseconds and seconds from the Epoch
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	None
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */
int clock_settime(int clock_id, struct timespec *tp)
{

	struct timeval new_time = {0,0};
	struct timespec max, res;
	int status;

	if (!tp) {
		SETERR(EINVAL);
		return(FAILURE);
	}

	switch (clock_id) {
		case CLOCK_REALTIME:
			status = clock_getres_d10(clock_id,&res,&max);
			if ((tp->tv_sec > max.tv_sec)
				||(tp->tv_nsec > max.tv_nsec)
				||(tp->tv_sec < 0)
				||(tp->tv_nsec < 0)) {
				SETERR(EINVAL);
				return(FAILURE);
			}
		/*
		 * Set the time. P1003.4/D11 sets the time in terms of seconds
		 * and nanoseconds; settimeofday expects units of seconds
		 * and microseconds, so do the conversion rounding up.
		 */
			new_time.tv_sec = tp->tv_sec;
			new_time.tv_usec = (tp->tv_nsec + NSEC_PER_USEC - 1) /
				NSEC_PER_USEC;
			return(settimeofday(&new_time, (struct timezone *) 0));
		default:
			break;
	}
	SETERR(EINVAL);
	return(FAILURE);
}

/*++
 *
 * clock_getres()
 *
 * Functional description:
 *
 *	This routine returns the resolution and the maximum
 *	value for a specified system-wide clock.
 *
 *	Currently only the CLOCK_REALTIME clock_type is supported.
 *
 * Inputs:
 *
 *	clock_type - system-wide clock, such as CLOCK_REALTIME
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	res - resolution of the specified system-wide clock
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */

int clock_getres(int clock_id, struct timespec *res)
{
	return(clock_getres_d10(clock_id,res,NULL));
}

/*++
 *
 * clock_getres_d10()
 *
 * Functional description:
 *
 *	This routine returns the resolution and the maximum
 *      value for a specified system-wide clock.
 *
 *      Currently only the CLOCK_REALTIME clock_type is supported.
 *
 * Inputs:
 *
 *	clock_type - system-wide clock, such as CLOCK_REALTIME
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	res - resolution of the specified system-wide clock
 *      maxvaxl - maximum value that can be passed to setclock()
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */


int clock_getres_d10(int clock_id, struct timespec *res, 
		     struct timespec *maxval)
{
  switch (clock_id) {
    
    /*
     * ticks per second = (hz)
     * nano_res = (ticks  per second) * nano_seconds_per_second
     */

  case CLOCK_REALTIME:
    {
    int result, speed;
    int start = 0;
    if (res) {
      result = getsysinfo(GSI_CLK_TCK, (void *)&speed, 
			  sizeof(speed), &start, NULL);
      res->tv_sec = 0;
      if (result > 0)
	res->tv_nsec = NSEC_PER_SEC/speed;
      else
	res->tv_nsec = NSEC_PER_SEC/CLOCKS_PER_SEC;
    }
    if (maxval) {
      maxval->tv_sec = INT_MAX;
      maxval->tv_nsec = NSEC_PER_SEC - 1;
    }
    return(SUCCESS);
  }
  default:
    SETERR(EINVAL);
    return(FAILURE);
  }
}

/*++
 *
 * clock_getdrift()
 *
 * Functional description:
 *
 *	This routine returns the current drift value applied to the specifed
 *	clock.
 *
 *	Currently only the CLOCK_REALTIME clock_type is supported.
 *
 * Inputs:
 *
 *	clock_type - a system-wide clock, such as CLOCK_REALTIME
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	oppb - drift applied in parts per billion
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */
int clock_getdrift(int clock_id, int *oppb)
{
	switch (clock_id) {
		case CLOCK_REALTIME:
			if((psx4_time_drift(PSX4_GETTIMEDRIFT, 0, oppb)) !=
					SUCCESS)
		 		return(FAILURE);
	    		return(SUCCESS);
		default:
		    	break;
  }
	SETERR(EINVAL);
	return(FAILURE);
}
/*++
 *
 * clock_setdrift()
 *
 * Functional description:
 *
 *	This routine sets the current drift value applied to the specifed
 *	clock.
 *
 *	Currently only the CLOCK_REALTIME clock_type is supported.
 *
 * Inputs:
 *
 *	clock_type - a system-wide clock, such as CLOCK_REALTIME
 *	ppb	   - drift value applied to specified clock in parts in billion
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	oppb - drift applied in parts per billion on pervious call
 *
 * Implicit outputs/side effects:
 *
 *	Time skewed to reflect the drift.
 *--
 */
int clock_setdrift(int clock_id, const int ppb, int *oppb)
{
	switch (clock_id) {
		case CLOCK_REALTIME:
			if(psx4_time_drift(PSX4_SETTIMEDRIFT, ppb, oppb) != SUCCESS)
		  		return(FAILURE);
			return(SUCCESS);
		default:
			break;
	}
	SETERR(EINVAL);
	return(FAILURE);
}

/*++
 *
 * timer_create()
 *
 * Functional description:
 *
 *	This routine establishes a per-process timer of the type
 *	clock_type.  The function returns a timer ID which is used
 *	by other timer functions such as gettimer().  The timer ID
 *	will actually be the address of the timerid structure.
 *
 *	Currently only the CLOCK_REALTIME clock_type is supported.
 *
 * Inputs:
 *
 *	clock_type - a systemwide timer, such as CLOCK_REALTIME
 *	evp	   - structure that contain the signal to raise on
 *		     expiration and the data to deliver if REALTIME SIGNALS
 *		     are supported.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	Returns the address of the block as the timerid
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */

timer_t timer_create(int clock_id, struct sigevent *evp)
{
	register timer_t timerid;

	/*
	 * if an event pointer is specified and the
	 * desired signal is > the maximum valid signal
	 * return and error.  POSIX doesn't have an error
	 * value for this case!!!  Vaild signals range from
	 * 1-SIGMAX (32 signals)
	 */

	if (evp != NULL) {
		if ((evp->sigev_signo < 0) || (evp->sigev_signo > SIGMAX)) {
			SETERR(EINVAL);
			return(FAILURE);
		}
	}

	/*
	 * Only the CLOCK_REALTIME clocks is currently supported.
	 * Attempt to allocate one of the available timerids
	 *  if one is availible allocate it and initalize it.
	 *  else return an error
	 */
	switch (clock_id)  {
		case CLOCK_REALTIME:
			TIMER_LOCK();
			if (timerid_avail_init != getpid()) {
				timerid_avail_init = getpid();
				memset(timerid_available,'\0',
                         		sizeof(timerid_available));
                	}
  			timerid = psx4_timer_create(clock_id, evp);
			if (timerid == -1) {
				return(FAILURE);
			}
			INIT_TIMERID_TOD(clock_id, timerid);
			TIMER_UNLOCK();
			break;
		default:	/* not CLOCK_REALTIME timer */
			SETERR(EINVAL);
			return(FAILURE);
	}
	return(timerid);
}

/*++
 *
 * timer_delete()
 *
 * Functional description:
 *
 *	This routine releases a previously-allocated per-process timer.
 *	If the timer is pending, it is canceled.  The timer is then marked
 *	as deallocated.
 *
 * Inputs:
 *
 *	timerid - address of timerid structure as returned by timer_create()
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	None
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */

int timer_delete(timer_t timerid)
{
	struct timer_id_struct *timeraddr;
	int status = 0;

	if (timerid < 0 || timerid >= TIMER_MAX) {
		SETERR(EINVAL);
		return(FAILURE);
	}
	TIMER_LOCK();
	timeraddr = &timerid_available[timerid];

	/*
	 * Invoke the delete function.
	 */

	status = (timeraddr->timer_del)(timerid);
	if (!status) {
		timeraddr->allocated = FALSE;
		TIMER_UNLOCK();
		return(SUCCESS);
	} else {
		TIMER_UNLOCK();
		return(status);
	}
}

/*++
 *
 * timer_gettime()
 *
 * Functional description:
 *
 *	Get the value of Per-Process timer
 *
 * Inputs:
 *
 *	timerid - address of timerid structure , as returned by timer_create()
 *	value - value.it_value is the value to which to set the timer
 *		expiration.  value.it_interval is the value to be used
 *		when reloading the timer.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	ovalue - remaining time (or zero) in value.it_value,
 *		 and repetition value last set.
 *
 * Implicit outputs/side effects:
 *
 *	None
 *--
 */

int timer_gettime(timer_t timerid, struct itimerspec *value)
{
	struct timer_id_struct *timeraddr;
	struct itimerval kvalue;

	if (timerid < 0 || timerid >= TIMER_MAX) {
		SETERR(EINVAL);
		return(FAILURE);
	}

	timeraddr = &timerid_available[timerid];
	TIMER_LOCK();
	if ((timeraddr->allocated != TRUE) || (value == NULL)) {
		SETERR(EINVAL);
		TIMER_UNLOCK();
		return(FAILURE);
	}

	/*
	 * Invoke the get_timer function assoicated with the supplied
	 * timerid.  Note the get_timer function needs to ensure that value
	 * is accessible otherwise it's possible to take a sigvec with the
	 * the TIMER_LOCK held.
	 */
	if ((timeraddr->timer_get(timeraddr->index, &kvalue)) != SUCCESS) {
		TIMER_UNLOCK();
		return(FAILURE);
	}

	/*
	 * Transfer kernel's itimerval entries back to itimerspecs for
	 * caller.
	 */
	value->it_value.tv_sec = kvalue.it_value.tv_sec;
	value->it_value.tv_nsec =
		kvalue.it_value.tv_usec * timeraddr->scale_factor;

	value->it_interval.tv_sec = kvalue.it_interval.tv_sec;
	value->it_interval.tv_nsec =
		kvalue.it_interval.tv_usec * timeraddr->scale_factor;
	TIMER_UNLOCK();
	return(SUCCESS);
}

/*++
 *
 * timer_settime()
 *
 * Functional description:
 *
 *	Start a per-process timer
 *
 * Inputs:
 *
 *	timerid - address of timerid structure , as returned by timer_create()
 *	value - value.it_value is the value to which to set the timer
 *		expiration.  value.it_interval is the value to be used
 *		when reloading the timer.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	ovalue - remaining time (or zero) in value.it_value,
 *		 and repetition value last set.
 *
 * Implicit outputs/side effects:
 *
 *	None
 *--
 */

int timer_settime(timer_t timerid, int flags, struct itimerspec *value,
		   struct itimerspec *ovalue)
{
	struct timespec res, max;
	struct itimerspec temp;
	struct timer_id_struct *timeraddr;
	struct itimerval ktemp, kovalue, *kop = NULL;
	int status;

	if (timerid < 0 || timerid >= TIMER_MAX) {
		SETERR(EINVAL);
		return(FAILURE);
	}

	/*
	 * Range check the nsecs fields.
	 */
	if ((value->it_value.tv_nsec > NSEC_PER_SEC-1) ||
			(value->it_value.tv_nsec < 0) ||
			(value->it_interval.tv_nsec > NSEC_PER_SEC-1) ||
			(value->it_interval.tv_nsec < 0)) {
		SETERR(EINVAL);
		return(FAILURE);
	}

	TIMER_LOCK();
	timeraddr = &timerid_available[timerid];
	if ((timeraddr->allocated != TRUE) || (value == NULL)) {
		SETERR(EINVAL);
		TIMER_UNLOCK();
		return(FAILURE);
	}

	/*
	 * Transfer the POSIX.4 time values into the itimerval
	 * structure the kernel expects, converting nsecs to usecs.
	 */
	ktemp.it_value.tv_sec = value->it_value.tv_sec;
	ktemp.it_value.tv_usec = (value->it_value.tv_nsec + /* round up */
		(timeraddr->scale_factor - 1)) / timeraddr->scale_factor;

	ktemp.it_interval.tv_sec = value->it_interval.tv_sec;
	ktemp.it_interval.tv_usec = (value->it_interval.tv_nsec + /* round up */
		(timeraddr->scale_factor - 1)) / timeraddr->scale_factor;

	/*
	 * If ovalue present, pass the address of the local itimerval for
	 * use by the kernel.
	 */
	if (ovalue)
		kop = &kovalue;

	/*
	 * Set the abs/rel timer flag.
	 */
	timeraddr->abs = flags & TIMER_ABSTIME;

	/*
	 * Invoke set_timer function
	*/
	status = (timeraddr->timer_set) (timeraddr->index,  &ktemp, kop,
		timeraddr->abs, timeraddr->signo, timeraddr->value);
	TIMER_UNLOCK();

	/*
	 * Convert kernel's returned ovalue to the POSIX.4 version.
	 */
	if (ovalue) {
		ovalue->it_value.tv_sec = kovalue.it_value.tv_sec;
		ovalue->it_value.tv_nsec =
			kovalue.it_value.tv_usec * timeraddr->scale_factor;
		ovalue->it_interval.tv_sec = kovalue.it_interval.tv_sec;
		ovalue->it_interval.tv_nsec =
			kovalue.it_interval.tv_usec * timeraddr->scale_factor;
	}
		
	return(status);
}

/*++
 *
 * timer_getres()
 *
 * Functional description:
 *
 *	This routine returns the resolution and the maximum value
 *	of the specified timer.	 The resolution is the smallest
 *	meaningful time that can be supplied as an argument to
 *	the reltimer() or abstimer() functions.	 The max value equals
 *	the maximum value that can be supplied to the abstimer() function.
 *
 * Inputs:
 *
 *	timerid - address of timerid structure , as returned by timer_create()
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	res - resolution
 *	max - maximum value
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */

int timer_getres(timer_t timerid, int abstime, struct timespec *res,struct timespec *maxval)
{
	struct timer_id_struct *timeraddr;
	int status;

	if (timerid < 0 || timerid >= TIMER_MAX) {
		SETERR(EINVAL);
		return(FAILURE);
	}

	timeraddr = &timerid_available[timerid];
	TIMER_LOCK();

	/*
	 * Invoke the get_res function associated with the specified timerid.
	 */

	status = (timeraddr->timer_res) (abstime, res, maxval);
	TIMER_UNLOCK();
	return(status);
}

/*++
 *
 * timer_getoverun()
 *
 * Functional description:
 *
 *	This routine returns the overrun count of the specified per-process
 *	timer.
 *
 *	Currently only the CLOCK_REALTIME clock_type is supported.
 *
 * Inputs:
 *
 *	timerid	 - handle to the pending timeout.
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	number of intervals which have expired since the timer timed out.
 *
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 * Note:  This function is not supported currently.
 *--
 */
int timer_getoverrun(timer_t timerid)
{
	SETERR(ENOSYS);
	return(FAILURE);
}

/*++
 *
 * nanosleep_getres()
 *
 * Functional description:
 *
 *	This routine returns the resolution and maximum value supported
 *	by the nanosleep() function.  The resolution is the smallest
 *	meaningful time that can be supplied as an argument to the
 *	nanosleep() function.  The max value equals the maximum time
 *	value that can be supplied to the nanosleep() function.
 *
 * Inputs:
 *
 *	None
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	res - resolution
 *	max - maximum value
 *
 * Implicit outputs/side effects:
 *
 *	None
 *
 *--
 */

int nanosleep_getres(struct timespec *res, struct timespec *maxval)
{

	if (clock_getres_d10(CLOCK_REALTIME, res, maxval) == FAILURE)
		return(FAILURE);
	if (maxval)
		maxval->tv_sec = TOD_MAX_SECONDS;
	return(SUCCESS);
}

/*++
 *
 * nanosleep()
 *
 * Functional description:
 *
 *	This routine performs a high resolution sleep.	It results in the
 *	calling process entering the wait state for the amount of time
 *	specified in rqtp.  The process remains in the wait state until
 *	a signal or an event is received, or until the time expires.
 *
 * Inputs:
 *
 *	rqtp - the amount of time to sleep, expressed as seconds and
 *	       nanoseconds
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *	rmtp - the amount of unslept time, if the process is
 *	       awoken from its sleep state by a signal or event
 *
 * Implicit outputs/side effects:
 *
 *
 *	None
 *--
 */

int nanosleep(struct timespec *rqtp, struct timespec *rmtp)
{
	struct timeval temp;
	struct timeval time_remaining;

	if (rqtp) {
		if ((rqtp->tv_nsec > NSEC_PER_SEC-1) ||
			(rqtp->tv_nsec < 0 )) {
			SETERR(EINVAL);
			return(FAILURE);
		}
		temp.tv_sec = rqtp->tv_sec;
		temp.tv_usec = (rqtp->tv_nsec + NSEC_PER_USEC -1) / NSEC_PER_USEC;
		if (usleep_thread(&temp, &time_remaining ) == FAILURE)
			return(FAILURE);

		if (rmtp) {
	 		rmtp->tv_sec =	time_remaining.tv_sec;
	 		rmtp->tv_nsec = time_remaining.tv_usec * NSEC_PER_USEC;
		}
		if ((time_remaining.tv_sec > 0) ||
				(time_remaining.tv_usec > 0)) {
			SETERR(EINTR);
			return(FAILURE);
		} else
	    		return(SUCCESS);
	}
	SETERR(EINVAL);
	return(FAILURE);
}

/*++
 *
 * tod_timerres
 *
 * Functional description:
 *
 *	Get the resolution of the Realtime Clock.
 *
 * Inputs:
 *
 *	abstime: Type (0 = REL, 1 = ABS)
 *
 * Implicit inputs:
 *
 *	None
 *
 * Outputs:
 *
 *
 *	res - resolution
 *	max - maximum value
 *
 * Implicit outputs/side effects:
 *
 *
 *	None
 *--
 */

int tod_timerres(int abstime, struct timespec *res, struct timespec *maxval)
{
  struct timespec time;
  int result, speed;
  int start = 0;

  if (clock_getres_d10(CLOCK_REALTIME, res, maxval) == FAILURE)
    return(FAILURE);

  if (maxval) {
    if (clock_gettime(CLOCK_REALTIME, &time) == FAILURE)
      return(FAILURE);
    result = getsysinfo(GSI_CLK_TCK, (void *)&speed, sizeof(speed), 
			&start, NULL);
    if (result > 0)
      maxval->tv_sec = INT_MAX / speed;
    else
      maxval->tv_sec = INT_MAX / CLOCKS_PER_SEC;
    if (abstime) {
      if ((maxval->tv_sec = maxval->tv_sec + time.tv_sec) > INT_MAX)
	maxval->tv_sec = INT_MAX;
    }	
    else {
      if ((maxval->tv_sec + time.tv_sec) > INT_MAX)
	maxval->tv_sec = INT_MAX - time.tv_sec;
    }
  }
  return(SUCCESS);
}

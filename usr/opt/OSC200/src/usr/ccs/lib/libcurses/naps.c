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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: naps.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 22:04:51 $";
#endif
/*
 * HISTORY
 */
/*** "naps.c  1.9  com/lib/curses,3.1,9008 12/4/89 21:02:09"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   napms, nap, napx, sleepnap
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Code for various kinds of delays.  Most of this is nonportable and
 * requires various enhancements to the operating system, so it won't
 * work on all systems.  It is included in curses to provide a portable
 * interface, and so curses itself can use it for function keys.
 */


#include "cursesext.h"
#include <signal.h>

#define NAPINTERVAL 100

#ifdef TIOCREMOTE

#include <sys/time.h>

/*
 * NAME:        napms
 *
 * FUNCTION:
 *
 *      Sleep for ms milliseconds.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Don't expect a particularly good
 *      resolution - 60ths of a second is normal, 10ths might even be good
 *      enough, but the rest of the program thinks in ms because the unit of
 *      resolution varies from system to system.  (In some countries, it's
 *      50ths, for example.)
 *
 *      Here are some reasonable ways to get a good nap.
 *
 *      (1) Use the select system call in Berkeley 4.2BSD.
 *
 *      (2) Use the 1/10th second resolution wait in the UNIX 3.0 tty
 *          driver.  It turns out this is hard to do - you need a tty
 *          line that is always unused that you have read permission on to
 *          sleep on.
 *
 *      (3) Install the ft (fast timer) device in your kernel.
 *          This is a psuedo-device to which an ioctl will wait n ticks
 *          and then send you an alarm.
 *
 *      (4) Install the nap system call in your kernel.
 *          This system call does a timeout for the requested number of
 *          ticks.
 *
 *      (5) Write a routine that busy waits checking the time with ftime.
 *          Ftime is not present on USG systems, and since this busy waits,
 *          it will drag down response on your system.  But it works.
 *
 *       Otherwise, pause for ms milliseconds.  Convert to ticks and wait
 *       that long.  Call nap, which is either defined below or a system
 *       call.
 */

napms(ms)
int ms;
{
	struct timeval t;

	/*
	 * If your 4.2BSD select still rounds up to the next higher second,
	 * you should remove this code and install the ft driver.
	 * This routine was written under the assumption that the problem
	 * would be corrected by 4.2BSD.
	 */
	t.tv_sec = ms/1000;
	t.tv_usec = 1000 * (ms % 1000);
	select(0, 0, 0, 0, &t);
	return OK;
}
#else /* TIOCREMOTE */
napms(ms)
int ms;
{
	int ticks;
	int rv;

	ticks = ms / (1000 / HZ);
	if (ticks <= 0)
		ticks = 1;
	rv = nap(ticks);  /* call either the code below or nap system call */
	return rv;
}

#ifdef FTIOCSET
#define HASNAP

#include <setjmp.h>
static jmp_buf jmp;
static int ftfd;

/*
 * NAME:        nap
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The following code is adapted from the sleep code in libc.
 *      It uses the "fast timer" device posted to USENET in Feb 1982.
 *      nap is like sleep but the units are ticks (e.g. 1/60ths of
 *      seconds in the USA).
 */

static int
nap(n)
unsigned n;
{
	int napx();
	unsigned altime;
	void (*alsig)() = SIG_DFL;
	char *ftname;
	struct requestbuf {
		_SHORT time;
		_SHORT signo;
	} rb;

	if (n==0)
		return OK;
	if (ftfd <= 0) {
		ftname = "/dev/ft0";
		while (ftfd <= 0 && ftname[7] <= '~') {
			ftfd = open(ftname, 0);
			if (ftfd <= 0)
				ftname[7] ++;
		}
	}
	if (ftfd <= 0) {	/* Couldn't open a /dev/ft? */
		sleepnap(n);
		return ERR;
	}
	altime = alarm(1000);	/* time to maneuver */
	if (setjmp(jmp)) {
		signal(SIGALRM, (void (*)(int))alsig);
		alarm(altime);
		return OK;
	}
	if (altime) {
		if (altime > n)
			altime -= n;
		else {
			n = altime;
			altime = 1;
		}
	}
	alsig = signal(SIGALRM, (void (*)(int))napx);
	rb.time = n;
	rb.signo = SIGALRM;
	ioctl(ftfd, FTIOCSET, &rb);
	for(;;)
		pause();
}

/*
 * NAME:        napx
 */

static
napx()
{
	longjmp(jmp, 1);
}
#endif

/*
 * NAME:        nap
 *
 * EXECUTION ENVIRONMENT:
 *
 *      If you have some other externally supplied nap(), add -DHASNAP to
 *      cflags.
 */

#ifndef HASNAP
static	int
nap(ms)
int ms;
{
	sleep((ms+999)/1000);
	return ERR;
}
#endif

/*
 * NAME:        sleepnap
 *
 * FUNCTION:
 *
 *      Simulate nap with sleep.
 */

static
sleepnap(ticks)
{
	sleep((ticks+(HZ-1))/HZ);
}

#endif /* TIOCREMOTE */

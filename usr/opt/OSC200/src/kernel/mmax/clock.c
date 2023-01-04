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
static char	*sccsid = "@(#)$RCSfile: clock.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:52 $";
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
 * OSF/1 Release 1.0
 */
/*
 *        Copyright 1985 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Module Function:
 * 	This module contains the machine dependent clock functions.
 *
 * Original Author: Tony Anzelmo	Created on: 85/04/29
 */

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

#include <simple_clock.h>
#include <stat_time.h>
#include <cpus.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <mach/boolean.h>

#include <mmaxio/crqdefs.h>
#include <mmax/sccdefs.h>
#include <mmax/cpudefs.h>
#include <mmax/tse.h>
#include <mmax/cpu.h>
#include <mmax/isr_env.h>

#include <vm/vm_kern.h>
#include <mach/vm_param.h>

extern	int	hz;		/* System clock interrupt rate */
extern	struct	timeval	time;	/* Current system time */
extern	int	maxdriftrecip;	/* Maximum drift rate */

unsigned int	Last_frcount[NCPUS];

/*
 *	Inittodr initializes the time of day hardware which provides
 *	date functions.  Its primary function is to use some file
 *	system information in case the hardare clock lost state.
 *
 *	Initialze the time of day register, based on the time base which is,
 *	e.g. from a filesystem.  Base provides the time to within six months,
 *	and the time of year clock provides the rest.
 *
 *	base		- Time base
 */
inittodr(base)
	time_t base;
{
	time_t	toy_secs;		/* place for toy time converted to */
					/* the number of secs since 1970 */
	int	fs_bogus = FALSE;
	int	toy_bogus = FALSE;
	int	s;
/*
 * Defines for time conversion.
 */
#define SEC_PER_MIN	60
#define SEC_PER_HOUR	(SEC_PER_MIN * 60)
#define SEC_PER_DAY	(SEC_PER_HOUR * 24)
#define SEC_PER_MONTH	(SEC_PER_DAY * 30)
#define SEC_PER_YEAR	(SEC_PER_DAY * 365)
#define LEAP_YEAR(year)		(((year) % 4 == 0 && (year) % 100 != 0) || \
				 (year) % 400 == 0)
#define DAYS_PER_YEAR(year)	((LEAP_YEAR (year)) ? 366 : 365)

/*
 * Setup to use the time from the super block as a last resort if the toy
 * clock time is bogus.
 */
	s = splhigh();
	TIME_WRITE_LOCK();
	time.tv_sec = base;
	time.tv_usec = NULL;
	TIME_WRITE_UNLOCK();
	splx(s);
/*
 * Validate that the time in the super block is somewhat reasonable.
 * Must be at least 1985. If not we set a base of mid-1985.
 */
	if (base < (15 * SEC_PER_YEAR)) {
		printf("WARNING: preposterous time in file system\n");
		s = splhigh();
		TIME_WRITE_LOCK();
		time.tv_sec = ((15 * SEC_PER_YEAR) + (186 * SEC_PER_DAY) 
			+ (SEC_PER_DAY / 2));
		TIME_WRITE_UNLOCK();
		splx(s);
		fs_bogus = TRUE;
	}
/*
 * Read the toy clock.
 */
	get_timeofyear (&toy_secs);	
	if (toy_secs < base) {
		printf("Time of year clock is out of date.\n");
		toy_bogus = TRUE;
	}
	else if ((!fs_bogus) && (toy_secs - base) > (6 * SEC_PER_MONTH)) {
		printf("Time of Year clock is more than 6 months\n");
		printf("ahead of the time in the root filesystem.\n");
		toy_bogus = TRUE;
	} else {
		s = splhigh();
		TIME_WRITE_LOCK();
		time.tv_sec = toy_secs;
		TIME_WRITE_UNLOCK();
		splx(s);
	}
	if (toy_bogus)
		resettodr();
	if (toy_bogus || fs_bogus)
		printf(" -- CHECK AND RESET THE DATE!\n");
}

resettodr()
{
	time_t	secs;
	int	s;

	s = splhigh();
	TIME_READ_LOCK();
	secs = time.tv_sec;
	TIME_READ_UNLOCK();
	splx(s);

/*
 * Send the time to the toy clock on the scc.
 */
	set_timeofyear (secs);
}

/*
 * Offsets from interrupt frame pointer (sp) of PC and PS
 * at point of interrupt.
 */
#if STAT_TIME
#define FRAME_PC	10	/* get past usp + r7 thru r0 + fp */
#define FRAME_PS	11
#else
#define FRAME_PC	11	/* get past timer value + above */
#define FRAME_PS	12
#endif

/*
 * tseintr - time-slice-end interrupt processing -- we assume sp
 * points to an interrupt frame.
 */
tseintr(ihp, sp)
ihandler_t *ihp;
long *sp;
{
#if	MMAX_DPC
	tsereld();
#endif
#if	!SIMPLE_CLOCK
	Last_frcount[cpu_number()] = FRcounter;  /* for microtime */
#endif
	hardclock(sp[FRAME_PC], sp[FRAME_PS]);
}


/*
 * startrtclock - time-slice-end processing initialization.  This provides
 *	clock interrupts to both the master and slaves.
 */
startrtclock()
{
#if	MMAX_DPC
	static int tsevec_allocated = 0;
	static dpcsendvec_t tse_vector;
#endif

#if	MMAX_DPC
	/*
	 * If this is the first time this routine is being called, then
	 * allocate an interrupt vector for the time-slice-end. For all
	 * CPUs, save the vector number of the allocated vector in the
	 * CPU's private register.
	 */

	if (!tsevec_allocated) {
		if(alloc_vector(&tse_vector, tseintr, 0, INTR_HARDCLK))
			panic ("startrtclock: unable to allocate vector\n");
		tsevec_allocated = 1;
	}
	*((char *)DPCREG_TSEVECWRITE) = tse_vector.f.v_vector;

	/*
	 * Set the rate of ticking for the time-slice-end clock to
	 * be in milliseconds.  Master clock rate is 10Mhz.
	 */

	*((char *)DPCREG_TSECTL) = SC1 | BYTES2 | TIM_MODE2;
	*((char *)DPCREG_TSECNT1) = 10000 & 0xff;
	*((char *)DPCREG_TSECNT1) = (10000 >> 8) & 0xff;

	tsereld();
#endif	/* MMAX_DPC */
#if	MMAX_XPC || MMAX_APC
	/*
	 * On the XPC/APC, TSE counter is in the master ICU.
	 */

	icu_tseinit();
#endif

	/*
	 * Initialize the last free counter value.
	 */
	Last_frcount[getcpuid()] = FRcounter;

        /*
         *  set the maximum drift rate for this processor
         */
        maxdriftrecip = MAXDRIFTRECIP;

}


#if	MMAX_DPC
/*
 * tsereld - set the time until the next time-slice-end interrupt occurs.
 */

tsereld()
{
	*((char *)DPCREG_TSECTL) = SC2 | BYTES2 | TIM_MODE0;
	*((char *)DPCREG_TSECNT2) = (1000/hz) & 0xff;
	*((char *)DPCREG_TSECNT2) = ((1000/hz) >> 8) & 0xff;
}
#endif

/*  Following two routines handle the hardware details of the software
 *  clock interrupt.  softclock_vector is the interrupt vector in the itbl,
 *  and sc_vector_valid indicates whether it has been initialized.
 */

board_vbxmit_t	softclock_vector;
int	sc_vector_valid = 0;
void	softclkintr();

/* setsoftclock queues a softclock interrupt */

setsoftclock()
{
	unsigned s;

	if (!sc_vector_valid) {
		if (alloc_vector(&softclock_vector, softclkintr, 0, INTR_SOFTCLK) != 0)
			panic("setsoftclock: could not allocate vector");
		else
			sc_vector_valid = 1;
		/*
		 * Make sure this interrupt is addressed only to the master
		 * processor.
		 */
		softclock_vector.f.v_class = SLAVE_CLASS;
#if MMAX_APC
#define CPU_TO_SLOT(cpu)	(((cpu) & APCCSR_SLOTID) >> 2)
#define CPU_TO_DEV(cpu)		((cpu) & APCCSR_CPUID)
#endif
#if MMAX_XPC
#define CPU_TO_SLOT(cpu)	(((cpu) & XPCCSR_SLOTID) >> 2)
#define CPU_TO_DEV(cpu)		((cpu) & XPCCSR_CPUID)
#endif
		softclock_vector.f.v_slot = CPU_TO_SLOT(master_cpu);
		softclock_vector.f.v_device = CPU_TO_DEV(master_cpu);
	}
	s = splhigh();
	send_vector(&softclock_vector);
	splx(s);
}

/*
 * softclkintr - softclock interrupt service routine -- we assume that
 * sp points to an interrupt stack frame.
 */
void
softclkintr(ihp, sp)
ihandler_t *ihp;
long *sp;
{
	softclock(sp[FRAME_PC], sp[FRAME_PS]);
}

#if	SIMPLE_CLOCK
/*
 *	usec_elapsed is called by clock interrupt routines to determine
 *	elapsed time since last interrupt.
 */

unsigned  usec_elapsed()
{
	unsigned uticks,temp;
	int	cpuid;
/*
 * On the Multimax, the free running counter is used to determine the actual
 * interval that has passed since the last clock interrupt.  Also save away
 * this timestamp so it'll be there next time.
 */
	cpuid = cpu_number();
	temp = FRcounter;
	uticks = temp - Last_frcount[cpuid];
	Last_frcount[cpuid] = temp;

	return(uticks);
}

/*
 *	sched_usec_elapsed is used by scheduler to determine how long
 *	a second really is.  [Clocks drift on multimax due to absence
 *	of rollover timers.]
 */

unsigned sched_timestamp = 0;
boolean_t	sched_timestamp_init = FALSE;

unsigned sched_usec_elapsed()
{
    unsigned	new_timestamp;
    unsigned	result;

    new_timestamp = FRcounter;
    if (sched_timestamp_init) {
	result = new_timestamp - sched_timestamp;
    }
    else {
	result = 0;
	sched_timestamp_init = TRUE;
    }
    sched_timestamp = new_timestamp;
    return(result);
}
#endif	/* SIMPLE_CLOCK */

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
static char	*sccsid = "@(#)$RCSfile: mach_factor.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/11/05 13:46:33 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 *	File:	kern/mach_factor.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1986, Avadis Tevanian, Jr.
 *
 *	Compute the Mach Factor.
 */

#include <cpus.h>

#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <kern/sched.h>
#include <mach/machine.h>
#include <kern/processor.h>
#include <mach/processor_info.h>
#include <sys/types.h>		/* for caddr_t */

long	avenrun[3] = {0, 0, 0};
long	mach_factor[3] = {0, 0, 0};

extern int hz;

/*
 * Values are scaled by LOAD_SCALE, defined in processor_info.h
 */
static	long	fract[3] = {
	800,			/* (4.0/5.0) 5 second average */
	966,			/* (29.0/30.0) 30 second average */
	983,			/* (59.0/60.) 1 minute average */
};

compute_mach_factor()
{
	register processor_set_t	pset;
	register processor_t		processor;
	register int		ncpus;
	register int		nthreads;
	register long		factor_now;
	register long		average_now;
	register long		load_now;
	int			s;

	simple_lock(&all_psets_lock);
	pset = (processor_set_t) queue_first(&all_psets);
	while (!queue_end(&all_psets, (queue_entry_t)pset)) {

	    /*
	     *	If no processors, this pset is in suspended animation.
	     *	No load calculations are performed.
	     */
	    pset_lock(pset);
	    if((ncpus = pset->processor_count) > 0) {

		/*
		 *	Count number of threads.
		 */
		nthreads = pset->runq.count;
		processor = (processor_t) queue_first(&pset->processors);
		while (!queue_end(&pset->processors,
		    (queue_entry_t)processor)) {
			nthreads += processor->runq.count;
			processor =
			    (processor_t) queue_next(&processor->processors);
		}

		/*
		 * account for threads on cpus.
		 */
		nthreads += ncpus - pset->idle_count; 

		/*
		 *	The current thread (running this calculation)
		 *	doesn't count; it's always in the default pset.
		 */
		if (pset == &default_pset)
		   nthreads -= 1;

		if (nthreads > ncpus) {
			factor_now = (ncpus * LOAD_SCALE) / (nthreads + 1);
			load_now = (nthreads << SCHED_SHIFT) / ncpus;
		}
		else {
			factor_now = (ncpus - nthreads) * LOAD_SCALE;
			load_now = SCHED_SCALE;
		}

		/*
		 *	Load average and mach factor calculations for
		 *	those that ask about these things.
		 */

		average_now = nthreads * LOAD_SCALE;

		pset->mach_factor =
			((pset->mach_factor << 2) + factor_now)/5;
		pset->load_average =
			((pset->load_average << 2) + average_now)/5;

		/*
		 *	And some ugly stuff to keep w happy.
		 */
		if (pset == &default_pset) {
		    register int i;

		    for (i = 0; i < 3; i++) {
			mach_factor[i] = ( (mach_factor[i]*fract[i])
				 + (factor_now*(LOAD_SCALE-fract[i])) )
				/ LOAD_SCALE;
			avenrun[i] = ( (avenrun[i]*fract[i])
				 + (average_now*(LOAD_SCALE-fract[i])) )
				/ LOAD_SCALE;
		    }
		}

		/*
		 *	sched_load is the only thing used by scheduler.
		 *	It is always at least 1 (i.e. SCHED_SCALE).
		 */
		pset->sched_load = (pset->sched_load + load_now) >> 1;
	    }

	    pset_unlock(pset);
	    pset = (processor_set_t) queue_next(&pset->all_psets);
	}

	simple_unlock(&all_psets_lock);
}

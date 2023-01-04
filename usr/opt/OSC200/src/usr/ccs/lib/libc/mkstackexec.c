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
static char *rcsid = "@(#)$RCSfile: mkstackexec.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/06/07 23:29:35 $";
#endif


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak make_stack_executable = __make_stack_executable
#endif
#include <sys/types.h>
#include <sys/mman.h>

/*
 *  make_stack_executable is a routine used to make a region in the
 *  stack executable.  This is needed for compiler run-time support
 *  (e.g. DEC Pascal)
 */

#if defined(_THREAD_SAFE)

void make_stack_executable(void *first, void *last)  /* starting and ending address (must exist) */
{
    mprotect(first,
	     (unsigned long) last - (unsigned long) first + 1,
	     (PROT_READ|PROT_WRITE|PROT_EXEC));
}

#else /* faster non-thread safe version */

void make_stack_executable(void *first, void *last) /* starting and ending address (must exist) */
{
    static unsigned long stack_hi = 0,	/* top of previous exe range */
		         stack_lo = 0;	/* bottom of previous exe range */

    static unsigned long pgmask = 0;    /* mask to make address start of page */

    unsigned long	high = (unsigned long) last,
                        low  = (unsigned long) first;


    if (!pgmask) {
	/*
	 *  Never been executed before.
	 */

        pgmask = ~((unsigned long) getpagesize() - 1);

	high &= pgmask; low &= pgmask;
	mprotect(low, high-low+1, (PROT_READ|PROT_WRITE|PROT_EXEC));
	stack_hi = high; stack_lo = low;

	return;
    }

    high &= pgmask; low &= pgmask;		/* Set to page beginning */

    if (high > stack_hi) {
	/*
	 *  Need to protect from stack_hi --> high
	 */

	mprotect(stack_hi, high-stack_hi+1, (PROT_READ|PROT_WRITE|PROT_EXEC));
	stack_hi = high;
    }
    

    if (low < stack_lo) {
	/*
	 *  Need to protect from low --> stack_lo.
	 */

	mprotect(low, stack_lo-low+1, (PROT_READ|PROT_WRITE|PROT_EXEC));
	stack_lo = low;
    }
}
#endif



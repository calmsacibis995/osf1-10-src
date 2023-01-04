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
static char *rcsid = "@(#)$RCSfile: cma_host.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/08/18 14:56:26 $";
#endif
/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	DEC OSF/1 Host-specific routines
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	18 November 1992
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Brian Keane 	19 January 1993
 *		Implement fork().
 *	002	Dave Butenhof	12 April 1993
 *		Add argument to cma__int_timed_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	003	Brian Keane	23 June 1993
 *		Cleanup ANSI namespace for the libc routines we preempt.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_tcb_defs.h>
#include <cma_timer.h>
#include <cma_mutex.h>
#include <cma_condition.h>

/*
 * GLOBAL DATA
 */

/*
 * LOCAL DATA
 */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	sleep() -- thread synchronous sleep.
 *
 *  FORMAL PARAMETERS:
 *
 *	secs	Number of seconds to sleep.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Always returns zero.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
#pragma weak sleep = __sleep
unsigned int
__sleep (unsigned int secs)
    {
    struct timeval	time;
    cma__t_int_tcb	*self;


    cma__get_time (&time);
    time.tv_sec += secs;
    self = cma__get_self_tcb ();
    cma__int_lock (self->tswait_mutex);

    TRY {
	while (cma__int_timed_wait (
		self->tswait_cv,
		self->tswait_mutex,
		&time,
		self) != cma_s_timed_out);
	}
    FINALLY {
	cma__int_unlock (self->tswait_mutex);
	}
    ENDTRY

    return (unsigned int)0;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *      fork() -- Unix fork function.
 *
 *  FORMAL PARAMETERS:
 *
 *      none.
 *
 *  IMPLICIT INPUTS:
 *
 *      None
 *
 *  IMPLICIT OUTPUTS:
 *
 *      None
 *
 *  FUNCTION VALUE:
 *
 *      The pid of the new process, or 0 in the child
 *
 *  SIDE EFFECTS:
 *
 *      None
 */
#pragma weak fork = __fork
int
__fork (void)
    {
    return cma_fork();
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_HOST.C */
/*  *5    29-JUN-1993 17:28:41 KEANE "Fix libc replacement routines for ANSI name compliance" */
/*  *4    16-APR-1993 13:09:55 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *3    19-JAN-1993 16:04:11 KEANE "Added fork()" */
/*  *2    20-NOV-1992 11:19:23 BUTENHOF "Fix pointer types" */
/*  *1    18-NOV-1992 13:55:28 BUTENHOF "Misc. thread-sync functions" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_HOST.C */

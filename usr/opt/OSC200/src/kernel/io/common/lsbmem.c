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
static char *rcsid = "@(#)$RCSfile: lsbmem.c,v $ $Revision: 1.2.2.3 $ (DEC) $Date: 1992/08/31 12:18:14 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)lsbmem.c	9.2	(ULTRIX/OSF)	10/24/91";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *                                                                      *
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 *   File name: machine/common/lsbmem.c
 *
 *   Source file description: 
 *	this file has the routines that deal with the laser system
 *	memory controller
 *
 *   Modification history:
 *
 * 21-Jun-91       jac
 *		Created this file for ruby support
 *
 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/param.h>
#include <machine/cpu.h>
#include <hal/cpuconf.h>



/*
 * Log memory errors in kernel buffer:
 *
 * Traps through SCB vector ??: uncorrectable memory errors.
 */
lsb_memerr()
{
	register int recover;	/* set to 1 if we can recover from this error */
	register u_int time;	/* from TODR */

	recover = 0;
	/*
	 * First note the time; then determine if hardware retry is
	 * possible, which will be used for the recoverable cases.
	 */
	time = 0;
	printf("memerr interrupt\n");
	panic ("memory error");
}

/*
 * Log CRD memory errors in kernel buffer:
 *
 * Traps through SCB vector ??: correctable memory errors.
 *
 * These errors are recoverable.
 */
lsb_crderr()
{
	int recover;	/* set to 1 if we can recover from this error */
	u_int time;

	recover = 0;
	time = 0;
	return(0);
}

/*
 * Log Memory CSRs
 */

lsb_logmempkt(recover)
	int recover;		/* for pkt priority */
{
	return(0);
}

/*
 * Enable CRD interrupts.
 * This runs at regular (15 min) intervals, turning on the interrupt.
 * It is called by the timeout call in memenable in machdep.c
 * The interrupt is turned off, in adu_crderr(), when 3 error interrupts
 * occur in 1 time period.  Thus we report @ most once per memintvl (15 mins)
 */
lsb_memenable()
{
        return(0);
}

lsb_memint()
{
        return(0);
}

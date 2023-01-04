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
static char	*sccsid = "@(#)$RCSfile: time_stamp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:28:06 $";
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

#include <mach/kern_return.h>
#include <mach/time_stamp.h>

/*
 *	ts.c - kern_timestamp system call.
 */

#ifdef	multimax
#include <mmax/timer.h>
#endif	multimax

unsigned ts_tick_count;

kern_return_t
kern_timestamp(tsp)
struct	tsval	*tsp;
{
	struct	tsval	temp;
#ifdef	multimax
	temp.low_val = FRcounter;
	temp.high_val = 0;
#else	multimax
	temp.low_val = 0;
	temp.high_val = ts_tick_count;
#endif	multimax

	if (copyout(&temp, tsp, sizeof(struct tsval)) != KERN_SUCCESS)
		return(KERN_INVALID_ADDRESS);
	return(KERN_SUCCESS);
}

/*
 *	Initialization procedure.
 */

timestamp_init()
{
#ifdef	multimax
#else	multimax
	ts_tick_count = 0;
#endif	multimax
}

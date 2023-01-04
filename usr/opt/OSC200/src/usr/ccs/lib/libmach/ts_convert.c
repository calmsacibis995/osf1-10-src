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
static char	*sccsid = "@(#)$RCSfile: ts_convert.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:11:25 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	ts_convert.c -- convert machine dependent timestamps to
 *		machine independent timevals.
 *
 */

#include <sys/time.h>
#include <mach/time_stamp.h>

convert_ts_to_tv(ts_format,tsp,tvp)
int	ts_format;
struct tsval *tsp;
struct timeval *tvp;
{
	switch(ts_format) {
		case TS_FORMAT_DEFAULT:
			/*
			 *	High value is tick count at 100 Hz
			 */
			tvp->tv_sec = tsp->high_val/100;
			tvp->tv_usec = (tsp->high_val % 100) * 10000;
			break;
		case TS_FORMAT_MMAX:
			/*
			 *	Low value is usec.
			 */
			tvp->tv_sec = tsp->low_val/1000000;
			tvp->tv_usec = tsp->low_val % 1000000;
			break;
		default:
			/*
			 *	Zero output timeval to indicate that
			 *	we can't decode this timestamp.
			 */
			tvp->tv_sec = 0;
			tvp->tv_usec = 0;
			break;
	 }
}

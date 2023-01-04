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
static char     *sccsid = "@(#)$RCSfile: gettstamp.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:38:34 $";
#endif
/*
 */

/*
 * gettstamp - return the system time in timestamp format
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_fp.h>
#include <ntp/ntp_unixtime.h>

/*
 * Reference to conversion table
 */
extern u_int ustotslo[];
extern u_int ustotsmid[];
extern u_int ustotshi[];

void
gettstamp(ts)
	l_fp *ts;
{
	struct timeval tv;

	/*
	 * Quickly get the time of day and convert it
	 */
	(void) gettimeofday(&tv, (struct timezone *)NULL);
	TVTOTS(&tv, ts);
	ts->l_uf += TS_ROUNDBIT;	/* guaranteed not to overflow */
	ts->l_ui += JAN_1970;
	ts->l_uf &= TS_MASK;
}

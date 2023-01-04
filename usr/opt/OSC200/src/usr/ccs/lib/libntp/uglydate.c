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
static char     *sccsid = "@(#)$RCSfile: uglydate.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:40:28 $";
#endif
/*
 */

/*
 * uglydate - convert a time stamp to something barely readable
 *	      The string returned is 37 characters long.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_fp.h>
#include <ntp/ntp_unixtime.h>
#include "lib_strbuf.h"

char *
uglydate(ts)
	l_fp *ts;
{
	char *bp;
	char *timep;
	struct tm *tm;
	u_int sec;
	u_int msec;
	int year;
	extern char *ulfptoa();

	timep = ulfptoa(ts, 6);		/* returns max 17 characters */

	LIB_GETBUF(bp);
	
	sec = ts->l_ui - JAN_1970;
	msec = ts->l_uf / 4294967;	/* fract / (2**32/1000) */
	tm = gmtime((int *)&sec);
	if (ts->l_ui == 0) {
		/*
		 * Probably not a real good thing to do.  Oh, well.
		 */
		year = 0;
		tm->tm_yday = 0;
		tm->tm_hour = 0;
		tm->tm_min = 0;
		tm->tm_sec = 0;
	} else {
		year = tm->tm_year;
		while (year >= 100)
			year -= 100;
	}

	(void) sprintf(bp, "%17s %02d:%03d:%02d:%02d:%02d.%03d",
	    timep, year, tm->tm_yday, tm->tm_hour, tm->tm_min,
	    tm->tm_sec, msec);

	return bp;
}

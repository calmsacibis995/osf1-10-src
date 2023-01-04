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
static char     *sccsid = "@(#)$RCSfile: prettydate.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:39:44 $";
#endif
/*
 */

/*
 * prettydate - convert a time stamp to something readable
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
prettydate(ts)
	l_fp *ts;
{
	char *bp;
	struct tm *tm;
	u_int sec;
	u_int msec;
	static char *months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	static char *days[] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};

	LIB_GETBUF(bp);
	
	sec = ts->l_ui - JAN_1970;
	msec = ts->l_uf / 4294967;	/* fract / (2**32/1000) */

	tm = localtime((int *)&sec);

	(void) sprintf(bp, "%08x.%08x  %s, %s %2d %4d %2d:%02d:%02d.%03d",
	    ts->l_ui, ts->l_uf, days[tm->tm_wday], months[tm->tm_mon],
	    tm->tm_mday, 1900+tm->tm_year, tm->tm_hour, tm->tm_min,
	    tm->tm_sec, msec);
	
	return bp;
}

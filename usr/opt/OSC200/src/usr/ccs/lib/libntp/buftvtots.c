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
static char     *sccsid = "@(#)$RCSfile: buftvtots.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:37:47 $";
#endif
/*
 */

/*
 * buftvtots - pull a Unix-format (struct timeval) time stamp out of
 *	       an octet stream and convert it to a l_fp time stamp.
 *	       This is useful when using the clock line discipline.
 */
#include <sys/types.h>
#include <sys/time.h>

#include <ntp/ntp_fp.h>
#include <ntp/ntp_unixtime.h>

/*
 * Conversion tables
 */
extern u_int ustotslo[];
extern u_int ustotsmid[];
extern u_int ustotshi[];

int
buftvtots(bufp, ts)
	char *bufp;
	l_fp *ts;
{
	register u_char *bp;
	register u_int sec;
	register u_int usec;

	bp = (u_char *)bufp;

	sec = (u_int)*bp++ & 0xff;
	sec <<= 8;
	sec += (u_int)*bp++ & 0xff;
	sec <<= 8;
	sec += (u_int)*bp++ & 0xff;
	sec <<= 8;
	sec += (u_int)*bp++ & 0xff;

	usec = (u_int)*bp++ & 0xff;
	usec <<= 8;
	usec += (u_int)*bp++ & 0xff;
	usec <<= 8;
	usec += (u_int)*bp++ & 0xff;
	usec <<= 8;
	usec += (u_int)*bp & 0xff;

	if (usec > 999999)
		return 0;

	ts->l_ui = sec + (u_int)JAN_1970;
	TVUTOTSF(usec, ts->l_uf);
	return 1;
}

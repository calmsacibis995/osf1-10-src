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
static char     *sccsid = "@(#)$RCSfile: calyearstart.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:38:13 $";
#endif
/*
 */

/*
 * calyearstart - determine the NTP time at midnight of January 1 in
 *		  the year of the given date.
 */
#include <sys/types.h>

#include <ntp/ntp_calendar.h>

/*
 * calyeartab - year start offsets from the beginning of a cycle
 */
u_int calyeartab[YEARSPERCYCLE] = {
	(SECSPERLEAPYEAR-JANFEBLEAP),
	(SECSPERLEAPYEAR-JANFEBLEAP) + SECSPERYEAR,
	(SECSPERLEAPYEAR-JANFEBLEAP) + 2*SECSPERYEAR,
	(SECSPERLEAPYEAR-JANFEBLEAP) + 3*SECSPERYEAR
};

u_int
calyearstart(dateinyear)
	register u_int dateinyear;
{
	register u_int cyclestart;
	register u_int nextyear, lastyear;
	register int i;

	/*
	 * Find the start of the cycle this is in.
	 */
	if (dateinyear >= MAR1988)
		cyclestart = MAR1988;
	else
		cyclestart = MAR1900;
	while ((cyclestart + SECSPERCYCLE) <= dateinyear)
		cyclestart += SECSPERCYCLE;
	
	/*
	 * If we're in the first year of the cycle, January 1 is
	 * two months back from the cyclestart and the year is
	 * a leap year.
	 */
	lastyear = cyclestart + calyeartab[0];
	if (dateinyear < lastyear)
		return (cyclestart - JANFEBLEAP);

	/*
	 * Look for an intermediate year
	 */
	for (i = 1; i < YEARSPERCYCLE; i++) {
		nextyear = cyclestart + calyeartab[i];
		if (dateinyear < nextyear)
			return lastyear;
		lastyear = nextyear;
	}

	/*
	 * Not found, must be in last two months of cycle
	 */
	return nextyear;
}

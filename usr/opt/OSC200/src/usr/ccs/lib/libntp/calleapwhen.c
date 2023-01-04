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
static char     *sccsid = "@(#)$RCSfile: calleapwhen.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:38:00 $";
#endif
/*
 */

/*
 * calleapwhen - determine the number of seconds to the next possible
 *		 leap occurance and the last one.
 */
#include <sys/types.h>

#include <ntp/ntp_calendar.h>

/*
 * calleaptab - leaps occur at the end of December and June
 */
int calleaptab[10] = {
	-(JAN+FEBLEAP)*SECSPERDAY,	/* leap previous to cycle */
	(MAR+APR+MAY+JUN)*SECSPERDAY,	/* end of June */
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT+NOV+DEC)*SECSPERDAY, /* end of Dec */
	(MAR+APR+MAY+JUN)*SECSPERDAY + SECSPERYEAR,
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT+NOV+DEC)*SECSPERDAY + SECSPERYEAR,
	(MAR+APR+MAY+JUN)*SECSPERDAY + 2*SECSPERYEAR,
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT+NOV+DEC)*SECSPERDAY + 2*SECSPERYEAR,
	(MAR+APR+MAY+JUN)*SECSPERDAY + 3*SECSPERYEAR,
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT+NOV+DEC)*SECSPERDAY + 3*SECSPERYEAR,
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT+NOV+DEC+JAN+FEBLEAP+MAR+APR+MAY+JUN)
	    *SECSPERDAY + 3*SECSPERYEAR,	/* next after current cycle */
};

void
calleapwhen(ntpdate, leaplast, leapnext)
	u_int ntpdate;
	u_int *leaplast;
	u_int *leapnext;
{
	register u_int dateincycle;
	register int i;

	/*
	 * Find the offset from the start of the cycle
	 */
	dateincycle = ntpdate;
	if (dateincycle >= MAR1988)
		dateincycle -= MAR1988;
	else
		dateincycle -= MAR1900;

	while (dateincycle >= SECSPERCYCLE)
		dateincycle -= SECSPERCYCLE;

	/*
	 * Find where we are with respect to the leap events.
	 */
	for (i = 1; i < 9; i++)
		if (dateincycle < (u_int)calleaptab[i])
			break;
	
	/*
	 * i points at the next leap.  Compute the last and the next.
	 */
	*leaplast = (u_int)((int)dateincycle - calleaptab[i-1]);
	*leapnext = (u_int)(calleaptab[i] - (int)dateincycle);
}

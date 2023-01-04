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
static char     *sccsid = "@(#)$RCSfile: caljulian.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:37:53 $";
#endif
/*
 */

/*
 * caljulian - determine the Julian date from an NTP time.
 */
#include <sys/types.h>

#include <ntp/ntp_calendar.h>

/*
 * calmonthtab - month start offsets from the beginning of a cycle.
 */
static u_short calmonthtab[12] = {
	0,						/* March */
	MAR,						/* April */
	(MAR+APR),					/* May */
	(MAR+APR+MAY),					/* June */
	(MAR+APR+MAY+JUN),				/* July */
	(MAR+APR+MAY+JUN+JUL),				/* August */
	(MAR+APR+MAY+JUN+JUL+AUG),			/* September */
	(MAR+APR+MAY+JUN+JUL+AUG+SEP),			/* October */
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT),		/* November */
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT+NOV),		/* December */
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT+NOV+DEC),	/* January */
	(MAR+APR+MAY+JUN+JUL+AUG+SEP+OCT+NOV+DEC+JAN),	/* February */
};

/*
 * caldaytab - calendar year start day offsets
 */
static u_short caldaytab[YEARSPERCYCLE] = {
	(DAYSPERYEAR - (JAN + FEB)),
	((DAYSPERYEAR * 2) - (JAN + FEB)),
	((DAYSPERYEAR * 3) - (JAN + FEB)),
	((DAYSPERYEAR * 4) - (JAN + FEB)),
};

void
caljulian(ntptime, jt)
	u_int ntptime;
	register struct calendar *jt;
{
	register int i;
	register u_int nt;
	register u_short snt;
	register int cyear;

	/*
	 * Find the start of the cycle this is in.
	 */
	nt = ntptime;
	if (nt >= MAR1988) {
		cyear = CYCLE22;
		nt -= MAR1988;
	} else {
		cyear = 0;
		nt -= MAR1900;
	}
	while (nt >= SECSPERCYCLE) {
		nt -= SECSPERCYCLE;
		cyear++;
	}
	
	/*
	 * Seconds, minutes and hours are too hard to do without
	 * divides, so we don't.
	 */
	jt->second = nt % SECSPERMIN;
	nt /= SECSPERMIN;		/* nt in minutes */
	jt->minute = nt % MINSPERHR;
	snt = nt / MINSPERHR;		/* snt in hours */
	jt->hour = snt % HRSPERDAY;
	snt /= HRSPERDAY;		/* nt in days */

	/*
	 * snt is now the number of days into the cycle, from 0 to 1460.
	 */
	cyear <<= 2;
	if (snt < caldaytab[0]) {
		jt->yearday = snt + JAN + FEBLEAP + 1;	/* first year is leap */
	} else {
		for (i = 1; i < YEARSPERCYCLE; i++)
			if (snt < caldaytab[i])
				break;
		jt->yearday = snt - caldaytab[i-1] + 1;
		cyear += i;
	}
	jt->year = cyear + 1900;

	/*
	 * One last task, to compute the month and day.  Normalize snt to
	 * a day within a cycle year.
	 */
	while (snt >= DAYSPERYEAR)
		snt -= DAYSPERYEAR;
	for (i = 0; i < 11; i++)
		if (snt < calmonthtab[i+1])
			break;
	
	if (i > 9)
		jt->month = i - 9;	/* January or February */
	else
		jt->month = i + 3;	/* March through December */
	jt->monthday = snt - calmonthtab[i] + 1;
}

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
static char     *sccsid = "@(#)$RCSfile: caltontp.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:38:06 $";
#endif
/*
 */

/*
 * caltontp - convert a julian date to an NTP time
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

u_int
caltontp(jt)
	register struct calendar *jt;
{
	register int cyear;
	register int resyear;
	register u_int nt;
	register int yearday;

	/*
	 * Find the start of the cycle this is in.
	 */
	cyear = (jt->year - 1900) >> 2;
	resyear = (jt->year - 1900) - (cyear << 2);
	yearday = 0;
	if (resyear == 0) {
		if (jt->yearday == 0) {
			if (jt->month == 1 || jt->month == 2)
				cyear--;
				resyear = 3;
		} else {
			if (jt->yearday <= (JAN+FEBLEAP)) {
				cyear--;
				resyear = 3;
				yearday = calmonthtab[10] + jt->yearday;
			} else {
				yearday = jt->yearday - (JAN+FEBLEAP);
			}
		}
	} else {
		if (jt->yearday == 0) {
			if (jt->month == 1 || jt->month == 2)
				resyear--;
		} else {
			if (jt->yearday <= JAN+FEB) {
				resyear--;
				yearday = calmonthtab[10] + jt->yearday;
			} else {
				yearday = jt->yearday - (JAN+FEB);
			}
		}
	}

	if (yearday == 0) {
		if (jt->month >= 3) {
			yearday = calmonthtab[jt->month - 3] + jt->monthday;
		} else {
			yearday = calmonthtab[jt->month + 9] + jt->monthday;
		}
	}

	nt = TIMESDPERC((u_int)cyear);
	while (resyear-- > 0)
		nt += DAYSPERYEAR;
	nt += (u_int) (yearday - 1);

	nt = TIMES24(nt) + (u_int)jt->hour;
	nt = TIMES60(nt) + (u_int)jt->minute;
	nt = TIMES60(nt) + (u_int)jt->second;

	return nt + MAR1900;
}

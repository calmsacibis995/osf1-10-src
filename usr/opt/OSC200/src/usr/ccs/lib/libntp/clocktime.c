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
static char     *sccsid = "@(#)$RCSfile: clocktime.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:38:19 $";
#endif
/*
 */

/*
 * clocktime - compute the NTP date from a day of year, hour, minute
 *	       and second.
 */
#include <sys/types.h>
#include <sys/time.h>

#include <ntp/ntp_fp.h>
#include <ntp/ntp_unixtime.h>


/*
 * Hacks to avoid excercising the multiplier.  I have no pride.
 */
#define	MULBY10(x)	(((x)<<3) + ((x)<<1))
#define	MULBY60(x)	(((x)<<6) - ((x)<<2))	/* watch overflow */
#define	MULBY24(x)	(((x)<<4) + ((x)<<3))

/*
 * Two days, in seconds.
 */
#define	TWODAYS		(2*24*60*60)

/*
 * We demand that the time be within CLOSETIME seconds of the receive
 * time stamp.  This is about 4 hours, which hopefully should be
 * wide enough to collect most data, while close enough to keep things
 * from getting confused.
 */
#define	CLOSETIME	(4*60*60)


int
clocktime(yday, hour, minute, second, tzoff, rec_ui, yearstart, ts_ui)
	int yday;
	int hour;
	int minute;
	int second;
	int tzoff;
	u_int rec_ui;
	u_int *yearstart;
	u_int *ts_ui;
{
	register int tmp;
	register u_int date;
	register u_int yst;
	extern u_int calyearstart();

	/*
	 * Compute the offset into the year in seconds.  Note that
	 * this could come out to be a negative number.
	 */
	tmp = (int)(MULBY24((yday-1)) + hour + tzoff);
	tmp = MULBY60(tmp) + (int)minute;
	tmp = MULBY60(tmp) + (int)second;

	/*
	 * Initialize yearstart, if necessary.
	 */
	yst = *yearstart;
	if (yst == 0) {
		yst = calyearstart(rec_ui);
		*yearstart = yst;
	}

	/*
	 * Now the fun begins.  We demand that the received clock time
	 * be within CLOSETIME of the receive timestamp, but
	 * there is uncertainty about the year the timestamp is in.
	 * Use the current year start for the first check, this should
	 * work most of the time.
	 */
	date = (u_int)(tmp + (int)yst);
	if (date < (rec_ui + CLOSETIME) &&
	    date > (rec_ui - CLOSETIME)) {
		*ts_ui = date;
		return 1;
	}

	/*
	 * Trouble.  Next check is to see if the year rolled over and, if
	 * so, try again with the new year's start.
	 */
	yst = calyearstart(rec_ui);
	if (yst != *yearstart) {
		date = (u_int)((int)yst + tmp);
		*ts_ui = date;
		if (date < (rec_ui + CLOSETIME) &&
		    date > (rec_ui - CLOSETIME)) {
			*yearstart = yst;
			return 1;
		}
	}

	/*
	 * Here we know the year start matches the current system
	 * time.  One remaining possibility is that the time code
	 * is in the year previous to that of the system time.  This
	 * is only worth checking if the receive timestamp is less
	 * than a couple of days into the new year.
	 */
	if ((rec_ui - yst) < TWODAYS) {
		yst = calyearstart(yst - TWODAYS);
		if (yst != *yearstart) {
			date = (u_int)(tmp + (int)yst);
			if (date < (rec_ui + CLOSETIME) &&
			    date > (rec_ui - CLOSETIME)) {
				*yearstart = yst;
				*ts_ui = date;
				return 1;
			}
		}
	}

	/*
	 * One last possibility is that the time stamp is in the year
	 * following the year the system is in.  Try this one before
	 * giving up.
	 */
	yst = calyearstart(rec_ui + TWODAYS);
	if (yst != *yearstart) {
		date = (u_int)((int)yst + tmp);
		if (date < (rec_ui + CLOSETIME) &&
		    date > (rec_ui - CLOSETIME)) {
			*yearstart = yst;
			*ts_ui = date;
			return 1;
		}
	}

	/*
	 * Give it up.
	 */
	return 0;
}

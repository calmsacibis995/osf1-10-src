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
/*
 *	@(#)$RCSfile: ntp_unixtime.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:41:41 $
 */
/*
 */

/*
 * ntp_unixtime.h - contains constants and macros for converting between
 *		    NTP time stamps (l_fp) and Unix times (struct timeval)
 */

/*
 * Time of day conversion constant.  Ntp's time scale starts in 1900,
 * Unix in 1970.
 */
#define	JAN_1970	2208988800	/* 1970 - 1900 in seconds */

/*
 * These constants are used to round the time stamps computed from
 * a struct timeval to the microsecond (more or less).  This keeps
 * things neat.
 */
#define	TS_MASK		0xfffff000	/* mask to usec, for time stamps */
#define	TS_ROUNDBIT	0x00000800	/* round at this bit */


/*
 * Convert usec to a time stamp fraction.  If you use this the program
 * must include the following declarations:
 *
 * extern u_int ustotslo[];
 * extern u_int ustotsmid[];
 * extern u_int ustotshi[];
 */
#define	TVUTOTSF(tvu, tsf) \
	(tsf) = ustotslo[(tvu) & 0xff] \
	    + ustotsmid[((tvu) >> 8) & 0xff] \
	    + ustotshi[((tvu) >> 16) & 0xf]

/*
 * Convert a struct timeval to a time stamp.
 */
#define TVTOTS(tv, ts) \
	do { \
		(ts)->l_ui = (unsigned int)(tv)->tv_sec; \
		TVUTOTSF((tv)->tv_usec, (ts)->l_uf); \
	} while(0)

/*
 * TV_SHIFT is used to turn the table result into a usec value.  To round,
 * add in TV_ROUNDBIT before shifting
 */
#define	TV_SHIFT	3
#define	TV_ROUNDBIT	0x4


/*
 * Convert a time stamp fraction to microseconds.  The time stamp
 * fraction is assumed to be unsigned.  To use this in a program, declare:
 *
 * extern int tstouslo[];
 * extern int tstousmid[];
 * extern int tstoushi[];
 */
#define	TSFTOTVU(tsf, tvu) \
	(tvu) = (tstoushi[((tsf) >> 24) & 0xff] \
	    + tstousmid[((tsf) >> 16) & 0xff] \
	    + tstouslo[((tsf) >> 9) & 0x7f] \
	    + TV_ROUNDBIT) >> TV_SHIFT
/*
 * Convert a time stamp to a struct timeval.  The time stamp
 * has to be positive.
 */
#define	TSTOTV(ts, tv) \
	do { \
		(tv)->tv_sec = (ts)->l_ui; \
		TSFTOTVU((ts)->l_uf, (tv)->tv_usec); \
		if ((tv)->tv_usec == 1000000) { \
			(tv)->tv_sec++; \
			(tv)->tv_usec = 0; \
		} \
	} while (0)

/*
 * Convert milliseconds to a time stamp fraction.  This shouldn't be
 * here, but it is convenient since the guys who use the definition will
 * often be including this file anyway.
 */
#define	MSUTOTSF(msu, tsf) \
	(tsf) = msutotsfhi[((msu) >> 5) & 0x1f] + msutotsflo[(msu) & 0x1f]

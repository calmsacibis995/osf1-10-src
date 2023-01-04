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
 *	@(#)$RCSfile: ntpdate.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:41:57 $
 */
/*
 */

/*
 * ntpdate.h - declarations for the ntpdate program
 */

/*
 * The server structure is a much simplified version of the
 * peer structure, for ntpdate's use.  Since we always send
 * in client mode and expect to receive in server mode, this
 * leaves only a very limited number of things we need to
 * remember about the server.
 */
struct server {
	struct sockaddr_in srcadr;	/* address of remote host */
	u_char leap;			/* leap indicator */
	u_char stratum;			/* stratum of remote server */
	s_char precision;		/* server's clock precision */
	u_char trust;			/* trustability of the filtered data */
	u_fp distance;			/* distance from primary clock */
	u_fp dispersion;		/* peer clock dispersion */
	u_int refid;			/* peer reference ID */
	l_fp reftime;			/* time of peer's last update */
	u_int event_time;		/* time for next timeout */
	u_short xmtcnt;			/* number of packets transmitted */
	u_short filter_nextpt;		/* index into filter shift register */
	u_fp filter_delay[PEER_SHIFT];	/* delay part of shift register */
	l_fp filter_offset[PEER_SHIFT];	/* offset part of shift register */
	s_fp filter_soffset[PEER_SHIFT]; /* offset in s_fp format, for disp */
	l_fp org;			/* peer's originate time stamp */
	l_fp xmt;			/* transmit time stamp */
	u_fp estdelay;			/* filter estimated delay */
	u_fp estdisp;			/* filter estimated dispersion */
	l_fp estoffset;			/* filter estimated clock offset */
	s_fp estsoffset;		/* fp version of above */
};


/*
 * ntpdate runs everything on a simple, short timeout.  It sends a
 * packet and sets the timeout (by default, to a small value suitable
 * for a LAN).  If it receives a response it sends another request.
 * If it times out it shifts zeroes into the filter and sends another
 * request.
 *
 * The timer routine is run often (once every 1/5 second currently)
 * so that time outs are done with reasonable precision.
 */
#define TIMER_HZ	(5)		/* 5 per second */

/*
 * ntpdate will make a long adjustment using adjtime() if the times
 * are close, or step the time if the times are farther apart.  The
 * following defines what is "close".
 */
#define	NTPDATE_THRESHOLD	(FP_SECOND >> 1)	/* 1/2 second */

/*
 * When doing adjustments, ntpdate actually overadjusts (currently
 * by 50%, though this may change).  While this will make it take longer
 * to reach a steady state condition, it will typically result in
 * the clock keeping more accurate time, on average.  The amount of
 * overshoot is limited.
 */
/* #define	ADJ_OVERSHOOT	1/2	/* this is hard coded */
#define	ADJ_MAXOVERSHOOT	0x10000000	/* 50 ms as a ts fraction */

/*
 * Since ntpdate isn't aware of some of the things that normally get
 * put in an NTP packet, we fix some values.
 */
#define	NTPDATE_PRECISION	(-6)		/* use this precision */
#define	NTPDATE_DISTANCE	FP_SECOND	/* distance is 1 sec */
#define	NTPDATE_DISP		FP_SECOND	/* so is the dispersion */
#define	NTPDATE_REFID		(0)		/* reference ID to use */


/*
 * Some defaults
 */
#define	DEFTIMEOUT	5		/* 5 timer increments */
#define	DEFSAMPLES	4		/* get 4 samples per server */
#define	DEFPRECISION	(-5)		/* the precision we claim */

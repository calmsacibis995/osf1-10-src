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
 *	@(#)$RCSfile: ntp_refclock.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:41:23 $
 */
/*
 */

/*
 * ntp_refclock.h - definitions for reference clock support
 */

/*
 * Macros to determine the clock type and unit numbers from a
 * 127.127.t.u address.
 */
#define	REFCLOCKTYPE(srcadr)	((SRCADR(srcadr) >> 8) & 0xff)
#define REFCLOCKUNIT(srcadr)	(SRCADR(srcadr) & 0xff)

/*
 * Struct refclock provides the interface between the reference
 * clock support and particular clock drivers.  There are entries
 * to open and close a unit, optional values to specify the
 * timer interval for calls to the transmit procedure and to
 * specify a polling routine to be called when the transmit
 * procedure executes.  There is an entry which is called when
 * the transmit routine is about to shift zeroes into the
 * filter register, and entries for stuffing fudge factors into
 * the driver and getting statistics from it.
 */
struct refclock {
	int (*clock_start)();		/* start a clock unit */
	void (*clock_shutdown)();	/* shut a clock down */
	void (*clock_poll)();		/* called from the xmit routine */
	void (*clock_leap)();		/* inform driver a leap has occured */
	void (*clock_control)();	/* set fudge values, return stats */
	void (*clock_init)();		/* initialize driver data at startup */
	void (*clock_buginfo)();	/* get clock dependent bug info */
	u_int clock_xmitinterval;	/* timer setting for xmit routine */
	u_int clock_flags;		/* flag values */
};

/*
 * Definitions for default values
 */
#define	noentry		0
#define	STDPOLL		(1<<NTP_MINPOLL)
#define	NOPOLL		0

/*
 * Definitions for flags
 */
#define	NOFLAGS			0
#define	REF_FLAG_BCLIENT	0x1	/* clock prefers to run as a bclient */

/*
 * Flag values
 */
#define	CLK_HAVETIME1	0x1
#define	CLK_HAVETIME2	0x2
#define	CLK_HAVEVAL1	0x4
#define	CLK_HAVEVAL2	0x8

#define	CLK_FLAG1	0x1
#define	CLK_FLAG2	0x2
#define	CLK_FLAG3	0x4
#define	CLK_FLAG4	0x8

#define	CLK_HAVEFLAG1	0x10
#define	CLK_HAVEFLAG2	0x20
#define	CLK_HAVEFLAG3	0x40
#define	CLK_HAVEFLAG4	0x80

/*
 * Structure for returning clock status
 */
struct refclockstat {
	u_char type;
	u_char flags;
	u_char haveflags;
	u_char lencode;
	char *lastcode;
	u_int polls;
	u_int noresponse;
	u_int badformat;
	u_int baddata;
	u_int timereset;
	char *clockdesc;	/* description of clock, in ASCII */
	l_fp fudgetime1;
	l_fp fudgetime2;
	int fudgeval1;
	int fudgeval2;
	u_char currentstatus;
	u_char lastevent;
	u_char unused[2];
};


/*
 * Reference clock I/O structure.  Used to provide an interface between
 * the reference clock drivers and the I/O module.
 */
struct refclockio {
	struct refclockio *next;
	void (*clock_recv)();
	caddr_t srcclock;	/* pointer to clock structure */
	int datalen;
	int fd;
	u_int recvcount;
};


/*
 * Sizes of things we return for debugging
 */
#define	NCLKBUGVALUES		16
#define	NCLKBUGTIMES		32

/*
 * Structure for returning debugging info
 */
struct refclockbug {
	u_char nvalues;
	u_char ntimes;
	u_short svalues;
	u_int stimes;
	u_int values[NCLKBUGVALUES];
	l_fp times[NCLKBUGTIMES];
};

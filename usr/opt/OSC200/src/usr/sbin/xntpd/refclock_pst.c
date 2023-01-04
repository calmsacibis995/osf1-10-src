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
static char     *sccsid = "@(#)$RCSfile: refclock_pst.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/05/04 16:59:12 $";
#endif
/*
 * refclock_pst - driver for the PSTI 1010/1020 WWV clock
 */
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sgtty.h>
#include <errno.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>
#include <ntp/ntp_refclock.h>
#include <ntp/ntp_unixtime.h>

#if defined(REFCLOCK) && defined(PST)
/*
 * This driver is in good measure due to David Schachter, who wrote
 * the firmware for the PST clock.  Not that he is to blame for
 * any of this, but he kindly loaned me a clock to allow me to
 * debug this.
 *
 * Postscript:
 *
 * The strategy in here is actually pretty good, especially if
 * you try to support the clock on something lacking low order
 * clock bits like a Sun, since all the business which is done
 * before taking a time stamp tends to randomize the taking of
 * the stamp with respect to the timer interrupt.  It is, however,
 * a big cpu hog, and in some ways is a bit of a waste since, as
 * it turns out, the PST clock can give you no better than a
 * millisecond precision and it doesn't pay to try to push it
 * harder.
 *
 * In any event, like the first waffle off the iron, this one
 * should probably be tossed.  My current preference would be
 * to retain the 12-a-minute schedule, but to use the QU command
 * instead of the QD and QT, and to only send a QM command with
 * the 12th poll of the minute to get the minutes-since-sync
 * and the station.  Need to get a clock which supports QU,
 * however.
 *
 * End postscript
 *
 * This driver polls the clock using the QM, QT and QD commands.
 * Ntpd actually uses QU instead of the last two, something I would
 * like to have done as well since it gives you the day and time
 * atom, but the firmware in the clock I had (X04.01.999) didn't know
 * about this command.
 *
 * The QM command produces output like:
 *
 *	O6B532352823C00270322
 *	   b     c  deeee
 *
 * We use (b) for the time zone, (c) to see whether time is available,
 * (d) to tell whether we are sync'd to WWV or WWVH, and (e) to determine
 * the number of minutes since the last signal was received.  We
 * don't trust the clock for more than about 20 minutes on its own.
 * After this, we keep taking the time but mark the clock unsynchronized.
 *
 * The QT command returns something that looks like this:
 *
 *	18:57:50.263D
 *
 * Note that this particular sample is in 24 hour format, local time
 * (daylight savings time even).  We allow just about anything for
 * this (sigh) since this leaves the clock owner free to set the
 * display mode in whatever way he finds convenient for setting
 * his watch.
 *
 * The QD command returns:
 *
 *	89/10/19/292
 *
 * We actually only use the day-of-the-year here.  We use the year
 * only to determine whether the PST clock thinks the current year
 * has 365 or 366 days in it.
 *
 * At the current writing, this code expects to be using a BSD-style
 * terminal driver.  It will compile code which uses the CLKLDISC
 * line discipline if it thinks this is available, but use cooked
 * mode otherwise.  The cooked mode stuff may not have been tested.
 */

/*
 * Definitions
 */
#define	MAXUNITS	4	/* maximum number of PST units permitted */
#define	PSTDEV	"/dev/pst%d"	/* device we open.  %d is unit number */
#define	NPSTSAMPS	12	/* take 12 PST samples per minute */

/*
 * When the PST clock is operating optimally we want the primary clock
 * distance to come out at 300 ms.  Thus, peer.distance in the PST peer
 * structure is set to 290 ms and we compute delays which are at least
 * 10 ms long.  The following are 290 ms and 10 ms expressed in u_fp format
 */
#define	PSTDISTANCE	0x00004a3d
#define	PSTBASEDELAY	0x0000028f

/*
 * Other constant stuff
 */
#define	PSTPRECISION	(-9)		/* what the heck */
#define	WWVREFID	"WWV\0"
#define	WWVHREFID	"WWVH"
#define	PSTHSREFID	0x7f7f030a	/* 127.127.3.10 refid for hi strata */

/*
 * Parameters for the clock
 */
#define	PSTBAUD		B9600
#ifdef CLKLDISC
#define	PSTMAGIC1	'\r'
#define	PSTMAGIC2	('\r' | 0x80)
#define	PSTEOL		'\r'
#else
#define	PSTEOL		'\n'
#endif

/*
 * Description of clock.  We fill in whether it is a 1010 or 1020,
 * and the firmware revision, using the QV command.
 */
#define	PSTDESCLEN	64
#define	PSTDESCRIPTION	"%s %s (%s) WWV/H Receiver"
#define	PSTDEFDESC	"PSTI/Traconex 10?0 (V??.??) WWV/H Receiver"

/*
 * Length of the PST time code.  This must be the length of the output
 * of the QM command, plus QT, plus QD, plus two spaces.  We make it
 * big just on principle.
 */
#define	PSTCODELEN	(128)

/*
 * Minimum and maximum lengths
 */
#define	PSTMINQVLEN	(16)
#define	PSTMAXQVLEN	(24)

#define	PSTMINQMLEN	(19)
#define	PSTMAXQMLEN	(32)

#define	PSTMINQDLEN	(12)
#define	PSTMAXQDLEN	(12)

#define	PSTMINQTLEN	(14)
#define	PSTMAXQTLEN	(14)

/*
 * It turns out that the QT command does *not* adjust for transmission
 * delays.  Since the QT command returns 15 characters at 9600 baud,
 * the adjustment for this should be 15.6 ms.  We'll default to this,
 * but don't let this stop you from fiddling with the fudge factors
 * to make things come out right
 */
#define	PSTQTFUDGE	0x04000000	/* about 15.6 ms */

/*
 * Default propagation delays.  About right for Toronto
 */
#define	DEFWWVPROP	0x01eb851f	/* about 7.5 ms */
#define	DEFWWVHPROP	0x06c8b439	/* about 26.5 ms */

/*
 * Maximum propagation delay we believe.  125 ms as an l_fp fraction
 */
#define	PSTMAXPROP	0x20000000

/*
 * Default minutes since an update.
 */
#define	DEFMAXFREERUN	(20)

/*
 * Hold time after a leap second occurs
 */
#define	PSTLEAPHOLD	(3*60*60)	/* 3 hours */

/*
 * Hack to avoid excercising the multiplier.  I have no pride.
 */
#define	MULBY10(x)	(((x)<<3) + ((x)<<1))

/*
 * PST unit control structure.
 */
struct pstunit {
	struct peer *peer;		/* associated peer structure */
	struct event psttimer;		/* timeout timer structure */
	struct refclockio pstio;	/* given to the I/O handler */
	l_fp rectimes[NPSTSAMPS];	/* times we received this stuff */
	l_fp reftimes[NPSTSAMPS];	/* times of codes received */
	l_fp lastrec;			/* last receive time */
	l_fp lastref;			/* last reference time */
	char description[PSTDESCLEN];	/* description of clock */
	char lastcode[PSTCODELEN];	/* last code we received */
	u_char lencode;			/* length of the last code */
	u_char nextsample;		/* the next offset expected */
	u_char unit;			/* unit number for this guy */
	u_char state;			/* what we're waiting for */
	s_char station;			/* WWV or WWVH? */
	u_char dontsync;		/* something detected to prevent sync */
	u_char flags;			/* flag byte */
	u_char status;			/* clock status */
	u_char lastevent;		/* last clock event */
	u_char timezone;		/* hour offset to time zone */
	u_char errors;			/* number of errors detected */
	u_char year;			/* year reported by clock */
	u_char month;			/* month, from clock */
	u_char monthday;		/* day, from clock */
	u_char hour;			/* hour of day */
	u_char minute;			/* minute of day */
	u_char second;			/* second of day */
	s_char tzoffset;		/* time zone offset */
	u_char reason;			/* reason for failure */
	u_short millisecond;		/* millisecond of day */
	u_short yearday;		/* day of the year */
	u_short timesincesync;		/* time since radio got sample */
	u_int yearstart;		/* NTP time at year start */
	u_int leapend;			/* time of ending of leap event */
	u_int lastupdate;		/* last time data received */
	u_int polls;			/* number of polls */
	u_int noreply;			/* number of time outs */
	u_int badformat;		/* number of bad format responses */
	u_int baddata;			/* number of invalid time codes */
	u_int timestarted;		/* time we started this */
};

/*
 * States we might be in
 */
#define	STATE_IDLE	0		/* not doing anything in particular */
#define	STATE_QV	1		/* trying to get version */
#define	STATE_QM	2		/* sent QM */
#define	STATE_QD	3		/* sent QD */
#define	STATE_QT	4		/* send QT */

/*
 * Status flags
 */
#define	PST_LEAPYEAR	0x1		/* pst clock thinks it is a leap year */
#define	PST_SIGFAULT	0x2		/* signal fault */
#define	PST_HARDERR	0x4		/* hardware error */
#define	PST_NOTIME	0x8		/* no time available */
#define	PST_WWVH	0x10		/* synchronized to WWVH */
#define	PST_DOQV	0x20		/* get version, reinit delays */
#define	PST_DORESET	0x40		/* reset the clock */

/*
 * The PST often encodes stuff by adding an ASCII '0' to it.  The
 * largest range of values encoded this way is 0 through 31, or '0'
 * through 'O'.  These macroes manipulate these values.
 */
#define	ISVALIDPST(c)	((c) >= '0' && (c) <= 'O')
#define	PSTTOBIN(c)	((int)(c) - '0')
#define	BINTOPST(c)	((char)((c) + '0'))

/*
 * Status bits.  Look at the QM command
 */
#define	SIGFAULT	0x1
#define	HARDFAULT	0x2
#define	OUTOFSPEC	0x4
#define	TIMEAVAILABLE	0x8

/*
 * Module reason codes
 */
#define	QVREASON	20
#define	QMREASON	40
#define	QDREASON	60
#define	QTREASON	80

/*
 * Station i.d. characters in QM output
 */
#define	WWV_CHAR	'C'
#define	WWVH_CHAR	'H'

/*
 * We allow a few errors, but if we get more than 12 seconds behind
 * the schedule we start from sample 0 again.  4 seconds is the minimum
 * time between time out routine executions.
 */
#define	PSTMAXDELAY	12
#define	PSTMINTIMEOUT	4

/*
 * The PST polling schedule.  We poll 12 times per 64 seconds (far too
 * many, but what the heck).  The polls are scheduled to finish in this
 * time with the assumption that the timer is good for no better than
 * 4 second resolution.  If we get too far behind (due to bad samples
 * or no responses) we start over.
 */
struct pstsched {
	u_short nextinterval;
	u_short tooold;
};

static struct pstsched psttab[NPSTSAMPS] = {
	{ 4,	PSTMAXDELAY+1 },
	{ 4,	PSTMAXDELAY+1+4 },
	{ 8,	PSTMAXDELAY+1+4+4 },
	{ 4,	PSTMAXDELAY+1+4+4+8 },
	{ 8,	PSTMAXDELAY+1+4+4+8+4 },
	{ 4,	PSTMAXDELAY+1+4+4+8+4+8 },
	{ 4,	PSTMAXDELAY+1+4+4+8+4+8+4 },
	{ 8,	PSTMAXDELAY+1+4+4+8+4+8+4+4 },
	{ 4,	PSTMAXDELAY+1+4+4+8+4+8+4+4+8 },
	{ 8,	PSTMAXDELAY+1+4+4+8+4+8+4+4+8+4 },
	{ 4,	PSTMAXDELAY+1+4+4+8+4+8+4+4+8+4+8 },
	{ 4,	PSTMAXDELAY+1+4+4+8+4+8+4+4+8+4+8+4 }
};


/*
 * Data space for the unit structures.  Note that we allocate these on
 * the fly, but never give them back.
 */
static struct pstunit *pstunits[MAXUNITS];
static u_char unitinuse[MAXUNITS];

/*
 * Structure to keep processed propagation data in.
 */
struct pst_propagate {
	u_int remainder;	/* left over submillisecond remainder */
	char msbchar;		/* character for high order bits */
	char lsbchar;		/* character for low order bits */
};


/*
 * Keep the fudge factors separately so they can be set even
 * when no clock is configured.
 */
static l_fp wwv_prop_delay[MAXUNITS];
static l_fp wwvh_prop_delay[MAXUNITS];
static struct pst_propagate wwv_prop_data[MAXUNITS];
static struct pst_propagate wwvh_prop_data[MAXUNITS];
static u_char stratumtouse[MAXUNITS];
static u_char sloppyclock[MAXUNITS];
static u_short freerun[MAXUNITS];

/*
 * Pointer to the default description
 */
static char *pstdefdesc = PSTDEFDESC;

/*
 * macro for writing to the clock, printing an error if we fail
 */
#define	pst_send(pst, str, len)		\
	if (write((pst)->pstio.fd, (str), (len)) < 0) \
		pst_write_error((pst))

/*
 * macro for resetting the clock structure to zero
 */
#define	pst_reset(pst) \
	do { \
		pst->nextsample = 0; \
		pst->station = 0; \
		pst->dontsync = 0; \
	} while (0)

/*
 * macro for event reporting
 */
#define	pst_event(pst, evnt_code) \
	do { \
		if ((pst)->status != (u_char)(evnt_code)) \
			pst_do_event((pst), (evnt_code)); \
	} while (0)

/*
 * Imported from the timer module
 */
extern u_int current_time;
extern struct event timerqueue[];

/*
 * Time conversion tables imported from the library
 */
extern u_int msutotsflo[];
extern u_int msutotsfhi[];


/*
 * pst_init - initialize internal PST driver data
 */
void
pst_init()
{
	register int i;
	void pst_compute_delay();

	/*
	 * Just zero the data arrays
	 */
	bzero((char *)pstunits, sizeof pstunits);
	bzero((char *)unitinuse, sizeof unitinuse);

	/*
	 * Initialize fudge factors to default.
	 */
	for (i = 0; i < MAXUNITS; i++) {
		wwv_prop_delay[i].l_ui = 0;
		wwv_prop_delay[i].l_uf = DEFWWVPROP;
		pst_compute_delay(DEFWWVPROP, &wwv_prop_data[i]);
		wwvh_prop_delay[i].l_ui = 0;
		wwvh_prop_delay[i].l_uf = DEFWWVHPROP;
		pst_compute_delay(DEFWWVHPROP, &wwvh_prop_data[i]);
		stratumtouse[i] = 0;
		sloppyclock[i] = 0;
		freerun[i] = DEFMAXFREERUN;
	}
}


/*
 * pst_start - open the PST device and initialize data for processing
 */
int
pst_start(unit, peer)
	u_int unit;
	struct peer *peer;
{
	register struct pstunit *pst;
	register int i;
	int fd;
	int ldisc;
	char pstdev[20];
	struct sgttyb ttyb;
	void pst_timeout();
	void pst_receive();
	extern int io_addclock();
	extern char *emalloc();

	if (unit >= MAXUNITS) {
		syslog(LOG_ERR, "pst clock: unit number %d invalid (max %d)",
		    unit, MAXUNITS-1);
		return 0;
	}
	if (unitinuse[unit]) {
		syslog(LOG_ERR, "pst clock: unit number %d in use", unit);
		return 0;
	}

	/*
	 * Unit okay, attempt to open the device.
	 */
	(void) sprintf(pstdev, PSTDEV, unit);

	if (setsid() < 0) {
	  syslog(LOG_ERR, "pst_clock: setsid failed, errno= %d", errno);
	}
	
	fd = open(pstdev, O_RDWR, 0777);
	if (fd == -1) {
		syslog(LOG_ERR, "pst clock: open of %s failed: %m", pstdev);
		return 0;
	}

	if (ioctl(fd, TIOCSCTTY, 0) < 0) {
	  syslog(LOG_ERR, "pst_clock: ioctl(%s, TIOCSCTTY): %m", pstdev);
	  (void) close (fd);
		return 0;
	}

	/*
	 * Set for exclusive use
	 */
	if (ioctl(fd, TIOCEXCL, (char *)0) < 0) {
		syslog(LOG_ERR, "pst clock: ioctl(%s, TIOCEXCL): %m", pstdev);
		(void) close(fd);
		return 0;
	}

	/*
	 * Set to raw mode
	 */
	ttyb.sg_ispeed = ttyb.sg_ospeed = PSTBAUD;
#ifdef CLKLDISC
	ttyb.sg_erase = PSTMAGIC1;
	ttyb.sg_kill = PSTMAGIC2;
	ttyb.sg_flags = EVENP|ODDP|RAW|CRMOD;
#else
	ttyb.sg_erase = ttyb.sg_kill = 0;
	ttyb.sg_flags = EVENP|ODDP|CRMOD;
#endif
	if (ioctl(fd, TIOCSETP, (char *)&ttyb) < 0) {
		syslog(LOG_ERR, "pst clock: ioctl(%s, TIOCSETP): %m", pstdev);
		return 0;
	}

	/*
	 * Looks like this might succeed.  Find memory for the structure.
	 * Look to see if there are any unused ones, if not we malloc()
	 * one.
	 */
	if (pstunits[unit] != 0) {
		pst = pstunits[unit];	/* The one we want is okay */
	} else {
		for (i = 0; i < MAXUNITS; i++) {
			if (!unitinuse[i] && pstunits[i] != 0)
				break;
		}
		if (i < MAXUNITS) {
			/*
			 * Reclaim this one
			 */
			pst = pstunits[i];
			pstunits[i] = 0;
		} else {
			pst = (struct pstunit *)emalloc(sizeof(struct pstunit));
		}
	}
	bzero((char *)pst, sizeof(struct pstunit));
	pstunits[unit] = pst;

	/*
	 * Set up the structure
	 */
	pst->peer = peer;
	pst->unit = (u_char)unit;
	pst->state = STATE_IDLE;
	pst->flags |= PST_DOQV;
	pst->timestarted = current_time;
	(void) strcpy(pst->description, pstdefdesc);

	pst->psttimer.peer = (struct peer *)pst;
	pst->psttimer.event_handler = pst_timeout;

	pst->pstio.clock_recv = pst_receive;
	pst->pstio.srcclock = (caddr_t)pst;
	pst->pstio.datalen = 0;
	pst->pstio.fd = fd;

	/*
	 * Okay.  Set the line discipline to the clock line discipline,
	 * if we have it, then give it to the I/O code to start receiving
	 * stuff.
	 */
#ifdef CLKLDISC
	ldisc = CLKLDISC;
	if (ioctl(fd, TIOCSETD, (char *)&ldisc) < 0) {
		syslog(LOG_ERR, "pst clock: ioctl(%s, TIOCSETD): %m", pstdev);
		(void) close(fd);
		return 0;
	}
#else
	ldisc = 0;
	if (ioctl(fd, TIOCFLUSH, (char *)&ldisc) < 0) {
		syslog(LOG_ERR, "pst clock: ioctl(%s, TIOCFLUSH): %m", pstdev);
		(void) close(fd);
		return 0;
	}
#endif
	if (!io_addclock(&pst->pstio)) {
		/*
		 *  Just close and return.
		 */
		(void) close(fd);
		return 0;
	}


	/*
	 * All done.  Initialize a few random peer variables, then
	 * start the timer and return success.
	 */
	peer->distance = PSTDISTANCE;
	peer->precision = PSTPRECISION;
	peer->stratum = stratumtouse[unit];
	if (stratumtouse[unit] <= 1)
		bcopy(WWVREFID, (char *)&peer->refid, 4);
	else
		peer->refid = htonl(PSTHSREFID);
	pst->psttimer.event_time = current_time + PSTMINTIMEOUT;
	TIMER_ENQUEUE(timerqueue, &pst->psttimer);
	unitinuse[unit] = 1;
	return 1;
}


/*
 * pst_shutdown - shut down a PST clock
 */
void
pst_shutdown(unit)
	int unit;
{
	register struct pstunit *pst;
	extern void io_closeclock();

	if (unit >= MAXUNITS) {
		syslog(LOG_ERR,
		  "pst clock: INTERNAL ERROR, unit number %d invalid (max %d)",
		    unit, MAXUNITS-1);
		return;
	}
	if (!unitinuse[unit]) {
		syslog(LOG_ERR,
		 "pst clock: INTERNAL ERROR, unit number %d not in use", unit);
		return;
	}

	/*
	 * Tell the I/O module to turn us off, and dequeue timer
	 * if any.  We're history.
	 */
	pst = pstunits[unit];
	TIMER_DEQUEUE(&pst->psttimer);
	io_closeclock(&pst->pstio);
	unitinuse[unit] = 0;
}


/*
 * pst_write_error - complain about writes to the clock
 */
static void
pst_write_error(pst)
	struct pstunit *pst;
{
	/*
	 * This will fill syslog is something is really wrong.  Should
	 * throttle it back.
	 */
	syslog(LOG_ERR, "pst clock: write error on unit %d: %m",
	    pst->unit);
}


/*
 * pst_timeout - process a timeout event
 */
void
pst_timeout(fakepeer)
	struct peer *fakepeer;
{
	register struct pstunit *pst;
	u_int poll;

	/*
	 * The timeout routine always initiates a chain of
	 * query-responses from the clock, by sending either
	 * a QV command (if we need to (re)set the propagation
	 * delays into the clock), a QM command or an SRY
	 * command (after a leap second).  The pst_receive()
	 * routine should complete the set of queries on its own
	 * long before the next time out is due, so if we see any
	 * state in here other than idle it means the clock hasn't
	 * responded.
	 */
	pst = (struct pstunit *)fakepeer;
	switch(pst->state) {
	case STATE_IDLE:
		poll = (u_int)psttab[pst->nextsample].nextinterval;
		break;				/* all is well */

	case STATE_QV:
		pst->flags |= PST_DOQV;		/* no response, do QV again */
		/*FALLSTHROUGH*/

	case STATE_QM:
	case STATE_QD:
	case STATE_QT:
		pst->noreply++;			/* mark the lack of response */
		poll = PSTMINTIMEOUT;		/* minimum time poll */
		break;

	default:
		syslog(LOG_ERR,
		    "pst clock: INTERNAL ERROR unit %d invalid state %d",
		    pst->unit, pst->state);
		poll = PSTMINTIMEOUT;		/* minimum time poll */
		break;
	}

	if (pst->flags & PST_DORESET) {
		/*
		 * Do a reset.  At the next interrupt, start with
		 * a QV command to set in the delays.
		 */
		pst->flags &= ~PST_DORESET;
		pst->flags |= PST_DOQV;
		pst->state = STATE_IDLE;
		pst_send(pst, "\003SRY", 4);
	} else if (pst->flags & PST_DOQV) {
		pst->polls++;
		pst->flags &= ~PST_DOQV;
		pst->state = STATE_QV;
		pst_send(pst, "\003QV", 3);
	} else {
		pst->polls++;
		pst->state = STATE_QM;
		pst_send(pst, "\003QM", 3);
	}

	pst->psttimer.event_time += poll;
	TIMER_ENQUEUE(timerqueue, &pst->psttimer);
}


/*
 * pst_QV_process - decode the results of a QV poll and insert fudge
 *		    factors into the clock.
 */
static int
pst_QV_process(pst, rbufp)
	register struct pstunit *pst;
	struct recvbuf *rbufp;
{
	register char *cp;
	register char *bp;
	register int len;
	char *model;
	char *company;
	char buf[20];
	static char wwvdelay[6] = { 'S', 'C', '\0', 'S', 'E', '\0' };
	static char wwvhdelay[6] = { 'S', 'H', '\0', 'S', 'G', '\0' };

	/*
	 * The output of the QV command looks like:
	 *
	 * PSTI ITS V04.01.000\r
	 *
	 * or
	 *
	 * TRAC ITS V04.01.000\r
	 *
	 * The minimum length of the string is about 16 characters.
	 * The maximum length is sort of unbounded, but we get suspicious
	 * if it is more than 34.
	 */
	len = rbufp->recv_length;
	if (len > PSTMAXQVLEN + 10)
		len = PSTMAXQVLEN + 10;
	
	bp = rbufp->recv_buffer;
	cp = pst->lastcode;
	while (len-- > 0) {
		*cp = (*bp++) & 0x7f;	/* strip parity */
		if (!isprint(*cp))
			break;
		cp++;
	}
	pst->lencode = (u_char)(cp - pst->lastcode);

	/*
	 * Okay, got all printable characters from the string
	 * copied.  We expect to have been terminated by the
	 * EOL character.  If not, forget it.  If the length
	 * is insane, forget it.
	 */
	/* if (len == 0 || *cp != PSTEOL */
	   if (*cp != PSTEOL
	    || pst->lencode < PSTMINQVLEN || pst->lencode > PSTMAXQVLEN) {
		pst->reason = QVREASON + 1;
		return 0;
	}

	/*
	 * Now, format check what we can.  Dump it at the least
	 * sign of trouble.
	 */
	cp = pst->lastcode;
	if (*cp++ != 'P' || *cp++ != 'S' || *cp++ != 'T'
	    || *cp++ != 'I' || *cp++ != ' ') {
		cp = pst->lastcode;
		if (*cp++ != 'T' || *cp++ != 'R' || *cp++ != 'A'
		    || *cp++ != 'C' || *cp++ != ' ') {
			pst->reason = QVREASON + 2;
			return 0;
		}
		company = "Traconex";
	} else {
		company = "Precision Standard Time";
	}

	if (*cp == 'M')
		model = "1010";
	else if (*cp == 'I')
		model = "1020";
	else {
		pst->reason = QVREASON + 3;
		return 0;
	}
	cp++;

	if (*cp++ != 'T' || *cp++ != 'S' || *cp++ != ' ') {
		pst->reason = QVREASON + 4;
		return 0;
	}
	if (*cp != 'X' && *cp != 'V') {
		pst->reason = QVREASON + 5;
		return 0;
	}
	
	/*
	 * Next is the version.  Copy it into the buffer.
	 */
	bp = buf;
	*bp++ = *cp++;
	while (isdigit(*cp) || *cp == '.')
		*bp++ = *cp++;
	*bp++ = '\0';

	/*
	 * Final bit of fluff is to set the description
	 */
	(void) sprintf(pst->description, PSTDESCRIPTION, company, model, buf);

	/*
	 * Now the serious stuff.  Since we are now sure that the
	 * clock is there, we can be fairly sure that the delay
	 * setting commands will take.  Send them.
	 */
	wwvdelay[2] = wwv_prop_data[pst->unit].msbchar;
	wwvdelay[5] = wwv_prop_data[pst->unit].lsbchar;
	pst_send(pst, wwvdelay, 6);

	/*
	 * Same thing for WWVH
	 */
	wwvhdelay[2] = wwvh_prop_data[pst->unit].msbchar;
	wwvhdelay[5] = wwvh_prop_data[pst->unit].lsbchar;
	pst_send(pst, wwvhdelay, 6);

	/*
	 * Should be okay.  Return positive response.
	 */
	return 1;
}


/*
 * pst_QM_process - process the output of a QM command
 */
static int
pst_QM_process(pst, rbufp)
	register struct pstunit *pst;
	struct recvbuf *rbufp;
{
	register char *cp;
	register char *bp;
	register int n;

	/*
	 * The output of the QM command looks like:
	 *
 	 * O6B532352823C00270322
	 *
	 * The minimum length of the string is 19 characters.
	 * The maximum length is sort of unbounded, but we get suspicious
	 * if it is more than 42.
	 */
	n = rbufp->recv_length;
	if (n > PSTMAXQMLEN + 10)
		n = PSTMAXQMLEN + 10;
	
	bp = rbufp->recv_buffer;
	cp = pst->lastcode;
	while (n-- > 0) {
		*cp = (*bp++) & 0x7f;	/* strip parity */
		if (!isprint(*cp))
			break;
		cp++;
	}
	pst->lencode = (u_char)(cp - pst->lastcode);

	/*
	 * Okay, got all printable characters from the string
	 * copied.  We expect to have been terminated by the
	 * EOL character.  If not, forget it.  If the length
	 * is insane, forget it.
	 */
	/* if (n == 0 || *cp != PSTEOL */
	   if (*cp != PSTEOL
	    || pst->lencode < PSTMINQMLEN || pst->lencode > PSTMAXQMLEN) {
		pst->reason = QMREASON + 1;
		return 0;
	}

	/*
	 * Ensure that the first PSTMINQMLEN characters are valid with
	 * respect to the way the clock encodes binary data.
	 
	cp = pst->lastcode;
	n = pst->lencode;
	while (n-- > 0) {
		if (!ISVALIDPST(*cp)) {
			pst->reason = QMREASON + 2;
			return 0;
		}
		cp++;
	}
	*/
	
	/*
	 * Collect information we are interested in.
	 */
	cp = pst->lastcode;
	pst->timezone = PSTTOBIN(cp[3]);
	if (pst->timezone > 23) {
		pst->reason = QMREASON + 3;
		return 0;
	}

	pst->flags &=
	    ~(PST_LEAPYEAR|PST_SIGFAULT|PST_HARDERR|PST_NOTIME|PST_WWVH);
	n = PSTTOBIN(cp[4]);
	if (n > 15) {
		pst->reason = QMREASON + 4;
		return 0;
	}
	if (((n + 2) & 0x3) == 0)
		pst->flags |= PST_LEAPYEAR;

	n = PSTTOBIN(cp[9]);
	if (n > 15) {
		pst->reason = QMREASON + 5;
		return 0;
	}
	if (n & SIGFAULT)
		pst->flags |= PST_SIGFAULT;
	if (n & HARDFAULT)
		pst->flags |= PST_HARDERR;
	if (!(n & TIMEAVAILABLE))
		pst->flags |= PST_NOTIME;

	if (cp[12] == 'H') {
		pst->flags |= PST_WWVH;
	} else if (cp[12] == 'C') {
		pst->flags &= ~PST_WWVH;
	} else {
		pst->reason = QMREASON + 6;
		return 0;
	}

	if (wwv_prop_data[pst->unit].msbchar != cp[5] ||
	    wwv_prop_data[pst->unit].lsbchar != cp[6] ||
	    wwvh_prop_data[pst->unit].msbchar != cp[7] ||
	    wwvh_prop_data[pst->unit].lsbchar != cp[8])
		pst->flags |= PST_DOQV;

	bp = cp + 13;
	pst->timesincesync = 0;
	while (bp < (cp + 17)) {
		if (!isdigit(*bp)) {
			pst->reason = QMREASON + 6;
			return 0;
		}
		pst->timesincesync = MULBY10(pst->timesincesync)
		    + PSTTOBIN(*bp);
		bp++;
	}

	/*
	 * That's about all we can do.  Return success.
	 */
	return 1;
}


/*
 * pst_QD_process - process the output of a QD command
 */
static int
pst_QD_process(pst, rbufp)
	register struct pstunit *pst;
	struct recvbuf *rbufp;
{
	register char *cp;
	register char *bp;
	register int n;
	char *cpstart;
	int len;

	/*
	 * The output of the QM command looks like:
	 *
 	 * 88/05/17/138\r
	 *
	 * The minimum length of the string is 12 characters as is
	 * the maximum length.
	 */
	n = rbufp->recv_length;
	if (n > PSTMAXQDLEN + 10)
		n = PSTMAXQDLEN + 10;
	
	bp = rbufp->recv_buffer;
	cp = &pst->lastcode[pst->lencode];
	*cp++ = ' ';
	cpstart = cp;
	while (n-- > 0) {
		*cp = (*bp++) & 0x7f;	/* strip parity */
		if (!isprint(*cp))
			break;
		cp++;
	}
	len = (cp - cpstart);
	pst->lencode = (u_char)(cp - pst->lastcode);

	/*
	 * Okay, got all printable characters from the string
	 * copied.  We expect to have been terminated by the
	 * EOL character.  If not, forget it.  If the length
	 * is insane, forget it.
	 */
	/* if (n == 0 || *cp != PSTEOL || */
	   if (*cp != PSTEOL ||
	    len < PSTMINQDLEN || len > PSTMAXQDLEN) {
		pst->reason = QDREASON + 1;
		return 0;
	}

	/*
	 * Ensure that the characters are formatted validly.  They
	 * are either digits or '/'s.
	 */
	cp = cpstart;
	if (!isdigit(cp[0]) || !isdigit(cp[1]) || cp[2] != '/' ||
	    !isdigit(cp[3]) || !isdigit(cp[4]) || cp[5] != '/' ||
	    !isdigit(cp[6]) || !isdigit(cp[7]) || cp[8] != '/' ||
	    !isdigit(cp[9]) || !isdigit(cp[10]) || !isdigit(cp[11])) {
		pst->reason = QDREASON + 2;
		return 0;
	}

	/*
	 * Decode into year, month, day and year day
	 */
	pst->year = MULBY10(PSTTOBIN(cp[0])) + PSTTOBIN(cp[1]);
	pst->month = MULBY10(PSTTOBIN(cp[3])) + PSTTOBIN(cp[4]);
	pst->monthday = MULBY10(PSTTOBIN(cp[6])) + PSTTOBIN(cp[7]);
	pst->yearday = MULBY10(PSTTOBIN(cp[9])) + PSTTOBIN(cp[10]);
	pst->yearday = MULBY10(pst->yearday) + PSTTOBIN(cp[11]);

	/*
	 * Format check these.
	 */
	if (pst->month > 12 || pst->monthday > 31 || pst->yearday > 366) {
		pst->reason = QDREASON + 3;
		return 0;
	}
	if (!(pst->flags & PST_LEAPYEAR) && pst->yearday > 365) {
		pst->reason = QDREASON + 4;
		return 0;
	}

	/*
	 * Done all we can.
	 */
	return 1;
}


/*
 * pst_QT_process - process the output of a QT command, return the times
 */
static int
pst_QT_process(pst, rbufp, tsclk, tsrec)
	register struct pstunit *pst;
	struct recvbuf *rbufp;
	l_fp *tsclk;
	l_fp *tsrec;
{
	register char *cp;
	register char *bp;
	register int n;
	char *cpstart;
	int len;
	int hour;
	int minute;
	int second;
	int msec;
	int tzoff;
	extern int buftvtots();

	/*
	 * The output of the QT command looks like:
	 *
 	 * A09:57:50.263D
 	 * 
	 * The minimum length of the string is 14 characters as is
	 * the maximum length.
	 */
	n = rbufp->recv_length;
	if (n > PSTMAXQTLEN + 10)
		n = PSTMAXQTLEN + 10;
	
	bp = rbufp->recv_buffer;
	cp = &pst->lastcode[pst->lencode];
	*cp++ = ' ';
	cpstart = cp;
	while (n-- > 0) {
		*cp = (*bp++) & 0x7f;	/* strip parity */
		if (!isprint(*cp))
			break;
		cp++;
	}
	len = (cp - cpstart);
	pst->lencode = (u_char)(cp - pst->lastcode);

	/*
	 * Okay, got all printable characters from the string
	 * copied.  We expect to have been terminated by the
	 * EOL character.  If not, forget it.  If the length
	 * is insane, forget it.
	 */
	/* if (n == 0 || *cp != PSTEOL || */
	   if (*cp != PSTEOL ||
	    len < PSTMINQTLEN || len > PSTMAXQTLEN) {
		pst->reason = QTREASON + 1;
		return 0;
	}
#ifdef CLKLDISC
	/*
	 * Receive time stamp should be in buffer after the code.
	 * Make sure we have enough characters in there.
	 */
	if (&rbufp->recv_buffer[rbufp->recv_length] - bp < 8) {
		pst->reason = QTREASON + 2;
		return 0;
	}
	if (!buftvtots(bp, tsrec)) {
		pst->reason = QTREASON + 3;
		return 0;
	}
#else
	/*
	 * Use the timestamp collected with the input.
	 */
	*tsrec = rbufp->recv_time;
#endif

	/*
	 * Ensure that the characters are formatted validly.  Mostly
	 * digits, but the occasional `:' and `.'.
	 */
	cp = cpstart;
	if (!isdigit(cp[1]) || !isdigit(cp[2]) || cp[3] != ':' ||
	    !isdigit(cp[4]) || !isdigit(cp[5]) || cp[6] != ':' ||
	    !isdigit(cp[7]) || !isdigit(cp[8]) || cp[9] != '.' ||
	    !isdigit(cp[10]) || !isdigit(cp[11]) || !isdigit(cp[12])) {
		pst->reason = QTREASON + 4;
		return 0;
	}

	/*
	 * Extract the hour, minute, second and millisecond
	 */
	hour = MULBY10(PSTTOBIN(cp[1])) + PSTTOBIN(cp[2]);
	minute = MULBY10(PSTTOBIN(cp[4])) + PSTTOBIN(cp[5]);
	second = MULBY10(PSTTOBIN(cp[7])) + PSTTOBIN(cp[8]);
	msec = MULBY10(PSTTOBIN(cp[10])) + PSTTOBIN(cp[11]);
	msec = MULBY10(msec) + PSTTOBIN(cp[12]);

	if (minute > 59 || second > 59) {
		pst->reason = QTREASON + 5;
		return 0;
	}

	/*
	 * Trouble here.  Adjust the hours for AM/PM, if this is
	 * on, and for daylight saving time.
	 */
	if (*cp == 'A') {
		if (hour > 12 || hour == 0) {
			pst->reason = QTREASON + 5;
			return 0;
		}
		if (hour == 12)
			hour = 0;
	} else if (*cp == 'P') {
		if (hour > 12 || hour == 0)
			return 0;
		if (hour < 12)
			hour += 12;
	} else if (*cp != ' ') {
		pst->reason = QTREASON + 6;
		return 0;
	}

	if (cp[13] == 'D')
		tzoff = -1;
	else if (cp[13] == ' ')
		tzoff = 0;
	else {
		pst->reason = QTREASON + 7;
		return 0;
	}

	/*
	 * Adjust for the timezone.  The PST manual is screwy here.
	 * it says the timezone is an integer in the range 0 to 23,
	 * but this doesn't allow us to tell the difference between
	 * +12 and -12.  Assume the 12 hour timezone is west of
	 * GMT.
	 */
	if (pst->timezone <= 12)
		tzoff += pst->timezone;
	else
		tzoff -= (24 - pst->timezone);


	/*
	 * Record for posterity
	 */
	pst->hour = (u_char)hour;
	pst->minute = (u_char)minute;
	pst->second = (u_char)second;
	pst->millisecond = (u_short)msec;
	pst->tzoffset = (s_char)tzoff;

	/*
	 * All that to get the day-hour-minute-second.  Turn this
	 * into the seconds part of a time stamp.  Also use the
	 * milliseconds part directly as the fractional part.
	 */
	MSUTOTSF(msec, tsclk->l_uf);
	if (!clocktime((int)pst->yearday, hour, minute, second, tzoff,
	    tsrec->l_ui, &pst->yearstart, &tsclk->l_ui)) {
		pst->reason = QTREASON + 8;
		return 0;
	}

	/*
	 * Add in the fudge
	 */
	if (pst->flags & PST_WWVH)
		L_ADDUF(tsclk, wwvh_prop_data[pst->unit].remainder);
	else
		L_ADDUF(tsclk, wwv_prop_data[pst->unit].remainder);

	/*
	 * Glad that's over with
	 */
	return 1;
}


/*
 * pst_do_event - update our status and report any changes
 */
static void
pst_do_event(pst, evnt_code)
	register struct pstunit *pst;
	int evnt_code;
{
	if (pst->status != (u_char)evnt_code) {
		pst->status = (u_char)evnt_code;
		if (evnt_code != CEVNT_NOMINAL)
			pst->lastevent = (u_char)evnt_code;
		/*
		 * Should trap this, but the trap code isn't up to
		 * it yet.
		 */
	}
}



/*
 * pst_process - process the data collected to produce an offset estimate
 */
static void
pst_process(pst)
	register struct pstunit *pst;
{
	register int i;
	register int n;
	register u_int tmp_ui;
	register u_int tmp_uf;
	register u_int date_ui;
	register u_int date_uf;
	u_fp delay;
	l_fp off[NPSTSAMPS];
	extern void refclock_receive();

	/*
	 * Compute offsets from the raw data.  Sort them into
	 * ascending order.
	 */
	for (i = 0; i < NPSTSAMPS; i++) {
		tmp_ui = pst->reftimes[i].l_ui;
		tmp_uf = pst->reftimes[i].l_uf;
		M_SUB(tmp_ui, tmp_uf, pst->rectimes[i].l_ui,
		    pst->rectimes[i].l_uf);
		for (n = i; n > 0; n--) {
			if (M_ISGEQ(tmp_ui, tmp_uf, off[n-1].l_ui,
			    off[n-1].l_uf))
				break;
			off[n] = off[n-1];
		}
		off[n].l_ui = tmp_ui;
		off[n].l_uf = tmp_uf;
	}

	/*
	 * Reject the furthest from the median until 8 samples left
	 */
	i = 0;
	n = NPSTSAMPS;
	while ((n - i) > 8) {
		tmp_ui = off[n-1].l_ui;
		tmp_uf = off[n-1].l_uf;
		date_ui = off[(n+i)/2].l_ui;
		date_uf = off[(n+i)/2].l_uf;
		M_SUB(tmp_ui, tmp_uf, date_ui, date_uf);
		M_SUB(date_ui, date_uf, off[i].l_ui, off[i].l_uf);
		if (M_ISHIS(date_ui, date_uf, tmp_ui, tmp_uf)) {
			/*
			 * reject low end
			 */
			i++;
		} else {
			/*
			 * reject high end
			 */
			n--;
		}
	}

	/*
	 * Compute the delay based on the difference between the
	 * extremes of the remaining offsets.
	 */
	tmp_ui = off[n-1].l_ui;
	tmp_uf = off[n-1].l_uf;
	M_SUB(tmp_ui, tmp_uf, off[i].l_ui, off[i].l_uf);
	delay = PSTBASEDELAY + MFPTOFP(tmp_ui, tmp_uf);

	/*
	 * Now compute the offset estimate.  If the sloppy clock
	 * flag is set, average the remainder, otherwise pick the
	 * median.
	 */
	if (sloppyclock[pst->unit]) {
		tmp_ui = tmp_uf = 0;
		while (i < n) {
			M_ADD(tmp_ui, tmp_uf, off[i].l_ui, off[i].l_uf);
			i++;
		}
		M_RSHIFT(tmp_ui, tmp_uf);
		M_RSHIFT(tmp_ui, tmp_uf);
		M_RSHIFT(tmp_ui, tmp_uf);
		i = 0;
		off[0].l_ui = tmp_ui;
		off[0].l_uf = tmp_uf;
	} else {
		i = (n+i)/2;
	}

	/*
	 * Add the default PST QT delay into this.
	 */
	L_ADDUF(&off[i], PSTQTFUDGE);

	/*
	 * Set the reference ID to the appropriate station
	 */
	if (stratumtouse[pst->unit] <= 1) {
		if (pst->station >= 0)
			bcopy(WWVREFID, (char *)&pst->peer->refid, 4);
		else
			bcopy(WWVHREFID, (char *)&pst->peer->refid, 4);
	}

	/*
	 * Give the data to the reference clock support code
	 */
	refclock_receive(pst->peer, &off[i], delay, &pst->reftimes[NPSTSAMPS-1],
	    &pst->rectimes[NPSTSAMPS-1], (pst->dontsync == 0));

	/*
	 * If the don't-sync flag isn't on, we're nominal.
	 */
	if (pst->dontsync == 0)
		pst_event(pst, CEVNT_NOMINAL);
	pst_reset(pst);
}



/*
 * pst_receive - receive data from a PST clock, call the appropriate
 *		 routine to process it, and advance the state.
 */
void
pst_receive(rbufp)
	struct recvbuf *rbufp;
{
	register struct pstunit *pst;
	register u_int tmp;
	void pst_process();

	pst = (struct pstunit *)rbufp->recv_srcclock;

	/*
	 * Process based on the current state.
	 */
	switch(pst->state) {
	case STATE_IDLE:
		return;			/* Ignore the input */

	case STATE_QV:
		if (!pst_QV_process(pst, rbufp)) {
			/*
			 * Set the state to idle, but request another
			 * QV poll.
			 */
			pst->badformat++;
			pst_event(pst, CEVNT_BADREPLY);
			pst->state = STATE_IDLE;
			pst->flags |= PST_DOQV;
		} else {
			/*
			 * This went okay.  Advance the state to
			 * QM and send the request.
			 */
			pst->state = STATE_QM;
			pst_send(pst, "QM", 2);
		}
		return;

	case STATE_QM:
		if (!pst_QM_process(pst, rbufp)) {
			/*
			 * Idle us and note the error
			 */
			pst->badformat++;
			pst_event(pst, CEVNT_BADREPLY);
			pst->state = STATE_IDLE;
			return;
		}
		if (pst->flags & PST_NOTIME) {
			/*
			 * Here we aren't getting any time because the
			 * clock is still searching.  Don't bother
			 * looking for anything.  Remove any leap
			 * second hold, however, since this should
			 * ensure the clock is sensible.
			 */
			pst_event(pst, CEVNT_FAULT);
			pst->state = STATE_IDLE;
			pst->leapend = 0;
			if (pst->nextsample > 0)
				pst_reset(pst);		/* Make sure rate low */
			return;
		}

		/*
		 * Next is QD.  Do it.
		 */
		pst->state = STATE_QD;
		pst_send(pst, "QD", 2);
		return;

	case STATE_QD:
		if (!pst_QD_process(pst, rbufp)) {
			/*
			 * Idle us and note the error
			 */
			pst->badformat++;
			pst_event(pst, CEVNT_BADDATE);
			pst->state = STATE_IDLE;
		} else {
			/*
			 * Last step is QT.
			 */
			pst->state = STATE_QT;
			pst_send(pst, "QT", 2);
		}
		return;

	case STATE_QT:
		pst->state = STATE_IDLE;
		if (!pst_QT_process(pst, rbufp, &pst->lastref, &pst->lastrec)) {
			/*
			 * Note the error
			 */
			pst->baddata++;
			pst_event(pst, CEVNT_BADTIME);
			return;
		}
		break;

	default:
		syslog(LOG_ERR,
	"pst clock: INTERNAL ERROR invalid state %d, unit %d, in receive",
		    pst->state, pst->unit);
		return;
	}


	/*
	 * You may not have noticed this, but the only way we end up
	 * out here is if we've completed polling and have a couple of
	 * valid time stamps.  First see if we should reset the
	 * structure.
	 */
	if (pst->nextsample > 0) {
		tmp = pst->lastrec.l_ui - pst->rectimes[0].l_ui;
		if (tmp > (u_int)psttab[pst->nextsample].tooold)
			pst_reset(pst);
	}

	pst->rectimes[pst->nextsample] = pst->lastrec;
	pst->reftimes[pst->nextsample] = pst->lastref;
	pst->nextsample++;
	if (pst->flags & PST_WWVH)
		pst->station--;
	else
		pst->station++;

	if (pst->flags & (PST_SIGFAULT|PST_HARDERR)) {
		pst_event(pst, CEVNT_FAULT);
		pst->dontsync++;
	} else if (pst->timesincesync > freerun[pst->unit]) {
		pst_event(pst, CEVNT_PROP);
		pst->dontsync++;
	} else if (pst->leapend > current_time) {
		pst->dontsync++;
	}

	if (pst->nextsample >= NPSTSAMPS)
		pst_process(pst);
}


/*
 * pst_leap - called when a leap second occurs
 */
void
pst_leap()
{
	register int i;

	/*
	 * This routine should be entered a few seconds after
	 * midnight UTC when a leap second occurs.  To ensure we
	 * don't get foolish time from the clock(s) we reset it
	 * (them).  We also set a 3 hour hold on each clock in
	 * case the reset doesn't take, though this will be canceled
	 * if the reset succeeds.
	 */
	for (i = 0; i < MAXUNITS; i++) {
		if (unitinuse[i]) {
			pstunits[i]->leapend = current_time + PSTLEAPHOLD;
			pstunits[i]->flags |= PST_DORESET;
		}
	}
}


/*
 * pst_compute_delay - compute appropriate things to tell clock about delays
 */
void
pst_compute_delay(prop_delay, prop_data)
	u_int prop_delay;
	struct pst_propagate *prop_data;
{
	register int code;
	register u_int tsf;
	extern int tsftomsu();

	/*
	 * Convert (truncate) the delay to milliseconds.  Save the
	 * characters needed to send this to the clock and compute
	 * the remainder to be added in later.
	 */
	code = tsftomsu(prop_delay, 0);
	MSUTOTSF(code, tsf);
	prop_data->remainder = prop_delay - tsf;
	if (prop_data->remainder & 0x80000000)
		prop_data->remainder = 0;
	prop_data->msbchar = BINTOPST((code >> 2) & 0x1f);
	prop_data->lsbchar = BINTOPST(code & 0x3);
}


/*
 * pst_control - set fudge factors, return statistics
 */
void
pst_control(unit, in, out)
	u_int unit;
	struct refclockstat *in;
	struct refclockstat *out;
{
	register struct pstunit *pst;
	void pst_compute_delay();

	if (unit >= MAXUNITS) {
		syslog(LOG_ERR, "pst clock: unit %d invalid (max %d)",
		    unit, MAXUNITS-1);
		return;
	}

	if (in != 0) {
		int doqv = 0;

		if (in->haveflags & CLK_HAVETIME1)
			if (in->fudgetime1.l_ui == 0
			    && in->fudgetime1.l_uf <= PSTMAXPROP) {
				wwv_prop_delay[unit] = in->fudgetime1;
				doqv = 1;
				pst_compute_delay(wwv_prop_delay[unit].l_uf,
				    &wwv_prop_data[unit]);
			}
		if (in->haveflags & CLK_HAVETIME2)
			if (in->fudgetime2.l_ui == 0
			    && in->fudgetime2.l_uf <= PSTMAXPROP) {
				wwvh_prop_delay[unit] = in->fudgetime2;
				doqv = 1;
				pst_compute_delay(wwvh_prop_delay[unit].l_uf,
				    &wwvh_prop_data[unit]);
			}
		if (in->haveflags & CLK_HAVEVAL1) {
			stratumtouse[unit] = (u_char)(in->fudgeval1 & 0xf);
		}
		if (in->haveflags & CLK_HAVEVAL2) {
			if (in->fudgeval2 > 0 && in->fudgeval2 < 9990)
				freerun[unit] = (u_short)in->fudgeval2;
		}
		if (in->haveflags & CLK_HAVEFLAG1) {
			sloppyclock[unit] = in->flags & CLK_FLAG1;
		}
		if (unitinuse[unit]) {
			/*
			 * Should actually reselect clock, but
			 * will wait for the next timecode
			 */
			if (in->haveflags & CLK_HAVEVAL1) {
				pstunits[unit]->peer->stratum
				    = stratumtouse[unit];
				if (stratumtouse[unit] > 1)
					pstunits[unit]->peer->refid
					    = htonl(PSTHSREFID);
			}

			if ((in->haveflags & CLK_HAVEFLAG3) &&
			    (in->flags & CLK_FLAG3)) {
				pstunits[unit]->flags |= PST_DORESET;
			} else if (doqv || ((in->haveflags & CLK_HAVEFLAG2) &&
			    (in->flags & CLK_FLAG2))) {
				pstunits[unit]->flags |= PST_DOQV;
			}
		}
	}

	if (out != 0) {
		out->type = REFCLK_WWV_PST;
		out->flags = 0;
		out->haveflags
		    = CLK_HAVETIME1|CLK_HAVETIME2|CLK_HAVEVAL1|
		      CLK_HAVEVAL2|CLK_HAVEFLAG1;
		out->fudgetime1 = wwv_prop_delay[unit];
		out->fudgetime2 = wwvh_prop_delay[unit];
		out->fudgeval1 = (int)stratumtouse[unit];
		out->fudgeval2 = (int)freerun[unit];
		out->flags = sloppyclock[unit];
		if (unitinuse[unit]) {
			pst = pstunits[unit];
			out->clockdesc = pst->description;
			out->lencode = pst->lencode;
			out->lastcode = pst->lastcode;
			out->timereset = current_time - pst->timestarted;
			out->polls = pst->polls;
			out->noresponse = pst->noreply;
			out->badformat = pst->badformat;
			out->baddata = pst->baddata;
			out->lastevent = pst->lastevent;
			out->currentstatus = pst->status;
		} else {
			out->clockdesc = pstdefdesc;
			out->lencode = 0;
			out->lastcode = "";
			out->polls = out->noresponse = 0;
			out->badformat = out->baddata = 0;
			out->timereset = 0;
			out->currentstatus = out->lastevent = CEVNT_NOMINAL;
		}
	}
}


/*
 * pst_buginfo - return clock dependent debugging info
 */
void
pst_buginfo(unit, bug)
	int unit;
	register struct refclockbug *bug;
{
	register struct pstunit *pst;
	register int i;

	bug->nvalues = bug->ntimes = 0;

	if (unit >= MAXUNITS) {
		syslog(LOG_ERR, "pst clock: unit %d invalid (max %d)",
		    unit, MAXUNITS-1);
		return;
	}

	if (!unitinuse[unit])
		return;
	pst = pstunits[unit];

	bug->nvalues = 14;
	bug->svalues = (1<<10);
	bug->values[0] = (u_int)pst->nextsample;
	bug->values[1] = (u_int)pst->state;
	bug->values[2] = (u_int)pst->reason;
	bug->values[3] = (u_int)pst->flags;
	bug->values[4] = (u_int)pst->yearday;
	bug->values[5] = (u_int)pst->hour;
	bug->values[6] = (u_int)pst->minute;
	bug->values[7] = (u_int)pst->second;
	bug->values[8] = (u_int)pst->millisecond;
	bug->values[9] = (u_int)pst->timezone;
	bug->values[10] = (u_int)((int)pst->tzoffset);
	bug->values[11] = (u_int)pst->timesincesync;
	bug->values[12] = pst->yearstart;
	if (pst->leapend > current_time)
		bug->values[13] = pst->leapend - current_time;
	else
		bug->values[13] = 0;

	bug->ntimes = ((NPSTSAMPS*2)+2) > NCLKBUGTIMES ? NCLKBUGTIMES :
	    ((NPSTSAMPS*2)+2);
	bug->stimes = 0;
	for (i = 0; i < (bug->ntimes-2)/2; i++) {
		bug->times[2*i] = pst->rectimes[i];
		bug->times[(2*i) + 1] = pst->reftimes[i];
	}
	bug->times[bug->ntimes - 2] = pst->lastrec;
	bug->times[bug->ntimes - 1] = pst->lastref;
}
#endif

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
static char     *sccsid = "@(#)$RCSfile: ntp_refclock.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 16:58:00 $";
#endif
/*
 */

/*
 * ntp_refclock - processing support for reference clocks
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>
#include <ntp/ntp_refclock.h>

#ifdef REFCLOCK
/*
 * Reference clock support is provided here by maintaining the
 * fiction that the clock is actually a peer.  As no packets are
 * exchanged with a reference clock, however, we replace the
 * transmit, receive and packet procedures with separate code
 * to simulate them.  Refclock_transmit and refclock_receive
 * maintain the peer variables in a state analogous to an
 * actual peer and pass reference clock data on through the
 * filters.  Refclock_peer and refclock_unpeer are called to
 * initialize and terminate reference clock associations.
 */

/*
 * The refclock configuration table.  Imported from refclock_conf.c
 */
extern struct refclock refclock_conf[];
extern int num_refclock_conf;

/*
 * Imported from the I/O module
 */
extern struct interface *any_interface;
extern struct interface *loopback_interface;

/*
 * Imported from the timer module
 */
extern u_int current_time;
extern struct event timerqueue[];

/*
 * Imported from the main and peer modules.  We use the same
 * algorithm for spacing out timers at configuration time that
 * the peer module does.
 */
extern u_int init_peer_starttime;
extern int initializing;
extern int debug;

/*
 * refclock_newpeer - initialize and start a reference clock
 */
int
refclock_newpeer(peer)
	struct peer *peer;
{
	int clktype;
	int unit;
	extern char *ntoa();
	void refclock_transmit();

	/*
	 * Sanity...
	 */
	if (!ISREFCLOCKADR(&peer->srcadr)) {
		syslog(LOG_ERR,
	"Internal error: attempting to initialize %s as reference clock",
		    ntoa(&peer->srcadr));
		return 0;
	}

	clktype = REFCLOCKTYPE(&peer->srcadr);
	unit = REFCLOCKUNIT(&peer->srcadr);

	/*
	 * If clktype is invalid, return
	 */
	if (clktype >= num_refclock_conf
	    || refclock_conf[clktype].clock_start == noentry) {
		syslog(LOG_ERR,
		    "Can't initialize %s, no support for clock type %d\n",
		    ntoa(&peer->srcadr), clktype);
		return 0;
	}

	/*
	 * Complete initialization of peer structure.
	 */
	peer->refclktype = clktype;
	peer->refclkunit = unit;
	peer->flags |= FLAG_REFCLOCK;
	peer->stratum = STRATUM_REFCLOCK;
	peer->ppoll = NTP_MINPOLL;
	
	/*
	 * Check the flags.  If the peer is configured in client mode
	 * but prefers the broadcast client filter algorithm, change
	 * him over.
	 */
	if (peer->hmode == MODE_CLIENT
	    && refclock_conf[clktype].clock_flags & REF_FLAG_BCLIENT)
		peer->hmode = MODE_BCLIENT;

	peer->event_timer.peer = peer;
	peer->event_timer.event_handler = refclock_transmit;

	/*
	 * Do driver dependent initialization
	 */
	if (!((refclock_conf[clktype].clock_start)(unit, peer))) {
		syslog(LOG_ERR, "Clock dependent initialization of %s failed",
		    ntoa(&peer->srcadr));
		return 0;
	}

	/*
	 * If the driver wants the transmit routine to do unreachability
	 * determination, set up the time out.
	 */
	if (refclock_conf[clktype].clock_xmitinterval != NOPOLL) {
		if (initializing) {
			init_peer_starttime += (1<<EVENT_TIMEOUT);
			if (init_peer_starttime >= (1<<NTP_MINPOLL))
				init_peer_starttime = (1<<EVENT_TIMEOUT);
			peer->event_timer.event_time = init_peer_starttime;
		} else {
			peer->event_timer.event_time = current_time
			    + refclock_conf[clktype].clock_xmitinterval;
		}
		TIMER_ENQUEUE(timerqueue, &peer->event_timer);
	}
	return 1;
}


/*
 * refclock_unpeer - shut down a clock
 */
void
refclock_unpeer(peer)
	struct peer *peer;
{
	extern char *ntoa();

	/*
	 * Do sanity checks.  I know who programmed the calling routine!
	 */
	if (peer->refclktype >= num_refclock_conf
	    || refclock_conf[peer->refclktype].clock_shutdown == noentry) {
		syslog(LOG_ERR, "Attempting to shutdown %s: no such clock!",
		    ntoa(&peer->srcadr));
		return;
	}

	/*
	 * Tell the driver we're finished.
	 */
	(refclock_conf[peer->refclktype].clock_shutdown)(peer->refclkunit);
}


/*
 * refclock_transmit - replacement transmit procedure for reference clocks
 */
void
refclock_transmit(peer)
	struct peer *peer;
{
	u_char opeer_reach;
	int clktype;
	int unit;
	extern struct peer *sys_peer;
	extern void get_systime();
	extern void clock_filter();
	extern void clock_select();
	extern void report_event();

	clktype = peer->refclktype;
	unit = peer->refclkunit;
	peer->sent++;

	/*
	 * The transmit procedure is supposed to freeze a time stamp.
	 * Get one just for fun, and to tell when we last were here.
	 */
	get_systime(&peer->xmt);

	/*
	 * Fiddle reachability.
	 */
	opeer_reach = peer->reach;
	peer->reach <<= 1;
	if (peer->reach == 0) {
		if (opeer_reach != 0) {
			report_event(EVNT_UNREACH, peer);
			/*
			 * Clear this one out.  No need to redo
			 * selection since this fellow will definitely
			 * be suffering from dispersion madness.
			 */
			if (opeer_reach != 0) {
				clear(peer);
				peer->timereachable = current_time;
			}
		}
	} else if (peer->valid >= 2) {
		l_fp off;

		off.l_ui = off.l_uf = 0;
		clock_filter(peer, &off, 0, 1);
		/*
		 * Peer must have gotten worse.  Redo selection.
		 * A reasonable optimization is to only rerun the
		 * selection algorithm if the peer is currently
		 * sys_peer.
		 */
		if (peer == sys_peer)
			clock_select(peer);
	} else {
		peer->valid++;
	}

	/*
	 * If he wants to be polled, do it.
	 */
	if (refclock_conf[clktype].clock_poll != noentry)
		(refclock_conf[clktype].clock_poll)(unit, peer);
	
	/*
	 * Finally, reset the timer
	 */
	peer->event_timer.event_time
	    += refclock_conf[clktype].clock_xmitinterval;
	TIMER_ENQUEUE(timerqueue, &peer->event_timer);
}


/*
 * refclock_receive - simulate the receive and packet procedures for clocks
 */
void
refclock_receive(peer, offset, delay, reftime, rectime, insync)
	struct peer *peer;
	l_fp *offset;
	u_fp delay;
	l_fp *reftime;
	l_fp *rectime;
	int insync;
{
	int restrict;
	int trustable;
	u_char oreach;
	extern void clock_filter();
	extern void clock_update();
	extern void report_event();
	extern int restrictions();
	extern u_char leap_indicator;

	/*
	 * The name of this routine is actually a misnomer since
	 * we mostly simulate the variable setting of the packet
	 * procedure.  We do check the flag values, though, and
	 * set the trust bits based on this.
	 */
	restrict = restrictions(&peer->srcadr);
	if (restrict & (RES_IGNORE|RES_DONTSERVE)) {
		/*
		 * Ours is not to reason why...
		 */
		return;
	}

	peer->received++;
	peer->processed++;
	peer->timereceived = current_time;
	if (restrict & RES_DONTTRUST)
		trustable = 0;
	else
		trustable = 1;

	if (peer->flags & FLAG_AUTHENABLE) {
		if (trustable)
			peer->flags |= FLAG_AUTHENTIC;
		else
			peer->flags &= ~FLAG_AUTHENTIC;
	}

	/*
	 * Diddle the leap bits
	 */
	if (!insync) {
		peer->leap = LEAP_NOTINSYNC;
		trustable = 0;
	} else
		peer->leap = leap_indicator;

	/*
	 * Set the timestamps.  reftime and org are in time code time
	 * while rec is in local time.  Just copy what we were given
	 * make it reftime plus the offset.
	 */
	peer->reftime = peer->org = *reftime;
	peer->rec = *rectime;

	/*
	 * Mark him reachable
	 */
	oreach = peer->reach;
	peer->reach |= 1;
	peer->valid = 0;

	/*
	 * If the interface has been set to any_interface, set it
	 * to the loop back address if we have one.  This is so
	 * that peers which are unreachable are easy to see in
	 *  peer display.
	 */
	if (peer->dstadr == any_interface && loopback_interface != 0)
		peer->dstadr = loopback_interface;

	/*
	 * Set peer.pmode based on the hmode.  For appearances only.
	 */
	switch (peer->hmode) {
	case MODE_ACTIVE:
		peer->pmode = MODE_PASSIVE;
		break;
	case MODE_CLIENT:
		peer->pmode = MODE_SERVER;
		break;
	case MODE_BCLIENT:
		peer->pmode = MODE_BROADCAST;
		break;
	default:
		syslog(LOG_ERR, "internal error in refclock_receive, mode = %d",
		    peer->hmode);
	}

	/*
	 * Give the data to the clock filter
	 */
	clock_filter(peer, offset, delay, trustable);

	/*
	 * If this guy was previously unreachable, report him
	 * reachable.
	 */
	if (oreach == 0)
		report_event(EVNT_REACH, peer);

	/*
	 * Now update the clock.
	 */
	clock_update(peer);
}


/*
 * refclock_control - set and/or return clock values
 */
void
refclock_control(srcadr, in, out)
	struct sockaddr_in *srcadr;
	struct refclockstat *in;
	struct refclockstat *out;
{
	int clktype;
	int unit;
	extern char *ntoa();

	/*
	 * Sanity...
	 */
	if (!ISREFCLOCKADR(srcadr)) {
		syslog(LOG_ERR,
	"Internal error: refclock_control received %s as reference clock",
		    ntoa(srcadr));
		return;
	}

	clktype = REFCLOCKTYPE(srcadr);
	unit = REFCLOCKUNIT(srcadr);

	/*
	 * If clktype is invalid, return
	 */
	if (clktype >= num_refclock_conf
	    || refclock_conf[clktype].clock_control == noentry) {
		syslog(LOG_ERR,
		   "Internal error: refclock_control finds %s as not supported",
		    ntoa(srcadr));
		return;
	}

	/*
	 * Give the stuff to the clock.
	 */
	(refclock_conf[clktype].clock_control)(unit, in, out);
}



/*
 * refclock_buginfo - return debugging info
 */
void
refclock_buginfo(srcadr, bug)
	struct sockaddr_in *srcadr;
	struct refclockbug *bug;
{
	int clktype;
	int unit;
	extern char *ntoa();

	/*
	 * Sanity...
	 */
	if (!ISREFCLOCKADR(srcadr)) {
		syslog(LOG_ERR,
	"Internal error: refclock_buginfo received %s as reference clock",
		    ntoa(srcadr));
		return;
	}

	clktype = REFCLOCKTYPE(srcadr);
	unit = REFCLOCKUNIT(srcadr);

	/*
	 * If clktype is invalid or call is unsupported, return
	 */
	if (clktype >= num_refclock_conf ||
	    refclock_conf[clktype].clock_buginfo == noentry) {
		return;
	}

	/*
	 * Give the stuff to the clock.
	 */
	(refclock_conf[clktype].clock_buginfo)(unit, bug);
}



/*
 * refclock_leap - inform the reference clocks that need it that
 *		   we've had a leap second
 */
void
refclock_leap()
{
	register int i;

	for (i = 0; i < num_refclock_conf; i++) {
		if (refclock_conf[i].clock_leap != noentry)
			(refclock_conf[i].clock_leap)();
	}
}


/*
 * init_refclock - initialize the reference clock drivers
 */
void
init_refclock()
{
	register int i;

	for (i = 0; i < num_refclock_conf; i++) {
		if (refclock_conf[i].clock_init != noentry)
			(refclock_conf[i].clock_init)();
	}
}
#endif

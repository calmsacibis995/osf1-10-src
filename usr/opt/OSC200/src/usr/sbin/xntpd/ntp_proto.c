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
static char     *sccsid = "@(#)$RCSfile: ntp_proto.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/07/07 15:51:58 $";
#endif
/*
 */

/*
 * ntp_proto.c - NTP version 2 protocol machinery
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>

/*
 * System variables are declared here.  See Section 3.2 of
 * the specification.
 */
u_char sys_leap;		/* system leap indicator */
u_char sys_stratum;		/* stratum of system */
s_char sys_precision;		/* local clock precision */
u_fp sys_distance;		/* distance to current sync source */
u_fp sys_dispersion;		/* dispersion of system clock */
u_int sys_refid;		/* reference source for local clock */
l_fp sys_reftime;		/* time we were last updated */
u_int sys_hold;		/* system hold counter */
struct peer *sys_peer;		/* our current peer */
u_fp sys_maxskew;		/* a configuration parameter, def NTP_MAXSKW */

/*
 * Non-specified system state variables.
 */
int sys_bclient;		/* we set our time to broadcasts */
u_int sys_bdelay;		/* default delay to use for broadcasting */
int sys_authenticate;		/* authenticate time used for syncing */

u_int sys_authdelay;		/* ts fraction, time it takes for encrypt() */

int sys_select_algorithm;	/* experiment algorithm selection */

/*
 * Statistics counters
 */
u_int sys_stattime;		/* time when we started recording */
u_int sys_badstratum;		/* packets with invalid incoming stratum */
u_int sys_oldversionpkt;	/* old version packets received */
u_int sys_newversionpkt;	/* new version packets received */
u_int sys_unknownversion;	/* don't know version packets */
u_int sys_badlength;		/* packets with bad length */
u_int sys_processed;		/* packets processed */
u_int sys_badauth;		/* packets dropped because of authorization */
u_int sys_wanderhold;		/* sys_peer held to prevent wandering */


/*
 * Imported from ntp_timer.c
 */
extern u_int current_time;
extern struct event timerqueue[];

/*
 * Imported from ntp_io.c
 */
extern struct interface *any_interface;

/*
 * The peer hash table.  Imported from ntp_peer.c
 */
extern struct peer *peer_hash[];

/*
 * debug flag
 */
extern int debug;


/*
 * transmit - Transmit Procedure.  See Section 3.4.1 of the specification.
 */
void
transmit(peer)
	register struct peer *peer;
{
	struct pkt xpkt;	/* packet to send */
	u_int peer_timer;
	extern void unpeer();
	void clock_select();
	void clock_filter();
	void clear();
	extern void sendpkt();
	extern void get_systime();
	extern void auth1crypt();
	extern void auth2crypt();
	extern int authhavekey();
	extern char *ntoa();

	if (peer->hmode != MODE_BCLIENT) {
		u_int xkeyid;
#ifdef DES_OK
		/*
		 * Figure out which keyid to include in the packet
		 */
		if ((peer->flags & FLAG_AUTHENABLE)
		    && (peer->flags & (FLAG_CONFIG|FLAG_AUTHENTIC))
		    && authhavekey(peer->keyid)) {
			xkeyid = peer->keyid;
		} else {
#endif /* DES_OK */
			xkeyid = 0;
#ifdef DES_OK
		}
#endif /* DES_OK */

		/*
		 * Make up a packet to send.
		 */
		xpkt.li_vn_mode
		    = PKT_LI_VN_MODE(sys_leap, peer->version, peer->hmode);
		xpkt.stratum = STRATUM_TO_PKT(sys_stratum);
		if (peer->reach == 0)
			xpkt.ppoll = NTP_MINPOLL;
		else
			xpkt.ppoll = peer->hpoll;
		xpkt.precision = sys_precision;
		xpkt.distance = HTONS_FP(sys_distance);
		xpkt.dispersion = HTONS_FP(sys_dispersion);
		xpkt.refid = sys_refid;
		HTONL_FP(&sys_reftime, &xpkt.reftime);
		HTONL_FP(&peer->org, &xpkt.org);
		HTONL_FP(&peer->rec, &xpkt.rec);

#ifdef DES_OK
		/*
		 * Decide whether to authenticate or not.  If so, call encrypt()
		 * to fill in the rest of the frame.  If not, just add in the
		 * xmt timestamp and send it quick.
		 */
		if (peer->flags & FLAG_AUTHENABLE) {
			xpkt.keyid = htonl(xkeyid);
			auth1crypt(xkeyid, (u_int *)&xpkt, LEN_PKT_NOMAC);
			get_systime(&peer->xmt);
			L_ADDUF(&peer->xmt, sys_authdelay);
			HTONL_FP(&peer->xmt, &xpkt.xmt);
			auth2crypt(xkeyid, (u_int *)&xpkt, LEN_PKT_NOMAC);
			sendpkt(&(peer->srcadr), peer->dstadr, &xpkt,
			    LEN_PKT_MAC);
#ifdef DEBUG
			if (debug > 1)
				printf("transmit auth to %s\n",
				    ntoa(&(peer->srcadr)));
#endif /* DEBUG */
			peer->sent++;
		} else {
#endif /* DES_OK */
			/*
			 * Get xmt timestamp, then send it without mac field
			 */
			get_systime(&(peer->xmt));
			HTONL_FP(&peer->xmt, &xpkt.xmt);
			sendpkt(&(peer->srcadr), peer->dstadr, &xpkt,
			    LEN_PKT_NOMAC);
#ifdef DEBUG
			if (debug > 1)
				printf("transmit to %s\n", ntoa(&(peer->srcadr)));
#endif /* DEBUG */
			peer->sent++;
#ifdef DES_OK
		}
#endif /* DES_OK */
	}

	if (peer->hmode != MODE_BROADCAST) {
		u_char opeer_reach;
		/*
		 * Determine reachability and diddle things if we
		 * haven't heard from the host for a while.
		 */
		opeer_reach = peer->reach;
		peer->reach <<= 1;
		if (peer->reach == 0) {
			if (opeer_reach != 0)
				report_event(EVNT_UNREACH, peer);
			/*
			 * Clear this guy out.  No need to redo clock
			 * selection since by now this guy won't be a player
			 */
			if (peer->flags & FLAG_CONFIG) {
				if (opeer_reach != 0) {
					clear(peer);
					peer->timereachable = current_time;
				}
			} else {
				unpeer(peer);
				return;
			}

			/*
			 * While we have a chance, if our system peer
			 * is zero or his stratum is greater than the
			 * last known stratum of this guy, make sure
			 * peer->hpoll is clamped to the minimum before
			 * resetting the timer.
			 */
			if (sys_peer == 0
			    || sys_peer->stratum > peer->stratum) {
				peer->hpoll = NTP_MINPOLL;
				peer->unreach = 0;
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
	}

	/*
	 * Arrange for our next time out.  hpoll will be less than
	 * NTP_MAXPOLL for sure
	 */
	peer_timer = 1 << max(min(peer->ppoll, peer->hpoll), NTP_MINPOLL);
	peer->event_timer.event_time = current_time + peer_timer;
	TIMER_ENQUEUE(timerqueue, &peer->event_timer);

	/*
	 * Finally, update the host-poll variable.
	 */
	if (peer == sys_peer || (peer->flags & FLAG_MINPOLL)
	    || peer->hmode == MODE_BROADCAST || peer->hmode == MODE_BCLIENT) {
		/* clamp it */
		peer->hpoll = NTP_MINPOLL;
	} else if (peer->reach == 0) {
		/*
		 * If the peer has been unreachable for a while
		 * and we have a system peer who is at least his
		 * equal, we may want to ramp his polling interval
		 * up to avoid the useless traffic.
		 */
		if (sys_peer != 0
		    && sys_peer->stratum <= peer->stratum) {
			if (peer->unreach < 16) {
				peer->unreach++;
				peer->hpoll = NTP_MINPOLL;
			} else if (peer->hpoll < NTP_MAXPOLL) {
				peer->hpoll++;
				peer->ppoll = peer->hpoll;
			}
		}
	} else if (peer->estdisp > PEER_THRESHOLD) {
		if (peer->hpoll > NTP_MINPOLL)
			peer->hpoll--;
	} else if (peer->trust == 0 || (peer->trust & 0x1) != 0) {
		/*
		 * Don't increase the polling interval if we're
		 * clearing out untrustworthy data.
		 */
		if (peer->hpoll < NTP_MAXPOLL)
			peer->hpoll++;
	}
}


/*
 * receive - Receive Procedure.  See section 3.4.2 in the specification.
 */
void
receive(rbufp)
	struct recvbuf *rbufp;
{
	register struct peer *peer;
	register struct pkt *pkt;
	register u_char hismode;
	int restrict;
	int has_mac;
	int trustable;
	int is_authentic;
	u_int hiskeyid;
	void process_packet();
	void fast_xmit();
	void clock_select();
	void clear();
	extern void unpeer();
	extern void monitor();
	extern int restrictions();
	extern int authdecrypt();
	extern int authistrusted();
	extern struct peer *findpeer();
	extern struct peer *newpeer();
	extern char *ntoa();

#ifdef DEBUG
	if (debug > 1)
		printf("receive from %s\n", ntoa(&rbufp->recv_srcadr));
#endif

	/*
	 * Let the monitoring software take a look at this first.
	 */
	monitor(rbufp);

	/*
	 * Get the restrictions on this guy.  If we're to ignore him,
	 * go no further.
	 */
	restrict = restrictions(&rbufp->recv_srcadr);
	if (restrict & RES_IGNORE)
		return;

	/*
	 * Get a pointer to the packet.
	 */
	pkt = &rbufp->recv_pkt;

	/*
	 * Catch packets whose version number we can't deal with
	 */
	if (PKT_VERSION(pkt->li_vn_mode) == NTP_VERSION) {
		sys_newversionpkt++;
	} else if (PKT_VERSION(pkt->li_vn_mode) == NTP_OLDVERSION) {
		sys_oldversionpkt++;
	} else {
		sys_unknownversion++;
		return;
	}

	/*
	 * Catch private mode packets.  Dump it if queries not allowed.
	 */
	if (PKT_MODE(pkt->li_vn_mode) == MODE_PRIVATE) {
		if (restrict & RES_NOQUERY)
			return;
		process_private(rbufp, ((restrict&RES_NOMODIFY) == 0));
		return;
	}

	/*
	 * Same with control mode packets.
	 */
	if (PKT_MODE(pkt->li_vn_mode) == MODE_CONTROL) {
		if (restrict & RES_NOQUERY)
			return;
		process_control(rbufp, restrict);
		return;
	}

	/*
	 * See if we're allowed to serve this guy time.  If not, ignore
	 * him.
	 */
	if (restrict & RES_DONTSERVE)
		return;

	/*
	 * Dump anything with a putrid stratum.  These will most likely
	 * come from someone trying to poll us with ntpdc.
	 */
	if (pkt->stratum > NTP_INFIN) {
		sys_badstratum++;
		return;
	}

	/*
	 * Find the peer.  This will return a null if this guy
	 * isn't in the database.
	 */
	peer = findpeer(&rbufp->recv_srcadr, rbufp->dstadr);

	/*
	 * Check the length for validity, drop the packet if it is
	 * not as expected.
	 *
	 * If this is a client mode poll, go no further.  Send back
	 * his time and drop it.
	 *
	 * The scheme we use for authentication is this.  If we are
	 * running in non-authenticated mode, we accept both frames
	 * which are authenticated and frames which aren't, but don't
	 * authenticate.  We do record whether the frame had a mac field
	 * or not so we know what to do on output.
	 *
	 * If we are running in authenticated mode, we only trust frames
	 * which have authentication attached, which are validated and
	 * which are using one of our trusted keys.  We respond to all
	 * other pollers without saving any state.  If a host we are
	 * passively peering with changes his key from a trusted one to
	 * an untrusted one, we immediately unpeer with him, reselect
	 * the clock and treat him as an unmemorable client (this is
	 * a small denial-of-service hole I'll have to think about).
	 * If a similar event occurs with a configured peer we drop the
	 * frame and hope he'll revert to our key again.  If we get a
	 * frame which can't be authenticated with the given key, we
	 * drop it.  Either we disagree on the keys or someone is trying
	 * some funny stuff.
	 */
	if (rbufp->recv_length == LEN_PKT_MAC) {
		has_mac = 1;
		hiskeyid = ntohl(pkt->keyid);
	} else if (rbufp->recv_length == LEN_PKT_NOMAC) {
		hiskeyid = 0;
		has_mac = 0;
	} else {
#ifdef DEBUG
		if (debug > 2)
			printf("receive: bad length %d\n", rbufp->recv_length);
#endif
		sys_badlength++;
		return;
	}



	/*
	 * Figure out his mode and validate it.
	 */
	hismode = PKT_MODE(pkt->li_vn_mode);
#ifdef DEBUG
	if (debug > 2)
		printf("receive: his mode %d\n", hismode);
#endif
	if (PKT_VERSION(pkt->li_vn_mode) == NTP_OLDVERSION && hismode == 0) {
		/*
		 * Easy.  If it is from the NTP port it is
		 * a sym act, else client.
		 */
		if (SRCPORT(&rbufp->recv_srcadr) == NTP_PORT)
			hismode = MODE_ACTIVE;
		else
			hismode = MODE_CLIENT;
	} else {
		if (hismode != MODE_ACTIVE && hismode != MODE_PASSIVE &&
		    hismode != MODE_SERVER && hismode != MODE_CLIENT &&
		    hismode != MODE_BROADCAST) {
			syslog(LOG_ERR, "bad mode %d received from %s",
			    PKT_MODE(pkt->li_vn_mode),
			    ntoa(&rbufp->recv_srcadr));
			return;
		}
	}


	/*
	 * If he included a mac field, decrypt it to see if it is authentic.
	 */
	is_authentic = 0;
#ifdef DES_OK
	if (has_mac) {
		if (authhavekey(hiskeyid)) {
			if (authdecrypt(hiskeyid, (u_int *)pkt, LEN_PKT_NOMAC))
				is_authentic = 1;
			else
				sys_badauth++;
		}
	}
#endif /* DES_OK */

	/*
	 * Dispatch client mode packets which made it this far.
	 */
	if (hismode == MODE_CLIENT) {
		fast_xmit(rbufp, hismode, is_authentic);
		return;
	}

	/*
	 * If this is someone we don't remember from a previous association,
	 * dispatch him now.  Either we send something back quick, we
	 * ignore him, or we allocate some memory for him and let
	 * him continue.
	 */
	if (peer == 0) {
		int mymode;

		switch(hismode) {
		case MODE_ACTIVE:
			/*
			 * See if this guy qualifies as being the least
			 * bit memorable.  If so we keep him around for
			 * later.  If not, send his time quick.
			 */
			if ((restrict & RES_NOPEER)
			    || PKT_LEAP(pkt->li_vn_mode) == LEAP_NOTINSYNC
			    || PKT_TO_STRATUM(pkt->stratum) >= NTP_INFIN
			    || PKT_TO_STRATUM(pkt->stratum) > sys_stratum) {
				fast_xmit(rbufp, hismode, is_authentic);
				return;
			}
			mymode = MODE_PASSIVE;
			break;

		case MODE_PASSIVE:
		case MODE_SERVER:
			/*
			 * These are obvious errors.  Ignore.
			 */
			return;

		case MODE_BROADCAST:
			/*
			 * Sort of a repeat of the above...
			 */
			if ((restrict & RES_NOPEER) || !sys_bclient
			    || PKT_LEAP(pkt->li_vn_mode) == LEAP_NOTINSYNC
			    || PKT_TO_STRATUM(pkt->stratum) >= NTP_INFIN
			    || PKT_TO_STRATUM(pkt->stratum) > sys_stratum)
				return;
			mymode = MODE_BCLIENT;
			break;
		}

		/*
		 * Okay, we're going to keep him around.  Allocate him
		 * some memory.
		 */
		peer = newpeer(&rbufp->recv_srcadr, rbufp->dstadr, mymode,
		    (int)PKT_VERSION(pkt->li_vn_mode), hiskeyid);
		if (peer == 0) {
			/*
			 * The only way this can happen is if the
			 * source address looks like a reference
			 * clock.  Since this is an illegal address
			 * this is one of those "can't happen" things.
			 */
			syslog(LOG_ERR,
			    "receive() failed to peer with %s, mode %d",
			    ntoa(&rbufp->recv_srcadr), mymode);
			return;
		}
	}

	/*
	 * Mark the time of reception
	 */
	peer->timereceived = current_time;

	/*
	 * If the peer isn't configured, set his keyid and authenable
	 * status based on the packet.
	 */
	if (!(peer->flags & FLAG_CONFIG)) {
		if (has_mac) {
			peer->keyid = hiskeyid;
			peer->flags |= FLAG_AUTHENABLE;
		} else {
			peer->keyid = 0;
			peer->flags &= ~FLAG_AUTHENABLE;
		}
	}


	/*
	 * If this message was authenticated properly, note this
	 * in the flags.
	 */
	if (is_authentic) {
		peer->flags |= FLAG_AUTHENTIC;
	} else {
		/*
		 * If this guy is authenable, and has been authenticated
		 * in the past, but just failed the authentic test, report
		 * the event.
		 */
		if (peer->flags & FLAG_AUTHENABLE
		    && peer->flags & FLAG_AUTHENTIC)
			report_event(EVNT_PEERAUTH, peer);
		peer->flags &= ~FLAG_AUTHENTIC;
	}

	/*
	 * Determine if this guy is basically trustable.
	 */
	if (restrict & RES_DONTTRUST)
		trustable = 0;
	else
		trustable = 1;
	
#ifdef DES_OK
	if (sys_authenticate && trustable) {
		if (!(peer->flags & FLAG_CONFIG)
		    || (peer->flags & FLAG_AUTHENABLE))
			trustable = 0;

		if (has_mac) {
			if (authistrusted(hiskeyid)) {
				if (is_authentic) {
					trustable = 1;
				} else {
					trustable = 0;
					peer->badauth++;
				}
			}
		}
	}
#endif /* DES_OK */

	/*
	 * Dispose of the packet based on our respective modes.  We
	 * don't drive this with a table, though we probably could.
	 */
	switch (peer->hmode) {
	case MODE_ACTIVE:
	case MODE_CLIENT:
		/*
		 * Active mode associations are configured.  If the data
		 * isn't trustable, ignore it and hope this guy brightens
		 * up.  Else accept any data we get and process it.
		 */
		switch (hismode) {
		case MODE_ACTIVE:
		case MODE_PASSIVE:
		case MODE_SERVER:
			process_packet(peer, pkt, &(rbufp->recv_time),
			    has_mac, trustable);
			break;

		case MODE_BROADCAST:
			/*
			 * No good for us, we want real time.
			 */
			break;
		}
		break;

	case MODE_PASSIVE:
		/*
		 * Passive mode associations are (in the current
		 * implementation) always dynamic.  If we've risen
		 * beyond his stratum, break the connection.  I hate
		 * doing this since it seems like a waste.  Oh, well.
		 */
		switch (hismode) {
		case MODE_ACTIVE:
			if (PKT_LEAP(pkt->li_vn_mode) == LEAP_NOTINSYNC
			    || PKT_TO_STRATUM(pkt->stratum) >= NTP_INFIN
			    || PKT_TO_STRATUM(pkt->stratum) > sys_stratum) {
				unpeer(peer);
				clock_select((struct peer *)0);
				fast_xmit(rbufp, hismode, is_authentic);
			} else {
				process_packet(peer, pkt, &(rbufp->recv_time),
				    has_mac, trustable);
			}
			break;

		case MODE_PASSIVE:
		case MODE_SERVER:
		case MODE_BROADCAST:
			/*
			 * These are errors.  Just ignore the packet.
			 * If he doesn't straighten himself out this
			 * association will eventually be disolved.
			 */
			break;
		}
		break;

	
	case MODE_BCLIENT:
		/*
		 * Broadcast client pseudo-mode.  We accept both server
		 * and broadcast data.  Passive mode data is an error.
		 */
		switch (hismode) {
		case MODE_ACTIVE:
			if (PKT_LEAP(pkt->li_vn_mode) == LEAP_NOTINSYNC
			    || PKT_TO_STRATUM(pkt->stratum) >= NTP_INFIN
			    || PKT_TO_STRATUM(pkt->stratum) > sys_stratum) {
				/*
				 * Strange situation.  We've been receiving
				 * broadcasts from him which we liked, but
				 * we don't like his active mode stuff.  Send
				 * him some time quickly, we'll figure it
				 * out later.
				 */
				fast_xmit(rbufp, hismode, is_authentic);
			} else {
				/*
				 * This guy wants to give us real time
				 * when we've been existing on lousy
				 * broadcasts!  Check to make sure data
				 * is authentic.  If so, convert this to
				 * passive mode, clear it out and do it
				 * that way.
				 */
				peer->hmode = MODE_PASSIVE;
				clear(peer);
				process_packet(peer, pkt, &rbufp->recv_time,
				    has_mac, trustable);
			}
			break;
		
		case MODE_PASSIVE:
			break;
		
		case MODE_SERVER:
		case MODE_BROADCAST:
			if (PKT_LEAP(pkt->li_vn_mode) == LEAP_NOTINSYNC
			    || PKT_TO_STRATUM(pkt->stratum) >= NTP_INFIN
			    || PKT_TO_STRATUM(pkt->stratum) > sys_stratum) {
				/*
				 * Do nothing, he sucks.  Let him time out.
				 */
			} else {
				process_packet(peer, pkt, &rbufp->recv_time,
				    has_mac, trustable);
			}
			break;
		}
	}
}


/*
 * process_packet - Packet Procedure, a la Section 3.4.3 of the specification.
 *	  	    Or almost, at least.  If we're in here we have a reasonable
 *		    expectation that we will be having a long term relationship
 *		    with this host.
 */
void
process_packet(peer, pkt, recv_ts, has_mac, trustable)
	register struct peer *peer;
	register struct pkt *pkt;
	l_fp *recv_ts;
	int has_mac;
	int trustable;
{
	u_int t23_ui, t23_uf;
	u_int t10_ui, t10_uf;
	s_fp di;
	l_fp ci, temp;
	int bogus_pkt = 0;
	int randomize;
	u_char ostratum, oreach;
	void clock_update();
	void poll_update();
	void clock_filter();
	extern char *ntoa();
	extern char *fptoa();
	extern char *umfptoa();
	extern char *mfptoa();
	extern char *lfptoa();

	sys_processed++;
	peer->processed++;

	/*
	 * If the xmt time stamp is the same as the last one from this guy,
	 * mark this as bogus.  If the org time stamp isn't the same as the
	 * one we sent, mark the packet as bogus and randomize our next
	 * time interval to break synchronization.  Randomize if this
	 * guy hasn't heard from us for a while.
	 */
	
	bogus_pkt = 0;
	randomize = POLL_RANDOMCHANGE;
	NTOHL_FP(&pkt->xmt, &temp);
	if (L_ISHIS(&peer->org, &temp)) {
		peer->oldpkt++;
#ifdef DEBUG
		if (debug)
			printf("peer %s sent an old packet\n",
			    ntoa(&peer->srcadr));
#endif
		if (!L_ISEQU(&peer->org, &temp)
		    && (temp.l_ui != 0 || temp.l_uf != 0))
			/* could be attack? */
			return;
		bogus_pkt = 1;
	} else if (PKT_MODE(pkt->li_vn_mode) != MODE_BROADCAST) {
		l_fp temp2;

		NTOHL_FP(&(pkt->org), &temp2);
		if (peer->xmt.l_ui == 0 || !L_ISEQU(&temp2, &(peer->xmt))) {
			bogus_pkt = 1;
			randomize = POLL_MAKERANDOM;
			peer->bogusorg++;
#ifdef DEBUG
			if (debug)
				printf("peer %s includes ts we didn't send\n",
				    ntoa(&peer->srcadr));
#endif
		}
	}


	/*
	 * Now update our state.
	 */
	peer->leap = PKT_LEAP(pkt->li_vn_mode);
	peer->pmode = PKT_MODE(pkt->li_vn_mode);
	if (has_mac)
		peer->pkeyid = ntohl(pkt->keyid);
	else
		peer->pkeyid = 0;
	ostratum = peer->stratum;
	peer->stratum = PKT_TO_STRATUM(pkt->stratum);
	peer->ppoll = pkt->ppoll;
	peer->precision = pkt->precision;
	peer->distance = NTOHS_FP(pkt->distance);
	peer->dispersion = NTOHS_FP(pkt->dispersion);
	peer->refid = pkt->refid;
	NTOHL_FP(&pkt->reftime, &peer->reftime);
	peer->org = temp;	/* reuse byte-swapped pkt->xmt */
	peer->rec = *recv_ts;
	oreach = peer->reach;
	if (peer->reach == 0) {
		peer->timereachable = current_time;
		/*
		 * If this guy was previously unreachable, set his
		 * polling interval to the minimum and reset the
		 * unreach counter.
		 */
		peer->unreach = 0;
		peer->hpoll = NTP_MINPOLL;
	}
	if (trustable && peer->trust != 0) {
		/*
		 * A wart.  If we're getting trustable data
		 * but we previously didn't trust him, drop
		 * his polling interval to the minimum to try
		 * to clear out the filters.
		 */
		peer->hpoll = NTP_MINPOLL;
	}
	peer->reach |= 1;


	/*
	 * Call poll_update().  This will either start us, if the
	 * association is new, or drop the polling interval if the
	 * association is existing and peer->ppoll has been reduced.
	 */
	poll_update(peer, peer->hpoll, randomize);

	/*
	 * If the packet was bogus, exit
	 */
	if (bogus_pkt) {
		/*
		 * If there was a reachability change report it even
		 * though the packet was bogus.
		 */
		if (oreach == 0)
			report_event(EVNT_REACH, peer);
		return;
	}

	/*
	 * Test to see if the peer is synchronized.  If not we mark
	 * the data untrustable.
	 */
	if (peer->leap == LEAP_NOTINSYNC)
		trustable = 0;
	if ((peer->org.l_ui - peer->reftime.l_ui)
	    >= NTP_MAXAGE) {
		peer->seltooold++;
		trustable = 0;
	}

	/*
	 * If running in a normal polled association, calculate the round
	 * trip delay (di) and the clock offset (ci).  We use the equations
	 * (reordered from those in the spec):
	 *
	 * d = (t2 - t3) - (t1 - t0)
	 * c = ((t2 - t3) + (t1 - t0)) / 2
	 *
	 * If running as a broadcast client, these change.  di becomes
	 * equal to two times our broadcast delay, while the offset
	 * becomes equal to:
	 *
	 * c = (t1 - t0) + estbdelay
	 */
	t10_ui = peer->org.l_ui;	/* peer->org == t1 */
	t10_uf = peer->org.l_uf;
	M_SUB(t10_ui, t10_uf, peer->rec.l_ui, peer->rec.l_uf); /*peer->rec==t0*/

	if (PKT_MODE(pkt->li_vn_mode) != MODE_BROADCAST) {
		t23_ui = ntohl(pkt->rec.l_ui);	/* pkt->rec == t2 */
		t23_uf = ntohl(pkt->rec.l_uf);
		M_SUB(t23_ui, t23_uf, ntohl(pkt->org.l_ui),
		    ntohl(pkt->org.l_uf));	/* pkt->org == t3 */
	}

	/* now have (t2 - t3) and (t0 - t1).  Calculate (ci) and (di) */
	ci.l_ui = t10_ui;
	ci.l_uf = t10_uf;
	if (peer->hmode == MODE_BCLIENT) {
#ifdef notdef
		if (PKT_MODE(pkt->li_vn_mode) == MODE_CLIENT) {
			/*
			 * A client mode packet, used for delay computation.
			 * Give the data to the filter.
			 */
			bdelay_filter(peer, t23_ui, t23_uf, t10_ui, t10_uf);
		}
#endif
		M_ADDUF(ci.l_ui, ci.l_uf, peer->estbdelay>>1);
		di = MFPTOFP(0, peer->estbdelay);
	} else {
		M_ADD(ci.l_ui, ci.l_uf, t23_ui, t23_uf);
		M_RSHIFT(ci.l_i, ci.l_uf);

		/*
		 * Calculate di in t23 in full precision, then truncate
		 * to an s_fp.
		 */
		M_SUB(t23_ui, t23_uf, t10_ui, t10_uf);
		di = MFPTOFP(t23_ui, t23_uf);
	}
#ifdef DEBUG
	if (debug > 3)
		printf("offset: %s, delay %s\n", lfptoa(&ci, 9), fptoa(di, 4));
#endif

	di += (FP_SECOND >> (-(int)sys_precision))
	    + (FP_SECOND >> (-(int)peer->precision)) + sys_maxskew;

	if (di <= 0) {		/* value still too raunchy to use? */
		peer->baddelay++;
		return;
	}
	di = max(di, NTP_MINDIST);

	/*
	 * This one is valid.  Mark it so, give it to clock_filter(),
	 */
	peer->valid = 0;
	clock_filter(peer, &ci, (u_fp)di, trustable);

	/*
	 * If this guy was previously unreachable, report him
	 * reachable.  Else if this guy's stratum has changed, report that.
	 * Note we do this here so that the peer values we return are
	 * the updated ones.
	 */
	if (oreach == 0)
		report_event(EVNT_REACH, peer);
	else if (peer->stratum != ostratum)
		report_event(EVNT_PEERSTRAT, peer);

	/*
	 * Now update the clock.
	 */
	clock_update(peer);
}


/*
 * clock_update - Clock-update procedure, see section 3.4.5.
 */
void
clock_update(peer)
	struct peer *peer;
{
	u_char oleap;
	u_char ostratum;
	void poll_update();
	void clock_select();
	void clear_all();
	extern int local_clock();
	extern void leap_process();
	extern char *ntoa();

#ifdef DEBUG
	if (debug)
		printf("clock_update(%s)\n", ntoa(&peer->srcadr));
#endif
	/*
	 * If we're holding don't bother with any of this
	 */
	if (sys_hold > current_time)
		return;

	/*
	 * Call the clock selection algorithm to see
	 * if this update causes the peer to change.
	 */
	clock_select(peer);

	/*
	 * If this is the current sys_peer update the system state
	 * and the local clock.
	 */
	if (peer == sys_peer) {
		oleap = sys_leap;
		ostratum = sys_stratum;
		sys_leap = peer->leap;
		/*
		 * N.B. peer->stratum was guaranteed to be less than
		 * NTP_INFIN by the receive procedure.
		 */
		sys_stratum = peer->stratum + 1;
		sys_distance = peer->distance + peer->estdelay;
		sys_dispersion = peer->dispersion + peer->estdisp;
		/*
		 * Hack for reference clocks.  Sigh.  This is the
		 * only real silly part, though, so the analogy isn't
		 * bad.
		 */
		if (peer->flags & FLAG_REFCLOCK
		    && peer->stratum == STRATUM_REFCLOCK)
			sys_refid = peer->refid;
		else
			sys_refid = peer->srcadr.sin_addr.s_addr;
		sys_reftime = peer->rec;

		/*
		 * Report changes.  Note that we never sync to
		 * an unsynchronized host.
		 */
		if (oleap == LEAP_NOTINSYNC)
			report_event(EVNT_SYNCCHG, (struct peer *)0);
		else if (ostratum != sys_stratum)
			report_event(EVNT_PEERSTCHG, (struct peer *)0);

		switch (local_clock(&(peer->estoffset), &(peer->srcadr))) {
		case -1:
			/*
			 * Clock is too screwed up.  Just exit for now.
			 */
			report_event(EVNT_SYSFAULT, (struct peer *)0);
			exit(1);
			/*NOTREACHED*/
		case 0:
			/*
			 * Clock was slewed.  Continue on normally.
			 */
			break;

		case 1:
			/*
			 * Clock was stepped.  Clear filter registers
			 * of all peers, and set the system hold.
			 */
			clear_all();
			sys_hold = (PEER_SHIFT * (1<<NTP_MINPOLL))
			    + current_time;
			leap_process();		/* reset the leap interrupt */
			report_event(EVNT_CLOCKRESET, (struct peer *)0);
			break;
		}
	}
}



/*
 * poll_update - update peer poll interval.  See Section 3.4.8 of the spec.
 */
void
poll_update(peer, new_hpoll, randomize)
	struct peer *peer;
	int new_hpoll;
	int randomize;
{
	register struct event *evp;
	register u_int new_timer;
	register int newpoll;
	u_int ranp2();
	char *ntoa();

#ifdef DEBUG
	if (debug)
		printf("poll_update(%s, %d, %d)\n", ntoa(&peer->srcadr),
		    new_hpoll, randomize);
#endif
	/*
	 * Catch reference clocks here.  The polling interval for a
	 * reference clock is fixed and needn't be maintained by us.
	 */
	if (peer->flags & FLAG_REFCLOCK)
		return;

	/*
	 * This routine * will randomly perturb the new peer.timer if
	 * requested, to try to prevent synchronization with the remote
	 * peer from occuring.  There are three options, based on the
	 * value of randomize:
	 *
	 * POLL_NOTRANDOM - essentially the spec algorithm.  If
	 * peer.timer is greater than the new polling interval,
	 * drop it to the new interval.
	 *
	 * POLL_RANDOMCHANGE - make changes randomly.  If peer.timer
	 * must be changed, based on the comparison about, randomly
	 * perturb the new value of peer.timer.
	 *
	 * POLL_MAKERANDOM - make next interval random.  Calculate
	 * a randomly perturbed poll interval.  If this value is
	 * less that peer.timer, update peer.timer.
	 */
	if (peer == sys_peer)
		peer->hpoll = NTP_MINPOLL;
	else {
		if (new_hpoll >= NTP_MAXPOLL)
			peer->hpoll = NTP_MAXPOLL;
		else if (new_hpoll <= NTP_MINPOLL)
			peer->hpoll = NTP_MINPOLL;
		else
			peer->hpoll = (u_char)new_hpoll;
	}

	/* hpoll <= NTP_MAXPOLL for sure */
	newpoll = (int)max(min(peer->ppoll, peer->hpoll), NTP_MINPOLL);
	if (randomize == POLL_MAKERANDOM)
		new_timer = RANDOM_POLL(newpoll, ranp2(RANDOM_SPREAD(newpoll)))
		    + current_time;
	else
		new_timer = (1<<newpoll) + current_time;
	evp = &(peer->event_timer);
	if (evp->next == 0 || evp->event_time > new_timer) {
		if (randomize == POLL_RANDOMCHANGE)
			new_timer =
			    RANDOM_POLL(newpoll, ranp2(RANDOM_SPREAD(newpoll)))
			    + current_time;
		TIMER_DEQUEUE(evp);
		evp->event_time = new_timer;
		TIMER_ENQUEUE(timerqueue, evp);
	}
}


/*
 * clear_all - clear all peer filter registers.  This is done after
 *	       a step change in the time.
 */
void
clear_all()
{
	register int i;
	register struct peer *peer;
	extern void unpeer();
	void clear();
	void poll_update();

	for (i = 0; i < HASH_SIZE; i++)
		for (peer = peer_hash[i]; peer != 0; peer = peer->next) {
			/*
			 * We used to drop all unconfigured pollers here.
			 * The problem with doing this is that if your best
			 * time source is unconfigured (there are reasons
			 * for doing this) and you drop him, he may not
			 * get around to polling you for a long time.  Hang
			 * on to everyone, dropping their polling intervals
			 * to the minimum.
			 */
			clear(peer);
			poll_update(peer, NTP_MINPOLL, POLL_RANDOMCHANGE);
		}

	/*
	 * Clear sys_peer.  We'll sync to one later.
	 */
	sys_peer = 0;

}


/*
 * clear - clear peer filter registers.  See Section 3.4.7 of the spec.
 */
void
clear(peer)
	register struct peer *peer;
{
	extern char *ntoa();

#ifdef DEBUG
	if (debug)
		printf("clear(%s)\n", ntoa(&peer->srcadr));
#endif
	bzero(CLEAR_TO_ZERO(peer), LEN_CLEAR_TO_ZERO);
	peer->estdisp = PEER_MAXDISP;

	/*
	 * Clear out the selection counters
	 */
	peer->candidate = 0;
	peer->falseticker = 0;
	peer->select = peer->select_total = 0;
	peer->was_sane = 0;

	/*
	 * Since we have a chance to correct possible funniness in
	 * our selection of interfaces on a multihomed host, do so
	 * by setting us to no particular interface.
	 */
	peer->dstadr = any_interface;
}


/*
 * clock_filter - add incoming clock sample to filter register and run
 *		  the filter procedure to find the best sample.
 */
void
clock_filter(peer, sample_offset, sample_delay, trustable)
	register struct peer *peer;
	l_fp *sample_offset;
	u_fp sample_delay;
	int trustable;
{
	register int i;
	register u_char *ord;
	register s_fp sample_soffset;
	extern char *ntoa();
	extern char *ufptoa();
	extern char *lfptoa();

#ifdef DEBUG
	if (debug)
		printf("clock_filter(%s, %s, %s)\n", ntoa(&peer->srcadr),
		    lfptoa(sample_offset, 9), ufptoa(sample_delay, 4));
#endif

	/*
	 * We keep a sort by delay of the current contents of the
	 * shift registers.  We update this by (1) removing the
	 * register we are going to be replacing from the sort, and
	 * (2) reinserting it based on the new delay value.
	 */
	ord = peer->filter_order;
	sample_soffset = LFPTOFP(sample_offset);

	for (i = 0; i < PEER_SHIFT-1; i++)	/* find old value */
		if (ord[i] == peer->filter_nextpt)
			break;
	for ( ; i < PEER_SHIFT-1; i++)	/* i is current, move everything up */
		ord[i] = ord[i+1];
	/* Here, last slot in ord[] is empty */

	if (sample_delay == 0)
		/*
		 * Last slot for this guy.
		 */
		i = PEER_SHIFT-1;
	else {
		register int j;
		register u_fp *delayp;

		delayp = peer->filter_delay;
		/*
		 * Find where he goes in, then shift everyone else down
		 */
		if (peer->hmode == MODE_BCLIENT) {
			register s_fp *soffsetp;
			/*
			 * Sort by offset.  The most positive offset
			 * should correspond to the minimum delay.
			 */
			soffsetp = peer->filter_soffset;
			for (i = 0; i < PEER_SHIFT-1; i++)
				if (delayp[ord[i]] == 0
				    || sample_soffset >= soffsetp[ord[i]])
					break;
		} else {
			/*
			 * Sort by delay.
			 */
			for (i = 0; i < PEER_SHIFT-1; i++)
				if (delayp[ord[i]] == 0
				    || sample_delay <= delayp[ord[i]])
					break;
		}

		for (j = PEER_SHIFT-1; j > i; j--)
			ord[j] = ord[j-1];
	}
	ord[i] = peer->filter_nextpt;

	/*
	 * Got everything in order.  Insert sample in current register
	 * and increment nextpt.
	 */
	peer->trust <<= 1;
	if (!trustable)
		peer->trust |= 1;

	peer->filter_delay[peer->filter_nextpt] = sample_delay;
	peer->filter_offset[peer->filter_nextpt] = *sample_offset;
	peer->filter_soffset[peer->filter_nextpt] = sample_soffset;
	peer->filter_nextpt++;
	if (peer->filter_nextpt >= PEER_SHIFT)
		peer->filter_nextpt = 0;
	
	/*
	 * Now compute the dispersion, and assign values to estdelay and
	 * estoffset.  If there are no samples in the register, estdelay and
	 * estoffset go to zero and estdisp is set to the maximum.
	 */
	if (peer->filter_delay[ord[0]] == 0) {
		peer->estdelay = 0;
		peer->estoffset.l_ui = peer->estoffset.l_uf = 0;
		peer->estsoffset = 0;
		peer->estdisp = PEER_MAXDISP;
	} else {
		register s_fp d;

		peer->estdelay = peer->filter_delay[ord[0]];
		peer->estoffset = peer->filter_offset[ord[0]];
		peer->estsoffset = LFPTOFP(&peer->estoffset);
		peer->estdisp = 0;
		for (i = 1; i < PEER_SHIFT; i++) {
			if (peer->filter_delay[ord[i]] == 0)
				d = PEER_MAXDISP;
			else {
				d = peer->filter_soffset[ord[i]]
				    - peer->filter_soffset[ord[0]];
				if (d < 0)
					d = -d;
				if (d > PEER_MAXDISP)
					d = PEER_MAXDISP;
			}
			/*
			 * XXX This *knows* PEER_FILTER is 1/2
			 */
			peer->estdisp += (u_fp)(d) >> i;
		}
		
		if (peer->hmode == MODE_BCLIENT) {
			register s_fp mdisp;
			/*
			 * Note that BCLIENT delays aren't really
			 * significant, since they really consist
			 * of the precision of the peers involved
			 * plus a constant value.  Yet the clock
			 * selection procedure relies on them heavily
			 * as a quality indicator.  What we do here
			 * is compute the mean dispersion of the
			 * samples in the registers and add twice
			 * that to peer.estdelay.  This is an estimate
			 * of the mean delay to the peer.  Doing this
			 * will (1) make BCLIENT delays look worse than
			 * polled delays, meaning the polled servers
			 * will be preferred a little, and (2) makes
			 * the noisiest servers look the worst.
			 */
			mdisp = 0;
			for (i = 1; i < PEER_SHIFT; i++) {
				if (peer->filter_delay[ord[i]] == 0)
					mdisp += PEER_MAXDSPDEL;
				else {
					d = peer->filter_soffset[ord[0]]
					    - peer->filter_soffset[ord[i]];
					if (d > PEER_MAXDSPDEL)
						mdisp += PEER_MAXDSPDEL;
					else
						mdisp += d;
				}
			}
			peer->estdelay += (mdisp << 1)/PEER_SHIFT;
		}
	}
	/*
	 * We're done
	 */
}



/*
 * clock_select - find the pick-of-the-litter clock
 */
void
clock_select(changed_peer)
	struct peer *changed_peer;	/* for a possible future optimization */
{
	register struct peer *peer;
	register int i;
	register int nlist;
	register s_fp d;
	register int j;
	register int n;
	u_fp local_threshold;
	struct peer *peer_list[NTP_MAXLIST];
	u_fp peer_badness[NTP_MAXLIST];
	struct peer *osys_peer;

#ifdef DEBUG
	if (debug)
		printf("clock_select()\n");
#endif
	/*
	 * Calculate the fixed part of the dispersion limit
	 */
	local_threshold = (FP_SECOND >> (-(int)sys_precision))
	    + sys_maxskew;

	/*
	 * This first chunk of code is supposed to go through all
	 * peers we know about to find the NTP_MAXLIST peers which
	 * are most likely to succeed.  We run through the list
	 * doing the sanity checks and trying to insert anyone who
	 * looks okay.  We are at all times aware that we should
	 * only keep samples from the top two strata and we only need
	 * NTP_MAXLIST of them.
	 */
	nlist = 0;	/* none yet */
	for (n = 0; n < HASH_SIZE; n++) {
		for (peer = peer_hash[n]; peer != 0; peer = peer->next) {
			/*
			 * Clear peer selection stats
			 */
			peer->falseticker = 0;
			peer->candidate = 0;
			peer->select = 0;
			peer->select_total = 0;
			peer->was_sane = 0;

			if (peer->estdelay == 0)
				continue;	/* not initialized */
			if (peer->stratum >= NTP_INFIN)
				continue;	/* stratum no good */
			if (peer->stratum > 1
			    && peer->refid == peer->dstadr->sin.sin_addr.s_addr)
				continue;	/* sync loop */
			if (peer->estdelay > NTP_MAXWGT) {
				peer->seldelaytoolarge++;
				continue;	/* too far away */
			}
			if (peer->estdisp > NTP_MAXWGT) {
				peer->seldisptoolarge++;
				continue;	/* too noisy or broken */
			}
			if (peer->org.l_ui < peer->reftime.l_ui) {
				peer->selbroken++;
				continue;	/* very broken host */
			}
			if (peer->trust != 0) {
				continue;	/* not trustworthy */
			}

			/*
			 * This one seems sane.  Find where he belongs
			 * on the list.
			 */
			peer->was_sane = 1;
			if (peer == sys_peer) {
				d = 0;
			} else {
				d = peer->estdisp + peer->dispersion
				    + local_threshold
				    + (FP_SECOND >> (-(int)peer->precision));
			}
			for (i = 0; i < nlist; i++)
				if (peer->stratum <= peer_list[i]->stratum)
					break;
			for ( ; i < nlist; i++) {
				if (peer->stratum < peer_list[i]->stratum)
					break;
				if (d < peer_badness[i])
					break;
			}

			/*
			 * If i points past the end of the list, this
			 * guy is a loser, else stick him in.
			 */
			if (i >= NTP_MAXLIST)
				continue;
			for (j = nlist; j > i; j--)
				if (j < NTP_MAXLIST) {
					peer_list[j] = peer_list[j-1];
					peer_badness[j]
					    = peer_badness[j-1];
				}
			
			peer_list[i] = peer;
			peer_badness[i] = d;
			if (nlist < NTP_MAXLIST)
				nlist++;
		}
	}

	/*
	 * Got the five-or-less best.  Cut the list where the number of
	 * strata exceeds two.
	 */
	j = 0;
	for (i = 1; i < nlist; i++)
		if (peer_list[i]->stratum > peer_list[i-1]->stratum)
			if (++j == 2) {
				nlist = i;
				break;
			}

	/*
	 * Whew!  What we should have by now is 0 to 5 candidates for
	 * the job of syncing us.  If we have none, we're out of luck.
	 * If we have one, he's a winner.  If we have more, do falseticker
	 * detection.  First record the position of each peer in the
	 * the list for posterity.  Also determine if our system peer
	 * made it.
	 */
	for (i = 0; i < nlist; i++) {
		peer_list[i]->candidate = i+1;
		peer_list[i]->select_total = nlist;
	}

	osys_peer = sys_peer;
	if (nlist == 0)
		sys_peer = 0;
	else if (nlist == 1) {
		sys_peer = peer_list[0];
		sys_peer->falseticker = 1;
		sys_peer->select = 1;
	} else {
		/*
		 * Re-sort by stratum, bdelay estimate quality and
		 * peer.estdelay.
		 */
		for (i = 0; i < nlist-1; i++)
			for (j = i+1; j < nlist; j++) {
				if (peer_list[i]->stratum
				    < peer_list[j]->stratum)
					break;	/* already sorted by stratum */
				if (peer_list[i]->estdelay
				    < peer_list[j]->estdelay)
					continue;
				if (peer_list[i]->estdelay
				    > peer_list[j]->estdelay)
					goto swapit;	/* sorry */
				if (peer_list[i] == sys_peer)
					continue;
				if (peer_list[j] == sys_peer)
					goto swapit;	/* and again */
				if (ranp2(1) == 0)
					continue;
swapit:
				peer = peer_list[i];
				peer_list[i] = peer_list[j];
				peer_list[j] = peer;
			}
		
		/*
		 * Record the ordering of peers before dropping any.
		 */
		for (i = 0; i < nlist; i++)
			peer_list[i]->falseticker = i+1;

		/*
		 * Now drop samples until we're down to one.
		 */
		while (nlist > 1) {
			for (n = 0; n < nlist; n++) {
				peer_badness[n] = 0;
				peer = peer_list[n];
				switch (sys_select_algorithm) {
				case SELECT_1:
				    /*
				     * This code uses the 3/4 weight
				     * from the spec.
				     */
				    for (j = 0; j < nlist; j++) {
				        if (j == n)	/* with self? */
					    continue;
					d = peer_list[j]->estsoffset
					    - peer->estsoffset;
					if (d < 0)  /* absolute value */
					    d = -d;
					/*
					 * XXX This code *knows* that
					 * NTP_SELECT is 3/4
					 */
					for (i = 0; i < j; i++)
					    d = (d>>1) + (d>>2);
					  peer_badness[n] += d;
				    }
				    break;

				case SELECT_2:
				    /*
				     * This code reduces the wieghting
				     * of higher stratum peers with
				     * respect to lower by about 9%
				     */
				    for (j = 0; j < nlist; j++) {
					if (j == n)	/* with self? */
					    continue;
					d = peer_list[j]->estsoffset
					    - peer->estsoffset;
					if (d < 0)  /* absolute value */
					    d = -d;

					if (j > 0) {
					    /*
					     * Use the normal
					     * 3/4 weight but
					     * stop one short.
					     */
					    for (i = 0; i < (j-1); i++)
						d = (d>>1) + (d>>2);

					    /*
					     * If the stratum of the
					     * jth peer is greater
					     * than the target, use
					     * a weight of 11/16
					     * instead of 3/4.
					     */
					    if (peer_list[j]->stratum
					      > peer->stratum)
						d = (d>>1) + (d>>3) + (d>>4);
					    else
						d = (d>>1) + (d>>2);
					}
					peer_badness[n] += d;
				    }
				    break;

				case SELECT_3:
				    /*
				     * Like above, except weighting
				     * reduced by 18%.
				     */
				    for (j = 0; j < nlist; j++) {
					if (j == n)	/* with self? */
					    continue;
					d = peer_list[j]->estsoffset
					  - peer->estsoffset;
					if (d < 0)  /* absolute value */
					    d = -d;

					if (j > 0) {
					    /*
					     * Use the normal
					     * 3/4 weight but
					     * stop one short.
					     */
					    for (i = 0; i < (j-1); i++)
						d = (d>>1) + (d>>2);

					    /*
					     * If the stratum of the
					     * jth peer is greater
					     * than the target, use
					     * a weight of 5/8
					     * instead of 3/4.
					     */
					    if (peer_list[j]->stratum
					      > peer->stratum)
						d = (d>>1) + (d>>3);
					    else
						d = (d>>1) + (d>>2);
					}
					peer_badness[n] += d;
				    }
				    break;

				case SELECT_4:
				    /*
				     * This code uses the spec algorithm
				     * but with a weight of 11/16
				     */
				    for (j = 0; j < nlist; j++) {
				        if (j == n)	/* with self? */
					    continue;
					d = peer_list[j]->estsoffset
					    - peer->estsoffset;
					if (d < 0)  /* absolute value */
					    d = -d;
					/*
					 * XXX This code *knows* that
					 * NTP_SELECT is 11/16
					 */
					for (i = 0; i < j; i++)
					    d = (d>>1) + (d>>3) + (d>>4);
					  peer_badness[n] += d;
				    }
				    break;

				case SELECT_5:
				    /*
				     * This code uses the spec algorithm
				     * but with a weight of 5/8
				     */
				    for (j = 0; j < nlist; j++) {
				        if (j == n)	/* with self? */
					    continue;
					d = peer_list[j]->estsoffset
					    - peer->estsoffset;
					if (d < 0)  /* absolute value */
					    d = -d;
					/*
					 * XXX This code *knows* that
					 * NTP_SELECT is 5/8
					 */
					for (i = 0; i < j; i++)
					    d = (d>>1) + (d>>3);
					  peer_badness[n] += d;
				    }
				    break;

				default:
				    /*
				     * Hideous error.  Die for this.
				     */
				    syslog(LOG_ERR,
			"clock_select: select algorithm is %d!!!  Bye-bye.",
					sys_select_algorithm);
				    exit(1);
				}
			}

			/*
			 * We now have an array of nlist badness
			 * coefficients.  Find the badest.  Find
			 * the minimum precision while we're at
			 * it.
			 */
			i = 0;
			n = peer_list[0]->precision;;
			for (j = 1; j < nlist; j++) {
				if (peer_badness[j] >= peer_badness[i])
					i = j;
				if (n > peer_list[j]->precision)
					n = peer_list[j]->precision;
			}
			
			/*
			 * i is the index of the peer with the worst
			 * dispersion.  If his dispersion is less than
			 * the threshold, stop now, else delete him and
			 * continue around again.
			 */
			if (peer_badness[i] < (local_threshold
			    + (FP_SECOND >> (-n))))
				break;
			for (j = i + 1; j < nlist; j++)
				peer_list[j-1] = peer_list[j];
			nlist--;
		}

		/*
		 * What remains is a list of less than 5 peers.  First
		 * record their order, then choose a peer.  If the
		 * head of the list has a polling interval of NTP_MINPOLL
		 * choose him right off.  If not, see if sys_peer is in
		 * the list.  If so, keep him.  If not, take the top of
		 * the list anyway.
		 */
		for (i = 0; i < nlist; i++)
			peer_list[i]->select = i+1;

		if (peer_list[0]->ppoll <= NTP_MINPOLL
		    || peer_list[0]->hpoll <= NTP_MINPOLL
		    || sys_peer == 0
		    || sys_peer->stratum > peer_list[0]->stratum) {
			sys_peer = peer_list[0];
		} else {
			for (i = 1; i < nlist; i++)
				if (peer_list[i] == sys_peer)
					break;
			if (i < nlist)
				sys_wanderhold++;
			else
				sys_peer = peer_list[0];
		}
	}

	/*
	 * If we got a new system peer from all of this, clamp his polling
	 * interval.  Also report the event.
	 */
	if (osys_peer != sys_peer) {
		report_event(EVNT_PEERSTCHG, (struct peer *)0);
		if (sys_peer != 0)
			poll_update(sys_peer, NTP_MINPOLL, POLL_NOTRANDOM);
	}
}




/*
 * fast_xmit - fast path send for stateless (non-)associations
 */
void
fast_xmit(rbufp, rmode, authentic)
	struct recvbuf *rbufp;
	int rmode;
	int authentic;
{
	struct pkt xpkt;
	register struct pkt *rpkt;
	u_char xmode;
	u_short xkey;
	int docrypt;
	l_fp xmt_ts;
	extern void unpeer();
	extern void auth1crypt();
	extern void auth2crypt();
	extern void sendpkt();
	extern void get_systime();
	extern char *ntoa();

#ifdef DEBUG
	if (debug)
		printf("fast_xmit(%s, %d)\n", ntoa(&rbufp->recv_srcadr), rmode);
#endif

	/*
	 * Make up new packet and send it quick
	 */
	rpkt = &rbufp->recv_pkt;
	if (rmode == MODE_ACTIVE)
		xmode = MODE_PASSIVE;
	else
		xmode = MODE_SERVER;

	if (rbufp->recv_length == LEN_PKT_MAC) {
		docrypt = 1;
		if (authentic)
			xkey = ntohl(rpkt->keyid);
		else
			xkey = 0;
	} else {
		docrypt = 0;
	}

	xpkt.li_vn_mode = PKT_LI_VN_MODE(sys_leap,
	    PKT_VERSION(rpkt->li_vn_mode), xmode);
	xpkt.stratum = STRATUM_TO_PKT(sys_stratum);
	xpkt.ppoll = max(NTP_MINPOLL, rpkt->ppoll);
	xpkt.precision = sys_precision;
	xpkt.distance = HTONS_FP(sys_distance);
	xpkt.dispersion = HTONS_FP(sys_dispersion);
	xpkt.refid = sys_refid;
	HTONL_FP(&sys_reftime, &xpkt.reftime);
	xpkt.org = rpkt->xmt;
	HTONL_FP(&rbufp->recv_time, &xpkt.rec);
#ifdef DES_OK
	/*
	 * If we are encrypting, do it.  Else don't.  Easy.
	 */
	if (docrypt)
	  {
	    xpkt.keyid = htonl(xkey);
	    auth1crypt(xkey, (u_int *)&xpkt, LEN_PKT_NOMAC);
	    get_systime(&xmt_ts);
	    L_ADDUF(&xmt_ts, sys_authdelay);
	    HTONL_FP(&xmt_ts, &xpkt.xmt);
	    auth2crypt(xkey, (u_int *)&xpkt, LEN_PKT_NOMAC);
	    sendpkt(&rbufp->recv_srcadr, rbufp->dstadr, &xpkt, LEN_PKT_MAC);
	  }
	else{
#endif /* DES_OK */
	  /*
	   * Get xmt timestamp, then send it without mac field
	   */
	  get_systime(&xmt_ts);
	  HTONL_FP(&xmt_ts, &xpkt.xmt);
	  sendpkt(&rbufp->recv_srcadr, rbufp->dstadr, &xpkt,
		  LEN_PKT_NOMAC);
	
#ifdef DES_OK
	}
#endif /* DES_OK */
      }



/*
 * init_proto - initialize the protocol module's data
 */
void
init_proto()
{
	/*
	 * Fill in the sys_* stuff.  Default is don't listen
	 * to broadcasting, don't authenticate.
	 */
	sys_leap = LEAP_NOTINSYNC;
	sys_stratum = STRATUM_UNSPEC;
	sys_precision = (s_char)DEFAULT_SYS_PRECISION;
	sys_distance = 0;
	sys_dispersion = 0;
	sys_refid = 0;
	sys_reftime.l_ui = sys_reftime.l_uf = 0;
	sys_hold = (PEER_SHIFT * (1<<NTP_MINPOLL));	/* wait before pick */
	sys_peer = 0;
	sys_maxskew = NTP_MAXSKW;

	sys_bclient = 0;
	sys_bdelay = DEFBROADDELAY;

	sys_authenticate = 0;
	sys_select_algorithm = SELECT_1;

	sys_stattime = 0;
	sys_badstratum = 0;
	sys_oldversionpkt = 0;
	sys_badlength = 0;
	sys_newversionpkt = 0;
	sys_unknownversion = 0;
	sys_processed = 0;
	sys_badauth = 0;
	sys_wanderhold = 0;
}



/*
 * proto_config - configure the protocol module
 */
void
proto_config(item, value)
	int item;
	int value;
{
	/*
	 * Figure out what he wants to change, then do it
	 */
	switch (item) {
	case PROTO_BROADCLIENT:
		/*
		 * Turn on/off facility to listen to broadcasts
		 */
		sys_bclient = (int)value;
		if (sys_bclient)
			io_setbclient();
		else
			io_unsetbclient();
		break;
	
	case PROTO_PRECISION:
		/*
		 * Set system precision
		 */
		sys_precision = (s_char)value;
		break;
	
	case PROTO_BROADDELAY:
		/*
		 * Set default broadcast delay
		 */
		sys_bdelay = (u_fp)value;
		break;
	
	case PROTO_AUTHENTICATE:
		/*
		 * Specify the use of authenticated data
		 */
		sys_authenticate = (int)value;
		break;


	case PROTO_AUTHDELAY:
		/*
		 * Provide an authentication delay value.  Round it to
		 * the microsecond.  This is crude.
		 */
		sys_authdelay = (((u_int)value) + 0x00000800) & 0xfffff000;
		break;

	case PROTO_MAXSKEW:
		/*
		 * Set the maximum skew value
		 */
		sys_maxskew = (u_fp)value;
		break;

	case PROTO_SELECT:
		/*
		 * Set the selection algorithm.  Check this value, since
		 * invalid values will break things.
		 */
		if (value < SELECT_1 || value > SELECT_5) {
			syslog(LOG_ERR,
			    "proto_config: illegal selection algorithm %d",
			    value);
		} else {
			sys_select_algorithm = (int)value;
		}
		break;

	default:
		/*
		 * Log this error
		 */
		syslog(LOG_ERR, "proto_config: illegal item %d, value %d",
		    item, value);
		break;
	}
}


/*
 * proto_clr_stats - clear protocol stat counters
 */
void
proto_clr_stats()
{
	sys_badstratum = 0;
	sys_oldversionpkt = 0;
	sys_newversionpkt = 0;
	sys_unknownversion = 0;
	sys_badlength = 0;
	sys_processed = 0;
	sys_badauth = 0;
	sys_wanderhold = 0;
	sys_stattime = current_time;
}

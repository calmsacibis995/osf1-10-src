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
static char     *sccsid = "@(#)$RCSfile: ntpdate.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/05/04 16:05:40 $";
#endif
/*
 */

/*
 * ntpdate - set the time of day by polling one or more NTP servers
 */
#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <netinet/in.h>

#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>
#include <ntp/ntp_unixtime.h>
#include <ntp/ntpdate.h>

#if defined(ULT_2_0)
#ifndef sigmask
#define	sigmask(m)	(1<<(m))
#endif
#endif

/*
 * Mask for blocking SIGIO and SIGALRM
 */
#define	BLOCKSIGMASK	(sigmask(SIGIO)|sigmask(SIGALRM))

/*
 * Scheduling priority we run at
 */
#define	NTPDATE_PRIO	(-12)

/*
 * Debugging flag
 */
int debug = 0;

/*
 * Initializing flag.  All async routines watch this and only do their
 * thing when it is clear.
 */
int initializing = 1;

/*
 * Alarm flag.  Set when an alarm occurs
 */
int alarm_flag = 0;

/*
 * Program name.
 */
char *progname;

/*
 * Systemwide parameters and flags
 */
int sys_samples = DEFSAMPLES;		/* number of samples/server */
u_int sys_timeout = DEFTIMEOUT;	/* timeout time, in TIMER_HZ units */
struct server **sys_servers;		/* the server list */
int sys_numservers = 0;			/* number of servers to poll */
int sys_maxservers = 0;			/* max number of servers to deal with */
int sys_authenticate = 0;		/* true when authenticating */
u_int sys_authkey = 0;			/* set to authentication key in use */
u_int sys_authdelay = 0;		/* authentication delay */
int sys_version = NTP_VERSION;		/* version to poll with */

/*
 * The current internal time
 */
u_int current_time = 0;

/*
 * Counter for keeping track of completed servers
 */
int complete_servers = 0;

/*
 * File of encryption keys
 */
#ifndef KEYFILE
#define	KEYFILE		"/etc/ntp.keys"
#endif	/* KEYFILE */

char *key_file = KEYFILE;

/*
 * Declarations for time format conversion tables
 */
extern int tstoushi[];
extern int tstousmid[];
extern int tstouslo[];

extern u_int ustotslo[];
extern u_int ustotsmid[];
extern u_int ustotshi[];

/*
 * Miscellaneous flags
 */
int syslogit = 0;
int verbose = 0;
int always_step = 0;

extern int errno;

/*
 * Main program.  Initialize us and loop waiting for I/O and/or
 * timer expiries.
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	int was_alarmed;
	struct recvbuf *rbuflist;
	struct recvbuf *rbuf;
	l_fp tmp;
	int errflg;
	int c;
	extern char *optarg;
	extern int optind;
	extern struct recvbuf *getrecvbufs();
	extern int getopt();
	void receive();
	void init_alarm();
	void timer();
	void init_io();
	void freerecvbuf();
	void addserver();
	void clock_adjust();
	extern char *Version;
	extern char *emalloc();

	errflg = 0;
	progname = argv[0];

	/*
	 * Decode argument list
	 */
	while ((c = getopt(argc, argv, "bdop:st:v")) != EOF)
		switch (c) {
		case 'a':
#ifdef DES_OK
			c = atoi(optarg);
			sys_authenticate = 1;
			sys_authkey = (u_int)c;
#else /* DES_OK */
			fprintf(stderr, "-a option (authentication) not supported\n");
#endif /* DES_OK */
			break;
		case 'b':
			always_step++;
			break;
		case 'd':
			++debug;
			break;
		case 'e':
#ifdef DES_OK
			if (!atolfp(optarg, &tmp)
			    || tmp.l_ui != 0) {
				(void) fprintf(stderr,
				    "%s: encryption delay %s is unlikely\n",
				    progname, optarg);
				errflg++;
			} else {
				sys_authdelay = tmp.l_uf;
			}
#else /* DES_OK */
                        fprintf(stderr, "-e option not supported\n");
#endif /* DES_OK */
			break;
		case 'k':
			key_file = optarg;
			break;
		case 'o':
			sys_version = NTP_OLDVERSION;
			break;
		case 'p':
			c = atoi(optarg);
			if (c <= 0 || c > PEER_SHIFT) {
				(void) fprintf(stderr,
				    "%s: number of samples (%d) is invalid\n",
				    progname, c);
				errflg++;
			} else {
				sys_samples = c;
			}
			break;
		case 's':
			syslogit = 1;
			break;
		case 't':
			if (!atolfp(optarg, &tmp)) {
				(void) fprintf(stderr,
				    "%s: timeout %s is undecodeable\n",
				    progname, optarg);
				errflg++;
			} else {
				sys_timeout = ((LFPTOFP(&tmp) * TIMER_HZ)
				    + 0x8000) >> 16;
				if (sys_timeout == 0)
					sys_timeout = 1;
			}
			break;
		case 'v':
			verbose = 1;
			break;
		case '?':
			++errflg;
			break;
		default:
			break;
		}
	
	sys_maxservers = argc - optind;
	if (errflg || sys_maxservers == 0) {
		(void) fprintf(stderr,
"usage: %s [-bdosv] [-p samples] [-t timeout] server ...\n",
		    progname);
		exit(2);
	}

	sys_servers = (struct server **)
	    emalloc(sys_maxservers * sizeof(struct server *));

	if (debug)
		setlinebuf(stdout);

	/*
	 * Logging.  Open the syslog if we have to
	 */
	if (syslogit) {
#ifndef	LOG_DAEMON
		openlog("ntpdate", LOG_PID);
#else

#ifndef	LOG_NTP
#define	LOG_NTP	LOG_DAEMON
#endif
		openlog("ntpdate", LOG_PID | LOG_NDELAY, LOG_NTP);
		if (debug)
			setlogmask(LOG_UPTO(LOG_DEBUG));
		else
			setlogmask(LOG_UPTO(LOG_INFO));
#endif	/* LOG_DAEMON */
	}

	if (debug || verbose)
		msyslog(LOG_INFO, "%s", Version);

	/*
	 * Add servers we are going to be polling
	 */
	for ( ; optind < argc; optind++)
		addserver(argv[optind]);

	if (sys_numservers == 0) {
		msyslog(LOG_ERR, "no servers can be used, exiting");
		exit(1);
	}

	/*
	 * Initialize the time of day routines and the I/O subsystem
	 */
#ifdef DES_OK
	if (sys_authenticate) {
		init_auth();
		if (!authreadkeys(key_file)) {
			msyslog(LOG_ERR, "no key file, exitting");
			exit(1);
		}
		if (!authhavekey(sys_authkey)) {
			char buf[10];

			(void) sprintf(buf, "%u", sys_authkey);
			msyslog(LOG_ERR, "authentication key %s unknown", buf);
			exit(1);
		}
	}
#endif /* DES_OK */

	init_io();
	init_alarm();

	/*
	 * Set the priority.
	 */
#if defined(NTPDATE_PRIO) && NTPDATE_PRIO != 0
	(void) setpriority(PRIO_PROCESS, 0, NTPDATE_PRIO);
#endif	/* ... */

	initializing = 0;

	/*
	 * Done all the preparation stuff, now the real thing.  We block
	 * SIGIO and SIGALRM and check to see if either has occured.
	 * If not, we pause until one or the other does.  We then call
	 * the timer processing routine and/or feed the incoming packets
	 * to the protocol module.  Then around again.  Continue until
	 * everything is completed.
	 */
	was_alarmed = 0;
	rbuflist = (struct recvbuf *)0;
	while (complete_servers < sys_numservers) {
		int omask;

		omask = sigblock(BLOCKSIGMASK);
		if (alarm_flag) {		/* alarmed? */
			was_alarmed = 1;
			alarm_flag = 0;
		}
		rbuflist = getrecvbufs();	/* get received buffers */

		if (!was_alarmed && rbuflist == (struct recvbuf *)0) {
			/*
			 * Nothing to do.  Wait for something.
			 */
			sigpause(omask);
			if (alarm_flag) {		/* alarmed? */
				was_alarmed = 1;
				alarm_flag = 0;
			}
			rbuflist = getrecvbufs();  /* get received buffers */
		}
		(void)sigsetmask(omask);

		/*
		 * Out here, signals are unblocked.  Call receive
		 * procedure for each incoming packet.
		 */
		while (rbuflist != (struct recvbuf *)0) {
			rbuf = rbuflist;
			rbuflist = rbuf->next;
			receive(rbuf);
			freerecvbuf(rbuf);
		}

		/*
		 * Call timer to process any timeouts
		 */
		if (was_alarmed) {
			timer();
			was_alarmed = 0;
		}

		/*
		 * Go around again
		 */
	}

	/*
	 * When we get here we've completed the polling of all servers.
	 * Adjust the clock, then exit.
	 */
	clock_adjust();
	exit(0);
}


/*
 * transmit - transmit a packet to the given server, or mark it completed.
 *	      This is called by the timeout routine and by the receive
 *	      procedure.
 */
void
transmit(server)
	register struct server *server;
{
	struct pkt xpkt;
	void server_data();
	extern char *ntoa();
	void sendpkt();
	void get_systime();

	if (debug)
		printf("transmit(%s)\n", ntoa(&server->srcadr));

	if (server->filter_nextpt < server->xmtcnt) {
		l_fp ts;
		/*
		 * Last message to this server timed out.  Shift
		 * zeros into the filter.
		 */
		ts.l_ui = ts.l_uf = 0;
		server_data(server, 0, &ts);
	}

	if (server->filter_nextpt >= sys_samples) {
		/*
		 * Got all the data we need.  Mark this guy
		 * completed and return.
		 */
		server->event_time = 0;
		complete_servers++;
		return;
	}

	/*
	 * If we're here, send another message to the server.  Fill in
	 * the packet and let 'er rip.
	 */
	xpkt.li_vn_mode = PKT_LI_VN_MODE(LEAP_NOTINSYNC,
	    sys_version, MODE_CLIENT);
	xpkt.stratum = STRATUM_TO_PKT(STRATUM_UNSPEC);
	xpkt.ppoll = NTP_MINPOLL;
	xpkt.precision = NTPDATE_PRECISION;
	xpkt.distance = htonl(NTPDATE_DISTANCE);
	xpkt.dispersion = htonl(NTPDATE_DISP);
	xpkt.refid = htonl(NTPDATE_REFID);
	xpkt.reftime.l_ui = xpkt.reftime.l_uf = 0;
	xpkt.org.l_ui = xpkt.org.l_uf = 0;
	xpkt.rec.l_ui = xpkt.rec.l_uf = 0;

	/*
	 * Determine whether to authenticate or not.  If so,
	 * fill in the extended part of the packet and do it.
	 * If not, just timestamp it and send it away.
	 */
#ifdef DES_OK
	if (sys_authenticate) {
		xpkt.keyid = htonl(sys_authkey);
		auth1crypt(sys_authkey, (u_int *)&xpkt, LEN_PKT_NOMAC);
		get_systime(&server->xmt);
		L_ADDUF(&server->xmt, sys_authdelay);
		HTONL_FP(&server->xmt, &xpkt.xmt);
		auth2crypt(sys_authkey, (u_int *)&xpkt, LEN_PKT_NOMAC);
		sendpkt(&(server->srcadr), &xpkt, LEN_PKT_MAC);

		if (debug > 1)
			printf("transmit auth to %s\n",
			    ntoa(&(server->srcadr)));
	} else {
#endif /* DES_OK */
		get_systime(&(server->xmt));
		HTONL_FP(&server->xmt, &xpkt.xmt);
		sendpkt(&(server->srcadr), &xpkt, LEN_PKT_NOMAC);

		if (debug > 1)
			printf("transmit to %s\n", ntoa(&(server->srcadr)));
#ifdef DES_OK
	}
#endif /* DES_OK */

	/*
	 * Update the server timeout and transmit count
	 */
	server->event_time = current_time + sys_timeout;
	server->xmtcnt++;
}


/*
 * receive - receive and process an incoming frame
 */
void
receive(rbufp)
	struct recvbuf *rbufp;
{
	register struct pkt *rpkt;
	register struct server *server;
	register s_fp di;
	register u_int t10_ui, t10_uf;
	register u_int t23_ui, t23_uf;
	l_fp org;
	l_fp rec;
	l_fp ci;
	int has_mac;
	int is_authentic;
	struct server *findserver();
	void server_data();
	extern char *lfptoa(), *fptoa();
	extern char *ntoa();

	if (debug)
		printf("receive(%s)\n", ntoa(&rbufp->srcadr));
	/*
	 * Check to see if the packet basically looks like something
	 * intended for us.
	 */
	if (rbufp->recv_length == LEN_PKT_NOMAC)
		has_mac = 0;
	else if (rbufp->recv_length == LEN_PKT_MAC)
		has_mac = 1;
	else {
		if (debug)
			printf("receive: packet length %d\n",
			    rbufp->recv_length);
		return;		/* funny length packet */
	}

	rpkt = &(rbufp->recv_pkt);
	if (PKT_VERSION(rpkt->li_vn_mode) == NTP_OLDVERSION) {
#ifdef notdef
		/*
		 * Fuzzballs do encryption but still claim
		 * to be version 1.
		 */
		if (has_mac)
			return;
#endif
	} else if (PKT_VERSION(rpkt->li_vn_mode) != NTP_VERSION) {
		return;
	}

	if ((PKT_MODE(rpkt->li_vn_mode) != MODE_SERVER
	    && PKT_MODE(rpkt->li_vn_mode) != MODE_PASSIVE)
	    || rpkt->stratum > NTP_INFIN) {
		if (debug)
			printf("receive: mode %d stratum %d\n",
			    PKT_MODE(rpkt->li_vn_mode), rpkt->stratum);
		return;
	}
	
	/*
	 * So far, so good.  See if this is from a server we know.
	 */
	server = findserver(&(rbufp->srcadr));
	if (server == NULL) {
		if (debug)
			printf("receive: server not found\n");
		return;
	}

	/*
	 * Decode the org timestamp and make sure we're getting a response
	 * to our last request.
	 */
	NTOHL_FP(&rpkt->org, &org);
	if (!L_ISEQU(&org, &server->xmt)) {
		if (debug)
			printf("receive: pkt.org and peer.xmt differ\n");
		return;
	}
	
#ifdef DES_OK
	/*
	 * Check out the authenticity if we're doing that.
	 */
	if (!sys_authenticate)
		is_authentic = 1;
	else {
		is_authentic = 0;
		if (has_mac && ntohl(rpkt->keyid) == sys_authkey) {
			if (authdecrypt(sys_authkey, (u_int *)rpkt),
			    LEN_PKT_NOMAC)
				is_authentic = 1;
		}
	}
	server->trust <<= 1;
	if (!is_authentic)
		server->trust |= 1;
#endif /* DES_OK */
	
	/*
	 * Looks good.  Record info from the packet.
	 */
	server->leap = PKT_LEAP(rpkt->li_vn_mode);
	server->stratum = PKT_TO_STRATUM(rpkt->stratum);
	server->precision = rpkt->precision;
	server->distance = ntohl(rpkt->distance);
	server->dispersion = ntohl(rpkt->dispersion);
	server->refid = rpkt->refid;
	NTOHL_FP(&rpkt->reftime, &server->reftime);
	NTOHL_FP(&rpkt->rec, &rec);
	NTOHL_FP(&rpkt->xmt, &server->org);

	/*
	 * Make sure the server is at least somewhat sane.  If not, try
	 * again.
	 */
	if ((rec.l_ui == 0 && rec.l_uf == 0) || !L_ISHIS(&server->org, &rec)) {
		transmit(server);
		return;
	}

	/*
	 * Calculate the round trip delay (di) and the clock offset (ci).
	 * We use the equations (reordered from those in the spec):
	 *
	 * d = (t2 - t3) - (t1 - t0)
	 * c = ((t2 - t3) + (t1 - t0)) / 2
	 */
	t10_ui = server->org.l_ui;	/* pkt.xmt == t1 */
	t10_uf = server->org.l_uf;
	M_SUB(t10_ui, t10_uf, rbufp->recv_time.l_ui,
	    rbufp->recv_time.l_uf);	/* recv_time == t0*/

	t23_ui = rec.l_ui;	/* pkt.rec == t2 */
	t23_uf = rec.l_uf;
	M_SUB(t23_ui, t23_uf, org.l_ui, org.l_uf);	/* pkt->org == t3 */

	/* now have (t2 - t3) and (t0 - t1).  Calculate (ci) and (di) */
	ci.l_ui = t10_ui;
	ci.l_uf = t10_uf;
	M_ADD(ci.l_ui, ci.l_uf, t23_ui, t23_uf);
	M_RSHIFT(ci.l_i, ci.l_uf);

	/*
	 * Calculate di in t23 in full precision, then truncate
	 * to an s_fp.
	 */
	M_SUB(t23_ui, t23_uf, t10_ui, t10_uf);
	di = MFPTOFP(t23_ui, t23_uf);

	if (debug > 3)
		printf("offset: %s, delay %s\n", lfptoa(&ci, 9), fptoa(di, 4));

	di += (FP_SECOND >> (-(int)NTPDATE_PRECISION))
	    + (FP_SECOND >> (-(int)server->precision)) + NTP_MAXSKW;

	if (di <= 0) {		/* value still too raunchy to use? */
		ci.l_ui = ci.l_uf = 0;
		di = 0;
	} else {
		di = max(di, NTP_MINDIST);
	}

	/*
	 * Shift this data in, then transmit again.
	 */
	server_data(server, (u_fp) di, &ci);
	transmit(server);
}


/*
 * server_data - add a sample to the server's filter registers
 */
void
server_data(server, d, c)
	register struct server *server;
	u_fp d;
	l_fp *c;
{
	register int i;

	i = server->filter_nextpt;
	if (i < PEER_SHIFT) {
		server->filter_delay[i] = d;
		server->filter_offset[i] = *c;
		server->filter_soffset[i] = MFPTOFP(c->l_ui, c->l_uf);
		server->filter_nextpt = i + 1;
	}
}


/*
 * clock_filter - determine a server's estdelay, estdisp and estoffset
 */
void
clock_filter(server)
	register struct server *server;
{
	register int i, j;
	int ord[PEER_SHIFT];

	/*
	 * Sort indices into increasing delay order
	 */
	for (i = 0; i < sys_samples; i++)
		ord[i] = i;
	
	for (i = 0; i < (sys_samples-1); i++) {
		for (j = i+1; j < sys_samples; j++) {
			if (server->filter_delay[ord[j]] == 0)
				continue;
			if (server->filter_delay[ord[i]] == 0
			    || (server->filter_delay[ord[i]]
			    > server->filter_delay[ord[j]])) {
				register int tmp;

				tmp = ord[i];
				ord[i] = ord[j];
				ord[j] = tmp;
			}
		}
	}

	/*
	 * Now compute the dispersion, and assign values to estdelay and
	 * estoffset.  If there are no samples in the register, estdelay and
	 * estoffset go to zero and estdisp is set to the maximum.
	 */
	if (server->filter_delay[ord[0]] == 0) {
		server->estdelay = 0;
		server->estoffset.l_ui = server->estoffset.l_uf = 0;
		server->estsoffset = 0;
		server->estdisp = PEER_MAXDISP;
	} else {
		register s_fp d;

		server->estdelay = server->filter_delay[ord[0]];
		server->estoffset = server->filter_offset[ord[0]];
		server->estsoffset = LFPTOFP(&server->estoffset);
		server->estdisp = 0;
		for (i = 1; i < sys_samples; i++) {
			if (server->filter_delay[ord[i]] == 0)
				d = PEER_MAXDISP;
			else {
				d = server->filter_soffset[ord[i]]
				    - server->filter_soffset[ord[0]];
				if (d < 0)
					d = -d;
				if (d > PEER_MAXDISP)
					d = PEER_MAXDISP;
			}
			/*
			 * XXX This *knows* PEER_FILTER is 1/2
			 */
			server->estdisp += (u_fp)(d) >> i;
		}
	}
	/*
	 * We're done
	 */
}


/*
 * clock_select - select the pick-of-the-litter clock from the samples
 *		  we've got.
 */
struct server *
clock_select()
{
	register struct server *server;
	register int i;
	register int nlist;
	register s_fp d;
	register int j;
	register int n;
	s_fp local_threshold;
	struct server *server_list[NTP_MAXLIST];
	u_fp server_badness[NTP_MAXLIST];
	struct server *sys_server;

	/*
	 * This first chunk of code is supposed to go through all
	 * servers we know about to find the NTP_MAXLIST servers which
	 * are most likely to succeed.  We run through the list
	 * doing the sanity checks and trying to insert anyone who
	 * looks okay.  We are at all times aware that we should
	 * only keep samples from the top two strata and we only need
	 * NTP_MAXLIST of them.
	 */
	nlist = 0;	/* none yet */
	for (n = 0; n < sys_numservers; n++) {
		server = sys_servers[n];
		if (server->estdelay == 0)
			continue;	/* no data */
		if (server->stratum > NTP_INFIN)
			continue;	/* stratum no good */
		if (server->estdelay > NTP_MAXWGT) {
			continue;	/* too far away */
		}
		if (server->leap == LEAP_NOTINSYNC)
			continue;	/* he's in trouble */
		if (server->org.l_ui < server->reftime.l_ui) {
			continue;	/* very broken host */
		}
		if ((server->org.l_ui - server->reftime.l_ui)
		    >= NTP_MAXAGE) {
			continue;	/* too long without sync */
		}
		if (server->trust != 0) {
			continue;
		}

		/*
		 * This one seems sane.  Find where he belongs
		 * on the list.
		 */
		d = server->estdisp + server->dispersion;
		for (i = 0; i < nlist; i++)
			if (server->stratum <= server_list[i]->stratum)
				break;
		for ( ; i < nlist; i++) {
			if (server->stratum < server_list[i]->stratum)
				break;
			if (d < server_badness[i])
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
				server_list[j] = server_list[j-1];
				server_badness[j]
				    = server_badness[j-1];
			}

		server_list[i] = server;
		server_badness[i] = d;
		if (nlist < NTP_MAXLIST)
			nlist++;
	}

	/*
	 * Got the five-or-less best.  Cut the list where the number of
	 * strata exceeds two.
	 */
	j = 0;
	for (i = 1; i < nlist; i++)
		if (server_list[i]->stratum > server_list[i-1]->stratum)
			if (++j == 2) {
				nlist = i;
				break;
			}

	/*
	 * Whew!  What we should have by now is 0 to 5 candidates for
	 * the job of syncing us.  If we have none, we're out of luck.
	 * If we have one, he's a winner.  If we have more, do falseticker
	 * detection.
	 */

	if (nlist == 0)
		sys_server = 0;
	else if (nlist == 1) {
		sys_server = server_list[0];
	} else {
		/*
		 * Re-sort by stratum, bdelay estimate quality and
		 * server.estdelay.
		 */
		for (i = 0; i < nlist-1; i++)
			for (j = i+1; j < nlist; j++) {
				if (server_list[i]->stratum
				    < server_list[j]->stratum)
					break;	/* already sorted by stratum */
				if (server_list[i]->estdelay
				    < server_list[j]->estdelay)
					continue;
				server = server_list[i];
				server_list[i] = server_list[j];
				server_list[j] = server;
			}
		
		/*
		 * Calculate the fixed part of the dispersion limit
		 */
		local_threshold = (FP_SECOND >> (-(int)NTPDATE_PRECISION))
		    + NTP_MAXSKW;

		/*
		 * Now drop samples until we're down to one.
		 */
		while (nlist > 1) {
			for (n = 0; n < nlist; n++) {
				server_badness[n] = 0;
				for (j = 0; j < nlist; j++) {
					if (j == n)	/* with self? */
						continue;
					d = server_list[j]->estsoffset
					    - server_list[n]->estsoffset;
					if (d < 0)	/* absolute value */
						d = -d;
					/*
					 * XXX This code *knows* that
					 * NTP_SELECT is 3/4
					 */
					for (i = 0; i < j; i++)
						d = (d>>1) + (d>>2);
					server_badness[n] += d;
				}
			}

			/*
			 * We now have an array of nlist badness
			 * coefficients.  Find the badest.  Find
			 * the minimum precision while we're at
			 * it.
			 */
			i = 0;
			n = server_list[0]->precision;;
			for (j = 1; j < nlist; j++) {
				if (server_badness[j] >= server_badness[i])
					i = j;
				if (n > server_list[j]->precision)
					n = server_list[j]->precision;
			}
			
			/*
			 * i is the index of the server with the worst
			 * dispersion.  If his dispersion is less than
			 * the threshold, stop now, else delete him and
			 * continue around again.
			 */
			if (server_badness[i] < (local_threshold
			    + (FP_SECOND >> (-n))))
				break;
			for (j = i + 1; j < nlist; j++)
				server_list[j-1] = server_list[j];
			nlist--;
		}

		/*
		 * What remains is a list of less than 5 servers.  Take
		 * the best.
		 */
		sys_server = server_list[0];
	}

	/*
	 * That's it.  Return our server.
	 */
	return sys_server;
}


/*
 * clock_adjust - process what we've received, and adjust the time
 *	         if we got anything decent.
 */
void
clock_adjust()
{
	register int i;
	register struct server *server;
	s_fp absoffset;
	int dostep;
	extern char *ntoa();
	extern char *lfptoa();
	void printserver();

	for (i = 0; i < sys_numservers; i++)
		clock_filter(sys_servers[i]);
	server = clock_select();

	if (debug) {
		for (i = 0; i < sys_numservers; i++)
			printserver(sys_servers[i], stdout);
	}

	if (server == 0) {
		msyslog(LOG_ERR,
		    "no server suitable for synchronization found");
		return;
	}
	
	dostep = 1;
	if (!always_step) {
		absoffset = server->estsoffset;
		if (absoffset < 0)
			absoffset = -absoffset;
		if (absoffset < NTPDATE_THRESHOLD)
			dostep = 0;
	}

	if (dostep) {
		if (step_systime(&server->estoffset)) {
			msyslog(LOG_INFO, "step time server %s offset %s",
			    ntoa(&server->srcadr),
			    lfptoa(&server->estoffset, 7));
		}
	} else {
		if (adj_systime(&server->estoffset)) {
			msyslog(LOG_INFO, "adjust time server %s offset %s",
			    ntoa(&server->srcadr),
			    lfptoa(&server->estoffset, 7));
		}
	}
}


/*
 * addserver - determine a server's address and allocate a new structure
 *	       for it.
 */
void
addserver(serv)
	char *serv;
{
	register struct server *server;
	u_int netnum;
	static int toomany = 0;
	extern char *emalloc();

	if (sys_numservers >= sys_maxservers) {
		if (!toomany) {
			/*
			 * This is actually a `can't happen' now.  Leave
			 * the error message in anyway, though
			 */
			toomany = 1;
			msyslog(LOG_ERR,
		"too many servers (> %d) specified, remainder not used",
			    sys_maxservers);
		}
		return;
	}

	if (!getnetnum(serv, &netnum)) {
		msyslog(LOG_ERR, "can't find host %s\n", serv);
		return;
	}

	server = (struct server *)emalloc(sizeof(struct server));
	bzero((char *)server, sizeof(struct server));

	server->srcadr.sin_family = AF_INET;
	server->srcadr.sin_addr.s_addr = netnum;
	server->srcadr.sin_port = htons(NTP_PORT);

	sys_servers[sys_numservers++] = server;
	server->event_time = (u_int)sys_numservers;
}


/*
 * findserver - find a server in the list given its address
 */
struct server *
findserver(addr)
	struct sockaddr_in *addr;
{
	register int i;
	register u_int netnum;

	if (htons(addr->sin_port) != NTP_PORT)
		return 0;
	netnum = addr->sin_addr.s_addr;

	for (i = 0; i < sys_numservers; i++) {
		if (netnum == sys_servers[i]->srcadr.sin_addr.s_addr)
			return sys_servers[i];
	}
	return 0;
}


/*
 * timer - process a timer interrupt
 */
void
timer()
{
	register int i;

	/*
	 * Bump the current idea of the time
	 */
	current_time++;

	/*
	 * Search through the server list looking for guys
	 * who's event timers have expired.  Give these to
	 * the transmit routine.
	 */
	for (i = 0; i < sys_numservers; i++) {
		if (sys_servers[i]->event_time != 0
		    && sys_servers[i]->event_time <= current_time)
			transmit(sys_servers[i]);
	}
}



/*
 * init_alarm - set up the timer interrupt
 */
void
init_alarm()
{
	struct itimerval itimer;
	void alarming();

	alarm_flag = 0;

	/*
	 * Set up the alarm interrupt.  The first comes 1/(2*TIMER_HZ)
	 * seconds from now and they continue on every 1/TIMER_HZ seconds.
	 */
	(void) signal(SIGALRM, alarming);
	itimer.it_interval.tv_sec = itimer.it_value.tv_sec = 0;
	itimer.it_interval.tv_usec = 1000000/TIMER_HZ;
	itimer.it_value.tv_usec = 1000000/(TIMER_HZ<<1);
	setitimer(ITIMER_REAL, &itimer, (struct itimerval *)0);
}


/*
 * alarming - record the occurance of an alarm interrupt
 */
void
alarming()
{
	alarm_flag++;
}


/*
 * Data related to I/O support
 */

#ifndef FD_SET
#define	NFDBITS		32
#define	FD_SETSIZE	32
#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))
#endif

/*
 * We do asynchronous input using the SIGIO facility.  A number of
 * recvbuf buffers are preallocated for input.  In the signal
 * handler we poll to see if the socket is ready and read the
 * packets from it into the recvbuf's along with a time stamp and
 * an indication of the source host and the interface it was received
 * through.  This allows us to get as accurate receive time stamps
 * as possible independent of other processing going on.
 *
 * We allocate a number of recvbufs equal to the number of servers
 * plus 2.  This should be plenty.
 */

/*
 * Block the interrupt, for critical sections.
 */
#define	BLOCKIO(osig)	(osig = sigblock(sigmask(SIGIO)))
#define	UNBLOCKIO(osig)	((void) sigsetmask(osig))

/*
 * recvbuf lists
 */
struct recvbuf *freelist;	/* free buffers */
struct recvbuf *fulllist;	/* buffers with data */

int full_recvbufs;	/* number of full ones */
int free_recvbufs;

/*
 * File descriptor masks etc. for call to select
 */
int fd;
fd_set fdmask;

/*
 * init_io - initialize I/O data and open socket
 */
void
init_io()
{
	register int i;
	register struct recvbuf *rb;
	void input_handler();	/* input handler routine */
	extern char *emalloc();

	/*
	 * Init buffer free list and stat counters
	 */
	rb = (struct recvbuf *)
	    emalloc((sys_numservers + 2) * sizeof(struct recvbuf));
	freelist = 0;
	for (i = sys_numservers + 2; i > 0; i--) {
		rb->next = freelist;
		freelist = rb;
		rb++;
	}

	fulllist = 0;
	full_recvbufs = 0;
	free_recvbufs = sys_numservers + 2;

	/*
	 * Point SIGIO at service routine
	 */
	(void) signal(SIGIO, input_handler);

	/*
	 * Open the socket
	 */
	BLOCKIO(i);

	/* create a datagram (UDP) socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		msyslog(LOG_ERR, "socket() failed: %m");
		exit(1);
		/*NOTREACHED*/
	}

	/*
	 * bind the socket to the NTP port
	 */
	if (!debug) {
		struct sockaddr_in addr;

		bzero((char *)&addr, sizeof addr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(NTP_PORT);
		addr.sin_addr.s_addr = INADDR_ANY;
		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			if (errno == EADDRINUSE)
				msyslog(LOG_ERR,
				    "the NTP socket is in use, NOT... exiting");
			else {
				msyslog(LOG_ERR, "bind() fails: %m");
				exit(1);
			      }
		}
	}

	FD_ZERO(&fdmask);
	FD_SET(fd, &fdmask);

	/*
	 * set ourselves as the receiver of signals for this socket.
	 */
#if !defined(SUN_3_3) && !defined(ULT_2_0)
	/*
	 * The way it says on the manual page
	 */
	if (fcntl(fd, F_SETOWN, getpid()) == -1) {
#else
	/*
	 * The way Sun did it as recently as SunOS 3.5.  Only
	 * in the case of sockets, of course, just to confuse
	 * the issue.  Don't they even bother to test the stuff
	 * they send out?  Ibid for Ultrix 2.0
	 */
	if (fcntl(fd, F_SETOWN, -getpid()) == -1) {
#endif
		msyslog(LOG_ERR, "fcntl(F_SETOWN) fails: %m");
		exit(1);
	}

	/*
	 * set non-blocking, async I/O on the descriptor
	 */
	if (fcntl(fd, F_SETFL, FNDELAY|FASYNC) < 0) {
		msyslog(LOG_ERR, "fcntl(FNDELAY|FASYNC) fails: %m");
		exit(1);
		/*NOTREACHED*/
	}
	UNBLOCKIO(i);
}


/*
 * getrecvbufs - get receive buffers which have data in them
 *
 * ***N.B. must be called with SIGIO blocked***
 */
struct recvbuf *
getrecvbufs()
{
	struct recvbuf *rb;

	if (full_recvbufs == 0) {
		return (struct recvbuf *)0;	/* nothing has arrived */
	}
	
	/*
	 * Get the fulllist chain and mark it empty
	 */
	rb = fulllist;
	fulllist = 0;
	full_recvbufs = 0;

	/*
	 * Return the chain
	 */
	return rb;
}


/*
 * freerecvbuf - make a single recvbuf available for reuse
 */
void
freerecvbuf(rb)
	struct recvbuf *rb;
{
	int osig;

	BLOCKIO(osig);
	rb->next = freelist;
	freelist = rb;
	free_recvbufs++;
	UNBLOCKIO(osig);
}


/*
 * sendpkt - send a packet to the specified destination
 */
void
sendpkt(dest, pkt, len)
	struct sockaddr_in *dest;
	struct pkt *pkt;
	int len;
{
	int cc;
	extern char *ntoa();

	cc = sendto(fd, (char *)pkt, len, 0, (struct sockaddr *)dest,
	    sizeof(struct sockaddr_in));
	if (cc == -1) {
		if (errno != EWOULDBLOCK && errno != ENOBUFS)
			msyslog(LOG_ERR, "sendto(%s): %m", ntoa(dest));
	}
}


/*
 * input_handler - receive packets asynchronously
 */
void
input_handler()
{
	register int n;
	register struct recvbuf *rb;
	struct timeval tvzero;
	int fromlen;
	l_fp ts;
	fd_set fds;
	void get_systime();

	/*
	 * Do a poll to see if we have data
	 */
	for (;;) {
		fds = fdmask;
		tvzero.tv_sec = tvzero.tv_usec = 0;
		n = select(fd+1, &fds, (fd_set *)0, (fd_set *)0, &tvzero);

		/*
		 * If nothing to do, just return.  If an error occurred,
		 * complain and return.  If we've got some, freeze a
		 * timestamp.
		 */
		if (n == 0)
			return;
		else if (n == -1) {
			msyslog(LOG_ERR, "select() error: %m");
			return;
		}
		get_systime(&ts);

		/*
		 * Get a buffer and read the frame.  If we
		 * haven't got a buffer, or this is received
		 * on the wild card socket, just dump the packet.
		 */
		if (initializing || free_recvbufs == 0) {
			char buf[100];

			(void) read(fd, buf, sizeof buf);
			continue;
		}

		rb = freelist;
		freelist = rb->next;
		free_recvbufs--;

		fromlen = sizeof(struct sockaddr_in);
		rb->recv_length = recvfrom(fd, (char *)&rb->recv_pkt,
		    sizeof(rb->recv_pkt), 0,
		    (struct sockaddr *)&rb->srcadr, &fromlen);
		if (rb->recv_length == -1) {
			rb->next = freelist;
			freelist = rb;
			free_recvbufs++;
			continue;
		}

		/*
		 * Got one.  Mark how and when it got here,
		 * put it on the full list.
		 */
		rb->recv_time = ts;
		rb->next = fulllist;
		fulllist = rb;
		full_recvbufs++;
	}
}


/*
 * The following routines (get_systime, step_systime, adj_systime)
 * implement an interface between the (more or less) system independent
 * bits of ntpdate and the peculiarities of dealing with the Unix system
 * clock.
 */

/*
 * get_systime - return the system time in timestamp format
 */
void
get_systime(ts)
	l_fp *ts;
{
	struct timeval tv;

	/*
	 * Quickly get the time of day and convert it
	 */
	(void) gettimeofday(&tv, (struct timezone *)NULL);
	TVTOTS(&tv, ts);
	ts->l_uf += TS_ROUNDBIT;	/* guaranteed not to overflow */
	ts->l_ui += JAN_1970;
	ts->l_uf &= TS_MASK;
}


/*
 * step_systime - do a step adjustment in the system time
 */
int
step_systime(ts)
	l_fp *ts;
{
	struct timeval timetv, adjtv;
	int isneg = 0;
	l_fp offset;

	/*
	 * We can't help but be a little sloppy here, because of the
	 * hole between the gettimeofday() and the settimeofday().
	 * Oh, well.
	 */
	offset = *ts;
	if (L_ISNEG(&offset)) {
		isneg = 1;
		L_NEG(&offset);
	}
	TSTOTV(&offset, &adjtv);

	(void) gettimeofday(&timetv, (struct timezone *)NULL);
	if (isneg) {
		timetv.tv_sec -= adjtv.tv_sec;
		timetv.tv_usec -= adjtv.tv_usec;
		if (timetv.tv_usec < 0) {
			timetv.tv_sec--;
			timetv.tv_usec += 1000000;
		}
	} else {
		timetv.tv_sec += adjtv.tv_sec;
		timetv.tv_usec += adjtv.tv_usec;
		if (timetv.tv_usec >= 1000000) {
			timetv.tv_sec++;
			timetv.tv_usec -= 1000000;
		}
	}
	if (!debug)
		if (settimeofday(&timetv, (struct timezone *)NULL) != 0) {
			msyslog(LOG_ERR, "Can't set time of day: %m");
			return 0;
		}
	return 1;
}


/*
 * adj_systime - do a big long slew of the system time
 */
int
adj_systime(ts)
	l_fp *ts;
{
	struct timeval adjtv, oadjtv;
	int isneg = 0;
	l_fp offset;
	l_fp overshoot;

	/*
	 * Take the absolute value of the offset
	 */
	offset = *ts;
	if (L_ISNEG(&offset)) {
		isneg = 1;
		L_NEG(&offset);
	}

	/*
	 * Calculate the overshoot.  XXX N.B. This code *knows*
	 * ADJ_OVERSHOOT is 1/2.
	 */
	overshoot = offset;
	L_RSHIFTU(&overshoot);
	if (overshoot.l_ui != 0 || (overshoot.l_uf > ADJ_MAXOVERSHOOT)) {
		overshoot.l_ui = 0;
		overshoot.l_uf = ADJ_MAXOVERSHOOT;
	}
	L_ADD(&offset, &overshoot);
	TSTOTV(&offset, &adjtv);

	if (isneg) {
		adjtv.tv_sec = -adjtv.tv_sec;
		adjtv.tv_usec = -adjtv.tv_usec;
	}

	if (adjtv.tv_usec != 0 && !debug) {
		if (adjtime(&adjtv, &oadjtv) != 0) {
			msyslog(LOG_ERR, "Can't adjust the time of day: %m");
			return 0;
		}
	}
	return 1;
}


/*
 * msyslog - either send a message to the terminal or print it on
 *	     the standard output.
 *
 * I am truly ashamed of this.
 */
msyslog(level, fmt, p0, p1, p2, p3, p4)
	int level;
	char *fmt;
	char *p0, *p1, *p2, *p3, *p4;
{
	char buf[1025];
	register int c;
	register char *b, *f;
	extern int sys_nerr;
	extern char *sys_errlist[];
	int olderrno;
	FILE *outfp;

	if (syslogit) {
		syslog(level, fmt, p0, p1, p2, p3, p4);
		return;
	}

	olderrno = errno;
	if (level <= LOG_ERR)
		outfp = stderr;
	else
		outfp = stdout;

	b = buf;
	f = fmt;
	while ((c = *f++) != '\0' && c != '\n' && b < &buf[1024]) {
		if (c != '%') {
			*b++ = c;
			continue;
		}
		if ((c = *f++) != 'm') {
			*b++ = '%';
			*b++ = c;
			continue;
		}
		if ((unsigned)olderrno > sys_nerr)
			sprintf(b, "error %d", olderrno);
		else
			strcpy(b, sys_errlist[olderrno]);
		b += strlen(b);
	}
	*b++ = '\n';
	*b = '\0';
	(void) fprintf(outfp, "%s: ", progname);
	(void) fprintf(outfp, buf, p0, p1, p2, p3, p4);
}


/*
 * getnetnum - given a host name, return its net number
 */
int
getnetnum(host, num)
	char *host;
	u_int *num;
{
	struct hostent *hp;
	int decodenetnum();

	if (decodenetnum(host, num)) {
		return 1;
	} else if ((hp = gethostbyname(host)) != 0) {
		bcopy(hp->h_addr, (char *)num, sizeof(u_int));
		return 1;
	}
	return 0;
}

/*
 * decodenetnum - return a net number (this is crude, but careful)
 */
int
decodenetnum(num, netnum)
	char *num;
	u_int *netnum;
{
	register char *cp;
	register char *bp;
	register int i;
	register int temp;
	char buf[80];		/* will core dump on really stupid stuff */

	cp = num;
	*netnum = 0;
	for (i = 0; i < 4; i++) {
		bp = buf;
		while (isdigit(*cp))
			*bp++ = *cp++;
		if (bp == buf)
			break;

		if (i < 3) {
			if (*cp++ != '.')
				break;
		} else if (*cp != '\0')
			break;

		*bp = '\0';
		temp = atoi(buf);
		if (temp > 255)
			break;
		*netnum <<= 8;
		*netnum += temp;
	}
	
	if (i < 4)
		return 0;
	*netnum = htonl(*netnum);
	return 1;
}


/*
 * printserver - print detail information for a server
 */
void
printserver(pp, fp)
	register struct server *pp;
	FILE *fp;
{
	register int i;
	char junk[5];
	char *str;
	extern char *prettydate();
	extern char *ntoa();
	extern char *numtoa();
	extern char *ufptoa();
	extern char *lfptoa();

	(void) fprintf(fp, "server %s, port %d\n",
	    ntoa(&pp->srcadr), ntohs(pp->srcadr.sin_port));

	(void) fprintf(fp, "stratum %d, precision %d, leap %c%c, trust %03o\n",
	    pp->stratum, pp->precision,
	    pp->leap & 0x2 ? '1' : '0',
	    pp->leap & 0x1 ? '1' : '0',
	    pp->trust);
	
	if (pp->stratum == 1) {
		junk[4] = 0;
		bcopy((char *)&pp->refid, junk, 4);
		str = junk;
	} else {
		str = numtoa(pp->refid);
	}
	(void) fprintf(fp,
	    "refid [%s], distance %s, dispersion %s\n",
	    str, ufptoa(pp->distance, 4),
	    ufptoa(pp->dispersion, 4));
	
	(void) fprintf(fp, "transmitted %d, in filter %d\n",
	    pp->xmtcnt, pp->filter_nextpt);

	(void) fprintf(fp, "reference time:      %s\n",
	    prettydate(&pp->reftime));
	(void) fprintf(fp, "originate timestamp: %s\n",
	    prettydate(&pp->org));
	(void) fprintf(fp, "transmit timestamp:  %s\n",
	    prettydate(&pp->xmt));
	
	(void) fprintf(fp, "filter delay: ");
	for (i = 0; i < PEER_SHIFT; i++) {
		(void) fprintf(fp, " %-8.8s", ufptoa(pp->filter_delay[i],4));
		if (i == (PEER_SHIFT>>1)-1)
			(void) fprintf(fp, "\n              ");
	}
	(void) fprintf(fp, "\n");

	(void) fprintf(fp, "filter offset:");
	for (i = 0; i < PEER_SHIFT; i++) {
		(void) fprintf(fp, " %-8.8s", lfptoa(&pp->filter_offset[i], 5));
		if (i == (PEER_SHIFT>>1)-1)
			(void) fprintf(fp, "\n              ");
	}
	(void) fprintf(fp, "\n");

	(void) fprintf(fp, "estdelay %s, estdisp %s\n",
	    ufptoa(pp->estdelay, 4), ufptoa(pp->estdisp, 4));

	(void) fprintf(fp, "estoffset %s\n\n",
	    lfptoa(&pp->estoffset, 7));
}

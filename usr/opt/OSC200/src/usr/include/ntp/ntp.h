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
 *	@(#)$RCSfile: ntp.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:40:59 $
 */
/*
 */

/*
 * ntp.h - NTP definitions for the masses
 */

/*
 * How to get signed characters.  On machines where signed char works,
 * use it.  On machines where signed char doesn't work, char had better
 * be signed.
 */
/*
 * #if defined(NO_SIGNED_CHAR_DECL)
 * typedef char s_char;
 * #else
 * typedef signed char s_char;
 * #endif
 */

#define s_char char
/*
 * NTP protocol parameters.  See section 3.2.6 of the specification.
 */
#define	NTP_VERSION	2	/* current version number */
#define	NTP_OLDVERSION	1	/* previous version number */
#define	NTP_PORT	123	/* included for sake of non-unix machines */
#define	NTP_INFIN	15	/* max stratum, infinity a la Bellman-Ford */
#define	NTP_MAXAGE	86400	/* one day in seconds */
#define	NTP_MAXSKW	0x28f	/* 0.01 sec in fp format */
#define	NTP_MINDIST	0x51f	/* 0.02 sec in fp format */
#define	NTP_MINPOLL	6	/* actually 1<<6, or 64 sec */
#define	NTP_MAXPOLL	10	/* actually 1<<10, or 1024 sec */
#define	NTP_WINDOW	8	/* reachability register size */
#define	NTP_MAXWGT	(8*FP_SECOND)	/* maximum select weight 8 seconds */
#define	NTP_MAXLIST	5	/* maximum select list size */
#define	NTP_MAXSTRA	2	/* maximum number of stata in select list */
#define	NTP_SELECT	(3*(256/4))	/* select weight, see code */
#define	NTP_MAXKEY	65535	/* maximum authentication key number */

#define	PEER_SHIFT	8	/* 8 suitable for crystal time base */
#define	PEER_MAXDISP	(64*FP_SECOND)	/* maximum dispersion (fp 64) */
#define	PEER_THRESHOLD	(FP_SECOND>>1)	/* filter threshold (0.5 seconds) */
#define	PEER_FILTER	1	/* filter weight (actually 1>>1) */
#define	PEER_MAXDSPDEL	(FP_SECOND>>3)	/* maximum delay disp (0.125 seconds */


/*
 * Loop filter parameters.  See section 5.1 of the specification.
 *
 * Note that these are appropriate for a crystal time base.  If your
 * system clock is line frequency controlled you should read the
 * specification for appropriate modifications.  Note that the
 * loop filter code will have to change if you change CLOCK_MAX
 * to be greater than or equal to 500 ms.
 */
#define	CLOCK_UPDATE	8	/* update interval: 64 seconds */
#define	CLOCK_ADJ	2	/* adjustment interval: 4 seconds */
#define	CLOCK_FREQ	10	/* frequency weight: 2**10 */
#define	CLOCK_PHASE	8	/* phase weight: 2**8 */
#define	CLOCK_TRACK	8	/* compliance weight: 2**8 */
#define	CLOCK_COMP	4	/* compliance maximum: 2**4 */
#define	CLOCK_FACTOR	18	/* compliance factor: 2**18 */

#define	CLOCK_MAX_F	0x20c49ba6	/* 128 ms, in time stamp format */
#define	CLOCK_MAX_I	0x0		/* both fractional and integral parts */

#define	CLOCK_WAYTOOBIG	1000		/* if clock 1000 sec off, forget it */

/*
 * Unspecified default.  sys.precision defaults to -6 unless otherwise
 * adjusted.
 */
#define	DEFAULT_SYS_PRECISION	(-6)


/*
 * Event timers are actually implemented as a sorted queue of expiry
 * times.  The queue is slotted, with each slot holding timers which
 * expire in a 2**(NTP_MINPOLL-1) (32) second period.  The timers in
 * each slot are sorted by increasing expiry time.  The number of
 * slots is 2**(NTP_MAXPOLL-(NTP_MINPOLL-1)), or 32, to cover a time
 * period of 2**NTP_MAXPOLL (1024) seconds into the future before
 * wrapping.
 */
#define	EVENT_TIMEOUT	CLOCK_ADJ

struct event {
	struct event *next;		/* next in chain */
	struct event *prev;		/* previous in chain */
	struct peer *peer;		/* peer this counter belongs to */
	void (*event_handler)();	/* routine to call to handle event */
	u_int event_time;		/* expiry time of counter */
};

#define	TIMER_SLOTTIME	(1<<(NTP_MINPOLL-1))
#define	TIMER_NSLOTS	(1<<(NTP_MAXPOLL-(NTP_MINPOLL-1)))
#define	TIMER_SLOT(t)	(((t) >> (NTP_MINPOLL-1)) & (TIMER_NSLOTS-1))

/*
 * TIMER_ENQUEUE() puts stuff on the timer queue.  It takes as
 * arguments (ea), an array of event slots, and (iev), the event
 * to be inserted.  This one searches the hash bucket from the
 * end, and is about optimum for the timing requirements of
 * NTP peers.
 */
#define	TIMER_ENQUEUE(ea, iev) \
	do { \
		register struct event *ev; \
		\
		ev = (ea)[TIMER_SLOT((iev)->event_time)].prev; \
		while (ev->event_time > (iev)->event_time) \
			ev = ev->prev; \
		(iev)->prev = ev; \
		(iev)->next = ev->next; \
		(ev)->next->prev = (iev); \
		(ev)->next = (iev); \
	} while(0)

/*
 * TIMER_INSERT() also puts stuff on the timer queue, but searches the
 * bucket from the top.  This is better for things that do very short
 * time outs, like clock support.
 */
#define	TIMER_INSERT(ea, iev) \
	do { \
		register struct event *ev; \
		\
		ev = (ea)[TIMER_SLOT((iev)->event_time)].next; \
		while (ev->event_time != 0 && \
		    ev->event_time < (iev)->event_time) \
			ev = ev->next; \
		(iev)->next = ev; \
		(iev)->prev = ev->prev; \
		(ev)->prev->next = (iev); \
		(ev)->prev = (iev); \
	} while(0)

/*
 * Remove an event from the queue.
 */
#define	TIMER_DEQUEUE(ev) \
	do { \
		if ((ev)->next != 0) { \
			(ev)->next->prev = (ev)->prev; \
			(ev)->prev->next = (ev)->next; \
			(ev)->next = (ev)->prev = 0; \
		} \
	} while (0)

/*
 * The interface structure is used to hold the addresses and socket
 * numbers of each of the interfaces we are using.
 */
struct interface {
	int fd;			/* socket this is opened on */
	int bfd;		/* socket for receiving broadcasts */
	struct sockaddr_in sin;	/* interface address */
	struct sockaddr_in bcast;	/* broadcast address */
	struct sockaddr_in mask;	/* interface mask */
	char name[8];		/* name of interface */
	int flags;		/* interface flags */
	int received;		/* number of incoming packets */
	int sent;		/* number of outgoing packets */
	int notsent;		/* number of send failures */
};

/*
 * Flags for interfaces
 */
#define	INT_BROADCAST	1	/* can broadcast out this interface */
#define	INT_BCASTOPEN	2	/* broadcast socket is open */
#define	INT_LOOPBACK	4	/* the loopback interface */


/*
 * The peer structure.  Holds state information relating to the guys
 * we are peering with.  Most of this stuff is from section 3.2 of the
 * spec.
 */
struct peer {
	struct peer *next;
	struct peer *ass_next;		/* link pointer in associd hash */
	struct sockaddr_in srcadr;	/* address of remote host */
	struct interface *dstadr;	/* pointer to address on local host */
	u_char leap;			/* leap indicator */
	u_char hmode;			/* association mode with this peer */
	u_char pmode;			/* peer's association mode */
	u_char stratum;			/* stratum of remote peer */
	u_char ppoll;			/* peer polling interval */
	s_char precision;		/* peer's clock precision */
	u_char hpoll;			/* local host's poll interval */
	u_char version;			/* version indicator (XXX delete) */
	u_char reach;			/* reachability, NTP_WINDOW bits */
	u_char flags;			/* peer flags */
	u_char refclktype;		/* reference clock type */
	u_char refclkunit;		/* reference clock unit number */
	u_fp distance;			/* distance from primary clock */
	u_fp dispersion;		/* peer clock dispersion */
	u_int refid;			/* peer reference ID */
	l_fp reftime;			/* time of peer's last update */
	struct event event_timer;	/* event queue entry */
	u_int keyid;			/* encription key ID */
	u_int pkeyid;			/* keyid used to encrypt last message */
	u_short associd;		/* association ID, a unique integer */
	u_char unused;
/* **Start of clear-to-zero area.*** */
/* Everything that is cleared to zero goes below here */
	u_char valid;			/* valid counter */
#define	clear_to_zero	valid
	u_char trust;			/* trust, PEER_SHIFT bits */
	u_char unreach;			/* unreachable count */
	u_short filter_nextpt;		/* index into filter shift register */
	u_fp filter_delay[PEER_SHIFT];	/* delay part of shift register */
	l_fp filter_offset[PEER_SHIFT];	/* offset part of shift register */
	s_fp filter_soffset[PEER_SHIFT]; /* offset in s_fp format, for disp */
	l_fp org;			/* originate time stamp */
	l_fp rec;			/* receive time stamp */
	l_fp xmt;			/* transmit time stamp */
/* ***End of clear-to-zero area.*** */
/* Everything that is cleared to zero goes above here */
	u_char filter_order[PEER_SHIFT]; /* we keep the filter sorted here */
#define	end_clear_to_zero	filter_order[0]
	u_fp estdelay;			/* filter estimated delay */
	u_fp estdisp;			/* filter estimated dispersion */
	l_fp estoffset;			/* filter estimated clock offset */
	s_fp estsoffset;		/* fp version of above */

	/*
	 * Stuff related to the experimental broadcast delay
	 * determination code.  The registers will probably go away
	 * later.
	 */
	u_int estbdelay;		/* broadcast delay, as a ts fraction */
	u_int filter_bdelay[PEER_SHIFT];	/* broadcast delay registers */
	u_short bdel_next;		/* where the next sample goes */
	u_short bdel_ticks;		/* countdown to next transmission */
	u_int bdel_time;		/* time to start next poll */

	/*
	 * statistic counters
	 */
	u_int timereset;		/* time stat counters were reset */
	u_int sent;			/* number of updates sent */
	u_int received;		/* number of frames received */
	u_int timereceived;		/* last time a frame received */
	u_int timereachable;		/* last reachable/unreachable event */
	u_int badlength;		/* number of funny length packets */
	u_int processed;		/* processed by the protocol */
	u_int badauth;			/* bad credentials detected */
	u_int bogusorg;		/* rejected due to bogus origin */
	u_int oldpkt;			/* rejected as duplicate packet */
	u_int baddelay;		/* reject due to bad delay */
	u_int seldelaytoolarge;	/* too long a delay for selection */
	u_int seldisptoolarge;		/* too much dispersion for selection */
	u_int selbroken;		/* broken NTP detected in selection */
	u_int seltooold;		/* too long since sync in selection */
	u_int untrustable;		/* not selected because not trustable */
	u_char candidate;		/* position after candidate selection */
	u_char falseticker;		/* position before falseticker sel */
	u_char select;			/* position at end of falseticker sel */
	u_char select_total;		/* number of peers in selection */
	u_char was_sane;		/* set to 1 if it passed sanity check */
	u_char last_event;		/* set to code for last peer error */
	u_char num_events;		/* num. of events which have occurred */
};

/*
 * Values for peer.leap, sys_leap
 */
#define	LEAP_NOWARNING	0x0	/* normal, no leap second warning */
#define	LEAP_ADDSECOND	0x1	/* last minute of day has 61 seconds */
#define	LEAP_DELSECOND	0x2	/* last minute of day has 59 seconds */
#define	LEAP_NOTINSYNC	0x3	/* overload, clock is free running */

/*
 * Values for peer.mode
 */
#define	MODE_UNSPEC	0	/* unspecified (probably old NTP version) */
#define	MODE_ACTIVE	1	/* symmetric active */
#define	MODE_PASSIVE	2	/* symmetric passive */
#define	MODE_CLIENT	3	/* client mode */
#define	MODE_SERVER	4	/* server mode */
#define	MODE_BROADCAST	5	/* broadcast mode */
#define	MODE_CONTROL	6	/* control mode packet */
#define	MODE_PRIVATE	7	/* implementation defined function */

#define	MODE_BCLIENT	8	/* a pseudo mode, used internally */


/*
 * Values for peer.stratum, sys_stratum
 */
#define	STRATUM_REFCLOCK	0	/* stratum claimed by primary clock */
#define	STRATUM_PRIMARY	1		/* host has a primary clock */
#define	STRATUM_INFIN	NTP_INFIN	/* infinity a la Bellman-Ford */
/* A stratum of 0 in the packet is mapped to 16 internally */
#define	STRATUM_PKT_UNSPEC	0	/* unspecified in packet */
#define	STRATUM_UNSPEC	(NTP_INFIN+1)	/* unspecified */

/*
 * Values for peer.flags
 */
#define	FLAG_CONFIG		0x1	/* association was configured */
#define	FLAG_AUTHENABLE		0x2	/* this guy needs authentication */
#define	FLAG_MINPOLL		0x4	/* keep polling interval minimized */
#define	FLAG_DEFBDELAY		0x8	/* using default bdelay */
#define	FLAG_AUTHENTIC		0x10	/* last message was authentic */
#define	FLAG_REFCLOCK		0x20	/* this is actually a reference clock */

/*
 * Definitions for the clear() routine.  We use bzero() to clear
 * the parts of the peer structure which go to zero.  These are
 * used to calculate the start address and length of the area.
 */
#define	CLEAR_TO_ZERO(p)	((char *)&((p)->clear_to_zero))
#define	END_CLEAR_TO_ZERO(p)	((char *)&((p)->end_clear_to_zero))
#define	LEN_CLEAR_TO_ZERO	(END_CLEAR_TO_ZERO((struct peer *)0) \
				    - CLEAR_TO_ZERO((struct peer *)0))

/*
 * Reference clock types.  Added as necessary.
 */
#define	REFCLK_NONE		0
#define	REFCLK_LOCALCLOCK	1
#define	REFCLK_WWV_HEATH	2
#define	REFCLK_WWV_PST		3
#define	REFCLK_WWVB_SPECTRACOM	4
#define	REFCLK_GOES_TRUETIME	5
#define	REFCLK_GOES_TRAK	6
#define	REFCLK_CHU		7

/*
 * We tell reference clocks from real peers by giving the reference
 * clocks an address of the form 127.127.t.u, where t is the type and
 * u is the unit number.  We define some of this here since we will need
 * some sanity checks to make sure this address isn't interpretted as
 * that of a normal peer.
 */
#define	REFCLOCK_ADDR	0x7f7f0000	/* 127.127.0.0 */
#define	REFCLOCK_MASK	0xffff0000	/* 255.255.0.0 */

#define	ISREFCLOCKADR(srcadr)	((SRCADR(srcadr) & REFCLOCK_MASK) \
					== REFCLOCK_ADDR)

/*
 * Macro for checking for invalid addresses.  This is really, really
 * gross, but is needed so no one configures a host on net 127 now that
 * we're encouraging it the the configuration file.
 */
#define	LOOPBACKADR	0x7f000001
#define	LOOPNETMASK	0xff000000

#define	ISBADADR(srcadr)	(((SRCADR(srcadr) & LOOPNETMASK) \
				    == (LOOPBACKADR & LOOPNETMASK)) \
				    && (SRCADR(srcadr) != LOOPBACKADR))

/*
 * Utilities for manipulating addresses and port numbers
 */
#define	NSRCADR(src)	((src)->sin_addr.s_addr) /* address in net byte order */
#define	NSRCPORT(src)	((src)->sin_port)	/* port in net byte order */
#define	SRCADR(src)	(ntohl(NSRCADR((src))))	/* address in host byte order */
#define	SRCPORT(src)	(ntohs(NSRCPORT((src))))	/* host port */

/*
 * NTP packet format.  The mac field is optional.  It isn't really
 * an l_fp either, but for now declaring it that way is convenient.
 * See Appendix A in the specification.
 *
 * Note that all u_fp and l_fp values arrive in network byte order
 * and must be converted (except the mac, which isn't, really).
 */
struct pkt {
	u_char li_vn_mode;	/* contains leap indicator, version and mode */
	u_char stratum;		/* peer's stratum */
	u_char ppoll;		/* the peer polling interval */
	s_char precision;	/* peer clock precision */
	u_fp distance;		/* distance to primary clock */
	u_fp dispersion;	/* clock dispersion */
	u_int refid;		/* reference clock ID */
	l_fp reftime;		/* time peer clock was last updated */
	l_fp org;		/* originate time stamp */
	l_fp rec;		/* receive time stamp */
	l_fp xmt;		/* transmit time stamp */
	u_int keyid;		/* key identification */
	l_fp mac;		/* message-authentication code */
};

/*
 * Packets can come in two flavours, one with a mac and one without.
 * These are their lengths.
 */
#define	MAC_LEN		(sizeof(l_fp) + sizeof(u_int))
#define	LEN_PKT_MAC	sizeof(struct pkt)
#define	LEN_PKT_NOMAC	(sizeof(struct pkt) - MAC_LEN)

/*
 * Stuff for extracting things from li_vn_mode
 */
#define	PKT_MODE(li_vn_mode)	((li_vn_mode) & 0x7)
#define	PKT_VERSION(li_vn_mode)	(((li_vn_mode) >> 3) & 0x7)
#define	PKT_LEAP(li_vn_mode)	(((li_vn_mode) >> 6) & 0x3)

/*
 * Stuff for putting things back into li_vn_mode
 */
#define	PKT_LI_VN_MODE(li, vn, md) \
	((u_char)((((li) << 6) & 0xc0) | (((vn) << 3) & 0x38) | ((md) & 0x7)))


/*
 * Dealing with stratum.  0 gets mapped to 16 incoming, and back to 0
 * on output.
 */
#define	PKT_TO_STRATUM(s)	(((s) == STRATUM_PKT_UNSPEC) ?\
				 (STRATUM_UNSPEC) : (s))

#define	STRATUM_TO_PKT(s)	(((s) == (STRATUM_UNSPEC)) ?\
				(STRATUM_PKT_UNSPEC) : (s))

/*
 * Format of a recvbuf.  These are used by the asynchronous receive
 * routine to store incoming packets and related information.
 */
#define	RX_BUFF_SIZE	(120)
struct recvbuf {
	struct recvbuf *next;		/* next buffer in chain */
	union {
		struct sockaddr_in X_recv_srcadr;
		caddr_t X_recv_srcclock;
	} X_from_where;
#define recv_srcadr	X_from_where.X_recv_srcadr
#define	recv_srcclock	X_from_where.X_recv_srcclock
	struct sockaddr_in srcadr;	/* where packet came from */
	struct interface *dstadr;	/* interface datagram arrived thru */
	l_fp recv_time;			/* time of arrival */
	void (*receiver)();		/* routine to receive buffer */
	int recv_length;		/* number of octets received */
	union {
		struct pkt X_recv_pkt;
		char X_recv_buffer[RX_BUFF_SIZE];
	} recv_space;
#define	recv_pkt	recv_space.X_recv_pkt
#define	recv_buffer	recv_space.X_recv_buffer
};


/*
 * Event codes.  Used for reporting errors/events to the control module
 */
#define	PEER_EVENT	0x80		/* this is a peer event */

#define	EVNT_UNSPEC	0
#define	EVNT_SYSRESTART	1
#define	EVNT_SYSFAULT	2
#define	EVNT_SYNCCHG	3
#define	EVNT_PEERSTCHG	4
#define	EVNT_CLOCKRESET	5
#define	EVNT_BADDATETIM	6
#define	EVNT_CLOCKEXCPT	7

#define	EVNT_PEERIPERR	(1|PEER_EVENT)
#define	EVNT_PEERAUTH	(2|PEER_EVENT)
#define	EVNT_UNREACH	(3|PEER_EVENT)
#define	EVNT_REACH	(4|PEER_EVENT)
#define	EVNT_PEERCLOCK	(5|PEER_EVENT)
#define	EVNT_PEERSTRAT	(6|PEER_EVENT)

/*
 * Clock event codes
 */
#define	CEVNT_NOMINAL	0
#define	CEVNT_TIMEOUT	1
#define	CEVNT_BADREPLY	2
#define	CEVNT_FAULT	3
#define	CEVNT_PROP	4
#define	CEVNT_BADDATE	5
#define	CEVNT_BADTIME	6

/*
 * Very misplaced value.  Default port through which we send traps.
 */
#define	TRAPPORT	18447


/*
 * To speed lookups, peers are hashed by the low order bits of the remote
 * IP address.  These definitions relate to that.
 */
#define	HASH_SIZE	32
#define	HASH_MASK	(HASH_SIZE-1)
#define	HASH_ADDR(src)	((SRCADR((src))^(SRCADR((src))>>8)) & HASH_MASK)


/*
 * The poll update procedure takes an extra argument which controls
 * how a random perturbation is applied to peer.timer.  The choice is
 * to not randomize at all, to randomize only if we're going to update
 * peer.timer, and to randomize no matter what (almost, the algorithm
 * is that we apply the random value if it is less than the current
 * timer count).
 */
#define	POLL_NOTRANDOM		0	/* don't randomize */
#define	POLL_RANDOMCHANGE	1	/* if you change, change randomly */
#define	POLL_MAKERANDOM		2	/* randomize next interval */


/*
 * How we randomize polls.  The poll interval is a power of two.
 * We chose a random value which is between 1/4 and 3/4 of the
 * poll interval we would normally use and which is an even multiple
 * of the EVENT_TIMEOUT.  The random number routine, given an argument
 * spread value of n, returns an integer between 0 and (1<<n)-1.  This
 * is shifted by EVENT_TIMEOUT and added to the base value.
 */
#define	RANDOM_SPREAD(poll)	((poll) - (EVENT_TIMEOUT+1))
#define	RANDOM_POLL(poll, rval)	((((rval)+1)<<EVENT_TIMEOUT) + (1<<((poll)-2)))

/*
 * min, min3 and max.  Makes it easier to transliterate the spec without
 * thinking about it.
 */
#define	min(a,b)	(((a) < (b)) ? (a) : (b))
#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#define	min3(a,b,c)	min(min((a),(b)), (c))


/*
 * Configuration items.  These are for the protocol module (proto_config())
 */
#define	PROTO_BROADCLIENT	1
#define	PROTO_PRECISION		2
#define	PROTO_AUTHENTICATE	3
#define	PROTO_BROADDELAY	4
#define	PROTO_AUTHDELAY		5
#define	PROTO_MAXSKEW		6
#define	PROTO_SELECT		7

/*
 * Configuration items for the loop filter
 */
#define	LOOP_DRIFTCOMP		1

/*
 * Configuration items for the stats printer
 */
#define	STATS_FREQ_FILE		1

/*
 * Default parameters.  We use these in the absense of something better.
 */
#define	DEFPRECISION	(-5)		/* conservatively low */
#define	DEFBROADDELAY	(0x020c49ba)	/* 8 ms.  This is round trip delay */


/*
 * Structure used optionally for monitoring when this is turned on.
 */
struct mon_data {
	struct mon_data *hash_next;	/* next structure in hash list */
	struct mon_data *hash_prev;	/* previous structure in hash list */
	struct mon_data *mru_next;	/* next structure in MRU list */
	struct mon_data *mru_prev;	/* previous structure in MRU list */
	u_int lasttime;		/* last time data updated */
	u_int firsttime;		/* time structure initialized */
	u_int count;			/* count we have seen */
	u_int rmtadr;			/* address of remote host */
	u_short rmtport;		/* remote port last came from */
	u_char mode;			/* mode of incoming packet */
	u_char version;			/* version of incoming packet */
};


/*
 * Structure used for restrictlist entries
 */
struct restrictlist {
	struct restrictlist *next;	/* link to next entry */
	u_int addr;			/* host address (host byte order) */
	u_int mask;			/* mask for address (host byte order) */
	u_int count;			/* number of packets matched */
	u_short flags;			/* accesslist flags */
	u_short mflags;			/* match flags */
};

/*
 * Access flags
 */
#define	RES_IGNORE		0x1	/* ignore if matched */
#define	RES_DONTSERVE		0x2	/* don't give him any time */
#define	RES_DONTTRUST		0x4	/* don't trust if matched */
#define	RES_NOQUERY		0x8	/* don't allow queries if matched */
#define	RES_NOMODIFY		0x10	/* don't allow him to modify server */
#define	RES_NOPEER		0x20	/* don't allocate memory resources */
#define	RES_NOTRAP		0x40	/* don't allow him to set traps */
#define	RES_LPTRAP		0x80	/* traps set by him are low priority */

#define	RES_ALLFLAGS \
    (RES_IGNORE|RES_DONTSERVE|RES_DONTTRUST|RES_NOQUERY\
    |RES_NOMODIFY|RES_NOPEER|RES_NOTRAP|RES_LPTRAP)

/*
 * Match flags
 */
#define	RESM_INTERFACE		0x1	/* this is an interface */
#define	RESM_NTPONLY		0x2	/* match ntp port only */

/*
 * Restriction configuration ops
 */
#define	RESTRICT_FLAGS		1	/* add flags to restrict entry */
#define	RESTRICT_UNFLAG		2	/* remove flags from restrict entry */
#define	RESTRICT_REMOVE		3	/* remove a restrict entry */


/*
 * Experimental alternate selection algorithm identifiers
 */
#define	SELECT_1	1
#define	SELECT_2	2
#define	SELECT_3	3
#define	SELECT_4	4
#define	SELECT_5	5

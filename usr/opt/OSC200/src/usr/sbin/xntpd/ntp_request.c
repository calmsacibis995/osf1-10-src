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
static char     *sccsid = "@(#)$RCSfile: ntp_request.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 16:58:09 $";
#endif
/*
 */

/*
 * ntp_request.c - respond to information requests
 */
#include <stdio.h>
#include <strings.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#ifdef convex
#include "/sys/sync/queue.h"
#include "/sys/sync/sema.h"
#endif
#include <net/if.h>
#include <netinet/in.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>
#include <ntp/ntp_request.h>
#include <ntp/ntp_control.h>
#include <ntp/ntp_refclock.h>

/*
 * Structure to hold request procedure information
 */
#define	NOAUTH	0
#define	AUTH	1

#define	NO_REQUEST	(-1)

struct req_proc {
	short request_code;	/* defined request code */
	short needs_auth;	/* true when authentication needed */
	short sizeofitem;	/* size of request data item */
	void (*handler)();	/* routine to handle request */
};

/*
 * Universal request codes
 */
struct req_proc univ_codes[] = {
	{ NO_REQUEST,		NOAUTH,	 0,	0 }
};

/*
 * Xntpd request codes
 */
void peer_list(), peer_list_sum();
void peer_info(), peer_stats();
void sys_info(), sys_stats(), io_stats();
void mem_stats(), loop_info(), timer_stats();
void do_conf(), do_unconf();
void set_sys_flag(), clr_sys_flag();
void do_monitor(), do_nomonitor();
void list_restrict(), do_unrestrict();
void do_resaddflags(), do_ressubflags();
void mon_getlist();
void reset_stats(), reset_peer();
void do_key_reread();
void do_dirty_hack(), dont_dirty_hack();
void trust_key(), untrust_key();
void get_auth_info();
void req_get_traps(), req_set_trap(), req_clr_trap();
void set_request_keyid(), set_control_keyid();
void get_ctl_stats(), get_leap_info();
#ifdef REFCLOCK
void get_clock_info(), set_clock_fudge(), get_clkbug_info();
#endif
void set_maxskew(), set_precision(), set_select_code();

struct req_proc xntp_codes[] = {
	{ REQ_PEER_LIST,	NOAUTH,	0,	peer_list },
	{ REQ_PEER_LIST_SUM,	NOAUTH,	0,	peer_list_sum },
	{ REQ_PEER_INFO,    NOAUTH, sizeof(struct info_peer_list), peer_info },
	{ REQ_PEER_STATS,   NOAUTH, sizeof(struct info_peer_list), peer_stats },
	{ REQ_SYS_INFO,		NOAUTH,	0,	sys_info },
	{ REQ_SYS_STATS,	NOAUTH,	0,	sys_stats },
	{ REQ_IO_STATS,		NOAUTH,	0,	io_stats },
	{ REQ_MEM_STATS,	NOAUTH,	0,	mem_stats },
	{ REQ_LOOP_INFO,	NOAUTH,	0,	loop_info },
	{ REQ_TIMER_STATS,	NOAUTH,	0,	timer_stats },
	{ REQ_CONFIG,	    AUTH, sizeof(struct conf_peer), do_conf },
	{ REQ_UNCONFIG,	    AUTH, sizeof(struct conf_unpeer), do_unconf },
	{ REQ_SET_SYS_FLAG, AUTH, sizeof(struct conf_sys_flags), set_sys_flag },
	{ REQ_CLR_SYS_FLAG, AUTH, sizeof(struct conf_sys_flags), clr_sys_flag },
	{ REQ_MONITOR,		AUTH,	0,	do_monitor },
	{ REQ_NOMONITOR,	AUTH,	0,	do_nomonitor },
	{ REQ_GET_RESTRICT,	NOAUTH,	0,	list_restrict },
	{ REQ_RESADDFLAGS, AUTH, sizeof(struct conf_restrict), do_resaddflags },
	{ REQ_RESSUBFLAGS, AUTH, sizeof(struct conf_restrict), do_ressubflags },
	{ REQ_UNRESTRICT,  AUTH, sizeof(struct conf_restrict), do_unrestrict },
	{ REQ_MON_GETLIST,	NOAUTH,	0,	mon_getlist },
	{ REQ_RESET_STATS, AUTH, sizeof(struct reset_flags), reset_stats },
	{ REQ_RESET_PEER,  AUTH, sizeof(struct conf_unpeer), reset_peer },
	{ REQ_REREAD_KEYS,	AUTH,	0,	do_key_reread },
	{ REQ_DO_DIRTY_HACK,	AUTH,	0,	do_dirty_hack },
	{ REQ_DONT_DIRTY_HACK,	AUTH,	0,	dont_dirty_hack },
	{ REQ_TRUSTKEY,    AUTH, sizeof(u_int),	trust_key },
	{ REQ_UNTRUSTKEY,  AUTH, sizeof(u_int),	untrust_key },
	{ REQ_AUTHINFO,		NOAUTH,	0,	get_auth_info },
	{ REQ_TRAPS,		NOAUTH, 0,	req_get_traps },
	{ REQ_ADD_TRAP,	   AUTH, sizeof(struct conf_trap), req_set_trap },
	{ REQ_CLR_TRAP,	   AUTH, sizeof(struct conf_trap), req_clr_trap },
	{ REQ_REQUEST_KEY, AUTH, sizeof(u_int),	set_request_keyid },
	{ REQ_CONTROL_KEY, AUTH, sizeof(u_int),	set_control_keyid },
	{ REQ_GET_CTLSTATS,	NOAUTH,	0,	get_ctl_stats },
	{ REQ_GET_LEAPINFO,	NOAUTH,	0,	get_leap_info },
	{ REQ_SET_MAXSKEW, AUTH, sizeof(u_fp),	set_maxskew },
	{ REQ_SET_PRECISION, AUTH, sizeof(int),	set_precision },
	{ REQ_SET_SELECT_CODE, AUTH, sizeof(u_int),	set_select_code },
#ifdef REFCLOCK
	{ REQ_GET_CLOCKINFO, NOAUTH, sizeof(u_int),	get_clock_info },
	{ REQ_SET_CLKFUDGE, AUTH, sizeof(struct conf_fudge), set_clock_fudge },
	{ REQ_GET_CLKBUGINFO, NOAUTH, sizeof(u_int),	get_clkbug_info },
#endif
	{ NO_REQUEST,		NOAUTH,	0,	0 }
};


/*
 * Authentication keyid used to authenticate requests.  Zero means we
 * don't allow writing anything.
 */
u_int info_auth_keyid;


/*
 * Statistic counters to keep track of requests and responses.
 */
u_int numrequests;		/* number of requests we've received */
u_int numresppkts;		/* number of resp packets sent with data */

u_int errorcounter[INFO_ERR_AUTH+1];	/* lazy way to count errors, indexed */
					/* by the error code */

/*
 * Imported from the I/O module
 */
extern struct interface *any_interface;

/*
 * Imported from the main routines
 */
extern int debug;

/*
 * Imported from the timer module
 */
extern u_int current_time;

/*
 * A hack.  To keep the authentication module clear of xntp-ism's, we
 * include a time reset variable for its stats here.
 */
static u_int auth_timereset;

/*
 * Response packet used by these routines.  Also some state information
 * so that we can handle packet formatting within a common set of
 * subroutines.  Note we try to enter data in place whenever possible,
 * but the need to set the more bit correctly means we occasionally
 * use the extra buffer and copy.
 */
static struct resp_pkt rpkt;
static int seqno;
static int nitems;
static int itemsize;
static int databytes;
static char exbuf[RESP_DATA_SIZE];
static int usingexbuf;
static struct sockaddr_in *toaddr;
static struct interface *frominter;

/*
 * init_request - initialize request data
 */
void
init_request()
{
	int i;

	numrequests = 0;
	numresppkts = 0;
	auth_timereset = 0;
	info_auth_keyid = 0;	/* by default, can't do this */

	for (i = 0; i < sizeof(errorcounter)/sizeof(errorcounter[0]); i++)
		errorcounter[i] = 0;
}


/*
 * req_ack - acknowledge request with no data
 */
void
req_ack(srcadr, inter, inpkt, errcode)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
	int errcode;
{
	/*
	 * fill in the fields
	 */
	rpkt.rm_vn_mode = RM_VN_MODE(RESP_BIT, 0);
	rpkt.auth_seq = AUTH_SEQ(0, 0);
	rpkt.implementation = inpkt->implementation;
	rpkt.request = inpkt->request;
	rpkt.err_nitems = ERR_NITEMS(errcode, 0);
	rpkt.mbz_itemsize = MBZ_ITEMSIZE(0);

	/*
	 * send packet and bump counters
	 */
	sendpkt(srcadr, inter, (struct pkt *)&rpkt, RESP_HEADER_SIZE);
	errorcounter[errcode]++;
}


/*
 * prepare_pkt - prepare response packet for transmission, return pointer
 *		 to storage for data item.
 */
char *
prepare_pkt(srcadr, inter, pkt, structsize)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *pkt;
	u_int structsize;
{
#ifdef DEBUG
	if (debug > 3)
		printf("request: preparing pkt\n");
#endif

	/*
	 * Fill in the implementation, reqest and itemsize fields
	 * since these won't change.
	 */
	rpkt.implementation = pkt->implementation;
	rpkt.request = pkt->request;
	rpkt.mbz_itemsize = MBZ_ITEMSIZE(structsize);

	/*
	 * Compute the static data needed to carry on.
	 */
	toaddr = srcadr;
	frominter = inter;
	seqno = 0;
	nitems = 0;
	itemsize = structsize;
	databytes = 0;
	usingexbuf = 0;

	/*
	 * return the beginning of the packet buffer.
	 */
	return &rpkt.data[0];
}


/*
 * more_pkt - return a data pointer for a new item.
 */
char *
more_pkt()
{
	/*
	 * If we were using the extra buffer, send the packet.
	 */
	if (usingexbuf) {
#ifdef DEBUG
		if (debug > 2)
			printf("request: sending pkt\n");
#endif
		rpkt.rm_vn_mode = RM_VN_MODE(RESP_BIT, MORE_BIT);
		rpkt.auth_seq = AUTH_SEQ(0, seqno);
		rpkt.err_nitems = htons((u_short)nitems);
		sendpkt(toaddr, frominter, (struct pkt *)&rpkt,
		    RESP_HEADER_SIZE+databytes);
		numresppkts++;

		/*
		 * Copy data out of exbuf into the packet.
		 */
		bcopy(exbuf, &rpkt.data[0], itemsize);
		seqno++;
		databytes = 0;
		nitems = 0;
		usingexbuf = 0;
	}

	databytes += itemsize;
	nitems++;
	if (databytes + itemsize <= RESP_DATA_SIZE) {
#ifdef DEBUG
		if (debug > 3)
			printf("request: giving him more data\n");
#endif
		/*
		 * More room in packet.  Give him the
		 * next address.
		 */
		return &rpkt.data[databytes];
	} else {
		/*
		 * No room in packet.  Give him the extra
		 * buffer unless this was the last in the sequence.
		 */
#ifdef DEBUG
		if (debug > 3)
			printf("request: into extra buffer\n");
#endif
		if (seqno == MAXSEQ)
			return (char *)0;
		else {
			usingexbuf = 1;
			return exbuf;
		}
	}
}


/*
 * flush_pkt - we're done, return remaining information.
 */
void
flush_pkt()
{
#ifdef DEBUG
	if (debug > 2)
		printf("request: flushing packet, %d items\n", nitems);
#endif
	/*
	 * Must send the last packet.  If nothing in here and nothing
	 * has been sent, send an error saying no data to be found.
	 */
	if (seqno == 0 && nitems == 0)
		req_ack(toaddr, frominter, (struct req_pkt *)&rpkt,
		    INFO_ERR_NODATA);
	else {
		rpkt.rm_vn_mode = RM_VN_MODE(RESP_BIT, 0);
		rpkt.auth_seq = AUTH_SEQ(0, seqno);
		rpkt.err_nitems = htons((u_short)nitems);
		sendpkt(toaddr, frominter, (struct pkt *)&rpkt,
		    RESP_HEADER_SIZE+databytes);
		numresppkts++;
	}
}



/*
 * process_private - process private mode (7) packets
 */
void
process_private(rbufp, mod_okay)
	struct recvbuf *rbufp;
	int mod_okay;
{
	struct req_pkt *inpkt;
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_proc *proc;

	/*
	 * Initialize pointers, for convenience
	 */
	inpkt = (struct req_pkt *)&rbufp->recv_pkt;
	srcadr = &rbufp->recv_srcadr;
	inter = rbufp->dstadr;

#ifdef DEBUG
	if (debug > 2)
		printf("prepare_pkt: impl %d req %d\n",
		    inpkt->implementation, inpkt->request);
#endif

	/*
	 * Do some sanity checks on the packet.  Return a format
	 * error if it fails.
	 */
	if (ISRESPONSE(inpkt->rm_vn_mode)
	    || ISMORE(inpkt->rm_vn_mode)
	    || INFO_VERSION(inpkt->rm_vn_mode) != NTP_VERSION
	    || INFO_SEQ(inpkt->auth_seq) != 0
	    || INFO_ERR(inpkt->err_nitems) != 0
	    || INFO_MBZ(inpkt->mbz_itemsize) != 0
	    || (rbufp->recv_length != REQ_LEN_MAC
	     && rbufp->recv_length != REQ_LEN_NOMAC)) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}


	/*
	 * Get the appropriate procedure list to search.
	 */
	if (inpkt->implementation == IMPL_UNIV)
		proc = univ_codes;
	else if (inpkt->implementation == IMPL_XNTPD)
		proc = xntp_codes;
	else {
		req_ack(srcadr, inter, inpkt, INFO_ERR_IMPL);
		return;
	}


	/*
	 * Search the list for the request codes.  If it isn't one
	 * we know, return an error.
	 */
	while (proc->request_code != NO_REQUEST) {
		if (proc->request_code == (short) inpkt->request)
			break;
		proc++;
	}
	if (proc->request_code == NO_REQUEST) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_REQ);
		return;
	}

#ifdef DEBUG
	if (debug > 3)
		printf("found request in tables\n");
#endif

	/*
	 * If we need to authenticate, do so.  Note that an
	 * authenticatable packet must include a mac field, must
	 * have used key info_auth_keyid and must have included
	 * a time stamp in the appropriate field.  The time stamp
	 * must be within INFO_TS_MAXSKEW of the receive
	 * time stamp.
	 */
	if (proc->needs_auth) {
#ifdef DES_OK
		register u_int tmp_ui;
		register u_int tmp_uf;
		
		/*
		 * If this guy is restricted from doing this, don't let him
		 * If wrong key was used, or packet doesn't have mac, return.
		 */
		if (!INFO_IS_AUTH(inpkt->auth_seq)
		    || info_auth_keyid == 0
		    || ntohl(inpkt->keyid) != info_auth_keyid) {
#ifdef DEBUG
			if (debug > 4)
				printf(
			"failed auth %d info_auth_keyid %u pkt keyid %u\n",
				    INFO_IS_AUTH(inpkt->auth_seq),
				    info_auth_keyid, ntohl(inpkt->keyid));
#endif
			req_ack(srcadr, inter, inpkt, INFO_ERR_AUTH);
			return;
		}
		if (rbufp->recv_length != REQ_LEN_MAC) {
#ifdef DEBUG
			if (debug > 4)
				printf("failed pkt length pkt %d req %d\n",
				    rbufp->recv_length, REQ_LEN_MAC);
#endif
			req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
			return;
		}
		if (!mod_okay || !authhavekey(info_auth_keyid)) {
#ifdef DEBUG
			if (debug > 4)
				printf("failed auth mod_okay %d\n", mod_okay);
#endif
			req_ack(srcadr, inter, inpkt, INFO_ERR_AUTH);
			return;
		}

		/*
		 * calculate absolute time difference between xmit time stamp
		 * and receive time stamp.  If too large, too bad.
		 */
		tmp_ui = ntohl(inpkt->tstamp.l_ui);
		tmp_uf = ntohl(inpkt->tstamp.l_uf);
		M_SUB(tmp_ui, tmp_uf, rbufp->recv_time.l_ui,
		    rbufp->recv_time.l_uf);
		if (M_ISNEG(tmp_ui, tmp_uf))
			M_NEG(tmp_ui, tmp_uf);
		
		if (M_ISHIS(tmp_ui, tmp_uf,
		    INFO_TS_MAXSKEW_UI, INFO_TS_MAXSKEW_UF)) {
			/*
			 * He's a loser.  Tell him.
			 */
			req_ack(srcadr, inter, inpkt, INFO_ERR_AUTH);
			return;
		}

		/*
		 * So far so good.  See if decryption works out okay.
		 */
		if (!authdecrypt(info_auth_keyid, (u_int *)inpkt,
		    REQ_LEN_NOMAC)) {
			req_ack(srcadr, inter, inpkt, INFO_ERR_AUTH);
			return;
		      }
#else /* DES_OK */
		req_ack(srcadr, inter, inpkt, INFO_ERR_AUTH);
		return;
#endif /* DES_OK */
	      }
      

	/*
	 * If we need data, check to see if we have some.  If we
	 * don't, check to see that there is none (picky, picky).
	 */
	if (INFO_ITEMSIZE(inpkt->mbz_itemsize) != proc->sizeofitem) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}
	if (proc->sizeofitem != 0)
		if (proc->sizeofitem*INFO_NITEMS(inpkt->err_nitems)
		    > sizeof(inpkt->data)) {
			req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
			return;
		}

#ifdef DEBUG
	if (debug > 3)
		printf("process_private: all okay, into handler\n");
#endif

	/*
	 * Packet is okay.  Call the handler to send him data.
	 */
	(proc->handler)(srcadr, inter, inpkt);
}


/*
 * peer_list - send a list of the peers
 */
void
peer_list(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_peer_list *ip;
	register struct peer *pp;
	register int i;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();
	extern struct peer *peer_hash[];
	extern struct peer *sys_peer;

	ip = (struct info_peer_list *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_peer_list));
	for (i = 0; i < HASH_SIZE && ip != 0; i++) {
		pp = peer_hash[i];
		while (pp != 0 && ip != 0) {
			ip->address = pp->srcadr.sin_addr.s_addr;
			ip->port = pp->srcadr.sin_port;
			ip->hmode = pp->hmode;
			ip->flags = 0;
			if (pp->flags & FLAG_CONFIG)
				ip->flags |= INFO_FLAG_CONFIG;
			if (pp == sys_peer)
				ip->flags |= INFO_FLAG_SYSPEER;
			if (pp->candidate != 0)
				ip->flags |= INFO_FLAG_SEL_CANDIDATE;
			if (pp->select != 0)
				ip->flags |= INFO_FLAG_SHORTLIST;
			ip = (struct info_peer_list *)more_pkt();
			pp = pp->next;
		}
	}
	flush_pkt();
}


/*
 * peer_list_sum - return extended peer list
 */
void
peer_list_sum(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_peer_summary *ips;
	register struct peer *pp;
	register int i;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();
	extern struct peer *peer_hash[];
	extern struct peer *sys_peer;

#ifdef DEBUG
	if (debug > 2)
		printf("wants peer list summary\n");
#endif

	ips = (struct info_peer_summary *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_peer_summary));
	for (i = 0; i < HASH_SIZE && ips != 0; i++) {
		pp = peer_hash[i];
		while (pp != 0 && ips != 0) {
#ifdef DEBUG
			if (debug > 3)
				printf("sum: got one\n");
#endif
			if (pp->dstadr == any_interface)
				ips->dstadr = 0;
			else
				ips->dstadr = pp->dstadr->sin.sin_addr.s_addr;
			ips->srcadr = pp->srcadr.sin_addr.s_addr;
			ips->srcport = pp->srcadr.sin_port;
			ips->stratum = pp->stratum;
			ips->hpoll = pp->hpoll;
			ips->ppoll = pp->ppoll;
			ips->reach = pp->reach;
			ips->flags = 0;
			if (pp == sys_peer)
				ips->flags |= INFO_FLAG_SYSPEER;
			if (pp->flags & FLAG_CONFIG)
				ips->flags |= INFO_FLAG_CONFIG;
			if (pp->flags & FLAG_REFCLOCK)
				ips->flags |= INFO_FLAG_REFCLOCK;
			if (pp->flags & FLAG_AUTHENABLE)
				ips->flags |= INFO_FLAG_AUTHENABLE;
			if (pp->flags & FLAG_MINPOLL)
				ips->flags |= INFO_FLAG_MINPOLL;
			if (pp->candidate != 0)
				ips->flags |= INFO_FLAG_SEL_CANDIDATE;
			if (pp->select != 0)
				ips->flags |= INFO_FLAG_SHORTLIST;
			ips->hmode = pp->hmode;
			ips->delay = HTONS_FP(pp->estdelay);
			HTONL_FP(&pp->estoffset, &ips->offset);
			ips->dispersion = HTONS_FP(pp->estdisp);
			pp = pp->next;
			ips = (struct info_peer_summary *)more_pkt();
		}
	}
	flush_pkt();
}


/*
 * peer_info - send information for one or more peers
 */
void
peer_info (srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_peer_list *ipl;
	register struct peer *pp;
	register struct info_peer *ip;
	register int items;
	register int i, j;
	struct sockaddr_in addr;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();
	extern struct peer *findexistingpeer();
	extern struct peer *sys_peer;

	bzero((char *)&addr, sizeof addr);
	addr.sin_family = AF_INET;
	items = INFO_NITEMS(inpkt->err_nitems);
	ipl = (struct info_peer_list *) inpkt->data;
	ip = (struct info_peer *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_peer));
	while (items-- > 0 && ip != 0) {
		addr.sin_port = ipl->port;
		addr.sin_addr.s_addr = ipl->address;
		ipl++;
		if ((pp = findexistingpeer(&addr, (struct peer *)0)) == 0)
			continue;
		ip->dstadr = NSRCADR(&pp->dstadr->sin);
		ip->srcadr = NSRCADR(&pp->srcadr);
		ip->srcport = NSRCPORT(&pp->srcadr);
		ip->flags = 0;
		if (pp == sys_peer)
			ip->flags |= INFO_FLAG_SYSPEER;
		if (pp->flags & FLAG_CONFIG)
			ip->flags |= INFO_FLAG_CONFIG;
		if (pp->flags & FLAG_REFCLOCK)
			ip->flags |= INFO_FLAG_REFCLOCK;
		if (pp->flags & FLAG_AUTHENABLE)
			ip->flags |= INFO_FLAG_AUTHENABLE;
		if (pp->flags & FLAG_MINPOLL)
			ip->flags |= INFO_FLAG_MINPOLL;
		if (pp->candidate != 0)
			ip->flags |= INFO_FLAG_SEL_CANDIDATE;
		if (pp->select != 0)
			ip->flags |= INFO_FLAG_SHORTLIST;
		ip->leap = pp->leap;
		ip->hmode = pp->hmode;
		ip->keyid = pp->keyid;
		ip->pkeyid = pp->pkeyid;
		ip->stratum = pp->stratum;
		ip->ppoll = pp->ppoll;
		ip->hpoll = pp->hpoll;
		ip->precision = pp->precision;
		ip->version = pp->version;
		ip->valid = pp->valid;
		ip->reach = pp->reach;
		ip->unreach = pp->unreach;
		ip->trust = pp->trust;
		ip->associd = htons(pp->associd);
		ip->distance = HTONS_FP(pp->distance);
		ip->dispersion = HTONS_FP(pp->dispersion);
		ip->refid = pp->refid;
		ip->timer = htonl(pp->event_timer.event_time - current_time);
		HTONL_FP(&pp->reftime, &ip->reftime);
		HTONL_FP(&pp->org, &ip->org);
		HTONL_FP(&pp->rec, &ip->rec);
		HTONL_FP(&pp->xmt, &ip->xmt);
		j = pp->filter_nextpt - 1;
		for (i = 0; i < PEER_SHIFT; i++, j--) {
			if (j < 0)
				j = PEER_SHIFT-1;
			ip->delay[i] = HTONS_FP(pp->filter_delay[j]);
			HTONL_FP(&pp->filter_offset[j], &ip->offset[i]);
			ip->order[i] = (pp->filter_nextpt+PEER_SHIFT-1)
			    - pp->filter_order[i];
			if (ip->order[i] >= PEER_SHIFT)
				ip->order[i] -= PEER_SHIFT;
		}
		ip->estdelay = HTONS_FP(pp->estdelay);
		HTONL_FP(&pp->estoffset, &ip->estoffset);
		ip->estdisp = HTONS_FP(pp->estdisp);
		j = pp->bdel_next-1;
		for (i = 0; i < PEER_SHIFT; i++, j--) {
			if (j < 0)
				j = PEER_SHIFT-1;
			ip->bdelay[i] = htonl(pp->filter_bdelay[j]);
		}
		ip->estbdelay = htonl(pp->estbdelay);
		ip = (struct info_peer *)more_pkt();
	}
	flush_pkt();
}


/*
 * peer_stats - send statistics for one or more peers
 */
void
peer_stats (srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_peer_list *ipl;
	register struct peer *pp;
	register struct info_peer_stats *ip;
	register int items;
	struct sockaddr_in addr;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();
	extern struct peer *findexistingpeer();
	extern struct peer *sys_peer;

	bzero((char *)&addr, sizeof addr);
	addr.sin_family = AF_INET;
	items = INFO_NITEMS(inpkt->err_nitems);
	ipl = (struct info_peer_list *) inpkt->data;
	ip = (struct info_peer_stats *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_peer_stats));
	while (items-- > 0 && ip != 0) {
		addr.sin_port = ipl->port;
		addr.sin_addr.s_addr = ipl->address;
		ipl++;
		if ((pp = findexistingpeer(&addr, (struct peer *)0)) == 0)
			continue;
		ip->dstadr = NSRCADR(&pp->dstadr->sin);
		ip->srcadr = NSRCADR(&pp->srcadr);
		ip->srcport = NSRCPORT(&pp->srcadr);
		ip->flags = 0;
		if (pp == sys_peer)
			ip->flags |= INFO_FLAG_SYSPEER;
		if (pp->flags & FLAG_CONFIG)
			ip->flags |= INFO_FLAG_CONFIG;
		if (pp->flags & FLAG_REFCLOCK)
			ip->flags |= INFO_FLAG_REFCLOCK;
		if (pp->flags & FLAG_AUTHENABLE)
			ip->flags |= INFO_FLAG_AUTHENABLE;
		if (pp->flags & FLAG_MINPOLL)
			ip->flags |= INFO_FLAG_MINPOLL;
		if (pp->candidate != 0)
			ip->flags |= INFO_FLAG_SEL_CANDIDATE;
		if (pp->select != 0)
			ip->flags |= INFO_FLAG_SHORTLIST;
		ip->timereceived = htonl(current_time - pp->timereceived);
		ip->timetosend
		    = htonl(pp->event_timer.event_time - current_time);
		ip->timereachable = htonl(current_time - pp->timereachable);
		ip->sent = htonl(pp->sent);
		ip->received = htonl(pp->received);
		ip->processed = htonl(pp->processed);
		ip->badlength = htonl(pp->badlength);
		ip->badauth = htonl(pp->badauth);
		ip->bogusorg = htonl(pp->bogusorg);
		ip->oldpkt = htonl(pp->oldpkt);
		ip->baddelay = htonl(pp->baddelay);
		ip->seldelay = htonl(pp->seldelaytoolarge);
		ip->seldisp = htonl(pp->seldisptoolarge);
		ip->selbroken = htonl(pp->selbroken);
		ip->selold = htonl(pp->seltooold);
		ip->candidate = pp->candidate;
		ip->falseticker = pp->falseticker;
		ip->select = pp->select;
		ip->select_total = pp->select_total;
		ip = (struct info_peer_stats *)more_pkt();
	}
	flush_pkt();
}


/*
 * sys_info - return system info
 */
void
sys_info(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_sys *is;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Importations from the protocol module
	 */
	extern u_char sys_leap;
	extern u_char sys_stratum;
	extern s_char sys_precision;
	extern u_fp sys_distance;
	extern u_fp sys_dispersion;
	extern u_int sys_refid;
	extern l_fp sys_reftime;
	extern u_int sys_hold;
	extern struct peer *sys_peer;
	extern int sys_bclient;
	extern u_int sys_bdelay;
	extern int sys_authenticate;
	extern u_int sys_authdelay;
	extern u_fp sys_maxskew;
	extern int sys_select_algorithm;

	is = (struct info_sys *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_sys));

	if (sys_peer != 0) {
		is->peer = NSRCADR(&sys_peer->srcadr);
		is->peer_mode = sys_peer->hmode;
	} else {
		is->peer = 0;
		is->peer_mode = 0;
	}
	is->leap = sys_leap;
	is->stratum = sys_stratum;
	is->precision = sys_precision;
	is->distance = htonl(sys_distance);
	is->dispersion = htonl(sys_dispersion);
	is->refid = sys_refid;
	HTONL_FP(&sys_reftime, &is->reftime);

	if (sys_hold > current_time)
		is->holdtime = htonl(sys_hold - current_time);
	else
		is->holdtime = 0;
	
	is->flags = 0;
	if (sys_bclient)
		is->flags |= INFO_FLAG_BCLIENT;
	if (sys_authenticate)
		is->flags |= INFO_FLAG_AUTHENABLE;
	is->selection = (u_char)sys_select_algorithm;
	
	HTONL_UF(sys_bdelay, &is->bdelay);
	HTONL_UF(sys_authdelay, &is->authdelay);
	is->maxskew = htonl(sys_maxskew);

	(void) more_pkt();
	flush_pkt();
}


/*
 * sys_stats - return system statistics
 */
void
sys_stats(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_sys_stats *ss;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Importations from the protocol module
	 */
	extern u_int sys_stattime;
	extern u_int sys_badstratum;
	extern u_int sys_oldversionpkt;
	extern u_int sys_newversionpkt;
	extern u_int sys_unknownversion;
	extern u_int sys_badlength;
	extern u_int sys_processed;
	extern u_int sys_badauth;
	extern u_int sys_wanderhold;

	ss = (struct info_sys_stats *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_sys_stats));

	ss->timeup = htonl(current_time);
	ss->timereset = htonl(current_time - sys_stattime);
	ss->badstratum = htonl(sys_badstratum);
	ss->oldversionpkt = htonl(sys_oldversionpkt);
	ss->newversionpkt = htonl(sys_newversionpkt);
	ss->unknownversion = htonl(sys_unknownversion);
	ss->badlength = htonl(sys_badlength);
	ss->processed = htonl(sys_processed);
	ss->badauth = htonl(sys_badauth);
	ss->wanderhold = htonl(sys_wanderhold);

	(void) more_pkt();
	flush_pkt();
}


/*
 * mem_stats - return memory statistics
 */
void
mem_stats(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_mem_stats *ms;
	register int i;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Importations from the peer module
	 */
	extern int peer_hash_count[HASH_SIZE];
	extern int peer_free_count;
	extern u_int peer_timereset;
	extern u_int findpeer_calls;
	extern u_int peer_allocations;
	extern u_int peer_demobilizations;
	extern int total_peer_structs;


	ms = (struct info_mem_stats *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_mem_stats));

	ms->timereset = htonl(current_time - peer_timereset);
	ms->totalpeermem = htons((u_short)total_peer_structs);
	ms->freepeermem = htons((u_short)peer_free_count);
	ms->findpeer_calls = htonl(findpeer_calls);
	ms->allocations = htonl(peer_allocations);
	ms->demobilizations = htonl(peer_demobilizations);

	for (i = 0; i < HASH_SIZE; i++) {
		if (peer_hash_count[i] > 255)
			ms->hashcount[i] = 255;
		else
			ms->hashcount[i] = (u_char)peer_hash_count[i];
	}

	(void) more_pkt();
	flush_pkt();
}


/*
 * io_stats - return io statistics
 */
void
io_stats(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_io_stats *io;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Importations from the io module
	 */
	extern u_int io_timereset;
	extern u_int full_recvbufs;
	extern u_int free_recvbufs;
	extern u_int total_recvbufs;
	extern u_int lowater_additions;
	extern u_int packets_dropped;
	extern u_int packets_ignored;
	extern u_int packets_received;
	extern u_int packets_sent;
	extern u_int packets_notsent;
	extern u_int handler_calls;
	extern u_int handler_pkts;

	io = (struct info_io_stats *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_io_stats));

	io->timereset = htonl(current_time - io_timereset);
	io->totalrecvbufs = htons((u_short) total_recvbufs);
	io->freerecvbufs = htons((u_short) free_recvbufs);
	io->fullrecvbufs = htons((u_short) full_recvbufs);
	io->lowwater = htons((u_short) lowater_additions);
	io->dropped = htonl(packets_dropped);
	io->ignored = htonl(packets_ignored);
	io->received = htonl(packets_received);
	io->sent = htonl(packets_sent);
	io->notsent = htonl(packets_notsent);
	io->interrupts = htonl(handler_calls);
	io->int_received = htonl(handler_pkts);

	(void) more_pkt();
	flush_pkt();
}


/*
 * timer_stats - return timer statistics
 */
void
timer_stats(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_timer_stats *ts;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Importations from the timer module
	 */
	extern u_int alarm_overflow;
	extern u_int timer_timereset;
	extern u_int timer_overflows;
	extern u_int timer_xmtcalls;

	ts = (struct info_timer_stats *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_timer_stats));

	ts->timereset = htonl(current_time - timer_timereset);
	ts->alarms = htonl(alarm_overflow);
	ts->overflows = htonl(timer_overflows);
	ts->xmtcalls = htonl(timer_xmtcalls);

	(void) more_pkt();
	flush_pkt();
}


/*
 * loop_info - return the current state of the loop filter
 */
void
loop_info(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_loop *li;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Importations from the loop filter module
	 */
	extern l_fp last_offset;
	extern l_fp drift_comp;
	extern int compliance;
	extern u_int watchdog_timer;

	li = (struct info_loop *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_loop));

	HTONL_FP(&last_offset, &li->last_offset);
	HTONL_FP(&drift_comp, &li->drift_comp);
	HTONL_F(compliance, &li->compliance);
	li->watchdog_timer = htonl(watchdog_timer);

	(void) more_pkt();
	flush_pkt();
}


/*
 * do_conf - add a peer to the configuration list
 */
void
do_conf(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct conf_peer *cp;
	register int items;
	struct sockaddr_in peeraddr;
	int fl;
	extern struct peer *peer_config();

	/*
	 * Do a check of everything to see that it looks
	 * okay.  If not, complain about it.  Note we are
	 * very picky here.
	 */
	items = INFO_NITEMS(inpkt->err_nitems);
	cp = (struct conf_peer *)inpkt->data;

	fl = 0;
	while (items-- > 0 && !fl) {
		if (cp->version != NTP_VERSION
		    && cp->version != NTP_OLDVERSION)
			fl = 1;
		if (cp->hmode != MODE_ACTIVE
		    && cp->hmode != MODE_CLIENT
		    && cp->hmode != MODE_BROADCAST)
			fl = 1;
		if (cp->flags & ~(CONF_FLAG_AUTHENABLE|CONF_FLAG_MINPOLL))
			fl = 1;
		cp++;
	}

	if (fl) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	/*
	 * Looks okay, try it out
	 */
	items = INFO_NITEMS(inpkt->err_nitems);
	cp = (struct conf_peer *)inpkt->data;
	bzero((char *)&peeraddr, sizeof(struct sockaddr_in));
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(NTP_PORT);

	/*
	 * Make sure the address is valid
	 */
#ifdef REFCLOCK
	if (!ISREFCLOCKADR(&peeraddr) && ISBADADR(&peeraddr)) {
#else
	if (ISBADADR(&peeraddr)) {
#endif
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	while (items-- > 0) {
		fl = 0;
		if (cp->flags & CONF_FLAG_AUTHENABLE)
			fl |= FLAG_AUTHENABLE;
		if (cp->flags & CONF_FLAG_MINPOLL)
			fl |= FLAG_MINPOLL;
		peeraddr.sin_addr.s_addr = cp->peeraddr;
		if (peer_config(&peeraddr, (struct interface *)0,
		    cp->hmode, cp->version, cp->keyid, fl) == 0) {
			req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
			return;
		}
		cp++;
	}

	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * do_unconf - remove a peer from the configuration list
 */
void
do_unconf(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct conf_unpeer *cp;
	register int items;
	register struct peer *peer;
	struct sockaddr_in peeraddr;
	int bad, found;
	extern struct peer *findexistingpeer();
	extern int peer_unconfig();

	/*
	 * This is a bit unstructured, but I like to be careful.
	 * We check to see that every peer exists and is actually
	 * configured.  If so, we remove them.  If not, we return
	 * an error.
	 */
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(NTP_PORT);

	items = INFO_NITEMS(inpkt->err_nitems);
	cp = (struct conf_unpeer *)inpkt->data;

	bad = 0;
	while (items-- > 0 && !bad) {
		peeraddr.sin_addr.s_addr = cp->peeraddr;
		found = 0;
		peer = (struct peer *)0;
		while (!found) {
			peer = findexistingpeer(&peeraddr, peer);
			if (peer == (struct peer *)0)
				break;
			if (peer->flags & FLAG_CONFIG)
				found = 1;
		}
		if (!found)
			bad = 1;
		cp++;
	}

	if (bad) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
		return;
	}

	/*
	 * Now do it in earnest.
	 */

	items = INFO_NITEMS(inpkt->err_nitems);
	cp = (struct conf_unpeer *)inpkt->data;
	while (items-- > 0) {
		peeraddr.sin_addr.s_addr = cp->peeraddr;
		peer_unconfig(&peeraddr, (struct conf_unpeer *)0);
		cp++;
	}

	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * set_sys_flag - set system flags
 */
void
set_sys_flag(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void setclr_flags();

	setclr_flags(srcadr, inter, inpkt, 1);
}


/*
 * clr_sys_flag - clear system flags
 */
void
clr_sys_flag(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void setclr_flags();

	setclr_flags(srcadr, inter, inpkt, 0);
}


/*
 * setclr_flags - do the grunge work of flag setting/clearing
 */
void
setclr_flags(srcadr, inter, inpkt, set)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
	int set;
{
	register u_int flags;
	extern void proto_config();

	if (INFO_NITEMS(inpkt->err_nitems) > 1) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	flags = ((struct conf_sys_flags *)inpkt->data)->flags;

	if (flags
	    & ~(SYS_FLAG_BCLIENT|SYS_FLAG_AUTHENTICATE)) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	if (flags & SYS_FLAG_BCLIENT)
		proto_config(PROTO_BROADCLIENT, (int)set);
	if (flags & SYS_FLAG_AUTHENTICATE)
		proto_config(PROTO_AUTHENTICATE, (int)set);
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * do_monitor - turn on monitoring
 */
void
do_monitor(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	extern void mon_start();

	mon_start();
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * do_nomonitor - turn off monitoring
 */
void
do_nomonitor(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	extern void mon_stop();

	mon_stop();
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * list_restrict - return the restrict list
 */
void
list_restrict(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_restrict *ir;
	register struct restrictlist *rl;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();
	extern struct restrictlist *restrictlist;

#ifdef DEBUG
	if (debug > 2)
		printf("wants peer list summary\n");
#endif

	ir = (struct info_restrict *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_restrict));
	for (rl = restrictlist; rl != 0 && ir != 0; rl = rl->next) {
		ir->addr = htonl(rl->addr);
		ir->mask = htonl(rl->mask);
		ir->count = htonl(rl->count);
		ir->flags = htons(rl->flags);
		ir->mflags = htons(rl->mflags);
		ir = (struct info_restrict *)more_pkt();
	}
	flush_pkt();
}



/*
 * do_resaddflags - add flags to a restrict entry (or create one)
 */
void
do_resaddflags(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void do_restrict();

	do_restrict(srcadr, inter, inpkt, RESTRICT_FLAGS);
}



/*
 * do_ressubflags - remove flags from a restrict entry
 */
void
do_ressubflags(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void do_restrict();

	do_restrict(srcadr, inter, inpkt, RESTRICT_UNFLAG);
}


/*
 * do_unrestrict - remove a restrict entry from the list
 */
void
do_unrestrict(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void do_restrict();

	do_restrict(srcadr, inter, inpkt, RESTRICT_REMOVE);
}





/*
 * do_restrict - do the dirty stuff of dealing with restrictions
 */
void
do_restrict(srcadr, inter, inpkt, op)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
	int op;
{
	register struct conf_restrict *cr;
	register int items;
	struct sockaddr_in matchaddr;
	struct sockaddr_in matchmask;
	int bad;
	extern void restrict();

	/*
	 * Do a check of the flags to make sure that only
	 * the NTPPORT flag is set, if any.  If not, complain
	 * about it.  Note we are very picky here.
	 */
	items = INFO_NITEMS(inpkt->err_nitems);
	cr = (struct conf_restrict *)inpkt->data;

	bad = 0;
	while (items-- > 0 && !bad) {
		if (cr->mflags & ~(RESM_NTPONLY))
			bad = 1;
		if (cr->flags & ~(RES_ALLFLAGS))
			bad = 1;
		if (cr->addr == INADDR_ANY && cr->mask != INADDR_ANY)
			bad = 1;
		cr++;
	}

	if (bad) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	/*
	 * Looks okay, try it out
	 */
	items = INFO_NITEMS(inpkt->err_nitems);
	cr = (struct conf_restrict *)inpkt->data;
	bzero((char *)&matchaddr, sizeof(struct sockaddr_in));
	bzero((char *)&matchmask, sizeof(struct sockaddr_in));
	matchaddr.sin_family = AF_INET;
	matchmask.sin_family = AF_INET;

	while (items-- > 0) {
		matchaddr.sin_addr.s_addr = cr->addr;
		matchmask.sin_addr.s_addr = cr->mask;
		restrict(op, &matchaddr, &matchmask, htons(cr->mflags),
		    htons(cr->flags));
		cr++;
	}

	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * mon_getlist - return monitor data
 */
void
mon_getlist(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_monitor *im;
	register struct mon_data *md;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();
	extern struct mon_data mon_mru_list;
	extern int mon_enabled;

#ifdef DEBUG
	if (debug > 2)
		printf("wants monitor list\n");
#endif
	if (!mon_enabled) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
		return;
	}

	im = (struct info_monitor *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_monitor));
	for (md = mon_mru_list.mru_next; md != &mon_mru_list && im != 0;
	    md = md->mru_next) {
		im->lasttime = htonl(current_time - md->lasttime);
		im->firsttime = htonl(current_time - md->firsttime);
		im->count = htonl(md->count);
		im->addr = md->rmtadr;
		im->port = md->rmtport;
		im->mode = md->mode;
		im->version = md->version;
		im = (struct info_monitor *)more_pkt();
	}
	flush_pkt();
}

/*
 * Module entry points and the flags they correspond with
 */
struct reset_entry {
	int flag;		/* flag this corresponds to */
	void (*handler)();	/* routine to handle request */
};

extern void peer_all_reset();
extern void peer_clr_stats();
extern void io_clr_stats();
extern void proto_clr_stats();
extern void timer_clr_stats();
void reset_auth_stats();
extern void ctl_clr_stats();

struct reset_entry reset_entries[] = {
	{ RESET_FLAG_ALLPEERS,	peer_all_reset },
	{ RESET_FLAG_IO,	io_clr_stats },
	{ RESET_FLAG_SYS,	proto_clr_stats },
	{ RESET_FLAG_MEM,	peer_clr_stats },
	{ RESET_FLAG_TIMER,	timer_clr_stats },
	{ RESET_FLAG_AUTH,	reset_auth_stats },
	{ RESET_FLAG_CTL,	ctl_clr_stats },
	{ 0,			0 }
};

/*
 * reset_stats - reset statistic counters here and there
 */
void
reset_stats(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	u_int flags;
	struct reset_entry *rent;

	if (INFO_NITEMS(inpkt->err_nitems) > 1) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	flags = ((struct reset_flags *)inpkt->data)->flags;

	if (flags & ~RESET_ALLFLAGS) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	for (rent = reset_entries; rent->flag != 0; rent++) {
		if (flags & rent->flag)
			(rent->handler)();
	}
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * reset_peer - clear a peer's statistics
 */
void
reset_peer(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct conf_unpeer *cp;
	register int items;
	register struct peer *peer;
	struct sockaddr_in peeraddr;
	int bad;
	extern struct peer *findexistingpeer();
	extern void peer_reset();

	/*
	 * We check first to see that every peer exists.  If not,
	 * we return an error.
	 */
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(NTP_PORT);

	items = INFO_NITEMS(inpkt->err_nitems);
	cp = (struct conf_unpeer *)inpkt->data;

	bad = 0;
	while (items-- > 0 && !bad) {
		peeraddr.sin_addr.s_addr = cp->peeraddr;
		peer = findexistingpeer(&peeraddr, (struct peer *)0);
		if (peer == (struct peer *)0)
			bad++;
		cp++;
	}

	if (bad) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
		return;
	}

	/*
	 * Now do it in earnest.
	 */

	items = INFO_NITEMS(inpkt->err_nitems);
	cp = (struct conf_unpeer *)inpkt->data;
	while (items-- > 0) {
		peeraddr.sin_addr.s_addr = cp->peeraddr;
		peer = findexistingpeer(&peeraddr, (struct peer *)0);
		peer_reset(peer);
		cp++;
	}

	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * do_key_reread - reread the encryption key file
 */
void
do_key_reread(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	extern void rereadkeys();

	rereadkeys();
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * do_dirty_hack
 */
void
do_dirty_hack(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	/* historical placeholder */
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * dont_dirty_hack
 */
void
dont_dirty_hack(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	/* historical placeholder */
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * trust_key - make one or more keys trusted
 */
void
trust_key(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void do_trustkey();

	do_trustkey(srcadr, inter, inpkt, 1);
}


/*
 * untrust_key - make one or more keys untrusted
 */
void
untrust_key(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void do_trustkey();

	do_trustkey(srcadr, inter, inpkt, 0);
}


/*
 * do_trustkey - make keys either trustable or untrustable
 */
void
do_trustkey(srcadr, inter, inpkt, trust)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
	int trust;
{
#ifdef DES_OK
	register u_int *kp;
	register int items;
	extern void authtrust();

	items = INFO_NITEMS(inpkt->err_nitems);
	kp = (u_int *)inpkt->data;
	while (items-- > 0) {
		authtrust(*kp, trust);
		kp++;
	}

#endif /* DES_OK */
	req_ack(srcadr, inter, inpkt, INFO_OKAY);

}


/*
 * get_auth_info - return some stats concerning the authentication module
 */
void
get_auth_info(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_auth *ia;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Importations from the authentication module
	 */
	extern u_int authnumkeys;
	extern u_int authnumfreekeys;
	extern u_int authkeylookups;
	extern u_int authkeynotfound;
	extern u_int authencryptions;
	extern u_int authdecryptions;
	extern u_int authdecryptok;
	extern u_int authkeyuncached;

	ia = (struct info_auth *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_auth));

	ia->numkeys = htonl(authnumkeys);
	ia->numfreekeys = htonl(authnumfreekeys);
	ia->keylookups = htonl(authkeylookups);
	ia->keynotfound = htonl(authkeynotfound);
	ia->encryptions = htonl(authencryptions);
	ia->decryptions = htonl(authdecryptions);
	ia->decryptok = htonl(authdecryptok);
	ia->keyuncached = htonl(authkeyuncached);
	ia->timereset = htonl(current_time - auth_timereset);
	
	(void) more_pkt();
	flush_pkt();
}



/*
 * reset_auth_stats - reset the authentication stat counters.  Done here
 *		      to keep xntp-isms out of the authentication module
 */
void
reset_auth_stats()
{
	/*
	 * Importations from the authentication module
	 */
	extern u_int authkeylookups;
	extern u_int authkeynotfound;
	extern u_int authencryptions;
	extern u_int authdecryptions;
	extern u_int authdecryptok;
	extern u_int authkeyuncached;

	authkeylookups = 0;
	authkeynotfound = 0;
	authencryptions = 0;
	authdecryptions = 0;
	authdecryptok = 0;
	authkeyuncached = 0;
	auth_timereset = current_time;
}


/*
 * req_get_traps - return information about current trap holders
 */
void
req_get_traps(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_trap *it;
	register struct ctl_trap *tr;
	register int i;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Imported from the control module
	 */
	extern struct ctl_trap ctl_trap[];
	extern int num_ctl_traps;

	if (num_ctl_traps == 0) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
		return;
	}

	it = (struct info_trap *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_trap));

	for (i = 0, tr = ctl_trap; i < CTL_MAXTRAPS; i++, tr++) {
		if (tr->tr_flags & TRAP_INUSE) {
			if (tr->tr_localaddr == any_interface)
				it->local_address = 0;
			else
				it->local_address
				    = NSRCADR(&tr->tr_localaddr->sin);
			it->trap_address = NSRCADR(&tr->tr_addr);
			it->trap_port = NSRCPORT(&tr->tr_addr);
			it->sequence = htons(tr->tr_sequence);
			it->settime = htonl(current_time - tr->tr_settime);
			it->origtime = htonl(current_time - tr->tr_origtime);
			it->resets = htonl(tr->tr_resets);
			it->flags = htonl((u_int)tr->tr_flags);
			it = (struct info_trap *)more_pkt();
		}
	}
	flush_pkt();
}


/*
 * req_set_trap - configure a trap
 */
void
req_set_trap(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void do_setclr_trap();

	do_setclr_trap(srcadr, inter, inpkt, 1);
}



/*
 * req_clr_trap - unconfigure a trap
 */
void
req_clr_trap(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	void do_setclr_trap();

	do_setclr_trap(srcadr, inter, inpkt, 0);
}



/*
 * do_setclr_trap - do the grunge work of (un)configuring a trap
 */
void
do_setclr_trap(srcadr, inter, inpkt, set)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
	int set;
{
	register struct conf_trap *ct;
	register struct interface *linter;
	int res;
	struct sockaddr_in laddr;
	extern int ctlsettrap();
	extern int ctlclrtrap();
	extern struct interface *findinterface();

	/*
	 * Prepare sockaddr_in structure
	 */
	bzero((char *)&laddr, sizeof laddr);
	laddr.sin_family = AF_INET;
	laddr.sin_port = ntohs(NTP_PORT);

	/*
	 * Restrict ourselves to one item only.  This eliminates
	 * the error reporting problem.
	 */
	if (INFO_NITEMS(inpkt->err_nitems) > 1) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}
	ct = (struct conf_trap *)inpkt->data;

	/*
	 * Look for the local interface.  If none, use the default.
	 */
	if (ct->local_address == 0) {
		linter = any_interface;
	} else {
		laddr.sin_addr.s_addr = ct->local_address;
		linter = findinterface(&laddr);
		if (linter == NULL) {
			req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
			return;
		}
	}

	laddr.sin_addr.s_addr = ct->trap_address;
	if (ct->trap_port != 0)
		laddr.sin_port = ct->trap_port;
	else
		laddr.sin_port = htons(TRAPPORT);

	if (set) {
		res = ctlsettrap(&laddr, linter, 0);
	} else {
		res = ctlclrtrap(&laddr, linter, 0);
	}

	if (!res) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
	} else {
		req_ack(srcadr, inter, inpkt, INFO_OKAY);
	}
	return;
}



/*
 * set_request_keyid - set the keyid used to authenticate requests
 */
void
set_request_keyid(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	u_int keyid;

	/*
	 * Restrict ourselves to one item only.
	 */
	if (INFO_NITEMS(inpkt->err_nitems) > 1) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	keyid = ntohl(*((u_int *)(inpkt->data)));
	info_auth_keyid = keyid;
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}



/*
 * set_control_keyid - set the keyid used to authenticate requests
 */
void
set_control_keyid(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	u_int keyid;
	extern u_int ctl_auth_keyid;

	/*
	 * Restrict ourselves to one item only.
	 */
	if (INFO_NITEMS(inpkt->err_nitems) > 1) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	keyid = ntohl(*((u_int *)(inpkt->data)));
	ctl_auth_keyid = keyid;
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}



/*
 * get_ctl_stats - return some stats concerning the control message module
 */
void
get_ctl_stats(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_control *ic;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Importations from the control module
	 */
	extern u_int ctltimereset;
	extern u_int numctlreq;
	extern u_int numctlbadpkts;
	extern u_int numctlresponses;
	extern u_int numctlfrags;
	extern u_int numctlerrors;
	extern u_int numctltooshort;
	extern u_int numctlinputresp;
	extern u_int numctlinputfrag;
	extern u_int numctlinputerr;
	extern u_int numctlbadoffset;
	extern u_int numctlbadversion;
	extern u_int numctldatatooshort;
	extern u_int numctlbadop;
	extern u_int numasyncmsgs;

	ic = (struct info_control *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_control));

	ic->ctltimereset = htonl(current_time - ctltimereset);
	ic->numctlreq = htonl(numctlreq);
	ic->numctlbadpkts = htonl(numctlbadpkts);
	ic->numctlresponses = htonl(numctlresponses);
	ic->numctlfrags = htonl(numctlfrags);
	ic->numctlerrors = htonl(numctlerrors);
	ic->numctltooshort = htonl(numctltooshort);
	ic->numctlinputresp = htonl(numctlinputresp);
	ic->numctlinputfrag = htonl(numctlinputfrag);
	ic->numctlinputerr = htonl(numctlinputerr);
	ic->numctlbadoffset = htonl(numctlbadoffset);
	ic->numctlbadversion = htonl(numctlbadversion);
	ic->numctldatatooshort = htonl(numctldatatooshort);
	ic->numctlbadop = htonl(numctlbadop);
	ic->numasyncmsgs = htonl(numasyncmsgs);

	(void) more_pkt();
	flush_pkt();
}



/*
 * get_leap_info - return some stats concerning the control message module
 */
void
get_leap_info(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_leap *il;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();

	/*
	 * Imported from the protocol module
	 */
	extern u_char sys_leap;

	/*
	 * Importations from the leap module
	 */
	extern u_char leap_indicator;
	extern u_char leap_warning;
	extern u_char leapbits;
	extern u_char leapseenstratum1;
	extern u_int leap_timer;
	extern u_int leap_processcalls;
	extern u_int leap_notclose;
	extern u_int leap_monthofleap;
	extern u_int leap_dayofleap;
	extern u_int leap_hoursfromleap;
	extern u_int leap_happened;

	il = (struct info_leap *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_leap));

	il->sys_leap = sys_leap;
	il->leap_indicator = leap_indicator;
	il->leap_warning = leap_warning;
	il->leap_bits = (leapbits & INFO_LEAP_MASK)
	    | ((leapseenstratum1 != 0) ? INFO_LEAP_SEENSTRATUM1 : 0);
	il->leap_timer = htonl(leap_timer - current_time);
	il->leap_processcalls = htonl(leap_processcalls);
	il->leap_notclose = htonl(leap_notclose);
	il->leap_monthofleap = htonl(leap_monthofleap);
	il->leap_dayofleap = htonl(leap_dayofleap);
	il->leap_hoursfromleap = htonl(leap_hoursfromleap);
	il->leap_happened = htonl(leap_happened);

	(void) more_pkt();
	flush_pkt();
}


#ifdef REFCLOCK
/*
 * get_clock_info - get info about a clock
 */
void
get_clock_info(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct info_clock *ic;
	register u_int *clkaddr;
	register int items;
	struct refclockstat clock;
	struct sockaddr_in addr;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();
	extern struct peer *findexistingpeer();
	extern void refclock_control();

	bzero((char *)&addr, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(NTP_PORT);
	items = INFO_NITEMS(inpkt->err_nitems);
	clkaddr = (u_int *) inpkt->data;

	ic = (struct info_clock *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_clock));

	while (items-- > 0) {
		addr.sin_addr.s_addr = *clkaddr++;
		if (!ISREFCLOCKADR(&addr) ||
		    findexistingpeer(&addr, (struct peer *)0) == 0) {
			req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
			return;
		}
		refclock_control(&addr, (struct refclockstat *)0, &clock);

		ic->clockadr = addr.sin_addr.s_addr;
		ic->type = clock.type;
		ic->flags = clock.flags;
		ic->lastevent = clock.lastevent;
		ic->currentstatus = clock.currentstatus;
		ic->polls = htonl(clock.polls);
		ic->noresponse = htonl(clock.noresponse);
		ic->badformat = htonl(clock.badformat);
		ic->baddata = htonl(clock.baddata);
		ic->timestarted = htonl(current_time - clock.timereset);
		HTONL_FP(&clock.fudgetime1, &ic->fudgetime1);
		HTONL_FP(&clock.fudgetime2, &ic->fudgetime2);
		ic->fudgeval1 = htonl(clock.fudgeval1);
		ic->fudgeval2 = htonl(clock.fudgeval2);

		ic = (struct info_clock *)more_pkt();
	}
	flush_pkt();
}



/*
 * set_clock_fudge - get a clock's fudge factors
 */
void
set_clock_fudge(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register struct conf_fudge *cf;
	register int items;
	struct refclockstat clock;
	struct sockaddr_in addr;
	extern struct peer *findexistingpeer();
	extern void refclock_control();

	bzero((char *)&addr, sizeof addr);
	bzero((char *)&clock, sizeof clock);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(NTP_PORT);
	items = INFO_NITEMS(inpkt->err_nitems);
	cf = (struct conf_fudge *) inpkt->data;

	while (items-- > 0) {
		addr.sin_addr.s_addr = cf->clockadr;
		if (!ISREFCLOCKADR(&addr) ||
		    findexistingpeer(&addr, (struct peer *)0) == 0) {
			req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
			return;
		}

		switch(ntohl(cf->which)) {
		case FUDGE_TIME1:
			NTOHL_FP(&cf->fudgetime, &clock.fudgetime1);
			clock.haveflags = CLK_HAVETIME1;
			break;
		case FUDGE_TIME2:
			NTOHL_FP(&cf->fudgetime, &clock.fudgetime2);
			clock.haveflags = CLK_HAVETIME2;
			break;
		case FUDGE_VAL1:
			clock.fudgeval1 = ntohl(cf->fudgeval_flags);
			clock.haveflags = CLK_HAVEVAL1;
			break;
		case FUDGE_VAL2:
			clock.fudgeval2 = ntohl(cf->fudgeval_flags);
			clock.haveflags = CLK_HAVEVAL2;
			break;
		case FUDGE_FLAGS:
			clock.flags = ntohl(cf->fudgeval_flags) & 0xf;
			clock.haveflags =
		(CLK_HAVEFLAG1|CLK_HAVEFLAG2|CLK_HAVEFLAG3|CLK_HAVEFLAG4);
			break;
		default:
			req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
			return;
		}

		refclock_control(&addr, &clock, (struct refclockstat *)0);
	}

	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}
#endif

/*
 * set_maxskew - set the system maxskew parameter
 */
void
set_maxskew(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register u_fp maxskew;
	extern void proto_config();

	if (INFO_NITEMS(inpkt->err_nitems) > 1) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	maxskew = NTOHS_FP(*(u_fp *)(inpkt->data));

	proto_config(PROTO_MAXSKEW, (int)maxskew);
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * set_precision - set the system precision
 */
void
set_precision(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register int precision;
	extern void proto_config();

	precision = ntohl(*(int *)(inpkt->data));

	if (INFO_NITEMS(inpkt->err_nitems) > 1 ||
	    precision > -1 || precision < -20) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	proto_config(PROTO_PRECISION, precision);
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


/*
 * set_select_code - set a select code to use
 */
void
set_select_code(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register u_int select_code;
	extern void proto_config();

	select_code = ntohl(*(u_int *)(inpkt->data));

	if (INFO_NITEMS(inpkt->err_nitems) > 1 ||
	    select_code < SELECT_1 || select_code > SELECT_5) {
		req_ack(srcadr, inter, inpkt, INFO_ERR_FMT);
		return;
	}

	proto_config(PROTO_SELECT, (int)select_code);
	req_ack(srcadr, inter, inpkt, INFO_OKAY);
}


#ifdef REFCLOCK
/*
 * get_clkbug_info - get debugging info about a clock
 */
void
get_clkbug_info(srcadr, inter, inpkt)
	struct sockaddr_in *srcadr;
	struct interface *inter;
	struct req_pkt *inpkt;
{
	register int i;
	register struct info_clkbug *ic;
	register u_int *clkaddr;
	register int items;
	struct refclockbug bug;
	struct sockaddr_in addr;
	char *prepare_pkt();
	char *more_pkt();
	void flush_pkt();
	extern struct peer *findexistingpeer();
	extern void refclock_buginfo();

	bzero((char *)&addr, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(NTP_PORT);
	items = INFO_NITEMS(inpkt->err_nitems);
	clkaddr = (u_int *) inpkt->data;

	ic = (struct info_clkbug *)prepare_pkt(srcadr, inter, inpkt,
	    sizeof(struct info_clkbug));

	while (items-- > 0) {
		addr.sin_addr.s_addr = *clkaddr++;
		if (!ISREFCLOCKADR(&addr) ||
		    findexistingpeer(&addr, (struct peer *)0) == 0) {
			req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
			return;
		}

		bzero((char *)&bug, sizeof bug);
		refclock_buginfo(&addr, &bug);
		if (bug.nvalues == 0 && bug.ntimes == 0) {
			req_ack(srcadr, inter, inpkt, INFO_ERR_NODATA);
			return;
		}

		ic->clockadr = addr.sin_addr.s_addr;
		i = bug.nvalues;
		if (i > NUMCBUGVALUES)
			i = NUMCBUGVALUES;
		ic->nvalues = (u_char)i;
		ic->svalues = htons((u_short)bug.svalues & ((1<<i)-1));
		while (--i >= 0)
			ic->values[i] = htonl(bug.values[i]);

		i = bug.ntimes;
		if (i > NUMCBUGTIMES)
			i = NUMCBUGTIMES;
		ic->ntimes = (u_char)i;
		ic->stimes = htonl((u_int)bug.stimes & ((1<<i)-1));
		while (--i >= 0) {
			ic->times[i].l_ui = htonl(bug.times[i].l_ui);
			ic->times[i].l_uf = htonl(bug.times[i].l_uf);
		}

		ic = (struct info_clkbug *)more_pkt();
	}
	flush_pkt();
}
#endif

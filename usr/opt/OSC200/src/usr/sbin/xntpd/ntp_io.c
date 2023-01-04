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
static char     *sccsid = "@(#)$RCSfile: ntp_io.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/05/04 16:57:07 $";
#endif
/*
 * xntp_io.c - input/output routines for xntpd.  The socket-opening code
 *	       was shamelessly stolen from ntpd.
 */
#include <stdio.h>
#include <strings.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
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
#include <ntp/ntp_refclock.h>

#ifndef FD_SET
#define	NFDBITS		32
#define	FD_SETSIZE	32
#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))
#endif

#if defined(ULT_2_0)
#ifndef sigmask
#define	sigmask(m)	(1<<(m))
#endif
#endif

/*
 * We do asynchronous input using the SIGIO facility.  A number of
 * recvbuf buffers are preallocated for input.  In the signal
 * handler we poll to see which sockets are ready and read the
 * packets from them into the recvbuf's along with a time stamp and
 * an indication of the source host and the interface it was received
 * through.  This allows us to get as accurate receive time stamps
 * as possible independent of other processing going on.
 *
 * We watch the number of recvbufs available to the signal handler
 * and allocate more when this number drops below the low water
 * mark.  If the signal handler should run out of buffers in the
 * interim it will drop incoming frames, the idea being that it is
 * better to drop a packet than to be inaccurate.
 */

/*
 * Block the interrupt, for critical sections.
 */
#define	BLOCKIO(osig)	(osig = sigblock(sigmask(SIGIO)))
#define	UNBLOCKIO(osig)	((void) sigsetmask(osig))

/*
 * recvbuf memory management
 */
#define	RECV_INIT	10	/* 10 buffers initially */
#define	RECV_LOWAT	3	/* when we're down to three buffers get more */
#define	RECV_INC	5	/* get 5 more at a time */
#define	RECV_TOOMANY	30	/* this is way too many buffers */

/*
 * Memory allocation
 */
u_int full_recvbufs;	/* number of recvbufs on fulllist */
u_int free_recvbufs;	/* number of recvbufs on freelist */

struct recvbuf *freelist;	/* free buffers */
struct recvbuf *fulllist;	/* buffers with data */

u_int total_recvbufs;	/* total recvbufs currently in use */
u_int lowater_additions;	/* number of times we have added memory */

struct recvbuf initial_bufs[RECV_INIT];	/* initial allocation */


/*
 * Other statistics of possible interest
 */
u_int packets_dropped;	/* total number of packets dropped on reception */
u_int packets_ignored;	/* packets received on wild card interface */
u_int packets_received;	/* total number of packets received */
u_int packets_sent;	/* total number of packets sent */
u_int packets_notsent;	/* total number of packets which couldn't be sent */

u_int handler_calls;	/* number of calls to interrupt handler */
u_int handler_pkts;	/* number of pkts received by handler */
u_int io_timereset;	/* time counters were reset */

/*
 * Interface stuff
 */
#define	MAXINTERFACES	10
struct interface *any_interface;	/* pointer to default interface */
struct interface *loopback_interface;	/* point to loopback interface */
struct interface inter_list[MAXINTERFACES];
int ninterfaces;

#ifdef REFCLOCK
/*
 * Refclock stuff.  We keep a chain of structures with data concerning
 * the guys we are doing I/O for.
 */
struct refclockio *refio;
#endif

/*
 * File descriptor masks etc. for call to select
 */
fd_set activefds;
int maxactivefd;

int init_first=FALSE;
/*
 * Imported from ntp_timer.c
 */
extern u_int current_time;

extern int errno;
extern int debug;


/*
 * init_io - initialize I/O data structures and call socket creation routine
 */
void
init_io()
{
	register int i;
	void input_handler();	/* input handler routine */

	/*
	 * Init buffer free list and stat counters
	 */
	freelist = 0;
	for (i = 0; i < RECV_INIT; i++) {
		initial_bufs[i].next = freelist;
		freelist = &initial_bufs[i];
	}

	fulllist = 0;
	free_recvbufs = total_recvbufs = RECV_INIT;
	full_recvbufs = lowater_additions = 0;
	packets_dropped = packets_received = 0;
	packets_ignored = 0;
	packets_sent = packets_notsent = 0;
	handler_calls = handler_pkts = 0;
	io_timereset = 0;
	loopback_interface = 0;

#ifdef REFCLOCK
	refio = 0;
#endif

	/*
	 * Point SIGIO at service routine
	 */
	(void) signal(SIGIO, input_handler);

	/*
	 * Create the sockets
	 */
	init_first=TRUE;
	BLOCKIO(i);
	(void) create_sockets(htons(NTP_PORT));
	UNBLOCKIO(i);
	init_first=FALSE;

#ifdef DEBUG
	if (debug)
		printf("init_io: maxactivefd %d\n", maxactivefd);
#endif
}


/*
 * create_sockets - create a socket for each interface plus a default
 *		    socket for when we don't know where to send
 */
int
create_sockets(port)
	unsigned int port;
{
	char	buf[1024];
	struct	ifconf	ifc;
	struct	ifreq	ifreq, *ifr;
	int n, i, j, vs;
	struct sockaddr_in resmask;
	int open_socket();
	extern void restrict();

	/*
	 * create pseudo-interface with wildcard address
	 */
	inter_list[0].sin.sin_family = AF_INET;
	inter_list[0].sin.sin_port = port;
	inter_list[0].sin.sin_addr.s_addr = INADDR_ANY;
	(void) strncpy(inter_list[0].name, "wildcard",
	     sizeof(inter_list[0].name));
	inter_list[0].mask.sin_addr.s_addr = htonl(~0);
	inter_list[0].received = 0;
	inter_list[0].sent = 0;
	inter_list[0].notsent = 0;
	inter_list[0].flags = INT_BROADCAST;

	if ((vs = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, "vs=socket(AF_INET, SOCK_DGRAM) %m");
		exit(1);
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(vs, SIOCGIFCONF, (char *)&ifc) < 0) {
		syslog(LOG_ERR, "get interface configuration: %m");
		exit(1);
	}
	n = ifc.ifc_len/sizeof(struct ifreq);

	i = 1;
	for (ifr = ifc.ifc_req; n > 0; n--, ifr++) {
		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;
		ifreq = *ifr;
		if (ioctl(vs, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, "get interface flags: %m");
			continue;
		}
		if ((ifreq.ifr_flags & IFF_UP) == 0)
			continue;
		inter_list[i].flags = 0;
		if (ifreq.ifr_flags & IFF_BROADCAST)
			inter_list[i].flags |= INT_BROADCAST;
#if !defined(SUN_3_3)
		if (ifreq.ifr_flags & IFF_LOOPBACK) {
			inter_list[i].flags |= INT_LOOPBACK;
			if (loopback_interface == 0)
				loopback_interface = &inter_list[i];
		}
#endif

		if (ioctl(vs, SIOCGIFADDR, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, "get interface addr: %m");
			continue;
		}

		(void)strncpy(inter_list[i].name, ifreq.ifr_name,
		    sizeof(inter_list[i].name));
		inter_list[i].sin = *(struct sockaddr_in *)&ifreq.ifr_addr;
		inter_list[i].sin.sin_family = AF_INET;
		inter_list[i].sin.sin_port = port;

#if defined(SUN_3_3)
		
		if (SRCADR(&inter_list[i].sin) == 0x7f000001) {
			inter_list[i].flags |= INT_LOOPBACK;
			if (loopback_interface == 0)
				loopback_interface = &inter_list[i];
		}
#endif
		if (inter_list[i].flags & INT_BROADCAST) {
			if (ioctl(vs, SIOCGIFBRDADDR, (char *)&ifreq) < 0) {
				syslog(LOG_ERR, "SIOCGIFBRDADDR fails");
				exit(1);
			}
#if defined(SUN_3_3)
			inter_list[i].bcast =
				*(struct sockaddr_in *)&ifreq.ifr_addr;
#else
			inter_list[i].bcast =
				*(struct sockaddr_in *)&ifreq.ifr_broadaddr;
#endif
			inter_list[i].bcast.sin_family = AF_INET;
			inter_list[i].bcast.sin_port = port;
		}
		if (ioctl(vs, SIOCGIFNETMASK, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, "SIOCGIFNETMASK fails");
			exit(1);
		}
		inter_list[i].mask = *(struct sockaddr_in *)&ifreq.ifr_addr;

		/* 
		 * look for an already existing source interface address.  If
		 * the machine has multiple point to point interfaces, then 
		 * the local address may appear more than once.
		 */		   
		for (j=0; j < i; j++)
			if (inter_list[j].sin.sin_addr.s_addr == 
			    inter_list[i].sin.sin_addr.s_addr) {
				break;
			}
		if (j == i)
			i++;
	}
	close(vs);
	ninterfaces = i;

	maxactivefd = 0;
	FD_ZERO(&activefds);

	for (i = 0; i < ninterfaces; i++) {
		inter_list[i].fd = open_socket(&inter_list[i].sin,
		    inter_list[i].flags & INT_BROADCAST);
	}

	/*
	 * Blacklist all bound interface addresses
	 */
	resmask.sin_addr.s_addr = ~0L;
	for (i = 1; i < ninterfaces; i++)
		restrict(RESTRICT_FLAGS, &inter_list[i].sin, &resmask,
		    RESM_NTPONLY|RESM_INTERFACE, RES_IGNORE);

	any_interface = &inter_list[0];
	return ninterfaces;
}


/*
 * io_setbclient - open the broadcast client sockets
 */
void
io_setbclient()
{
	int i;
	int open_socket();


	for (i = 1; i < ninterfaces; i++) {
		if (!(inter_list[i].flags & INT_BROADCAST))
			continue;
		if (inter_list[i].flags & INT_BCASTOPEN)
			continue;
		inter_list[i].bfd = open_socket(&inter_list[i].bcast, 0);
		inter_list[i].flags |= INT_BCASTOPEN;
	}
}


/*
 * io_unsetbclient - close the broadcast client sockets
 */
void
io_unsetbclient()
{
	int i;
	void close_socket();

	for (i = 1; i < ninterfaces; i++) {
		if (!(inter_list[i].flags & INT_BCASTOPEN))
			continue;
		close_socket(inter_list[i].bfd);
		inter_list[i].flags &= ~INT_BCASTOPEN;
	}
}



/*
 * open_socket - open a socket, returning the file descriptor
 */
int
open_socket(addr, bcast)
	struct sockaddr_in *addr;
	int bcast;
{
	int fd;
	int on = 1, off = 0;

	/* create a datagram (UDP) socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	        if (init_first == TRUE)
		  printf("socket() failed: %m");
		syslog(LOG_ERR, "socket() failed: %m");
		exit(1);
		/*NOTREACHED*/
	}

	if (fd > maxactivefd)
		maxactivefd = fd;
	FD_SET(fd, &activefds);

	/* set SO_REUSEADDR since we will be binding the same port
	   number on each interface */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			  (char *)&on, sizeof(on))) {
		syslog(LOG_ERR, "setsockopt SO_REUSEADDR on fails: %m");
	}

	/*
	 * bind the local address.
	 */
	if (bind(fd, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
		syslog(LOG_ERR, "bind() fails: %m");
		exit(1);
	}

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
	 * in the case of sockets.
	 */
	if (fcntl(fd, F_SETOWN, -getpid()) == -1) {
#endif
		syslog(LOG_ERR, "fcntl(F_SETOWN) fails: %m");
		exit(1);
	}

	/*
	 * set non-blocking, async I/O on the descriptor
	 */
	if (fcntl(fd, F_SETFL, FNDELAY|FASYNC) < 0) {
		syslog(LOG_ERR, "fcntl(FNDELAY|FASYNC) fails: %m");
		exit(1);
		/*NOTREACHED*/
	}

	/*
	 *  Turn off the SO_REUSEADDR socket option.  It apparently
	 *  causes heartburn on systems with multicast IP installed.
	 *  On normal systems it only gets looked at when the address
	 *  is being bound anyway..
	 */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
	    (char *)&off, sizeof(off))) {
		syslog(LOG_ERR, "setsockopt SO_REUSEADDR off fails: %m");
	}

#ifdef SO_BROADCAST
	/* if this interface can support broadcast, set SO_BROADCAST */
	if (bcast) {
		if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
		    (char *)&on, sizeof(on))) {
			syslog(LOG_ERR, "setsockopt(SO_BROADCAST): %m");
		}
	}
#endif

#ifdef DEBUG
	if (debug > 1)
		printf("flags for fd %d: 0%o\n", fd,
		    fcntl(fd, F_GETFL, 0));
#endif

	return fd;
}


/*
 * closesocket - close a socket and remove from the activefd list
 */
void
close_socket(fd)
	int fd;
{
	int i, newmax;

	(void) close(fd);
	FD_CLR(fd, &activefds);

	if (fd >= maxactivefd) {
		newmax = 0;
		for (i = 0; i < maxactivefd; i++)
			if (FD_ISSET(i, &activefds))
				newmax = i;
		maxactivefd = newmax;
	}
}



/*
 * findbcastinter - find broadcast interface corresponding to address
 */
struct interface *
findbcastinter(addr)
	struct sockaddr_in *addr;
{
#ifdef	SIOCGIFCONF
	register int i;
	register u_int netnum;

	netnum = NSRCADR(addr);
	for (i = 1; i < ninterfaces; i++) {
		if (!(inter_list[i].flags & INT_BROADCAST))
			continue;
		if (NSRCADR(&inter_list[i].bcast) == netnum)
			return &inter_list[i];
		if ((NSRCADR(&inter_list[i].sin) & NSRCADR(&inter_list[i].mask))
		    == (netnum & NSRCADR(&inter_list[i].mask)))
			return &inter_list[i];
	}
#endif
	return any_interface;
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
	extern char *emalloc();

#ifdef DEBUG
	if (debug > 4)
		printf("getrecvbufs: %d handler interrupts, %d frames\n",
		    handler_calls, handler_pkts);
#endif

	if (full_recvbufs == 0) {
#ifdef DEBUG
		if (debug > 4)
			printf("getrecvbufs called, no action here\n");
#endif
		return (struct recvbuf *)0;	/* nothing has arrived */
	}
	
	/*
	 * Get the fulllist chain and mark it empty
	 */
#ifdef DEBUG
	if (debug > 4)
		printf("getrecvbufs returning %d buffers\n", full_recvbufs);
#endif
	rb = fulllist;
	fulllist = 0;
	full_recvbufs = 0;

	/*
	 * Check to see if we're below the low water mark.
	 */
	if (free_recvbufs <= RECV_LOWAT) {
		register struct recvbuf *buf;
		register int i;

		if (total_recvbufs >= RECV_TOOMANY)
			syslog(LOG_ERR, "too many recvbufs allocated (%d)",
			    total_recvbufs);
		else {
			buf = (struct recvbuf *)
			    emalloc(RECV_INC*sizeof(struct recvbuf));
			for (i = 0; i < RECV_INC; i++) {
				buf->next = freelist;
				freelist = buf;
				buf++;
			}

			free_recvbufs += RECV_INC;
			total_recvbufs += RECV_INC;
			lowater_additions++;
		}
	}

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
sendpkt(dest, inter, pkt, len)
	struct sockaddr_in *dest;
	struct interface *inter;
	struct pkt *pkt;
	int len;
{
	int cc;
	extern char *ntoa();

#ifdef DEBUG
	char *ntoa();

	if (debug)
		printf("sendpkt(%s, %s, %d)\n", ntoa(dest),
			ntoa(&inter->sin), len);
#endif
		
	cc = sendto(inter->fd, (char *)pkt, len, 0, (struct sockaddr *)dest,
	    sizeof(struct sockaddr_in));
	if (cc == -1) {
		inter->notsent++;
		packets_notsent++;
		if (errno != EWOULDBLOCK && errno != ENOBUFS)
			syslog(LOG_ERR, "sendto(%s): %m", ntoa(dest));
	} else {
		inter->sent++;
		packets_sent++;
	}
}


/*
 * input_handler - receive packets asynchronously
 */
void
input_handler()
{
	register int i, n;
	register struct recvbuf *rb;
	register int doing;
	register int fd;
	struct timeval tvzero;
	int fromlen;
	l_fp ts;
	fd_set fds;
	extern void receive();

	handler_calls++;

	/*
	 * Do a poll to see who has data
	 */
again:
	fds = activefds;
	tvzero.tv_sec = tvzero.tv_usec = 0;
	n = select(maxactivefd+1, &fds, (fd_set *)0, (fd_set *)0, &tvzero);

	/*
	 * If nothing to do, just return.  If an error occurred, complain
	 * and return.  If we've got some, freeze a timestamp.
	 */
	if (n == 0)
		return;
	else if (n == -1) {
		syslog(LOG_ERR, "select() error: %m");
		return;
	}
	get_systime(&ts);
	handler_pkts += n;

#ifdef REFCLOCK
	/*
	 * Check out the reference clocks first, if any
	 */
	if (refio != 0) {
		register struct refclockio *rp;

		for (rp = refio; rp != 0 && n > 0; rp = rp->next) {
			fd = rp->fd;
			if (FD_ISSET(fd, &fds)) {
				n--;
				if (free_recvbufs == 0) {
					char buf[RX_BUFF_SIZE];

					(void) read(fd, buf, sizeof buf);
					packets_dropped++;
					continue;
				}

				rb = freelist;
				freelist = rb->next;
				free_recvbufs--;

				i = (rp->datalen == 0
				    || rp->datalen > sizeof(rb->recv_space))
				    ? sizeof(rb->recv_space) : rp->datalen;
				
				rb->recv_length = read(fd,
				    (char *)&rb->recv_space, i);

				if (rb->recv_length == -1) {
					syslog(LOG_ERR, "clock read: %m");
					rb->next = freelist;
					freelist = rb;
					free_recvbufs++;
					continue;
				}
	
				/*
				 * Got one.  Mark how and when it got here,
				 * put it on the full list and do bookkeeping.
				 */
				rb->recv_srcclock = rp->srcclock;
				rb->dstadr = 0;
				rb->recv_time = ts;

				rb->receiver = rp->clock_recv;

				rb->next = fulllist;
				fulllist = rb;
				full_recvbufs++;

				rp->recvcount++;
				packets_received++;
			}
		}
	}
#endif

	/*
	 * Loop through the interfaces looking for data to read.
	 */
	for (i = ninterfaces-1; i >= 0 && n > 0; i--) {
		for (doing = 0; doing < 2 && n > 0; doing++) {
			if (doing == 0) {
				fd = inter_list[i].fd;
			} else {
				if (!(inter_list[i].flags & INT_BCASTOPEN))
					break;
				fd = inter_list[i].bfd;
			}
			if (FD_ISSET(fd, &fds)) {
				n--;
				/*
				 * Get a buffer and read the frame.  If we
				 * haven't got a buffer, or this is received
				 * on the wild card socket, just dump the packet.
				 */
				if (i == 0 || free_recvbufs == 0) {
					char buf[RX_BUFF_SIZE];

					(void) read(fd, buf, sizeof buf);
					if (i == 0)
						packets_ignored++;
					else
						packets_dropped++;
					continue;
				}
	
				rb = freelist;
				freelist = rb->next;
				free_recvbufs--;
	
				fromlen = sizeof(struct sockaddr_in);
				rb->recv_length = recvfrom(fd,
				    (char *)&rb->recv_space,
				    sizeof(rb->recv_space), 0,
				    (struct sockaddr *)&rb->recv_srcadr,
				    &fromlen);
				if (rb->recv_length == -1) {
					syslog(LOG_ERR, "recvfrom: %m");
					rb->next = freelist;
					freelist = rb;
					free_recvbufs++;
					continue;
				}
	
				/*
				 * Got one.  Mark how and when it got here,
				 * put it on the full list and do bookkeeping.
				 */
				rb->dstadr = &inter_list[i];
				rb->recv_time = ts;
				rb->receiver = receive;
	
				rb->next = fulllist;
				fulllist = rb;
				full_recvbufs++;
	
				inter_list[i].received++;
				packets_received++;
			}
		}
	}
	/*
	 * Done everything from that select.  Poll again.
	 */
	goto again;
}


/*
 * findinterface - utility used by other modules to find an interface
 *		   given an address.
 */
struct interface *
findinterface(addr)
	struct sockaddr_in *addr;
{
	register int i;
	register u_int saddr;

	/*
	 * Just match the address portion.
	 */
	saddr = addr->sin_addr.s_addr;
	for (i = 0; i < ninterfaces; i++) {
		if (inter_list[i].sin.sin_addr.s_addr == saddr)
			return &inter_list[i];
	}
	return (struct interface *)0;
}


/*
 * io_clr_stats - clear I/O module statistics
 */
void
io_clr_stats()
{
	packets_dropped = 0;
	packets_ignored = 0;
	packets_received = 0;
	packets_sent = 0;
	packets_notsent = 0;

	handler_calls = 0;
	handler_pkts = 0;
	io_timereset = current_time;
}


#ifdef REFCLOCK
/*
 * io_addclock - add a reference clock to the list and arrange that we
 *		 get SIGIO interrupts from it.
 */
int
io_addclock(rio)
	struct refclockio *rio;
{
	int osig;

	
	/*
	 * The file descriptor is assumed to be open and more or
	 * less set up.  Make it non-blocking and arrange for the
	 * delivery of SIGIO interrupts to us.
	 */
/*
	if (fcntl(rio->fd, F_SETOWN, getpid()) == -1) {
		syslog(LOG_ERR, "fcntl(F_SETOWN) fails for clock I/O: %m");
		return 0;
	}
*/
	BLOCKIO(osig);
	/*
	 * set non-blocking, async I/O on the descriptor
	 */
	if (fcntl(rio->fd, F_SETFL, FNDELAY|FASYNC) < 0) {
		syslog(LOG_ERR,
		    "fcntl(FNDELAY|FASYNC) fails for clock I/O: %m");
		return 0;
	}

	/*
	 * Stuff the I/O structure in the list and mark the descriptor
	 * in use.  There is a harmless (I hope) race condition here.
	 */
	rio->next = refio;
	refio = rio;

	if (rio->fd > maxactivefd)
		maxactivefd = rio->fd;
	FD_SET(rio->fd, &activefds);
	UNBLOCKIO(osig);

	return 1;
}


/*
 * io_closeclock - close the clock in the I/O structure given
 */
void
io_closeclock(rio)
	struct refclockio *rio;
{
	/*
	 * Remove structure from the list
	 */
	if (refio == rio) {
		refio = rio->next;
	} else {
		register struct refclockio *rp;

		for (rp = refio; rp != 0; rp = rp->next)
			if (rp->next == rio) {
				rp->next = rio->next;
				break;
			}
		
		if (rp == 0) {
			/*
			 * Internal error.  Report it.
			 */
			syslog(LOG_ERR,
			    "internal error: refclockio structure not found");
			return;
		}
	}

	/*
	 * Close the descriptor.  close_socket does the right thing despite
	 * the misnomer.
	 */
	close_socket(rio->fd);
}
#endif

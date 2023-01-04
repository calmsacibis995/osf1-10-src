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
static char *rcsid = "@(#)$RCSfile: gw_screen.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:41:47 $";
#endif

/*
 *	16 December 1988	Jeffrey Mogul/DECWRL
 *		Created.
 *	Copyright (c) 1988 by Digital Equipment Corporation
 */

/*
 * Gateway Screening mechanism
 */

#include "sys/errno.h"
#include "sys/ioctl.h"
#include "sys/kernel.h"
#include "sys/mbuf.h"
#include "sys/param.h"
#include "sys/proc.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/time.h"
#include "sys/user.h"
#include "net/gw_screen.h"
#include "net/net_globals.h"
#include "mach/vm_param.h"	/* for round_page() */
#include "vm/vm_kern.h"		/* for kmem_alloc() and kernel_map */

int gw_mustberoot = 1; 	
int gw_screen_on = 0;		/* Screening on, if true */

struct screen_stats screen_stats;	/* statistics */

int screen_list_initialized = 0;

#if NETSYNC_LOCK
simple_lock_data_t gwscreen_lock;
#define SCREEN_LOCKINIT()	simple_lock_init(&gwscreen_lock)
#define SCREEN_LOCK()		simple_lock(&gwscreen_lock)
#define SCREEN_UNLOCK()		simple_unlock(&gwscreen_lock)
#else
#define SCREEN_LOCKINIT()
#define SCREEN_LOCK()
#define SCREEN_UNLOCK()
#endif

/*
 * Access to this facility should be by a new system call, but
 * to keep things simple, we use several new ioctls instead.
 * screen_control() is called from ifioctl().
 */

screen_control(so, cmd, data)
	struct socket *so;
	int cmd;
	caddr_t data;
{
	int old_screen_on, new_screen_on;
	int error = 0;
	struct screen_data *sdp = (struct screen_data *)data;

	if (!screen_list_initialized) 	/* serious configuration error? */
	    return(ENOBUFS);

	switch (cmd) {

	case SIOCSCREENON:
		/*
		 * Turns screening on/off (based on arg)
		 * and returns previous setting (value-result arg)
		 */
		new_screen_on = *((int *)data);
		old_screen_on = gw_screen_on;
		switch (new_screen_on) {
		    case SCREENMODE_OFF:
		    case SCREENMODE_ON:
			if (gw_mustberoot && (so->so_state & SS_PRIV) == 0) {
			    return(EACCES);
			}
			gw_screen_on = new_screen_on;
			break;

		    case SCREENMODE_NOCHANGE:
		    default:
			/* no change */
			break;
		}

		/* return old value in any case */
		*((int *)data) = old_screen_on;
		break;

	case SIOCSCREEN:
		/*
		 * Transmits from user to kernel a screen_data
		 * struct, and then copies the next one from kernel
		 * to user into the same buffer (value-result arg).
		 * This allows us to do each transaction with
		 * only one system call.
		 *
		 * This ioctl blocks until the next transaction
		 * is available.
		 */

		if (gw_mustberoot && (so->so_state & SS_PRIV) == 0) {
		    return(EACCES);
		}

		if (!gw_screen_on) {
			return(ENOPROTOOPT);
		}

		/* Honor user's wishes on previous packet */
		screen_disposition(sdp->sd_xid, sdp->sd_action);
		
		/* Wait for next packet, create next transaction */
		if (error = screen_getnext(sdp))
			return(error);
		break;

	case SIOCSCREENSTATS:
		/*
		 * Provides current statistics block
		 */
		*(struct screen_stats *)data = screen_stats;
		break;
		
	default:
		printf("screen_control: unknown cmd %x\n", cmd);
		panic("screen_control");
	}

	return(0);
}

/*
 * This procedure is called, for example,
 *	instead of ip_forward() from ipintr().
 */
void
gw_forwardscreen(pkt, srcrt, af, fwdproc, errorproc)
	struct mbuf *pkt;
	int srcrt;
	int af;
	void (*fwdproc)();
	void (*errorproc)();
{
	if (gw_screen_on == SCREENMODE_OFF) {	/* not our problem */
		(*fwdproc)(pkt, srcrt);
	}
	else {
		screen_bufenter(pkt, srcrt, af, fwdproc, errorproc);
	}
}

/*
 * Unprocessed packets kept on a list
 */

extern int screen_maxpend;	/* max # of packets pending */

struct screen_listentry {
	struct screen_listentry *next;	/* forward pointer */
	struct screen_listentry *prev;	/* backward pointer */
	u_int	xid;		/* transaction id */
	struct mbuf *pkt;	/* pointer to the packet */
	int srcrt;		/* argument to ip_forward */
	pid_t	pid;		/* "owning" process, if nonzero */
	struct timeval arrival;	/* arrival time for the packet */
	short	family;		/* address family */
	void	(*fwdproc)();	/* forwarding action (takes pkt, srcrt) */
	void	(*errorproc)();	/* error action (takes pkt, srcrt) */
};

struct screen_listentry screen_pending;		/* queue header */
struct screen_listentry screen_free;		/* free queue header */

u_int	screen_next_xid = 99;	/* next transaction id */

/*
 * Called from netinit() if gwscreen pseudo-device is configured.
 * If gwscreen is not configured, netinit() calls the empty function
 * defined in gw_screen_data.c.
 * Allocates permanent kernel memory based on SCREEN_MAXPEND specified
 * in gw_screen_data.c.  Also initializes gwscreen lock.
 */
void
gw_screen_init()
{
	register struct screen_listentry *sclp;
	register int i;
	register int allocsize = 
		screen_maxpend * sizeof(struct screen_listentry);

	/* extra serious configuration error */
	if (screen_maxpend < 1)
		return;

	screen_pending.next = &screen_pending;
	screen_pending.prev = &screen_pending;

	screen_free.next = &screen_free;
	screen_free.prev = &screen_free;

	sclp = (struct screen_listentry *)kmem_alloc(kernel_map, 
			round_page(allocsize));

	if (sclp == NULL)
		return;		/* we COULD try again later */

	/* Chop up the memory and insert it on the free queue */
	for (i = 0; i < screen_maxpend; i++) {
		insque(sclp, &screen_free);
		sclp++;
	}
	screen_list_initialized = 1;
	SCREEN_LOCKINIT();
}

/*
 * Unprocessed packets go onto the pending list.  When the list
 * is full, we drop incoming packets (I think this is the "right
 * thing" to do, based on the way congestion control algorithms
 * work).  We wake up any available screening processes.
 */
void
screen_bufenter(pkt, srcrt, family, fwdproc, errorproc)
struct mbuf *pkt;
int srcrt;
int family;
void (*fwdproc)();
void (*errorproc)();
{
	register struct screen_listentry *sclp;
	int s;

	s = splimp();
	SCREEN_LOCK();

	screen_stats.ss_packets++;

	/* get next free listentry */
	if ((sclp = screen_free.next) == &screen_free) {
	    screen_stats.ss_nobuffer++;
	    m_freem(pkt);			/* drop packet */
	    SCREEN_UNLOCK();
	    (void)splx(s);
	    wakeup((caddr_t)&screen_pending);	/* just in case */
	    return;
	}
	remque(sclp);
	
	sclp->xid = screen_next_xid++;
	sclp->pkt = pkt;
	sclp->srcrt = srcrt;
	sclp->pid = 0;
	sclp->arrival = time;
	sclp->family = family;
	sclp->fwdproc = fwdproc;
	sclp->errorproc = errorproc;

	/* link this on onto pending list */
	insque(sclp, &screen_pending);
	
	SCREEN_UNLOCK();
	(void)splx(s);

	/* notify waiting screen processes */
	wakeup((caddr_t)&screen_pending);
}

/*
 * Block until something is there, then mark entry as "owned"
 * and return the contents.
 */
int
screen_getnext(sdp)
register struct screen_data *sdp;
{
	int s;
	register struct screen_listentry *sclp;
	register struct mbuf *m;
	register int remlen;
	register int len;
	int error = 0;
	register char *dp;

	s = splimp();
	SCREEN_LOCK();
	while (1) {
    
	    /* search buffer for un-owned entry, FIFO order */
	    sclp = screen_pending.prev;
	    while (sclp != &screen_pending) {
		/* if not claimed and family matches or is wildcarded */
		if (sclp->pid == 0
			&& ((sdp->sd_family == sclp->family)
				|| (sdp->sd_family == AF_UNSPEC)) ) {
		   goto found;
		}
		sclp = sclp->prev;
	    }

	    /* buffer is empty */

	    screen_purgeold();	/* get rid of stale entries */
    
	    /* mpsleep() relocks if successful */
#if NETSYNC_LOCK
	    if (error = mpsleep((caddr_t)&screen_pending, PUSER|PCATCH,
		             "gwscreen", 0, &gwscreen_lock, MS_LOCK_SIMPLE))
		return(error);
#else
	    if (error = tsleep((caddr_t)&screen_pending, PUSER|PCATCH,  
                             "gwscreen", 0)) 
		return(error);
#endif
			    /* this sleep is interruptible */
	}

found:
	sclp->pid = u.u_procp->p_pid;
	sdp->sd_xid = sclp->xid;
	sdp->sd_arrival = sclp->arrival;
	sdp->sd_family = sclp->family;
	m = sclp->pkt;

	/* copy packet header into sd_data */
	remlen = SCREEN_DATALEN;
	dp = sdp->sd_data;
	while (m && (remlen > 0)) {
	    len = min(remlen, m->m_len);
	    bcopy(mtod(m, caddr_t), dp, len);
	    dp += len;
	    remlen -= len;
	    m = m->m_next;
	}
	
	sdp->sd_count = sizeof(struct screen_data);
	sdp->sd_dlen = dp - sdp->sd_data;
	sdp->sd_action = SCREEN_DROP|SCREEN_NONOTIFY;

	SCREEN_UNLOCK();
	(void)splx(s);
	return(0);
}

void
screen_disposition(xid, action)
register u_int xid;
int action;
{
	int s;
	register struct screen_listentry *sclp;
	register pid_t mypid = u.u_procp->p_pid;

	s = splimp();
	SCREEN_LOCK();

	/* search for our current transaction; flush stale ones */
	sclp = screen_pending.prev;
	while (sclp != &screen_pending) {
		if (sclp->pid == mypid) {
			remque(sclp);
			if (sclp->xid == xid) {
				/* match */
				goto found;
			}
			else {
				/* stale, flush it */
				m_freem(sclp->pkt);
				insque(sclp, &screen_free);
				screen_stats.ss_badsync++;
				sclp = screen_pending.prev;
					/* rescan is slow but simple */
			}
		}
		else
			sclp = sclp->prev;
	}
	SCREEN_UNLOCK();
	(void)splx(s);
	return;		/* nothing to dispose of */

found:
	if (action & SCREEN_ACCEPT) {
	    screen_stats.ss_accept++;
	    SCREEN_UNLOCK();
	    (void)splx(s);
	    (*(sclp->fwdproc))(sclp->pkt, sclp->srcrt);
		/* this frees sclp->pkt */
	} else {
	    /* this packet is rejected */
	    screen_stats.ss_reject++;
	    SCREEN_UNLOCK();
	    (void)splx(s);
    
	    if (action & SCREEN_NOTIFY) {
		(*(sclp->errorproc))(sclp->pkt, sclp->srcrt);
		/* this frees sclp->pkt */
	    }
	    else
		m_freem(sclp->pkt);
	}

	s = splimp();
	SCREEN_LOCK();
	insque(sclp, &screen_free);
	SCREEN_UNLOCK();
	(void)splx(s);
}

#define	SCREEN_MAXAGE	5	/* default maximum age for packets */

int	screen_maxage = SCREEN_MAXAGE;

/*
 * Free up entries on the pending queue that are more than
 * screen_maxage seconds old.
 *
 * ASSUMPTION: called at high IPL, with lock held
 */
void
screen_purgeold()
{
	register int cutoff;
	register struct screen_listentry *sclp;
	
	/* Calculate oldest legal age; grace period of 1 sec for roundoff */
	cutoff = (time.tv_sec - screen_maxage) - 1;
	
	sclp = screen_pending.next;
	while (sclp != &screen_pending) {
		if (sclp->arrival.tv_sec < cutoff) {
			screen_stats.ss_stale++;
			remque(sclp);
			m_freem(sclp->pkt);
			insque(sclp, &screen_free);
			sclp = screen_pending.next;
		}
		else
			sclp = sclp->next;
	}
}

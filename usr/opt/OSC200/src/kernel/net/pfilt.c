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
static char *rcsid = "@(#)$RCSfile: pfilt.c,v $ $Revision: 1.1.7.6 $ (DEC) $Date: 1994/01/11 22:45:01 $";
#endif

/*
 * Based on:
 * static  char    *RCSid = "@(#)pfilt.c  6.3	(ULTRIX)	4/29/92";
 */
/*********************************************************************
 *  Ethernet packet filter layer,
 *
 *		- pfilt.c -
 **********************************************************************
 */

#define	MAXUNITS	16	/* Maximum number of interfaces supported */

#include "packetfilter.h"

#if PACKETFILTER > 0
#define BPF  1	/* compile in BPF compatibility */
#endif

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/mbuf.h"
#include "sys/buf.h"
#include "sys/user.h"
#include "sys/kernel.h"
#include "sys/ioctl.h"
#include "sys/map.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/uio.h"
#include "mach/vm_param.h"	/* for round_page() */
#include "vm/vm_kern.h"		/* for kmem_alloc() and kernel_map */

#include "sys/protosw.h"
#include "sys/socket.h"
#include "net/if.h"
#include "netinet/in.h"
#include "netinet/if_ether.h"
#include "netinet/if_fddi.h"
#include "net/ether_driver.h"
#include "sys/types.h"
#include "sys/time.h"
#include "sys/poll.h"		/* select support */

#ifdef SEC_BASE
#include <sys/security.h>	/* privileged() replaces suser() */
#endif

#undef	queue
#undef	dequeue

#ifdef	BPFTEST
#include "../bpf_stuff/pfilt.h" 
#ifdef	BPF
#include "../bpf_stuff/bpf.h"
#endif
#include "../bpf_stuff/pfiltdefs.h"
#else
#include "net/pfilt.h"
#ifdef	BPF
#include "net/bpf.h"
#endif
#include "net/pfiltdefs.h"
#endif	/* BPFTEST */

#if RT_PREEMPT
simple_lock_data_t	pf_slock; /* packetfilter lock for RT option */
#define PF_LOCKINIT()	simple_lock_init(&pf_slock)
#define PF_LOCK()	simple_lock(&pf_slock)
#define PF_UNLOCK()	simple_unlock(&pf_slock)
#else
#define PF_LOCKINIT()
#define PF_LOCK()
#define PF_UNLOCK()
#endif

#ifndef SMP_SUPPORT
/* SMP fake defines, to get compiled for now */
int smp_debug=0;
int smp=0;
#define smp_owner(addr) 1
#endif

#define	NPACKETFILTER	256		/* maximum number of minor devices */

/*#define DEBUG	1*/
/*#define INNERDEBUG 1*/	/* define only when debugging PfiltDoFilter()
					or PfiltInputDone()  */

#define pfiltenqueue(head, elt)\
{ (elt)->F = (head);(elt)->B = (head)->B;(head)->B = (elt);((elt)->B)->F = (elt);};

#define pfiltinitqueue(head)\
    { (head)->F = (head); (head)->B = (head); };

#define pfiltdequeue(head, elt)\
{\
struct Queue *q_tmp1, *q_tmp2;\
q_tmp1 = q_tmp2  = (head)->F;\
q_tmp2->B->F = q_tmp2->F;\
q_tmp2->F->B = q_tmp2->B;\
(elt) = (q_tmp1 == (head)) ? ((struct enPacket *)0):\
(struct enPacket *)q_tmp2;\
};

int pfactive=0;		/* number of currently active filters */

/* #define	DEBUG_ER 1  */

#ifdef	DEBUG_ER
int er_debug=0;
#endif

#define	Pfiltprintf(flags)	if (enDebug&(flags)) printf

/*
 * Symbolic definitions for enDebug flag bits
 *	ENDBG_TRACE should be 1 because it is the most common
 *	use in the code, and the compiler generates faster code
 *	for testing the low bit in a word.
 */

#define	ENDBG_TRACE	1	/* trace most operations */
#define	ENDBG_DESQ	2	/* trace descriptor queues */
#define	ENDBG_INIT	4	/* initialization info */
#define	ENDBG_SCAV	8	/* scavenger operation */
#define	ENDBG_ABNORM	16	/* abnormal events */
#define	ENDBG_NOMEM	32	/* could not allocate an mbuf */
#define	ENDBG_MISC	64	/* sleep intrs, uiomov size mismatch, etc. */


#define min(a,b)	( ((a)<=(b)) ? (a) : (b) )

#define	splenet	splimp	/* used to be spl6 but I'm paranoid */

#define PRINET  26			/* interruptible */

/*
 *  'enQueueElts' is the pool of packet headers used by the driver.
 *  'enPackets'   is the pool of packets used by the driver (these should
 *		  be allocated dynamically when this becomes possible).
 *  'enFreeq'     is the queue of available packets
 *  'enStatePs'   is the driver state table per logical unit number
 *			(a vector of pointers to state records)
 *  'enUnit'  	  is the physical unit number table per logical unit number;
 *		  the first "attach"ed ethernet is logical unit 0, etc.
 *  'enUnitMap'	  maps minor device numbers onto interface unit #s
 *  'enAllocMap'  indicates if minor device is allocated or free
 *  'enAllDescriptors' stores OpenDescriptors, indexed by minor device #
 *  'enFreeqMin'  is the minimum number of packets ever in the free queue
 *		  (for statistics purposes)
 *  'enScavenges' is the number of scavenges of the active input queues
 *		  (for statustics purposes)
 *  'enScavDrops' is the number of packets dropped during scavenging
 *		  (for statustics purposes)
 *  'enDebug'	  is a collection of debugging bits which enable trace and/or
 *		  diagnostic output as defined above (ENDBG_*)
 *  'enUnits'	  is the number of attached units
 */
struct mbuf *pf_mcopym();
struct enPacket	enQueueElts[ENPACKETS];
struct enQueue	enFreeq;
struct enState *enStatePs[MAXUNITS];
char		enUnitMap[NPACKETFILTER];
char		enAllocMap[NPACKETFILTER];
struct enOpenDescriptor
		*enAllDescriptors[NPACKETFILTER];
int		enFreeqMin = ENPACKETS;
int		enScavenges = 0;
int		enScavDrops = 0;
int		enDebug = ENDBG_ABNORM;
int		enUnits = 0;
int		enMaxMinors = NPACKETFILTER;

/*
 *  Forward declarations for subroutines which return other
 *  than integer types.
 */
extern boolean PfiltDoFilter();


/*
 * Linkages to if_XXXXX.c
 */

struct enet_info {
	struct	ifnet *ifp;	/* which ifp for output */
};

struct enet_info enet_info[MAXUNITS];

struct sockaddr enetaf = { 0, AF_IMPLINK };

/*
 * Used in filter processing to avoid having to do an m_pullup
 */
struct enpextent {
	long	afterEnd;	/* pkt offset of first byte after extent */
	long	offset;		/* pkt offset of first byte of extent */
					/* LONG SIGNED TO MAKE IT RUN FAST */
	char   *data;		/* memory address of first byte of extent */
};

#define	MAX_EXTENTS	16	/* enough for an ether pkt in mbufs */
#define	MAX_OFFSET	(2<<ENF_NBPA)
				/* guaranteed to be larger than any offset
				   that can be embedded in a filter */


/* #define	SELF_PROF	1 */
#ifdef	SELF_PROF
int	enSelfProf = 0;		/* set this non-zero for profiling */
struct	timeval enPerPktProf = {0, 0};
int	enPktCount = 0;
struct	timeval enPerFiltProf = {0, 0};
int	enFiltCount = 0;
struct	timeval enCalibrateProf = {0, 0};
int	enCalibrateCount = 0;
#endif	/* SELF_PROF */


/****************************************************************
 *								*
 *		Various Macros & Routines			*
 *								*
 ****************************************************************/

/*
 *  forAllOpenDescriptors(p) -- a macro for iterating
 *  over all currently open devices.  Use it in place of
 *      "for ( ...; ... ; ... )"
 *  and supply your own loop body.  The loop variable is the
 *  parameter p which is set to point to the descriptor for
 *  each open device in turn.
 */

#define forAllOpenDescriptors(p)					\
	for ((p) = (struct enOpenDescriptor *)enDesq.enQ_F;		\
	      (struct Queue *)(&enDesq) != &((p)->enOD_Link);		\
	      (p) = (struct enOpenDescriptor *)(p)->enOD_Link.F)

/*
 *  enEnqueue - add an element to a queue
 */

#define	PfiltEnqueue(q, elt)						\
{									\
	pfiltenqueue((struct Queue *)(q), (struct Queue *)(elt));		\
	(q)->enQ_NumQueued++;						\
}

/*
 *  PfiltFlushQueue - release all packets from queue, freeing any
 *  whose reference counts drop to 0.  Assumes caller
 *  is at high IPL so that queue will not be modified while
 *  it is being flushed.
 */

PfiltFlushQueue(q)
register struct enQueue *q;
{

   register struct enPacket *qelt;

    PF_LOCK();
    pfiltdequeue((struct Queue *)q, qelt);
    /* "qelt" is filled in at the end of the call pfiltdequeue */

    while(qelt != NULL)
    {
	if (0 == --(qelt->enP_RefCount))
	{
	    PfiltEnqueue(&enFreeq, qelt);
	}
	pfiltdequeue((struct Queue *)q, qelt);
    }
    q->enQ_NumQueued = 0;
    PF_UNLOCK();

}

/*
 *  PfiltInitWaitQueue - initialize an empty packet wait queue
 */

PfiltInitWaitQueue(wq)
register struct enWaitQueue *wq;
{			

    wq->enWQ_Head = 0;
    wq->enWQ_Tail = 0;
    wq->enWQ_NumQueued = 0;
    wq->enWQ_MaxWaiting = ENDEFWAITING;

}

/*
 *  PfiltEnWaitQueue - add a packet to a wait queue
 *  SMP: assumes lock held coming in
 */

PfiltEnWaitQueue(wq, p)
register struct enWaitQueue *wq;
struct enPacket *p;
{
    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltEnWaitQueue: not lock owner");
    }
    wq->enWQ_Packets[wq->enWQ_Tail] = p;
    wq->enWQ_NumQueued++;
    PfiltNextWaitQueueIndex(wq->enWQ_Tail);
}

/*
 *  PfiltDeWaitQueue - remove a packet from a wait queue
 *  SMP: assumes lock held coming in
 */

struct enPacket *
PfiltDeWaitQueue(wq)
register struct enWaitQueue *wq;
{

    struct enPacket *p;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltDeWaitQueue: not lock owner");
    }
    if (wq->enWQ_NumQueued < 1)
    	panic("PfiltDeWaitQueue");
    wq->enWQ_NumQueued--;
    p = wq->enWQ_Packets[wq->enWQ_Head];
    PfiltNextWaitQueueIndex(wq->enWQ_Head);
    return(p);
}

/*
 *  PfiltPutBack - undo the effect of PfiltDeWaitQueue; MUST be called
 *	immediately after corresponding PfiltDeWaitQueue with no
 *	intervening interrupts!
 *  SMP: assumes lock held coming in
 */
PfiltPutBack(wq, p)
register struct enWaitQueue *wq;
struct enPacket *p;
{
    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltPutBack: not lock owner");
    }
    PfiltPrevWaitQueueIndex(wq->enWQ_Head);
    wq->enWQ_Packets[wq->enWQ_Head] = p;
    wq->enWQ_NumQueued++;
}

/*
 *  PfiltTrimWaitQueue - cut a wait queue back to size
 *  SMP: assumes lock held coming in
 */
PfiltTrimWaitQueue(d, threshold)
register struct enOpenDescriptor *d;
int threshold;
{
    register struct enWaitQueue *wq = &(d->enOD_Waiting);
    register int Counter = (wq->enWQ_NumQueued - threshold);
    register struct enPacket *p;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltTrimWaitQueue: not lock owner");
    }
#ifdef	DEBUG
    Pfiltprintf(ENDBG_SCAV)
    		("PfiltTrimWaitQueue(%x, %d): %d\n", d, threshold, Counter);
#endif
    while (Counter-- > 0)
    {
	wq->enWQ_NumQueued--;
	PfiltPrevWaitQueueIndex(wq->enWQ_Tail);
	p = wq->enWQ_Packets[wq->enWQ_Tail];
	d->enOD_Drops += (1 + p->enP_Stamp.ens_dropped);
	    /* dropped this packet and it might have recorded other drops. */
	enScavDrops++;
	if (0 == --(p->enP_RefCount))
	{
	    m_freem(p->enP_mbuf);
	    PfiltEnqueue(&enFreeq, p);
	}
    }
}
/*
 *  PfiltFlushWaitQueue - remove all packets from wait queue
 */
#define	PfiltFlushWaitQueue(wq)	PfiltTrimWaitQueue(wq, 0)

/*
 *  scavenging thresholds:
 *
 *  index by number of active files;  for N open files, each queue may retain
 *  up to 1/Nth of the packets not guaranteed to be freed on scavenge.  The
 *  total number of available packets is computed less one for sending.
 *  (assumes high IPL)
 */
unsigned short enScavLevel[NPACKETFILTER+1];

/*
 *  PfiltInitScavenge -- set up ScavLevel table
 */
PfiltInitScavenge()
{
    register int PoolSize = (ENPACKETS-ENMINSCAVENGE);
    register int i = sizeof(enScavLevel)/sizeof(enScavLevel[0]);

    PoolSize--;		/* leave one for transmitter */
    while (--i>0)
	enScavLevel[i] = (PoolSize / i);
}

/*
 *  PfiltScavenge -- scan all OpenDescriptors for all ethernets, releasing
 *    any queued buffers beyond the prescribed limit and freeing any whose
 *    refcounts drop to 0.
 *    Assumes caller is at high IPL so that it is safe to modify the queues.
 *    SMP: assumes lock held coming in
 */
PfiltScavenge()
{

    register struct enOpenDescriptor *d;
    register int threshold = 0;
    register struct enState *enStatep;
    register int unit;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltScavenge: not lock owner");
    }
    for (unit = 0; unit < enUnits; unit++) {
	enStatep = enStatePs[unit];
	threshold += enCurOpens;
    }
    threshold = enScavLevel[threshold];

    enScavenges++;
#ifdef	DEBUG
    Pfiltprintf(ENDBG_SCAV)("PfiltScavenge: %d\n", threshold);
#endif
    for (unit = 0; unit < enUnits; unit++) {
	enStatep = enStatePs[unit];
	if (enDesq.enQ_F == 0)
	    continue;			/* never initialized */
	forAllOpenDescriptors(d)
	{
	    PfiltTrimWaitQueue(d, threshold);
	}
    }
}

/*
 *  PfiltAllocatePacket - allocate the next packet from the free list
 *  Assumes IPL is at high priority so that it is safe to touch the
 *  packet queue.  If the queue is currently empty, scavenge for
 *  more packets.
 *  SMP: assumes lock held coming in
 */

struct enPacket *
PfiltAllocatePacket()
{

    register struct enPacket *p;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltAllocatePacket: not lock owner");
    }
    if (0 == enFreeq.enQ_NumQueued)
	PfiltScavenge();

    pfiltdequeue((struct Queue *)&enFreeq, p);

   /* "p" is filled in at the end of the call, pfiltdequeue */
    if (p == NULL)
	panic("PfiltAllocatePacket");
    if (enFreeqMin > --enFreeq.enQ_NumQueued)
	enFreeqMin = enFreeq.enQ_NumQueued;

    p->enP_RefCount = 0;	/* just in case */

    return(p);

}

/*
 *  PfiltDeallocatePacket - place the packet back on the free packet queue
 *  (High IPL assumed).
 */

#define	PfiltDeallocatePacket(p)						\
{									\
	if (p->enP_RefCount) panic("PfiltDeallocatePacket: refcount != 0");\
	pfiltenqueue((struct Queue *)&enFreeq, (struct Queue *)(p));		\
	enFreeq.enQ_NumQueued++;					\
}

/****************************************************************
 *								*
 *	    Routines to move uio data to/from mbufs		*
 *								*
 ****************************************************************/

/*
 * These two routines were inspired by/stolen from ../sys/uipc_socket.c
 *	   Both return error code (or 0 if success).
 */

/*
 * read: return contents of mbufs to user.  DO NOT free them, since
 *	there may be multiple claims on the packet!
 */
Pfiltrmove(p, uio, flags, padneeded, truncation)
struct enPacket *p;
register struct uio *uio;
short flags;
int *padneeded;			/* by reference, so we can change it */
int truncation;
{
	register int len;
	register int count;
	register int error = 0;
	register struct mbuf *m = p->enP_mbuf;
	int total = 0;
	int pn = *padneeded;
	
	/* first, transfer header stamp */
	if (flags & (ENTSTAMP|ENBATCH)) {
	    if (uio->uio_resid >= (sizeof(struct enstamp) + pn)) {
		error = uiomove((caddr_t)&(p->enP_Stamp) - pn,
				    sizeof(struct enstamp) + pn, uio);
		if (error) {
		    return(error);
		}
	    }
	    else {	/* no room for stamp */
	    	uio->uio_resid = 0;	/* make sure Pfiltread() exits */
		return(EMSGSIZE);
	    }
	}

	count = min(p->enP_ByteCount, uio->uio_resid); /* # bytes to return */
	if (count > truncation)
	    count = truncation;
	while ((count > 0) && m && (error == 0)) {
	    len = min(count, m->m_len);	/* length of this transfer */
	    count -= len;
	    total += len;
	    error = uiomove(mtod(m, caddr_t), (int)len, uio);
	    
	    m = m->m_next;
	}
#if	DEBUG
	if ((error == 0) && (total != p->enP_ByteCount) &&
		(truncation > p->enP_ByteCount) &&
		(uio->uio_resid != 0)) {
	    Pfiltprintf(ENDBG_MISC)("Pfiltrmove: %d != %d (resid %d)\n",
					total, p->enP_ByteCount,
					uio->uio_resid);
	}
#endif
	/*
	 * record how much padding is needed if another packet is batched
	 * following this one in the same read()
	 */
	if ((total &= ENALIGNMASK) == 0) {
	    *padneeded = 0;	/* already aligned */
	}
	else {
	    *padneeded = ENALIGNMENT - total;
	}

	return(error);
}

Pfiltwmove(uio, mbufp, pad)
register struct uio *uio;
register struct mbuf **mbufp;	/* top mbuf is returned by reference */
int pad;
{
	struct mbuf *mtop = 0;
	register struct mbuf *m;
	register struct mbuf **mp = &mtop;
	register struct iovec *iov;
	register int len;
	int error = 0;
	int first=1;
	int totlen=0;
	
	while ((uio->uio_resid > 0) && (error == 0)) {
	    iov = uio->uio_iov;
	    
	    if (iov->iov_len == 0) {
	    	uio->uio_iov++;
		uio->uio_iovcnt--;
		if (uio->uio_iovcnt < 0)
		    panic("Pfiltwmove: uio_iovcnt < 0 while uio_resid > 0");
	    }
	    if (first) {
		MGETHDR(m, M_DONTWAIT, MT_DATA);	/* first in chain has header */
	    } else {
		MGET(m, M_DONTWAIT, MT_DATA);
	    }
	    if (m == NULL) {
#ifdef	DEBUG
		Pfiltprintf(ENDBG_NOMEM)("Pfiltwmove: MGET\n");
#endif
	    	error = ENOBUFS;
		break;
	    }
	    /* pad=2 (for Ethernet), so the IP header will be aligned */
	    if (iov->iov_len >= (MINCLSIZE - pad)) { /* enough to use a page */
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0) {
		    goto nopages;
		}
		m->m_data += pad;  /* word align IP/UDP header */
		m->m_len -= pad;
		len = iov->iov_len; /* we can fit it all into an 8K buffer */
	    } else {
nopages:
		if (first)
		    len = MIN((MHLEN - pad), iov->iov_len);
		else
		    len = MIN(MLEN, iov->iov_len);
	    }
	    if (first) {
		/* start at offset +pad for IP header alignment */
		m->m_data += pad;
		first = 0;
	    }

	    error = uiomove(mtod(m, caddr_t), len, uio);
	    m->m_len = len;
	    totlen += len;
	    *mp = m;
	    mp = &(m->m_next);
	}

	if (error) {		/* probably uiomove fouled up */
	    if (mtop)
		m_freem(mtop);
	    return(error);
	}
	*mbufp = mtop;	/* return ptr to top mbuf */

	/* keep ether_input() happy, in case this gets looped back */
	mtop->m_pkthdr.len = totlen;

	return(error);
}

/*
 *  Pfilt_open - open ether net device
 *
 *  Errors:	ENXIO	- illegal minor device number
 *		EBUSY	- minor device already in use
 *		ENOBUFS	- KM_ALLOC failed
 */

/* ARGSUSED */
Pfilt_open(dev, flag, newmin)
dev_t dev;
int flag;
int *newmin;
{
    register int md;
    register int unit;
    register struct enState *enStatep;
    int s;

#ifdef	DEBUG
    Pfiltprintf(ENDBG_TRACE)("Pfilt_open(%o, %x):\n", dev, flag);
#endif

#ifdef	SELF_PROF
    if (enSelfProf) {
	struct timeval starttv, endtv;
	microtime(&starttv);
	microtime(&endtv);
	enCalibrateProf.tv_sec += (endtv.tv_sec - starttv.tv_sec); 
	enCalibrateProf.tv_usec += (endtv.tv_usec - starttv.tv_usec); 
	enCalibrateCount++;
    }
#endif	/* SELF_PROF */

#ifdef	GNODE_CLONING
    /*
     * Each open enet file has a different minor device number.
     * When a user tries to open any of them, we actually open
     * any available minor device and associate it with the
     * corresponding unit.
     *
     * This is not elegant, but UNIX will call
     * open for each new open file using the same inode but calls
     * close only when the last open file referring to the inode 
     * is released. This means that we cannot know inside the
     * driver code when the resources associated with a particular
     * open of the same inode should be deallocated.  Thus, we have
     * to make up a temporary inode to represent each simultaneous
     * open of the ethernet.  Each inode has a different minor device number.
     */
    unit = minor(dev);

    /* check for illegal unit */
    if ( (unit >= enUnits)				/* bad unit */
	|| (enet_info[unit].ifp == 0)			/* ifp not known */
	|| ((enet_info[unit].ifp->if_flags & IFF_UP) == 0) )
							/* or if down */
    {
	return(ENXIO);
    }
#else
    /*
     * Without gnode cloning, we have to have a separate entry in
     * /dev/ for each minor device.  They all start out bound to
     * the first "up" interface.
     */
    for (unit = 0; unit < enUnits; unit++) {	/* check all units in order */
	if (enet_info[unit].ifp				/* interface known */
	    && (enet_info[unit].ifp->if_flags & IFF_UP) ) 	/* and up */
		break;	/* fall out of loop */
    }
    /* check for illegal unit (i.e., nothing "up") */
    if (unit >= enUnits) {				/* bad unit */
	return(ENXIO);
    }
#endif	/* GNODE_CLONING */

#ifdef	GNODE_CLONING
    /* Allocate a minor device number */
    md = PfiltFindMinor();
#ifdef	DEBUG
    Pfiltprintf(ENDBG_TRACE)("Pfilt_open: md = %d\n", md);
#endif
    if (md < 0)
    {
	return(EBUSY);
    }
    *newmin = md;
#else
    /* Use the minor device number supplied */
    md = minor(dev);
    if (md >= NPACKETFILTER) {
	return(ENXIO);		/* an illegal minor device number */
    }
    if (enAllocMap[md]) {
	return(EBUSY);		/* already in use */
    }
#endif	/* GNODE_CLONING */

    enUnitMap[md] = unit;
    enAllocMap[md] = TRUE;

    enStatep = enStatePs[unit];
    Pfiltprintf(ENDBG_DESQ)
    	("Pfilt_open: Desq: %x, %x\n", enDesq.enQ_F, enDesq.enQ_B);
    s = splenet(); /* SMP */
    PF_LOCK();

    /* Allocate memory for this descriptor */
    enAllDescriptors[md] = (struct enOpenDescriptor *)kmem_alloc(kernel_map,
		    round_page(sizeof(struct enOpenDescriptor)));

    if (enAllDescriptors[md] == NULL) {
	return(ENOBUFS);
    }
    PfiltInitDescriptor(enAllDescriptors[md], ENHOLDSIG);
		/* ENHOLDSIG is set by default; historical accident */
    PfiltInsertDescriptor(&(enDesq), enAllDescriptors[md]);
    pfactive++;
    PF_UNLOCK();
    splx(s);
    return(0);
}

#ifdef	GNODE_CLONING
/*
 * PfiltFindMinor - find a free logical device on specified unit
 */
PfiltFindMinor()
{
	register int md;
	
	for (md = 0; md < enMaxMinors; md++) {
		if (enAllocMap[md] == FALSE)
			return(md);
	}
	return(-1);
}
#endif

/*
 *  PfiltInit - initialize ethernet unit (called by pfilt_attach)
 */

PfiltInit(enStatep, unit)
register struct enState *enStatep;
register int unit;
{
    int s;

#ifdef	DEBUG
    Pfiltprintf(ENDBG_INIT)("PfiltInit(%x %d):\n", enStatep, unit);
#endif

    s = splenet(); /* SMP */
    PF_LOCK();
    /*  initialize free queue if not already done  */
    if (enFreeq.enQ_F == 0)
    {
	register int i;

	pfiltinitqueue((struct Queue *)&enFreeq);
	for (i=0; i<ENPACKETS; i++)
	{
	    register struct enPacket *p;

	    p = &enQueueElts[i];
	    p->enP_RefCount = 0;
	    p->enP_Stamp.ens_stamplen = sizeof(struct enstamp);
	    PfiltDeallocatePacket(p);
	}

	/* calculate scavenger thresholds */
	PfiltInitScavenge();

	/* also a good time to init enAllocMap */
	for (i = 0; i < enMaxMinors; i++)
		enAllocMap[i] = FALSE;
    }
    pfiltinitqueue((struct Queue *)&enDesq);	/* init descriptor queue */
    enStatep->ens_UserMaxWaiting = ENNOTSUWAITING;
    enStatep->ens_AllowPromisc = false;
    PF_UNLOCK();
    splx(s);
}

/*
 *  Pfilt_close - ether net device close routine
 */

/* ARGSUSED */
Pfilt_close(dev, flag)
{
    register int md = ENINDEX(dev);
    register struct enState *enStatep = enStatePs[ENUNIT(dev)];
    register struct enOpenDescriptor *d = enAllDescriptors[md];
    struct enPacket *dummy;	/* a dummy pointer - used only for consistency
				 * in the macro pfiltdequeue
				 */
    int ipl;
    extern PfiltTimeout();

    enAllocMap[md] = FALSE;

#ifdef	DEBUG
    Pfiltprintf(ENDBG_TRACE)("Pfilt_close(%d, %x):\n", md, flag);
#endif

    /*
     *  insure that receiver doesn't try to queue something
     *  for the device as we are decommissioning it.
     *  (I don't think this is necessary, but I'm a coward.)
     */
    ipl = splenet();
    PF_LOCK();
    pfactive--;

    /* if necessary, drop counter for promiscuous mode */
    if (d->enOD_Flag & ENPROMISC) {
	decPromiscCount(enStatep, dev);
    }

    /* if necessary, drop counter for copy-all mode */
    if (d->enOD_Flag & ENCOPYALL) {
	decCopyAllCount(enStatep, dev);
    }

    pfiltdequeue((struct Queue *)d->enOD_Link.B, dummy);
    enCurOpens--;
    Pfiltprintf(ENDBG_DESQ)
    		("Pfilt_close: Desq: %x, %x\n", enDesq.enQ_F, enDesq.enQ_B);
    PfiltFlushWaitQueue(d);
    untimeout(PfiltTimeout, (caddr_t)d);
    kmem_free(kernel_map, d, sizeof(d));
    enAllDescriptors[md] = (struct enOpenDescriptor *)0;
    PF_UNLOCK();
    splx(ipl);
}

/*
 *  Pfilt_read - read next packet from net
 */

/* VARARGS */
Pfilt_read(dev, uio)
dev_t dev;
register struct uio *uio;
{
    register struct enOpenDescriptor *d = enAllDescriptors[ENINDEX(dev)];
    register struct enPacket *p;
    int ipl;
    int error;
    int padneeded = 0;		/* modified by Pfiltrmove() */
    int first = 1;		/* is this the first packet in a batch? */
    extern PfiltTimeout();

#if	DEBUG
    Pfiltprintf(ENDBG_TRACE)("Pfilt_read(%x):", dev);
#endif

    ipl = splenet();
    PF_LOCK();
    /*
     *  If nothing is on the queue of packets waiting for
     *  this open enet file, then set timer and sleep until
     *  either the timeout has occurred or a packet has
     *  arrived.
     */

    while (0 == d->enOD_Waiting.enWQ_NumQueued)
    {
	if (d->enOD_Timeout < 0)
	{
	    PF_UNLOCK();
	    splx(ipl);
	    return(0);
	}
	if (d->enOD_Timeout)
	{
	    /*
	     *  If there was a previous timeout pending for this file,
	     *  cancel it before setting another.  This is necessary since
	     *  a cancel after the sleep might never happen if the read is
	     *  interrupted by a signal.
	     */
	    if (d->enOD_RecvState == ENRECVTIMING) {
		untimeout(PfiltTimeout, (caddr_t)d);
	    }
	    timeout(PfiltTimeout, (caddr_t)d, (int)(d->enOD_Timeout));
	    d->enOD_RecvState = ENRECVTIMING;
	}
	else
	    d->enOD_RecvState = ENRECVIDLE;

#if RT_PREEMPT
	mpsleep((caddr_t)d, PRINET|PCATCH, 0, 0, &pf_slock, MS_LOCK_SIMPLE);
#else
	mpsleep((caddr_t)d, PRINET|PCATCH, 0, 0, 0, 0);
#endif
	PF_LOCK();

	switch (d->enOD_RecvState)
	{
	    case ENRECVTIMING:
	    {
		/* Fix for ^C tcpdump hang: if numqueued is zero, someone
		 * unexpectedly woke us up; return instead of looping forever.
		 */
		if (d->enOD_Waiting.enWQ_NumQueued == 0) {
			PF_UNLOCK();
			splx(ipl);
			return(0);
		}
		untimeout(PfiltTimeout, (caddr_t)d);
		d->enOD_RecvState = ENRECVIDLE;
		break;
	    }
	    case ENRECVTIMEDOUT:
	    {
		PF_UNLOCK();
		splx(ipl);
		return(0);
	    }
	}
    }

    /* We believe there is something waiting for us in the queue */
    while (1) {
	if (d->enOD_Waiting.enWQ_NumQueued <= 0) {
	    /*
	     * Either we emptied the queue or someone else did while
	     * we weren't looking.
	     */
	    PF_UNLOCK();
	    splx(ipl);
	    return(0);
	}
	p = PfiltDeWaitQueue(&(d->enOD_Waiting));

	if (first) {
	    first = 0;
	}
	else {	/* this is not the first packet in a batch */
	    if (uio->uio_resid <
	    	(padneeded + sizeof(struct enstamp) + p->enP_ByteCount)) {
		/* no more room in user's buffer; don't truncate packet */
		PfiltPutBack(&(d->enOD_Waiting), p);
		PF_UNLOCK();
		splx(ipl);
		return(0);
	    }
	}

	/*  
	 * Move data from packet into user space.
	 */
	PF_UNLOCK();	/* unlock for uiomove */
	splx(ipl);
#ifdef	BPF
	if (d->enOD_Flag & ENBPFHDR) {
	    int hdrlen = enStatePs[ENUNIT(dev)]->ens_DevParams.end_hdr_len;
	    hdrlen = BPF_WORDALIGN(hdrlen + SIZEOF_BPF_HDR) - hdrlen;
	    error = bpf_rmove(p, uio, &padneeded, d->enOD_Truncation, hdrlen);
	}
	else
#endif
	error = Pfiltrmove(p, uio, d->enOD_Flag, &padneeded, d->enOD_Truncation);
	ipl = splenet();
	PF_LOCK(); /* relock after uiomove */
    
	if (--(p->enP_RefCount) <= 0)	/* if no more claims on this packet */
	{
	    m_freem(p->enP_mbuf);	/* release mbuf */
	    PfiltDeallocatePacket(p);	/* and packet */
	}
#ifdef	BPF
	if ((d->enOD_Flag & (ENBATCH|ENBPFHDR))
				&& !error && uio->uio_resid > 0)
#else
	if (d->enOD_Flag & ENBATCH && !error && uio->uio_resid > 0)
#endif
	    continue;
	else
	    break;
    }
    PF_UNLOCK();
    splx(ipl);
    return(error);
}


/*
 *  PfiltTimeout - process ethernet read timeout
 */

PfiltTimeout(d)
register struct enOpenDescriptor * d;
{
    register int ipl;
    register struct enOpenDescriptor *t;
    register struct enState *enStatep;
    register int unit;

#ifdef	DEBUG
    Pfiltprintf(ENDBG_TRACE)("PfiltTimeout(%x):\n", d);
#endif
    ipl = splenet();
    PF_LOCK();
    if (smp) {
	/* don't call Pfilt_wakeup if the descriptor has gone away */
	for (unit = 0; unit < enUnits; unit++) {
	    enStatep = enStatePs[unit];
	    if (enDesq.enQ_F == 0)
		continue;			/* never initialized */
	    forAllOpenDescriptors(t) {
		if (d == t)
		    goto out;
	    }
	}
	/* if we fall through to here, then descriptor is no longer valid */
	PF_UNLOCK();
	splx(ipl);
	return(0);
    }
out:
    d->enOD_RecvState = ENRECVTIMEDOUT;
    Pfilt_wakeup(d);
    PF_UNLOCK();
    splx(ipl);
    wakeup((caddr_t)d);
}

/*
 *  Pfilt_write - write next packet to net
 */

/* VARARGS */
Pfilt_write(dev, uio)
dev_t dev;
register struct uio *uio;
{
    register int unit = ENUNIT(dev);
    register struct enState *enStatep = enStatePs[unit];
    struct mbuf *mp;
    register struct ifnet *ifp = enet_info[unit].ifp;
    int ipl;
    int error;
    int padneeded = 0;

#if	DEBUG
    Pfiltprintf(ENDBG_TRACE)("Pfilt_write(%x):\n", dev);
#endif

    if (uio->uio_resid == 0) {
	return(0);
    }
    if (uio->uio_resid > enStatep->ens_DevParams.end_MTU) {	/* too large */
	return(EMSGSIZE);
    }
    if (ifp->if_type == IFT_ETHER)
	padneeded = 2; /* offset +2 bytes, so IP header is word aligned */

    /*
     * Copy user data into mbufs
     */
    if (error = Pfiltwmove(uio, &mp, padneeded)) {
	return(error);
    }

    ipl = splenet();
    PF_LOCK();
    if (IF_QFULL(&(ifp->if_snd))) {
	    m_freem(mp);
	    PF_UNLOCK();
	    splx(ipl);
	    return(ENOBUFS);
    }
    PF_UNLOCK();
    /* place mbuf chain on outgoing queue & start if necessary */
    error = (*ifp->if_output)(ifp, mp, &enetaf, NULL);
			/* this always frees the mbuf chain */
    enXcnt++;
    splx(ipl);
    return(error);
}


/*
 *  Pfilt_ioctl - ether net control
 *
 *  EIOCGETP	 - get ethernet parameters
 *  EIOCSETP	 - set ethernet read timeout
 *  EIOCSETF	 - set ethernet read filter
 *  EIOCENBS	 - enable signal when read packet available
 *  EIOCINHS     - inhibit signal when read packet available
 *  FIONREAD	 - check for read packet available
 *  EIOCSETW	 - set maximum read packet waiting queue length
 *  EIOCFLUSH	 - flush read packet waiting queue
 *  EIOCMBIS	 - set mode bits
 *  EIOCMBIC	 - clear mode bits
 *  EICODEVP	 - get device parameters
 *  EIOCMFREE	 - number of free minors
 *  EIOCIFNAME	 - get name of interface for this minor
 *  EIOCTRUNCATE - set maximum number of bytes of packet to be returned
 *  EIOCALLOWPROMISC - allows non-super-users to set IFF_PROMISC
 *  EIOCALLOWCOPYALL - allows non-super-users to set IFF_PFCOPYALL
 *  EIOCMAXBACKLOG - sets non-super-user's maximum backlog
 *  EIOCSRTIMEOUT - set read timeout
 *  EIOCGRTIMEOUT - get read timeout
 *  EIOCSETIF	  - set interface for this minor
 *
 * Also implements:
 *  SIOCGIFADDR	  - Get interface address
 */

/* ARGSUSED */
Pfilt_ioctl(dev, cmd, addr, flag)
caddr_t addr;
dev_t flag;
{

    register struct enState *enStatep = enStatePs[ENUNIT(dev)];
    register struct enOpenDescriptor * d = enAllDescriptors[ENINDEX(dev)];
    struct enPacket *dummy;	/* a dummy pointer - used only for consistency
				 * in the macro pfiltdequeue
				 */
    int ipl;

#if	DEBUG
    Pfiltprintf(ENDBG_TRACE)
	    	("Pfilt_ioctl(%x, %x, %x, %x):\n", dev, cmd, addr, flag);
#endif

    switch (cmd)
    {
	case EIOCGETP:
	{
	    struct eniocb t;

#if SEC_BASE
	    if (privileged(SEC_SYSATTR, 0)) {
	    	t.en_maxwaiting = ENMAXWAITING;
	    } else {
	    	t.en_maxwaiting = enStatep->ens_UserMaxWaiting;
	    }
#else
	    if (suser(u.u_cred, &u.u_acflag) == 0) {
	    	t.en_maxwaiting = ENMAXWAITING;
	    } else {
	    	t.en_maxwaiting = enStatep->ens_UserMaxWaiting;
	    }
#endif

	    t.en_maxpriority = ENMAXPRI;
	    t.en_rtout = d->enOD_Timeout;
	    t.en_addr = -1;
	    t.en_maxfilters = ENMAXFILTERS;

	    bcopy((caddr_t)&t, addr, sizeof t);
	}
	endcase

	case EIOCSETP:
	{
	    struct eniocb t;

	    bcopy(addr, (caddr_t)&t, sizeof t);
	    d->enOD_Timeout = t.en_rtout;
	}
	endcase

	case EIOCSETF:
	{
	    struct enfilter f;
	    unsigned short *fp;

	    bcopy(addr, (caddr_t)&f, sizeof f);
	    if (f.enf_FilterLen > ENMAXFILTERS)
	    {
		return(EINVAL);
	    }
	    /* insure that filter is installed indivisibly */
	    ipl = splenet();
	    PF_LOCK();
#ifdef	BPF
	    /* If a BPF filter is set, get rid of it */
	    if (d->enOD_bpf_filter) {
		kmem_free(kernel_map, d->enOD_bpf_filter, sizeof(d->enOD_bpf_filter));
		d->enOD_bpf_filter = (struct bpf_insn *)NULL;
	    }
#endif
	    bcopy((caddr_t)&f, (caddr_t)&(d->enOD_OpenFilter), sizeof f);
	    /* pre-compute length of filter */
	    fp = &(d->enOD_OpenFilter.enf_Filter[0]);
	    d->enOD_FiltEnd = &(fp[d->enOD_OpenFilter.enf_FilterLen]);
	    d->enOD_RecvCount = 0;	/* reset counts when filter changes */
	    d->enOD_Drops = 0;
	    pfiltdequeue((struct Queue *)d->enOD_Link.B, dummy);
	    enDesq.enQ_NumQueued--;
	    PfiltInsertDescriptor(&(enDesq), d);
	    PF_UNLOCK();
	    splx(ipl);
	}
	endcase

	/*
	 *  Enable signal n on input packet
	 */
	case EIOCENBS:
	{
	    int snum;

	    bcopy(addr, (caddr_t)&snum, sizeof snum);
	    if (snum < NSIG) {
		    d->enOD_SigProc = u.u_procp;
		    d->enOD_SigPid  = u.u_procp->p_pid;
		    d->enOD_SigNumb = snum;	/* This must be set last */
	    } else {
		    return(EINVAL);
	    }
	}
	endcase

	/*
	 *  Disable signal on input packet
	 */
	case EIOCINHS:
	{
		d->enOD_SigNumb = 0;
	}
	endcase

	/*
	 *  Check for packet waiting
	 */
	case FIONREAD:
	{
	    int n;
	    register struct enWaitQueue *wq;

	    ipl = splenet();
	    PF_LOCK();
	    if ((wq = &(d->enOD_Waiting))->enWQ_NumQueued)
		n = wq->enWQ_Packets[wq->enWQ_Head]->enP_ByteCount;
	    else
		n = 0;
	    PF_UNLOCK();
	    splx(ipl);
	    bcopy((caddr_t)&n, addr, sizeof n);
	}
	endcase

	/*
	 *  Set maximum recv queue length for a device
	 */
	case EIOCSETW:
	{
	    unsigned un;

	    bcopy(addr, (caddr_t)&un, sizeof un);
	    /*
	     *  unsigned un	 MaxQueued
	     * ----------------    ------------
	     *  0	       ->  DEFWAITING
	     *  1..MAXWAITING   ->  un
	     *  MAXWAITING..-1  ->  MAXWAITING
	     *
	     * Non-superusers use enStatep->ens_UserMaxWaiting
	     * instead of ENMAXWAITING
	     */

#if SEC_BASE
	    if (!privileged(SEC_SYSATTR, 0)) {
		d->enOD_Waiting.enWQ_MaxWaiting =
				   (un) ? min(un, enStatep->ens_UserMaxWaiting)
					: ENDEFWAITING;
	    } else {
		d->enOD_Waiting.enWQ_MaxWaiting = (un) ? min(un, ENMAXWAITING)
					: ENDEFWAITING;
	    }
#else
	    if (suser(u.u_cred, &u.u_acflag) != 0) {
		d->enOD_Waiting.enWQ_MaxWaiting =
				   (un) ? min(un, enStatep->ens_UserMaxWaiting)
					: ENDEFWAITING;
	    } else {
		d->enOD_Waiting.enWQ_MaxWaiting = (un) ? min(un, ENMAXWAITING)
					: ENDEFWAITING;
	    }
#endif
	}
	endcase

	/*
	 *  Flush all packets queued for a device
	 */
	case EIOCFLUSH:
	{
	    ipl = splenet();
	    PF_LOCK();
	    PfiltFlushWaitQueue(d);
	    d->enOD_Drops = 0;
	    PF_UNLOCK();
	    splx(ipl);
	}
	endcase

	/*
	 *  Set mode bits
	 */
	case EIOCMBIS:
	{
	    u_short mode;
	    short oldmode = d->enOD_Flag;

	    bcopy(addr, (caddr_t)&mode, sizeof mode);
	    if (mode&ENPRIVMODES) {
		return(EINVAL);
	    } else
		d->enOD_Flag |= mode;

	    /* if necessary, bump counter for promiscuous mode */
	    if ((mode & ENPROMISC) && ((oldmode & ENPROMISC) == 0)) {
		ipl = splenet();	/* lock PromiscCount */
		PF_LOCK();
		incPromiscCount(enStatep, dev);
		PF_UNLOCK();
		splx(ipl);		/* unlock PromiscCount */
	    }

	    /* if necessary, bump counter for copy-all mode */
	    if ((mode & ENCOPYALL) && ((oldmode & ENCOPYALL) == 0)) {
		ipl = splenet();	/* lock CopyAllCount */
		PF_LOCK();
		incCopyAllCount(enStatep, dev);
		PF_UNLOCK();
		splx(ipl);		/* unlock CopyAllCount */
	    }
	}
	endcase

	/*
	 *  Clear mode bits
	 */
	case EIOCMBIC:
	{
	    u_short mode;
	    short oldmode = d->enOD_Flag;

	    bcopy(addr, (caddr_t)&mode, sizeof mode);
	    if (mode&ENPRIVMODES) {
		return(EINVAL);
	    } else
		d->enOD_Flag &= ~mode;

	    /* if necessary, drop counter for promiscuous mode */
	    if ((oldmode & ENPROMISC) && (mode & ENPROMISC)) {
		ipl = splenet();	/* lock PromiscCount */
		PF_LOCK();
		decPromiscCount(enStatep, dev);
		PF_UNLOCK();
		splx(ipl);		/* unlock PromiscCount */
	    }

	    /* if necessary, drop counter for copy-all mode */
	    if ((oldmode & ENCOPYALL) && (mode & ENCOPYALL)) {
		ipl = splenet();	/* lock CopyAllCount */
		PF_LOCK();
		decCopyAllCount(enStatep, dev);
		PF_UNLOCK();
		splx(ipl);		/* unlock CopyAllCount */
	    }
	}
	endcase

	/*
	 * Return hardware-specific device parameters.
	 */
	case EIOCDEVP:
	{
	    bcopy((caddr_t)&(enDevParams), addr, sizeof(struct endevp));
	}
	endcase;

	/*
	 * Return # of free minor devices.
	 */
	case EIOCMFREE:
	{
	    register int md;
	    register int sum = 0;
	    
	    for (md = 0; md < enMaxMinors; md++)
	    	if (enAllocMap[md] == FALSE)
			sum++;
	    *(int *)addr = sum;
	}
	endcase;
	
	case EIOCIFNAME:
	{
		struct ifreq ifr;
		register char *cp, *ep;
		register struct ifnet *ifp = enet_info[ENUNIT(dev)].ifp;
		
		ep = ifr.ifr_name + sizeof(ifr.ifr_name) - 2;
		bcopy(ifp->if_name, ifr.ifr_name, (sizeof(ifr.ifr_name) - 2));
		for (cp = ifr.ifr_name; (cp < ep) && *cp; cp++)
			;
		*cp++ = '0' + ifp->if_unit;	/* unit better be < 10 */
		*cp = '\0';
		
		bcopy((caddr_t)&ifr, addr, sizeof(ifr));
	}
	endcase;

	/*
	 *  Set maximum packet length to return
	 */
	case EIOCTRUNCATE:
	{
	    int truncation;

	    bcopy(addr, (caddr_t)&truncation, sizeof truncation);
	    if (truncation < 0)
	    	d->enOD_Truncation = ENMAXINT;
	    else
	    	d->enOD_Truncation = truncation;
	}
	endcase

	/*
	 *  Allows non-super-users to set IFF_PROMISC.  This ioctl
	 * (super-user-only) sets/clears a per-interface flag; if
	 * the flag is set, then descriptors with ENPROMISC cause
	 * the interface to go into promiscuous mode.  We keep a
	 * reference count on this, and clear IFF_PROMISC when the
	 * the count goes to zero.
	 */
	case EIOCALLOWPROMISC:
	{
	    int allowpromisc;
	    register struct ifnet *ifp = enet_info[ENUNIT(dev)].ifp;
	    int wantpromisc;

	    bcopy(addr, (caddr_t)&allowpromisc, sizeof allowpromisc);

	    if (allowpromisc < 0) {	/* attempt to read current setting */
		allowpromisc = (enStatep->ens_AllowPromisc == true);
		bcopy((caddr_t)&allowpromisc, addr, sizeof allowpromisc);
		return(0);
	    }

#if SEC_BASE
	    if (!privileged(SEC_SYSATTR, 0)) {
		return(EPERM);
	    }
#else
	    if (suser(u.u_cred, &u.u_acflag) != 0) {
		return(EPERM);
	    }
#endif
	    ipl = splenet();	/* lock PromiscCount */
	    PF_LOCK();
	    if ((enStatep->ens_AllowPromisc == true) && (allowpromisc == 0)) {
		/* Must disable IFF_PROMISC if we set it */
		if (enStatep->ens_PromiscCount > 0)
		    enetSetIfflags(ifp, (ifp->if_flags & ~IFF_PROMISC));
		enStatep->ens_PromiscCount = 0;
		enStatep->ens_AllowPromisc = false;
	    }
	    if ((enStatep->ens_AllowPromisc == false) && (allowpromisc != 0)) {
		/* Count the number of descriptors wanting promiscuous mode */
		wantpromisc = 0;
		forAllOpenDescriptors(d)
		    if (d->enOD_Flag & ENPROMISC)
			wantpromisc++;
		enStatep->ens_PromiscCount = wantpromisc;
		if (wantpromisc > 0)
		    enetSetIfflags(ifp, (ifp->if_flags | IFF_PROMISC));
		enStatep->ens_AllowPromisc = true;
	    }
	    PF_UNLOCK();
	    splx(ipl);		/* unlock PromiscCount */
	}
	endcase

	/*
	 *  Allows non-super-users to set IFF_PFCOPYALL.  This ioctl
	 * (super-user-only) sets/clears a per-interface flag; if
	 * the flag is set, then descriptors with ENCOPYALL cause
	 * ether_read() to go into copy-all mode.  We keep a
	 * reference count on this, and clear IFF_PFCOPYALL when the
	 * the count goes to zero.
	 */
	case EIOCALLOWCOPYALL:
	{
	    int allowcopyall;
	    register struct ifnet *ifp = enet_info[ENUNIT(dev)].ifp;
	    int wantcopyall;

	    bcopy(addr, (caddr_t)&allowcopyall, sizeof allowcopyall);

	    if (allowcopyall < 0) {	/* attempt to read current setting */
		allowcopyall = (enStatep->ens_AllowCopyAll == true);
		bcopy((caddr_t)&allowcopyall, addr, sizeof allowcopyall);
		return(0);
	    }

#if SEC_BASE
	    if (!privileged(SEC_SYSATTR, 0)) {
		return(EPERM);
	    }
#else
	    if (suser(u.u_cred, &u.u_acflag) != 0) {
		return(EPERM);
	    }
#endif
	    ipl = splenet();	/* lock CopyAllCount */
	    PF_LOCK();
	    if ((enStatep->ens_AllowCopyAll == true) && (allowcopyall == 0)) {
		/* Must disable IFF_PFCOPYALL if we set it */
		if (enStatep->ens_CopyAllCount > 0)
		    enetSetIfflags(ifp, (ifp->if_flags & ~IFF_PFCOPYALL));
		enStatep->ens_CopyAllCount = 0;
		enStatep->ens_AllowCopyAll = false;
	    }
	    if ((enStatep->ens_AllowCopyAll == false) && (allowcopyall != 0)) {
		/* Count the number of descriptors wanting copy-all mode */
		wantcopyall = 0;
		forAllOpenDescriptors(d)
		    if (d->enOD_Flag & ENCOPYALL)
			wantcopyall++;
		enStatep->ens_CopyAllCount = wantcopyall;
		if (wantcopyall > 0)
		    enetSetIfflags(ifp, (ifp->if_flags | IFF_PFCOPYALL));
		enStatep->ens_AllowCopyAll = true;
	    }
	    PF_UNLOCK();
	    splx(ipl);		/* unlock CopyAllCount */
	}
	endcase

	/*
	 * Set the maximum backlog allowed for non-super-user descriptors.
	 * Make sure it is within the legal range.
	 */
	case EIOCMAXBACKLOG:
	{
	    int maxbacklog;

	    bcopy(addr, (caddr_t)&maxbacklog, sizeof maxbacklog);

	    if (maxbacklog < 0) {	/* attempt to read current setting */
		maxbacklog = enStatep->ens_UserMaxWaiting;
		bcopy((caddr_t)&maxbacklog, addr, sizeof maxbacklog);
		return(0);
	    }
#if SEC_BASE
	    if (!privileged(SEC_SYSATTR, 0)) {
		return(EPERM);
	    }
#else
	    if (suser(u.u_cred, &u.u_acflag) != 0) {
		return(EPERM);
	    }
#endif
	    if ((maxbacklog < 1) || (maxbacklog > ENMAXWAITING)) {
		return(EINVAL);
	    }
	    enStatep->ens_UserMaxWaiting = maxbacklog;
	}
	endcase

	/*
	 * Set the read timeout for this descriptor.  Converts
	 * from struct timeval to ticks.
	 */
	case EIOCSRTIMEOUT:
	{
	    struct timeval rtv;
	    long ticks;

	    bcopy(addr, (caddr_t)&rtv, sizeof rtv);

	    /* Check to make sure this is not too big */
	    if (rtv.tv_sec >= ((1<<((NBBY*sizeof(long))-1))/hz) )
	    if (rtv.tv_usec >= 1000000) {
	    	return(EINVAL);
	    }

	    ticks = (rtv.tv_sec * hz) + (rtv.tv_usec / tick);
	    d->enOD_Timeout = ticks;
	}
	endcase

	/*
	 * Get the read timeout for this descriptor.  Converts
	 * from ticks to struct timeval.
	 */
	case EIOCGRTIMEOUT:
	{
	    struct timeval rtv;
	    long ticks = d->enOD_Timeout;
	    
	    rtv.tv_sec = ticks / hz;
	    ticks = ticks % hz;
	    rtv.tv_usec = ticks * tick;

	    bcopy((caddr_t)&rtv, addr, sizeof rtv);
	}
	endcase

	/*
	 * Change the interface associated with a minor device
	 */
	case EIOCSETIF:
	{
		struct ifreq ifr;
		register struct ifnet *oldifp = enet_info[ENUNIT(dev)].ifp;
		register struct ifnet *newifp;
		register struct enState *newStatep = (struct enState *)0;
		int unit;
		struct ifnet *ifunit();
		static struct ifnet *genericUnit();
		
		bcopy(addr, (caddr_t)&ifr, sizeof(ifr));
		if ((newifp = genericUnit(ifr.ifr_name)) == (struct ifnet *)0)
		    if ((newifp = ifunit(ifr.ifr_name)) == (struct ifnet *)0) {
			return(EINVAL);
		}

		if (newifp == oldifp) {	/* no change */
		    return(0);
		}

		/* find enState pointer for new interface */
		for (unit = 0; unit < enUnits; unit++) {
		    if (enet_info[unit].ifp == newifp) {
			newStatep = enStatePs[unit];
			break;
		    }
		}
		if (newStatep == (struct enState *)0) {
		    return(EINVAL);	/* interface not known */
		}
		
		ipl = splenet();
		PF_LOCK();
		/* remove this from the old enState struct */
		pfiltdequeue((struct Queue *)d->enOD_Link.B, dummy);
		enCurOpens--;
		if (d->enOD_Flag & ENPROMISC)
		    decPromiscCount(enStatep, dev);
		if (d->enOD_Flag & ENCOPYALL)
		    decCopyAllCount(enStatep, dev);

		enUnitMap[ENINDEX(dev)] = unit;	/* change unit map */

		/* add this to the new enState struct */
		PfiltInsertDescriptor(&(newStatep->ens_Desq), d);
		PfiltFlushWaitQueue(d);
#ifdef	BPF
		bpf_reset_counts(d);
#endif
		if (d->enOD_Flag & ENPROMISC)
		    incPromiscCount(newStatep, dev);
		if (d->enOD_Flag & ENCOPYALL)
		    incCopyAllCount(newStatep, dev);
		PF_UNLOCK();
		splx(ipl);
	}
	endcase;

	/*
	 * Get interface address
	 *	(Added for BPF compatibility but generally a good thing
	 *	to support for packet filter descriptors).
	 */
	case SIOCGIFADDR: {
	    struct ifnet *ifp;
	    int error;
	    
	    ifp = enet_info[ENUNIT(dev)].ifp;
	    
	    error = (*(ifp->if_ioctl))(ifp, cmd, addr);
	    
	    return(error);
	}
	endcase;

	default:
	{
#ifdef	BPF
	    int i;
	    /* It might be a BPF ioctl */
	    i = bpf_ioctl(dev, cmd, addr, flag);
	    return(i);
#else
	    return(EINVAL);
#endif
	}
    }

    return(0);

}
				
/****************************************************************
 *								*
 *		Support for select() system call		*
 *								*
 *	Other hooks in:						*
 *		PfiltInitDescriptor()				*
 *		PfiltInputDone()					*
 *		PfiltTimeout()					*
 ****************************************************************/
/*
 * inspired by the code in tty.c for the same purpose.
 */

/*
 * Pfilt_select - returns true iff the specific operation
 *	will not block indefinitely.  Otherwise, return
 *	false but make a note that a selwakeup() must be done.
 */
Pfilt_select(dev, events, revents, scanning)
	dev_t dev;
	short *events, *revents;
	int scanning;
{
	register struct enOpenDescriptor *d;
	register struct enWaitQueue *wq;
	register int s;

	if (*events & POLLNORM) {	/* select READ */
	    s = splenet();
	    PF_LOCK();
	    d = (enAllDescriptors[ENINDEX(dev)]);
	    if (scanning) {
		wq = &(d->enOD_Waiting);
		if (wq->enWQ_NumQueued) {
			*revents |= POLLNORM;  /* at least one packet queued */
		} else {
			/*
			 * If there's already a select() waiting on this
			 * minor device then this is a collision.
			 * [This shouldn't happen because enet minors
			 * really should not be shared, but if a process
			 * forks while one of these is open, it is possible
			 * that both processes could select() us.]
			 */
			/* add this thread to the list of waiting threads */
			select_enqueue(&d->enOD_selq);
		}
	    } else {
		select_dequeue(&d->enOD_selq); 
	    }
	    PF_UNLOCK();
	    splx(s);	
	}
	if (*events & POLLOUT) {	/* select WRITE */
	    /*
	     * since the queueing for output is shared not just with
	     * the other enet devices but also with the IP system,
	     * we can't predict what would happen on a subsequent
	     * write.  However, since we presume that all writes
	     * complete eventually, and probably fairly fast, we
	     * pretend that select() is true.
	     */
	    if (scanning) {
		*revents |= POLLOUT;  /* say it's writeable */
	    }
	}
	return(0);
}

Pfilt_wakeup(d)
register struct enOpenDescriptor *d;
{
	if (!queue_empty(&d->enOD_selq)) {
		select_wakeup(&d->enOD_selq);
	}
}

/*
 * pfilt_filter - incoming linkage from if_XXXXX.c
 */

struct mbuf *PfiltInputDone();

struct mbuf *pfilt_filter(edp, m, eptr, istrailer, encap_type, loopback)
struct ether_driver *edp;
register struct mbuf *m;
struct ether_header *eptr;
int istrailer;
u_short	encap_type;	/* if non-zero, use this instead of eptr->ether_type */
int loopback;		/* == 1, this is one of our own output packets */
{
    register struct enPacket *p;
    struct mbuf *m0;
    int s;
    int HowReceived = 0;
    int mustcopy;
    register struct enpextent *exp;
    register int offset;
    register int extents_left;
    struct enpextent map[MAX_EXTENTS];
#ifdef	SELF_PROF
    struct timeval starttv, endtv;

    if (enSelfProf)
	microtime(&starttv);
#endif	/* SELF_PROF */

#if	DEBUG
    Pfiltprintf(ENDBG_TRACE)("pfilt_filter:\n");
#endif

    /*
     * Two-step process:
     *	(1) Determine how the packet was received
     *	(2) Figure out if anyone wants it and if so
     *	    we need to make another copy for the packet filter
     */
    /* Step 1: How did we get the packet? */

    if (loopback > 0) {
	/* If this is one of our own output packets, save a copy for pfilt.
	 * We know IFF_PFCOPYALL is set because loopback is set.
	 * We already have the ethernet header, so just queue it.
	 */
	struct ether_header *eh;
	mustcopy = 1;
	/* important: never set how=PROMISC type for loopback! */
	m0 = pf_mcopym(m, 0, M_COPYALL, M_DONTWAIT);
	if (m0 == NULL) { /* no mbufs */
	    return(m);
	}
	eh = mtod(m0, struct ether_header *);
	if (encap_type == 0)
		/* get type from packet and convert to network order */
		encap_type = htons((u_short)eh->ether_type);

	if (m->m_flags & M_BCAST)
		HowReceived = ENSF_BROADCAST;		/* set BROADCAST */
	else if (m->m_flags & M_MCAST)
		HowReceived = ENSF_MULTICAST;		/* set MULTICAST */
	/* else How=0, initial value */

    } else if (m->m_flags & (M_BCAST|M_MCAST)) {
	if (m->m_flags & M_BCAST)
		HowReceived = ENSF_BROADCAST;		/* set BROADCAST */
	else if (m->m_flags & M_MCAST)
		HowReceived = ENSF_MULTICAST;		/* set MULTICAST */
    } else {
	register short *sb, *dap;
	short d_addr[4];
	/* Copy the destination address into main memory */

	if (edp->ess_if.if_type == IFT_FDDI)
	    sb = (short *)((struct fddi_header *)eptr)->fddi_dhost;
	else {	/* ASSUMPTION: if not FDDI, then Ethernet */
	    sb = (short *)eptr->ether_dhost;
	    if (encap_type == 0)
		encap_type = eptr->ether_type;
	}
	dap = d_addr;
	dap[0] = sb[0];
	dap[1] = sb[1];
	dap[2] = sb[2];

	/* Compare destination address with our own address */
	sb = (short *)edp->ess_addr;
	if ((*((int *)dap) != *((int *)sb)) || (dap[2] != sb[2])) {
		HowReceived = ENSF_PROMISC;
	}
    }

/* printf is captured by syslog */
#ifdef	DEBUG_ER
    if (er_debug) {
	if (edp->ess_if.if_type == IFT_FDDI) {
	    struct fddi_header *fh = (struct fddi_header *)eptr;
	    printf("%02x-%02x-%02x-%02x-%02x-%02x->%02x-%02x-%02x-%02x-%02x-%02x %04x %x\n",
	    fh->fddi_shost[0], fh->fddi_shost[1], fh->fddi_shost[2],
	    fh->fddi_shost[3], fh->fddi_shost[4], fh->fddi_shost[5],
	    fh->fddi_dhost[0], fh->fddi_dhost[1], fh->fddi_dhost[2],
	    fh->fddi_dhost[3], fh->fddi_dhost[4], fh->fddi_dhost[5],
	    encap_type, HowReceived);
	}
	else {
	    printf("%02x-%02x-%02x-%02x-%02x-%02x->%02x-%02x-%02x-%02x-%02x-%02x %04x/%04x %x\n",
	    eptr->ether_shost[0], eptr->ether_shost[1], eptr->ether_shost[2],
	    eptr->ether_shost[3], eptr->ether_shost[4], eptr->ether_shost[5],
	    eptr->ether_dhost[0], eptr->ether_dhost[1], eptr->ether_dhost[2],
	    eptr->ether_dhost[3], eptr->ether_dhost[4], eptr->ether_dhost[5],
	    eptr->ether_type, encap_type, HowReceived);
	}
    }
#endif	/* DEBUG_ER */

    /* Performance optimization: skip work if no packet filters present */
    if (pfactive == 0) {	/* performance optimization */
	if (HowReceived == ENSF_PROMISC) {
	    /* Don't let DLI/IP/ARP see promiscuously-received packets */
	    m_freem(m);
	    return(0);
	}
	else {
	    return(m);
	}
    }

    /*
     * Invariants at this point:
     *	    pfactive is TRUE
     */

    /* Step 2: who wants it, and do we need to make a copy? */
    if (HowReceived == ENSF_PROMISC) {
	mustcopy = 0;    /* ONLY the packet filter can see this */
    }
    else if (HowReceived & (ENSF_MULTICAST|ENSF_BROADCAST)) {
	/* all broadcast/multicast packets are shown to the packet filter */
	mustcopy = 1;
    }
    else {	/* Unicast packet */
	/* Check to see if IP or ARP wants the packet */
	if ((encap_type == ETHERTYPE_IP) || (encap_type == ETHERTYPE_ARP)) {
	    /* IP or ARP does want the packet */
	    if (edp->ess_if.if_flags & IFF_PFCOPYALL) {
		mustcopy = 1;	/* packet filter might want it, too */
	    } else {
		return(m);	/* packet filter NOT allowed to see this pkt */
	    }
	} else {  /* DLI will ALWAYS want the packet, argh, what a waste */
	    /*
	     * mustcopy=1 here insures DECnet, LAT, and other packetfilter apps
	     * will continue to work.
	     * The bogus part is we never know whether these are running.
	     * TODO: use some kind of "protocol registry".
	     */
	    mustcopy = 1;
	}
    }

    /*
     * Invariants at this point:
     *  if HowReceived == ENSF_PROMISC, mustcopy is FALSE
     *		[so that we don't return promiscuously-received
     *		 packet to ether_input() and thence to DLI]
     */

#ifdef	DEBUG_ER
    if (er_debug) {
	printf("mustcopy %x\n", mustcopy);
    }
#endif

    /* if it's one of our own output packets, we already have the header */
    if (loopback > 0) {
	goto gotheader;
    }

    /*
     * We need local net header after all.
     */
    /* this copy is for pfilt only, don't need to use MGETHDR here */
    MGET(m0, M_DONTWAIT, MT_DATA);
    if (m0 == 0) {	/* out of mbufs? */
	if (mustcopy) {
	    return (m);
	} else {
	    m_freem(m);
	    return (struct mbuf *) 0;
	}
    }

    /*
     * If we must copy the mbuf, do it now.  If it
     * fails, screw the packet filter and process it normally.
     * If we don't have to copy, just append the mbuf.
     */
    if (mustcopy) {
	m0->m_next = pf_mcopym(m, 0, M_COPYALL, M_DONTWAIT);
	/* m_copy returns NULL if it fails */
	if (m0->m_next == NULL) {
	    m_free(m0);
	    return (m);
	}
    } else {
	m0->m_next = m;
    }

    if (edp->ess_if.if_type == IFT_FDDI) {
	/*
	 *  Copy the FDDI header; no "protocol type" field here
	 */
	*(mtod(m0, struct fddi_header *)) = *(struct fddi_header *)eptr;
	m0->m_len = sizeof(struct fddi_header);
    }
    else {	/* ASSUMPTION: if not FDDI, then Ethernet */
	/*
	 *  Copy the ether header and swap the protocol
	 *  type in "wire" format.
	 */

	*(mtod(m0, struct ether_header *)) = *eptr;
	eptr = mtod(m0, struct ether_header *);
	eptr->ether_type = htons((u_short)eptr->ether_type);
	m0->m_len = sizeof(struct ether_header);
    }

gotheader:
    s = splenet();
    PF_LOCK();
    p = PfiltAllocatePacket();	/* panics if not possible */

    p->enP_ByteCount = m_length(m0);
    microtime(&(p->enP_Stamp.ens_tstamp));
    p->enP_mbuf = m0;
    p->enP_Stamp.ens_flags = HowReceived;
    p->enP_Stamp.ens_ifoverflows = edp->ess_missed;
    
    if (istrailer) { /* was trailer encapsulation */
	p->enP_ByteCount -= 2 * sizeof(u_short);
	/* we've dropped trailer type & length */
	p->enP_Stamp.ens_flags |= ENSF_TRAILER;
    }

    /* Build a map from packet byte offsets to mbufs */
    exp = map;
    /* ASSUMPTION: m is non-null upon entry to this procedure */
    exp->data = mtod(m0, char *);
    exp->offset = 0;
    offset = m0->m_len;
    exp->afterEnd = offset;
#if	INNERDEBUG
    Pfiltprintf(ENDBG_TRACE)("[o%d: aE:%d]",exp->offset,exp->afterEnd);
#endif
    exp++;

    extents_left = (MAX_EXTENTS - 2);	/* 1 just used, 1 left for sentinel */
    while ((m0 = m0->m_next) && (--extents_left >= 0)) {
	exp->data = mtod(m0, char *);
	exp->offset = offset;
	offset += m0->m_len;
	exp->afterEnd = offset;
#if	INNERDEBUG
	Pfiltprintf(ENDBG_TRACE)("[o%d: aE:%d]",exp->offset,exp->afterEnd);
#endif
	exp++;
    }
#if	INNERDEBUG
    Pfiltprintf(ENDBG_TRACE)("\n");
#endif
    /* The last extent always has data == 0; we reserved one for this */
    exp->afterEnd = MAX_OFFSET;
    exp->data = (char *)0;
    
    m0 = PfiltInputDone(enStatePs[edp->ess_enetunit], p, map);
    if (m0)
	m_freem(m0);

#ifdef	SELF_PROF
    if (enSelfProf) {
	microtime(&endtv);
	
	enPerPktProf.tv_sec += (endtv.tv_sec - starttv.tv_sec);
	enPerPktProf.tv_usec += (endtv.tv_usec - starttv.tv_usec);
	enPktCount++;
    }
#endif	/* SELF_PROF */
    PF_UNLOCK();
    splx(s);
    if (mustcopy)
	return (m);
    else
	return (0);
}

/*
 * PfiltInputDone - process correctly received packet
 * SMP: assumes lock held coming in
 */

struct mbuf *PfiltInputDone(enStatep, p, exp)
register struct enState *enStatep;
register struct enPacket *p;
struct enpextent *exp;
{
    register struct enOpenDescriptor *d;
    int queued = 0;
    int drops = 0;
    register unsigned long rcount;
    register struct enOpenDescriptor *prevd;
    int Exclusive;
    struct mbuf *notwanted = 0;
#ifdef	SELF_PROF
    struct timeval starttv, endtv;
    boolean accept;
#endif

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltInputDone: not lock owner");
    }
#if	INNERDEBUG
    Pfiltprintf(ENDBG_TRACE)("PfiltInputDone(%x): %x\n", enStatep, p);
#endif

    enRmissed = p->enP_Stamp.ens_ifoverflows;

    forAllOpenDescriptors(d)
    {
	/* restrict access to promiscuously received packets */
	if ( (p->enP_Stamp.ens_flags & ENSF_PROMISC) &&
		((d->enOD_Flag & ENPROMISC) == 0) )
	    continue;
#ifdef	SELF_PROF
	/* null filters always accept all packets */
	if (d->enOD_OpenFilter.enf_FilterLen == 0)
	    accept = true;
	else if (!enSelfProf) {
	    accept = PfiltDoFilter(p, d, exp);
	} else {
	    microtime(&starttv);
	    accept = PfiltDoFilter(p, d, exp);
	    microtime(&endtv);

	    enPerFiltProf.tv_sec += (endtv.tv_sec - starttv.tv_sec);
	    enPerFiltProf.tv_usec += (endtv.tv_usec - starttv.tv_usec);
	    enFiltCount++;
	}
	if (accept)
#else
#ifdef	BPF
	/*
	 * Complex logic:
	 *   Accept IFF
	 *	BPF filter set AND BPF filter matches
	 *		OR
	 *	BPF filter not set AND PF len = 0 or PF filter matches
	 */
	if ( (d->enOD_bpf_filter &&
		bpf_filter(d->enOD_bpf_filter, p->enP_mbuf,
				p->enP_ByteCount, 0))
	     || ((d->enOD_bpf_filter == (struct bpf_insn *)0) &&
			((d->enOD_OpenFilter.enf_FilterLen == 0)
				|| PfiltDoFilter(p, d, exp)) ) )
#else
	/* null filters always accept all packets */
	if ((d->enOD_OpenFilter.enf_FilterLen == 0)
		|| PfiltDoFilter(p, d, exp))
#endif	/* BPF */
#endif	/* SELF_PROF */
	{
	    if (d->enOD_Waiting.enWQ_NumQueued <
	    		d->enOD_Waiting.enWQ_MaxWaiting)
	    {
		PfiltEnWaitQueue(&(d->enOD_Waiting), p);
		p->enP_RefCount++;
		p->enP_Stamp.ens_dropped = d->enOD_Drops;
		d->enOD_Drops = 0;
		queued++;
		wakeup((caddr_t)d);
		Pfilt_wakeup(d);
#if	INNERDEBUG
		Pfiltprintf(ENDBG_TRACE)("PfiltInputDone: queued\n");
#endif
	    } else {
	    	d->enOD_Drops++;
		drops++;
	    }

	    /*  send notification when input packet received  */
	    if (d->enOD_SigNumb) {
		if (d->enOD_SigProc->p_pid == d->enOD_SigPid)
			psignal(d->enOD_SigProc, d->enOD_SigNumb);
		if ((d->enOD_Flag & ENHOLDSIG) == 0)
			d->enOD_SigNumb = 0;		/* disable signal */
	    }
	    rcount = ++(d->enOD_RecvCount);
	    
	    Exclusive = !(d->enOD_Flag & ENNONEXCL);

	    /* see if ordering of filters is wrong */
	    if (d->enOD_OpenFilter.enf_Priority >= ENHIPRI) {
	    	prevd = (struct enOpenDescriptor *)d->enOD_Link.B;
		/*
		 * If d is not the first element on the queue, and
		 * the previous element is at equal priority but has
		 * a lower count, then promote d to be in front of prevd.
		 */
		if (((struct Queue *)prevd != &(enDesq.enQ_Head)) &&
	    	    (d->enOD_OpenFilter.enf_Priority ==
				prevd->enOD_OpenFilter.enf_Priority)) {
		    /* threshold difference to avoid thrashing */
		    if ((100 + prevd->enOD_RecvCount) < rcount) {
			PfiltReorderQueue(&(prevd->enOD_Link), &(d->enOD_Link));
		    }
		}
	    }

	    if (Exclusive)
		break;	/* this filter wants exclusive delivery */
	}
    }
    if (queued == 0)			/* this buffer no longer in use */
    {
	notwanted = p->enP_mbuf;
	PfiltDeallocatePacket(p);			/*  and packet */
	if (drops == 0)
	    enRunwanted++;
    }
    else
	enRcnt++;

    enRdrops += drops;

    return(notwanted);		/* returns unwanted packet or NULL */
}

#define	opx(i)	(i>>ENF_NBPA)

/* SMP: assumes lock held coming in */
boolean
PfiltDoFilter(p, d, exp)
struct enPacket *p;
struct enOpenDescriptor *d;
struct enpextent *exp;
{

    register unsigned short *sp;
    register unsigned short *fp;
    register unsigned short *fpe;
    register unsigned op;
    register unsigned arg;
    register struct enpextent *exp1;
    unsigned short stack[ENMAXFILTERS+1];
    struct fw {unsigned short arg:ENF_NBPA, op:ENF_NBPO;};

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltDoFilter: not lock owner");
    }
#ifdef	INNERDEBUG
    Pfiltprintf(ENDBG_TRACE)("PfiltDoFilter(%x,%x):\n", p, d);
#endif
    sp = &stack[ENMAXFILTERS];
    fp = &d->enOD_OpenFilter.enf_Filter[0];
    fpe = d->enOD_FiltEnd;
	/* ^ is really: fpe = &fp[d->enOD_OpenFilter.enf_FilterLen]; */
    *sp = TRUE;

    for (; fp < fpe; )
    {
	op = ((struct fw *)fp)->op;
	arg = ((struct fw *)fp)->arg;
	fp++;
	switch (arg)
	{
	    default:
		if (arg < ENF_PUSHWORD) {
		    Pfiltprintf(ENDBG_TRACE)("bad arg %d\n", arg);
		    return(false);
		}
	    	arg -= ENF_PUSHWORD;
		arg *= 2;		/* convert to byte offset */
		/*
		 * We know that arg is between 0 and MAX_OFFSET
		 * and we know that the last extent has a sentinel
		 * value (afterEnd = MAX_OFFSET) so we don't have to check it.
		 *
		 * We are searching for the first extent that ends after
		 * arg.
		 */
		exp1 = exp;
		while (exp1->afterEnd <= arg) {
#ifdef	INNERDEBUG
		    Pfiltprintf(ENDBG_TRACE)("a:%d aE:%d ", exp1->afterEnd, arg);
#endif
		    exp1++;
		}
		if (exp1->data) {
		    *--sp = *(unsigned short *)&(exp1->data[arg-exp1->offset]);
#ifdef	INNERDEBUG
		    Pfiltprintf(ENDBG_TRACE)("o:%d %x\n", exp1->offset, *sp);
#endif
		}
		else
		{
#ifdef	INNERDEBUG
		    Pfiltprintf(ENDBG_TRACE)("a:%d aE:%d =>0(len)\n",
		    				exp1->afterEnd, arg);
#endif
		    return(false);
		}
		break;
	    case ENF_PUSHLIT:
		*--sp = *fp++;
		break;
	    case ENF_PUSHZERO:
		*--sp = 0;
		break;
	    case ENF_PUSHONE:
		*--sp = 1;
		break;
	    case ENF_PUSHFFFF:
		*--sp = 0xFFFF;
		break;
	    case ENF_PUSH00FF:
		*--sp = 0x00FF;
		break;
	    case ENF_PUSHFF00:
		*--sp = 0xFF00;
		break;
	    case ENF_NOPUSH:
		break;
	}
	if (sp < &stack[2])	/* check stack overflow: small yellow zone */
	{
	    Pfiltprintf(ENDBG_TRACE)("=>0(--sp)\n");
	    return(false);
	}
	if (op == ENF_NOP)
	    continue;
	/*
	 * all non-NOP operators binary, must have at least two operands
	 * on stack to evaluate.
	 */
	if (sp > &stack[ENMAXFILTERS-2])
	{
	    Pfiltprintf(ENDBG_TRACE)("=>0(sp++)\n");
	    return(false);
	}
	arg = *sp++;
	switch (op)
	{
	    default:
#ifdef	INNERDEBUG
		Pfiltprintf(ENDBG_TRACE)("=>0(def)\n");
#endif
		return(false);
	    case opx(ENF_AND):
		*sp &= arg;
		break;
	    case opx(ENF_OR):
		*sp |= arg;
		break;
	    case opx(ENF_XOR):
		*sp ^= arg;
		break;
	    case opx(ENF_EQ):
		*sp = (*sp == arg);
		break;
	    case opx(ENF_NEQ):
		*sp = (*sp != arg);
		break;
	    case opx(ENF_LT):
		*sp = (*sp < arg);
		break;
	    case opx(ENF_LE):
		*sp = (*sp <= arg);
		break;
	    case opx(ENF_GT):
		*sp = (*sp > arg);
		break;
	    case opx(ENF_GE):
		*sp = (*sp >= arg);
		break;

	    /* short-circuit operators */

	    case opx(ENF_COR):
	    	if (*sp++ == arg) {
#ifdef	INNERDEBUG
		    Pfiltprintf(ENDBG_TRACE)("=>COR %x\n", *sp);
#endif
		    return(true);
		}
		break;
	    case opx(ENF_CAND):
	    	if (*sp++ != arg) {
#ifdef	INNERDEBUG
		    Pfiltprintf(ENDBG_TRACE)("=>CAND %x\n", *sp);
#endif
		    return(false);
		}
		break;
	    case opx(ENF_CNOR):
	    	if (*sp++ == arg) {
#ifdef	INNERDEBUG
		    Pfiltprintf(ENDBG_TRACE)("=>COR %x\n", *sp);
#endif
		    return(false);
		}
		break;
	    case opx(ENF_CNAND):
	    	if (*sp++ != arg) {
#ifdef	INNERDEBUG
		    Pfiltprintf(ENDBG_TRACE)("=>CAND %x\n", *sp);
#endif
		    return(true);
		}
		break;
	}
    }
#ifdef	INNERDEBUG
    Pfiltprintf(ENDBG_TRACE)("=>%x\n", *sp);
#endif
    return(*sp ? true : false);

}

/* SMP: assumes lock held coming in */
PfiltInitDescriptor(d, flag)
register struct enOpenDescriptor *d;
{

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltInitDescriptor: not lock owner");
    }
#if	DEBUG
    Pfiltprintf(ENDBG_TRACE)("PfiltInitDescriptor(%x):\n", d);
#endif
    d->enOD_RecvState = ENRECVIDLE;
    d->enOD_OpenFilter.enf_FilterLen = 0;
    d->enOD_OpenFilter.enf_Priority = 0;
    d->enOD_FiltEnd = &(d->enOD_OpenFilter.enf_Filter[0]);
    d->enOD_RecvCount = 0;
    d->enOD_Truncation = ENMAXINT;
    d->enOD_Timeout = 0;
    d->enOD_SigNumb = 0;
    d->enOD_Flag = flag;
    d->enOD_SelColl = 0;
    d->enOD_SelProc = 0;
    queue_init(&d->enOD_selq);

    /*
     * Remember the PID that opened us, at least until some process
     * sets a signal for this minor device
     */
    d->enOD_SigPid = u.u_procp->p_pid;

    /* Clear all counters */
    d->enOD_Drops = 0;
    d->enOD_DropCount = 0;
    d->enOD_FlushRecv = 0;
    d->enOD_FlushDrop = 0;

#ifdef	BPF
    /* No BPF filter set yet */
    d->enOD_bpf_filter = (struct bpf_insn *)NULL;
#endif

    PfiltInitWaitQueue(&(d->enOD_Waiting));
#if	DEBUG
    Pfiltprintf(ENDBG_TRACE)("=>eninitdescriptor\n");
#endif

}

/*
 *  PfiltInsertDescriptor - insert open descriptor in queue ordered by priority
 *  SMP: assumes lock held coming in
 */

PfiltInsertDescriptor(q, d)
register struct enQueue *q;
register struct enOpenDescriptor *d;
{
    struct enOpenDescriptor * nxt;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltInsertDescriptor: not lock owner");
    }
    nxt = (struct enOpenDescriptor *)q->enQ_F;

    while ((struct Queue *)q != &(nxt->enOD_Link))
    {
	if (d->enOD_OpenFilter.enf_Priority > nxt->enOD_OpenFilter.enf_Priority)
	    break;
	nxt = (struct enOpenDescriptor *)nxt->enOD_Link.F;
    }
    pfiltenqueue((struct Queue *)&(nxt->enOD_Link),(struct Queue *)&(d->enOD_Link));
    Pfiltprintf(ENDBG_DESQ)("enID: Desq: %x, %x\n", q->enQ_F, q->enQ_B);
    q->enQ_NumQueued++;
}

int enReorderCount = 0;		/* for external monitoring */

/*
 * PfiltReorderQueue - swap order of two elements in queue
 *	assumed to be called at splenet
 * SMP: assumes lock held coming in
 */
PfiltReorderQueue(first, last)
register struct Queue *first;
register struct Queue *last;
{
	register struct Queue *prev;
	register struct Queue *next;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("PfiltReorderQueue: not lock owner");
    }
	Pfiltprintf(ENDBG_DESQ)("enReorderQ: %x, %x\n", first, last);
	
	enReorderCount++;

	/* get pointers to other queue elements */
	prev = first->B;
	next = last->F;
	
	/*
	 * no more reading from queue elements; this ensures that
	 * the code works even if there are fewer than 4 elements
	 * in the queue.
	 */

	prev->F = last;
	next->B = first;
	
	last->B = prev;
	last->F = first;
	
	first->F = next;
	first->B = last;
}

#if RT_PREEMPT
static pflockinit_done = 0;
#endif

pfilt_attach(ifp, devp)
struct ifnet *ifp;
struct endevp *devp;
{
    register struct enState *enStatep;
    enStatep = (struct enState *)kmem_alloc(kernel_map,
		    round_page(sizeof(struct enState)));
    if (enStatep == NULL)
	panic("pfilt_attach: not enough memory");
    enStatePs[enUnits] = enStatep;

#ifdef	DEBUG
    Pfiltprintf(ENDBG_INIT) ("pfilt_attach: type %d, addr ", devp->end_dev_type);
    if (enDebug&ENDBG_INIT) {
	register int i;
	for (i = 0; i < devp->end_addr_len; i++)
	    printf("%x ", devp->end_addr[i]);
	printf("\n");
    }
#endif	/* DEBUG */

#if RT_PREEMPT
    if (!pflockinit_done) {
	PF_LOCKINIT();	/* only do this once */
	pflockinit_done = 1;
    }
#endif


    enet_info[enUnits].ifp = ifp;
    bcopy((caddr_t)devp, (caddr_t)&(enDevParams), sizeof(struct endevp));
    PfiltInit(enStatep, enUnits);
    return(enUnits++);
}

/*
 * Called by interface driver when its hardware address changes
 * (either because of explicit change, or because it was set wrong initially)
 * Address length cannot change
 */
pfilt_newaddress(unit, hwaddr)
int unit;
u_char *hwaddr;
{
	struct endevp *devp;

	/* unit must already exist */
	if ((unit < 0) || (unit >= enUnits)) {
	    panic("pfilt_newaddress: bad unit number");
	}
	
	devp = &(enStatePs[unit]->ens_DevParams);
	bcopy((caddr_t)hwaddr, (caddr_t)(devp->end_addr),
			devp->end_addr_len);
}

/* SMP: assumes lock held coming in */
enetSetIfflags(ifp, newflags)
register struct ifnet *ifp;
register int newflags;
{
	struct ifreq ifr;
	int ret;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("enetSetIfflags: not lock owner");
    }
	/* code more or less copied from net/if.c */
	ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) |
					(newflags & ~IFF_CANTCHANGE);

	ifr.ifr_flags = newflags;
	/* unlock before calling the driver */
	PF_UNLOCK();
	ret=(*(ifp->if_ioctl))(ifp, SIOCSIFFLAGS, &ifr);
	PF_LOCK();
	return(ret);
}

/* SMP: assumes lock held coming in */
decPromiscCount(enStatep, dev)
register struct enState *enStatep;
register dev_t dev;
{
	struct ifnet *ifp;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("decPromiscCount: not lock owner");
    }
	if (enStatep->ens_AllowPromisc &&
		(--(enStatep->ens_PromiscCount) == 0)) {
	    ifp = enet_info[ENUNIT(dev)].ifp;
	    enetSetIfflags(ifp, (ifp->if_flags & ~IFF_PROMISC));
	}
}

/* SMP: assumes lock held coming in */
incPromiscCount(enStatep, dev)
register struct enState *enStatep;
register dev_t dev;
{
	struct ifnet *ifp;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("incPromiscCount: not lock owner");
    }
	if (enStatep->ens_AllowPromisc &&
		(enStatep->ens_PromiscCount++ == 0)) {
	    ifp = enet_info[ENUNIT(dev)].ifp;
	    enetSetIfflags(ifp, (ifp->if_flags | IFF_PROMISC));
	}
}

/* SMP: assumes lock held coming in */
decCopyAllCount(enStatep, dev)
register struct enState *enStatep;
register dev_t dev;
{
	struct ifnet *ifp;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("decCopyAllCount: not lock owner");
    }
	if (enStatep->ens_AllowCopyAll &&
		(--(enStatep->ens_CopyAllCount) == 0)) {
	    ifp = enet_info[ENUNIT(dev)].ifp;
	    enetSetIfflags(ifp, (ifp->if_flags & ~IFF_PFCOPYALL));
	}
}

/* SMP: assumes lock held coming in */
incCopyAllCount(enStatep, dev)
register struct enState *enStatep;
register dev_t dev;
{
	struct ifnet *ifp;

    if (smp_debug) {
	if (!(smp_owner(&lk_pfilt)))
		panic("incCopyAllCount: not lock owner");
    }
	if (enStatep->ens_AllowCopyAll &&
		(enStatep->ens_CopyAllCount++ == 0)) {
	    ifp = enet_info[ENUNIT(dev)].ifp;
	    enetSetIfflags(ifp, (ifp->if_flags | IFF_PFCOPYALL));
	}
}

/* Convert generic name to a struct ifnet pointer */
static char genericPrefix[] = ENGENPREFIX;
static struct ifnet *genericUnit(iname)
register char *iname;
{
	register int i = 0;
	register int unit;
	
	/* Check for correct prefix */
	while (genericPrefix[i]) {
	    if (genericPrefix[i] != iname[i])
		return((struct ifnet *)0);
	    i++;
	}

	/* TODO: support more than single digit */
	/* Check that suffix is a single digit and within range */
	unit = iname[i] - '0';
	if ((unit < 0) || (unit > 9) || (unit >= enUnits) || iname[i+1])
	    return((struct ifnet *)0);

	return(enet_info[unit].ifp);
}

#define COPY_DATA 1	/* copy mbuf clusters into new clusters */
/*
 * Copy a cluster mbuf into a new cluster mbuf (avoid using a ref count).
 * Based on m_copym() from uipc_mbuf.c 
 */
/*
 * Make a copy of an mbuf chain starting "off0" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * The wait parameter is a choice of M_WAIT/M_DONTWAIT from caller.
 */
struct mbuf *
pf_mcopym(m, off0, len, wait)
	register struct mbuf *m;
	int off0, wait;
	register int len;
{
	register struct mbuf *n, **np, *m2;
	register int off = off0;
	struct mbuf *top;
	int copyhdr = 0;

	if (off < 0 || len < 0)
		panic("m_copym");
	if (off == 0 && m->m_flags & M_PKTHDR)
		copyhdr = 1;
	while (off > 0) {
		if (m == 0)
			panic("m_copym");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copym");
			break;
		}
		MGET(n, wait, m->m_type);
		*np = n;
		if (n == 0)
			goto nospace;
		if (copyhdr) {
			M_COPY_PKTHDR(n, m);
			if (len == M_COPYALL)
				n->m_pkthdr.len -= off0;
			else
				n->m_pkthdr.len = len;
			copyhdr = 0;
		}
		n->m_len = MIN(len, m->m_len - off);
		if (m->m_flags & M_EXT) {
			int s = splimp();
			MBUF_LOCK();
#ifdef COPY_DATA
			MCLGET(n, M_DONTWAIT);
			if ((n->m_flags & M_EXT) == 0) {
				goto nospace;
			}
			/* copy cluster data into new cluster mbuf n */
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
#else
			n->m_ext = m->m_ext;
			insque(&n->m_ext.ext_ref, &m->m_ext.ext_ref);
#endif
			MBUF_UNLOCK();
			splx(s);
#ifndef COPY_DATA
			/* if not copying, just point at old m_data,
			 * and mark n as cluster with EXT flag
			 */
			n->m_data = m->m_data + off;
			n->m_flags |= M_EXT;
#endif
		} else
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	return (top);
nospace:
	m_freem(top);
	return (0);
}


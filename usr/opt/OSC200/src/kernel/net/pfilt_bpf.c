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
static char *rcsid = "@(#)$RCSfile: pfilt_bpf.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/05/11 22:27:36 $";
#endif

/*
 * Based on:
 * static  char    *sccsid = "@(#)pfilt_bpf.c	6.2  (ULTRIX) 10/5/92";
 */
/*********************************************************************
 *  Ethernet packet filter layer/support for Berkeley Packet Filter
 *	(BPF) compatibility.
 **********************************************************************
 * HISTORY
 *
 * 5-Oct-92	Jeffrey Mogul	DECWRL
 *	Created.
 **********************************************************************
 */

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


#include "sys/socket.h"
#include "net/if.h"

#undef	queue
#undef	dequeue

#define BPF 1

#ifdef	BPFTEST
#include "../bpf_stuff/pfilt.h" 
#include "../bpf_stuff/bpf.h"
#include "../bpf_stuff/pfiltdefs.h"
#else
#include "net/pfilt.h" 
#include "net/bpf.h"
#include "net/pfiltdefs.h"
#endif

#if RT_PREEMPT
extern simple_lock_data_t pf_slock; /* packetfilter lock, defined in pfilt.c */
#define PF_LOCK()	simple_lock(&pf_slock)
#define PF_UNLOCK()	simple_unlock(&pf_slock)
#else
#define PF_LOCK()
#define PF_UNLOCK()
#endif

/* These "extern" definitions and #defines should be in pfiltdefs.h */

extern struct enState  *enStatePs[];
extern struct enOpenDescriptor
		       *enAllDescriptors[];
extern char		enUnitMap[];

/*
 *  PfiltFlushWaitQueue - remove all packets from wait queue
 */
#define	PfiltFlushWaitQueue(wq)	PfiltTrimWaitQueue(wq, 0)

#define	splenet	splimp	/* used to be spl6 but I'm paranoid */

/*
 * The default filter accepts the maximum number of bytes from each packet.
 */
struct bpf_insn bpf_default_filter[] = {
	BPF_STMT(BPF_RET|BPF_K, -1),
};

/*
 *  bpf_ioctl - BPF-related ioctls
 *
 *  BIOCGBLEN	  - Get read buffer len [emulated by returning a constant]
 *  BIOCSBLEN	  - Set read buffer len (No-op, output echos the input value)
 *  BIOCSETF	  - Set BPF-style filter
 *  BIOCFLUSH	  - Like EIOCFLUSH, but also resets BPF counts
 *  BIOCPROMISC	  - Sets ENPROMISC
 *  BIOCGDLT	  - Returns BPF value for data link type
 *  BIOCGETIF	  - Same as EIOCIFNAME
 *  BIOCSETIF	  - Same as EIOCSETIF [which now resets BPF counts]
 *  BIOCSRTIMEOUT - Same as EIOCSRTIMEOUT
 *  BIOCGRTIMEOUT - Same as EIOCGRTIMEOUT
 *  BIOCGSTATS	  - Get packet, drop counts
 *  BIOCIMMEDIATE - No-op
 *  BIOCVERSION	  - Get filter language version
 *  
 */

/* ARGSUSED */
bpf_ioctl(dev, cmd, addr, flag)
caddr_t addr;
dev_t flag;
{

    register struct enState *enStatep = enStatePs[ENUNIT(dev)];
    register struct enOpenDescriptor * d = enAllDescriptors[ENINDEX(dev)];
    int ipl;

#if	DEBUG
    Pfiltprintf(ENDBG_TRACE)
	    	("bpf_ioctl(%x, %x, %x, %x):\n", dev, cmd, addr, flag);
#endif

    switch (cmd)
    {
	/*
	 * Same as EIOCIFNAME
	 */
	case BIOCGETIF:
	    return(Pfilt_ioctl(dev, EIOCIFNAME, addr, flag));

	/*
	 * Same as EIOCSRTIMEOUT
	 */
	case BIOCSRTIMEOUT:
	    return(Pfilt_ioctl(dev, EIOCSRTIMEOUT, addr, flag));

	/*
	 * Same as EIOCGRTIMEOUT
	 */
	case BIOCGRTIMEOUT:
	    return(Pfilt_ioctl(dev, EIOCGRTIMEOUT, addr, flag));

	/*
	 * Same as EIOCSETIF
	 */
	case BIOCSETIF:
	    return(Pfilt_ioctl(dev, EIOCSETIF, addr, flag));

	/*
	 * Get read buffer len [emulated by returning a constant]
	 */
	case BIOCGBLEN: {
	    *(u_int *)addr = BPF_MAXBUFSIZE;	/* XXX Could be bigger! XXX */
	}
	endcase;

	/*
	 * Set read buffer len (No-op, output echos the input value)
	 */
	case BIOCSBLEN: {
	}
	endcase;

	/*
	 * Set BPF-style filter
	 */
	case BIOCSETF: {
	    struct bpf_program *fp = (struct bpf_program *)addr;
	    struct bpf_insn *fcode;
	    u_int flen, size;
	    
	    /* NULL instruction list means use default filter */
	    if (fp->bf_insns == 0) {
		if (fp->bf_len != 0)
			return(EINVAL);
		bpf_filter_install(d, bpf_default_filter,
			sizeof(bpf_default_filter)/sizeof(struct bpf_insn));
		return(0);
	    }
	    
	    flen = fp->bf_len;
	    size = flen * sizeof(*fp->bf_insns);
	    
	    /* Don't let user commit too much allocated kernel memory */
	    if (flen > BPF_MAXINSNS)
		return(EINVAL);

	    fcode = (struct bpf_insn *)kmem_alloc(kernel_map,
                    round_page(size));

	    /* Copy in, validate filter before installing it */
	    if ( (copyin((caddr_t)(fp->bf_insns), (caddr_t)fcode, size) == 0)
		&& bpf_validate(fcode, (int)flen) ) {
		bpf_filter_install(d, fcode, flen);
	    }
	    else {
		kmem_free(kernel_map, fcode, size);
		return(EINVAL);
	    }
	}
	endcase;

	/*
	 * Sets ENPROMISC mode bit
	 */
	case BIOCPROMISC: {
	    short oldmode = d->enOD_Flag;
	    
	    if (oldmode & ENPROMISC)
		break;	/* already set */

	    d->enOD_Flag |= ENPROMISC;
	    
	    /* bump counter for promiscuous mode */
	    ipl = splenet();	/* lock PromiscCount */
	    PF_LOCK();
	    incPromiscCount(enStatep, dev);
	    PF_UNLOCK();
	    splx(ipl);		/* unlock PromiscCount */
	    
	}
	endcase;

	/*
	 * Same as EIOCFLUSH, but also resets BPF counts
	 */
	case BIOCFLUSH: {
	    ipl = splenet();
	    PF_LOCK();
	    PfiltFlushWaitQueue(d);
	    bpf_reset_counts(d);
	    d->enOD_Drops = 0;
	    PF_UNLOCK();
	    splx(ipl);
	}
	endcase;

	/*
	 * Returns BPF value for data link type
	 */
	case BIOCGDLT: {
	    unsigned int dt;

	    /* Translate from internal to BPF type codes */
	    switch (enDevParams.end_dev_type) {
		case ENDT_3MB:
		    dt = DLT_EN3MB;
		    break;

		case ENDT_10MB:
		    dt = DLT_EN10MB;
		    break;

		case ENDT_FDDI:
		    dt = DLT_FDDI;
		    break;

		default:
		    dt = 0;
		    break;
	    }
	    *(unsigned int*)addr = dt;
	}
	endcase;

	/*
	 * Get packet, drop counts
	 */
	case BIOCGSTATS: {
	    struct bpf_stat *bs = (struct bpf_stat *)addr;
	    
	    bs->bs_recv = d->enOD_RecvCount - d->enOD_FlushRecv;
	    bs->bs_drop = d->enOD_DropCount - d->enOD_FlushDrop;
	}
	endcase;

	/*
	 * No-op
	 */
	case BIOCIMMEDIATE: {
	}
	endcase;

	/*
	 * Get filter language version
	 */
	case BIOCVERSION: {
	    struct bpf_version *bv = (struct bpf_version *)addr;

	    bv->bv_major = BPF_MAJOR_VERSION;
	    bv->bv_minor = BPF_MINOR_VERSION;
	}
	endcase;

	default:
	{
	    return(EINVAL);
	}

    }

    return(0);

}

/****************************************************************
 *								*
 *	Support subroutines for BPF				*
 *								*
 ****************************************************************/

/*
 * Install a validated BPF filter; free old filter if necessary
 */
bpf_filter_install(d, fcode, flen)
struct enOpenDescriptor *d;
struct bpf_insn *fcode;
int flen;
{
	int ipl = splenet();
	PF_LOCK();

	/* Free old filter (unless it is the default, which is not alloc'd) */
	if (d->enOD_bpf_filter
		&& (d->enOD_bpf_filter != bpf_default_filter))
	    kmem_free(kernel_map, d->enOD_bpf_filter, sizeof(d->enOD_bpf_filter));

	d->enOD_bpf_filter = fcode;
	d->enOD_bpf_flen = flen;

	PfiltFlushWaitQueue(d);

	bpf_reset_counts(d);

	d->enOD_Drops = 0;

	PF_UNLOCK();
	splx(ipl);
}

/*
 * "Flush" BPF counts (by bumping up the base values)
 */
bpf_reset_counts(d)
struct enOpenDescriptor *d;
{
	d->enOD_FlushRecv = d->enOD_RecvCount;
	d->enOD_FlushDrop = d->enOD_DropCount;
}

/*
 * read: return contents of mbufs to user.  DO NOT free them, since
 *	there may be multiple claims on the packet!
 * Prepends BPF-style header, and pads so that network layer rather
 * 	than data-link layer is word-aligned.
 */

bpf_rmove(p, uio, padneeded, truncation, hdrlen)
struct enPacket *p;
register struct uio *uio;
int *padneeded;			/* by reference, so we can change it */
int truncation;
int hdrlen;
{
	register int len;
	register int count;
	register int error = 0;
	register struct mbuf *m = p->enP_mbuf;
	int total = hdrlen;
	int pn = *padneeded;
	struct bpf_hdr hdrbuf[2];
		/* Leave a little room after the real header for padding */
	struct bpf_hdr *hdrp = hdrbuf;
	
	/* first, transfer BPF header */
	hdrp->bh_tstamp = p->enP_Stamp.ens_tstamp;
	hdrp->bh_caplen = min(p->enP_ByteCount, truncation);
	hdrp->bh_datalen = p->enP_ByteCount;
	hdrp->bh_hdrlen = hdrlen;

	if (uio->uio_resid >= (hdrlen + pn)) {
	    error = uiomove((caddr_t)(hdrp) - pn, hdrlen + pn, uio);
	    if (error)
		return(error);
	}
	else {	/* no room for stamp */
	    uio->uio_resid = 0;	/* make sure Pfiltread() exits */
	    return(EMSGSIZE);
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

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
static char *rcsid = "@(#)$RCSfile: adu_ln.c,v $ $Revision: 1.2.5.2 $ (DEC) $Date: 1993/05/16 01:04:50 $";
#endif

/*
 *	@(#)$RCSfile: adu_ln.c,v $ $Revision: 1.2.5.2 $ (DEC) $Date: 1993/05/16 01:04:50 $
 */

/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University	of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/* ---------------------------------------------------------------------
 * Modification History: adu_ln.c
 *
 *  14-Feb-91 -- rjl
 *	Rewrote a vast amount to work on the adu
 *
 *  10-Oct-90  map (Mark Parenti)
 *	Created this file.  Derived from if_ln.c
 *
 * --------------------------------------------------------------------- */
#include "aduln.h"

#if NADULN > 0 || defined(BINARY)
/*
 * Digital ADU Network Interface
 */
#include <data/adu_ln_data.c>
#include <hal/adudefs.h>

char *nexus;
extern int cpu;

static int	lndebug = 2;		/* debug flag, range 0->4 */
int	alnprobe(), alnattach(), alnintr();
int	alninit(), alnoutput(), alnioctl(), alnwatch(), alnrint(), alntint();
INIT_PKT *adu_send_init();
struct	mbuf *alnget();


caddr_t alnstd[] = { 0 };
struct	driver adulndriver =
	{ alnprobe, 0, alnattach, 0, 0, alnstd, 0, 0, "aduln", alninfo };

static u_char ln_sunused_multi[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static unsigned int ln_crc_table[16];		/* crc initialization table */
static unsigned int ln_poly = 0xedb88320;	/* polynomial initialization */

extern struct tv_config *tv_cnfg_base;	/* TV node configuration pointer */
extern int adu_vector_alloc();

/*
 * This is an array of pointers to the lance related IO space registers.
 * The array is here initialized to the offsets from the base address (BB).
 * Prior to actual usage of these registers the base address (BB) will be
 * added to the intialized offset to result in a pointer to the actual IO
 * space register itself.
 */
struct adu_niregs adu_ni_regs = {
	(u_long *)ADU_NI_BASE,	
	(u_long *)ADU_NI_ICR,
	(u_long *)ADU_NI_DB,
};

struct adu_niregs *adu_niregptr;


long	adu_ln_intr_reg = 0;

/*
 * Probe the Lance to see if it's there
 */
alnprobe(io_base_addr, ctlr)
	register char *io_base_addr;
	struct controller *ctlr;   /* Only for MIPS machine */
{
	static int unit=0; 	/* Walk thru softc's only for VAX */	
	register struct ln_softc *sc;
	register struct lnsw *lnsw;	/* ptr to switch struct */
	register struct ln_initb *initb; /* LRB initb ptr */
	register INIT_PKT *initpkt;
	register struct ln_ring *rptr;
	int prp;	 	/* physical addr ring pointer */
	int pi;			/* physical addr init block */
	int i, j, oldcvec,index;
	unsigned int tmp, x;
	unsigned short flag;
	struct ln_ring *adu_lnptr;

	struct mbuf mbf;

#ifdef MACH
	adu_lnptr = (struct ln_ring *)kmem_alloc(kernel_map, 8192);
#else
	KM_ALLOC(adu_lnptr, struct ln_ring *, 8192, KM_DEVBUF, 
		 KM_NOW_CL_CO_CA | KM_NOCACHE );
#endif
	if (adu_lnptr == (struct ln_ring *)NULL)
		return(0);

#ifdef MACH
	sc = (struct ln_softc *)kmem_alloc(kernel_map, sizeof(struct ln_softc));
#else
	KM_ALLOC(sc, struct ln_softc *, sizeof(struct ln_softc),KM_DEVBUF, KM_CLEAR);
#endif
	if (!sc)
		return(0);		

#ifdef MACH
	bzero((char *)sc, sizeof(struct ln_softc));
#endif

	initb = &sc->ln_initb;
	rptr = sc->rptr = adu_lnptr;

	/*
	 * The ni register pointers are already initialized to the 
	 * offset from the base register.  Now add the base address of the
	 * IO module's TVBus registers to these offsets to get pointers to 
	 * the registers themselves.
	 * 
	 * Jump through hoops with the casting!
 	 */
	 adu_ni_regs.base = (u_long *) ((u_long)io_base_addr +
				      (u_long)adu_ni_regs.base);
	 adu_ni_regs.icr = (u_long *) ((u_long)io_base_addr +
				      (u_long)adu_ni_regs.icr);
	 adu_ni_regs.db = (u_long *) ((u_long)io_base_addr +
				      (u_long)adu_ni_regs.db);
	/*
	 * CPU identifiers are found in cpuconf.h
	 */
	switch (cpu) {
		case ALPHA_ADU:			/* ADU */
			unit = ctlr->ctlr_num;
			aln_softc[unit] = sc;
			sc->lnsw = lnsw = adusw;
			flag = (LN_IDON | LN_INIT);
			break;
		default: 
			printf("alnprobe : cpu type %d unknown\n", cpu );
			return(0);
	}

	/*
	 * Initialize some per unit counters
	 */
	sc->callno = 0;

	lnshowmulti = 0;	
	lnbablcnt=0;	
	lnmisscnt=0;	
	lnmerrcnt=0;
	lnrestarts=0;	

	/* 
	 * Initialize multicast address array
	 */
	for (i=0; i<NMULTI; i++) {
		sc->muse[i] = 0;
		bcopy(ln_sunused_multi,&sc->multi[i],MULTISIZE);
	}


	/*
	 * Initialize init block, ln_initb
	 *
	  ln_mode;			mode word
	  ln_sta_addr;			station address
	  ln_multi_mask;		multicast address mask
	 */


	initb->ln_mode = ETHER_NORMAL;	/* normal operation (mode==0) */


	/*
	 * fill out multicast address mask
	 */
	for (i = 0; i < 4; i++) {
		initb->ln_multi_mask[i] = 0x0000;
	}

	/*
	 * initialize the multicast address CRC table,
	 * using initb as a dummy variable.
	 */
	for (index=0; index<16; index++) {
			tmp = index;
			for (j=0; j<4; j++) {
			    x = (tmp & 0x01);
			    tmp = (tmp >> 1);	/* logical shift right 1 bit */
			    if (x == 1)
				tmp = (tmp ^ ln_poly);	/* XOR */
			}
			ln_crc_table[index] = tmp;
		}
	
	/*
	 * Init the buffer descriptors and
	 * indexes for each of the lists.
	 */
	adu_init_bufs(sc);
	for ( i = 0 ; i < nALNNRCV ; i++ ) {
		alninitdesc (sc, &rptr->rcv_ring[i], sc->rlbuf[i],
			    ETHER_RECEIVE_PACKET, i);
	}
	for ( i = 0 ; i < nALNNXMT ; i++ ) {
		alninitdesc (sc, &rptr->trans_ring[i], sc->tlbuf[i],
			    ETHER_EMPTY, i);
	}
	sc->nxmit = sc->otindex = sc->tindex = sc->rindex = 0;
	/*
	 * Tell the tvbus where the rings are
	 */
#ifdef MACH
	if (pmap_svatophys(sc->rptr, adu_ni_regs.base) == KERN_INVALID_ADDRESS)
	{
	    printf("alnprobe : can't get kva for sc->rptr\n");
	    return(0);
	}
#else
	*adu_ni_regs.base = (u_long)svtophy( (char *)sc->rptr );
#endif
	RING_NI_DOORBELL;

	return( sizeof(struct ln_initb));
}
 
/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
alnattach(ctlr)
	struct controller *ctlr;

{
	register struct ln_softc *sc = aln_softc[ctlr->ctlr_num];
	register struct ifnet *ifp = &sc->is_if;
	register int i;
	register struct sockaddr_in *sin;

	sc->is_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;/*Bdcast address*/
	sc->is_ac.ac_arphrd = ARPHRD_ETHER;                   /*Ethernet addr */

	ifp->if_unit = ctlr->ctlr_num;
	ifp->if_name = "aduln";
	ifp->if_mtu = ETHERMTU;
	ifp->if_type = IFT_ETHER;			 /* Ethernet */
	ifp->if_addrlen = 6;				 /* Media address len */
	ifp->if_hdrlen = sizeof(struct ether_header) + 8;/* Media hdr len
							  * +8 for SNAP
							  */

	ifp->if_flags |= IFF_SIMPLEX | IFF_BROADCAST |
			 IFF_NOTRAILERS;		/* flags; IFF_SIMPLEX
							 * indicates we need to
							 * hear our own
							 * broadcasts. The
							 * looping is done at a
							 * higher level in
							 * if_ethersubr.c
							 */

	((struct arpcom *)ifp)->ac_ipaddr.s_addr = 0;

	/*
	 * Read the physical address  and save it.
	 */
	for ( i=0 ; i<6 ; i++ ) {
		sc->is_addr[i] = 
			(u_char)(tv_cnfg_base[ctlr->slot].tv_config_spec.tv_io_config.tv_ether_addr[i] & 0xFF);
	}

	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;
	ifp->if_init = alninit;
	ifp->if_output = ether_output;
	ifp->if_start = alnoutput;   /* NOT alnstart! need ifp -> unit 1st */
	ifp->if_ioctl = alnioctl;
	ifp->if_timer = 0;
	ifp->if_watchdog = alnwatch; 
	ifp->if_reset = 0;
/*	ifp->d_affinity = ALLCPU;       -- SMP */
	ifp->if_version = "DEC ADU Ethernet Interface";
	printf("\taduln%d: %s, hardware address: %s \n", ctlr->ctlr_num,
		ifp->if_version,ether_sprintf(sc->is_addr));
	
#if NPACKETFILTER > 0
        attachpfilter(&(sc->is_ed));
#endif /* NPACKETFILTER > 0 */
	if_attach(ifp);
}

/*
 * Initialization of interface and allocation of mbufs for receive ring
 */
alninit(unit)
	int unit;
{
	register struct ln_softc *sc = aln_softc[unit];
	register struct ln_initb *initb = &sc->ln_initb;
	register struct lnsw *lnsw = sc->lnsw;
	register struct ifnet *ifp = &sc->is_if;
	register struct ln_ring *rptr = sc->rptr;
	register INIT_PKT *initpkt;
	int i,k;
	int vector_return;

	/* address not known */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;
	if (ifp->if_flags & IFF_RUNNING)
		return;

	/*
	 * We only nail down our buffer addresses on the very
	 * first call, then don't relinquish them again.
	 */
        k = splimp();

	sc->callno++;

	/*
	 * Take the interface out of reset, program the vector, 
	 * enable interrupts, and tell the world we are up.
	 */

	initpkt = adu_send_init(initb, sc);

	/* If zero status then init failed. Return without setting RUN bit */
	if(adu_check_init(initpkt) == 0) {
		splx(k);
		return;
	}
	/*
	 * Only setup interrupt on first call
	 */
	if( sc->callno == 1 ){
		/*
		 * Allocate SCB vector to enable interrupt handling.  The params
		 * are (interrupt ipl, interrupt routine, int routine parameter).
		 * Use the returned value to setup the IRQNODE and IRQCHAN fields
		 * of the icr register.
		 *
		 * Since there are only 2 SCB vectors at SPL21 and 20 at SPL20,
		 * request that the ipl be 20 instead of 21.  Thats why this has a 
		 * hardcoded value of 20 instead of SPLBIO which equals 21.
		 */
		vector_return = adu_vector_alloc(20, alnrint, 0);
		if (vector_return == 0) {
			panic("adu_ln_init: call to adu_vector_alloc failed");
		}
		adu_ln_intr_reg = ((ADU_GETIRQNODE(vector_return) << 2) |
				   (ADU_GETIRQCHAN(vector_return) << 6));

		vector_return = adu_vector_alloc(20, alntint, 0);
		if (vector_return == 0) {
			printf("adu_ln_init: adu_vector_alloc failed on alntint.\n");
			return(0);
		}
	

		adu_ln_intr_reg |= (ADU_GETIRQCHAN(vector_return) << 11);

	}
	/*
	 * Init setup buffers but we lied to the io processor so that
	 * it doesn't fill up without us knowing about it.
	 */
	for ( i = 0 ; i < nALNNRCV ; i++ ) {
		((RCV_PKT *)rptr->rcv_ring)[i].flag = ETHER_EMPTY;
	}
	ENABLE_LN_INTERRUPT(ETHER_RE | ETHER_TE);
	mb();
	ifp->if_flags |= (IFF_UP | IFF_RUNNING);
	ifp->if_flags &= ~IFF_OACTIVE;
	alnstart( unit );
	sc->ztime = time.tv_sec;
	splx(k);
}
 
/*
 * Ethernet interface transmit interrupt.
 */
alntint(unit)
	int unit;
{
	register struct ln_softc *sc = aln_softc[unit];
	register index;
	register struct ln_trn_pkt *dp;
	register struct lnsw *lnsw = sc->lnsw;
	register struct ln_ring *rptr;

	struct mbuf *mp;
	struct ifnet *ifp = &sc->is_if;

	/*
	 * While we haven't caught up to current transmit index,
	 * AND the buffer is ours, AND there are still transmits
	 * which haven't been pulled off the ring yet, proceed
	 * around the ring in search of the last packet. We grab
	 * the "ln_flag" byte out of the ring entry to check the
	 * ownership bit FIRST, then grab the "ln_flag2" short
	 * word later to eliminate a timing window in which the
	 * LANCE may update the "ln_flag" before updating "ln_flag2".
	 */
	rptr = sc->rptr;
	dp = (struct ln_trn_pkt *)&(rptr->trans_ring[sc->otindex]);
	while ( (sc->otindex != sc->tindex)
		&& (dp->flag == ETHER_EMPTY)
		&& sc->nxmit > 0 ) {

		index = sc->otindex;


		/*
		 * Found last buffer in the packet
		 * (hence a valid string of descriptors)
		 * so free things up.
		 */
		mp = sc->tmbuf[index];

		sc->tmbuf[index] = (struct mbuf *)NULL;

		/* increment old index pointer, if it catches up
		 * with current index pointer, we will break out
		 * of the loop.  Otherwise, go around again
		 * looking for next end-of-packet cycle.
		 */
		if(--sc->nxmit <= 0){
			ifp->if_timer = 0;
			ifp->if_flags &= ~IFF_OACTIVE;
			sc->nxmit = 0;
		}

		sc->is_if.if_opackets++;

		/* Bump collision counter */
		if( dp->status == ETHER_OK ) {
			sc->is_if.if_collisions += dp->ncol;
		}

		/*
		 * Check for transmission errors.
		 */
		if (dp->status != ETHER_OK) {
			sc->is_if.if_oerrors++;
			if (sc->ctrblk.est_sendfail != 0xffff) {
				sc->ctrblk.est_sendfail++;
				if (dp->status == ETHER_COLLISION) {
					/* excessive collisions */
					if (lndebug) printf("ln%d: excessive collisions (RTRY)\n",unit);
					sc->ctrblk.est_sendfail_bm |= 1;
				}
				if (dp->status == ETHER_REJECT) {
					if (lndebug) printf("ln%d: ETHER_REJECT error\n", unit);
					printf("ln%d: ETHER_REJECT error\n", unit);
				}

				if (dp->status == ETHER_FIRMWARE) {
					if (lndebug) printf("ln%d: ADU Firmware Error\n",unit);
				}
		        }
			m_freem(mp);
		} else {
			if (mp) {
				m_freem( mp );
			}
		}
		/*
		 * Init the buffer descriptor
		 */
		alninitdesc(sc, &rptr->trans_ring[index], 
			   sc->tlbuf[index], ETHER_EMPTY,  index);
		sc->otindex = ++index % nALNNXMT;

		/* for next while lp */
		dp = (struct ln_trn_pkt *)&(rptr->trans_ring[sc->otindex]);
	}
	/*
	 * Dequeue next transmit request, if any.
	 */
	if (!(ifp->if_flags & IFF_OACTIVE))
		alnstart( unit );
}
 
/*
 * Ethernet interface receiver interrupt.
 * If can't determine length from type, then have to drop packet.  
 * Othewise decapsulate packet based on type and pass to type specific 
 * higher-level input routine.
 */
alnrint(unit)
	int unit;
{
	register struct ln_softc *sc = aln_softc[unit];
	register struct ln_rcv_pkt *dp;
	register struct ln_ring *rptr;
	register struct lnsw *lnsw = sc->lnsw;
	register index = 0;
	register len;
	register struct ifnet *ifp = &sc->is_if;

	int first, status;
	int ring_cnt, s;
#ifdef lint
	unit = unit;
#endif

	rptr = sc->rptr;
	/*
	 * Traverse the receive ring looking for packets to pass back.
	 * The search is complete when we find a descriptor that is in
	 * use (owned by Lance). 
	 */

	for ( ; ((RCV_PKT *)rptr->rcv_ring)[sc->rindex].flag != ETHER_EMPTY;
	      sc->rindex = ++index % nALNNRCV) {

		dp = (RCV_PKT *)&(rptr->rcv_ring[sc->rindex]);
		first = index = sc->rindex;


		/*
		 * If ETHER_OK then successful transfer
		 */
		if ( (dp->status == ETHER_OK) ) {
			len = dp->nrbuf;
			alnread(sc, sc->rlbuf[index], len, index);

			/*
			 * If we're on an architecture which provides DMA,
			 * re-initialize the descriptor to get a new buf.
			 */
			if (lnsw->ln_dodma)
				lnsw->lninitdesc (sc, &rptr->rcv_ring[index],
					   sc->rlbuf[index], 0,index);


		/*
		 * else Not a good packet sequence, check for receiver errors
		 */
		} else {
			status = dp->status;
			sc->is_if.if_ierrors++;

			if (status != ETHER_FIRMWARE) {
				if (lndebug)
					printf("ln%d: recv err %02x\n",unit, status&0xff);
				if (sc->ctrblk.est_recvfail != 0xffff) {
					sc->ctrblk.est_recvfail++;

					if (status == ETHER_CRC) {
				    		sc->ctrblk.est_recvfail_bm |= 1;
					}
					if (status == ETHER_FRAMING) {
					    	sc->ctrblk.est_recvfail_bm |= 2;
					}
				}
			} else {
				if (lndebug)
					printf("ln%d: firmware error\n", unit);
			}
		}
		/*
		 * Free the ring entry
		 */
		s = splimp();
		((RCV_PKT *)rptr->rcv_ring)[index].flag = ETHER_EMPTY;   
		RING_NI_DOORBELL;
		splx(s);
        }

}

/*
 * Ethernet output routine.
 * This routine calls alnstart to actually start the output.
 * The rest of the functionality that this routine used to have
 * has been moved up to net/if_ethersubr.c in OSF.
 */
alnoutput(ifp)
	register struct ifnet *ifp;
{
	alnstart(ifp->if_unit);
}
 
/*
 * Start output on interface.
 *
 */
/* SMP - call with device lock already set.  us.  */

alnstart(unit)
	int unit;
{
	register struct ln_softc *sc = aln_softc[unit];
	register struct mbuf *m;
	register int dp; 	/* data buffer pointer */
	register struct ln_ring *rptr = sc->rptr;
	register struct lnsw *lnsw = sc->lnsw;
	int tlen, s;
	int index;
	register struct ifnet *ifp = &sc->is_if;

	s = splimp();
	for (index = sc->tindex; sc->nxmit < (nALNNXMT - 1);
	     sc->tindex = index = ++index % nALNNXMT) {

		/*
		 * Dequeue the transmit request, if any.
		 */
		IF_DEQUEUE(&sc->is_if.if_snd, m);
		if (m == 0) {
			return;		/* Nothing on the queue	*/
		}
	
		/*
		 * "alnput" pads out anything less than "MINDATA" with NULL's
		 */
		tlen = alnput(sc, index, m);

		/* Give ownership to network interface */
		((TRN_PKT *)rptr->trans_ring)[index].ntbuf = tlen;
		((TRN_PKT *)rptr->trans_ring)[index].flag = ETHER_TRANSMIT_PACKET;
		RING_NI_DOORBELL;

		sc->nxmit++;
		ifp->if_flags |= IFF_OACTIVE;
		/*
		 * Accumulate statistics for DECnet
		 */
		if ((sc->ctrblk.est_bytesent + tlen) > sc->ctrblk.est_bytesent)
			sc->ctrblk.est_bytesent += tlen;
		if (sc->ctrblk.est_bloksent != (unsigned)0xffffffff)
			sc->ctrblk.est_bloksent++;
		sc->is_if.if_timer = 5;
	}
	splx(s);
}

/*
 * Process an ioctl request.
 */
alnioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	unsigned int cmd;
	caddr_t data;
{
	int ln_docrc();

	register struct ln_softc *sc = aln_softc[ifp->if_unit];
	register struct ln_initb *initb = &sc->ln_initb;
	register int i;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	struct ctrreq *ctr = (struct ctrreq *)data;
	/*struct protosw *pr;*/
	struct ifaddr *ifa = (struct ifaddr *)data;
	struct controller *ctlr = alninfo[ifp->if_unit];
	int bitpos;		/* top 6 bits of crc = bit in multicast mask */
	u_short newmask[4];	/* new value of multicast address mask */
	int j = -1, s, error=0;

	s = splimp();
	switch (cmd) {

	case SIOCENABLBACK:
		if (lndebug>1) printf("SIOCENABLBACK ");
		printf("ln: internal loopback requested\n");
		printf("ln: Unsupported on ADU\n");
		break;
 
	case SIOCDISABLBACK:
		if (lndebug>1) printf("SIOCDISABLBACK ");
		printf("ln: internal loopback disable requested\n");
		printf("ln: Unsupported on ADU\n");
		break;
 
	case SIOCRPHYSADDR:
		/*
		 * read default hardware address.
		 */
		if (lndebug>1) printf("SIOCRPHYSADDR ");
		bcopy(sc->is_addr, ifd->current_pa, 6);
		for ( i=0 ; i<6 ; i++ ) {
			ifd->default_pa[i] =
			(u_char)(tv_cnfg_base[ctlr->slot].tv_config_spec.tv_io_config.tv_ether_addr[i] & 0xFF);
		}
		break;
 
	case SIOCSPHYSADDR:
		if (lndebug>1) printf("SIOCSPHYSADDR ");
		bcopy(ifr->ifr_addr.sa_data, sc->is_addr, 6);
#ifdef NPACKETFILTER > 0
                pfilt_newaddress(sc->is_ed.ess_enetunit, sc->is_addr);
#endif /* NPACKETFILTER > 0 */



		if (ifp->if_flags & IFF_RUNNING) {
			alnrestart(ifp);
			alninit(ifp->if_unit);
		} else {
			alnrestart(ifp);
		}
		break;

	case SIOCDELMULTI:
	case SIOCADDMULTI:

		if (cmd == SIOCDELMULTI) {
			if (lndebug>1) printf("SIOCDELMULTI ");
			for (i = 0; i < NMULTI; i++)
				if (bcmp(&sc->multi[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
					if (--sc->muse[i] == 0) {
						bcopy(ln_sunused_multi,&sc->multi[i],MULTISIZE);
					}
					if (lnshowmulti) printf("%d deleted.\n",i);

				}
		} else {
			if (lndebug>1) printf("SIOCADDMULTI ");
			for (i = 0; i < NMULTI; i++) {
				if (bcmp(&sc->multi[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
					sc->muse[i]++;
					if (lnshowmulti) printf("already using index %d\n", i);
					goto done;
				}
				if (bcmp(&sc->multi[i],ln_sunused_multi,MULTISIZE) == 0)
					j = i;
			}
			/*
			 * j is initialized to -1; if j > 0, then
			 * represents the last valid unused location
			 * in the multicast table.
			 */
			if (j == -1) {
				printf("ln: SIOCADDMULTI failed, multicast list full: %d\n",NMULTI);
				error = ENOBUFS;
				goto done;
			}
			bcopy(ifr->ifr_addr.sa_data, &sc->multi[j], MULTISIZE);
			sc->muse[j]++;

			if (lnshowmulti)
				printf("added index %d.\n", j);
		}
		/*
		 * Recalculate all current multimask crc/bits
		 * and reload multimask info.
		 *
		 * For each currently used multicast address,
		 * calculate CRC, save top 6 bits, load
		 * appropriate mask bit into newmask[i]
		 */
		for (i=0; i<4; i++)
			newmask[i] = 0x0000;

		for (i=0; i<NMULTI; i++) {
			if (sc->muse[i] == 0)
				continue;
			/* returns 32-bit crc in global variable _ln_crc */
			ln_docrc(&sc->multi[i], 0, sc);
			bitpos = ((unsigned int)sc->ln_crc >> 26) & 0x3f;
			if (lnshowmulti)
				printf("crc=0x%x, bit=%d.\n",sc->ln_crc,bitpos);

			/* 0-15 */
			if (bitpos >= 0 && bitpos < 16)
				newmask[0] |= (1 << (bitpos - 0));
			/* 16-31 */
			else if (bitpos < 32)
				newmask[1] |= (1 << (bitpos - 16));
			/* 32-47 */
			else if (bitpos < 48)
				newmask[2] |= (1 << (bitpos - 32));
			/* 48-63 */
			else if (bitpos < 64)
				newmask[3] |= (1 << (bitpos - 48));
			else {
				if (lndebug || lnshowmulti)
				    printf("ln: bad crc, bitpos=%d.\n", bitpos);
			}
		}
		i = 0;
		for (i = 0; i < 4; i++)
			initb->ln_multi_mask[i] = newmask[i] & 0xffff;
		if (lnshowmulti) {
		   printf("new 64-bit multimask= 0x%04x 0x%04x 0x%04x 0x%04x\n",
			newmask[0], newmask[1], newmask[2], newmask[3]);
		}

		if (ifp->if_flags & IFF_RUNNING) {
			alnrestart(ifp);
			alninit(ifp->if_unit);
		} else {
			alnrestart(ifp);
		}
		break;

	case SIOCRDCTRS:
	case SIOCRDZCTRS:

		if (lndebug>1) printf("SIOCRDCTRS ");
		ctr->ctr_ether = sc->ctrblk;
		ctr->ctr_type = CTR_ETHER;
		ctr->ctr_ether.est_seconds = (time.tv_sec - sc->ztime) > 0xfffe ? 0xffff : (time.tv_sec - sc->ztime);
		if (cmd == SIOCRDZCTRS) {
			if (lndebug>1) printf("SIOCRDZCTRS ");
			sc->ztime = time.tv_sec;
			bzero(&sc->ctrblk, sizeof(struct estat));

		}
		break;

	case SIOCSIFADDR:
		if (lndebug>1) printf("SIOCSIFADDR ");
		ifp->if_flags |= IFF_UP;
		alninit(ifp->if_unit);
		switch(ifa->ifa_addr->sa_family) {
#ifdef INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			/*
			 * DISABLE
			 */
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif

		default:
			/*if (pr=iffamily_to_proto(ifa->ifa_addr.sa_family)) {
				error = (*pr->pr_ifioctl)(ifp, cmd, data);
			} */
			if (lndebug > 1)
			    printf("Unknown Protocol received\n");
			break;
		}
		break;
#ifdef  IFF_PROMISC     /* IFF_ALLMULTI and NPACKETFILTER, as well */
        case SIOCSIFFLAGS:
                if (lndebug > 1) printf("SIOCSIFFLAGS ");
                /*
                 * Restart the LANCE chip in case things have changed;
                 * someone may be trying to turn on (or off) promiscuous
                 * mode, for example.
                 */
                if (ifp->if_flags & IFF_RUNNING) {
                        alnrestart(ifp);
                        alninit(ifp->if_unit);
                } else {
                        alnrestart(ifp);
                }
                break;
#endif  /* IFF_PROMISC */

	default:
		error = EINVAL;
	}
done:	splx(s);
	return (error);
}

/*
 * Calculate 32-bit CRC (AUTODIN-II) for the given 48-bit
 * multicast address.  The CRC table must first be initialized
 * for the vax crc instruction to work.  The crc is returned in
 * variable ln_crc.
 */
static
ln_docrc(addr, flag, sc)
	struct ln_multi *addr;
	int flag;
	struct ln_softc *sc;
{
	unsigned int crc256();

	/*
	 * NOTE: do not change the order of these
	 * register declarations due to asm() instruction
	 * used below.
	 */
	register unsigned int *tbl_addr;	/* initialization table addr */
	register int inicrc;			/* initial CRC */
	register int len;			/* length of multicast addr */
	register struct ln_multi *multi_addr;	/* multicast address, 48 bits */
	register int *ln_crc = &sc->ln_crc;

	/* initialize polynomial table, only done once from alnprobe() */

	if (lnshowmulti) {
		printf("addr=%x.%x.%x.%x.%x.%x, ",
			(struct ln_multi *)addr->ln_multi_char[0],
			(struct ln_multi *)addr->ln_multi_char[1],
			(struct ln_multi *)addr->ln_multi_char[2],
			(struct ln_multi *)addr->ln_multi_char[3],
			(struct ln_multi *)addr->ln_multi_char[4],
			(struct ln_multi *)addr->ln_multi_char[5]);
	}

	/* initialize arguments */
	tbl_addr = ln_crc_table;
	inicrc = -1;
	len = 6;
	multi_addr = addr;

#ifdef lint
	tbl_addr = tbl_addr;
	inicrc = inicrc;
	len = len;
	multi_addr = multi_addr;
#endif

#ifdef vax
	/* calculate CRC */
	asm( "crc	(r11),r10,r9,(r8)" );
	asm( "movl	r0, (r7)" );
#else 
	/* Need to calculate CRC */
	*ln_crc = crc256(multi_addr, len);
#endif /* vax */

	return(0);
}


static int  crctbl[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D };

static unsigned int
crc256(addr, len)
	u_char *addr;
	int len;
{
	register unsigned int crc;

	crc = (u_int)0xffffffffL;
	while(len--)
                crc = (crc >> 8) ^ crctbl[*addr++] ^ crctbl[crc&0xff];

	return(crc);
}


/*
 * Restart the Lance chip -- this routine is called:
 *   - after changing the multicast address filter mask
 *   - on any loopback mode state change
 *   - on any error which disables the transmitter
 *   - on all missed packet errors
 *
 * The routine first halts the Lance chip, clears any previously
 * allcated mbufs, and then re-initializes the hardware (same as in
 * alnprobe() routine).  The alninit() routine is usually called next
 * to reallocate mbufs for the receive ring, and then actually
 * start the chip running again.
 */
alnrestart( ifp )
	register struct ifnet *ifp;
{
	register struct ln_softc *sc = aln_softc[ifp->if_unit];
	register int i, pi;
	register INIT_PKT *initpkt;
	int s;

	/*
	 * stop the chip
	 */
        s = splimp();

	/*
	 * stop network activity
	 */
	if (ifp->if_flags & IFF_RUNNING) {
		ifp->if_flags &= ~(IFF_UP | IFF_RUNNING);
	}
	lnrestarts++;

	if (lndebug)
		printf("alnrestart: restarted ln%d  %d\n",ifp->if_unit,lnrestarts);

	/*
	 * free up any mbufs currently in use
	 */
	for (i=0; i<nALNNXMT; i++) {
		if (sc->tmbuf[i])
			m_freem(sc->tmbuf[i]);
		sc->tmbuf[i] = (struct mbuf *)NULL;
	}

	if ( sc->lnsw->ln_dodma ) {
	/*
	 * free up any mbufs currently in use
	 */
		for (i=0; i<nALNNRCV; i++) {
			if (sc->rmbuf[i])
				m_freem(sc->rmbuf[i]);
			sc->rmbuf[i] = (struct mbuf *)NULL;
		}
	}

#ifdef  IFF_PROMISC
        if (ifp->if_flags & IFF_PROMISC) {
            /* should be in promiscuous mode */
            sc->ln_initb.ln_mode = ETHER_PROMISCUOUS;
        }
        else {
            /* should not be in promiscuous mode (might be leaving) */
            sc->ln_initb.ln_mode = ETHER_NORMAL;
        }
#endif  /* IFF_PROMISC */

	/*
	 * Resend Init Command
	 */
	initpkt = adu_send_init(&sc->ln_initb, sc);

	if(adu_check_init(initpkt) == 0) {
		printf("ADU_ln: Restart failed\n");
	}

        splx(s);

	return(0);
}
/*
 * Initialize a ring descriptor. Tie the buffer pointed to by
 * "bp" to the descriptor.
 */
alninitdesc (sc, tp, buf, option, index)
	register struct ln_softc *sc;
	register TRN_PKT *tp;
	register char **buf;
	int option;
	int index;
{
	register int dp;		/* data pointer */
	register struct lnsw *lnsw = sc->lnsw;

	bzero(tp, sizeof(TRN_PKT));
#ifdef MACH
	if (pmap_svatophys((caddr_t)buf, &tp->bufp) == KERN_INVALID_ADDRESS)
	{
	    printf("alninitdesc : can't get kva for buf\n");
	    return(0);
	}
#else
	tp->bufp = svtophy((caddr_t)buf);
#endif
	tp->flag = option;
	mb();
	return(0);
}

/*
 * Put an mbuf chain into the appropriate RAM buffer.
 */
alnput(sc, index, m)
	struct ln_softc *sc;
	int index;
	register struct mbuf *m;
{
	register caddr_t dp;
	int len = 0;

	sc->tmbuf[index] = m;	/* mbuf chain to free on xmit */
	while (m) {
		if (m->m_len == 0)
			goto next_m;
		dp = mtod(m, char *);
		bcopy(dp, 
		      (char *)sc->tlbuf[index] + len,
		      (unsigned)m->m_len); 
next_m:	
		len += m->m_len;
		m = m->m_next;
	}

	if (len < MINDATA) {
                bzero((char *)sc->tlbuf[index] + len,
		      MINDATA - len);
		len = MINDATA;
	}
	return (len);
}


/*
 * Pull read data off an interface.
 * Len is length of data, with local net header stripped.
 * Off is non-zero if a trailer protocol was used, and
 * gives the offset of the trailer information.
 * We first copy all the normal data into a cluster then deal 
 * with trailer.
 */
struct mbuf *
alnget(sc,rlbuf, totlen, off, index)
	struct ln_softc *sc;
	caddr_t rlbuf;
	int totlen,index;
	register off;
{
	struct mbuf *top, **mp, *m;
	int len;
	register caddr_t lrbptr = rlbuf;
	register struct lnsw *lnsw = sc->lnsw;

	top = 0;
	mp = &top;
	
	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == 0) {
printf("NO MGETHDR ");
	    return(0);
	}

	m->m_pkthdr.rcvif = &sc->is_if;

	m->m_pkthdr.len = totlen;
	m->m_len = MHLEN;

	/*
	 * If trailers are used get the header, first.
	 * We deal with the trailer-protocol header later in lnread()
	 */
	if (off) {
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0) {
printf("NO MCLGET ");
			return(0);
		}
		len = MIN(off, MCLBYTES);
		m->m_len = len;
		bcopy(lrbptr, mtod(m,caddr_t), (u_long)len);
		lrbptr += len;
		totlen -= len;
		*mp = m;
		mp = &m->m_next;
	}
		
	while (totlen > 0) {
		if (top) {
			MGET(m, M_DONTWAIT, MT_DATA);
			if (m == 0) {
				m_freem(top);
printf("NO MGET ");
				return(0);
			}
			m->m_len = MLEN;
		}
		len = totlen;

		if (len > MINCLSIZE) {
			MCLGET(m, M_DONTWAIT);
			if (m->m_flags & M_EXT)
				m->m_len = MCLBYTES;
		}

		if (len < m->m_len) {
			/*
			 * Place initial small packet/header at the end of mbuf.
			 * NOTE: max_linkhdr is the largest link level header.
			 */
			if (top == 0 && len + max_linkhdr <= m->m_len)
				m->m_data += max_linkhdr;
			m->m_len = len;
		}

		bcopy(lrbptr, mtod(m,caddr_t), (u_long)m->m_len);
		lrbptr += m->m_len;

		*mp = m;
		mp = &m->m_next;
		totlen -= m->m_len;
	}
	
#ifdef mips
	wbflush();
#endif
	return (top);
}

/*
 * Pass a packet to the higher levels.
 * We deal with the trailer protocol here.
 */
alnread(sc, rlbuf, len, index)
	register struct ln_softc *sc;
	register char *rlbuf;
	int len,index;
{
	register struct ether_header *eptr, eh;
	register struct lnsw *lnsw = sc->lnsw;	/* ptr to switch struct */
    	struct mbuf *m;
	/* struct protosw *pr; */
	int off, resid;

	/*
	 * Deal with trailer protocol: if type is INET trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
	{
		if ( lnsw->ln_dodma && (m= sc->rmbuf[index]) && sc->dma[index] )
		{
                      	eptr = mtod(m, struct ether_header *);
			m->m_data += sizeof( struct ether_header );
		} else {	
			if ( lnsw->ln_dodma && !m ) 
	 			/* Couldn't get an mbuf */
				return;
			eptr = &eh;
			bcopy(rlbuf, eptr, sizeof(struct ether_header));
			rlbuf += sizeof(struct ether_header);
		}

		/*
		 * Done with ether_header; drop len
		 */
		len -= sizeof(struct ether_header);
	}

 	if (!(sc->is_if.if_flags & IFF_PROMISC) ) { 
	  /*
	   * Make sure our multicast address filter doesn't hand us
	   * up something that doesn't belong to us!
	   */
	  if ((eptr->ether_dhost[0] & 1) && (bcmp ((caddr_t)eptr->ether_dhost,
	          (caddr_t)etherbroadcastaddr,MULTISIZE))) {
	    int i;
	    for (i = 0; i < NMULTI; i++) {
		if (sc->muse[i] &&
		    !(bcmp (&sc->multi[i],eptr->ether_dhost,MULTISIZE)))
			break;
	    }
	    if (i == NMULTI ) { 
			if ( lnsw->ln_dodma ) 
				m_freem(sc->rmbuf[index]);
			return;
	    }
	  }
	}
	sc->is_if.if_ipackets++;

	eptr->ether_type = ntohs((u_short)eptr->ether_type);
	if ((eptr->ether_type >= ETHERTYPE_TRAIL &&
	    eptr->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER)) {
		off = (eptr->ether_type - ETHERTYPE_TRAIL) * 512;
		if (off >= ETHERMTU) {
			if ( lnsw->ln_dodma ) 
				m_freem(sc->rmbuf[index]);
			return;		/* sanity */
		}
		{
		 	if(m = lnsw->lnget(sc, rlbuf, len, off, index)) {
		     		eptr->ether_type =
					ntohs( *mtod(m, u_short *));
				resid = ntohs( *(mtod(m, u_short *)+1));
				if (off + resid > len) {
					if ( lnsw->ln_dodma ) 
						m_freem(sc->rmbuf[index]);
					 return;		/* sanity */
				}
				len = off + resid;
			}	
			else /* can't get mbuf */
				return;
		}
	} else {
		off = 0;

	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; lnget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
		if(lnsw->ln_dodma && sc->dma[index])
			m->m_len = len;
		else
			m = lnsw->lnget(sc, rlbuf, len, off, index);
	}

	if (m == 0) {
		return;
	}

	if (off) {
		m->m_data += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
	}

	/* Dispatch this packet */
	ether_input(&sc->is_if, eptr, m);
}

/*
 * alntint hasn't been called in a while, so restart chip,
 * and reset timer.
 */
alnwatch(unit)
	int unit;
{
	register struct ln_softc *sc = aln_softc[unit];
 	register struct ifnet *ifp = &sc->is_if;
	int s;

	s=splimp();

	ifp->if_timer = 0;
	sc->ln_debug.trfull++;
	
	alnrestart(ifp);
	alninit(ifp->if_unit);
	splx(s);
}




INIT_PKT *
adu_send_init(initb, sc)
	struct	ln_initb *initb;
	struct	ln_softc *sc;
{
	register INIT_PKT *initpkt;
	register struct ln_ring *rptr = sc->rptr;
	int i;

	
	if (sc->nxmit == (nALNNXMT - 1))
		panic("No entries to init ln\n");
	initpkt = (INIT_PKT *)&(rptr->trans_ring[sc->tindex]);
	sc->tindex = sc->tindex++ % nALNNXMT;

	initpkt->rxmode = initb->ln_mode;
	/* 
	 * Initialize multicast address array
	 */
	for (i=0; i<6; i++) {
		initpkt->lamask[i] = sc->ln_initb.ln_multi_mask[i] & 0xffff;
	}

	/*
	 * Initialize hardware address
	 */
	for (i=0; i<6; i++) {
		initpkt->rxaddr[i] = sc->is_addr[i];
	}

	initpkt->flag = ETHER_INITIALIZE; /* Must be last as this is the */
	                                  /* ownership semaphore	 */

	RING_NI_DOORBELL;
	return(initpkt);
}

int
adu_check_init(initpkt)
	INIT_PKT *initpkt;
{
	while(initpkt->flag != ETHER_EMPTY)
		mb(); /* Wait for command to complete */

	if(initpkt->status) {	/* Non-Zero means failure */
		printf("ADU Ethernet initialize failure: code = %x\n",
		       initpkt->status);
		return(0);
	}
	return( sizeof(struct ln_initb));
}

int
adu_init_bufs(sc)
	struct ln_softc *sc;
{
	register int 	i;
	char		*bptr;

	if(sc->callno == 1) return(1);


	/* In order to get buffers on 32-byte alignment a big chunk is
	 * allocated and individual buffers are carved from the big chunk.
	 * In order for this to work, the buffer size must be a multiple of
	 * 32.  It is currently 1536.
	 */

#ifdef MACH
	bptr = (char *)kmem_alloc(kernel_map, ((nALNNRCV + nALNNXMT) *  LN_BUF_SIZE));
#else
	KM_ALLOC(bptr, char *, ((nALNNRCV + nALNNXMT) *  LN_BUF_SIZE),
				 KM_DEVBUF, KM_NOW_CL_CO_CA | KM_NOCACHE);
#endif
	if(bptr == (char *)NULL)
		return(0);
	for (i=0; i < nALNNXMT; i++) {
		sc->tlbuf[i] = bptr;
		bptr += LN_BUF_SIZE;
	}
	for (i=0; i < nALNNRCV; i++) {
		sc->rlbuf[i] = bptr;
		bptr += LN_BUF_SIZE;
	}
	return(1);
}

/*
 * dummy routines to define symbols
 */

#endif  /* NLN > 0 || defined(BINARY) */

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
static char *rcsid = "@(#)$RCSfile: if_fza.c,v $ $Revision: 1.2.15.11 $ (DEC) $Date: 1993/12/07 21:07:48 $";
#endif

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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/


/*-----------------------------------------------------------------------
 * Modification History: 
 *
 * 05-May-92    chc
 *      Change codes in fzacpy routine to fix the odd byte alignment 
 *      problem which is caused by the new compiler. 
 *      
 * 09-Mar-92	tlh
 *	Ported to Alpha OSF/1 Platform (DEC/3000 - M500).
 *
 * 06-Feb-92    chc
 *	Added the ALLMULTICAST support
 *
 * 05-Jun-91	chc
 *	Ported to OSF/1 Platform. Support SIOCIFSETCHAR ioctl
 *
 * 19-Dec-90   chc
 * 	Fixed the counter initialization problem. Also allow to
 *	read counter while the adapter is in the MAINTENANCE
 *      state.
 *
 * 31-Oct-90   chc
 *	Added code to fix the SMT chained and SMT 6.2 duplicated
 *	address detected problems. Also added code to support the
 *	read status. 
 *
 * 5-Oct-90	chc
 *	Added code to sync the firmware and fixed the counter.
 *
 * 7-Sep-90	chc
 *	Added code to support FDDI MIB
 *
 * 9-Aug-90	chc (Chran-Ham Chang)
 *	Bugs Fixed and support the EEPROM update
 *
 * 27-Apr-90 	chc (Chran-Ham Chang)
 *	Created the if_fza.c module   
 *	Derived from if_xna.c.
 *
 *---------------------------------------------------------------------- */
#include "fza.h"

#if NFZA > 0 || defined(BINARY)

/* #include "packetfilter.h"   */    /* NPACKETFILTER */
#include <data/if_fza_data.c>

#define FZASIZE         4096	
#ifdef __alpha
/*
 *  The R3000 preserves write ordering of all requests; the
 *  write buffer is a FIFO.  Alpha does not guarantee preservation
 *  of write ordering.  So memory barriers are required between
 *  multiple writes to I/O registers (and memory buffers) where
 *  order is important.
 *  To enforce a desired order, we define the primitive FZIOSYNC().
 *
 *  Also, the effect of a MIPS writebuffer flush is achieved by
 *  alpha's memory barrier.
 */
#define FZIOSYNC()	mb()
#else
#define FZIOSYNC()	/* nop */
#endif /* __alpha */

#ifdef __alpha
/*
 *  These macros convert a pointer in heap space to a unique 32-bit
 *  quantity and vice versa.  Used for squirreling away mbuf pointers
 *  in xmit rings.  Ideally, should probably store mbuf pointers
 *  someplace else.
 */
#define FZ_HEAPTO32(x)	\
		((x)?(vm_offset_t)(x)-VM_MIN_KERNEL_ADDRESS:(vm_offset_t)(x))

#define FZ_32TOHEAP(x)	\
		((x)?(vm_offset_t)(x)+VM_MIN_KERNEL_ADDRESS:(vm_offset_t)(x))

#else

#define FZ_HEAPTO32(x)	((vm_offset_t)(x))
#define FZ_32TOHEAP(x)	((vm_offset_t)(x))

#endif /* __alpha */

extern	struct protosw *iftype_to_proto(), *iffamily_to_proto();
extern	struct timeval	time;
extern  fddi_units ;

int	fzaattach(), fzaintr(),fzaoutput(),fzaprobe(),fzastart();
int	fzainit(),fzaioctl(),fzareset(),fzawatch();

struct  driver fzadriver =
	{ fzaprobe, 0 ,fzaattach, 0 , 0, 0, 0,0,"fza", fzainfo };

extern  int ether_output();
struct	mbuf *fzamkparam();
FZACMDRING *fzamkcmd();
u_char  fza_dbeacon[] = {0x01, 0x80, 0xc2, 0x00, 0x01, 0x00};
u_char  fza_rpurge[] = {0x09, 0x00, 0x2b, 0x02, 0x01, 0x05};
u_char  fza_notset[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int fzadebug = 0;	/* Make sure FZADEBUG is defined in if_fzareg.h */
int dludebug = 0;	/* Make sure FZADEBUG is defined in if_fzareg.h */

#define XMTSEGSIZE	512

#define FZAMCLGET(m,head) { \
	if(head) { \
		MGETHDR((m), M_DONTWAIT, MT_DATA) \
	} else {\
		MGET((m), M_DONTWAIT, MT_DATA) \
	} \
	if ((m)) { \
		MCLGET((m), M_DONTWAIT) \
		if (((m)->m_flags & M_EXT) == 0) { \
			m_freem(m); \
			(m) = (struct mbuf *)NULL; \
		} \
	} \
}

#define	FZATIMEOUT(cp,timeout ) { \
	register int s = splnet(); \
	register int sec0 = time.tv_sec; \
	while ( !(cp->cmd_own) && ((time.tv_sec - sec0) < 15 )); \
	if ( !(cp->cmd_own) ) \
		timeout = 1; \
	splx(s); \
}

/*
 * Probe the FZA to see if there 
 * 
 */
fzaprobe(reg,ctlr)
	caddr_t reg;
	struct	controller *ctlr;
{
	register struct	fza_softc *sc;
	register FZARCVRING *rp;
	register FZAXMTRING *tp;
	struct rmbuf *bp;
	struct mbuf *m;
	int unit = ctlr->ctlr_num;
	caddr_t mp;
	int i;

	/*
	 *  Allocate memory for softc structure.
	 */
	sc=(struct fza_softc *)kmem_alloc(kernel_map, sizeof(struct fza_softc));
#if FZADEBUG
	if (fzadebug)
		printf("fza0: fza_softc is <0x%lx>\n", (vm_offset_t)sc);
#endif
	if (!sc)
		return(0);
	bzero((char *)sc, sizeof(struct fza_softc));
	fza_softc[unit] = sc;


#ifdef __alpha
	sc->basereg = (vm_offset_t)reg;
#else
	sc->basereg = PHYS_TO_K1((u_long)reg);
#endif
	/* fill out the register address and others */
	sc->resetaddr = (struct fzareg  *)(sc->basereg + FZA_RESET);
	sc->ctlaaddr = (struct fzareg *)(sc->basereg + FZA_CTL_A);
	sc->ctlbaddr = (struct fzareg *)(sc->basereg + FZA_CTL_B);
	sc->intraddr = (struct fzareg *)(sc->basereg + FZA_INT_EVENT);
	sc->maskaddr = (struct fzareg *)(sc->basereg + FZA_INT_MASK);
	sc->statusaddr  = (struct fzareg *)(sc->basereg + FZA_STATUS );

#ifdef __alpha
	/*
	 *  basereg initially is a "sparse space" address.  It
	 *  is used above to set up CSR pointers.  From here on
	 *  it is used as a base for buffers and rings.
	 */
	sc->basereg = (vm_offset_t)(PHYS_TO_KSEG(DENSE(KSEG_TO_PHYS(reg))));
#endif

	/* fill out the cmd and unsolicited ring address */
	sc->cmdring = (FZACMDRING *)(sc->basereg + FZACMD_PHY_ADDR);
	sc->unsring = (FZACMDRING *)(sc->basereg + FZAUNS_PHY_ADDR); 


	/* turn on the driver active mode */
#ifdef __alpha
	sc->reg_ctlb = (u_long)((0x0CL<<32)|((u_long)(FZA_ACTIVE<<16)));
#else
	sc->reg_ctlb = FZA_ACTIVE ;
#endif
	FZIOSYNC();		/* guarantee write ordering */

	/* Clear the interrupt event register */
	FZCSR_WR(sc->reg_mask, ((FZA_INTR_MASK)&0x0ffff));
	FZIOSYNC();
	FZCSR_SET(sc->reg_intr,0);
	WBFLUSH();

	/*
	 * perform the adapter self test
	*/

	if(!fzaselftest(sc,unit)) {
		printf("fza%d: fzaprobe selftest fail\n",unit);
		return(0);
	}

	/* set the interrup mask */
#ifdef mips
/*
 * Revisit this later.  Why doesn't this work on alpha?
 */
	FZCSR_WR(sc->reg_mask, ((FZA_INTR_MASK)&0x0ffff));
	FZIOSYNC();
#endif


	/*
	 * Allocate space for the ctrblk, initblk and statusblk
	 */
	mp = (caddr_t)kmem_alloc(kernel_map, 512 * 3);
	if (!mp) {
		printf("fza%d: Couldn't allocate memory for internal structure\n", unit);
		return(0);
	}
	sc->ctrblk = (struct _fzactrs *)mp;
	sc->initblk =  (struct fzainit *)(mp + 512);
	sc->statusblk = (struct fzastatus *)(mp + 512 * 2);

	fzaindex_init(sc);

	sc->initflag = 1; /* turn on the flag only once */

	/*
	 * clear the receive mbuf entries
	 */
        for (i = 0, bp = &sc->rmbuf[i]; (i < NFZARCV ); i++, bp++) {
                bp->mbufa = 0 ; 
		bp->mbufb = 0 ;
	}
	/*
	 * We are now guaranteed that the port has succesfully completed
	 * self test and is in the "UNINITIALIZED" state. At this point,
	 * we can prepare a command buffer to issue the INIT command
	 * 
	 */
	if(!fzacmdinit(sc,unit)) {
		printf("fzaprobe init command fail\n",unit);
		kmem_free(kernel_map,mp, 512 * 3 );
		return(0);
	}

	FZCSR_SET(sc->reg_intr,0);
	FZIOSYNC();
	/* 
	 * get the initial value from the INIT command
	 * fill out the init value 
	 */
	fzafillinit(sc,sc->initblk);

	FZCSR_WR(sc->reg_mask, ((FZA_INTR_MASK)&0x0ffff));
	FZIOSYNC();

	/*
	 * put two multicast addresses :
	 *
 	 *	Ring Purger	: 09-00-2b-02-01-05
	 *	Directed Beacon : 01-80-c2-00-01-00
	 */
	bcopy(fza_rpurge,&sc->is_multi[0][0],6);
	bcopy(fza_dbeacon,&sc->is_multi[1][0],6);
	sc->is_muse[0] = sc->is_muse[1] = 1;
	return (sizeof(struct fza_softc));

fail:
	kmem_free(kernel_map, mp, 512 * 3);
	for (i = 0, bp = &sc->rmbuf[0]; (i < sc->nrecv ); i++, bp++) {
		if(bp->mbufa != NULL)
			m_freem(bp->mbufa);
		if(bp->mbufb != NULL)
			m_freem(bp->mbufb);
	}
	printf("fza%d: fzaprobe fail - can not allocate enough receive buffer\n",unit);
	return(0);
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
fzaattach(ctlr)
	struct controller *ctlr;
{
	register struct fza_softc *sc = fza_softc[ctlr->ctlr_num];
	register struct ifnet *ifp = &sc->is_if;
	register struct sockaddr_in *sin;
	register int i;

	sc->is_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
	sc->is_ac.ac_arphrd = ARPHRD_ETHER ;
	ifp->if_unit = ctlr->ctlr_num;
	ifp->if_name = "fza";
	ifp->if_mtu = FDDIMTU;
	ifp->if_mediamtu = FDDIMAX;
	ifp->if_flags |= IFF_SNAP| IFF_BROADCAST|IFF_MULTICAST| IFF_NOTRAILERS | IFF_SIMPLEX ;
	ifp->if_type = IFT_FDDI;
	ifp->if_addrlen = 6; 
	ifp->if_hdrlen = sizeof(struct fddi_header) + 8 ; /* LLC header is 8 octects */
	((struct arpcom *)ifp)->ac_ipaddr.s_addr = 0;

	/*
	 * Initialize multicast address table
	 */
	for (i = 2; i < NMULTI; i++) {
		bcopy((caddr_t)etherbroadcastaddr, sc->is_multi[i], 6);
		sc->is_muse[i] = 0;
	}

	/*
	 * Set maxlen for the SMT receive queue; attach interface
	 */
	sc->is_smt.ifq_maxlen = IFQ_MAXLEN;
	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;
	ifp->if_output = ether_output;
	ifp->if_start = fzaoutput;
	ifp->if_init = fzainit;
	ifp->if_ioctl = fzaioctl;
	ifp->if_reset = fzareset;
	ifp->if_watchdog = fzawatch;
	ifp->if_timer = 1;
/*	ifp->d_affinity = ALLCPU; */
	ifp->if_baudrate = FDDI_BANDWIDTH_100MB;

	/*
	 * Hook up the device interrupt vector to begin allowing interrupts.
	 * We know that the device has been successfully initialized at this
	 * point. Initialize if_version string.
	 */

	ifp->if_version = "DEC DEFZA FDDI Interface";

	printf("fza%d: %s, hardware address %s ROM rev %c%c%c%c Firmware rev %c%c%c%c\n", 
		ifp->if_unit, ifp->if_version, ether_sprintf(sc->is_dpaddr),
		sc->phy_rev[0],sc->phy_rev[1],sc->phy_rev[2],sc->phy_rev[3],
		sc->fw_rev[0],sc->fw_rev[1],sc->fw_rev[2],sc->fw_rev[3]);
	attachpfilter(&(sc->is_ed));
	if_attach(ifp);
	fddi_units ++ ; 	/* number of FDDI adapter */
}

/*
 * Initialize interface. May be called by a user process or a software
 * interrupt scheduled by fzareset().
 */
fzainit(unit)
	int unit;
{
	register struct fza_softc *sc = fza_softc[unit];
	register FZACMDRING *cp; 
	register struct ifnet *ifp = &sc->is_if;
	int s, t,timeout=0;

	char *initfail = "port init failed";
	char *err = (char *)0;

	/* not yet, if address still unknown */

	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;
	if (ifp->if_flags & IFF_RUNNING)
		return;

	/*
	 * Lock the softc to ensure coherency of softc data needed to fill
	 * out the command buffers by "fzamkcmd". 
	 */
	t = splnet();
	s = splimp();

	/* clear the interrupt event register */
	FZCSR_SET(sc->reg_intr,0);
	WBFLUSH();



	/*
	 * Issue the MODCAM command to set up the CAM addresses
         */
	if(cp=fzamkcmd(sc,CMD_MODCAM,unit)) {
                splx(s);
                FZATIMEOUT(cp,timeout)
                s = splimp();
                if(timeout) {
			err = "port modify CAM command timeout";
                        goto done;
		} else if ( cp->status_id != CMD_SUCCESS ) {
			err = initfail;
                        goto done;
                }
        } else {
		err = initfail;
                goto done;
        }

	/*
	 * Issue a "PARAM" command to fill in the initial  
	 * system parameters. Wait for completion,
	 * which will be signaled by the interrupt handler.
	 */
	if(cp=fzamkcmd(sc, CMD_PARAM, unit)) {  
                splx(s);
                FZATIMEOUT(cp,timeout)
                s = splimp();
                if(timeout) {
                        err = "port PARAM command timeout";
                        goto done;
                } else if ( cp->status_id != CMD_SUCCESS ) {
			err = initfail;
                        goto done;
                }
        } else {
		err = initfail;
                goto done;
        }


	if (ifp->if_flags & IFF_PROMISC) {
		if (cp =fzamkcmd(sc, CMD_MODPROM,ifp->if_unit)) {
			splx(s);
			FZATIMEOUT(cp,timeout)
			s = splimp();
			if(timeout) {
				err = "port MODPROM command timeout";
				goto done;
			} else if (cp->status_id != CMD_SUCCESS) { 
				err = initfail;
                        	goto done;
			}
		} else {
			err = initfail;
                        goto done;
		}
	}
	/*
	 * Mark interface up and running; start output
	 * on device.
	 */
	sc->is_if.if_flags |= IFF_RUNNING;
	sc->is_if.if_flags &= ~IFF_OACTIVE; 
	if(sc->initflag) {
		sc->ztime = time.tv_sec;
		sc->initflag = 0;
	}
	if (sc->is_if.if_snd.ifq_head)
		fzastart(unit);		/* queue output packets */
done:
	if (err)
		printf("fza%d: %s\n", unit, err);
	/*
	 * Relinquish softc lock, drop IPL.
	 */
	splx(s); 
	splx(t);

#if FZADEBUG
	if(fzadebug)
		PRT_REG(sc)
#endif
	return;
}

fzaoutput(ifp)
	register struct ifnet *ifp;
{
	fzastart(ifp->if_unit);
}

/*
 * FZA  start routine. Strings output packets and SMT output packets  
 * onto the transmit ring. 
 */
fzastart(unit)
	int unit;
{
	register FZAXMTRING  *tp;
	register struct mbuf *m;
	register int index;

	struct fza_softc *sc = fza_softc[unit];
	struct ifnet *ifp = &sc->is_if;
	FZAXMTRING *tpstart;
	struct mbuf *m0;
	int last,totlen;
	u_long tmp;
	/*
	 * Process the transmit queues.
	 */
	for (index = sc->tindex, tp = &sc->tring[index] ; index !=
sc->ltindex && !(tp->own & FZA_RMC_OWN) ;
	     index = ++index % sc->nrmcxmt , tp = &sc->tring[index]) {
next_m:
		if (sc->is_if.if_snd.ifq_head ) {
			IF_DEQUEUE(&sc->is_if.if_snd, m0);
		} else  {
			sc->tindex = index;
			return;
		}

		/* 
		 * get the total packet size and check the owner ship
 		 * bit for the required number of RMC descriptors
		 */
		if (!(m0->m_flags & M_PKTHDR)) {
			/*
			 * drop this packet for not having
			 * the packet header
			 */
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: Not having the packet header mbuf 0x%lx\n",unit,m0); 
#endif
			m_freem(m0);
			goto next_m;
		}
		/*
		 * two bytes Preamble and one byte starting delimiter 
		 */
		if( (totlen = m0->m_pkthdr.len) > FDDIMAX )  {
			/* 
			 * drop this packet for size too long *
			 */ 
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: size too long %d\n",unit,totlen);
#endif
			m_freem(m0);
			goto next_m;
		}

		/* 
		 * the last descriptor not own by host
		 * put the mbuf back to the queue 
		 */
		last = index + totlen/XMTSEGSIZE - (((totlen % XMTSEGSIZE) != 0)? 0 : 1 );
		if ( sc->tring[last % sc->nrmcxmt].own == FZA_RMC_OWN ) {
			IF_PREPEND(&sc->is_if.if_snd, m0); 
			sc->tindex = index;
#if FZADEBUG
			if(fzadebug)
				printf("fza descriptor not own by host\n");
#endif
			return;
		}


		/*
		 * String the mbuf chain onto the transmit ring entry's
		 * buffer segment addresses.
		 * Architectures which need to use program I/O to data copy
		 * mbuf chain into one or several ( maximum 9 ) RMC transmit
		 * rings which are 512-byte segments.
		 */
#if FZADEBUG
		if(fzadebug > 1 )
			printf("fzastart: ready to copy data len %d\n",totlen);
#endif
		tpstart = tp;
		{
		register int tleft = 512 , mleft;
		register  caddr_t mptr, tptr,pt;
		int i,j,k ;

		int space;
		int nring = 1;

		/*
		 *	rleft : space left for current RMC XMT ring
		 *	mleft : bytes left for copying in current mbuf 
		 */

		m = m0;
		tptr = (caddr_t)FZAXMTADDR(tp,sc->basereg); 
		while (m) {
			mleft = m->m_len ;
			mptr = mtod(m, caddr_t);
			while (mleft) { 
				space = MIN(tleft,mleft); 
				fzacpy(mptr, tptr , space);
				tptr   +=  space ;
				mptr   += space ;
				mleft  -= space  ;  
				tleft  -= space ;
				if (tleft == 0 && mleft > 0 ) {
					index = ++index % sc->nrmcxmt ;
					sc->tring[index].own = FZA_RMC_OWN;
					sc->tring[index].rmc = 0;
					if(index == 0 )
						tptr = (caddr_t)FZAXMTADDR(&sc->tring[index],sc->basereg);
					tleft = 512;
					nring++;
				}
			}

			m = m->m_next;
		}

		/*
		 * need more than one XMT rings
		 */
		if( nring > 1 ) { 
			sc->tring[index].rmc = FZA_EOP ;
			tpstart->rmc = FZA_SOP | FZA_XMT_VBC | totlen ;
		} else
			tpstart->rmc = FZA_SOP | FZA_EOP | FZA_XMT_VBC | totlen;
		/*
		 * save the mbuf chain
		 */
		tpstart->xmt_mbuf = (unsigned int)(FZ_HEAPTO32(m0));
		FZIOSYNC();		/* make ring visible before ownership */
		tpstart->own = FZA_RMC_OWN;
		FZIOSYNC();		/* make ownership visible */

		/* 
		 * this is try to solve the RMC pipe line problem
		 */
		tmp = tpstart->own ; 

#if FZADEBUG
		if(fzadebug > 1)
			printf("fzastart index %d rmc 0x%8x\n",index,tpstart->own);
#endif

		}
		sc->nxmit++;
		ifp->if_flags |= IFF_OACTIVE ;
		/*
		 * Advise the port that a new transmit packet is pending
		 */
		FZCSR_SET(sc->reg_ctla,XMT_POLL);
		WBFLUSH();

	}
	sc->tindex = index;
}


/*
 * FZA device interrupt handler
 */
fzaintr(unit)
	int unit;
{
	register struct fza_softc *sc = fza_softc[unit];
	register FZASMTRING *smttp;
	register FZAXMTRING *tp;
	register int index;

	struct mbuf *m0;
	struct ifnet *ifp = &sc->is_if;
	u_short csr ; 
	unsigned short port_status;

	int s = splimp();

	FZIOSYNC();	/* guarantee visibility of updated shared memory data */

	csr = FZCSR_RD(sc->reg_intr);

	/* clear the interrupt event register */
	FZCSR_WR(sc->reg_intr,csr);
	FZIOSYNC();	/* flush to TURBOchannel */

	/*
	 * Lock softc, since we will be updating the per-unit ring pointers
	 * and active ring entry counts frequently.
	 */

	/*
	 * See if we got here due to a port state change interrupt. 
	 */
	if (csr & STATE_CHG )  {
		FZIOSYNC();
		port_status = FZCSR_2RD(sc->reg_status);
		switch ((port_status & ADAPTER_STATE)) {    

		case STATE_HALTED :
			printf("fza%d: port in the Halt State -> ",unit);
			switch(port_status & ID_FIELD ) {

			case HALT_UNKNOWN:
				printf("Unknown reason\n");
				break;

			case HALT_HOST_DIRECTED:
				printf("Host request to halt\n");
				break;

			case HALT_HBUS_PARITY:
				printf("Host Maxbus parity errors\n");
				break;

			case HALT_HNXM:
				printf("Host Non-exist Memory\n");
				break;

			case HALT_ADAP_SW_FAULT:
				printf("Adapter software fault\n");
				break;

			case HALT_ADAP_HW_FAULT:
				printf("Adapter hardware fault\n");
				break;

			case HALT_CNS_PC_TEST:
				printf("CNS PC trace path test\n");
				sc->flag = FZA_PC_TRACED ;
				break;

			case HALT_CNS_SW_FAULT:
				printf("CNS software fault\n");
				break;

			case HALT_CNS_HW_FAULT:
				printf("CNS hardware fault\n");
				break;

			default:
				printf("unknown halt id %d\n",
					port_status & ADAPTER_STATE);
				break;
			}
#ifdef __ERRLOG__
			{
				struct el_rec *elrp;

				if((elrp = ealloc(sizeof(struct el_fza), EL_PRILOW))) {
					struct el_fza *elbod = &elrp->el_body.el_fza;
					bcopy(((char *)(sc->basereg + FZA_DLU_ADDR )),
						elbod,sizeof(struct el_fza));
					LSUBID(elrp,ELCT_DCNTL,ELFZA,0,0,unit,(port_status & ID_FIELD));
				} 
				EVALID(elrp);
			}
#endif
			/*
			 * This is used for HALT recovrey.
			 * It will bring back the
			 * RUNNING state that help the
			 * link unavilable before the 
			 * adapter change to HALT state
			 * case.
			 */
			if (!(ifp->if_flags & IFF_RUNNING)) 
				ifp->if_flags |= IFF_RUNNING ;
			fzareset(unit);
			break;

		case STATE_RESET:
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: port in the Resetting State\n",unit);
#endif
			break;

		case STATE_UNINITIALIZED:
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: port in the Uninitialized State\n",unit);
#endif
			break; 

		case STATE_INITIALIZED:
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: port in the Initialized State\n",unit);
#endif
			break;

		case STATE_RUNNING:
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: port in the Running State\n",unit);
#endif
			break;

		case STATE_MAINTENANCE:
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: port in the Maintenance State\n",unit);
#endif
			break;

		default: 
			printf("fza%d: Undefined state id %d\n", unit, port_status & ADAPTER_STATE  );
			break;
		}
		/*
		 * turn off the interface because of unrecovery errors
		 */
		splx(s);
		return;
	}


	/*
	 * See if we get the FLUSH_TX interrupt. If so, the driver
	 * will return the ownership of all pending SMT XMT buffer back
	 * to the adapter and turn on the DTP bit in all the Tx descriptors 
	 * which is owned by the RMC 
	 */
	if(csr & FLUSH_TX) {

#if FZADEBUG
		if(fzadebug)
			printf("fza%d: flush the XMT buffer\n",unit);
#endif
		for ( index = sc->tsmtindex, smttp = &sc->smttring[index] ;
				 (smttp->own & FZA_HOST_OWN); 
			index = index++ % sc->nsmtxmt,smttp = &sc->smttring[index] )  
				smttp->own &= ~(FZA_HOST_OWN);
		sc->tsmtindex = index;
		FZIOSYNC();		/* make ownership visible to adapter */

		/*
		 * Clean up the Tx ring
		 */ 

		for ( index = sc->tindex - 1  , tp = &sc->tring[index] ; 
			(index != sc->tindex) && (tp->own & FZA_RMC_OWN) ;) { 

			tp->rmc |= FZA_XMT_DTP ;
			if(--index < 0 )
				index =  sc->nrmcxmt-1;
			tp = &sc->tring[index];
		}

		sc->ltindex = sc->tindex - 1 ;
		sc->nxmit = 0;
		FZIOSYNC();	/* make ring changes visible to adapter */
		/*
		 * notice adapter the flush was done
		 */

		FZCSR_SET(sc->reg_ctla,FLUSH_DONE);
		WBFLUSH();
		splx(s);
		return;
	}


	/*
	 * See if we get the LINK STATUS CHANGE interrupt. If so,
	 * we need to turn off or turn on the interface 
	 */
	if (csr & LINK_STATUS_CHG) {
#if FZADEBUG
		if(fzadebug)
			printf("fza%d: LINK STATE CHANGE\n",unit);
#endif
		if (FZCSR_2RD(sc->reg_status) & LINK_AVAILABLE ) {
			if (!(ifp->if_flags & IFF_RUNNING)) {
				ifp->if_flags |= IFF_RUNNING ;
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: LINK available\n",unit);
#endif
			}
		} else {
			if (ifp->if_flags & IFF_RUNNING ) {
				ifp->if_flags &= ~IFF_RUNNING;
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: LINK unavailable\n",unit);
#endif
				/* 
		 		 * drop packets on the transmit queue
		 		 */
				while(sc->is_if.if_snd.ifq_head) {
					IF_DEQUEUE(&sc->is_if.if_snd, m0);
					m_freem(m0);
				}
			} 
		}
	}


	if (csr & RCV_POLL)  /* receive interrupt */
		fzarint(unit);

	if (csr & XMT_PKT_DONE) /* transmit done interrupt */ 
		fzatint(unit);

	if (csr & CMD_DONE ) /* command done interrupt */ 
		fzacmdint(unit);

	if (csr & SMT_XMT_POLL) /* SMT transmit done interrupt */ 
		fzasmtint(unit);

	if (csr & UNS_POLL ) {  /* unsolicited event interrupt */
		register int index;
		register FZAUNSRING *up;

		for(index = sc->unsindex, up= &sc->unsring[index];
				(up->cmd_own) ; index = ++index % NFZAUNS, 
					up = &sc->unsring[index] ) {
			/*
			 * process the unsolicited ring
			 */
			switch(up->status_id) {

			case UNS_UNDEFINED:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Unsolicited Event Undefined\n",unit);
#endif
				break;

			case UNS_RINGINIT:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Ring Init Initiated\n",unit);
#endif
				break;

			case UNS_RINGINITRCV:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Ring Init Received\n",unit);
#endif
				break;

			case UNS_RINGBEACONINIT:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Ring Beaconing Initiated\n",unit);
#endif
				break;

			case UNS_DUPADDR:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Duplicate Address Detected\n",unit);
#endif
				break;

			case UNS_DUPTOKEN:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Duplicated Token Detected\n",unit);
#endif
				break;

			case UNS_RINGPURGEERR:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Ring Purge Error\n",unit);
#endif
				break;

			case UNS_BRIDGESTRIPERR:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Bridge Strip Error\n",unit);
#endif
				break;

			case UNS_RINGOPOSC:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Ring Op Oscillation\n",unit);
#endif
				break;

			case UNS_DIRECTEDBEACON:
				{
					register char *spt;
					spt = (char *) (up->buf_addr + sc->basereg) ; 
					sc->ndbr++;
					bcopy(spt,&sc->db_saddr[0],6);
					bcopy((spt+8),&sc->db_una_addr[0],6);
				}
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Directed Beacon Received\n",unit);
#endif
				break;

			case UNS_PCINIT:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: PC Trace Initiated\n",unit);
#endif
				break;

			case UNS_PCRCV:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: PC Trace Received\n",unit);
#endif
				break;

			case UNS_XMT_UNDERRUN:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Transmit Underrun\n",unit);
#endif
				break;

			case UNS_XMT_FAILURE:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Transmit Failure\n",unit);
#endif
				break;

			case UNS_RCV_OVERRUN:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: Receive Overrun\n",unit);
#endif
				break;

			default:
				printf("fza%d Unknown event %d\n",unit,up->status_id);
				break;
			}
			up->cmd_all = 0 ; /* turn back the ownership to RMC */

		}
		sc->unsindex = index;
	} 


	 /* 
	  * Dequeue next SMT received packet from queue and copy to SMT RCV ring
	  */
	{
		FZASMTRING *sp = &sc->smtrring[sc->rsmtindex];
		register struct mbuf *m;
		register caddr_t bp;

		/*
		 * check for something need to be send to the SMT RCV ring
		 */
		while ( sc->is_smt.ifq_head && (sp->own & FZA_HOST_OWN) ) { 
			IF_DEQUEUE(&sc->is_smt, m0);
			bp = (caddr_t)(sc->basereg + sp->buf_addr);
			m = m0;
			while(m) {
				fzacpy(mtod(m,caddr_t),bp,m->m_len);
				bp += m->m_len;
				m = m->m_next;
			}
			/*
			 * use the original RMC descriptor
			 */
			sp->rmc = FZA_SOP | FZA_EOP | sc->smt_rmc[sc->lsmtrmcindex];
			FZIOSYNC();	/* make ring visible before ownership */
			sp->own &= ~FZA_HOST_OWN;
			FZIOSYNC();	/* make ownership visible */
			sc->lsmtrmcindex = ++sc->lsmtrmcindex % IFQ_MAXLEN; 
			m_freem(m0);
			sc->rsmtindex = ++sc->rsmtindex % sc->nsmtrcv;
			sp = &sc->smtrring[sc->rsmtindex];
		}
		/*
		 * ask adapter to process this smt frame
		 */
		FZCSR_SET(sc->reg_ctla,SMT_RCV_POLL);
		WBFLUSH();
	}

	/*
	 * Dequeue next transmit request if interface is no longer busy.
	 */
	if (sc->nxmit <= 0) {
		sc->is_if.if_flags &= ~IFF_OACTIVE;
		fzastart( unit );
	}

	splx(s);
}

/*
 * FZA smt interrupt routine
 */
fzasmtint(unit)
int unit;
{
	register struct fza_softc *sc = fza_softc[unit];
	register FZASMTRING *sp;
	register FZAXMTRING *tp;
	register int index;

	FZAXMTRING *tpstart;
	caddr_t smtbp,tbp;
	int len,last,nring,tlen;
	u_long tmp;
	/*
	 * process SMT XMT frame
	 */
	for(index = sc->tsmtindex , sp = &sc->smttring[index];
		(sp->own & FZA_HOST_OWN); index = ++index % sc->nsmtxmt, 
					sp= &sc->smttring[index]) {
		tpstart = &sc->tring[sc->tindex];
		tlen = len = sp->rmc & RMC_PBC_MASK;

		/*
		 * check the last xmt descriptor 
		 */
		last = sc->tindex + len/XMTSEGSIZE - (((len % XMTSEGSIZE) != 0) ? 0 : 1);
		/*
		 * last descriptor not own by host
		 */
		if( sc->tring[last % sc->nsmtxmt].own & FZA_RMC_OWN ) {
			sc->tsmtindex = index ;
			return;
		} 

		/*
		 * copy the data from SMT XMT ring to RMC XMT ring
		 */
		nring = 0;
		smtbp = (caddr_t)(sp->buf_addr + sc->basereg); 
		while(len > 0) {
			tp = &sc->tring[sc->tindex];
			tbp = (caddr_t)FZAXMTADDR(tp,sc->basereg);
			fzacpy(smtbp,tbp,MIN(len,XMTSEGSIZE));
			smtbp += MIN(len,XMTSEGSIZE);
			len -= MIN(len,XMTSEGSIZE);
                        if(nring) {
                                tp->own = FZA_RMC_OWN;
                                tp->rmc = 0;
			}
			sc->tindex = ++sc->tindex % sc->nrmcxmt;
			nring++;
		}
		if(nring > 1 ) {
			tpstart->rmc = FZA_SOP | FZA_XMT_VBC | tlen;
			if(sc->tindex)
				sc->tring[sc->tindex-1].rmc = FZA_EOP ;
			else
				sc->tring[sc->nrmcxmt-1].rmc = FZA_EOP;
		} else 
			tpstart->rmc = FZA_SOP | FZA_EOP | FZA_XMT_VBC | tlen;

		tpstart->xmt_mbuf = 0;
		FZIOSYNC();		/* make ring visible before ownership */
		tpstart->own = FZA_RMC_OWN;
		FZIOSYNC();		/* make ownership visible */

		tmp = tpstart->own ; 
		/*
		 * give back the ownership to adapter
		 */
		sp->own &= ~FZA_HOST_OWN; 
		FZIOSYNC();		/* make ownership visible */
		sc->nxmit++;
	}
	/* 
	 * notice the port that one or more transmit packet is pending
	 */
	FZCSR_SET(sc->reg_ctla,XMT_POLL);
	WBFLUSH();
	sc->tsmtindex = index ;
}


/*
 * FZA command interrupt routine
 */
fzacmdint(unit)
int unit;
{
	register struct fza_softc *sc = fza_softc[unit];
	register FZACMDRING *cp;
	register FZACMD_BUF *xcmd;
	register int index;

	for ( index = (sc->lcmdindex + 1) % NFZACMD, cp = &sc->cmdring[index];
		(index != sc->cmdindex ) && (cp->cmd_own); 
			index = ++index % NFZACMD ,cp = &sc->cmdring[index] ) {
		/*
		 * process the cmd descriptor
		 */
		if(cp->status_id == 0 ) { /* command succeeded */
			xcmd = (FZACMD_BUF *) (cp->buf_addr + sc->basereg) ; 
			switch(cp->cmd_id) {

			case CMD_RDCNTR: /* copy the counter*/
				if(fzadebug)
					sc->fza_debug.cmdcnt.rdcntr++;
				bcopy(xcmd,sc->ctrblk,sizeof(struct _fzactrs));
				break;

			case CMD_STATUS:
				if(fzadebug)
					sc->fza_debug.cmdcnt.status++;
				bcopy(xcmd,sc->statusblk,sizeof(struct fzastatus));
				break;

			case CMD_MODCAM:
				if(fzadebug)
					sc->fza_debug.cmdcnt.modcam++;
				break;

			case CMD_SETCHAR:
				if(fzadebug)
					sc->fza_debug.cmdcnt.setchar++;
				break;

			case CMD_RDCAM:
				if(fzadebug)
					sc->fza_debug.cmdcnt.rdcam++;
				bcopy(xcmd,&sc->is_multi[0][0],512);
				break;

			case CMD_MODPROM:
				if(fzadebug)
					sc->fza_debug.cmdcnt.modprom++;
				break;

			case CMD_PARAM:
				if(fzadebug)
					sc->fza_debug.cmdcnt.param++;
				break;

			case CMD_INIT:
				if(fzadebug)
					sc->fza_debug.cmdcnt.init++;
				break;

			case CMD_NOP:
				break;

			default:
#if FZADEBUG
				if(fzadebug)
					printf("fza%d: unknown command id %d\n",unit,cp->cmd_id);
#endif
				break;
			}
		} else { /* error command */
			printf("fza%d: command failed, ",unit);
			fzacmdstatus(cp->status_id,unit,"fzacmdint");
		}
	}
	if(index)
		sc->lcmdindex = index - 1;
	else 
		sc->lcmdindex = NFZACMD - 1;
}
/*
 * FZA transmit interrupt routine
 */
fzatint(unit)
int unit;
{
	register struct fza_softc *sc = fza_softc[unit];
	register FZAXMTRING *tp;
	register struct mbuf *m0;
	register int index,len;

	struct mbuf *mp;
	struct fddi_header *fh;
	/*
	 * Process all outstanding transmits completed by
	 * the port.
	 */
	for (index = (sc->ltindex + 1)%sc->nrmcxmt ,tp = &sc->tring[index]; 
			(sc->nxmit > 0) && !(tp->own & FZA_RMC_OWN) ; 
		index = ++index % sc->nrmcxmt, tp = &sc->tring[index]) { 

		/* 
		 * if this is a start packet, process it. if not, do nothing
		 */

		if(tp->rmc & FZA_SOP ) { 
			/*
		 	 * Process xmit descriptor, we have no way to know the
		 	 * packet transmit status
		 	 */
			sc->is_if.if_opackets++;
			mp = (struct mbuf *)(FZ_32TOHEAP(tp->xmt_mbuf));
			tp->xmt_mbuf = 0;
			if( tp->rmc & FZA_XMT_SUCCESS ) {
				len = tp->rmc & RMC_PBC_MASK;
				fstc_bytesent += len ;
				if( fstc_pdusent != 0xffffffff)
					fstc_pdusent++;
				/*
		 	 	 * Loop back any LLC broadcasts we send
				 * 
				 * mp == 0 that means this is a SMT packet 
				 * this packet was copy from SMT XMT ring
		 	 	 */
				if(mp) { /* LLC packet */
					fh = mtod(mp, struct fddi_header *);
					/*
					 * multicast packets
					 */
					if( fh->fddi_dhost[0] & 1 ) {
						fstc_mbytesent += len;
						if(fstc_mpdusent != 0xffffffff)
						fstc_mpdusent++;
					}
					m_freem(mp);
				}
			} else {
				if(mp)
					m_freem(mp);
				sc->is_if.if_oerrors++;
			}

			sc->nxmit--;
		}

	}
	if(index)
		sc->ltindex = index - 1;	/* Last xmit processed */
	else
		sc->ltindex = sc->nrmcxmt - 1;
}



/*
 * FZA receive interrupt routine 
 */
fzarint(unit)
int unit;
{
	register struct fza_softc *sc = fza_softc[unit];
	register int index,len;
	register FZARCVRING *rp;

	struct mbuf *m,*m1,*mp;
	struct fddi_header *fptr;
	struct rmbuf *bp;
	int nrcv = 0;
	int fddi_type;
	/*
	 * Process all incoming packets on the receive ring. Stop if
	 * we get to the current receive index to avoid locking out
	 * the system, but give back one descriptor for each one we
	 * process to keep the device busy.
	 * 
	 * 
	 */
	for (index = sc->rindex,
		rp = &sc->rring[index],
		bp = &sc->rmbuf[index];
	     (rp->rcv_own & FZA_RCV_OWN) &&  nrcv < sc->nrecv ;
	     index = sc->rindex = ++index % NFZARCV,
		rp = &sc->rring[index],
		bp = &sc->rmbuf[index],
		sc->nbindex = ++sc->nbindex % NFZARCV )
	{
		/*
		 * check the DMA RCV status. If no error, process it
		 * we only process the LLC and SMT frame for decword
		 */
		if(!(rp->rmc & FZA_RCV_ERROR)) {
			len = rp->rmc & RMC_PBC_MASK  ;
			if ( len > FDDIMAX ) { /* Frame too long */
				if(fstc_pdulen != 0xffffffff)
					fstc_pdulen++;
				goto error;
			} else {
#ifdef __alpha
				fptr = (struct fddi_header *)PHYS_TO_KSEG(bp->phymbufa);
#else
				fptr = (struct fddi_header *)PHYS_TO_K1(bp->phymbufa);
#endif
				switch (fptr->fddi_fc & ~FDDIFC_Z ) { /* take out the priority */
				case FDDIFC_LLC_ASYNC: /* for LLC frame */
				case FDDIFC_LLC_SYNC: 
					if( len < FDDILLCMIM ) {
#if FZADEBUG
						if(fzadebug)
							printf("fza%d: LLC frame too short - frame len %d",unit,len);
#endif
						if(fstc_pdulen != 0xffffffff)
					  	        fstc_pdulen++;
						goto error;
					}
					fddi_type = FZA_LLC;
			/*
			 * The length reported by RMC is including one
			 * byte Frame Control, real data and 4 bytes CRC. 
			 * The driver interprets the frame as 4 bytes
			 * FDDI header ( including one byte Frame Control)  
			 * and real data. So, we need to decrement one
		  	 * for the length. 
			 */ 
					len--;
					break;

				case FDDIFC_SMT:	/* for SMT frame */
					if( len < FDDISMTMIM) {
#if FZADEBUG
						if(fzadebug)
							printf("fza%d: LLC frame too short - frame len %d",unit,len);
#endif
						if(fstc_pdulen != 0xffffffff)
					  	        fstc_pdulen++;
						goto error;
					}
					fddi_type = FZA_SMT;
					/*
                                         * mismatch with the firmware
                                         * RMC told us the wrong length
                                         */
					len = len + 3 ;
					break;

				case FDDIFC_MAC:
				default:
#if FZADEBUG
					if(fzadebug)
						printf("fza%d: unrecognize frame FC 0x%2x\n",unit,fptr->fddi_fc);
#endif
					fzanundrop++;
					goto error;
					break;
				}
			}

			/*
			 * Allocate a pair of new mbufs for the current
			 * receive descriptor. 
			 *
			 * Each receive buffer will use two cluster mbufs.
			 * 
			 * The first cut will be :
			 *
			 * If the size of packet less than 4K, then the second
			 * mbuf will be reused.
			 *
			 * The second cut will be:
			 *
 			 * If the size of FDDI frame less than the small
			 * mbuf length (which is MLEN = 108), driver will 
			 * allocate a small mbuf then data copy the
			 * packet to it. This will save a large cluster mbuf. 
			 *
			 * In addition, if the second mbuf is used for 
			 * less than MLEN size, a small mbuf will be allocated  
			 * and data copy the rest of packet from the second 
			 * cluster mbuf to this small mbuf.
			 * 
			 */
#if FZADEBUG
			if(fzadebug > 1 ) {
				printf("fzarint: got packet size %d type",len); 
				if(fddi_type == FZA_LLC )
					printf(" LLC frame\n");
				else
					printf(" SMT frame\n");
			}
#endif

			if(len > MHLEN ) { 
				FZAMCLGET(mp,1)
			} else { 
				MGETHDR((mp), M_DONTWAIT, MT_DATA); 
			}
			m = bp->mbufa ; 
			if ( mp ) {
				if ( len > FZASIZE ) {
					FZAMCLGET(m1,0)
					if(m1) {
#ifdef mips
						clean_dcache(PHYS_TO_K0(bp->phymbufa),MCLBYTES); 
						clean_dcache(PHYS_TO_K0(bp->phymbufb),len - MCLBYTES);
#endif
						m->m_pkthdr.len = len;
						m->m_len = FZASIZE ; 
						m->m_next = bp->mbufb;
						m->m_next->m_len = len - FZASIZE;
						fzanlarge++;
					} else {
						m_freem(mp);
						goto doinit;
					}
				} else if ( len > MHLEN ) {
					m1 = bp->mbufb;
#ifdef mips
					clean_dcache(PHYS_TO_K0(bp->phymbufa),len); 
#endif
					m->m_pkthdr.len = m->m_len = len;
					fzanmiddle++;
				} else {
					/* 
					 * if size < = MHLEN
					 */
#ifdef __alpha
					bcopy((PHYS_TO_KSEG(bp->phymbufa)),mtod(mp,caddr_t),len);
#else
					bcopy((PHYS_TO_K1(bp->phymbufa)),mtod(mp,caddr_t),len);
#endif
					m = mp;
					m->m_pkthdr.len = m->m_len = len;
					mp = bp->mbufa; 
					m1 = bp->mbufb;
					fzansmall++;
				}
				if(fddi_type == FZA_LLC )
					fzaread (sc, m, len);
				else { /* for SMT frame, just queue it */
					if(IF_QFULL(&sc->is_smt)){
						IF_DROP(&sc->is_smt);
						/*
						 * increase the system buffer
						 * unavailable
						 */
						FZCSR_WR(sc->reg_ctla,SMT_RCV_OVERFLOW);
						FZIOSYNC();
						fzansmtdrop++;
						m_freem(m);
					} else {
						fzansmtrcvd++;
						IF_ENQUEUE(&sc->is_smt,m);
						/* Save the RMC descriptor */
						sc->smt_rmc[sc->smtrmcindex]= rp->rmc; 
						sc->smtrmcindex = ++sc->smtrmcindex % IFQ_MAXLEN;
					}
				}
				fzainitdesc(&sc->rring[sc->nbindex],
					&sc->rmbuf[sc->nbindex],mp,m1);
				bp->mbufa=bp->mbufb=0;
			} else  
				goto doinit;
		} else { /* if error happened, parse the RMC descriptor */ 
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: recv err 0x%x\n",unit,rp->rmc);
#endif
			switch(rp->rmc & FZA_RCV_RCC ) {

			case FZA_RCV_OVERRUN: /* frame too long */
				if(len == 8192 || len == 8191) 
				        fstc_pdulen++;
				else {
#if FZADEBUG
				     if (fzadebug)
					printf("fza%d: RMC FIFO Overflow",unit);
#endif
				}
				break;

			case FZA_RCV_INTERFACE_ERR:/* RMC/MAC interface error */
				/*
				 * adapter should take care this
				 * driver will never see this
				 */
#if FZADEBUG
				if (fzadebug)
					printf("fza%d: RMC/MAC interface error",unit);
#endif
				break;

			default:
				switch(rp->rmc & FZA_RCV_RCC_rrr) {

				case FZA_RCV_RCC_NORMAL:
					if(rp->rmc & FZA_RCV_RCC_C) {
						fstc_fcserror++;
#if FZADEBUG
						if(fzadebug)
							printf("fza%d: Block Check Error\n",unit); 
#endif
					} else if ( !(rp->rmc & FZA_RCV_FSC) || (rp->rmc & FZA_RCV_FSB_E)){
						fstc_fseerror++;
#if FZADEBUG
						if(fzadebug)
							printf("fza%d: Frame status error\n",unit); 
#endif
					}
					break;

				case FZA_RCV_RCC_INVALID_LEN:
					fstc_pdualig++;
#if FZADEBUG
					if(fzadebug)		 
						printf("fza%d: Frame Alignment Error\n",unit);
#endif
					break;

				case FZA_RCV_RCC_SA_MATCHED:
				case FZA_RCV_RCC_DA_NOMATCHED:
				case FZA_RCV_RCC_RMC_ABORT:
#if FZADEBUG
					if(fzadebug) 
						printf("fza%d: Hardware problem\n",unit);
#endif
					/*
					 * go to halt state, log
					 * the errors
					 */
					FZCSR_WR(sc->reg_ctla,HALT);
					FZIOSYNC();
					return;
					break;

				case FZA_RCV_RCC_FRAGMENT:
				case FZA_RCV_RCC_FORMAT_ERR:
				case FZA_RCV_MAC_RESET:
#if FZADEBUG
					if(fzadebug)
						printf("fza%d: Fragment or format error or MAC reset\n",unit);
#endif
					break;

				default:
#if FZADEBUG
					if(fzadebug)
						printf("fza%d: Wrong RMC descriptor report 0x%x\n",unit,rp->rmc);
#endif
					break;
				}
				break;
			}	 
error:
			sc->is_if.if_ierrors++;
doinit:
			fzannombuf++;
			fzainitdesc(&sc->rring[sc->nbindex],
				&sc->rmbuf[sc->nbindex],bp->mbufa,bp->mbufb);
			bp->mbufa=bp->mbufb=0;
		}

	}
}

/*
 * FZA read routine. Pass input packets up to higher levels.
 */
fzaread (sc, m, len)
	register struct fza_softc *sc;
	register int len;
	struct mbuf *m;
{
	register struct fddi_header *eptr;
	register int off;


	eptr = mtod(m, struct fddi_header *);


	/*
	 * Trim off fddi header
	 */
	m->m_data += sizeof (struct fddi_header);
	m->m_len -= sizeof (struct fddi_header);

	/*
	 * Subtract length of header from len
	 */
	len -= sizeof (struct fddi_header);

	m->m_pkthdr.rcvif = &sc->is_if;
	m->m_pkthdr.len = len;

	/*
	 * feed the ether struct by the fddi struct
	 */
	/*	bcopy(&eptr->fddi_dhost[0],&eh.ether_dhost[0],6); */
	/*	bcopy(&eptr->fddi_shost[0],&eh.ether_shost[0],6); */
	/*      eh.ether_type = len; */
	/*
	 * Bunp up the DECnet counter
	 */
	sc->is_if.if_ipackets++;
	fstc_bytercvd += len;
	if ( fstc_pdurcvd != (unsigned) 0xffffffff)
		fstc_pdurcvd++;

	if( eptr->fddi_dhost[0] & 1 ) {
		fstc_mbytercvd += len;
		if(fstc_mpdurcvd != 0xffffffff)
			fstc_mpdurcvd++;
	}

	/* Dispatch this packet */
   	ether_input(&(sc->is_ed), (struct ether_header *)eptr, m, 0);
}

/*
 * Perform a node halt/reset (SOFT reset) due to the HALT state change
 * interrupt.
 */
fzareset(unit)
	int unit;
{
	register struct fza_softc *sc = fza_softc[unit];

	struct ifnet *ifp = &sc->is_if;
	struct mbuf *m, *m1;
	struct rmbuf *bp;
	FZAXMTRING *tp;
	FZARCVRING *rp;
	int i;


	/*
	 * release all of the receive mbufs, transmit mbufs , and then reset 
	 * the adapter
	 */

	for (i = 0, bp = &sc->rmbuf[i];  i < NFZARCV ; i++ , bp++) {
		if(bp->mbufa)
			m_freem(bp->mbufa);
		if(bp->mbufb)
			m_freem(bp->mbufb);
		bp->mbufa = bp->mbufb = 0; 
		bp->phymbufa = bp->phymbufb = 0;
	}
	for ( i=0, tp = &sc->tring[0]; i < sc->nrmcxmt && tp->xmt_mbuf ; i++, tp++ ) {
		m_freem((struct mbuf *)(FZ_32TOHEAP(tp->xmt_mbuf)));
	}


	/*
	 * If the adapter is in SHUT state, don't do the selftest
         */
        if((sc->flag == FZA_NORMAL || sc->flag == FZA_PC_TRACED )) {
		/*
		 * take out this because it may happen FZAMAXRESET
		 * times per year; the best way is use a ratio of 
		 * how many times per hour . 
		 */
		/* 
		if (!(sc->flag & FZA_PC_TRACED) && (++fzanreset > FZAMAXRESET))
		{
			ifp->if_flags &= ~IFF_UP;
			printf("fza %d: exceed the maximum number of reset\n",unit);
			return(0);
		}
		*/
		/*
	 	 * do the adapter selftest
	 	 */
		if(!(fzaselftest(sc,unit))) { 
			printf("fza%d: fzareset selftest fail\n",unit);
			return(0);
		}
		/* set the interrup mask */
		FZCSR_WR(sc->reg_mask, ((FZA_INTR_MASK)&0x0ffff));
		FZIOSYNC();
	}

	fzaindex_init(sc);

	if(!fzacmdinit(sc,unit)) {
		printf("fza%d: fzrestart init command fail\n",unit);
		return(0);
	}

	/*
	 * fill up the information
	 */
	fzafillinit(sc,sc->initblk);

	/* set the interrup mask */
	FZCSR_WR(sc->reg_mask, ((FZA_INTR_MASK)&0x0ffff));
	FZIOSYNC();

	if(sc->flag == FZA_DLU) {
		printf("fza%d: Firmware revision %c%c%c%c\n",unit,
				sc->fw_rev[0],sc->fw_rev[1],sc->fw_rev[2],
				sc->fw_rev[3]);
		if(sc->nduflag & IFF_UP ) /* turn on this device */
			ifp->if_flags |= IFF_RUNNING ;
		else 			  /* turn off this device */
			ifp->if_flags &= ~IFF_RUNNING ;
	}

	/*
	 * clean the flag
	 */
	sc->flag = FZA_NORMAL;

	if(!fzaallocate(sc)){
		printf("fza%d: fzareset fail - can not allocate enough receive buffer\n",unit);
		return(0);
	}

	if(ifp->if_flags & (IFF_RUNNING|IFF_UP)) {
		ifp->if_flags &= ~(IFF_RUNNING);
		untimeout(fzainit, unit); 
		timeout(fzainit, unit, 1);
	}

	return(1);
}

fzaindex_init(sc)
	register struct fza_softc *sc;
{
	/*
	 * initialize all the indexes 
	 *
         * sc->tindex == index of next transmit desc. to activate
         * sc->ltindex  == index of last transmit processed
         * sc->nxmit  == # of transmit pending
         * sc->rindex == index of next recv. descriptor to activate
         * sc->tsmtindex  == index of next SMT tansmit to CNS
         * sc->rsmtindex  == index of next SMT receive from CNS
         * sc->cmdindex  == index of next command desc. to activate
         * sc->lcmdindex  == index of last comman processed
         * sc->unsindex ==  index of next unsolicited desc. to activate
	 * sc->ndbr == # of direct beacon received
         */

	sc->smtrmcindex = sc->lsmtrmcindex = 0;
        sc->tindex = sc->nxmit = sc->rindex = 0;
        sc->tsmtindex = sc->rsmtindex = 0;
        sc->cmdindex = sc->unsindex = 0;
        sc->ltindex = sc->lcmdindex = -1;
	sc->ndbr = 0 ;
	sc->nrecv = sc->nbindex = 0 ;
}
/*
 * allocate receive mbufs and clear the transmit ring mbuf entry
 */
fzaallocate(sc)
	struct fza_softc *sc;
{
	register FZAXMTRING *tp;
	register FZARCVRING *rp;
	register struct rmbuf *bp;
	register int i;
	struct mbuf *m, *m1;

        /*
         * allocate mbufs for receive ring
         */
	sc->nrecv = NFZARCV - 1 ; 
	for (i = 0, rp = &sc->rring[0], bp = &sc->rmbuf[0] ; (i < sc->nrecv ); 
		i++, rp++ , bp++) {
                FZAMCLGET(m,1)
                FZAMCLGET(m1,0)
                if(m && m1) 
                       	fzainitdesc(rp,bp, m, m1);
		else 
			break;
	} 
	sc->nrecv = sc->nbindex = i ;
	/* sc->nbindex = sc->nbindex % NFZARCV ;*/
	if(sc->nrecv < 2 ) { /* At least two buffers */ 
        	for (i = 0, bp = &sc->rmbuf[i]; (i < sc->nrecv ); i++, bp++) {
			if(bp->mbufa)
                       		 m_freem(bp->mbufa);
                	if(bp->mbufb)
                       		 m_freem(bp->mbufb);
        	}
		return(0);
	}

        /*
         * clear the transmit ring mbuf entry
         */
        for ( i=0, tp = &sc->tring[0]; i < sc->nrmcxmt ; i++, tp++) 
		tp->xmt_mbuf = 0;

	return(1);

}
/*
 * Process an ioctl request.
 */
fzaioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	unsigned int cmd;
	caddr_t data;
{
	register struct fza_softc *sc = fza_softc[ifp->if_unit];
	struct protosw *pr;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	register struct ifaddr *ifa = (struct ifaddr *)data;
	struct ctrreq *ctr = (struct ctrreq *)data;
	struct ifeeprom *ife = (struct ifeeprom *)data;
	struct ifchar *ifc = (struct ifchar *)data;

	int s, delay, error = 0, ttimeout = 0;
	unsigned short port_status;

	switch (cmd) {

        case SIOCENABLBACK:
        case SIOCDISABLBACK:
		if (cmd == SIOCENABLBACK) { 
#if FZADEBUG
			if(fzadebug) printf("SIOCENABLBACK");
#endif
                	ifp->if_flags |= IFF_LOOPBACK;
		} else {
#if FZADEBUG
			if(fzadebug) printf("SIOCDISABLBACK");
#endif
                	ifp->if_flags &= ~IFF_LOOPBACK;
		}
		if (ifp->if_flags & IFF_RUNNING) {

			/*
			 * Lock softc. issue a SHUT command 
			 * to cause a state change and then
		         * bring down the adapter and reset it 
			 */
			s = splimp();
			FZCSR_WR(sc->reg_ctla,SHUT);
			FZIOSYNC();		/* guarantee write ordering */
			sc->flag = FZA_SHUT;
			/*
		 	 * Wait 30 seconds for FZA change to Uninitialized state
		 	 */ 
			for (delay = 3000; delay > 0 && ((FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) != STATE_UNINITIALIZED) ; delay--)  
				 DELAY(10000);
			if((FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) == STATE_UNINITIALIZED)
				fzareset(ifp->if_unit);
			else
				printf("fza%d: Can't transition to Uninitialize State\n",ifp->if_unit); 
			splx(s);
		}
                break;
 
        case SIOCRPHYSADDR: 
                /*
                 * read default hardware address. Lock softc while accessing
		 * per-unit physical address info.
                 */
		s = splimp();
		bcopy(sc->is_dpaddr, ifd->default_pa, 6);
		bcopy(sc->is_addr, ifd->current_pa, 6);
		splx(s);
                break;
 

	case SIOCSPHYSADDR: 
	case SIOCDELMULTI: 
	case SIOCADDMULTI: 

		/*
		 * Lock softc while updating per-unit multicast address
		 * list and for command processing as in "fzainit()".
		 */
		s = splimp();
		if (cmd == SIOCDELMULTI) {
			int i;
#if FZADEBUG
			if(fzadebug > 1 ) printf("SIOCDELMULTI");
#endif
			/*
			 * If we're deleting a multicast address, decrement
			 * the is_muse count and invalidate the address if
			 * count goes to zero.
			 */
			for (i = 0; i < NMULTI; i++) {
				if (bcmp(sc->is_multi[i],
				    ifr->ifr_addr.sa_data,6) == 0)
					break;
			}
			if ((i < NMULTI) && (--sc->is_muse[i] == 0))
				bcopy(etherbroadcastaddr,sc->is_multi[i],6);
			else {
				splx(s);
				goto done;
			}
		} else {
			int i, j = -1;
			if( cmd == SIOCSPHYSADDR ) {
				/*
				 * we can not change the physical station
				 * address; just add an entry to the CAM as 
				 * an alias address
				 */
#if FZADEBUG
				if(fzadebug > 1)
					printf("SIOCSPHYSADDR");
#endif

               			bcopy(ifr->ifr_addr.sa_data, sc->is_addr, 6);
                		pfilt_newaddress(sc->is_ed.ess_enetunit, ifr->ifr_addr.sa_data);
			}
			else {
#if FZADEBUG
				if(fzadebug > 1 ) 
					printf("SIOCADDMULTI");
#endif

			} 
			/*
			 * If we're adding a multicat address, increment the
			 * is_muse count if it's already in our table, and
			 * return. Otherwise, add it to the table or return
			 * ENOBUFS if we're out of entries.
			 */
			for (i = 0; i < NMULTI; i++) {
				if (bcmp(sc->is_multi[i],
				    ifr->ifr_addr.sa_data,6) == 0) {
					sc->is_muse[i]++;
					splx(s);
					goto done;
				}
				if ((j < 0) && (bcmp(sc->is_multi[i],
				    etherbroadcastaddr,6) == 0))
					j = i;
			}
			if (j < 0) {
				printf("fza%d: addmulti failed, multicast list full: %d\n",
					ifp->if_unit, NMULTI);
				error = ENOBUFS;
				splx(s);
				goto done;
			} else {
		    		bcopy(ifr->ifr_addr.sa_data,
				      sc->is_multi[j], 6);
		    		sc->is_muse[j]++;
			}
		}
		if (ifp->if_flags & IFF_RUNNING) {
			/*
			 * If we've successfully init'ed the interface,
			 * issue a MODCAM command to update the fddi
			 * user's multicast address list. Otherwise, the
			 * list will be initialized upon the first call
			 * to "fzainit()".
			 */
			FZACMDRING *cp;
			if (cp=fzamkcmd(sc, CMD_MODCAM,ifp->if_unit)) {
				splx(s);
				FZATIMEOUT(cp,ttimeout)/* Wait */
				if(ttimeout ||  (cp->status_id != CMD_SUCCESS)) 
						error = EINVAL;
			} else {
				error = ENOBUFS;
				splx(s);
			}
		}
		else {
			splx(s);
		}
		/* If an IP address has been configured then an ARP packet
		 * must be broadcast to tell all hosts which currently have
		 * our address in their ARP tables to update their information.
		 */
		if( cmd == SIOCSPHYSADDR ) {
		    if(((struct arpcom *)ifp)->ac_ipaddr.s_addr)
			    rearpwhohas((struct arpcom *)ifp,
				  &((struct arpcom *)ifp)->ac_ipaddr);
		}
		break;

	case SIOCRDZCTRS:
	case SIOCRDCTRS:

#if FZADEBUG
		if(fzadebug) {
			if (cmd == SIOCRDZCTRS) 
				printf("SIOCRDZCTRS");
			else 
				printf("SIOCRDCTRS");
		}
#endif

		/*
		 * The adapter does not support clear counter command.
		 * So, we only support read counter. 
		 *
		 * Copyin most recent contents of unit's counter
		 * block.
		 */
		FZIOSYNC();
		port_status = FZCSR_2RD(sc->reg_status) & ADAPTER_STATE;
		if ((ifp->if_flags & IFF_RUNNING) ||
		   (port_status) == STATE_RUNNING || 
		   (port_status) == STATE_MAINTENANCE || 
		   (port_status) == STATE_INITIALIZED ) 
		{
			switch (ctr->ctr_type) {

			case FDDIMIB_SMT:
			case FDDIMIB_MAC:
			case FDDIMIB_PATH:
			case FDDIMIB_PORT:
			case FDDIMIB_ATTA:
				fmib_fill(sc,ctr,ctr->ctr_type);
				break;

			case FDDI_STATUS:
				fzagetstatus(sc, &ctr->sts_fddi,sc->statusblk);
				break;

			default:
				fzagetctrs(sc, &ctr->ctr_fddi,sc->ctrblk);
				ctr->ctr_type = CTR_FDDI;
				break;
			}
		}
		break;

	case SIOCSIFADDR:

		/*
		 * Init the interface if its not already running
		 */
#if FZADEBUG
		if(fzadebug) printf("SIOCSIFADDR\n");
#endif
		s = splimp();
		if (!(ifp->if_flags & IFF_RUNNING)) {
			if(!fzaallocate(sc)) {
				printf("fza%d: fzaioctl can not allocate enough receive buffers",ifp->if_unit);
				error = ENOBUFS;
				splx(s);
				goto done;
			}
			fzainit(ifp->if_unit); 
			ifp->if_flags |= IFF_UP;
		}
		switch(ifa->ifa_addr->sa_family) {
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			break;
		}
		splx(s);
		break;

	case SIOCSIFFLAGS:
#if FZADEBUG
		if(fzadebug) 
			printf("SIOCSIFFLAGS flag 0x%x adapter state 0x%x\n",ifp->if_flags, (FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) );
#endif

		if(ifp->if_flags & IFF_UP) {
			FZACMDRING *cp;
			/* 
			 * If we've successfully init'ed the interface, 
			 * issue a MODPROM command to update the ethernet 
			 * user's promiscuous bit based upon the interface
			 * flags.
			 *
			 * Only support LLC promiscuous mode for now 
			 */
			switch (FZCSR_2RD(sc->reg_status) & ADAPTER_STATE){

			case STATE_RUNNING :
				/* we can issue the MODPROM
				 * command only when adapter
				 * is in running state
				 */ 
				s = splimp();
				if (cp =fzamkcmd(sc, CMD_MODPROM,ifp->if_unit)) {
					splx(s);
					FZATIMEOUT(cp,ttimeout)
					if(ttimeout ||  (cp->status_id != CMD_SUCCESS)) 
						error = EINVAL;
				} else {
					error = ENOBUFS;
					splx(s);
				}
				break;

			case STATE_INITIALIZED :
				/* 
				 * need to turn on the adapter
				 */
				s = splimp();
				/*
				 * turn on the interface
				 */
				untimeout(fzainit);
				ifp->if_flags &= ~IFF_RUNNING;
				timeout(fzainit,ifp->if_unit,500);
				splx(s);
				break;

			default:
				break;
			}
		} else if ( (FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) != STATE_INITIALIZED ) {
			/*
			 * The interface has not been turn off; Turn it
			 * off. The adapter will state on the 
			 * INITIALIZATION 
			 */
#if FZADEBUG
			if(fzadebug) printf("Turn off the interface\n");
#endif
			s = splimp();
			FZCSR_WR(sc->reg_ctla,SHUT);
			FZIOSYNC();		/* guarantee write ordering */
			/*
	 	 	 * Wait for FZA change to Uninitialized state
	 	 	 */ 
			for (delay = 3000; delay > 0 && ((FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) != STATE_UNINITIALIZED) ; delay--)  
		 		DELAY(10000);
			if((FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) == STATE_UNINITIALIZED) {
				sc->flag = FZA_SHUT;
				ifp->if_flags &= ~IFF_RUNNING ;
				fzareset(ifp->if_unit);

			} else
				printf("fza%d: Can't SHUT the adapter\n",ifp->if_unit); 
			splx(s);
		}
		break;

	case SIOCIFRESET: /* reset the adapter */

#if FZADEBUG
		if(fzadebug) printf ("SIOCIFRESET\n");
#endif
		s = splimp();
		if(!fzaselftest(sc,ifp->if_unit)) {
			printf(" SIOCIFRESET selftest fail\n");
			splx(s);
			return(FZCSR_2RD(sc->reg_status) & ID_FIELD);
		} else {
			/*
			 * self test succeeded the adapter is in
			 * Uninitialized state, do the driver 
			 * reset.
			 */
			sc->flag = FZA_DLU;
			fzareset(ifp->if_unit);
			/*
			 * turn on the timer 
			 */
			ifp->if_timer = 1;
			splx(s);
			return(0);
		}
		break;

	case SIOCEEUPDATE: /* EEPROM update */

#if FZADEBUG
		if(fzadebug) printf("SIOCEEUPDATE\n");
#endif
		s = splimp();

		/*
	 	 * if the adapter is not in the uninitialized mode, force it
	 	 * to that mode.
	 	 */
		FZIOSYNC();
		port_status = FZCSR_2RD(sc->reg_status) & ADAPTER_STATE;
		if(((port_status) != STATE_UNINITIALIZED) && 
			((port_status) !=  STATE_RESET )) {

			/*turn off the read counters routine */
			ifp->if_timer = 0; 

			/* save the original state */
			sc->nduflag = ifp->if_flags ;

			/* disable the interface */
			ifp->if_flags &= ~IFF_UP;

			FZCSR_SET(sc->reg_intr,0);
			FZIOSYNC();
			FZCSR_WR(sc->reg_ctla,SHUT);
			FZIOSYNC();		/* guarantee write ordering */
			/*
		 	 * Wait 30 seconds for FZA state change
		 	 */ 
			for (delay = 3000; delay > 0 && !(FZCSR_RD(sc->reg_intr) & STATE_CHG); delay-- )
				 DELAY(10000);
			/*
		 	 * check for state change
			 */
			if(!(FZCSR_RD(sc->reg_intr) & STATE_CHG)) {
				printf("fza%d: Down Line Upgrade state change time out\n",ifp->if_unit);
				splx(s);
				return(1);
			} else if((FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) !=  
					STATE_UNINITIALIZED) { 
				printf("fza%d: Down Line Upgrade can not change to Uninitialized Mode",ifp->if_unit);
				splx(s);
				return(1);
			}
		}
		/*
	 	 * set the DLU and reset bit
	 	 */
		FZCSR_WR(sc->reg_reset,(DLU_MODE | RESET));
		FZIOSYNC();		/* guarantee write ordering */

		if(!fzadlu(ifp->if_unit,sc,ife->ife_data,ife->ife_offset,ife->ife_blklen)) { 
			splx(s);
			return(1); /* dlu failure */
		}
		/*
		 * if this is last block, blast flush
		 */
		if(ife->ife_lastblk == IFE_LASTBLOCK ) {
			int i;
			FZCSR_CLR(sc->reg_reset,RESET);
			FZIOSYNC();		/* guarantee write ordering */
			/*
			 * wait for 90 seconds for blast FLUSH finish
			 */
			for (i = 9000; i > 0 && !(FZCSR_RD(sc->reg_intr) & DLU_DONE); i--)
				DELAY(10000);
			/*
			 * check the DLU_DONE bit
			 */
			if(!(FZCSR_RD(sc->reg_intr) & DLU_DONE)) {
				printf("fza%d: Blast Flush time out",ifp->if_unit);
				splx(s);
				return(1);
			} else {
				FZIOSYNC();
				switch ( FZCSR_2RD(sc->reg_status) & DLU_STATUS ) {

				case DLU_FAILURE:
					printf("fza%d: DLU fatal failure - Brain Dead\n",ifp->if_unit);
					splx(s);
					return(1);
					break;

				case DLU_ERROR:
					printf("fza%d: DLU error - Recoverable\n",ifp->if_unit);
					splx(s);
					return(1);
					break;

				case DLU_SUCCESS: 
					FZCSR_WR(sc->reg_reset,FZAREG_CLEAR);
					FZIOSYNC();/* guarantee write order */
					break;
				}
			}
		} 
		/* turn off the interface */
		splx(s);
		return (0);
		break;

	case SIOCIFSETCHAR:
		s=splimp();
		if (ifp->if_flags & IFF_RUNNING) {
			/*
			 * If we've successfully init'ed the interface,
			 */
			FZACMDRING *cp;
			if (cp=fzamkcmd(sc, CMD_SETCHAR,ifp->if_unit,ifc)) {
				splx(s);
				FZATIMEOUT(cp,ttimeout)/* Wait */
				if(ttimeout ||  (cp->status_id != CMD_SUCCESS)) 
						error = EINVAL;
				else {
					if(ifc->ifc_treq != 0xffffffff )
						sc->t_req = ifc->ifc_treq;
					if(ifc->ifc_rtoken != 0xffffffff )
						sc->rtoken_timeout = ifc->ifc_rtoken;
					if(ifc->ifc_tvx != 0xffffffff )
						sc->tvx = ifc->ifc_tvx;
					if(ifc->ifc_lem != 0xffffffff )
						sc->lem_threshold = ifc->ifc_lem;
					if(ifc->ifc_ring_purger != 0xffffffff)
						sc->ring_purger = ifc->ifc_ring_purger ;
				}
			} else {
				error = ENOBUFS;
				splx(s);
			}
		} else
			splx(s);

		break;

	case SIOCSIPMTU:
		{
		u_short ifmtu;

		bcopy(ifr->ifr_data, (u_char *)(&ifmtu), sizeof(u_short));	

		if (ifmtu > FDDIMTU || ifmtu < IP_MINMTU)
			error = EINVAL;
		else
			ifp->if_mtu = ifmtu;
		}
		break;

	default:
		error = EINVAL;
	}
done:
	return(error);
}

fzadlu(unit,sc,data,offset,blklen)
int unit,offset,blklen;
register struct fza_softc *sc;
char *data;
{
	char *pmiaddr,*bufaddr = data;
	u_short stmp1;
	int i;

	pmiaddr = (char *)(sc->basereg + FZA_DLU_ADDR + offset) ;
#if FZADEBUG
	if(dludebug)
		printf("\n dlu address 0x%lx size %d data --->",pmiaddr,blklen);
#endif
	for ( i = blklen ; i >= 2 ; i = i - 2, bufaddr = bufaddr + 2 ) {
		*((u_short *)pmiaddr) = htons(*((u_short *)bufaddr));
		stmp1 = *(u_short *)pmiaddr;
		pmiaddr = pmiaddr + 2;
#if FZADEBUG
		if(dludebug)
			printf("%d New 0x%x OLD 0x%x ",i, stmp1, *((u_short *)bufaddr));
#endif
		/*
		 * check the status
		 */  
		FZIOSYNC();		/* guarantee event ordering */
		if(FZCSR_RD(sc->reg_intr) & PM_PARITY_ERR) {
			printf("fza%d, Down Line Load parity error\n",unit);
			return(0);
		}
	}
	if(i > 0 ) { 
		*(u_short *)pmiaddr  = htons(*((u_short *)bufaddr)) & 0xff00 ;
		stmp1 = *(u_short *)pmiaddr;
		/*
		 * check the status
		 */  
		FZIOSYNC();		/* guarantee event ordering */
		if(FZCSR_RD(sc->reg_intr) & PM_PARITY_ERR) {
			printf("fza%d, Down Line Load parity error\n",unit);
			return(0);
		}
	}

#if FZADEBUG
	if(dludebug)
		printf("\n");
#endif
return(1);
}

/*
 * FZA watchdog timer (runs once per second). Schedule a "read counters"
 * command and "read status" to update the per-unit counter block. 
 */
fzawatch (unit)
	int unit;
{
	register struct fza_softc *sc = fza_softc[unit];
	register struct ifnet *ifp = &sc->is_if;
	static int status;
	int s;

	FZIOSYNC();
	if ((FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) != STATE_HALTED ) {

		s = splimp();
		/*
		 * Schedule a read counters cmd to update the counter
		 * block for this unit. 
		 */
		fzamkcmd(sc,CMD_RDCNTR,unit);

		if(status++ > 3 ) {
			/*
			 * schedule a read status command
			 */ 
			fzamkcmd(sc,CMD_STATUS,unit);
			status = 0;
		}
		splx(s);

	}
	ifp->if_timer = 1;
}

/*
 * FZA INIT command and wait until done
 */
fzacmdinit(sc,unit)
register struct fza_softc *sc;
int unit;
{
	register FZACMDRING *cp;
	int delay,index = sc->cmdindex;

	if((cp=fzamkcmd(sc,CMD_INIT,unit))) {
		/*
		 * wait 2 seconds for the INIT command done - only need one
	 	 * second
	 	 */
		FZIOSYNC();		/* guarantee event ordering */
		for (delay = 200 ; delay > 0 && !(FZCSR_RD(sc->reg_intr) & CMD_DONE); delay--)
				DELAY(10000);

		sc->lcmdindex++;
		if(!(FZCSR_RD(sc->reg_intr) & CMD_DONE)) {
			printf("fza%d: INIT command timeout\n",unit);
#if FZADEBUG
			if(fzadebug) 
				PRT_REG(sc)
#endif

			return(0);
		} else if(!(fzacmdstatus(cp->status_id,unit,"fzacmdinit"))) {
#if FZADEBUG
			if(fzadebug)
				PRT_REG(sc)
#endif

			return(0);
		} else {
#if FZADEBUG
			if(fzadebug)
				printf("fza%d: INIT command succeeded\n",unit);
#endif
			bcopy((caddr_t)(cp->buf_addr+sc->basereg),sc->initblk, sizeof(struct fzainit));
			return(sizeof(struct fza_softc));
		}
	} else 
		return(0);
}

fzainitdesc(rp,bp,m,m1)
register FZARCVRING *rp;
register struct rmbuf *bp; 
struct mbuf *m, *m1;
{
	rp->rmc = 0;
	bp->mbufb=m1;
	if((svatophys(mtod(m1,u_long *),&(bp->phymbufb))) == KERN_INVALID_ADDRESS)
		panic("fzainitdesc: Invalid physical address!\n");
	rp->bufaddr2 = bp->phymbufb >> FZASHIFT ; 
	FZIOSYNC();		/* guarantee write ordering */
	bp->mbufa=m;
	if((svatophys(mtod(m,u_long *), &(bp->phymbufa))) == KERN_INVALID_ADDRESS)
		panic("fzainitdesc: Invalid physical address!\n");
	rp->bufaddr1 = ( bp->phymbufa >> FZASHIFT) & ~FZA_HOST_OWN ;
	FZIOSYNC();		/* guarantee write ordering */
}

fzaselftest(sc,unit)
register struct fza_softc *sc;
int unit;
{
	register int delay;

	/* 
	 * perform the self test
	 */
	FZCSR_WR(sc->reg_reset,RESET);
	FZIOSYNC();		/* guarantee write ordering */
        DELAY(1000);
	FZCSR_WR(sc->reg_reset,FZAREG_CLEAR);
	FZIOSYNC();		/* guarantee write ordering */

	/*
	 * wait 40 seconds (adapter need 25 seconds) for FZA state
	 * change from Reseting state to Uninitialized state
	 */
	for (delay = 4000 ; delay > 0 && !(FZCSR_RD(sc->reg_intr) & STATE_CHG); delay--)
		DELAY(10000);

	/*
         * check for the state change
	 */
	if (!( FZCSR_RD(sc->reg_intr) & STATE_CHG) ) {
		/*
                 * didn't change state 
	         */
		printf("fza%d: selftest timeout, couldn't pass selftest id %d\n",
					unit,(FZCSR_2RD(sc->reg_status) & ID_FIELD));
#if FZADEBUG
		if(fzadebug) 
			PRT_REG(sc)
#endif

		return(0);
	} else if ( (FZCSR_2RD(sc->reg_status) & ADAPTER_STATE) != STATE_UNINITIALIZED ) {
		printf("fza%d: selftest error, couldn't change to Uninitialized State id %d\n",unit,(FZCSR_2RD(sc->reg_status) & ID_FIELD));
#if FZADEBUG
		if(fzadebug)  
			PRT_REG(sc)
#endif
		return(0);
	} else {
#if FZADEBUG
		if(fzadebug) 
			printf("fza%d: selftest succeeded\n");
#endif
	}
	return(1);
}


fzafillinit(sc,pt)
register struct fza_softc *sc;
struct fzainit *pt;
{

	bcopy(&pt->mla[0],&sc->is_addr[0],6);
	bcopy(&pt->mla[0],&sc->is_dpaddr[0],6);

	/* 
	 * fill out the default characteristics value
	 */
	sc->t_max = pt->def_t_max;
	sc->t_req = pt->def_t_req;
	sc->tvx = pt->def_tvx;
	sc->lem_threshold= pt->lem_threshold;
	sc->pmd_type = pt->pmd_type;
	sc->smt_version = pt->smt_version;
	sc->rtoken_timeout = pt->rtoken_timeout ;
	sc->ring_purger = pt->ring_purger;
	sc->smt_maxid = pt->smt_maxid;
	sc->smt_minid = pt->smt_minid;
	/*
	 * fill out the ring address and number of ring entrys
	 */
	sc->rring=(FZARCVRING *)(pt->rcvbase_addr + sc->basereg);
	sc->tring=(FZAXMTRING *)(pt->xmtbase_addr + sc->basereg);
	sc->smttring=(FZASMTRING *)(pt->smtxmt_addr + sc->basereg);
	sc->smtrring=(FZASMTRING *)(pt->smtrcv_addr + sc->basereg);

	sc->nrmcxmt = pt->xmt_entry;
	sc->nsmtxmt = pt->smtxmt_entry;
	sc->nsmtrcv = pt->smtrcv_entry;

	/*
	 * fill out the adapter specific information
	 */
	bcopy(&pt->pmc_rev[0],&sc->pmc_rev[0],4);
	bcopy(&pt->phy_rev[0],&sc->phy_rev[0],4);
	bcopy(&pt->fw_rev[0],&sc->fw_rev[0],4);
	sc->mop_type = 	pt->mop_type;
	sc->station_id.lo = pt->def_station_id.lo;
	sc->station_id.hi = pt->def_station_id.hi;

}


fzagetstatus(sc,ctr,fs)
	register struct fza_softc *sc;
	register struct fstatus *ctr;
	register struct fzastatus *fs;
{
	unsigned short port_status;

	/*
	 * Fill out the fddi status 
	 */
	bzero(ctr, sizeof(struct fstatus));

	/* 
	 * assign the default characteristics value 
	 * and revision number
	 */
	ctr->t_req = sc->t_req;
	ctr->t_max = sc->t_max;
	ctr->tvx   = sc->tvx;
	ctr->lem_threshold = sc->lem_threshold;
	ctr->rtoken_timeout = sc->rtoken_timeout; 
	ctr->pmd_type = sc->pmd_type ;
	ctr->smt_version = sc->smt_version ;
	bcopy(&sc->phy_rev[0],&ctr->phy_rev[0],4);
	bcopy(&sc->fw_rev[0],&ctr->fw_rev[0],4);

	/*
	 * When the adapter in HALT state, we can
	 * not issue the get status command
	 */
	FZIOSYNC();
	port_status = FZCSR_2RD(sc->reg_status) & ADAPTER_STATE;
	if((port_status) == STATE_HALTED ) {
		ctr->led_state = 2 ; /* Red */
		ctr->link_state = 1 ; /* Off Ready */ 
		ctr->dup_add_test =  0 ; /* Unknown */
		ctr->ring_purge_state = 0 ; /* Purger Off */
		ctr->phy_state = 2 ; /* Off Ready */
	} else { 
		ctr->led_state = fs->led_state;
		ctr->link_state = fs->link_state;
		ctr->phy_state = fs->phy_state ;
		ctr->dup_add_test = fs->dup_add_test;
		ctr->ring_purge_state = fs->ring_purge_state;
	}
	ctr->state = (port_status) >> 8 ;
	ctr->rmt_state = fs->rmt_state;
	ctr->neg_trt = fs->neg_trt;
	bcopy(&fs->upstream[0],&ctr->upstream[0],6);
	bcopy(&fs->downstream[0],&ctr->downstream[0],6);
	ctr->una_timed_out = fs->una_timed_out;
	ctr->frame_strip_mode = fs->frame_strip_mode;
	ctr->claim_token_mode = fs->claim_token_mode;
	ctr->neighbor_phy_type = fs->neighbor_phy_type;
	ctr->rej_reason = fs->rej_reason;
	ctr->phy_link_error = fs->phy_link_error;
	bcopy(sc->is_dpaddr,&ctr->mla[0],6);
	ctr->ring_error = fs->ring_error & 0x0f ;
	bcopy(&fs->dir_beacon[0],&ctr->dir_beacon[0],6);
}

fzagetctrs(sc,ctr,xcmd)
	register struct fza_softc *sc;
	register struct fstat *ctr;
	register struct _fzactrs *xcmd;
{

	register int seconds;

	/*
	 * Fill out the fddi counters through the ethernet counter
	 * "estat" structure. It is  based upon the information
	 * returned by the CMD_{RDC,RCC}CNTR command and driver  
	 * maintained counter. 
	 */
	bzero(ctr, sizeof(struct fstat));

	seconds = fstc_second = time.tv_sec - sc->ztime;
	if (seconds & 0xffff0000)
	    ctr->fst_second = 0xffff;
	else
	    ctr->fst_second = seconds & 0xffff;

	/* driver counter */

	ctr->fst_bytercvd   = fstc_bytercvd;
	ctr->fst_bytesent   = fstc_bytesent;
	ctr->fst_pdurcvd    = fstc_pdurcvd;
	ctr->fst_pdusent    = fstc_pdusent;
	ctr->fst_mbytercvd  = fstc_mbytercvd;
	ctr->fst_mpdurcvd   = fstc_mpdurcvd;
	ctr->fst_mbytesent  = fstc_mbytesent;
	ctr->fst_mpdusent   = fstc_mpdusent;
	ctr->fst_pduunrecog  = fstc_pduunrecog = sc->is_ctrblk.est_unrecog;
	ctr->fst_mpduunrecog = fstc_mpduunrecog;
	ctr->fst_fcserror    = fstc_fcserror ;
	ctr->fst_fseerror    = fstc_fseerror ;
	ctr->fst_pdulen      = fstc_pdulen;
	ctr->fst_pdualig     = fstc_pdualig;
	ctr->fst_ringbeaconrecv = sc->ndbr ; 

	/* adapter counter */ 
	if (xcmd->frame_count.hi) 
		ctr->fst_frame = fstc_frame  = 0xffffffff;
	else
		ctr->fst_frame = fstc_frame = xcmd->frame_count.lo;

	if (xcmd->error_count.hi)
		ctr->fst_error = fstc_error = 0xffffffff;
	else
		ctr->fst_error = fstc_error = xcmd->error_count.lo;

	if (xcmd->lost_count.hi)
		ctr->fst_lost = fstc_lost = 0xffffffff;
	else
		ctr->fst_lost = fstc_lost = xcmd->lost_count.lo;

	if (xcmd->xmt_fail.hi || 
			(xcmd->xmt_fail.lo & 0xffff0000))
	    ctr->fst_sendfail = fstc_sendfail = 0xffff;
	else
	    ctr->fst_sendfail = fstc_sendfail = *(u_short*)(&xcmd->xmt_fail.lo);

	if (xcmd->xmt_underrun.hi ||
			(xcmd->xmt_underrun.lo & 0xffff0000))
		ctr->fst_underrun = fstc_underrun = 0xffff;
	else
		ctr->fst_underrun = fstc_underrun = *(u_short*)(&xcmd->xmt_underrun.lo);

	if ( xcmd->rcv_overrun.hi || 
			(xcmd->rcv_overrun.lo & 0xffff0000))  
	   	ctr->fst_overrun = fstc_overrun = 0xffff;

	else
	    	ctr->fst_overrun = fstc_overrun = *(u_short*)(&xcmd->rcv_overrun.lo);

	if (xcmd->sysbuf.hi ||
			(xcmd->sysbuf.lo & 0xffff0000))
	    	ctr->fst_sysbuf = fstc_sysbuf = 0xffff;
	else
	    	ctr->fst_sysbuf = fstc_sysbuf = *(u_short*)(&xcmd->sysbuf.lo);

	if (xcmd->ring_init_init.hi ||
			(xcmd->ring_init_init.lo & 0xffff0000))
		ctr->fst_ringinit = fstc_ringinit = 0xffff;
	else
		ctr->fst_ringinit = fstc_ringinit = *(u_short*)(&xcmd->ring_init_init.lo);

	if (xcmd->ring_init_rcv.hi ||
			(xcmd->ring_init_rcv.lo & 0xffff0000))
		ctr->fst_ringinitrcv = fstc_ringinitrcv = 0xffff;
	else
		ctr->fst_ringinitrcv = fstc_ringinitrcv = *(u_short*)(&xcmd->ring_init_rcv.lo);

	if (xcmd->ring_beacon_init.hi ||
			(xcmd->ring_beacon_init.lo & 0xffff0000))
		ctr->fst_ringbeacon = fstc_ringbeacon = 0xfffff;
	else
		ctr->fst_ringbeacon = fstc_ringbeacon = *(u_short*)(&xcmd->ring_beacon_init.lo);

	if (xcmd->dup_addr_fail.hi ||
			(xcmd->dup_addr_fail.lo & 0xffff0000))
		ctr->fst_dupaddfail = fstc_dupaddfail = 0xffff;
	else
		ctr->fst_dupaddfail = fstc_dupaddfail = *(u_short*)(&xcmd->dup_addr_fail.lo);

	if (xcmd->ring_purge_err.hi ||
			(xcmd->ring_purge_err.lo & 0xffff0000))
		ctr->fst_ringpurge = fstc_ringpurge = 0xffff;
	else
		ctr->fst_ringpurge = fstc_ringpurge = *(u_short*)(&xcmd->ring_purge_err.lo);

	if (xcmd->dup_token.hi || 
			(xcmd->dup_token.lo & 0xffff0000))
		ctr->fst_duptoken = fstc_duptoken = 0xffff;
	else
		ctr->fst_duptoken = fstc_duptoken = *(u_short*)(&xcmd->dup_token.lo);

	if (xcmd->bridge_strip_err.hi ||
			(xcmd->bridge_strip_err.lo & 0xffff0000))
		ctr->fst_bridgestrip= fstc_bridgestrip= 0xffff;
	else
		ctr->fst_bridgestrip= fstc_bridgestrip= *(u_short*)(&xcmd->bridge_strip_err.lo);

	if (xcmd->trace_init.hi || 
			(xcmd->trace_init.lo & 0xffff0000))
		ctr->fst_traceinit= fstc_traceinit= 0xffff;
	else
		ctr->fst_traceinit= fstc_traceinit= *(u_short*)(&xcmd->trace_init.lo);

       	if (xcmd->trace_rcvd.hi ||
                        (xcmd->trace_rcvd.lo & 0xffff0000))
                ctr->fst_tracerecv= fstc_tracerecv= 0xffff;
        else
                ctr->fst_tracerecv= fstc_tracerecv= *(u_short*)(&xcmd->trace_rcvd.lo);

        if (xcmd->lem_rej.hi ||
                        (xcmd->lem_rej.lo & 0xffff0000))
                ctr->fst_lem_rej= fstc_lem_rej= 0xffff;
        else
                ctr->fst_lem_rej= fstc_lem_rej= *(u_short*)(&xcmd->lem_rej.lo);

	if (xcmd->lct_rej.hi ||
                        (xcmd->lct_rej.lo & 0xffff0000))
                ctr->fst_lct_rej= fstc_lct_rej= 0xffff;
        else
                ctr->fst_lct_rej = fstc_lct_rej= *(u_short*)(&xcmd->lct_rej.lo);

	if (xcmd->tne_exp_rej.hi ||
                        (xcmd->tne_exp_rej.lo & 0xffff0000))
                ctr->fst_tne_exp_rej= fstc_tne_exp_rej= 0xffff;
        else
                ctr->fst_tne_exp_rej = fstc_tne_exp_rej= *(u_short*)(&xcmd->tne_exp_rej.lo);

	if (xcmd->lem_event.hi ||
                        (xcmd->lem_event.lo & 0xffff0000))
                ctr->fst_lem_events = fstc_lem_events = 0xffff;
        else
                ctr->fst_lem_events = fstc_lem_events = *(u_short*)(&xcmd->lem_event.lo);

        if (xcmd->connection.hi ||
                        (xcmd->connection.lo & 0xffff0000))
                ctr->fst_connection= fstc_connection= 0xffff;
        else
                ctr->fst_connection= fstc_connection= *(u_short*)(&xcmd->connection.lo) ;

        if (xcmd->elasticity_buf_err.hi ||
                        (xcmd->elasticity_buf_err.lo & 0xffff0000))
                ctr->fst_ebf_error = fstc_ebf_error = 0xffff;
        else
                ctr->fst_ebf_error = fstc_ebf_error = *(u_short*)(&xcmd->elasticity_buf_err.lo) ;

}


FZACMDRING *fzamkcmd(sc,cmdid,unit,ifc)
	register struct fza_softc *sc;
	int cmdid,unit;
	struct ifchar *ifc;
{
	register FZACMDRING *cp;
	register union fzacmd_buf *cmdbp;

	struct ifnet *ifp = &sc->is_if;

	cp = &sc->cmdring[sc->cmdindex];
	if(!(cp->cmd_own)) {
#if FZADEBUG
		if(fzadebug)
			printf("fza%d: command ring is not own by host\n",unit);
#endif
		return((FZACMDRING *)0); /* no ring owned by host */
	}

	cmdbp = (union fzacmd_buf *)(cp->buf_addr + sc->basereg);

#if FZADEBUG
	if(fzadebug > 1) {
		printf("fzamkcmd: cmd addr 0x%lx base addr 0x%lx\n",
		    cp->buf_addr, sc->basereg);
		printf("cmdid: %d, index %d\n",cmdid,sc->cmdindex );
	}
#endif
	switch(cmdid) {

	case CMD_INIT:  /* INIT command */
		cmdbp->fzainit.xtm_mode = FZA_XMT_MODE;
		cmdbp->fzainit.rcv_entry = NFZARCV;

		/* 
		 * copy the initial counter value
		 */
#if FZADEBUG
		if(fzadebug > 1)
			printf("init cmd: cp from 0x%lx to 0x%lx len %d\n",
				sc->ctrblk,
				&cmdbp->fzainit.fzactrs,
				sizeof(struct _fzactrs));
#endif

		fzacpy(sc->ctrblk,&cmdbp->fzainit.fzactrs,sizeof(struct _fzactrs));
		break;

	case CMD_PARAM:  /* PARAM command */
		if(ifp->if_flags & IFF_LOOPBACK)
			cmdbp->fzaparam.loop_mode = LOOP_INTER;
		else
			cmdbp->fzaparam.loop_mode = LOOP_NORMAL;
		cmdbp->fzaparam.t_max = sc->t_max; 
		cmdbp->fzaparam.t_req = sc->t_req;
		cmdbp->fzaparam.tvx = sc->tvx;
		cmdbp->fzaparam.lem_threshold = sc->lem_threshold;
		cmdbp->fzaparam.station_id.lo = sc->station_id.lo;
		cmdbp->fzaparam.station_id.hi = sc->station_id.hi;
		/* cmdbp->fzaparam.rtoken_timeout = sc->rtoken_timeout; */ 
		cmdbp->fzaparam.ring_purger = sc->ring_purger ;  
		break;

	case CMD_MODCAM:  /* MODCAM command */
		fzacpy(&sc->is_multi[0][0],cmdbp,512);
		break;

	case CMD_MODPROM:
		if(ifp->if_flags & IFF_PROMISC)
			cmdbp->fzamodprom.llc_prom = 1;
		else
			cmdbp->fzamodprom.llc_prom = 0;
		if(ifp->if_flags & IFF_ALLMULTI)
			cmdbp->fzamodprom.llc_multi_prom = 1;
		else
			cmdbp->fzamodprom.llc_multi_prom = 0;
		cmdbp->fzamodprom.smt_prom = 0;
		cmdbp->fzamodprom.llc_broad_prom = 0;
		break;

	case CMD_SETCHAR: /* SET CHAR command */
		cmdbp->fzasetchar.t_req = ifc->ifc_treq;
		cmdbp->fzasetchar.tvx = ifc->ifc_tvx;
		cmdbp->fzasetchar.rtoken_timeout  = ifc->ifc_rtoken;
		cmdbp->fzasetchar.lem_threshold = ifc->ifc_lem;
		cmdbp->fzasetchar.ring_purger = ifc->ifc_ring_purger;
		break;

	case CMD_RDCNTR:
	case CMD_STATUS:
	case CMD_RDCAM:
	case CMD_NOP:
		break;

	default:
#if FZADEBUG
		if(fzadebug) 
			printf("fza%d: non supported command id %d\n",
				unit,cmdid);
#endif
		break;
	}

	cp->cmd_all = (0x7fffffff & cmdid );  /* own by rmc and set the cmdid */ 
	/* cp->cmd_own = 0;  own by rmc  */
	FZCSR_WR(sc->reg_ctla,CMD_POLL);
	WBFLUSH();

	sc->cmdindex = ++sc->cmdindex % NFZACMD;
	return(cp);
}

fzacmdstatus(statusid,unit,s)
int statusid,unit;
char *s;
{
#if FZADEBUG
	if(fzadebug)
		printf("%s routine: ",s);
#endif
	switch(statusid) {
		case CMD_SUCCESS:
			return(1);
			break;
		case CMD_STATE_INVALID:
			printf("fza%d: Invalid Adapter State\n",unit);
			break;
		case CMD_XTM_INVALID:
			printf("fza%d: Invalid Transmit Mode\n",unit);
			break;
		case CMD_RCVENT_INVALID:
			printf("fza%d: Invalid Host Receive Entries\n");
			break;
		case CMD_LOOPBK_INVALID:
			printf("fza%d: Invalid Lookback Mode\n");
			break;
		case CMD_TMAX_INVALID:
			printf("fza%d: Invalid T_MAX value\n",unit);
			break;
		case CMD_TREQ_INVALID:
			printf("fza%d: Invalid T_REQ value\n", unit);
			break;
		case CMD_TVX_INVALID:
			printf("fza%d: Invalid TVX value\n", unit);
			break;
		case CMD_LEM_INVALID:
			printf("fza%d: Invalid LEM value\n", unit);
			break;
		case CMD_STATION_ID_INVALID:
			printf("fza%d: Invalid Station ID\n", unit);
			break;
		case CMD_CMD_INVALID:
			printf("fza%d: Invalid Command\n", unit);
			break;
		case CMD_LLC_MULTI_INVALID:
			printf("fza%d: Invalid LLC Multicast Promiscous Mode\n", unit);
			break;
		case CMD_SMTPROM_INVALID:
			printf("fza%d: Invalid SMT Promiscous Mode\n", unit);
			break;
		case RTOKEN_TIMEOUT_INVALID:
			printf("fza%d: Invalid Ristricted Token Timeout Value\n", unit);
			break;
		case RING_PURGER_INVALID:
			printf("fza%d: Invalid Ring Purger Value\n", unit);
			break;
		case CMD_LLC_PHY_INVALID:
			printf("fza%d: Invalid LLC Physical Promiscous Mode\n", unit);
			break;
		case CMD_LLC_BRO_INVALID:
			printf("fza%d: Invalid LLC Broadcast Promiscous Mode\n", unit);
			break;
		default:
			printf("fza%d: unknown command status %d\n",unit,statusid);
			break;
	}
	return(0);
}

/*
 * routine to fill up the snmp FDDI MIB attributes  
 */
fmib_fill(sc,fmib,type)
register struct fza_softc *sc;
struct ctrreq *fmib;
int type;
{

        switch (type) {
                case FDDIMIB_SMT:       /* SMT group */
                        /* adapter will provide this number */
			fmib->fmib_smt.smt_number = fddi_units;
			fmib->fmib_smt.smt_index = sc->is_if.if_unit + 1;
			bcopy(sc->is_dpaddr,&fmib->fmib_smt.smt_stationid[0],6);
                        fmib->fmib_smt.smt_opversionid = sc->smt_version ;
                        fmib->fmib_smt.smt_hiversionid = sc->smt_maxid ;
                        fmib->fmib_smt.smt_loversionid = sc->smt_minid ;
                        fmib->fmib_smt.smt_macct = 1;
                        fmib->fmib_smt.smt_nonmasterct = 1;
                        fmib->fmib_smt.smt_masterct = 0;
                        fmib->fmib_smt.smt_pathsavail = 1;
                        fmib->fmib_smt.smt_configcap = 0;
                        fmib->fmib_smt.smt_configpolicy = 0;
                        fmib->fmib_smt.smt_connectpolicy = 0x8029;
                        fmib->fmib_smt.smt_timenotify = 30;
                        fmib->fmib_smt.smt_statusreport = 2 ;
                        fmib->fmib_smt.smt_ecmstate = 2 ;
                        fmib->fmib_smt.smt_cfstate = 2;
                        fmib->fmib_smt.smt_holdstate = 1;
                        fmib->fmib_smt.smt_remotedisconn = 2 ;
                        /* adapter need to provide this */
                        /* fmib->fmib_smt.smt_msgtimestamp[8]  */
			fmib->fmib_smt.smt_action = 1;
                        break;

                case FDDIMIB_MAC:       /* MAC group */
                        fmib->fmib_mac.mac_number = fddi_units;
                        fmib->fmib_mac.mac_smt_index = sc->is_if.if_unit + 1;
                        fmib->fmib_mac.mac_fsc = 1;
                        fmib->fmib_mac.mac_gltmax = 2097 ; /*167.77224*1000/80*/
                        fmib->fmib_mac.mac_gltvx = 31 ; /* 2.5*1000/80
*/
                        fmib->fmib_mac.mac_paths = 1;
                        fmib->fmib_mac.mac_current = 2;
			bcopy(&sc->statusblk->upstream[0],
				&fmib->fmib_mac.mac_upstream[0],6);
                        bcopy(&sc->statusblk->old_una_address[0],
                                &fmib->fmib_mac.mac_oldupstream[0],6);
			fmib->fmib_mac.mac_dupaddrtest = sc->statusblk->dup_add_test;
                        fmib->fmib_mac.mac_pathsreq = 1;
                        fmib->fmib_mac.mac_downstreamtype = 2;
			bcopy(sc->is_dpaddr,&fmib->fmib_mac.mac_smtaddress[0],6);
			fmib->fmib_mac.mac_treq = sc->t_req;
			fmib->fmib_mac.mac_tneg = sc->statusblk->neg_trt;
                        fmib->fmib_mac.mac_tmax = sc->t_max;
                        fmib->fmib_mac.mac_tvx = sc->tvx;
                        fmib->fmib_mac.mac_tmin = 500 ; /* 40000/80 */
                        fmib->fmib_mac.mac_framestatus = 1;
                        fmib->fmib_mac.mac_counter = sc->ctrblk->frame_count.lo;
                        fmib->fmib_mac.mac_error = sc->ctrblk->error_count.lo;
                        fmib->fmib_mac.mac_lost = sc->ctrblk->lost_count.lo;
                        fmib->fmib_mac.mac_rmtstate = sc->statusblk->rmt_state;
			if(sc->statusblk->dup_add_test == 2)
                                fmib->fmib_mac.mac_dupaddr = 1;
                        else
                                fmib->fmib_mac.mac_dupaddr = 2;
			if(sc->statusblk->up_dup_flag == 0 )
                        	fmib->fmib_mac.mac_updupaddr = 2;
                        else
                        	fmib->fmib_mac.mac_updupaddr = 1;
                        fmib->fmib_mac.mac_condition = 2;
                        fmib->fmib_mac.mac_action = 1;
                        break;

                case  FDDIMIB_PORT:     /* PORT group */
                        fmib->fmib_port.port_number = fddi_units;
                        fmib->fmib_port.port_smt_index = sc->is_if.if_unit +1 ;
                        fmib->fmib_port.port_index = 1 ;
                        fmib->fmib_port.port_pctype = 3;
                        fmib->fmib_port.port_pcneighbor =
                                sc->statusblk->neighbor_phy_type + 1;
                        fmib->fmib_port.port_connpolicy = 4;
                        fmib->fmib_port.port_remoteind =
                                        sc->statusblk->remote_mac_ind;
                        if(sc->statusblk->phy_state == 7)
                                fmib->fmib_port.port_CEstate = 2;
                        else
                                fmib->fmib_port.port_CEstate = 1;
                        fmib->fmib_port.port_pathreq = 1;
                        fmib->fmib_port.port_placement = 1;
                        fmib->fmib_port.port_availpaths = 1;
                        fmib->fmib_port.port_looptime = 2500; /* 200*1000/80 */
                        fmib->fmib_port.port_TBmax = 625;  /* 50*1000/80 */
                        fmib->fmib_port.port_BSflag = 2;
                        fmib->fmib_port.port_LCTfail = sc->ctrblk->lct_rej.lo;
                    	fmib->fmib_port.port_LerrEst = sc->statusblk->phy_link_error; 
			fmib->fmib_port.port_Lemreject = sc->ctrblk->lem_rej.lo;
                        fmib->fmib_port.port_Lem = sc->ctrblk->lem_event.lo;
                        fmib->fmib_port.port_baseLerEst = 0 ;
                        fmib->fmib_port.port_baseLerrej = 0;
                        fmib->fmib_port.port_baseLerrej = 0;
                        fmib->fmib_port.port_baseLer = 0;
			/* need work */
                        /* fmib->fmib_port.port_baseLerTime = 0; */
                        fmib->fmib_port.port_Lercutoff = sc->lem_threshold;
                        fmib->fmib_port.port_alarm = sc->lem_threshold;
                        switch(sc->statusblk->phy_state) {
                                case 2 :
					fmib->fmib_port.port_connectstate = 1;
                                        fmib->fmib_port.port_PCMstate = 1;
                                        break;
                                case 5 :
                                case 6 :
					fmib->fmib_port.port_connectstate = 3;
                                        fmib->fmib_port.port_PCMstate = 5;
                                        break;
                                case 7 :
					fmib->fmib_port.port_connectstate = 4;
                                        fmib->fmib_port.port_PCMstate = 9;
              				break;
                                default :
					fmib->fmib_port.port_connectstate = 2;
                                        fmib->fmib_port.port_PCMstate = 5;
                                        break;
			}
			fmib->fmib_port.port_PCwithhold = 0 ;
                        fmib->fmib_port.port_Lercondition = 2 ;
                        fmib->fmib_port.port_action = 1 ;
                        break;
                case FDDIMIB_ATTA:
                        fmib->fmib_atta.atta_number = fddi_units;
                        fmib->fmib_atta.atta_smt_index = sc->is_if.if_unit + 1 ;
                        fmib->fmib_atta.atta_index = 1 ;
                        fmib->fmib_atta.atta_class = 1;
                        fmib->fmib_atta.atta_bypass = 2;
                        fmib->fmib_atta.atta_IMaxExpiration = 0;
                        fmib->fmib_atta.atta_InsertedStatus = 3;
                        fmib->fmib_atta.atta_InsertPolicy = 3;
                        break;
        }

}

/*
 * DEFZA can only support words read and short words/words write. 
 * In addition, the short words write must be aligned correctly within the 
 * words. 
 * This specialized copy routine will check the dst address and make it
 * words aligned. Then, it will use the "bcopy" routine to copy the
 * data until the word aligned tail.
 */

fzacpy(from,to,len)
caddr_t from; 
caddr_t to;
int len;
{


	register int tlen = len; 
	register caddr_t srcp = from;
	register caddr_t dstp = to;
	register u_short tmp;
	int nbyte;

	/* align the destination address */

	/* check for short word aligned */
	if((nbyte = ((u_long)dstp % 4))) {
		switch(nbyte) {
			case 1:
			case 3:
				tmp = *(u_short *)(--dstp) & 0xff;
				*(u_short *)(dstp) = tmp | (u_short)(*srcp++ & 0xff) << 8;
				tlen--;
				dstp = dstp + 2;
				if(nbyte == 3 || tlen < 2 )
					break;
			case 2:
				tmp = (u_short)(*srcp++ & 0xff);
				*(u_short *)(dstp) = tmp | (u_short)(*srcp++ & 0xff) << 8;
				dstp += 2;
				tlen -= 2;
				break;
			}
	} 

        if( tlen >=  8 ) {
		if((nbyte = tlen % 4)) { /* check for word align copy */
			bcopy(srcp,dstp,tlen - nbyte);
			srcp += tlen - nbyte ; 
			dstp += tlen - nbyte ;
			switch(nbyte) {
				case 1 :
					*(u_short *)(dstp) = (u_short)(*srcp & 0xff);
					break;
				case 2 :
				case 3 :
					tmp = (u_short)(*srcp++ & 0xff);
					*(u_short *)(dstp) = tmp | (u_short)(*srcp++ & 0xff) << 8;
					if(nbyte == 3 ) {
						dstp += 2 ;
						 *(u_short *)(dstp) = (u_short)(*srcp & 0xff);
					}
					break;

			}

		} else
			bcopy (srcp,dstp,tlen);
	} else {
		while ( tlen >= 2 ) {
			tmp = (u_short)(*srcp++ & 0xff);
			*(u_short *)(dstp) = tmp | (u_short)(*srcp++ & 0xff) << 8;
			dstp += 2;
			tlen -= 2;
		}
		if(tlen)
			*(u_short *)(dstp) = (u_short)(*srcp & 0xff);
	}
	FZIOSYNC();		/* make DMA buffer visible to adapter */
}
#endif 

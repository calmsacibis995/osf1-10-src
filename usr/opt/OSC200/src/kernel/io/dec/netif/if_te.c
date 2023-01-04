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
static char *rcsid = "@(#)$RCSfile: if_te.c,v $ $Revision: 1.1.12.14 $ (DEC) $Date: 1993/12/20 18:52:42 $";
#endif
/*
 * Digital TGEC Network Interface
 */
#include "te.h"
#include <data/if_te_data.c>

extern struct protosw *iftype_to_proto(), *iffamily_to_proto();
extern int ether_output();
extern struct timeval time;
extern struct io_csr *Io_regs;
extern vm_offset_t te_bufptr[NTE];

int	teprobe(), teattach(), teintr(), tereset();
int	teinit(), teoutput(), teioctl(), tewatch();

u_short testd[] = { 0 };
struct	driver tedriver =
{ teprobe, 0, teattach, 0, 0, 0 , 0, 0,  "te", teinfo };

u_char te_unused_multi[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
u_char te_multi[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00 };


/*
 * macros to call mbox csr macros
 */

/*
 * returns the softc data structure for the TGEC with passed controller
 *	arguments	
 *		c	controller with unit number
 */
#define TE_UNIT_SC(c) 	te_softc[(c)->ctlr_num]

/*
 * returns the secondary address of the TGEC unit
 *	arguments	
 *		c	controller with unit number
 */
#define TE_UNIT_SADDR(c) (TE_UNIT_SC(c).te_csrs)

/*
 * write tgec csr
 *	arguments	
 *		c	controller with unit number
 *		r	register (as defined in if_tereg.h)
 *		d	data to write to the register
 * return value - none
 */
#define	WRT_TGEC_CSR(c,r,d) WRTCSR(LONG_32, (c), (TE_UNIT_SADDR(c) | (r)), (d))
/*
 * read tgec csr
 *
 * arguments	
 *	c	controller with unit number
 *	r	register (as defined in if_tereg.h)
 *
 * return value - register data
 */
#define	RD_TGEC_CSR(c,r)     RDCSR(LONG_32, (c), (TE_UNIT_SADDR( c ) | (r)))

/*
 * read tgec status register
 *
 *	arguments	
 *		c	controller
 *
 * return value - status register contents
 */
#define RD_TGEC_STATUS(c)	RD_TGEC_CSR((c), CSR5_SADDR)

/*
 * write tgec status register
 *
 * arguments	
 *	c	controller
 *	d	data to write to status register
 *
 * return value - none
 */
#define WRT_TGEC_STATUS(c,d) 	WRT_TGEC_CSR((c),CSR5_SADDR, d)

/*
 * clear tgec status register by writing back bits that are set
 *
 * arguments	
 *	c	controller
 *	d	data to write to status register, has status after execution
 *
 * return value - none
 */
#define CLEAR_TGEC_STATUS(c,d)	{			\
	d = RD_TGEC_STATUS(c);				\
	WRT_TGEC_STATUS(c,d);				\
}

/*
 * macro to read virtual TGEC csr
 *	1. read csr, operation completes, but no valid data sent to host
 *	2. Wait for dn bit in status register
 *	3. reissue read csr to the SAME csr as step 1, and the host
 *	   will receive valid data.	
 *
 * arguments	
 *	c	controller
 *	a 	secondary address of the register
 *	r	data in the register
 *
 * return value - none
 */
#define RD_TGEC_CSR_VIRT(c,a,r)	{				\
	int d, st;						\
	r = RD_TGEC_CSR(c, a);					\
 	st = RD_TGEC_STATUS(c);					\
	for (d = 100000; d && !(st & TE_CSR5_DN); d--)		\
		st = RD_TGEC_STATUS(c);				\
	if (d == 0)						\
		printf("tgec virtual csr unreadable");		\
	else							\
		r = RD_TGEC_CSR(c, a);				\
}	
/*
 * macro to write virtual TGEC csr
 *	1. write csr, operation completes, but the data is not yet copied to the
 *	   tgec
 *	2. Wait for dn bit in status register
 *
 * arguments	
 *	c	controller
 *	a 	secondary address of the register
 *	w	data to write to the register
 *
 * return value - none
 */
#define WRT_TGEC_CSR_VIRT(c,a,w)	{			\
	int d, st;						\
	WRT_TGEC_CSR(c, a, w);					\
 	st = RD_TGEC_STATUS(c);				\
	for (d = 100000; d && !(st & TE_CSR5_DN); d--)	       	\
		st = RD_TGEC_STATUS(c);				\
	if (d == 0)						\
		printf("tgec virtual csr unwriteable");		\
}	

/*
 * read ethernet address ROM
 *
 * arguments	
 *	c	controller with unit number
 *	j	byte number
 *
 * return value - contents of ethernet address ROM
 */							
#define RD_ETH_ADDR(c,r)  	RDCSR(LONG_32,(c),(r))

/*
 * issue transmit poll demand on csr1
 *
 * arguments	
 *	c	controller with unit number
 *
 * return value - none
 */
#define TRANS_POLL(c)	 WRT_TGEC_CSR(c, CSR1_SADDR,TE_CSR1_PD);

/*
 * set the command register csr6
 *
 * arguments	
 *	c	controller with unit number
 *	d	command to be issued
 *
 * return value - none
 */
#define ISSUE_TGEC_COMMAND(c,d)  WRT_TGEC_CSR(c, CSR6_SADDR ,d);

/*
 * macro returning contents of LINT
 *
 * return value - lint register
 */
#define LINT		Io_regs->io_lint

/*
 * macro to return the lint bit for the controller
 *
 * arguments
 *	c 	controller with unit number
 *
 * return value - lint bit for controller
 */
#define LINT_BIT(c)	(TE_UNIT_SC(c).te_lint)

/*
 * macro to return true if the LINT bit for the controller is set
 *
 * arguments	
 *	c	controller with unit number
 *
 * return value 
 *	0	lint bit clear
 *	1	lint bit set
 */
#define LINT_SET(c) 	(LINT & LINT_BIT(c))

/*
 * macro to clear the lint bit for the controller
 *
 * arguments	
 *	c	controller with unit number
 *
 * return value - none
 */
#define CLEAR_LINT(c)	LINT = LINT_BIT(c)

/*
 * macro to return true if receive packets are pending on the controller
 * Packets are pending if the current packet is owned by the TGEC
 *
 * arguments	
 *	c	controller with unit number
 *
 * return value 
 *	0 	no packets pending
 *	1 	packets pending
 */
#define RECV_PACKETS_PENDING(c)	\
	(!((TE_UNIT_SC(c).rring[s->rindex].te_own) & TE_OWN))

/*
 * macro to return true if receive packets are pending on the controller
 * transmit packets are pending if number of transmits pending is > 0
 *
 * arguments	
 *	c	controller with unit number
 *
 * return value 
 *	0 	no packets pending
 *	1 	packets pending
 */
#define TRANS_PACKETS_PENDING(c) ((TE_UNIT_SC(c).nxmit) != 0)

/*
 * macro to make it easier to play with pl levels
 *
 * arguments		none
 *
 * return value 	none
 */
#define SPL_TE()	splimp()

/*
 * macro to increment x up to a maximum of m
 */
#define INCR(x,m)	x = ((x != (unsigned) m) ? x++ : m)

/* 
 * error printfs 
 */
/* #define EPRINTF(x) 	printf x */
#define EPRINTF(x)
/*  
 * #define DEBUG_SETUP_FRAME	define to ignore broadcast packets 
 */

/*
 * variable used as a switch when debugging with dbx -k
 */
int	tedebug = 0; 


/*
 * macro get an mbuf
 * Arguments
 * 	m	mbuf
 * 	f 	flag for M_WAIT or M_DONTWAIT
 */
#define TEMCLGET(m,f) { 			\
      MGETHDR((m), (f), MT_DATA); 		\
      if ((m)) { 				\
	 MCLGET2((m), (f)); 			\
	 if (((m)->m_flags & M_EXT) == 0) { 	\
	     m_freem((m));			\
	     (m) = (struct mbuf *)NULL; 	\
         } 					\
      } 					\
}

/*
 * macro to align the mbuf
 * Arguments
 * 	start	starting address
 *	align	number of bits to align.
 */
#define TEALIGN(start,align)	(((int)start&align) ? 			\
				 (caddr_t)((((int)start)&(~align))+ 	\
					   align+1): (caddr_t)(start))

/*
 * teprobe()
 * Probe the TGEC to see if it's there 
 *	initialize date in the softc structure for the driver
 *	reset the chip and check self test status
 *	initialize csr0 (Vector Address, IPL, Sync/Asynch)
 *	allocate and initialize rings
 *	allocate and initialize mbufs
 *	initialize multicast address table
 *	read narom and get ethernet address for controller
 *
 * Arguments:
 *	unit_address 	unused 
 *	ctlr		struct controller containing the controller number.
 *
 * Return Value:
 *	Success:	~0
 *	Failure:	0
 */
teprobe(unit_address, ctlr)
	caddr_t unit_address;
	struct controller  *ctlr;
{	
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];

	if (!tescinit(ctlr))
		return(0);

	if (!tesoftreset(ctlr))
		return(0);  	

	if (!tesetcsr0(ctlr))
		return(0);  	
	
	if (!teinitmbufs(ctlr))
		return(0);

	if (!teinitrings(ctlr))
		return(0);

	teinitmulticast(ctlr);
	tereadnarom(ctlr);	

	return(~0);
}

/*
 * teattach()
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets. Print ethernet address for unit.
 *
 * Arguments:
 *	ctlr		struct controller containing the controller number.
 *
 * Return Value:
 *	None.
 */
teattach(ctlr)
	struct	controller *ctlr;
{
	struct item_list mop_sysid;
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register struct ifnet *ifp = &sc->is_if;
        register struct sockaddr_in *sin;
	extern int cpu;

	ifp->if_addrlen = 6;	/* media address len */
	ifp->if_hdrlen = sizeof(struct ether_header) + 8  ;

	sc->is_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
	sc->is_ac.ac_arphrd = ARPHRD_ETHER; 
	ifp->if_unit = unit;
	ifp->if_name = "te";
	ifp->if_mtu = ETHERMTU;
	ifp->if_mediamtu = ETHERMTU;
	ifp->if_type = IFT_ETHER;
	ifp->if_flags |= (IFF_SIMPLEX | IFF_BROADCAST | IFF_MULTICAST | 
			  IFF_NOTRAILERS);
	((struct arpcom *)ifp)->ac_ipaddr.s_addr = 0;
        sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;
	ifp->if_init = teinit;
	ifp->if_output = ether_output;
	ifp->if_start = teoutput;
	ifp->if_ioctl = teioctl;
	ifp->if_timer = 0;
	ifp->if_watchdog = tewatch;
	ifp->if_reset = tereset;
    	ifp->if_baudrate = ETHER_BANDWIDTH_10MB;

	mop_sysid.function = MOP_SYSID;
	mop_sysid.out_flags = IN_LINE;
	mop_sysid.in_flags = 0;
	mop_sysid.next_function = NULL;
	mop_sysid.input_data = 0;
	if (get_info(&mop_sysid))
		ifp->if_sysid_type = mop_sysid.output_data;
	else {
		printf("teprobe : Unsupported system type.\n");
		return(0);
	}

	/*
	 * Get the TGEC version
	 */
	sc->te_hw_rev = te_hw_rev(ctlr);
	sc->te_fw_rev = te_fw_rev(ctlr);
	ifp->if_version = "DEC TGEC Ethernet Interface";

	printf("te%d: %s, HV%d  FV%d hardware address: %s \n", unit,
	       ifp->if_version, sc->te_hw_rev, sc->te_fw_rev, 
	       ether_sprintf(sc->is_addr)); 

	attachpfilter(&(sc->is_ed));
	if_attach(ifp);
}

/* 
 * teinit()
 * Initialize interface, apply tgec patch if necessary.
 * Allocate the mbufs for the receive and transmit  rings,
 * fill up the setup frame and start the interface.
 *
 * Arguments:
 *	unit		The unit number of the interface
 *
 * Return Value:
 *	Success:	~EIO		
 *	Failure:	EIO		
 */
teinit(unit)
	register int unit;
{
	register struct controller *ctlr = teinfo[unit];
	register struct te_softc *sc = &te_softc[unit];
	register struct ifnet *ifp = &sc->is_if;
	register TEDESC *rp;
	register struct mbuf *m;	
	register s, csr5, csr, i, status, delay;

	/* address not known */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;
	if (ifp->if_flags & IFF_RUNNING){
		return;
	}

	s = SPL_TE();

	if ((sc->te_hw_rev == 1) && (sc->te_fw_rev ==  1))
		tgec_patch(ctlr);

	if (ifp->if_flags & IFF_PROMISC) {
		sc->te_cmdcsr |= TE_AF_PRO ;
		csr = RD_TGEC_CSR(ctlr, CSR6_SADDR);
		ISSUE_TGEC_COMMAND(ctlr, csr & ~(TE_CSR6_SR | TE_CSR6_ST));
	} else {
		sc->te_cmdcsr &= ~TE_AF_PRO ;

       		/* prepare the setup frame */
		tesetup(ctlr,TE_SETUP_IC);
		sc->tring[0].te_bfaddr = KSEG_TO_PHYS((vm_offset_t)sc->te_setup); 
        	sc->tring[0].te_bsize = TE_SETUPSIZE;

		/* info field set to setup frame, 
		 * and interrupt on setup completion 
		 */
        	sc->tring[0].te_info = TE_DT_TSET | TE_IC;
        	sc->tring[0].te_own = TE_OWN;

		/* startup the setup packet and wait for interrupt */
		WRT_TGEC_CSR(ctlr, CSR4_SADDR, sc->tring_phys);
		ISSUE_TGEC_COMMAND(ctlr,
				   (sc->te_cmdcsr | TE_CSR6_ST | TE_CSR6_IE));
		TRANS_POLL(ctlr);

		for (delay = 100; (delay>0) && !(LINT_SET(ctlr)); delay--){
			DELAY(10000);  
		}	

		csr5 = RD_TGEC_STATUS(ctlr);
		if(!delay || !(csr5 & TE_CSR5_TI)) {   
			printf("te%d : setup frame transmit timeout\n", unit);
			status = EIO;
			goto done;
		} else {
			if (sc->tring[0].te_flag & TE_SETUP_SE) {
				printf("te%d : setup frame error \n", unit);
				status = EIO;
				goto done;
			}
			/* clear flag set in tesetup */
			sc->setupqueued = 0;
		}
		CLEAR_LINT(ctlr);
		CLEAR_TGEC_STATUS(ctlr,csr5);
	}

	/* set up the receive buffers */
	for (i=0,rp=sc->rring ; i < sc->nrring; i++,rp++) {
		teinitdesc(rp,sc->rbufs[i]);
		rp->te_own = TE_OWN;		/* te owns receive buffers */
	}

	/* set up the transmit buffer */
	for (i=0,rp=sc->tring ; i < sc->ntring; i++,rp++) {
		teinitdesc(rp,sc->tbufs[i]);	/* host owns trans buffers */
	}

	sc->nxmit = sc->tindex = sc->otindex = sc->rindex = 0;
	/* fill out the descriptor address list */
	WRT_TGEC_CSR(ctlr, CSR3_SADDR, sc->rring_phys);
	WRT_TGEC_CSR(ctlr, CSR4_SADDR, sc->tring_phys);
	if (ifp->if_flags & IFF_ALLMULTI){
		sc->te_cmdcsr |= TE_AF_MUL ;	/* set all multicast */
	}

	csr = sc->te_cmdcsr | TE_CSR6_SR | TE_CSR6_ST | TE_CSR6_IE;
	if (ifp->if_flags & IFF_LOOPBACK) {
		/* if this is a internal loop back */
		csr |= TE_OM6_EXL;
	} else {
		csr |= TE_OM6_NOR;
	}
	ISSUE_TGEC_COMMAND(ctlr, csr);

	/*	
	 * mark the interface up; place the setup frame in the xmit list
	 */
	/*
	 * Changed this to be like the lance -- kmbl
	 * ifp->if_flags |= IFF_UP | IFF_RUNNING; (sgec version)
	 */
	ifp->if_flags |= IFF_RUNNING;
	ifp->if_flags &= ~IFF_OACTIVE;
	teoutput(ifp) ;

	/*
	 * set time counters zeroed
	 */
	sc->ztime = time.tv_sec;

	status = ~EIO;

      done:
	splx(s);
	return (status);
}

/*
 * tgec_patch()
 * Issue the patch written by P. Klein for version 3 tgecs.
 * Patch to avoid crossing an hexaword boundary  within the same burst 
 * when the buffer size is < 32. The patch applies only for Tx buffers 
 * (The Receive DMA is OK if  the Rx buffers are hexaword aligned 
 * or > 32 bytes).
 *
 * Arguments:
 *	ctlr		controller structure with unit number
 *
 * Return Value:
 * 	None.
 */
u_short diag_buffer[9] = {
   0xDC11, 0xD908, 0xDA00, 0xDC02, 0x9C02, 0xC357, 0x064B, 0xC357, 0x0655
};

tgec_patch(ctlr)
	register struct controller *ctlr;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register status, delay;
	register TEDESC *dp = &sc->tring[0];	/* descriptor pointer */

	bzero (dp, sizeof(TEDESC));	/* zero descriptor */

	/* DDES3:       buffer physical address (must be word aligned) */
	dp->te_bfaddr = KSEG_TO_PHYS((vm_offset_t)sc->te_diagbf); 

	/* DDES2:       000922AA   load size = 9 words, ram address = 22AAh */
	dp->te_bsize = 0x9;
	dp->te_pg_off = 0x22aa;	/* RAM address */

	/* setup frame, and interrupt on setup completion */
	/* DDES1:       30800000      DT = 3,  WD = 1, ST = 0 */
	dp->te_info = 0x3080;

	/* DDES0: 	80000000      OW = 1 */
	dp->te_own = TE_OWN;

	/* startup the packet and wait for interrupt */
	WRT_TGEC_CSR(ctlr, CSR4_SADDR, sc->tring_phys);
	ISSUE_TGEC_COMMAND(ctlr, (TE_CSR6_OM | TE_CSR6_ST | TE_CSR6_IE));
	TRANS_POLL(ctlr);

	for (delay = 100; (delay > 0) && !(LINT_SET(ctlr)); delay--){
		DELAY(10000);  
	}	
	
	status = RD_TGEC_STATUS(ctlr);
	if(!delay || !(status & TE_CSR5_TI)) {   
		printf("te%d : diagnostic frame transmit timeout\n", unit);
		status = EIO;
	}
	CLEAR_LINT(ctlr);
	CLEAR_TGEC_STATUS(ctlr, status);

	WRT_TGEC_CSR_VIRT(ctlr, CSR14_SADDR, 0xA2AA0648);
}

int Limited_Tx = 6 ; 
/*
 * teoutput(ifp)  
 * Ethernet output routine - start output on the interface 
 *
 * Arguments:
 *	ifp		The ifnet pointer, pointing to the intf. to o/p on.
 *
 * Return Value:
 * 	None.
 */
teoutput(ifp)
	register struct ifnet *ifp; 
{
	register int unit = ifp->if_unit;
	register struct te_softc *sc = &te_softc[unit];
	register struct controller *ctlr = teinfo[unit];
	register TEDESC *rp;
	register index;

	struct mbuf *m,*m0;
	int bufaddr,tlen,s;
	int nTENXMT = sc->ntring;
	int Ntx = 0 ; 

	for (index = sc->tindex; 
	     /*  SGEC version      
	      * (sc->tring[index].te_own != TE_OWN && sc->nxmit < (nTENXMT - 1));
	      */
	     sc->nxmit < (nTENXMT - 1) && Ntx < Limited_Tx ; /* kmbl - made it like the LANCE */
	     sc->tindex = index = ++index % nTENXMT, Ntx++ ){

		rp = &sc->tring[index];
		if(sc->setupqueued){
			/* sending a setup frame packet */
			rp->te_bfaddr = KSEG_TO_PHYS((vm_offset_t)sc->te_setup);
			tlen = TE_SETUPSIZE;  
			rp->te_info = TE_DT_TSET;
			sc->setupqueued = 0;
			if(sc->te_setup_ic) {
				rp->te_info |= TE_SETUP_IC ;
				sc->te_setup_ic = 0;
			}
		} else { 
			/* sending a normal packet */
			IF_DEQUEUE(&sc->is_if.if_snd, m);
			if(m == 0){
				return;
			}
			tlen = teput(ctlr,index,m);
			/* 
			 * first segment and last segment and 
			 * interrupt on completion 
			 */
			rp->te_info |=  TE_TFS | TE_TLS | TE_IC ;
		}
		rp->te_bsize = tlen;
		rp->te_own = TE_OWN;	
		sc->nxmit++;
		sc->is_if.if_flags |= IFF_OACTIVE;

		/*
		 * tell TGEC to look
		 */
		TRANS_POLL(ctlr);
		sc->is_if.if_timer = 5;			 
	} 
}

/*
 * teintr()
 *
 * Ethernet interface interrupt processor. This routine
 * is called each time the network interface receives an
 * interrupt. After identifying what kind of an interrupt
 * was received - transmit or receive - it calls the 
 * respective routine to process the interrupt.
 *
 * For DEC/4000, this routine is called each time the lint bit for an interface 
 * gets set.  The lint bit is edge-triggered from the TGEC status interrupt bits.
 * Foreach unit, the lint is cleared (if it was set), then the TGEC status 
 * register is checked and cleared.  This should prevent lost interrupts because 
 * the LINT is set by the TGEC. The appropriate action (error, transmit or 
 * receive processing) is taken based on the status.  
 *
 * The routine checks unit 0 repeatedly until no more interrupts are pending.
 * Then unit 0 is checked repeatedly until no more interrupts are pending.
 *
 * Arguments:
 *	none
 *
 * Return Value:
 * 	None.
 */
teintr()
{
	register unit;
	register struct te_softc *sc;
	register struct controller *ctlr;
	register int status;
	register int s;

	s = SPL_TE();
			
	for (unit=0; unit < nTE; unit++){
		sc = &te_softc[unit];
		/* pass over uninitialized controllers */
		if((ctlr = teinfo[unit]) == NULL)
			continue;
		if (LINT_SET(ctlr)) {
			do {
				CLEAR_LINT(ctlr);
				CLEAR_TGEC_STATUS(ctlr, status);
				if (status & TE_CSR5_IE) {
					/*
					 * big problems here, have to reset the
					 * TGECbefore continuing
					 */
					teeint(status, ctlr);
					goto done;
				}
				if (status & TE_CSR5_RU) {
					/* Receive Buffer Unavailable. When 
					 * set, indicates that the next 
					 * descriptor on the  receive list is 
					 * owned by the host,  
					 * ==> a packet was lost.
					 */
					temisscnt++;
					EPRINTF(("te%d: missed pkt ", unit));
					EPRINTF(("(Recv Buf Unav.)"));
					INCR(sc->ctrblk.est_sysbuf, 0xffff);
					/* 
					 * Don't need to poll the receive. The 
					 * TGEC will turn on the receive 
					 * running state when next frame 
					 * arrives.
					 */ 
				}
				if (status & TE_CSR5_RI)  terint(ctlr);
				if (status & TE_CSR5_TI)  tetint(ctlr);
				status = RD_TGEC_STATUS(ctlr);
			} while (status & 0x0000000f);
		}
		/* 
		 * if there are pending xmits, kick the TGEC 
		 */
		if (TRANS_PACKETS_PENDING(ctlr)){
			TRANS_POLL(ctlr);
			/*
			 * if this is set, the driver is entered recursively.  
			 * Doesn't make much sense -- kmbl
			 */
		}
	} 
      done:
	splx(s);
}

/*
 * teeint() - tgec error interrupt, interface interrupt due to an error 
 * condition, handle it here
 * 
 * arguments - 
 *	status - tgec status
 *	ctlr - controller with unit number
 * return value - none
 */
teeint(status, ctlr)
	register int status;
	struct controller *ctlr;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];

	if (status & TE_CSR5_TW) {
		tetbablcnt++;
		EPRINTF(("te%d:transmit timeout (TW) st: 0x%x\n",unit,status));
	} else if (status & TE_CSR5_ME) {
		/*
		 * Memory Error
		 * It happened when any one of following occurs: 
		 * 1) host memory problem
		 * 2) Parity error detected on an host to TGEC
		 *    CSR write or TGEC read from memory
		 * - should this be logged somewhere?? --kmbl
		 */
		temerrcnt++;
	}
	/* reset the chip */
	tereset(TE_UNIT_SC(ctlr));
}

/*
 * tetint()  Ethernet interface transmit interrupt. This deals with
 * a transmit interrupt. We get this after we have finished transmitting 
 * a packet.
 *
 * Either all the frames in the transmit list have been transmitted, or a frame
 * transmission was aborted due to a locally induced error.  Scan through 
 * descriptor list to figure it out.  Transmission is in the suspended state.
 *
 * A transmit interrupt could also be cause on a transmission completion.
 *
 * Arguments:
 *	ctlr		structure with interrupting controller
 *
 * Return Value:
 * 	None.
 */
tetint(ctlr)
	struct controller *ctlr;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register index = sc->otindex;
	register i;
	register TEDESC *rp= &sc->tring[sc->otindex];	/* ring pointer */
	register status;
	struct mbuf *mp,*mp0;
	struct ether_header *eh;
	int 	nTENXMT = sc->ntring;
	struct ifnet *ifp = &sc->is_if;
	int	ccount;
	short   tlen;
	int 	loop_check;
	int	ot, ti, own, nx;

	/*
	 * While we haven't caught up to current transmit index,
	 * AND the buffer is ours, AND there are still transmits
	 * which haven't been pulled off the ring yet, proceed
	 * around the ring in search of the last packet. 
	 */
	loop_check = 0;

	/* old trans ind != trans ind 
	 * and the host owns descriptor 
 	 * and transmits are in progress 
	 * kmbl ?? otindex != tindex, and nxmit==0 seem to be the same thing 
	 */
	while (((ot = sc->otindex) != (ti = sc->tindex)) &&
	       ((own = rp->te_own) != TE_OWN) && (nx = sc->nxmit) > 0) {
		loop_check = 1;
		status = rp->te_flag;
		tlen = rp->te_bsize; 

		/*
		 * Found last buffer in the packet (hence a valid string of 
		 * descriptors) so free things up.
		 */
		mp = sc->smbuf[index];
		sc->smbuf[index] = (struct mbuf *)NULL; 

		/* decrement the number of transmit packets pending */
		if(!(--sc->nxmit)) {
			/* deactivate if end of ring */
			ifp->if_timer = 0; 
			sc->is_if.if_flags &= ~IFF_OACTIVE;
		}

		if(!(rp->te_info & TE_DT_TSET)) { /* START- not setup packet */
			/* 
			 * Network statistics
			 */
			sc->is_if.if_opackets++;
			ccount = (status & TE_CC) >> 3;	
			sc->is_if.if_collisions += ccount;
			/*
			 * Decnet statistics
			 */
			if (ccount) { /* collision happened */
				if(ccount == 1) {
					INCR(sc->ctrblk.est_single,0xffffffff);
				} else  /* more than one collision */
					INCR(sc->ctrblk.est_multiple,0xffffffff);
			} 
			if(status & TE_DE) 
				INCR(sc->ctrblk.est_deferred,0xffffffff);
			
			if((status & TE_HF) && !(status & TE_UF)) 
				/* Heartbeat Fail */
				INCR(sc->ctrblk.est_collis, 0xffff);

			/*
			 * Check for transmission errors. This assumes that 
			 * transmission errors are always reported in the last 
			 * packet.
			 */
			if(status & TE_ES) {	/* transmission errors */
				tetint_errors(ctlr, status, rp);
				m_freem(mp);	/* dump the packet */

			} else {	/* no transmission errors */
				/*
			 	 * Accumulate statistics for DECnet
		 		 */
				if ((sc->ctrblk.est_bytesent + tlen) > 
				    sc->ctrblk.est_bytesent)
					sc->ctrblk.est_bytesent += tlen;
                		INCR(sc->ctrblk.est_bloksent,0xffffffff);
				if (mp) {
					eh = mtod(mp, struct ether_header *);
					m_freem(mp);
				}
			}
			/* END- not a setup packet */
		} else { /* START - setup frame */
			if (status & TE_SETUP_SE) {
				printf("te%d: setup frame buffer ", unit);
				printf("size error (should be 128 bytes)\n");
				teinit(unit);
			}	
		}
		/*
		 * Init the buffer descriptor
		 */
		teinitdesc(&sc->tring[index],sc->tbufs[index]);

		/*
		 * Increment old index pointer, if it catches up
		 * with current index pointer, we will break out
		 * of the loop.  Otherwise, go around again
		 * looking for next end-of-packet cycle.
		 */
		sc->otindex = index = ++index % nTENXMT;
		rp = (TEDESC *) &sc->tring[index];
	}	/* end while */
	/*
	 * If there are pending transmits - tell the TGEC to look
	 */
	if (sc->nxmit > 0 && (sc->tring[sc->otindex].te_own == TE_OWN)){
		TRANS_POLL(ctlr);
	}

	/*
	 *  Dequeue the next transmit request if the interface is not active
	 */
	if (!(ifp->if_flags & IFF_OACTIVE))
 		teoutput(ifp);

}

/*
 * tetint_errors() parse errors occuring on transmit interrupt
 *
 * Arguments:
 *	ctlr		structure with interrupting controller
 *
 * Return Value:
 * 	none
 */
tetint_errors(ctlr, status, rp)
	register struct controller *ctlr;
	register int status;
	register TEDESC *rp;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];

	sc->is_if.if_oerrors++;
	EPRINTF(("te%d: trans err %02x\n", unit, rp->te_flag & 0xff));
	if (sc->ctrblk.est_sendfail != 0xffff) {
		sc->ctrblk.est_sendfail++;
		if (status & TE_EC) {/* Excessive Collisions */
			EPRINTF(("te%d: ",unit));
			EPRINTF(("trans err excessive collision (EC)\n"));
			sc->ctrblk.est_sendfail_bm |= 1;
		}
		if (status & TE_LC) {
			EPRINTF(("te%d: ",unit));
			EPRINTF(("trans err late transmit collision (LC)\n"));
			sc->ctrblk.est_sendfail_bm |= 2; 
		}
		if (status & (TE_NC | TE_LO)) {
			/* no carrier */
			EPRINTF(("te%d: trans err",unit));
			EPRINTF((" no carrier or lost carrier (LO or NC)\n"));
			sc->ctrblk.est_sendfail_bm |= 4;
		}
		if (status & TE_UF) {/* Underflow error */
			EPRINTF(("te%d: trans err underflow error (UF)\n",unit));
		}	
		if (status & TE_TLE) {/* Length error */
			EPRINTF(("te%d: trans err",unit));
			EPRINTF((" transmit buffer length error (LE)\n"));
		}
	}
}


/*
 * terint() - Ethernet interface receiver interrupt.
 *
 * If can't determine length from type, then have to drop packet.  
 * Othewise decapsulate packet based on type and pass to type specific 
 * higher-level input routine.
 *
 * Arguments:
 *	ctlr		structure with interrupting controller
 *
 * Return Value:
 * 	None.
 */
terint(ctlr)
	struct controller *ctlr;
{
	register unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register TEDESC  *rp = &sc->rring[sc->rindex];	/* ring pointer */
	register int index;
	register struct ifnet *ifp = &sc->is_if;
	register int first;
	int	nTENRCV = sc->nrring;
	int	rlen;	

	/*
	 * Traverse the receive ring looking for packets to pass back.
	 * The search is complete when we find a descriptor not in use
	 * (which means owned by TGEC).
	 */
	for (index=sc->rindex; !(rp->te_own & TE_OWN); rp=&sc->rring[index]) {

		/*
		 * We don't need to check the chained packet.
		 * The TGEC will filter out those chained packet by
		 * setting the CSR6 DC bit on. The RDES0<FS,LS> will
		 * always be set
		 */
		if(!(rp->te_flag & TE_ES)) { /* valid descriptor (no errors) */
			rlen = rp->te_com & 0x00007fff ;
			teread(ctlr,rp,index,rlen);
		} else {	
			/* Else not a good packet, check for errors */
			sc->is_if.if_ierrors++;

			if(sc->ctrblk.est_recvfail != 0xffff) {
				terint_errors(rp, ctlr);
			} 

		}
		teinitdesc(rp,sc->rbufs[index]);
		rp->te_own = TE_OWN;
		sc->rindex = index =  ++index % nTENRCV;
	}
}

/*
 * terint_errors() 
 * parse errors occuring on tgec receive interrupts
 *
 * Arguments:
 *	rp		pointer to current ring descriptor
 *	ctlr		structure with interrupting controller
 *
 * Return Value:
 * 	None.
 */
terint_errors(rp,ctlr)
	register TEDESC  *rp;
	struct controller  *ctlr;
{		
	int	unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];

	sc->ctrblk.est_recvfail++;
	if(rp->te_flag & TE_OF) { /* overflow */
		sc->ctrblk.est_recvfail_bm |=4 ;
		EPRINTF(("te%d: recv err overflow\n",unit));
	}
	if(rp->te_flag & TE_CE) { /* CRC error */
		EPRINTF(("te%d: recv err crc error\n",unit));
		if(rp->te_flag & TE_DB) 
			/* Block check error */
			sc->ctrblk.est_recvfail_bm |=1 ;
		else
			/* Frame error */
			sc->ctrblk.est_recvfail_bm |=2 ;
	}
	if(rp->te_flag & TE_CS) { /* Collision seen */
		EPRINTF(("te%d: recv err collision seen\n",unit));
		sc->ctrblk.est_recvfail_bm |=2 ;
	}
	if(rp->te_flag & TE_RTL) { /* Frame too long */
		EPRINTF(("te%d: recv err frame too long\n",unit));
		sc->ctrblk.est_recvfail_bm |=4 ;
	}
	if((rp->te_flag & TE_RF) && !(rp->te_flag & TE_OF)) { 
		/* Runt Frame - will only receive if */
		/* CSRT6 <PB> is set. Meaningless if */
		/* RDES0<OF> is set	*/
		EPRINTF(("te%d: recv err runt frame\n",unit));
		sc->ctrblk.est_recvfail_bm |=4 ;
	}
	if(rp->te_flag & TE_RLE) { /* length error */
		EPRINTF(("te%d: recv err length error\n",unit));
	}
}

/* 
 * teread() 
 * Pass a packet to the higher levels. We deal with the trailer protocol
 *
 * Arguments
 *	ctlr		controller structure unit number
 *	dp		descriptor pointer
 *	index		index into rbufs
 *	rlen		length of packet
 */
teread(ctlr, dp, index, rlen)
	struct controller  *ctlr;
	register TEDESC *dp;
	int index,rlen;
{
	register int	unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register struct ether_header eh, *eptr = &eh;
	
	struct mbuf *m;
	int off,resid,len = rlen - 4;	/* CRC */
	vm_offset_t rbufp = sc->rbufs[index];
	
	/* Save the header so we can pass it to ether_input below. */
	eh = *(struct ether_header *)rbufp;
	
	/*
	 * Deal with trailer protocol: if type is INET trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
        eptr->ether_type = ntohs((u_short)eptr->ether_type);
	if ((eptr->ether_type >= ETHERTYPE_TRAIL &&
	     eptr->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER)) {
	    	off = (eptr->ether_type - ETHERTYPE_TRAIL) * 512 +
			sizeof(struct ether_header);
		if (off >= ETHERMTU)
			return;         /* sanity */
		eptr->ether_type = ntohs(*(short *)(rbufp + off));
		resid = ntohs(*(short *)(rbufp + off + 2));
		if (off + resid > m->m_len)
			return;            /* sanity */
	} else {
		off = 0;
	}
	
	/* Get an mbuf to copy the data into, from our special receive buffer.
	 * The -2 is needed since we'll be skipping over the first 2 bytes of 
	 * the mbuf (see below).
	 */
	if (len <= MHLEN - 2) {
		MGETHDR(m, M_DONTWAIT, MT_DATA);
	} else
		TEMCLGET(m, M_DONTWAIT);

	/*
	 * If got it then move the data 2 bytes beyond the start of the mbuf
	 * so that the protocol headers line-up on a longword (32-bit) boundary.
	 * This is an NFS requirement. Drop the packet if no mbuf is available.
	 */
	if (m) {
		m->m_data += 2;
		bcopy(rbufp, mtod(m, vm_offset_t), len);
		m->m_len = len;
	} else {
		len <= MHLEN ? tenosmbuf++ : tenolmbuf++;	
		return;
	}

	/*
	 * Pull packet off interface.  (In the case of TE, only need
	 * to allocate another mbuf to switch the old one) Off is nonzero 
	 * if packet has trailing header; qeget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	{
		struct mbuf *m0 = m;
		if (off) {
			int nbytes;
			int cnt = resid;
			struct mbuf *n;
			struct mbuf **mp = &m;
			
			while (cnt > 0) {
				MGET(n,M_DONTWAIT,MT_DATA);
				if (n == 0) {
					m_freem(m);
					tenosmbuf++;	
					return;
				}
				nbytes = MIN(MLEN,cnt);
				bcopy((mtod(m, caddr_t) + off),
				      mtod(n, caddr_t), nbytes);
				n->m_len = nbytes;
				off += nbytes;
				cnt -= nbytes;
				*mp = n;
				mp = &n->m_next;
			}
			*mp = m0;
			m0->m_len -= resid;
			/* strip out the trailer header */
			m->m_data += 2 * sizeof (u_short);
			m->m_len -= 2 * sizeof (u_short);
		}
		
		/*
		 * Trim off ethernet header
		 */
		m0->m_data += sizeof (struct ether_header);
		m0->m_len -= sizeof (struct ether_header);
	}
	
	/*
	 * Subtract length of header from len
	 */
	len -= sizeof (struct ether_header);
	
	/*
	 * Bump up DECnet counters. Input packets for "netstat" include
	 * ALL directed, multicast, and error inputs. For DECnet, only
	 * error-free input packets are counted.
	 */
	sc->is_if.if_ipackets++;
	sc->ctrblk.est_bytercvd += len ;
	INCR(sc->ctrblk.est_blokrcvd, 0xffffffff);
	
	if(eptr->ether_dhost[0] & 1) {
		sc->ctrblk.est_mbytercvd += len;
		INCR(sc->ctrblk.est_mblokrcvd, 0xffffffff);
	}
	/* Dispatch this packet */
	m->m_pkthdr.len = m->m_len;
	m->m_pkthdr.rcvif = &sc->is_if;
	ether_input(&(sc->is_ed), (struct ether_header *)eptr, m, (off != 0));
	
}

/* teioctl()
 * Process an ioctl request.   In doing so, the interface is
 * restarted and reinitialized.
 *
 * Arguments:
 *	ifp		Pointer to the ifnet structure
 *	cmd		The ioctl to be performed
 *	data		The corresponding data for the ioctl.
 *
 * Return Value:
 *	error status
 */
teioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	unsigned int cmd;
	caddr_t data;
{
	register struct te_softc *sc = &te_softc[ifp->if_unit];
	register struct controller *ctlr = teinfo[ifp->if_unit];
	register unit=ifp->if_unit;

	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	struct ctrreq *ctr = (struct ctrreq *)data;
	struct protosw *pr;
	struct ifaddr *ifa = (struct ifaddr *)data;
	int s, i, error = 0 , j = -1, delay, csr5;

	s = SPL_TE();
	switch (cmd) {

	case SIOCENABLBACK:
	case SIOCDISABLBACK:
		if (cmd == SIOCENABLBACK) { 
			printf("te%d: internal loopback enable requested\n", 
			       unit);
       		        ifp->if_flags |= IFF_LOOPBACK;
		} else {
			printf("te%d: internal loopback disable requested\n", 
				unit);
                	ifp->if_flags &= ~IFF_LOOPBACK;
		}
		if (ifp->if_flags & IFF_RUNNING) { 
			ISSUE_TGEC_COMMAND(ctlr, ~(TE_CSR6_SR | TE_CSR6_ST));
			csr5 = RD_TGEC_STATUS(ctlr);
			/*
			 * Polling CSR5 <TS> and <RS> until transmission state 
			 * and receive state are running
			 */
			for (delay = 10000; 
			     delay && (csr5 & (TE_TS_STP | TE_RS_STP));
			     delay--)
				csr5 = RD_TGEC_STATUS(ctlr);
			teinit(unit);
		}
		break;
 
	case SIOCRPHYSADDR:
		/*
		 * read default hardware address
		 */
		bcopy(sc->is_addr, ifd->current_pa, 6);
		bcopy(sc->is_dpaddr, ifd->default_pa, 6);
		break;
 
	case SIOCSPHYSADDR:
		bcopy(ifr->ifr_addr.sa_data, sc->is_addr, 6);
		pfilt_newaddress(sc->is_ed.ess_enetunit, sc->is_addr);

		for (i = 0; i < 6; i++)
			sc->te_setup[0].setup_char[i] = sc->is_addr[i];
		if (ifp->if_flags & IFF_RUNNING) {
			tesetup(ctlr,0);
			teoutput(ifp); 
		} else
			tesetup(ctlr,0);
		/* If an IP address has been configured then an ARP packet
		 * must be broadcast to tell all hosts which currently have
		 * our address in their ARP tables to update their information.
		 */
#ifdef INET
		if(((struct arpcom *)ifp)->ac_ipaddr.s_addr)
			rearpwhohas((struct arpcom *)ifp,
				  &((struct arpcom *)ifp)->ac_ipaddr);
#endif
		break;
	case SIOCDELMULTI:
	case SIOCADDMULTI:
		if (cmd == SIOCDELMULTI) {
			for (i = 0; i < NMULTI - 1 ; i++) {
				if (bcmp(&sc->multi[i],ifr->ifr_addr.sa_data,
					 MULTISIZE) == 0) {
					if (--sc->muse[i] == 0)
						bcopy(te_multi, &sc->multi[i],
						      MULTISIZE);
				}
			}
		} else {
			for (i = 0; i < NMULTI - 1 ; i++) {
				if (bcmp(&sc->multi[i],ifr->ifr_addr.sa_data,
					 MULTISIZE) == 0) {
					sc->muse[i]++;
					goto done;
				}
				if (bcmp(&sc->multi[i], te_multi,MULTISIZE) == 0)
					j = i;
			}
			/*
			 * j is initialized to -1; if j > 0, then
			 * represents the last valid unused location
			 * in the multicast table.
			 */
			if (j == -1) {
				printf("te%d: SIOCADDMULTI failed",ifp->if_unit);
				printf(",multicast list full: %d\n", NMULTI);
				error = ENOBUFS;
				goto done;
			}
			bcopy(ifr->ifr_addr.sa_data, &sc->multi[j], MULTISIZE);
			sc->muse[j]++;

		}
		for (i = 0; i < 6; i++)
			sc->te_setup[0].setup_char[i] = sc->is_addr[i];
		if (ifp->if_flags & IFF_RUNNING) {
			tesetup(ctlr,0);
			teoutput(ifp);
		} else
			tesetup(ctlr, 0);
		break;

	case SIOCRDCTRS:
	case SIOCRDZCTRS:

		ctr->ctr_ether = sc->ctrblk;
		ctr->ctr_type = CTR_ETHER;
		ctr->ctr_ether.est_seconds = 
			(time.tv_sec - sc->ztime) > 0xfffe ? 
				0xffff : (time.tv_sec - sc->ztime);
		if (cmd == SIOCRDZCTRS) {
			sc->ztime = time.tv_sec;
			bzero(&sc->ctrblk, sizeof(struct estat));
		}
		break;

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		teinit(unit);
		switch(ifa->ifa_addr->sa_family) {
#ifdef INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr = 
				IA_SIN(ifa)->sin_addr;
			break;
#endif

		default:
/*********
			if (pr = iffamily_to_proto(ifa->ifa_addr->sa_family)) {
				error = (*pr->pr_ifioctl)(ifp, cmd, data);
			} 
**************/
			break;
		}
		break;
#ifdef  IFF_PROMISC     /* IFF_ALLMULTI and NPACKETFILTER, as well */
	case SIOCSIFFLAGS:
		/*
		 * Run a setup packet in case things have changed;
		 * someone may be trying to turn on (or off) promiscuous
		 * mode, for example.
		 */
		if (ifp->if_flags & IFF_RUNNING) { 
			ifp->if_flags &= ~IFF_RUNNING;  
			teinit(unit);
		}
		break;
#endif IFF_PROMISC
	case SIOCSIPMTU:
		{
		u_short ifmtu;

		bcopy(ifr->ifr_data, (u_char *)(&ifmtu), sizeof(u_short));	

		if (ifmtu > ETHERMTU || ifmtu < IP_MINMTU)
			error = EINVAL;
		else
			ifp->if_mtu = ifmtu;
		}
		break;

	default:
		error = EINVAL;

	}

done:	splx(s);
	return (error);
}

/*
 * teput()
 * Put an mbuf chain into the appropriate transmit mbuf
 *
 * Arguments:
 *	ctlr		The controller structure for the interface.
 *	index		The index of the buffer.
 *	m		The mbuf chain to be copied to the RAM buffer.
 *
 * Return Value:
 *	len		The total length copied.
 */
teput(ctlr, index, m)
	struct controller  *ctlr;
	int index;
	register struct mbuf *m;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register caddr_t dp,bp;
	register int len = 0;
	struct mbuf *m0;

	m0 = m;
	bp = (caddr_t)sc->tbufs[index];
	while(m) {
		if (m->m_len != 0) { 
			dp = mtod(m,caddr_t);
			bcopy(dp,bp,(unsigned)m->m_len);
			bp += m->m_len;
			len += m->m_len;
		}
		m = m->m_next;
	}
	
	if(len < MINDATA) {
		bzero(bp,MINDATA - len);
		len = MINDATA;
	}

	/* 
	 * save the mbuf chain pointer
	 */
	sc->smbuf[index]=m0;

	return(len);
}

/*
 * tesetup()
 * fill out the setup packet - the physical address will 
 * already be presented in first row.
 *
 * Arguments
 *	ctlr	controller structure with unit number
 *	flag	indicates setup packet
 *
 * Return Value
 *	none
 */
tesetup(ctlr,flag)
	struct controller *ctlr;
	int flag;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register int i, j;

	/* 
	 * fill out the broadcast address to the rest of the entries 
	 * the first one will be the station address. The second one
         * will be all 1's 
	 */
	for(j=1; j <= 15; j++){
		bcopy(te_multi, &sc->te_setup[j], 6);
	}
	/* 
	 * If there have multicast addresses, just fill in the rest of
	 * setup frame. 
	 */
	for(i = 0, j = 2 ; i < NMULTI - 2 ; i++) {
		if(bcmp(&sc->multi[i], te_multi, 6) != 0) {
			bcopy(&sc->multi[i], &sc->te_setup[j], 6);	
			j++;
		}
	}
	sc->setupqueued++;
	if(flag)
		sc->te_setup_ic = 1;
}

/* 
 * teinitdesc()
 * Initialize a ring descriptor
 *
 * Arguments:
 *	descp - descriptor
 *	bfaddr - (virtual) address of xmt/rcv buffer.
 *
 * Return Value: None
 */
teinitdesc(descp,bfaddr) 
	register TEDESC *descp;
	register vm_offset_t bfaddr;
{
	bzero (descp, sizeof(TEDESC));	/* zero descriptor */
	
	descp->te_bsize = ETHERMTU + sizeof(struct ether_header) + 4; 
	descp->te_bfaddr = KSEG_TO_PHYS(bfaddr);
	descp->te_info = 0; 
	return;
}


/*
 * tereset()
 * Reset the TGEC chip, check the selftest, reallocate the mbufs 
 *
 * Argument
 *	sc	softc structure for the controller
 *
 * Return Value - none
 */
tereset(sc)
	struct te_softc *sc;
{
	register struct ifnet *ifp = &sc->is_if;
	register int unit = ifp->if_unit;
	register struct controller *ctlr = teinfo[unit];
	int i;

	teresets++;
	/*
	 * Stop network activity
	 */
	if (ifp->if_flags & IFF_RUNNING) 
		/* changed this to be like LANCE -- kmbl */
                ifp->if_flags &= ~IFF_RUNNING; 	
/*                ifp->if_flags &= ~(IFF_UP | IFF_RUNNING);  sgec version */

	tesoftreset(ctlr);
	tesetcsr0(ctlr);

	/*
	 * free up all the xmt mbufs 
	 */
	for (i=0; i < sc->ntring; i++) {
		if(sc->smbuf[i])
			m_freem(sc->smbuf[i]);
		sc->smbuf[i] = (struct mbuf *)NULL;
	}
	teinit(unit);
}	

/*
 * tesoftreset()
 * Reset the TGEC chip, check the selftest result.
 *
 * Argument
 *	ctlr	controller structure with unit number
 *
 * Return value
 *	0 	reset failure
 *	1	reset success
 */
tesoftreset(ctlr)
	struct controller  *ctlr;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];

	int delay;
	u_int csr5;

	/* 
	 * Reset the TGEC chip 
	 * Wait up to 100ms (only need 25ms) for complete the reset
	 * and initialization
	 */
	ISSUE_TGEC_COMMAND(ctlr, TE_CSR6_RE);
	
	csr5 = RD_TGEC_STATUS(ctlr);
	for(delay=100; !(csr5 | TE_CSR5_ID) || delay < 0 ; delay--) {
		DELAY(10000);
		csr5 = RD_TGEC_STATUS(ctlr);
	}
	
	if (!(csr5 | TE_CSR5_ID)){
		printf("te%d : cannot reset the TGEC chip \n",ctlr->ctlr_num);
		return(0);
	}

	if(!(csr5 & TE_CSR5_SF)) 
		return (1); /* selftest success */
	else if(csr5 & TE_SS_ROM) 
		printf("te%d : self test failed - ROM error\n", 
		       ctlr->ctlr_num); 
	else if(csr5 & TE_SS_RAM)
		printf("te%d : self test failed - RAM error\n", 
		       ctlr->ctlr_num);
	else if(csr5 & TE_SS_AFR)
		printf("te%d : self test failed - Address Filter RAM error\n",
		       ctlr->ctlr_num);
	else if(csr5 & TE_SS_TFF)
		printf("te%d : self test failed - Transmit FIFO error\n",
		       ctlr->ctlr_num);
	else if(csr5 & TE_SS_RFF)
		printf("te%d : self test failed - Receive FIFO error\n",
		       ctlr->ctlr_num);
	else if(csr5 & TE_SS_SLE)
		printf("te%d : self test failed - Lookback error\n",
		       ctlr->ctlr_num);

	return(0);
}

/* 
 * tesetcsr0() 
 * initialize the CSR0 with IPL, SA and IV.  See if_te_data.c
 *
 * Argument
 *	ctlr	controller structure with unit number
 *
 * Return Value
 *	0	failure
 *	~0	success
 */
tesetcsr0(ctlr)
	struct controller  *ctlr;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register int s, retry, csr0, csr5;

	s = SPL_TE();  /* raise the ipl to disable other interrupts */
	retry = 5;
	do {
		WRT_TGEC_CSR(ctlr, CSR0_SADDR, sc->te_initcsr);
		csr0 = RD_TGEC_CSR(ctlr, (sc->te_csrs | CSR0_SADDR));
		retry --;
	} while ((sc->te_initcsr != csr0) && (retry >= 0));
	splx(s);

	if(csr0 != sc->te_initcsr) { 
		printf(" te%d : failed to write CSR0\n", ctlr->ctlr_num);
		return(0);
	}
		
	/* check for the pending parity error interrupt */
	csr5 = RD_TGEC_CSR(ctlr, CSR5_SADDR );
	if (csr5 & TE_CSR5_ME)
		WRT_TGEC_CSR(ctlr, CSR5_SADDR, (TE_CSR5_IS | TE_CSR5_ME)) ; 
	return(~0);
}

/*
 * tewatch() 
 * If tetint hasn't been called in a while, restart chip, and reset timer.
 * 
 * Arguments:
 *	unit		The unit number to restart.
 *
 * Return Value:
 * 	None.
 */
tewatch(unit)
	int unit;
{
	register struct te_softc *sc = &te_softc[unit];
	register struct ifnet *ifp = &sc->is_if;
	register struct controller *ctlr = teinfo[unit];
	int s;
	int i;
	int csr5;

	s = SPL_TE();
	ifp->if_timer = 0;
	tereset(sc);
	splx(s);
}

/*
 * tescinit()
 * Fill in information into the softc based on the cpu we are using
 *
 * Arguments:
 *	ctlr		Input:  A controller structure to get the controller
 *				number of the adaptor.
 * Return Value:
 *	Success:	Size of the ln_softc structure.
 *	Failure:	NULL.
 */
tescinit(ctlr)
	struct controller  *ctlr;
{
	int	unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register struct tesw *tesw;     /* ptr to switch struct */
	extern int cpu;
	struct item_list switch_request;	

	switch_request.function = TEGC_SWITCH;
	switch_request.out_flags = IN_LINE;
	switch_request.in_flags = IN_LINE;
	switch_request.next_function = NULL;
	switch_request.input_data = unit;
	if (get_info(&switch_request))
		sc->tesw = tesw = (struct tesw *)switch_request.output_data;
	else {
		printf("teprobe : Unsupported system type\n", cpu);
		return(0);
	}
	/* set secondary address for csrs and narom */
	sc->te_csrs =  tesw->te_saddr_csr;
	sc->te_narom = tesw->te_saddr_narom;
	sc->te_lint = tesw->te_lint_mask;

	/* csr0 */
	sc->te_initcsr = (u_int)(tesw->te_ipl | tesw->te_vec | 
				 tesw->te_mode|TE_CSR0_MB1|TE_CSR0_SA);
	/* csr6 */
	sc->te_cmdcsr = tesw->te_burst << TE_BURST_SHIFT | 
		tesw->te_sigle | TE_CSR6_DC | TE_CSR6_HW; 
		
	sc->ntring = tesw->ttdesc;
	sc->nrring = tesw->trdesc;

	return(~0);
}

/*
 * teinitrings()
 * Allocate and initialize transmit and receive rings
 * Allocate and initialize transmit and receive buffers
 * Also allocate and initialize setup and diagnostic buffers
 *
 * Arguments
 *	ctlr		controller structure with unit number
 *
 * Return Value
 *	0		failure	
 *	~0		success
 */
teinitrings(ctlr)
	struct controller  *ctlr;
{
	int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register vm_offset_t bufptr = te_bufptr[unit];
	register u_int i;

	/*
	 * Make sure the buffer we obtained from steal_mem.c is as expected
	 */
	if ((bufptr == 0) || (bufptr & 0xf)) {
		printf("te%d: buffer space not allocated or aligned properly\n",
			unit);
		return(0);
	}

	/* 
	 * Start carving up our single buffer into individual pieces. Do the
	 * transmit and receive buffers first since they are a good multiple
	 * of 512-bytes.
	 */
	for (i=0; i<sc->ntring; i++,bufptr+=TE_BFSIZE) {
		sc->tbufs[i] = bufptr;
		sc->smbuf[i] = (struct mbuf*)NULL;
	}

	for (i=0; i<sc->nrring; i++,bufptr+=TE_BFSIZE)
		sc->rbufs[i] = bufptr;

	/* 
	 * Now the transmit and receive rings. These will be a multiple of
	 * octawords (and aligned as such) since each descriptor is an ow.
	 * We need 2 extra descriptors for chaining.
	 */
	sc->tring = (TEDESC *)bufptr;
	bufptr += (sizeof(TEDESC) * (sc->ntring + sc->nrring + 2));
	
	/* Zero out the ring-space since it's not guaranteed to be that way. */
	bzero(sc->tring, (sizeof(TEDESC) * (sc->ntring + sc->nrring + 2)));
	
	/* chain back the transmit ring */
	/* chain back the transmit ring */
	sc->tring_phys = KSEG_TO_PHYS((vm_offset_t)sc->tring);
	sc->tring[sc->ntring].te_bfaddr = (u_int) sc->tring_phys;
	sc->tring[sc->ntring].te_info = TE_CA; 
	sc->tring[sc->ntring].te_own = TE_OWN;

	/* transmit descriptors plus chaining descriptor */
	sc->rring = sc->tring + sc->ntring + 1;	

	/* chain back the receive ring */
	sc->rring_phys = KSEG_TO_PHYS((vm_offset_t)sc->rring);
	sc->rring[sc->nrring].te_bfaddr = (u_int) sc->rring_phys;
	sc->rring[sc->nrring].te_info = TE_CA; 
	sc->rring[sc->nrring].te_own = TE_OWN;

	/* 
	 * Buffer for the setup frame. This needs to be word-aligned, but
	 * we are doing much better than (octaword-aligned).
	 */
	sc->te_setup = (struct te_setup *)bufptr;
	bufptr += TE_SETUPSIZE;

	/* 
	 * And finally, the diagnostic buffer. Since the setup buffer was
	 * 128 bytes, we are once again, at least octaword-aligned here.
	 * Initialize the buffer with the expected pattern (see tgec_patch).
	 */
	sc->te_diagbf = (caddr_t)bufptr;
	bcopy(diag_buffer, sc->te_diagbf, TE_DIAGBFSIZE);
	bufptr += TE_DIAGBFSIZE;

	/* 
	 * Save pointer to remaining residual bytes for no good reason!
	 */
	te_bufptr[unit] = bufptr;
	return(~0);
}

/*
 * teinitmbufs()
 * Allocate mbuf pointers for transmit and receive buffer
 *
 *	tbufs : pointer to transmit data buffer 
 *	smbuf : mbuf chained point passed by higher network layer
 *	rbufs : pointer to the receive buffer
 *
 *	Arguments
 *		ctlr	controller structure with unit number
 *
 *	Return Value
 *		0		failure	
 *		~0		success
 */
teinitmbufs(ctlr)
	struct controller  *ctlr;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register vm_offset_t *smbuf_ptr;

	sc->tbufs = (vm_offset_t *)
		kmem_alloc(kernel_map,  
			   ((sizeof(struct mbuf *) * sc->ntring) +
			    (sizeof(vm_offset_t) * (sc->ntring + sc->nrring))));
	
	if(sc->tbufs == 0) {
		printf(" te%d: couldn't allocate memory for buffer pointer\n",
		       unit);
		return(0);
	}
	
	sc->rbufs = sc->tbufs + sc->ntring ;
	smbuf_ptr = sc->rbufs + sc->nrring ;
	sc->smbuf = (struct mbuf **)smbuf_ptr;
	
	return(~0);
}

/*
 * teinitmulticast()
 * initialize multicast address
 *	Argument
 *		ctlr	controller structure with unit number
 *
 *	Return Value - None
 */
teinitmulticast(ctlr)
	struct	controller *ctlr;
{
	register int unit;
	register struct te_softc *sc = &te_softc[unit];
	register int i;

	for (i=0; i<NMULTI; i++) {
		sc->muse[i] = 0;
		bcopy(te_multi, &sc->multi[i], MULTISIZE);
	}
}

/* 
 * tereadnarom()
 * read the station address from narom 
 *
 * Argument
 *	ctlr	controller structure with unit number
 *
 * Return Value - None
 */
tereadnarom(ctlr)
	struct	controller *ctlr;
{
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];

	register int i, j;
	void (*tmp)();
	extern void enet_addr_mbox_cmd();

	tmp = ((mbox_t)ctlr->ctlr_mbox)->mbox_cmd;
	((mbox_t)ctlr->ctlr_mbox)->mbox_cmd = enet_addr_mbox_cmd;

	for (i = 0, j = sc->te_narom; i < 6 ; i++, j +=4) {
		sc->te_setup[0].setup_char[i] = sc->is_addr[i] =
			RD_ETH_ADDR(ctlr, j);
	}
	bcopy(sc->is_addr, sc->is_dpaddr, 6);
	((mbox_t)ctlr->ctlr_mbox)->mbox_cmd = tmp;
}

/*
 * te_hw_rev()
 * returns the te hardware revision
 *
 * Argument
 *	ctlr	controller structure with unit number
 *
 * Return Value - hardware revision number
 */
te_hw_rev(ctlr)
	struct	controller *ctlr;
{	
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register int delay;
	register int csr5, rev;

	RD_TGEC_CSR_VIRT(ctlr, CSR10_SADDR, rev); 
	
	return((rev & TE_CSR10_HRN) >> TE_HW_RE_SHIFT) ;
}

/*
 * te_fw_rev()
 * returns the te firmware revision
 *
 * Argument
 *	ctlr	controller structure with unit number
 *
 * Return Value - hardware revision number
 */
te_fw_rev(ctlr)
	struct	controller *ctlr;
{	
	register int unit = ctlr->ctlr_num;
	register struct te_softc *sc = &te_softc[unit];
	register int delay;
	register int csr5, rev;

	RD_TGEC_CSR_VIRT(ctlr, CSR10_SADDR, rev); 
	
	return((rev & TE_CSR10_FRN) >> TE_FW_RE_SHIFT) ;
}

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
static char *rcsid = "@(#)$RCSfile: if_mfa.c,v $ $Revision: 1.1.14.9 $ (DEC) $Date: 1993/11/10 14:32:57 $";
#endif
#include "mfa.h"
#if NMFA > 0 || defined(BINARY)

/*
 * Revision History:
 *
 *      01/15/92 - Chris Beute
 *                 Ported if_mfa.c to Alpha/OSF
 *	12/01/90 - Matt Thomas
 *		   Created the if_mfa.c module
 */

#ifndef BINARY
#define BINARY
#endif
/*#include "packetfilter.h" */	/* NPACKETFILTER */
#include <data/if_mfa_data.c>

#define	sizeofarray(p)	(sizeof(p)/sizeof(p[0]))

extern struct protosw *iffamily_to_proto();
extern int ether_output();	
extern struct timeval time;
extern struct xmidata *get_xmi();
extern struct mbstat mbstat;
extern mfa_fddi_units;
extern mclbytes;


int	mfaattach(), mfaintr(), mfaoutput(),mfacuintr(), mfaeintr(), mfaprobe();
int	mfainit(),mfastart(),mfaioctl(),mfareset(), mfawatch();
struct	mbuf *mfamkparam(), *mfamkuser();

char *mfa_noringresources = "mfa%d: failed to allocate resources (%s)\n";

char *mfa_cmderrors[] = {
	"No Error",			"Buffer length error",
	"Wrong adapter state",		"Buffer transfer failed",
	"Invalid command",		"Invalid opcode",
	"Invalid loopback command", 	"Valid Command Failed",
};

char *mfa_unsolopcodes[] = {
	"Ring Init Initiated",
	"Ring Init Received",		"Ring Beacon Initiated",
	"Duplicate Address Test Fail",	"Duplicate Token Detected",
	"Ring Purger Error",		"Bridge Strip Error",
	"Ring Op Oscillation",		"Directed Beacon Received",
	"PC Trace Initiated",		"PC Trace Received",
};

char *mfa_unsolopcodes2[] = {
	"Transmit Underrun",		"Transmit Failure",
	"Receive Overrun",
};

char *mfa_unsolopcodes3[] = {
	"LEM Reject",			"EBUFF Error",
	"LCT Reject",
};

int mfadebug = 0;
int max_fddi_rcv = 0;
int max_xmit_seg = 0;

#define	RCV_INITENTRY	0x01
#define	RCV_GETBUFFER	0x02
#define RCV_INITESP	0x04

#define	MFATIMEOUT(xcmd, val) { \
	register int s = splnet(); \
	register long sec0 = time.tv_sec; \
	while (((xcmd)->opcode == (val)) && ((time.tv_sec - sec0) < 3)); \
	if ((xcmd)->opcode == (val)) \
		(xcmd)->opcode = CMD_NOP; \
	splx(s); \
}
#define MFACLGET(m) { \
        MGETHDR((m), M_DONTWAIT, MT_DATA); \
        if ((m)) { \
                MCLGET((m), M_DONTWAIT); \
                m->m_len = mclbytes; \
                if (!((m)->m_flags & M_EXT)) { \
                        m_freem((m)); \
                        (m) = (struct mbuf *)NULL; \
                } \
        } \
}
#define ADDQ(ctr, val) { \
        register u_long foo = 0; \
        foo = ((u_long)ctr.hi << 32) | ctr.lo; \
        foo += val; \
        ctr.lo = (u_int)(foo & 0xffffffff); \
        ctr.hi = (u_int)((foo >> 32) & 0xffffffff); \
}


/*
 * Probe the MFA; set up the register mappings with "reg" giving the
 * base address.
 */
mfaprobe(reg, ctlr)
caddr_t	reg;
struct	controller *ctlr;
{
	register struct mfa_softc *sc = &mfa_softc[ctlr->ctlr_num];
	struct mfadevice *addr = &sc->xregs;
	register mfapdb_t *xpdb;
	register totsiz, i, delay;
	struct xmidata *xmidata;
	struct mbuf *m;
        vm_offset_t vecaddr;
	int unit = ctlr->ctlr_num;
	caddr_t mp, dpa;
	u_long phy, phy1, phy2;
	int pd;
	int tmp_offset;

	/*
	 * Verify device type. Map register blocks and check for power-up 
	 * errors.
	 */
#ifdef MFADEBUG
	if (mfadebug)
		printf("In mfaprobe()\n");
#endif
	sc->ctlr = ctlr;
	addr->mfabase = reg;
	sc->dev_type = RDCSR(LONG_32, ctlr, reg);
	if ((sc->dev_type & XMIDTYPE_TYPE) != XMIDEMFA)
		return(0);
	sc->callno = sc->flags = 0;
	bzero(sc->xparams, sizeof(mfaparam_t));
	bzero(sc->xctrs, sizeof(mfactrblk_t));
	bzero(sc->xstat, sizeof(mfastatus_t));
	sc->t_req = sc->rtoken = sc->tvx = sc->lem = sc->ring_purger = 0xffffffff;
	/*  
 	 * set up addresses of the XMI registers  
 	 */
	addr->xber_xmi = reg + XMI_XBER;
	addr->xfadr_xmi = reg + XMI_XFADR;
	addr->xfaer_xmi = reg + XMI_XFAER;
	addr->xcomm_xmi = reg + XMI_XCOMM;
	/* 
 	 * set up addresses of the MFA registers 
 	 */
        addr->xpd1 = reg + MFA_XPD1;
        addr->xpd2 = reg + MFA_XPD2;
        addr->xpst = reg + MFA_XPST;
        addr->xpud = reg + MFA_XPUD;
        addr->xpci = reg + MFA_XPCI;
        addr->xpcs = reg + MFA_XPCS;
        addr->xmit_fl = reg + MFA_XMIT_FL;
        addr->rcv_fl = reg + MFA_RCV_FL;
        addr->cmd_fl = reg + MFA_CMD_FL;
        addr->unsol_fl = reg + MFA_UNSOL_FL;
        addr->xmit_hib_lo = reg + MFA_XMIT_HIB_LO;
        addr->xmit_hib_hi = reg + MFA_XMIT_HIB_HI;
        addr->rcv_hib_lo = reg + MFA_RCV_HIB_LO;
        addr->rcv_hib_hi = reg + MFA_RCV_HIB_HI;
        addr->eeprom_upd = reg + MFA_EEPROM_UPD;
	/*
	 * Wait up to 10 seconds for XPST_UNINIT state to appear.
	 */
	for (delay = 1000; delay && (RDCSR(LONG_32, ctlr, addr->xpst) & XPST_MASK) != XPST_UNINIT; delay--)
		DELAY(10000);
	if ((RDCSR(LONG_32, ctlr, addr->xber_xmi) & XMI_STF)
		 || (RDCSR(LONG_32, ctlr, addr->xpst) & XPST_MASK) != XPST_UNINIT) {
		mfaprintporterror(sc, "port self test failed (probe)");
		return(0);
	}
	/*
	 * We are now guaranteed that the port has succesfully completed
	 * self test and is in the "UNINITIALIZED" state. At this point,
	 * we can grab the default physical address from the port data
	 * registers.
	 */
	pd = RDCSR(LONG_32, ctlr, addr->xpd1);
	dpa = (caddr_t)&pd;
	for (i = 0; i < 4; i++)
		sc->is_dpaddr[i] = *dpa++;
	pd = RDCSR(LONG_32, ctlr, addr->xpd2);
	dpa = (caddr_t)&pd;
	for (; i < 6; i++)
		sc->is_dpaddr[i] = *dpa++;
	/*
	 * Device actual physical address not set yet
	 */
	bcopy(sc->is_dpaddr, sc->is_addr, 6);
	/* 
	 * We allocate 1 Alpha page (8kb or larger) to hold the rings and
	 * PDB. We'll then place the PDB and each of the rings into an aligned
	 * 512 byte region, based on the total size of the rings.
         */
	totsiz = sizeof(mfapdb_t)
		+ (max(512, (MFARCVMAX * sizeof(mfarcv_ent_t)) + 511) & ~511)
		+ (max(512, (MFACMDMAX * sizeof(mfacmd_ent_t)) + 511) & ~511)
		+ (max(512, (MFAXMTMAX * sizeof(mfaxmt_ent_t)) + 511) & ~511)
		+ (max(512, (MFAUNSOLMAX * sizeof(mfaunsol_ent_t)) + 511) & ~511);
	mp = (caddr_t)kmem_alloc(kernel_map, totsiz);
	if ((xpdb = sc->xpdb = (mfapdb_t*)mp) == 0) {
		printf(mfa_noringresources, unit, "PDB");
		return(0);
	}
#ifdef MFADEBUG3
	printf("Allocated pdb and ring memory size of %d bytes at 0x%x\n",
		totsiz, mp);
#endif
	mp += sizeof(mfapdb_t);
	if (mfaallocring(mp, &sc->rcv, MFARCVMAX, sizeof(mfarcv_ent_t), TRUE)) {
		printf(mfa_noringresources, unit, "receive ring");
		goto failed;
	}
	mp += max(512, (MFARCVMAX * sizeof(mfarcv_ent_t)) + 511) & ~511;
	if (mfaallocring(mp, &sc->xmt, MFAXMTMAX, sizeof(mfaxmt_ent_t), TRUE)) {
		printf(mfa_noringresources, unit, "transmit ring");
		goto failed;
	}
	mp += max(512, (MFAXMTMAX * sizeof(mfaxmt_ent_t)) + 511) & ~511;
	if (mfaallocring(mp, &sc->cmd, MFACMDMAX, sizeof(mfacmd_ent_t), TRUE)) {
		printf(mfa_noringresources, unit, "command ring");
		goto failed;
	}
	mp += max(512, (MFACMDMAX * sizeof(mfacmd_ent_t)) + 511) & ~511;
	if (mfaallocring(mp, &sc->unsol, MFAUNSOLMAX, sizeof(mfaunsol_ent_t), FALSE)) {
		printf(mfa_noringresources, unit, "unsolicited ring");
		goto failed;
	}
#ifdef MFADEBUG
	if (mfadebug)
		printf("mfa%d: ring VAs rcv=0x%x, xmt=0x%x,\n cmd=0x%x, unsol=0x%x\n", unit,
			sc->rcv.ring, sc->xmt.ring, sc->cmd.ring, sc->unsol.ring);
        if((svatophys(&sc->xctrs, &phy1)) == KERN_INVALID_ADDRESS)
                panic("mfa: invalid physical address!\n");
        if((svatophys(&sc->xstat, &phy2)) == KERN_INVALID_ADDRESS)
                panic("mfa: invalid physical address!\n");
	if (mfadebug)
		printf("mfa%d: Counters block PA = 0x%x, Status block PA = 0x%x\n",
			sc->unit, phy1, phy2);
#endif

	MFASETPHYADDR(&xpdb->cmd, sc->cmd.ring, 0, sc->cmd.size);
	MFASETPHYADDR(&xpdb->rcv, sc->rcv.ring, sizeof(mfarcv_ent_t), sc->rcv.size);
	MFASETPHYADDR(&xpdb->xmt, sc->xmt.ring, 0, sc->xmt.size);
	MFASETPHYADDR(&xpdb->unsol, sc->unsol.ring, 0, sc->unsol.size);
	MFASETPHYADDR(&xpdb->ubua, &sc->mfa_ubuacntr, 0, 0);
	xpdb->max_buf_size = FDDIMAX;
	xpdb->max_rcv_frame = FDDIMAX;   
	/* We need to allocate 3 interrupt vectors on a 64 byte boundary,
	 * to handle the normal, command/unsolocited, and status/error
	 * interrupts. We need to allocate 7 of these 16 byte reserved slots,
	 * then find the one on the proper boundary. Then we set up the
	 * interrupt vectors manually
	 */
        if(allocvec(7, &vecaddr) != KERN_SUCCESS) {
		printf(mfa_noringresources, unit, "interrupt vectors");
		goto failed;
	}
        tmp_offset = vecoffset(vecaddr);
        if(tmp_offset & 0x30) {
        /*
         * Offset is not 64 byte aligned so we modify vecaddr appropriately
         * Subsequent calls to vecoffset will then result in a 64 byte
         * aligned vector.
         */
                if(((tmp_offset + 0x10) & 0x30) == 0)
                        vecaddr += 0x10;
                else if(((tmp_offset + 0x20) & 0x30) == 0)
                        vecaddr += 0x20;
                else if(((tmp_offset + 0x30) & 0x30) == 0)
                        vecaddr += 0x30;
        }

	intrsetvec(vecoffset(vecaddr), mfaintr, ctlr);
	intrsetvec(vecoffset(vecaddr + 0x10), mfacuintr, ctlr);
	intrsetvec(vecoffset(vecaddr + 0x20), mfaeintr, ctlr);
	/*
	 * set up interrupt and error interrupt vectors
	 */
	xmidata = get_xmi(ctlr->bus_num);
	xpdb->intvec.level = 2;
	xpdb->intvec.nid_mask = xmidata->xmiintr_dst;
	xpdb->intvec.vector = (vecoffset(vecaddr) & 0xffc0) >> 4;
	return(sizeof(struct mfa_softc));

failed:
	mfafreering(&sc->rcv, MFARCVMAX, sizeof(mfarcv_ent_t));
	mfafreering(&sc->cmd, MFACMDMAX, sizeof(mfacmd_ent_t));
	mfafreering(&sc->xmt, MFAXMTMAX, sizeof(mfaxmt_ent_t));
	mfafreering(&sc->unsol, MFAUNSOLMAX, sizeof(mfaunsol_ent_t));
	kmem_free(kernel_map, sc->xpdb, totsiz);
	return(0);
}

mfaallocring(mp, ring, entries, entry_size, need_mbufs)
caddr_t mp;
register mfaring_t *ring;
int entries;
int entry_size;
int need_mbufs;
{
	caddr_t kmp;
	u_long phy;
	/*
	 * allocate data structures, fill out port data block, and init device.
	 */
	if (need_mbufs) {
		kmp = (caddr_t)kalloc(entries * sizeof(struct mbuf *));
		if ((ring->mbufs = (struct mbuf **)kmp) == 0)
			return(ENOBUFS);
		bzero(ring->mbufs, entries * sizeof(struct mbuf *));
	}
	/*
	 * ring->next	= index of next desc. to activate
	 * ring->last	= index of last desc. processed
	 * ring->active  = # of desc. pending
	 * ring->max	 = max # of desc. that can be queued
	 * ring->modulo  = wrap number of ring
	 * ring->size	= size of ring
	 */
	ring->ring = mp;
        if((svatophys(ring->ring, &phy)) == KERN_INVALID_ADDRESS)
                panic("mfa: invalid physical address!\n");
	ring->pa_ring = (caddr_t)phy;
	ring->nextin = ring->ring;
	ring->nextout = ring->ring;
	ring->lastin = ring->ring;
	ring->goal = 0;
	ring->active = 0;
	ring->max = entries;
	ring->size = entries * entry_size;
	ring->limit = ring->ring + ring->size;
	return(0);
}

mfafreering(ring, entries, entry_size)
mfaring_t *ring;
int entries;
int entry_size;
{
	if (ring->mbufs)
		kfree(ring->mbufs, entries * sizeof(struct mbuf *));	
	bzero(&ring, sizeof(*ring));
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
mfaattach(ctlr)
struct controller *ctlr;
{
	register struct mfa_softc *sc = &mfa_softc[ctlr->ctlr_num];
	register struct ifnet *ifp = &sc->is_if;
	struct mfadevice *addr = &sc->xregs;
	register struct sockaddr_in *sin;
	register int i;

#ifdef MFADEBUG
	if(mfadebug)
		printf("In mfaattach()\n");	
#endif
        sc->is_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
        sc->is_ac.ac_arphrd = ARPHRD_ETHER;     /* Ethernet addr  */
	ifp->if_unit = sc->unit = ctlr->ctlr_num;
	ifp->if_name = "mfa";
/*	ifp->lk_softc = &sc->lk_mfa_softc; */
	ifp->if_mtu = FDDIMTU;
	ifp->if_mediamtu = FDDIMAX;
	ifp->if_type = IFT_FDDI;
	ifp->if_flags |= IFF_SNAP|IFF_BROADCAST|IFF_MULTICAST|IFF_NOTRAILERS|IFF_SIMPLEX;
	ifp->if_addrlen = 6;
	ifp->if_hdrlen = sizeof(struct fddi_header) + 8; /* LLC header is 8 octets */
	((struct arpcom *)ifp)->ac_ipaddr.s_addr = 0;

	/*
	 * Initialize if_version string.
	 */
	ifp->if_version = "DEC DEMFA FDDI Interface";
	/*
	 * Initialize multicast address table
	 * Reserve first slot to listen for broadcasts
	 */
	for (i = 0; i < NMULTI; i++) {
		bcopy(etherbroadcastaddr, &sc->is_multi[i], 6);
		sc->is_muse[i] = 0;
	}
	sc->is_muse[0]++; 
	/*
	 * Set maxlen for the command queue; attach interface
	 */
/*	lockinit(&sc->lk_mfa_softc, &lock_device15_d); */
	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;
	ifp->if_init = mfainit;
	ifp->if_output = ether_output;
	ifp->if_start = mfaoutput;
	ifp->if_ioctl = mfaioctl;
	ifp->if_reset = mfareset;
	ifp->if_watchdog = mfawatch;
	ifp->if_timer = 1;
/*  	ifp->d_affinity = ALLCPU; */
	ifp->if_baudrate = FDDI_BANDWIDTH_100MB;
	printf("mfa%d: %s, hardware address %s\n", sc->unit,
		ifp->if_version,
		ether_sprintf(sc->is_dpaddr));
	attachpfilter(&(sc->is_ed));
	if_attach(ifp);
	mfa_fddi_units++;	/* Increment number of live FDDI adapters */
}

/*
 * Perform a node halt/reset (SOFT reset) due to state change interrupt.
 * Issue a timeout() on mfainit, but give up if we're being reset
 * constantly. mfainit() will set IFF_RUNNING if the reset succedes.
 */
mfareset(unit)
int unit;
{
	register struct mfa_softc *sc = &mfa_softc[unit];
	struct mfadevice *addr = &sc->xregs;
	register struct ifnet *ifp = &sc->is_if;
	mfapdb_t *xpdb = sc->xpdb;
	mfacmd_buf_t *xcmd;
	struct mbuf *m;
	int i, xbe, delay, mbfree, ifflags, s;
	u_long phy;

	s = splimp();
#ifdef MFADEBUG
	if(mfadebug)
		printf("In mfareset\n");
#endif
	/*
	 *  We assume the port block has been correctly set up at this point
	 *  and we are about to go from UNINIT to INIT.
	 */
	RING_RESET(&sc->rcv, mfarcv_ent_t, next->owner = 1, (m_freem(ringm), 1));
	RING_RESET(&sc->xmt, mfaxmt_ent_t, next->owner = 1, (m_freem(ringm), 1));
	RING_RESET(&sc->cmd, mfacmd_ent_t, next->owner = 1,
		(mtod(ringm, mfacmd_buf_t *)->opcode = CMD_INVAL, 0));
	RING_RESET(&sc->unsol, mfaunsol_ent_t, next->owner = 1, 0);

	while (RING_FREESPACE(&sc->unsol) > 0) {
		register mfaunsol_ent_t *nextout = RING_NEXTOUT(&sc->unsol, mfaunsol_ent_t);
		nextout->owner = 0;
		RING_ADV_NEXTOUT(&sc->unsol, mfaunsol_ent_t);
	}
	MFAFLUSH();

	while (m = ifp->if_snd.ifq_head) {
		IF_DEQUEUE(&ifp->if_snd, m);
		m_freem(m);
	}
	/*
	 * If the interface is halted, perform a node reset.
	 */
	if ((RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK) == XPST_HALT) {
		xbe = RDCSR(LONG_32, sc->ctlr, addr->xber_xmi);
		xbe |= XMI_NRST;
		WRTCSR(LONG_32, sc->ctlr, addr->xber_xmi, xbe);			/* reset mode */
		MFAFLUSH();
		splnet();
		for (delay = 1000; delay && (RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK) != XPST_UNINIT; delay--)
			DELAY(10000);
		splimp();
		if ((RDCSR(LONG_32, sc->ctlr, addr->xber_xmi) & XMI_STF) 
			|| (RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK) != XPST_UNINIT) {
			mfaprintporterror(sc, "port reset failed (reset)");
			splx(s);
			return(1);
		} else {
#ifdef MFADEBUG
		i = RDCSR(LONG_32, sc->ctlr, addr->xpst);
		printf("xpst status after NRST = 0x%x\n", i);
		i = RDCSR(LONG_32, sc->ctlr, addr->xber_xmi);
		printf("xber status after NRST = 0x%x\n", i);
#endif
		MFAFLUSH();
		}
	}

	/* Enable the XMI MORE protocol and HexaWord writes. */
	WRTCSR(LONG_32, sc->ctlr, addr->xber_xmi, (XMI_MORE|XMI_EHWW));
	MFAFLUSH();

	/*
	 * For init we are supposed to provide 2 receive buffers and one transmit.
	 */
	if (mfarcvdesc(sc, RCV_INITESP, NULL) != 0
		|| mfarcvdesc(sc, RCV_INITESP, NULL) != 0) {
		printf(mfa_noringresources, unit, "ESP rcv buffers");
		splx(s);
		return(FALSE);
	}
	MFACLGET(m);
	if (!m) {
		printf(mfa_noringresources, unit, "ESP xmit buffers");
		splx(s);
		return(FALSE);
	}
	m->m_len = 512;
	IF_ENQUEUE(&ifp->if_snd, m);
	MFACLGET(m);
	if (!m) {
		printf(mfa_noringresources, unit, "ESP xmit buffers");
		splx(s);
		return(FALSE);
	}
	m->m_len = 512;
	IF_ENQUEUE(&ifp->if_snd, m);
	mfastart(unit);

	/* 
	 * Now write the PDB address to the device registers before we try
	 * to initialize the device
         */
        if((svatophys(xpdb, &phy)) == KERN_INVALID_ADDRESS)
                panic("mfa: invalid physical address!\n");
	WRTCSR(LONG_32, sc->ctlr, addr->xpd1, phy & 0xffffffff);
	WRTCSR(LONG_32, sc->ctlr, addr->xpd2, (phy >> 32) & 0xffffffff);
	MFAFLUSH();
#ifdef MFADEBUG3
	printf("mfa%d: PDB VA=0x%x, PA=0x%x, size=%d (0x%x)\n", unit, xpdb,
		phy, sizeof(mfapdb_t), sizeof(mfapdb_t));
	DELAY(1000000);
#endif
	/*
	 * Hit device init, and wait for XPST_INIT state to appear
	 * (up to two seconds).
	 */
	WRTCSR(LONG_32, sc->ctlr, addr->xpci, XPCI_INIT);
	MFAFLUSH();
	splnet();
	i = RDCSR(LONG_32, sc->ctlr, addr->xpst);
	for (delay = 200; delay && ((i & XPST_MASK) == XPST_UNINIT); delay--){
		DELAY(100000);
		i = RDCSR(LONG_32, sc->ctlr, addr->xpst);
	}
	splimp();
	RING_RESET(&sc->rcv, mfarcv_ent_t, next->owner = 1, (m_freem(ringm), 1));
	RING_RESET(&sc->xmt, mfaxmt_ent_t, next->owner = 1, (m_freem(ringm), 1));
	while (m = ifp->if_snd.ifq_head) {
		IF_DEQUEUE(&ifp->if_snd, m);
		m_freem(m);
	}
	if ((RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK) != XPST_INIT) {
		mfaprintporterror(sc, "port init failed (reset)");
#ifdef MFADEBUG
		halt();
#endif
		splx(s);
		return(FALSE);
	}

	/*
	 * Queue up some receive buffers. Since cluster mbuf allocation can't
	 * be done in an interrupt thread, we avoid taking all available 
	 * cluster mbufs, leaving two for the PARAM and USER commands. We
	 * count on the watchdog routine to bring us up to rcv.goal later.
	 */
	sc->rcv.goal = MFARCVMIN; 
	mbfree = mbstat.m_clfree - 2;
	if (mbfree > MFARCVMIN)
		mbfree = MFARCVMIN;
	if (mbfree < 2) {
		splx(s);
		return(FALSE);
	}
	for (i = RING_ACTIVE(&sc->rcv); i < mbfree; i++) {
		if (mfarcvdesc(sc, RCV_INITENTRY, NULL) != 0)
			break;
	}
	sc->omfarcv_hib_lo =  sc->omfaxmit_hib_lo = 0;
	/*
	 * Schedule an "mfainit" at SPLNET if device was running.
	 */
	if (ifp->if_flags & (IFF_RUNNING|IFF_UP)) {
		ifp->if_flags &= ~IFF_RUNNING;
		untimeout(mfainit, unit);
		timeout(mfainit, unit, 1);
	}
	splx(s);
	return(TRUE);
}

/*
 * Initialize interface. May be called by a user process or a software
 * interrupt scheduled by mfareset().
 */
mfainit(unit)
int unit;
{
	register struct mfa_softc *sc = &mfa_softc[unit];
	struct mfadevice *addr = &sc->xregs;
	register struct ifnet *ifp = &sc->is_if;
	register mfacmd_buf_t *xcmd;
	register struct mbuf *m;
	register int delay;
	int s, t, state;

#ifdef MFADEBUG
	if (mfadebug)
		printf("In mfainit\n");
#endif
	/* not yet, if address still unknown */

	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;
	if (ifp->if_flags & IFF_RUNNING)
		return;

	/*
	 * Lock the softc to ensure coherency of softc data needed to fill
	 * out the command buffers by "mfamkparam()" and "mfamkuser()", and
	 * to lock access to the if_cmd queue by "mfacmd()". (mfaintr writes
	 * command completion info. into the first longword of the command
	 * buffer; MFATIMEOUT flags command timeouts. Need the splnet() if
	 * we're called from softclock() by mfareset.
	 */
	s = splnet(); t = splimp();
/*	smp_lock(&sc->lk_mfa_softc, LK_RETRY); */

	state = (RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK);
	if (state != XPST_RUNNING && state != XPST_MAINTENANCE && state != XPST_INIT) {
		if (state == XPST_UNINIT || state == XPST_HALT)
			mfareset(unit);
		state = (RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK);
		if (state != XPST_INIT) {
			ifp->if_flags &= ~IFF_UP;
			return;
		}
	}

	/*
	 * Issue a "PARAM" command to fill in actual physical address (from
	 * sc->is_addr) and other system parameters. Wait for completion,
	 * which will be signaled by the interrupt handler.
	 */
	if (state == XPST_INIT) {
		if (m = mfamkparam(sc)) {
			mfacmd(sc, m);
	/*		smp_unlock(&sc->lk_mfa_softc); */
			splx(t);
			xcmd = mtod(m, mfacmd_buf_t *);
			MFATIMEOUT(xcmd, CMD_PARAM);	/* Wait */
			t = splimp();
	/*		smp_lock(&sc->lk_mfa_softc, LK_RETRY); */
			switch (xcmd->opcode) {
			case CMD_COMPLETE:
				/*
			 	* Save the results in the softc fields
			 	*/
				xcmd = mtod(m, mfacmd_buf_t *);
				bcopy(xcmd->mfaparam.version, sc->xparams,
					sizeof(mfaparam_t));
				m_freem(m);
				break;
			case CMD_INVAL:
				m_freem(m);
			case CMD_NOP:
			default:
				/*
			 	* Don't free m if adapter appears to be hung
			 	*/
				printf ("mfa%d: port init (param) failed\n", unit);
				goto done;
			}
		} else {
			printf ("mfa%d: port init (param) failed\n", unit);
			goto done;
		}
	}
	/*
	 * Issue a "USTART" command for the Ethernet user. (Will initialize
	 * all multicast addresses here.)
	 */
	if (m = mfamkuser(sc, MFA_UNKNOWNU, CMD_USTART)) {
		mfacmd(sc, m);
/*		smp_unlock(&sc->lk_mfa_softc); */
		splx(t);
		xcmd = mtod(m, mfacmd_buf_t *);
		MFATIMEOUT(xcmd, CMD_USTART);	/* Wait */
		t = splimp();
/*		smp_lock(&sc->lk_mfa_softc, LK_RETRY); */
		switch (xcmd->opcode) {
			case CMD_COMPLETE:
			m_freem(m);
			break;
			case CMD_INVAL:
			m_freem(m);
			case CMD_NOP:
			default:
			/*
			 * Don't free m if adapter appears to be hung
			 */
			printf ("mfa%d: port init (ustart) failed\n", unit);
			goto done;
		}
	} else {
		printf ("mfa%d: port init (ustart) failed\n", unit);
		goto done;
	}

	/*
	 * Clear reset, mark interface up and running; start output
	 * on device.
	 */
	sc->flags &= ~MFA_RFLAG;
	ifp->if_flags |= (IFF_RUNNING | IFF_UP);
	ifp->if_flags &= ~IFF_OACTIVE;
	if (sc->ztime == 0)
		sc->ztime = time.tv_sec;
	/*
	 * Tell the device it has unsolicited and receive entries 
	 */
	WRTCSR(LONG_32, sc->ctlr, addr->unsol_fl, XPCP_POLL);
	WRTCSR(LONG_32, sc->ctlr, addr->rcv_fl, XPCP_POLL);
	MFAFLUSH();
	if (ifp->if_snd.ifq_head)
		mfastart(unit);			/* queue output packets */
done:
	/*
	 * Relinquish softc lock, drop IPL.
	 */
/*	smp_unlock(&sc->lk_mfa_softc); */
	splx(t); splx(s);
	return;
}

/*
 * MFA command start routine. Wraps a command buffer in an mbuf (for queuing
 * purposes ONLY), then enqueues the request to mfastart(). Command start
 * routine is called at splimp() WITH the softc locked.
 */
mfacmd(sc, m)
register struct mfa_softc *sc;
register struct mbuf *m;
{
	register mfacmd_buf_t *xcmd = mtod(m, mfacmd_buf_t *);
	register mfacmd_ent_t *nextout;
	struct mfadevice *addr = &sc->xregs;
	u_long phyaddr;

	if (RING_FREESPACE(&sc->cmd) == 0) {
	/*
	 * Couldn't enqueue command request; 
	 * invalidate opcode to signal error.
	 */
		xcmd->opcode = CMD_INVAL;
		return(EBUSY);
	}

	nextout = RING_NEXTOUT(&sc->cmd, mfacmd_ent_t);
	nextout->length = m->m_len;
	nextout->uindex = 0;
	if (xcmd->opcode == CMD_USTART || xcmd->opcode == CMD_UCHANGE
		|| xcmd->opcode == CMD_STATUS || xcmd->opcode == CMD_USTOP) 
		nextout->uindex = MFA_UNKNOWNU;
        if((svatophys(mtod(m, u_long), &phyaddr)) == KERN_INVALID_ADDRESS)
                panic("mfa: invalid physical address!\n");
	nextout->pa_lo = phyaddr & 0x1ff;
	nextout->pa_hi = phyaddr >> 9;
	nextout->ers = 0;
	nextout->owner = 0;
	RING_SETMBUF(&sc->cmd, nextout, mfacmd_ent_t, m);
	RING_ADV_NEXTOUT(&sc->cmd, mfacmd_ent_t);
	WRTCSR(LONG_32, sc->ctlr, addr->cmd_fl, XPCP_POLL);
	MFAFLUSH();
	return(0);
}

/*
 * MFA output routine, just calls existing start routine
 */
mfaoutput(ifp)
register struct ifnet *ifp;
{
	mfastart(ifp->if_unit);
}

/*
 * MFA start routine. Strings output packets onto the command
 * ring. Start routine is called at splimp() WITH the softc locked.
 */
mfastart(unit)
int unit;
{
	register struct mbuf *m;
	register mfaxmt_ent_t *eop;
	mfaxmt_ent_t *sop;
	struct mfa_softc *sc = &mfa_softc[unit];
	struct mfadevice *addr = &sc->xregs;
	register struct ifnet *ifp = &sc->is_if;
	struct mbuf *n, *m0;
	int numpkts, pktlen, new_xmit = 0;

	/*
	 * Process the transmits.  Done when we either run out of requests or 
	 * ring entries to string them on.
	 */
#ifdef MFADEBUG3
	printf("In mfastart()\n");
#endif
	while ((m = ifp->if_snd.ifq_head) != NULL) {
		int freespace = RING_FREESPACE(&sc->xmt);
#ifdef MFADEBUG
		if (mfadebug > 1)
			printf("mfastart: free=%d/%d, ifq_head=0x%x, len=%d, next=0x%x\n",
				freespace, sc->xmt.max,
				ifp->if_snd.ifq_head,
				ifp->if_snd.ifq_head->m_len,
				ifp->if_snd.ifq_head->m_next);
#endif

		if (freespace == 0) {
			ifp->if_flags |= IFF_OACTIVE;
			break;
		}
/*
 * Count the number of entries needed for this record. If we exhaust
 * available ring space, bail out. Also, check to see that we aren't
 * trying to place a zero length entry onto the transmit ring. This
 * causes the DEMFA to die a horrible, non-restartable death.
 */
		numpkts = pktlen = 0;
		for (; m; m = m->m_next) {
			register u_long addr;
			while ((m->m_next) && (m->m_next->m_len == 0)) {
				n = m->m_next;
				m->m_next = n->m_next;
				n->m_next = 0L;
				m_freem(n);
			}
			pktlen += m->m_len;
			numpkts++;
			freespace--;

			/*
			 * If there's a page-crossing in this mbuf, then we'll
			 * be using an extra entry; so account for that.
			 */
			addr = mtod(m, u_long);
                        if ((addr & (MCLBYTES - 1)) + m->m_len > MCLBYTES) {
                                numpkts++;
                                freespace--;
                        }
		}
		if (numpkts > max_xmit_seg) {
			max_xmit_seg = numpkts;
#ifdef MFADEBUG
			printf("FDDI max xmit segments = %d\n", max_xmit_seg);
#endif
		}
		if (freespace < 0) {
			ifp->if_flags |= IFF_OACTIVE;
			break;
		}
	/* 
	 * Extract the mbuf and sanity check it before we try to send it
	 */
		IF_DEQUEUE(&ifp->if_snd, m);
		if (!(m->m_flags & M_PKTHDR)) {
			/* 
			 * drop this packet for not having a packet header
			 */
#ifdef MFADEBUG
			if (mfadebug)
				printf("mfa%d: dropping mbuf without packet header 0x%x\n",
				unit, m);
#endif
			m_freem(m);
			continue;
		}
		if (pktlen > FDDIMAX) {
			/* 
			 * drop this packet for being too large
			 */
#ifdef MFADEBUG
			if (mfadebug)
				printf("mfa%d: dropping mbuf, invalid length 0x%x\n",
				unit, m);
#endif
			m_freem(m);
			continue;
		}
		sop = RING_NEXTOUT(&sc->xmt, mfaxmt_ent_t);
#ifdef MFADEBUG4
	printf("mfastart: about to queue mbuf %x to xmit sop %x\n", m, sop);
	mfadumpmbuf(m);
#endif
		for (m0 = m; m; m = m->m_next) {
			register int len = m->m_len;
			register u_long addr = mtod(m, u_long);
			u_long phyaddr;
			while (len > 0) {
				u_long clsize = MCLBYTES - (((u_long) addr) & (MCLBYTES - 1));
				int slen = min(len, clsize);
				eop = RING_NEXTOUT(&sc->xmt, mfaxmt_ent_t);
        			if(svatophys(addr, &phyaddr) == KERN_INVALID_ADDRESS)
                			panic("mfa: invalid physical address!\n");
				eop->ers = 0;
				eop->pa_lo = phyaddr & 0x1ff;
				eop->pa_hi = phyaddr >> 9;
				eop->length = slen;
				eop->sop = eop->eop = FALSE;
				if (eop != sop)
					eop->owner = FALSE;
				len -= slen;
				addr += slen;
				RING_ADV_NEXTOUT(&sc->xmt, mfaxmt_ent_t);
			}
		}
		RING_SETMBUF(&sc->xmt, eop, mfaxmt_ent_t, m0);
		eop->eop = TRUE;
		sop->sop = TRUE;
		sop->owner = FALSE;
		new_xmit++;
#ifdef MFADEBUG4
	printf("mfastart: xmit ring %x (%s%s%s) pa_hi = %x, pa_lo = %x len = %d\n", 
			sop, (sop->owner ? "O" : ""),
			(sop->sop ? "S" : ""), (sop->eop ? "E" : ""),
			sop->pa_hi, sop->pa_lo, sop->length);
	if (eop != sop)
		printf("mfastart: xmit ring %x (%s%s%s) pa_hi = %x, pa_lo = %x len = %d\n", 
			eop, (eop->owner ? "O" : ""),
			(eop->sop ? "S" : ""), (eop->eop ? "E" : ""),
			eop->pa_hi, eop->pa_lo, eop->length);
#endif
		MFAFLUSH();
	}
	/*
	 * Don't tickle the polling bit if we've just put xmits on for the
	 * ESP test, self-test will fail.
	 */
	if (sc->flags & MFA_RFLAG)
		return;
	/*
	 * Advise the port that a new transmit packet is pending
	 */
	if ((ifp->if_flags & IFF_RUNNING) && new_xmit) {
#ifdef MFADEBUG3
		printf("mfastart: added %d new xmits, polling device\n", new_xmit);
#endif
		WRTCSR(LONG_32, sc->ctlr, addr->xmit_fl, XPCP_POLL);
		MFAFLUSH();
	}
}

/*
 * MFA device interrupt handler
 */
mfaintr(ctlr)
struct controller *ctlr;
{
	int unit = ctlr->ctlr_num;
	register struct mfa_softc *sc = &mfa_softc[unit];
	struct mfadevice *addr = &sc->xregs;
	register struct ifnet *ifp = &sc->is_if;
	register mfaring_t *curring;
	register struct fddi_header *fptr;
	struct fddi_header fh;
	struct mbuf *m;
	u_long pa;
	int s = splimp();
	int i, xmits, rcvs;

#ifdef MFADEBUG3
	printf("In mfaintr, unit = %x\n", unit);
#endif
	/*
	 * Lock softc, since we will be updating the per-unit ring pointers
	 * and active ring entry counts frequently.
	 */
/*	smp_lock(&sc->lk_mfa_softc, LK_RETRY); */

	/*
	 * Process all incoming packets on the receive ring. Stop if
	 * we get to the current receive index to avoid locking out
	 * the system, but give back one descriptor for each one we
	 * process to keep the device busy.
	 */
	for (rcvs = 0, curring = &sc->rcv; !RING_EMPTY(curring);) {
		if (mfarcvdesc(sc, RCV_GETBUFFER, &m) == EWOULDBLOCK)
			break;
		ifp->if_ipackets++;
		rcvs++;
		if (m != NULL) {
			if (ifp->if_flags & IFF_RUNNING) {
				fptr = mtod(m, struct fddi_header *);
				m->m_len -= sizeof (struct fddi_header);
				m->m_data += sizeof (struct fddi_header);
                                ADDQ(sc->xpdb->pdb_octets_rcvd, m->m_pkthdr.len);
                                ADDQ(sc->xpdb->pdb_frames_rcvd, 1L);
                                if (fptr->fddi_dhost[0] & 1) {
                                        ADDQ(sc->xpdb->pdb_mc_octets_rcvd, m->m_pkthdr.len);
                                        ADDQ(sc->xpdb->pdb_mc_frames_rcvd, 1L);
                                }
#ifdef MFADEBUG3
				printf("Calling ether_input with receive mbuf\n");
				mfadumpmbuf(m);
#endif
				ether_input(&(sc->is_ed), (struct ether_header *)fptr, m, 0);
			} else m_freem(m);
		}
	}

	for (xmits = 0, curring = &sc->xmt; !RING_EMPTY(curring); ) {
		register mfaxmt_ent_t *nextin;
		mfaxmt_ent_t *sop;
		sop = RING_NEXTIN(curring, mfaxmt_ent_t);
#ifdef MFADEBUG
		if (mfadebug > 1)
			printf("mfa%d: mfaintr: xmt ring 0x%x (%s%s%s 0x%x)\n", sc->unit,
				sop,
				(sop->owner ? "O" : ""),
				(sop->sop ? "S" : ""),
				(sop->eop ? "E" : ""),
				*(u_long *)sop);
#endif
		if (sop->owner == FALSE)
			break;
		xmits++;
		if (sop->sop != TRUE)
			panic("mfa: mfaintr: xmt ring: owner but sop = FALSE");
		if (sop->ers == TRUE)
			ifp->if_oerrors++;
		nextin = sop;
		while (nextin->eop == FALSE) {
		/* 
		 * Don't zero these fields, they're useful for debugging
                 *      nextin->pa_lo = nextin->pa_hi = nextin->ers = 0;
		 */
			nextin->owner = TRUE;
			RING_ADV_NEXTIN(curring, mfaxmt_ent_t);
			nextin = RING_NEXTIN(curring, mfaxmt_ent_t);
#ifdef MFADEBUG
			if (mfadebug > 1)
				printf("mfa%d: mfaintr: xmt ring 0x%x (%s%s%s 0x%x)\n", sc->unit,
				nextin,
				(nextin->owner ? "O" : ""),
				(nextin->sop ? "S" : ""),
				(nextin->eop ? "E" : ""),
				*(u_long *)nextin);
#endif
		}
		ifp->if_opackets++;
		if ((m = RING_MBUF(curring, nextin, mfaxmt_ent_t)) != NULL) {
                       if (sop->ers != TRUE) {
                                struct mbuf *m0;
                                int pktlen;
                                for (pktlen = 0, m0 = m; m0; m0 = m0->m_next)
                                        pktlen += m0->m_len;
                                pktlen -= sizeof(struct fddi_header);
                                fptr = mtod(m, struct fddi_header *);
                                ADDQ(sc->xpdb->pdb_octets_sent, pktlen);
                                ADDQ(sc->xpdb->pdb_frames_sent, 1L);
                                if (fptr->fddi_dhost[0] & 1) {
                                        ADDQ(sc->xpdb->pdb_mc_octets_sent, pktlen);
                                        ADDQ(sc->xpdb->pdb_mc_frames_sent, 1L);
                                }
                        }
			RING_SETMBUF(curring, nextin, mfaxmt_ent_t, NULL);
			m_freem(m);
		}
	/*
	 * don't zero these fields, they're useful for debugging 
         *      nextin->pa_lo = nextin->pa_hi = nextin->ers = 0;
	 */
		nextin->owner = TRUE;
		MFAFLUSH();
		RING_SETLASTIN(curring, sop, mfaxmt_ent_t);
		RING_ADV_NEXTIN(curring, mfaxmt_ent_t);
	}

	/*
	 * Ring Release function. Tell the adapter the last entry we processed.
	 * (Dequeue next transmit request if the transmit ring has space.)
	 */
	{
	if (rcvs || xmits) {
		pa = (u_long)RING_LASTIN_PA(&sc->rcv, mfarcv_ent_t);
#ifdef MFADEBUG3
		printf("mfaintr: Setting rcv_hib pa to 0x%x\n", pa);
#endif
		WRTCSR(LONG_32, sc->ctlr, addr->rcv_hib_hi, (pa >> 32) & 0xffffffff);
		WRTCSR(LONG_32, sc->ctlr, addr->rcv_hib_lo,  pa & 0xffffffff);
		pa = (u_long)RING_LASTIN_PA(&sc->xmt, mfaxmit_ent_t);
#ifdef MFADEBUG3
		printf("mfaintr: Setting xmit_hib pa to 0x%x\n", pa);
#endif
		WRTCSR(LONG_32, sc->ctlr, addr->xmit_hib_hi, (pa >> 32) & 0xffffffff);
		WRTCSR(LONG_32, sc->ctlr, addr->xmit_hib_lo, pa & 0xffffffff);

		sc->omfarcv_hib_lo = RING_LASTIN_PA(&sc->rcv, mfarcv_ent_t);
		sc->omfaxmit_hib_lo = RING_LASTIN_PA(&sc->xmt, mfaxmt_ent_t);
		MFAFLUSH();
	} else 
		printf("mfa%d: stray interrupt\n", sc->unit);
	}
	if (xmits) {
		ifp->if_flags &= ~IFF_OACTIVE;
		mfastart(unit);
	}

	/*
	 * Drop softc lock and return from interrupt.
	 */
/*	smp_unlock(&sc->lk_mfa_softc); */
	splx(s);
}

mfacuintr(ctlr)
struct controller *ctlr;
{
	int unit = ctlr->ctlr_num;
	register struct mfa_softc *sc = &mfa_softc[unit];
	struct mfadevice *addr = &sc->xregs;
	register struct ifnet *ifp = &sc->is_if;
	struct mbuf *m;
	int s = splimp();
	int unsol = 0;

#ifdef MFADEBUG
	if (mfadebug > 1)
		printf("In mfacuintr, unit = %x\n", unit);
#endif
	/*
	 * Lock softc, since we will be updating the per-unit ring pointers
	 * and active ring entry counts frequently.
	 */
/*	smp_lock(&sc->lk_mfa_softc, LK_RETRY); */

	while (!RING_EMPTY(&sc->cmd)) {
		register mfacmd_buf_t *xcmd;
		register mfacmd_ent_t *nextin;
		nextin = RING_NEXTIN(&sc->cmd, mfacmd_ent_t);
		if (nextin->owner == FALSE)
			break;
		m = RING_MBUF(&sc->cmd, nextin, mfacmd_ent_t);
		RING_SETMBUF(&sc->cmd, nextin, mfacmd_ent_t, NULL);
		xcmd = mtod(m, mfacmd_buf_t *);
#ifdef MFADEBUG
		if (mfadebug > 1)
			printf("command complete, opcode = %d\n", xcmd->opcode);
#endif
	/*
	 * Grab command buffer address from mbuf wrapper.
	 * Process which posted command will free both the
	 * wrapper and the command buffer.
	 */
		switch (xcmd->opcode) {
			case CMD_NOP:
			/*
			 * MFATIMEOUT timer went off before this command was
			 * processed; simply free the mbuf, since the person who
			 * issued this command has gone away.
			 */
			printf("mfa%d: command (type = %d) timed out\n", unit, xcmd->opcode);
			m_freem(m);
			break;
			case CMD_PARAM:
			case CMD_RDCNTR:
			case CMD_SYSID:
			case CMD_USTART:
			case CMD_UCHANGE:
			case CMD_USTOP:
			case CMD_STATUS:
			case CMD_SETSMT:
		/*
		 * On command failure, alert caller
		 * by invalidating the command opcode.
		 */
			if (nextin->ers == TRUE) {
				printf("mfa%d: command (type = %d) failed, error code: 0x%x (%s)\n",
				sc->unit, xcmd->opcode, nextin->erc,
				mfa_cmderrors[nextin->erc]);
				xcmd->opcode = CMD_INVAL;
			} else {
				xcmd->opcode = CMD_COMPLETE;
				break;
			}
			default:
				xcmd->opcode = CMD_INVAL;
				break;
		}
		RING_ADV_NEXTIN(&sc->cmd, mfacmd_ent_t);
	}

	while (1) {
		char **table = NULL;
		int offset;
		register mfaunsol_ent_t *nextin, *nextout;
		nextin = RING_NEXTIN(&sc->unsol, mfaunsol_ent_t);
		if (nextin->owner == FALSE)
			break;
		if (nextin->opcode >= MFAUNSOL_BASE && nextin->opcode - MFAUNSOL_BASE < sizeofarray(mfa_unsolopcodes)) {
			table = mfa_unsolopcodes; offset = nextin->opcode - MFAUNSOL_BASE;
		} else if (nextin->opcode >= MFAUNSOL_BASE2 && nextin->opcode - MFAUNSOL_BASE2 < sizeofarray(mfa_unsolopcodes2)) {
			table = mfa_unsolopcodes2; offset = nextin->opcode - MFAUNSOL_BASE2;
		} else if (nextin->opcode >= MFAUNSOL_BASE3 && nextin->opcode - MFAUNSOL_BASE3 < sizeofarray(mfa_unsolopcodes3)) {
			table = mfa_unsolopcodes3; offset = nextin->opcode - MFAUNSOL_BASE3;
	}
	if (table == NULL) {
		printf("mfa%d: Unsolicited Event: #%d (unknown)\n", unit, nextin->opcode);
	} else {
		printf("mfa%d: Unsolicited Event: %s (state is %d)\n", unit,
			table[offset], 
		RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK);
	}
	RING_ADV_NEXTIN(&sc->unsol, mfaunsol_ent_t);
	nextout = RING_NEXTOUT(&sc->unsol, mfaunsol_ent_t);
	nextout->owner = 0;
	unsol++;
	MFAFLUSH();
	RING_ADV_NEXTOUT(&sc->unsol, mfaunsol_ent_t);
	}
	if ((ifp->if_flags & IFF_RUNNING) && unsol) {
		WRTCSR(LONG_32, sc->ctlr, addr->unsol_fl, XPCP_POLL);
		MFAFLUSH();
	}

/*	smp_unlock(&sc->lk_mfa_softc); */
	splx(s);
	return;
}

mfaeintr(ctlr)
struct controller *ctlr;
{
	int unit = ctlr->ctlr_num;
	register struct mfa_softc *sc = &mfa_softc[unit];
	struct mfadevice *addr = &sc->xregs;
	register struct ifnet *ifp = &sc->is_if;
	int s = splimp();
	int pst, i, *ptr, state;
	struct mbuf *m;

#ifdef MFADEBUG
	if(mfadebug)
		printf("In mfaeintr, unit = %x\n", unit);
#endif

	/*
	 * Lock softc, since we will be updating the per-unit ring pointers
	 * and active ring entry counts frequently.
	 */
/*	smp_lock(&sc->lk_mfa_softc, LK_RETRY); */

	/*
	 * See if we got here due to a port state change interrupt. If so,
	 * need to log error, reset interface, and re-init.
	 * The port logout area on the DEMNA and DEMFA are identical in size,
	 * so we use that structure and defines to do a register dump of
	 * the DEMFA into the binary error logger.
	 */
	pst = RDCSR(LONG_32, sc->ctlr, addr->xpst);
	state = pst & XPST_MASK;
	if (state == XPST_HALT) {
	   if (((pst >> 16) & 0xff) != 0xff) {
		register struct el_rec *elrp;
		if ((elrp = ealloc(sizeof(struct el_xna), EL_PRILOW))) {
			struct el_xna *elbod = &elrp->el_body.elxna;
			bcopy(sc->xpdb->port_err, &elbod->xnatype.xnaxmi.xnaxmi_fatal,
				sizeof(struct el_xna));
			LSUBID(elrp,ELCT_DCNTL,ELDEV_REGDUMP,0,sc->xpdb->intvec.nid_mask,unit,
				XNA_FATAL);
			EVALID(elrp);
		}
		mfaprintporterror(sc, "port halted (eintr)");
		log(LOG_ERR, "mfa%d halted, port logout area follows:\n", sc->unit);
		ptr = (int *)sc->xpdb->port_err;
		for (i = 0; i < 32; i++, ptr++)
			log(LOG_ERR, "0x%x\n", *ptr);
#ifdef MFADEBUG4
		halt();
#endif
	   }
	   if (!(sc->flags & MFA_RFLAG)) {
		sc->flags |= MFA_RFLAG;
		mfareset(unit);
	   } else {
		mfaprintporterror(sc, "port reset failed (eintr)");
	   }
	   while (m = ifp->if_snd.ifq_head) {
		IF_DEQUEUE(&ifp->if_snd, m);
		m_freem(m);
	   }
	   ifp->if_flags &= ~IFF_RUNNING;
	} else if (mfadebug) {
#ifdef MFADEBUG
		switch(state) {
		case XPST_RESET:
			printf("mfaeintr: new state is RESET\n");
			break;
		case XPST_UNINIT:
			printf("mfaeintr: new state is UNINIT\n");
			break;
		case XPST_INIT:
			printf("mfaeintr: new state is INIT\n");
			break;
		case XPST_RUNNING:
			printf("mfaeintr: new state is RUNNING\n");
			break;
		case XPST_MAINTENANCE:
			printf("mfaeintr: new state is MAINTENANCE\n");
			break;
		case XPST_HALT:
			printf("mfaeintr: new state is HALTED\n");
			break;
		default:
			printf("mfaeintr: new state is unknown, val = %d\n",
				state);
		}
		printf("xpst contents = 0x%x\n", pst);
#endif
	}

/*	smp_unlock(&sc->lk_mfa_softc); */
	splx(s);
	return;
}

/*
 * Process an ioctl request.
 */
mfaioctl(ifp, cmd, data)
struct ifnet *ifp;
u_int cmd;
caddr_t data;
{
	register struct mfa_softc *sc = &mfa_softc[ifp->if_unit];
	struct mfadevice *addr = &sc->xregs;
	register mfacmd_buf_t *xcmd;
	struct protosw *pr;
	struct mbuf *m;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	register struct ifaddr *ifa = (struct ifaddr *)data;
	struct ctrreq *ctr = (struct ctrreq *)data;
	struct ifeeprom *ife = (struct ifeeprom *)data;
	struct ifchar *ifc = (struct ifchar *)data;
	int s, delay, ttimeout = 0, error = 0, state_change = 0;

#ifdef MFADEBUG
	if (mfadebug)
		printf("In mfaioctl, cmd = %x\n", cmd);
#endif
	switch (cmd) {
        	case SIOCENABLBACK:
        	case SIOCDISABLBACK:
		if (cmd == SIOCENABLBACK) { 
#ifdef MFADEBUG
			if(mfadebug) printf("SIOCENABLBACK\n");
#endif
			if ((ifp->if_flags & IFF_LOOPBACK) == 0)
				state_change = 1;
                	ifp->if_flags |= IFF_LOOPBACK;
		} else {
#ifdef MFADEBUG
			if(mfadebug) printf("SIOCDISABLBACK\n");
#endif
			if (ifp->if_flags & IFF_LOOPBACK)
				state_change = 1;
                	ifp->if_flags &= ~IFF_LOOPBACK;
		}
		if ((ifp->if_flags & IFF_RUNNING) && state_change) {

			/*
			 * Lock softc. issue a SHUT command 
			 * to cause a state change and then
		         * bring down the adapter and reset it 
			 */
			s = splimp();
			WRTCSR(LONG_32, sc->ctlr, addr->xpcs, XPCS_SHUT);
			sc->flags |= MFA_RFLAG;
			ifp->if_flags &= ~IFF_RUNNING;
			/*
		 	 * Wait 1 second for MFA change to Uninitialized state
			 * Then restart it to pick up the parameter change
		 	 */ 	
			for (delay = 100; delay > 0 && ((RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK) != XPST_UNINIT) ; delay--)  
				 DELAY(10000);
			if((RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK) == XPST_UNINIT) 
				mfainit(ifp->if_unit);
			else
			{
				printf("mfa%d: Can't transition to Uninitialized State \n",ifp->if_unit); 
				mfaprintporterror(sc, "loop back failure");
				error = EINVAL;
			}
			splx(s);
		}
                break;
		case SIOCRPHYSADDR: 
			/*
			 * read default hardware address. Lock softc while accessing
			 * per-unit physical address info.
			 */
#ifdef MFADEBUG
			if(mfadebug) printf("SIOCRPHYSADDR\n");
#endif
			s = splimp();
/*			smp_lock(&sc->lk_mfa_softc, LK_RETRY);  */
			bcopy(sc->is_dpaddr, ifd->default_pa, 6);
			bcopy(sc->is_addr, ifd->current_pa, 6);
/*			smp_unlock(&sc->lk_mfa_softc); */
			splx(s);
			break;
 

	case SIOCSPHYSADDR: 
		/* 
		 * Set physaddr. Lock softc while updating per-unit physical
		 * address, and for command processing as in "mfainit()".
		 */
#ifdef MFADEBUG
		if(mfadebug) printf("SIOCRPHYSADDR\n");
#endif
		s = splimp();
/*		smp_lock(&sc->lk_mfa_softc, LK_RETRY); */
		bcopy(ifr->ifr_addr.sa_data, sc->is_addr, 6);

		pfilt_newaddress(sc->is_ed.ess_enetunit, sc->is_addr);

		if (ifp->if_flags & IFF_RUNNING) {
			if (m = mfamkuser(sc, MFA_UNKNOWNU, CMD_UCHANGE)) {
				mfacmd(sc, m);
/*				smp_unlock(&sc->lk_mfa_softc); */
				splx(s);
				xcmd = mtod(m, mfacmd_buf_t *);
				MFATIMEOUT(xcmd, CMD_UCHANGE); /* Wait */
				switch (xcmd->opcode) {
					case CMD_COMPLETE:
					m_freem(m);
					break;
				case CMD_INVAL:
					m_freem(m);
				case CMD_NOP:
				default:
					error = EINVAL;
					break;
				}
			} else {
/*			smp_unlock(&sc->lk_mfa_softc); */
			error = ENOBUFS;
			splx(s);
			}
		} else {
/*			smp_unlock(&sc->lk_mfa_softc); */
			splx(s);
		}
		/* If an IP address has been configured then an ARP packet
		 * must be broadcast to tell all hosts which currently have
		 * our address in their ARP tables to update their information.
		 */
		if(((struct arpcom *)ifp)->ac_ipaddr.s_addr)
			rearpwhohas((struct arpcom *)ifp,
				  &((struct arpcom *)ifp)->ac_ipaddr);
		break;
	case SIOCDELMULTI: 
	case SIOCADDMULTI: 
#ifdef MFADEBUG
		if(mfadebug) {
			if(cmd == SIOCDELMULTI)
				printf("SIOCDELMULTI\n");
			else
				printf("SIOCADDMULTI\n");
		}
#endif
		/*
		 * Lock softc while updating per-unit multicast address
		 * list and for command processing as in "mfainit()".
		 */
		if ((ifr->ifr_addr.sa_data[0] & 1) == 0) {
			error = EINVAL;
/*			smp_unlock(&sc->lk_mfa_softc); */
			splx(s);
			goto done;
		}
		s = splimp();
/*		smp_lock(&sc->lk_mfa_softc, LK_RETRY); */
		if (cmd == SIOCDELMULTI) {
			int i;
		/*
		 * If we're deleting a multicast address, decrement
		 * the is_muse count and invalidate the address if
		 * count goes to zero.
		 */
			if (bcmp(ifr->ifr_addr.sa_data, etherbroadcastaddr, 6) == 0) {
/*				smp_unlock(&sc->lk_mfa_softc); */
		   		splx(s);
		   		goto done;
			}
			for (i = 0; i < NMULTI; i++) {
				if (bcmp(&sc->is_multi[i], ifr->ifr_addr.sa_data, 6) == 0)
				break;
			}
			if (i < NMULTI && (--sc->is_muse[i] == 0)) {
				bcopy(etherbroadcastaddr, &sc->is_multi[i], 6);
			} else {
/*				smp_unlock(&sc->lk_mfa_softc); */
				splx(s);
				goto done;
			}
		} else {
		/*
		 * If we're adding a multicast address, increment the
		 * is_muse count if it's already in our table, and
		 * return. Otherwise, add it to the table or return
		 * ENOBUFS if we're out of entries.
		 */
			int i, j = -1;
			if (bcmp(ifr->ifr_addr.sa_data, etherbroadcastaddr, 6) == 0) {
/*				smp_unlock(&sc->lk_mfa_softc); */
				splx(s);
				goto done;
			}
			for (i = 0; i < NMULTI; i++) {
				if (bcmp(&sc->is_multi[i], ifr->ifr_addr.sa_data, 6) == 0) {
					sc->is_muse[i]++;
/*					smp_unlock(&sc->lk_mfa_softc); */
					splx(s);
					goto done;
				}
				if (j == -1 && sc->is_muse[i] == 0)
					j = i;
			}
			if (j < 0) {
/*				smp_unlock(&sc->lk_mfa_softc); */
				error = ENOBUFS;
				splx(s);
				goto done;
			} else {
				bcopy(ifr->ifr_addr.sa_data, &sc->is_multi[j], 6);
				sc->is_muse[j]++;
			}
		}
		if (ifp->if_flags & IFF_RUNNING) {
		/*
		 * If we've successfully init'ed the interface,
		 * issue a UCHANGE command to update the ethernet
		 * user's multicast address list. Otherwise, the
		 * list will be initialized upon the first call
		 * to "mfainit()".
		 */
			if (m = mfamkuser(sc, MFA_UNKNOWNU, CMD_UCHANGE)) {
				mfacmd(sc, m);
/*				smp_unlock(&sc->lk_mfa_softc); */
				splx(s);
				xcmd = mtod(m, mfacmd_buf_t *);
				MFATIMEOUT(xcmd, CMD_UCHANGE);	/* Wait */
				switch (xcmd->opcode) {
				case CMD_COMPLETE:
					m_freem(m);
					break;
				case CMD_INVAL:
					m_freem(m);
				case CMD_NOP:
				default:
					error = EINVAL;
					break;
				}
			} else {
/*				smp_unlock(&sc->lk_mfa_softc); */
				error = ENOBUFS;
				splx(s);
			}
		} else {
/*			smp_unlock(&sc->lk_mfa_softc); */
			splx(s);
		}
		break;

	case SIOCRDZCTRS:
	case SIOCRDCTRS:
#ifdef MFADEBUG
		if(mfadebug) {
			if (cmd == SIOCRDZCTRS) 
				printf("SIOCRDZCTRS\n");
			else 
				printf("SIOCRDCTRS\n");
		}
#endif
		/*
		 * The adapter does not support clear counter command.
		 * So, we only support read counters. 
		 */
		if (ifp->if_flags & IFF_RUNNING)  {
			switch (ctr->ctr_type) {
				case FDDIMIB_SMT:
                                case FDDIMIB_MAC:
                                case FDDIMIB_PATH:
                                case FDDIMIB_PORT:
                                case FDDIMIB_ATTA:
					mfafmibfill(sc,ctr,ctr->ctr_type);
					break;
				case FDDI_STATUS:
					mfagetstatus(sc, &ctr->sts_fddi,sc->xstat);
					break;
				default:
					mfagetctrs(sc, &ctr->ctr_fddi);
					ctr->ctr_type = CTR_FDDI;
					break;
			}
		}
		if (cmd == SIOCRDZCTRS)
			error = EINVAL;
		break;
	case SIOCSIFADDR:
#ifdef MFADEBUG
		if (mfadebug) printf("SIOCSIFADDR\n");
#endif
		/*
		 * Init the interface if its not already running
		 */
		mfainit(sc->unit);
		switch(ifa->ifa_addr->sa_family) {
		case AF_INET:
			s = splimp();
/*			smp_lock(&lk_ifnet, LK_RETRY); */
			((struct arpcom *)ifp)->ac_ipaddr = IA_SIN(ifa)->sin_addr;
/*			smp_unlock(&lk_ifnet); */
			splx(s);
			break;
		default:
/*			if (pr=iffamily_to_proto(ifa->ifa_addr.sa_family))
				error = (*pr->pr_ifioctl)(ifp, cmd, data);
			printf("mfa%d: Unknown Protocol received\n", sc->unit);
*/
			break;
		}
		break;

	case SIOCSIFFLAGS:
#ifdef MFADEBUG
		if (mfadebug) printf("SIOCSIFFLAGS\n");
#endif
		if (ifp->if_flags & IFF_RUNNING) {
		/*
		 * If we've successfully init'ed the interface,
		 * issue a UCHANGE command to update the ethernet
		 * user's promiscuous bit based upon the interface
		 * flags.
		 */
			s = splimp();
/*			smp_lock(&sc->lk_mfa_softc, LK_RETRY); */
			if (m = mfamkuser(sc, MFA_UNKNOWNU, CMD_UCHANGE)) {
				mfacmd(sc, m);
/*				smp_unlock(&sc->lk_mfa_softc); */
				splx(s);
				xcmd = mtod(m, mfacmd_buf_t *);
				MFATIMEOUT(xcmd, CMD_UCHANGE);	/* Wait */
				switch (xcmd->opcode) {
				case CMD_COMPLETE:
					m_freem(m);
					break;
				case CMD_INVAL:
					m_freem(m);
				case CMD_NOP:
				default:
					error = EINVAL;
					break;
				}
			} else {
/*				smp_unlock(&sc->lk_mfa_softc); */
				error = ENOBUFS;
				splx(s);
			}
		}
		break;
	case SIOCIFRESET: /* reset the adapter */
#ifdef MFADEBUG
		if(mfadebug) printf ("SIOCIFRESET \n");
#endif
		s = splimp();
		if (ifp->if_flags & IFF_RUNNING) {

			/*
			 * Lock softc. issue a SHUT command 
			 * to cause a state change and then
		         * bring down the adapter and reset it 
			 */
			s = splimp();
			WRTCSR(LONG_32, sc->ctlr, addr->xpcs, XPCS_SHUT);
			sc->flags |= MFA_RFLAG;
			ifp->if_flags &= ~IFF_RUNNING;
			/*
		 	 * Wait 1 second for MFA change to Uninitialized state
			 * Then restart it to pick up the parameter change
		 	 */ 	
			for (delay = 100; delay > 0 && ((RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK) != XPST_UNINIT) ; delay--)  
				 DELAY(10000);
			if((RDCSR(LONG_32, sc->ctlr, addr->xpst) & XPST_MASK) == XPST_UNINIT) 
				mfainit(ifp->if_unit);
			else
			{
				printf("mfa%d: Can't transition to Uninitialized State \n",ifp->if_unit); 
				error = EINVAL;
			}
			splx(s);
		}
		splx(s);
		break;
	case SIOCIFSETCHAR:
#ifdef MFADEBUG
		if (mfadebug) printf ("SIOCIFSETCHAR \n");
#endif
		s = splimp();
		if (ifp->if_flags & IFF_RUNNING) {
			/*
			 * If we've successfully init'ed the interface,
			 * try to reset the FDDI ring parameters.
			 */
			struct mbuf *m;
			MGETHDR(m, M_DONTWAIT, MT_DATA);
			if (m) {
/*				smp_lock(&sc->lk_mfa_softc);  */
				xcmd = mtod(m, mfacmd_buf_t *);
				m->m_len = sizeof(xcmd->opcode) + sizeof(mfasetsmt_t);
			/* 
			 * Work around for microcode not accepting valid length
			 * of the SET_SMT command
			 */
				m->m_len = 24;
				bzero(xcmd, m->m_len);
				xcmd->opcode = CMD_SETSMT;
			/* 
			 * the ring purger comes up with the opposite value of
			 * what the DEMFA expects (1 enables purging, 0 disables
			 */
				if (ifc->ifc_ring_purger != 0xffffffff) {
					if (ifc->ifc_ring_purger)
						xcmd->mfasetsmt.dis_ring_purger = 0;
					else
						xcmd->mfasetsmt.dis_ring_purger = 1;
					xcmd->mfasetsmt.valid |= SMT_RING_PURGE;
				}
				if (ifc->ifc_treq != 0xffffffff) {
					xcmd->mfasetsmt.t_req = ifc->ifc_treq;
					xcmd->mfasetsmt.valid |= SMT_TREQ;
				}
				if (ifc->ifc_tvx != 0xffffffff) {
					xcmd->mfasetsmt.tvx = ifc->ifc_tvx;
					xcmd->mfasetsmt.valid |= SMT_TVX;
				}
				if (ifc->ifc_lem != 0xffffffff) {
					xcmd->mfasetsmt.lem_threshold = ifc->ifc_lem;
					xcmd->mfasetsmt.valid |= SMT_LEM;
				}
				if (ifc->ifc_rtoken != 0xffffffff) {
				/*
				 * the rtoken field is only a word, so we need
				 * to divide the rtoken field by 2000 to convert
				 * it from the 80ns units fddi_config uses to
				 * the 160us units the DEMFA uses.
				 */
					xcmd->mfasetsmt.rtoken_timeout = 
						ifc->ifc_rtoken / 2000;
					xcmd->mfasetsmt.valid |= SMT_RTOKEN;
				}
				mfacmd(sc, m);
/*				smp_unlock(&sc->lk_mfa_softc); */
				splx(s);
				xcmd = mtod(m, mfacmd_buf_t *);
				MFATIMEOUT(xcmd, CMD_SETSMT);
				switch (xcmd->opcode) {
					case CMD_COMPLETE:
					m_freem(m);
					if(ifc->ifc_treq != 0xffffffff )
						sc->t_req = ifc->ifc_treq;
					if(ifc->ifc_rtoken != 0xffffffff )
						sc->rtoken = ifc->ifc_rtoken;
					if(ifc->ifc_tvx != 0xffffffff )
						sc->tvx = ifc->ifc_tvx;
					if(ifc->ifc_lem != 0xffffffff )
						sc->lem = ifc->ifc_lem;
					if(ifc->ifc_ring_purger != 0xffffffff )
						sc->ring_purger = ifc->ifc_ring_purger;
					mfaupdstatus(sc, s);
					break;
				case CMD_INVAL:
					m_freem(m);
				case CMD_NOP:
				default:
					error = EINVAL;
					break;
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
		break;
	}
done:
#ifdef MFADEBUG3
	printf("mfaioctl: exiting with error = %d\n", error);
	DELAY(1000000)
#endif
	return(error);
}

/*
 * MFA watchdog timer (runs once per second). . Look at the "p_sbua"
 * counter in the port data block to determine if we should bump up
 * (or reduce) the number of active receive descriptors. If we need
 * more, try to allocate them now. Update our counters block once a
 * second.
 */
mfawatch(unit)
	int unit;
{
	register struct mfa_softc *sc = &mfa_softc[unit];
	struct mfadevice *addr = &sc->xregs;
	register struct ifnet *ifp = &sc->is_if;
	register mfapdb_t *xpdb = sc->xpdb;
	int s, i;

	/*
	 * Lock softc
	 */
	s = splimp();
/*  smp_lock(&sc->lk_mfa_softc, LK_RETRY); */
	if (ifp->if_flags & IFF_RUNNING) {

	/*
	 * Tweak sc->nactv based upon the "potential sbua"
	 * count provided by the adapter. We bump up the number
	 * of active descriptors if the p_sbua count has gone up
	 * during the last second, but bump it down if it has
	 * stayed the same for the last 60 seconds.
	 */
		sc->callno++;
		if (xpdb->p_sbua) {
			xpdb->p_sbua = 0;
			if (sc->rcv.goal < MFARCVMAX) {
			sc->rcv.goal++;
			sc->callno = 0;
#ifdef MFADEBUG3
			printf("mfawatch: rcv.goal raised to %d\n", sc->rcv.goal);
#endif
			}
		} else {
			if ((sc->callno > 60) && (sc->rcv.goal > MFARCVMIN)) {
			sc->rcv.goal--;
			sc->callno = 0;
#ifdef MFADEBUG3
			printf("mfawatch: rcv.goal lowered to %d\n", sc->rcv.goal);
#endif
			}
		}
		for (i = RING_ACTIVE(&sc->rcv); i < sc->rcv.goal; i++) {
			if (mfarcvdesc(sc, RCV_INITENTRY, NULL) != 0)
			break;
		}
		mfaupdctrs(sc, s);
		if ((sc->callno % 5) == 0)
			mfaupdstatus(sc, s);
	}
	ifp->if_timer = 1;
/*  smp_unlock(&sc->lk_mfa_softc); */
	splx(s);
}

mfarcvdesc(sc, op, m_ret)
struct mfa_softc *sc;
int op;
struct mbuf **m_ret;
{
	register mfaring_t *rcvring = &sc->rcv;
	register mfarcv_ent_t *nextin, *nextout;
	register struct ifnet *ifp = &sc->is_if;
	register struct mbuf **m_ptr;
	struct mfadevice *addr = &sc->xregs;
	register int phyaddr;
	mfarcv_ent_t *sop;
	int len, error = 0, failed = 0, mbnum = 0;
	struct mbuf *m_in = NULL, *m_out = NULL;

#ifdef MFADEBUG3
	printf("In mfarcvdesc\n");
#endif
	if (m_ret)
	*m_ret = NULL;
	/*
	 *  If we were called to get the next buffer, make sure we own it.
	 */
	if (op & RCV_GETBUFFER) {
		struct fddi_header *fptr;
		sop = nextin = RING_NEXTIN(rcvring, mfarcv_ent_t);
#ifdef MFADEBUG
		if (mfadebug > 1)
			printf("mfa%d: rcv ring 0x%x (0x%x)\n", sc->unit, sop, *(int *)sop);
#endif
		if (sop->owner == FALSE)
			return(EWOULDBLOCK);

	/* At this point the ring entry is owned by us, lets start processing */
		m_in = RING_MBUF(rcvring, nextin, mfarcv_ent_t);
		fptr = mtod(m_in, struct fddi_header *);
		RING_SETMBUF(rcvring, nextin, mfarcv_ent_t, NULL);
		RING_ADV_NEXTIN(rcvring, mfarcv_ent_t); /* point to next buffer */
		RING_SETLASTIN(rcvring, sop, mfarcv_ent_t);

		if (sop->sop == FALSE) {
#ifdef MFADEBUG
			if (mfadebug)
				printf("mfa%d: mfacrvdesc: SOP not set in rcv frame?\n", sc->unit);
#endif
				goto bad;
		}
		if (sop->eop == TRUE) {
			mbnum = 1;
		/*	m_in = RING_MBUF(rcvring, nextin, mfarcv_ent_t); */
		/*	RING_SETMBUF(rcvring, nextin, mfarcv_ent_t, NULL); */
		/*	RING_ADV_NEXTIN(rcvring, mfarcv_ent_t);	point to next buffer */
		} else {
			register mfarcv_ent_t *eop;
			struct mbuf *m_last;
			m_ptr = &m_in;
			while (nextin->eop == FALSE) {
				nextin = RING_NEXTIN(rcvring, mfarcv_ent_t);
				if (nextin->owner == FALSE) {
#ifdef MFADEBUG
				if (mfadebug)
					printf("mfa%d: multiple receive with slot not owned!!! \n", sc->unit);
#endif
					goto bad;
				}
				nextin->owner = TRUE;
				m_last = RING_MBUF(rcvring, nextin, mfarcv_ent_t);
				if (nextin->length1 == 0)
					m_freem(m_last);
				else {
					*m_ptr = m_last;
					m_ptr = &(*m_ptr)->m_next;
				}
				RING_SETMBUF(rcvring, nextin, mfarcv_ent_t, NULL);
				RING_ADV_NEXTIN(rcvring, mfarcv_ent_t);		/* point to next buffer */
				mbnum++;
				if (mbnum > 2)
					panic("mfa: invalid number of receive segments");
			}
			*m_ptr = NULL;
		}
		MFAFLUSH();
		/* RING_SETLASTIN(rcvring, sop, mfarcv_ent_t); */
		len = sop->length1;
		if (sop->ers) {
			switch (sop->rmc_rcc & MFARCV_RCC) {
			 case MFARCV_OVERRUN: /*  frame too long */
			   if(len == 8192 || len == 8191) {
				ADDQ(sc->xpdb->pdb_frame_toolong_errors,1L);
			   } else {
#ifdef MFADEBUG
				if (mfadebug)
				   printf("mfa%d: RMC FIFO Overflow\n",sc->unit);
#endif
			   }
			   break;
			 case MFARCV_INTERFACE_ERR: /* RMC/MAC interface error */
			   printf("mfa%d: RMC/MAC interface error\n",sc->unit);
			   break;
			 default:
			   switch(sop->rmc_rcc & MFARCV_RCC_rrr) { 
			     case MFARCV_RCC_NORMAL:
                               if(sop->rmc_rcc & MFARCV_RCC_C) {
				   ADDQ(sc->xpdb->pdb_block_check_errors,1L);
#ifdef MFADEBUG
				   if (mfadebug)
                                     printf("mfa%d: Block Check Error\n",sc->unit);
#endif
                               } else 
				   if ( (!(sop->fsc2 & MFARCV_FSC_29) && 
					 !(sop->fsc & MFARCV_FSC_28_27)) || 
				         (sop->rmc_fsb & MFARCV_FSB_E)) {
				   	      ADDQ(sc->xpdb->pdb_frame_status_errors,1L);
#ifdef MFADEBUG
				   	      if (mfadebug)
                                                printf("mfa%d: Frame status error\n",sc->unit);
#endif
                                   }
                                   break;
                             case MFARCV_RCC_RMC_INVALID_LEN:
			       ADDQ(sc->xpdb->pdb_frame_alignment_errors,1L);
#ifdef MFADEBUG
			       if (mfadebug)
                                  printf("mfa%d: Frame Alignment Error\n",sc->unit);
#endif
                               break;
                             case MFARCV_RCC_DA_NOMATCH:
			       ADDQ(sc->xpdb->pdb_unrecog_indiv_frame_dst,1L);
			       if (fptr->fddi_dhost[0] & 1)
			           ADDQ(sc->xpdb->pdb_unrecog_mc_frame_dst,1L);
                             case MFARCV_RCC_SA_MATCHED:
                             case MFARCV_RCC_RMC_ABORT:
#ifdef MFADEBUG
			       if (mfadebug)
                                  printf("mfa%d: Hardware problem \n",sc->unit);
#endif
                               break;
                             case MFARCV_RCC_FRAGMENT:
                             case MFARCV_RCC_FORMAT_ERR:
                             case MFARCV_MAC_RESET:
#ifdef MFADEBUG
			       if (mfadebug)
                                 printf("mfa%d: Fragment or format error or MAC reset\n", sc->unit);
#endif
                               break;
                             defualt:
#ifdef MFADEBUG
			       if (mfadebug)
                                 printf("mfa%d: Wrong RMC descriptor report 0x%x \n", sc->unit,sop->rmc_rcc);
#endif
                               break;
                           }
                           break;
			}
			ifp->if_ierrors++;
			goto bad;
		}
		len = sop->length1 - 4; 	/* ignore trailing CRC bytes */
#ifdef MFADEBUG
		if (len > max_fddi_rcv) {
			max_fddi_rcv = len;
			printf("New FDDI max receive size = %d\n", len);
		}
#endif
		m_in->m_len = len;
		/* fptr = mtod(m_in, struct fddi_header *); */
		switch (fptr->fddi_fc & ~FDDIFC_Z) {	/* take out the priority */
			case FDDIFC_LLC_ASYNC:		/* for LLC frame */
			case FDDIFC_LLC_SYNC: 
			if (len < FDDILLCMIM) {
#ifdef MFADEBUG
				if (mfadebug)
					printf("mfa%d: LLC frame too short - frame len %d", sc->unit, len);
#endif
					goto bad;
			}
			break;
			default:
#ifdef MFADEBUG
				if (mfadebug)
					printf("mfa%d: unrecognize frame FC 0x%2x\n", sc->unit, fptr->fddi_fc);
#endif
				goto bad;
		}
                /*
                 * Setup the packet-header at this time. The length should be
                 * adjusted for the network-layer, but we still need to continue
                 * working with m_len since the header needs a place to live.
                 */
                m_in->m_pkthdr.rcvif = &sc->is_if;
                m_in->m_pkthdr.len = len - sizeof(struct fddi_header);

                /*
                 * If we can fit the incoming data into a couple of small mbufs,
                 * swap the data, then free the cluster mbuf for reuse later.
                 */
                if (len <= MINCLSIZE) {
                         struct mbuf *nh, *n;
                         int nhlen=0, nlen=0;

                        /* Get an mbuf with a packet-header first */
                        MGETHDR(nh, M_DONTWAIT, MT_DATA);

                        if (nh) {
                                /* If one mbuf isn't enough, get another. */
                                if (len > MHLEN) {
                                        MGET(n, M_DONTWAIT, MT_DATA);

                                        if (n) {
                                                nhlen = MHLEN;
                                                nlen = len - nhlen;
                                        } else
                                                m_freem(nh);
                                } else
                                        nhlen = len;

                                /* Copy from the big mbufs to the small ones. */
                                if (nhlen) {
                                        nh->m_pkthdr = m_in->m_pkthdr;
                                        bcopy(mtod(m_in, caddr_t),
                                              mtod(nh, caddr_t), nhlen);
                                        nh->m_len = nhlen;

                                        if (nlen) {
                                                nh->m_next = n;
                                                bcopy(mtod(m_in, caddr_t)+nhlen,
                                                      mtod(n, caddr_t), nlen);
                                                n->m_len = nlen;
                                        }

                                        /*
                                         * Complete the swap for our caller &
                                         * re-use the cluster mbuf for the next
                                         * receive.
                                         */
                                        *m_ret = nh;
                                        m_out = m_in;
                                        goto good;
                                }
                        }
                }
	}
	/*
	 * For the ESP test, we only need 512 bytes.
	 */
	if (op & RCV_INITESP)
		mbnum = 1;

	/*
	 * When initializing an entry, most we'll ever need is one
	 * ring slot per entry for alpha.
	 */
	if (op & RCV_INITENTRY) 
		mbnum = 1;

	if (mbnum > RING_FREESPACE(rcvring))
		mbnum = RING_FREESPACE(rcvring);
	/*
	 * At this point, we need to try to allocate a cluster to either
	 * initialize the ring entry or to replenish the one we have taken
	 * away.
	 */

	for (m_ptr = &m_out; mbnum > 0; mbnum--, m_ptr = &(*m_ptr)->m_next) {
		MFACLGET((*m_ptr));
		if ((*m_ptr) == 0) {
			failed = TRUE;
			break;
		}
	}
	if (failed) {
		m_freem(m_out);
#ifdef MFADEBUG3
		printf("mfarcvdesc: failed to get mbuf, free = %d\n", mbstat.m_clfree);
#endif
	/*
	 * If we failed to allocate the mbufs and we were trying
	 * fetch a incoming frame, drop the frame, copy the ring entry
	 * to the end of ring, increment user buffer unavail, and return NULL.
	 */
		if (op & RCV_GETBUFFER)
			sc->mfa_ubuacntr++;
bad:
		if (op & RCV_GETBUFFER)
			*m_ret = NULL;

        /* Do not return EWOULDBLOCK. At this point we have processed a
         * receive entry, therefore m_in has the pointer to that ring desc.
         * - ff -
		error = EWOULDBLOCK;
		if (!m_in) 
			return(error);
	 */
	/*
	 * We have failed to allocate enough mbuf to replenish the receive
	 * ring.  So we set the replenish list to the input list and refill
	 * the ring;
	 */
		m_out = m_in;
		m_in = NULL;
	} else if (op & RCV_GETBUFFER) {
	/*
	 * At this point, this routine has allocated all the resources it needs.
	 */
		*m_ret = m_in;
	} else {
		if (op & RCV_INITESP)
			m_out->m_len = 512;
	}

	/*
	 * Finally! We are to init the ring entry withe mbufs we have allocated.
	 * We fill each ring entry with one cluster mbuf at a time.  We will
	 * fill the entry with 2 segments - 1 of 4096 bytes, the other of 4096
	 * minus 64, needed to work around a DEMFA feature where the total can't
	 * be 8192...
	 */
good:
	for (; m_out != NULL; m_out = m_out->m_next) {
		u_long pfnum, phy, addr;

		nextout = RING_NEXTOUT(rcvring, mfarcv_ent_t);
	/*
	 * Can't bzero the entire entry, since it clears the owner bit.
	 * Just zero the fields not to be filled in
	 * bzero(nextout, sizeof(mfarcv_ent_t));
	 */
		nextout->unknown = nextout->musers = nextout->sop = 0;
		nextout->eop = nextout->fsc2 = nextout->ers = 0;
		nextout->uindex = 0;
		nextout->rmc_rcc = 0;
		nextout->rmc_fsb = 0;
		nextout->fsc = 0;
		RING_SETMBUF(rcvring, nextout, mfarcv_ent_t, m_out);
		addr = mtod(m_out, u_long);
		len = m_out->m_len;

		nextout->pa_lo = 0;
        	if((svatophys(addr, &phy)) == KERN_INVALID_ADDRESS)
                	panic("mfa: invalid physical address!\n");
		nextout->pa_hi1 = phy >> 9;
		nextout->length1 = mclbytes/2;
		if (op & RCV_INITESP)
			nextout->length1 = 512;
		nextout->lst1 = FALSE;
		nextout->length2 = (mclbytes/2) - 64;
		nextout->pa_hi2 = (phy + (mclbytes/2)) >> 9;
		nextout->lst2 = TRUE;
#ifdef MFADEBUG3
		printf("mfarcvdesc: added rcv mbuf, pa = %x\n", phy);
		DELAY(500000);
		if (phy & 0x1fff) {
			printf("mfarcvdesc: non-page aligned mbuf\n");
			mfadumpmbuf(m_out);
			panic("mfarcvdesc");
		}
#endif
		nextout->owner = FALSE;
		MFAFLUSH();
		RING_ADV_NEXTOUT(rcvring, mfarcv_ent_t);
		}
                if ((ifp->if_flags & IFF_RUNNING) && !(sc->flags & MFA_RFLAG)) {
			WRTCSR(LONG_32, sc->ctlr, addr->rcv_fl, XPCP_POLL);
			MFAFLUSH();
	}
	return(0);
}

struct mbuf *
mfamkparam(sc)
struct mfa_softc *sc;
{
	register struct ifnet *ifp = &sc->is_if;
	register struct mbuf *m;
	register mfacmd_buf_t *xparam;
	u_int purger = 0;
	/*
	 * Allocate PARAM buffer
	 */
	MFACLGET(m);
	if (!m)
		return(0);
	xparam = mtod(m, mfacmd_buf_t *);
	m->m_len = sizeof(xparam->opcode) + sizeof(mfaparam_t);
	bzero(xparam, sizeof(mfaparam_t));
	xparam->opcode = CMD_PARAM;
	/*
	 * reset any saved changes to ring parameters, plus our last
	 * shot of counters (all zeros if this is the first time)
	 */
	if (sc->t_req != 0xffffffff)
		xparam->mfaparam.t_req = sc->t_req;
	if (sc->tvx != 0xffffffff)
		xparam->mfaparam.tvx = sc->tvx;
	/* 
	 * work around for a firmware bug - can't put values 5 or 8 into
	 * the PARAM command until V1.5 microcode fix is in. 
	 */
	if (sc->lem != 0xffffffff) {
		if (((sc->lem == 5) || (sc->lem == 8))
		&& (((sc->dev_type >> 16) && 0xff) <= 0x14))
			xparam->mfaparam.lem_threshold = 0;
		else
			xparam->mfaparam.lem_threshold = sc->lem;
	}
	/* Convert from 80ns units to 160usec as expected by the adapter. */
	if (sc->rtoken != 0xffffffff)
		xparam->mfaparam.rtoken_timeout = sc->rtoken / 2000;
	if (sc->ring_purger != 0xffffffff)
		purger = (sc->ring_purger ? 0 : PARAM_Ring_Purger);
	bcopy(&sc->xctrs, &xparam->mfaparam.cntrs, sizeof(mfactrblk_t));
	/*
	 * Never set the MLA here.  Use USTART/UCHANGE to set a different
	 * user physical address.
	 */
	if (ifp->if_flags & IFF_LOOPBACK)
		xparam->mfaparam.flags = (PARAM_LM_ExtLoopback | purger);
	else
		xparam->mfaparam.flags = (PARAM_LM_NoLoopback | purger);
	return(m);
}

struct mbuf *
mfamkuser(sc, index, opcode)
struct mfa_softc *sc;
int index, opcode;
{
	register struct mbuf *m;
	register struct ifnet *ifp = &sc->is_if;
	register mfacmd_buf_t *xuser;
	register int cnt;
	int i;
	/*
	 * Allocate USER buffer
	 */
	MFACLGET(m);
	if (!m)
		return(0);
	xuser = mtod(m, mfacmd_buf_t *);
	m->m_len = sizeof(xuser->opcode) + sizeof(mfaustart_t);
	bzero(xuser, m->m_len);
	xuser->opcode = opcode;
	switch (index) {
	/* 
	 * Only handle the "unknown user" for now
	 */
	case MFA_UNKNOWNU:
		xuser->mfaustart.mode = USTART_MODE_Normal|USTART_DE;
		bcopy(sc->is_addr, &xuser->mfaustart.user_phys_addr, 6);
		cnt = 0;
		for (i = 0; i < NMULTI; i++) { 
			if (sc->is_muse[i] > 0)
				bcopy(&sc->is_multi[i], &xuser->mfaustart.mcalist[cnt++], 6);
		}
		xuser->mfaustart.addr_len = cnt;
		xuser->mfaustart.fc = USTART_FC_LLC;
		if (ifp->if_flags & IFF_PROMISC)	/* XXX */
			xuser->mfaustart.mode |= USTART_MODE_Promisc;
		if (ifp->if_flags & IFF_ALLMULTI)
			xuser->mfaustart.mode |= USTART_AMC;
		break;
	default:
		m_freem(m);
		return(0);
	}
	return(m);
}

mfaupdctrs(sc, s)
register struct mfa_softc *sc;
int s;
{
	register mfacmd_buf_t *xcntr;
	struct mbuf *m;

	/*
	 * Try to update the counters block in the softc by issuing
	 * a RDCNTR command. First, allocate an mbuf cluster.
	 */
	MFACLGET(m);
	if (!m)
		return;
	xcntr = mtod(m, mfacmd_buf_t *);
	m->m_len = sizeof(xcntr->opcode) + sizeof(mfactrblk_t);
	bzero(xcntr, m->m_len);
	xcntr->opcode = CMD_RDCNTR; 
	mfacmd(sc, m);
/*	smp_unlock(&sc->lk_mfa_softc); */
	splx(s);
	MFATIMEOUT(xcntr, CMD_RDCNTR); /* Wait */
	s = splimp();
/*  	smp_lock(&sc->lk_mfa_softc, LK_RETRY); */
	switch (xcntr->opcode) {
	case CMD_COMPLETE:
		bcopy(xcntr->mfactrs.reltime, &sc->xctrs, sizeof(mfactrblk_t));
		m_freem(m);
		break;
	case CMD_INVAL:
		m_freem(m);
	case CMD_NOP:
	default:
		break;
	}
}

mfaupdstatus(sc, s)
register struct mfa_softc *sc;
int s;
{
	register mfacmd_buf_t *xstat;
	struct mbuf *m;

	/*
	 * Try to update the status block in the softc by issuing 
	 * a STATUS command. First, allocate an mbuf cluster.
	 */
	MFACLGET(m);
	if (!m) 
		return;
	xstat = mtod(m, mfacmd_buf_t *);
/*
	Need to hard code size of status command to 108 bytes, even
	though it returns more. The following statement won't work:
	m->m_len = sizeof(xstat->opcode) + sizeof(mfastatus_t);
*/
	m->m_len = 0x6c;
	bzero(xstat, m->m_len);
	xstat->opcode = CMD_STATUS; 
	mfacmd(sc, m);
/*	smp_unlock(&sc->lk_mfa_softc); */
	splx(s);
	MFATIMEOUT(xstat, CMD_STATUS); /* Wait */
	s = splimp();
/*  	smp_lock(&sc->lk_mfa_softc, LK_RETRY); */
	switch (xstat->opcode) {
	case CMD_COMPLETE:
		bcopy(xstat->mfastatus.reltime, &sc->xstat, sizeof(mfastatus_t));
		m_freem(m);
		break;
	case CMD_INVAL:
		m_freem(m);
	case CMD_NOP:
	default:
		break;
	}
}

mfagetstatus(sc,ctr,fs)
	register struct mfa_softc *sc;
	register struct fstatus *ctr;
	register mfastatus_t *fs;
{
	struct mfadevice *addr = &sc->xregs;
	int scratch;

	/*
	 * Fill out the fddi status 
	 */
	bzero(ctr, sizeof(struct fstatus));
	
	/* 
	 * assign the default characterists value 
	 * and revision number; 
	 */
	ctr->t_req = sc->t_req;
	if (sc->t_req == 0xffffffff)
		ctr->t_req = sc->xstat.t_neg;
	ctr->t_max = sc->xstat.t_max;
	ctr->tvx   = sc->xstat.tvx;
	ctr->lem_threshold = sc->xstat.lem_threshold;
	/*
	 * The DEMFA keeps the rtoken field in a word, so we need to convert
	 * from the 160us units to the 80ns units fddi_config needs by
	 * multiplying by 2000.
	 */
	ctr->rtoken_timeout = sc->xstat.rtoken_timeout * 2000;
	ctr->pmd_type = sc->xstat.pmd_type;
	ctr->smt_version = sc->xstat.smt_version_id;
	scratch = (sc->dev_type >> 24) & 0xff;
	bcopy(&scratch, &ctr->phy_rev[0],2);
	scratch = (sc->dev_type >> 16) & 0xff;
	bcopy(&scratch, &ctr->fw_rev[0],2);
	
	/*
	 * When the adapter in HALT state, we can't
	 * return the last good values.
	 */
	scratch = RDCSR(LONG_32, sc->ctlr, addr->xpst);
	if((scratch & XPST_MASK) == XPST_HALT  || (scratch & XPST_MASK) == XPST_RESET) {
		ctr->link_state = 1 ; /* Off Ready */ 
		ctr->dup_add_test =  0 ; /* Unknown */
		ctr->ring_purge_state = 0 ; /* Purger Off */
		ctr->phy_state = 2 ; /* Off Ready */
	} else { 
		ctr->link_state = (sc->xstat.flags & STS_ST) >> 1;
		ctr->phy_state = sc->xstat.phy_state ;
		ctr->dup_add_test = (sc->xstat.flags & STS_DAT) >> 4;
		ctr->ring_purge_state = (sc->xstat.flags & STS_RP) >> 6; 
	}
	ctr->led_state = (scratch >> 11) & 0x7;
	bcopy(&sc->xstat.upstream_neighbor,&ctr->upstream[0],6);
	ctr->claim_token_mode = (sc->xstat.flags & STS_CTY) >> 8;
	bcopy(sc->is_dpaddr,&ctr->mla[0],6);
	ctr->state = scratch & 0x7;
	bcopy(&sc->xstat.downstream_neighbor, &ctr->downstream[0],6);
	ctr->neighbor_phy_type = sc->xstat.neighbor_phy_type;
	ctr->rej_reason = sc->xstat.reject_reason;
	ctr->ring_error = sc->xstat.ring_fault_reason;
	ctr->neg_trt = sc->xstat.t_neg;
	ctr->una_timed_out = (sc->xstat.flags & STS_UNTO) >> 9;
	ctr->phy_link_error = sc->xstat.link_error_estimate;
}	

mfagetctrs(sc,ctr)
	register struct mfa_softc *sc;
	register struct fstat *ctr;
{
	register int seconds;

	/*
	 * Fill out the fddi counters through the ethernet counter
	 * "fstat" structure. It is based upon the information
	 * returned by the RDCNTR command, updated by the mfawatch
	 * routine once a second. Just a lot of ugly work here to fit 64 bit
	 * counters into 16 and 32 bit fields. One note: the fst_ringbeacon
	 * and fst_ringbeaconrecv fields are always returned as zero, since
	 * the DEMFA counters block just doesn't record them. Everything else
	 * basically mirrors the DEFZA counters.
	 */
	bzero(ctr, sizeof(struct fstat));
	seconds = time.tv_sec - sc->ztime;
	if (seconds & 0xffff0000)
	    ctr->fst_second = 0xffff;
	else
	    ctr->fst_second = seconds & 0xffff;
	if (sc->xctrs.frame_count.hi)
		ctr->fst_frame = 0xffffffff;
	else 
		ctr->fst_frame = sc->xctrs.frame_count.lo;
	if (sc->xctrs.error_count.hi)
		ctr->fst_error= 0xffffffff;
	else 
		ctr->fst_error = sc->xctrs.error_count.lo;
	if (sc->xctrs.lost_count.hi)
		ctr->fst_lost = 0xffffffff;
	else 
		ctr->fst_lost = sc->xctrs.lost_count.lo;
	if (sc->xctrs.octets_rcvd.hi)
		ctr->fst_bytercvd = 0xffffffff;
	else 
		ctr->fst_bytercvd = sc->xctrs.octets_rcvd.lo;
	if (sc->xctrs.octets_sent.hi)
		ctr->fst_bytesent = 0xffffffff;
	else 
		ctr->fst_bytesent = sc->xctrs.octets_sent.lo;
	if (sc->xctrs.frames_rcvd.hi)
		ctr->fst_pdurcvd = 0xffffffff;
	else 
		ctr->fst_pdurcvd = sc->xctrs.frames_rcvd.lo;
	if (sc->xctrs.frames_sent.hi)
		ctr->fst_pdusent = 0xffffffff;
	else 
		ctr->fst_pdusent = sc->xctrs.frames_sent.lo;
	if (sc->xctrs.mc_octets_rcvd.hi)
		ctr->fst_mbytercvd = 0xffffffff;
	else 
		ctr->fst_mbytercvd = sc->xctrs.mc_octets_rcvd.lo;
	if (sc->xctrs.mc_frames_rcvd.hi)
		ctr->fst_mpdurcvd = 0xffffffff;
	else 
		ctr->fst_mpdurcvd = sc->xctrs.mc_frames_rcvd.lo;
	if (sc->xctrs.mc_octets_sent.hi)
		ctr->fst_mbytesent = 0xffffffff;
	else 
		ctr->fst_mbytesent = sc->xctrs.mc_octets_sent.lo;
	if (sc->xctrs.mc_frames_sent.hi)
		ctr->fst_mpdusent = 0xffffffff;
	else 
		ctr->fst_mpdusent = sc->xctrs.mc_frames_sent.lo;
	if (sc->xctrs.xmt_underruns.hi || 
			(sc->xctrs.xmt_underruns.lo & 0xffff0000))
	    ctr->fst_underrun = 0xffff;
	else
	    ctr->fst_underrun = (u_short)sc->xctrs.xmt_underruns.lo;
	if (sc->xctrs.xmt_failures.hi || 
			(sc->xctrs.xmt_failures.lo & 0xffff0000))
	    ctr->fst_sendfail= 0xffff;
	else
	    ctr->fst_sendfail = (u_short)sc->xctrs.xmt_failures.lo;
	if (sc->xctrs.block_check_errors.hi || 
			(sc->xctrs.block_check_errors.lo & 0xffff0000))
	    ctr->fst_fcserror = 0xffff;
	else
	    ctr->fst_fcserror = (u_short)sc->xctrs.block_check_errors.lo;
	if (sc->xctrs.frame_status_errors.hi || 
			(sc->xctrs.frame_status_errors.lo & 0xffff0000))
	    ctr->fst_fseerror = 0xffff;
	else
	    ctr->fst_fseerror = (u_short)sc->xctrs.frame_status_errors.lo;
	if (sc->xctrs.frame_alignment_errors.hi || 
			(sc->xctrs.frame_alignment_errors.lo & 0xffff0000))
	    ctr->fst_pdualig = 0xffff;
	else
	    ctr->fst_pdualig = (u_short)sc->xctrs.frame_alignment_errors.lo;
	if (sc->xctrs.frame_toolong_errors.hi || 
			(sc->xctrs.frame_toolong_errors.lo & 0xffff0000))
	    ctr->fst_pdulen = 0xffff;
	else
	    ctr->fst_pdulen = (u_short)sc->xctrs.frame_toolong_errors.lo;
	if (sc->xctrs.unrecog_indiv_frame_dst.hi || 
			(sc->xctrs.unrecog_indiv_frame_dst.lo & 0xffff0000))
	    ctr->fst_pduunrecog = 0xffff;
	else
	    ctr->fst_pduunrecog = (u_short)sc->xctrs.unrecog_indiv_frame_dst.lo;
	if (sc->xctrs.unrecog_mc_frame_dest.hi || 
			(sc->xctrs.unrecog_mc_frame_dest.lo & 0xffff0000))
	    ctr->fst_mpduunrecog = 0xffff;
	else
	    ctr->fst_mpduunrecog = (u_short)sc->xctrs.unrecog_mc_frame_dest.lo;
	if (sc->xctrs.data_overruns.hi || 
			(sc->xctrs.data_overruns.lo & 0xffff0000))
	    ctr->fst_overrun = 0xffff;
	else
	    ctr->fst_overrun = (u_short)sc->xctrs.data_overruns.lo;
	if (sc->xctrs.link_buffer_unavailable.hi || 
			(sc->xctrs.link_buffer_unavailable.lo & 0xffff0000))
	    ctr->fst_sysbuf = 0xffff;
	else
	    ctr->fst_sysbuf = (u_short)sc->xctrs.link_buffer_unavailable.lo;
	if (sc->xctrs.user_buffer_unavailable.hi || 
			(sc->xctrs.user_buffer_unavailable.lo & 0xffff0000))
	    ctr->fst_userbuf = 0xffff;
	else
	    ctr->fst_userbuf = (u_short)sc->xctrs.user_buffer_unavailable.lo;
	if (sc->xctrs.ring_init_other_station.hi || 
			(sc->xctrs.ring_init_other_station.lo & 0xffff0000))
	    ctr->fst_ringinit = 0xffff;
	else
	    ctr->fst_ringinit = (u_short)sc->xctrs.ring_init_other_station.lo;
	if (sc->xctrs.ring_init_initiated.hi || 
			(sc->xctrs.ring_init_initiated.lo & 0xffff0000))
	    ctr->fst_ringinitrcv = 0xffff;
	else
	    ctr->fst_ringinitrcv = (u_short)sc->xctrs.ring_init_initiated.lo;
	if (sc->xctrs.duplicate_token_detected.hi || 
			(sc->xctrs.duplicate_token_detected.lo & 0xffff0000))
	    ctr->fst_duptoken = 0xffff;
	else
	    ctr->fst_duptoken = (u_short)sc->xctrs.duplicate_token_detected.lo;
	if (sc->xctrs.dup_addr_test_failed.hi || 
			(sc->xctrs.dup_addr_test_failed.lo & 0xffff0000))
	    ctr->fst_dupaddfail = 0xffff;
	else
	    ctr->fst_dupaddfail = (u_short)sc->xctrs.dup_addr_test_failed.lo;
	if (sc->xctrs.ring_purge_errors.hi || 
			(sc->xctrs.ring_purge_errors.lo & 0xffff0000))
	    ctr->fst_ringpurge = 0xffff;
	else
	    ctr->fst_ringpurge = (u_short)sc->xctrs.ring_purge_errors.lo;
	if (sc->xctrs.bridge_strip_errors.hi || 
			(sc->xctrs.bridge_strip_errors.lo & 0xffff0000))
	    ctr->fst_bridgestrip = 0xffff;
	else
	    ctr->fst_bridgestrip = (u_short)sc->xctrs.bridge_strip_errors.lo;
	if (sc->xctrs.pc_traces_initiated.hi || 
			(sc->xctrs.pc_traces_initiated.lo & 0xffff0000))
	    ctr->fst_traceinit = 0xffff;
	else
	    ctr->fst_traceinit = (u_short)sc->xctrs.pc_traces_initiated.lo;
	if (sc->xctrs.pc_traces_received.hi || 
			(sc->xctrs.pc_traces_received.lo & 0xffff0000))
	    ctr->fst_tracerecv = 0xffff;
	else
	    ctr->fst_tracerecv = (u_short)sc->xctrs.pc_traces_received.lo;
	if (sc->xctrs.lem_rejects.hi || 
			(sc->xctrs.lem_rejects.lo & 0xffff0000))
	    ctr->fst_lem_rej = 0xffff;
	else
	    ctr->fst_lem_rej = (u_short)sc->xctrs.lem_rejects.lo;
	if (sc->xctrs.lem_link_errors.hi || 
			(sc->xctrs.lem_link_errors.lo & 0xffff0000))
	    ctr->fst_lem_events = 0xffff;
	else
	    ctr->fst_lem_events = (u_short)sc->xctrs.lem_link_errors.lo;
	if (sc->xctrs.lct_rejects.hi || 
			(sc->xctrs.lct_rejects.lo & 0xffff0000))
	    ctr->fst_lct_rej = 0xffff;
	else
	    ctr->fst_lct_rej = (u_short)sc->xctrs.lct_rejects.lo;
	if (sc->xctrs.tne_exp_rejects.hi || 
			(sc->xctrs.tne_exp_rejects.lo & 0xffff0000))
	    ctr->fst_tne_exp_rej = 0xffff;
	else
	    ctr->fst_tne_exp_rej = (u_short)sc->xctrs.tne_exp_rejects.lo;
	if (sc->xctrs.connections_completed.hi || 
			(sc->xctrs.connections_completed.lo & 0xffff0000))
	    ctr->fst_connection = 0xffff;
	else
	    ctr->fst_connection = (u_short)sc->xctrs.connections_completed.lo;
	if (sc->xctrs.elm_parity_errors.hi || 
			(sc->xctrs.elm_parity_errors.lo & 0xffff0000))
	    ctr->fst_ebf_error = 0xffff;
	else
	    ctr->fst_ebf_error = (u_short)sc->xctrs.elm_parity_errors.lo;

}

/*
 * routine to fill up the snmp FDDI MIB attributes  
 */
mfafmibfill(sc,fmib,type)
register struct mfa_softc *sc;
struct ctrreq *fmib;
int type;
{
#ifdef NOTYET
        switch (type) {
                case FDDIMIB_SMT:       /* SMT group */
                        /* adapter will provide this number */
			fmib->fmib_smt.smt_number = mfa_fddi_units;
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
                        fmib->fmib_mac.mac_number = mfa_fddi_units;
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
                        fmib->fmib_port.port_number = mfa_fddi_units;
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
                        fmib->fmib_atta.atta_number = mfa_fddi_units;
                        fmib->fmib_atta.atta_smt_index = sc->is_if.if_unit + 1 ;
                        fmib->fmib_atta.atta_index = 1 ;
                        fmib->fmib_atta.atta_class = 1;
                        fmib->fmib_atta.atta_bypass = 2;
                        fmib->fmib_atta.atta_IMaxExpiration = 0;
                        fmib->fmib_atta.atta_InsertedStatus = 3;
                        fmib->fmib_atta.atta_InsertPolicy = 3;
                        break;
                }
#endif
}

mfaprintporterror(sc, errstr)
register struct mfa_softc *sc;
char *errstr;
{
	struct mfadevice *addr = &sc->xregs;

	printf("mfa%d: %s: xber = 0x%x, xpst = 0x%x, \n\txpd1 = 0x%x, xpd2 = 0x%x, xpud = 0x%x\n",
	sc->unit, errstr, RDCSR(LONG_32, sc->ctlr, addr->xber_xmi), 
		RDCSR(LONG_32, sc->ctlr, addr->xpst),
	RDCSR(LONG_32, sc->ctlr, addr->xpd1), 
	RDCSR(LONG_32, sc->ctlr, addr->xpd2), 
	RDCSR(LONG_32, sc->ctlr, addr->xpud));
#ifdef MFADEBUG3
	DELAY(1000000);
#endif
}
mfadumpmbuf(m)
struct mbuf *m;
{
	int i;
	long phy;
	u_int *ptr;

#ifdef MFADEBUG
        if((svatophys(m, &phy)) == KERN_INVALID_ADDRESS)
                panic("mfa: invalid physical address!\n");
	printf("\n***Dumping contents of mbuf 0x%x pa = 0x%x \n", m, phy);
	printf("m_next = %x  m_nextpkt = %x \n", m->m_next, m->m_nextpkt);
	printf("m_type = 0x%x m_len = 0x%x \n", m->m_type, m->m_len);
	printf("m_flags = 0x%x %s %s %s \n", m->m_flags, 
		(m->m_flags & M_EXT ? "M_EXT" : ""), 
		(m->m_flags & M_PKTHDR ? "M_PKTHDR" : ""), 
		(m->m_flags & M_EOR ? "M_EOR" : "")); 
	DELAY(1000000);
	printf("m_data = 0x%x\n", m->m_data);
	printf("m_ipq[0] = 0x%x, m_ipq[1] = 0x%x\n", m->m_ipq[0], m->m_ipq[1]);
	printf("m_ipq[2] = 0x%x, m_ipq[3] = 0x%x\n", m->m_ipq[2], m->m_ipq[3]);
	DELAY(1000000);
	if (m->m_flags & M_PKTHDR) {
		printf("Packet Header Information and data: \n");
		printf("m_pkthdr.len = 0x%x m_pkthdr.rcvif = 0x%x\n", 
			m->m_pkthdr.len, (u_long)m->m_pkthdr.rcvif);
		DELAY(1000000);
		if (!(m->m_flags & M_EXT)) {
			printf("Packet Header Data (20 longwords): \n");
			ptr = (u_int *)m->m_data;
			for (i = 0; i < 20; i++, ptr++)
				printf(" %d: 0x%x \n", i, *ptr);
			DELAY(1000000);
		}
	}
	if (m->m_flags & M_EXT) {
		printf("External Packet Information and data:\n");
		printf("m_ext.ext_buf = 0x%x, m_ext.ext_free = 0x%x\n",
			m->m_ext.ext_buf, m->m_ext.ext_free);
		printf("m_ext.ext_size = 0x%x m->m_ext.ext_arg = 0x%x\n",
			m->m_ext.ext_size, m->m_ext.ext_arg);
		DELAY(1000000);
		printf("External Packet Data (20 longwords): \n");
		ptr = (u_int *)m->m_data;
		for (i = 0; i < 20; i++, ptr++)
			printf(" %d: 0x%x \n", i, *ptr);
		DELAY(1000000);
	}
	if (!(m->m_flags & M_EXT) && !(m->m_flags & M_PKTHDR)) {
		printf("Local Mbuf Packet Data (20 longwords): \n");
		ptr = (u_int *)m->m_data;
		for (i = 0; i < 20; i++, ptr++)
			printf(" %d: 0x%x \n", i, *ptr);
		DELAY(1000000);
	}

/* Call ourselves recursively if we're in a chain */
	if (m->m_next)
		mfadumpmbuf(m->m_next);
#endif
}
#endif

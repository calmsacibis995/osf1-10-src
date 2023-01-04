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
static char *rcsid = "@(#)$RCSfile: if_ln.c,v $ $Revision: 1.2.35.21 $ (DEC) $Date: 1994/01/11 18:08:05 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from if_ln.c	1.1	(OSF)	2/26/91";
 */

/* ---------------------------------------------------------------------
 * Modification History:
 *
 * Jan-16-92 Chran-Ham Chang
 *	Fixed a malformatted transmit frame problem on 3max
 *      Write the ownership bit after copy the transmit descriptor to
 *	the lance adapter	
 *
 * Nov-12-91 Tim Hoskins
 *	Added support for Alpha/OSF.
 *
 * June-04-91 Dave Gerson
 *	Changed structure array name "ds3maxplussw" to "dskn03sw"
 *	to apply more generic naming.
 *
 * May-29-91 Paul Grist/Dave Gerson
 *	Add support for 3MAX+/BIGMAX (DS_5000_300), uses same chip as
 *	3MIN, but different register locations.
 *
 * May-19-91 Uttam Shikarpur
 *	1) Fixed code in lnget to prevent panic when large UDP packets
 *	   were sent to the host, and the host ran out of mbufs.
 *	2) Removed looping back own broadcast. This is being done at
 *	   a higher layer.
 *	3) 3Min needs a contiguous block of memory. Such allocation is
 *	   not possible. It is now statically allocated.
 *	4) Cleaned up unwanted commented out SMP lines.
 *
 *	
 * Apr-19-91 Uttam Shikarpur
 *	Zeroed out buffers that were getting malloced.
 *
 * Apr-12-91 Uttam Shikarpur
 *	Bug fix in lnget() and some clean up.
 *
 * Mar-06-91	Mark Parenti
 *	Modify to use new I/O data structures.
 *
 * Feb-21-91	Uttam Shikarpur
 *	Ported ULTRIX 4.2 LANCE driver to OSF.
 *	This is based on ULTRIX version 4.14 of the if_ln.c 
 *	Incorporated some additions from OSF/1 code.
 * --------------------------------------------------------------------- */

#include "ln.h"

#if     NLN > 0 || defined(BINARY)
/*
 * Digital LANCE Network Interface
 */
#include <data/if_ln_data.c>

#ifdef __alpha
/*
 *  As stated in the "Porting Guide for Flamingo", credit to
 *  Andrew Duane, "... The R3000 preserves write ordering of all
 *  requests; the write buffer is a FIFO.  Alpha does not.  So
 *  memory barriers are required between multiple writes to I/O
 *  registers where order is important."  Ditto for memory buffers.
 *  For synchronization, we define the primitive LNIOSYNC().
 *
 *  Also, the MIPS writebuffer flush is the same as our memory barrier.
 */
#define LNIOSYNC()	mb()

#ifndef wbflush
#define	wbflush()	mb()
#endif

#else
#define LNIOSYNC()
volatile unsigned int *rdpptr;
#endif /* __alpha */

extern int cpu;
typedef volatile unsigned char * pvoluchar;

#define LNDEBUG 0
int	lndebug = 0;		/* debug flag - don't forget LNDEBUG */
int	lnprobe(), lnattach(), lnintr(), lnoutput();
int	lninit(), lnstart(), lnioctl(), lnwatch();
void	netnuke();

caddr_t lnstd[] = { 0 };
struct	driver lndriver =
	{ lnprobe, 0, lnattach, 0, 0, lnstd, 0, 0, "ln", lninfo };

static u_char ln_sunused_multi[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static unsigned int ln_crc_table[16];		/* crc initialization table */
static unsigned int ln_poly = 0xedb88320;	/* polynomial initialization */

#define LRBADDR(start,off)	(((int)(start))&0x01 ? \
				  (caddr_t)((vm_offset_t)(start)+(off*2)+ \
				  (off%2)): \
				  ((caddr_t)((vm_offset_t)(start)+(off*2)- \
				  (off%2))))
/*
 * Macros for accessing the generic io read/write routines.
 */
#define	LNREGWR8(r,v)	LNIOSYNC(); WRITE_BUS_D8((r),(v))
#define	LNREGWR16(r,v)	LNIOSYNC(); WRITE_BUS_D16((r),(v))
#define	LNREGWR32(r,v)	LNIOSYNC(); WRITE_BUS_D32((r),(v))

#define	LNREGRD8(r)	READ_BUS_D8((r))
#define	LNREGRD16(r)	READ_BUS_D16((r))
#define	LNREGRD32(r)	READ_BUS_D32((r))

/*
 * Convenient macros for enabling and disabling DMA in the IOASIC
 */
#define	LN_ENABLE_DMA(sc) {\
	LNREGWR32(sc->ssraddr, (LNREGRD32(sc->ssraddr) | LN_SR_DMA));\
	}

#define	LN_DISABLE_DMA(sc) {\
	LNREGWR32(sc->ssraddr, (LNREGRD32(sc->ssraddr) & ~LN_SR_DMA));\
	}



lnprobe(reg,ctlr)
vm_offset_t reg;
struct controller *ctlr;
/*
 * Probe the LANCE to see if it's there and initialize data
 * in the softc structure for the driver.
 *
 * Arguments:
 *	reg		Input: 	The base io-handle for the device
 *	ctlr		Input:  A controller structure to get the controller
 *				number of the adaptor.
 *
 * Return Value:
 *	Success:	Size of the ln_softc structure.
 *	Failure:	NULL.
 */
{
    int unit = 0;			/* The unit number */
    register struct ln_softc *sc;	/* The softc structure */
    int j;

    sc = (struct ln_softc *)kmem_alloc(kernel_map, sizeof(struct ln_softc));
    if (lndebug) {
	printf("ln%d: ln_softc is <0x%lx>\n", ctlr->ctlr_num, (vm_offset_t)sc);
    }

    if (!sc)
	return(0);

    /*
     * Clear out the memory
     */
    bzero((char *)sc, sizeof(struct ln_softc));

    /*
     * Fill in the unit number and initialize
     * the softc structure for the unit.
     */
    unit = ctlr->ctlr_num;
    ln_softc[unit] = sc;


    /*
     * Before we can physically access the LANCE,
     * the IOASIC must be programmed, via the Slot Register,
     * to properly decode the chip selects for LANCE I/O.
     * This step has been done for us by the firmware or
     * other means in previous stages of the boot process.
     */
    /**/

    /*
     * Fill in information into the softc based on
     * the cpu we are running on.
     */
    if (!ln_scinfo(reg, unit, ctlr)) {
	kmem_free(kernel_map, sc, sizeof (struct ln_softc));
	ln_softc[unit] = 0;
	return(0);
    }

    /*
     * Address CSR0 - Control and Status Register zero - and write 
     * a STOP into the CSR data.
     */
    LNREGWR16(sc->rapaddr, LN_CSR0);
    LNREGWR16(sc->rdpaddr, LN_STOP);
    LNIOSYNC();		/* guarantee write ordering */

    /*
     * Start lrb_offset to point to first byte of the local RAM
     * buffer. lnalloc bumps pointer as chunks of the buffer are
     * allocated.
     */
    sc->lrb_offset = 0;

    /*
     * Initialize some per unit counters
     */
    sc->callno = 0;

    lnshowmulti = 0;	
    lnbablcnt=0;	
    lnmisscnt=0;	
    lnmerrcnt=0;
    lnrestarts=0;	
    lncarcnt=0;

    /*
     * Allocate memory for the buffers and initialize the
     * init block and copy it out onto the chip.
     */
    ln_initbufs(unit);

    /*
     * Stop the chip completely by "nuking" it.
     * In this case we really are just preparing the chip
     * to commence operations.
     */
    netnuke(unit);

    if (sc->lnsw->ln_dma)
	LN_ENABLE_DMA(sc);		/* enable lance DMA in IOASIC */

    /*
     * clear IDON by writing 1, and start INIT sequence.
     * Note that the rdp had better NOT be cached in any way.
     */
    LNREGWR16(sc->rdpaddr, (LN_IDON | LN_INIT));
    LNIOSYNC();		/* guarantee write preceeds polling below */

    /* wait for init done */
    j=0;
    while (j++ < 100) {
	if ((LNREGRD16(sc->rdpaddr) & LN_IDON) != 0)
	    break;
	DELAY(10000);		/* this must not sleep */
    }

    /* Make sure got out okay */
    if ((LNREGRD16(sc->rdpaddr) & LN_IDON) == 0) {
	if (LNREGRD16(sc->rdpaddr) & LN_ERR) {
	    printf("ln%d: initialization error, csr0 = 0x%04x\n",
		unit,LNREGRD16(sc->rdpaddr));
	} else {
	    printf("ln%d: cannot initialize Lance\n",unit);
	}
	return(0);		/* didn't interrupt */
    }

    /*
     * set STOP to clear INIT and IDON (and everything else)
     */
    if (sc->lnsw->ln_dma)
	LN_DISABLE_DMA(sc);		/* disable lance DMA in IOASIC */

    LNREGWR16(sc->rdpaddr, LN_STOP);

    if (sc->lnsw->ln_dma)
	LN_ENABLE_DMA(sc);		/* reenable lance DMA in IOASIC */
    
    wbflush();
      /*
       * Register the shutdown routine for LANCE which will shutdown the
       * interface and prepare for starting up again.
       */
    drvr_register_shutdown(netnuke, (caddr_t)unit, DRVR_REGISTER);
 


    return( sizeof(struct ln_softc));
}


ln_scinfo(reg, unit, ctlr)
/*
 *	This function initializes the softc for the LANCE driver
 *	based on the cpu type we are running on.
 *
 * Arguments:
 *	reg	The io-handle base for the device.
 *	unit	The unit for which the softc structure is to be initialized.
 *
 * Return Value:
 *	Success:	Size of struct ln_softc.
 *	Failure:	NULL.
 */
io_handle_t reg;
int unit;
struct controller *ctlr;
{
    register struct ln_softc *sc = ln_softc[unit];
    char buf[TC_ROMNAMLEN + 1];
    int slot_id;
    struct bus_mem mem_info;

    sc->ln_is_depca = 0; 	/* default to non-depca type */
    sc->lrb_size = LN_LRB_SIZE;	/* Over-ride below if different. */

    switch (cpu) {

#ifdef __mips
    /* Will need some work to support mips again. */
    case DS_5400:			/* mipsfair */
	sc->lnsw = ds5400sw;
	sc->ln_narom = (u_int *)PHYS_TO_K1(sc->lnsw->ln_phys_narom);
	sc->ln_lrb = (pvoluchar)PHYS_TO_K1(sc->lnsw->ln_phys_lrb);
	sc->is_if.if_sysid_type = 94;
	break;

    case DS_5000:			/* 3max */
    case DS_MAXINE:                     /* MAXine */
    case DS_5000_100:			/* 3min */
    case DS_5000_300:			/* 3max+/bigmax */
	sc->is_if.if_sysid_type = ((cpu == DS_5000) ? 94 : 118);
	tc_addr_to_name(reg, buf);
	if (!strcmp(buf,"PMAD-AA ")) {  /* 3max like */
	    sc->lnsw = ds5000sw;
	    sc->ln_narom = (u_int *)((vm_offset_t)reg + sc->lnsw->ln_phys_narom);
	    sc->ln_lrb = (pvoluchar)((vm_offset_t)reg + sc->lnsw->ln_phys_lrb);
	} else {	/* PMAD_BA, IOASIC DMA version  */
	    vm_offset_t phy;

	    if (cpu == DS_5000_100)
		sc->lnsw = ds3minsw;
	    else if (cpu == DS_5000_300)
		sc->lnsw = dskn03sw;		
	    else
                sc->lnsw = dsMaxinesw;

	    sc->ln_narom = (u_int *)PHYS_TO_K1(sc->lnsw->ln_phys_narom);
	    sc->ssraddr = (u_int *)PHYS_TO_K1(sc->lnsw->ln_phys_ssr);
	    sc->siraddr = (u_int *)PHYS_TO_K1(sc->lnsw->ln_phys_sir);
	    sc->ldpaddr = (u_int *)PHYS_TO_K1(sc->lnsw->ln_phys_ldp);
	    sc->ln_lrb = (pvoluchar)ln_lrb;
	    if (!sc->ln_lrb) return(0);
	    
	    if((svatophys(sc->ln_lrb, &phy)) == KERN_INVALID_ADDRESS)
		panic("invalid physical address!\n");
    	    /*
             * Clear out the memory
             */
     	    bzero((char *)(sc->ln_lrb), sc->lrb_size);
	    
	    while (phy & 0xffff) {
		sc->ln_lrb++;	/* 64K align */
		if((svatophys(sc->ln_lrb, &phy)) == KERN_INVALID_ADDRESS)
		    printf("ln%d: ln_scinfo: Invalid physical address!\n", unit);
	    }
	    
	    /* enable IOASIC to do lance DMA */
	    LNREGWR32(sc->ssraddr, LNREGRD32(sc->ssraddr) | BIT16SET);
	    
	    /* next is really svtoioasic */
	    if((svatophys(sc->ln_lrb, &phy)) == KERN_INVALID_ADDRESS)
		panic("ln_scinfo: Invalid physical address!\n");
	    LNREGWR32(sc->ldpaddr, ((phy & LDPBITS) << 3));
	}
	break;

    case DS_3100:		/* pmax */
    case DS_5100:		/* mipsmate */
	sc->lnsw = pmaxsw;
	sc->ln_narom = (u_int *)PHYS_TO_K1(sc->lnsw->ln_phys_narom);
	sc->ln_lrb = (pvoluchar)PHYS_TO_K1(sc->lnsw->ln_phys_lrb);
	sc->is_if.if_sysid_type = (cpu == DS_3100 ? 67 : 94);
	break;

#endif /* mips */ 

    case DEC_3000_500:	/* Flamingo, Sandpiper */
    case DEC_3000_300:	/* Pelican */
      {
	register u_int tmp;
	vm_offset_t phy;	/* for ram buffer */

	tc_addr_to_name(reg, buf);
	printf("ln%d: DEC LANCE Module Name: %s\n", unit, buf);

	if (!strcmp(buf,"PMAD-AA ")) {
	    sc->lnsw = pmadaa_lnsw;
	    sc->is_if.if_sysid_type = 94;
#define DENSE(x) ((x) - 0x10000000)
	    sc->ln_narom = (u_int *)((vm_offset_t)DENSE(reg) + sc->lnsw->ln_phys_narom);
	    sc->ln_lrb = (pvoluchar)((vm_offset_t)DENSE(reg) + sc->lnsw->ln_phys_lrb);
	} else {
	    switch (cpu) {
	      case DEC_3000_500:	
	        sc->lnsw = kn15aa_lnsw;
	        break;
	      case DEC_3000_300:	
	        sc->lnsw = kn16aa_lnsw;
	        break;
	    }
	    sc->is_if.if_sysid_type = 170;
	    sc->ln_narom = (u_int *)PHYS_TO_KSEG(sc->lnsw->ln_phys_narom);
    	    sc->siraddr = reg + sc->lnsw->ln_sir_offset;
	    sc->ssraddr = reg + sc->lnsw->ln_ssr_offset;
    	    sc->ldpaddr = reg + sc->lnsw->ln_ldp_offset;

	    sc->ln_lrb = (pvoluchar)ln_lrb;
	    if (!sc->ln_lrb)
		return(0);

	    if((svatophys(sc->ln_lrb, &phy)) == KERN_INVALID_ADDRESS)
		panic("ln_scinfo: Invalid physical address!\n");
	    /*
	     * Clear out the memory
	     */
	    bzero((char *)(sc->ln_lrb), sc->lrb_size);

	    while (phy & 0x0ffff) {
		sc->ln_lrb++;	/* 64K align */
		if((svatophys(sc->ln_lrb, &phy)) == KERN_INVALID_ADDRESS)
		    panic("ln_scinfo: Invalid physical address!\n");
	    }
	    if (lndebug)
		printf("ln%d: ln_scinfo: ln_lrb<0x%lx> sc->ln_lrb<0x%lx> phy<0x%lx> ",
		    unit, ln_lrb,sc->ln_lrb,phy);


	    /*
	     * Set up IOASIC for LANCE, except for DMA.
	     * Don't enable DMA until after buffers and
	     * rings and LDP are set up.
	     *
	     * Do take LANCE out of reset and out of loopback.
	     */
	    LNIOSYNC();	/* make sure previous register accesses finish */
	    tmp  = LNREGRD32(sc->ssraddr);
	    tmp &= ~LN_SR_DMA;
	    /*
	     *  Console figures out thickwire vs 10BaseT.
	     *    tmp &= ~LN_SR_10BASET;
	     */
	    tmp |= (LN_SR_RSTOFF | LN_SR_NOLOOP);/* punt reset and loopback */
	    LNREGWR32(sc->ssraddr, tmp);

	    if (lndebug)
	        printf("ln%d: sc->ssraddr<0x%lx> ",unit, tmp);
	    LNIOSYNC();	/* force SSR update before LDP update below */


	    /* next is really svtoioasic */
	    if((svatophys(sc->ln_lrb, &phy)) == KERN_INVALID_ADDRESS)
		panic("ln_scinfo: Invalid physical address!\n");

	    /*
	     * Forgive the constants.  This disfigurement of the
	     * physical address is per the IO Controller ASIC Functional
	     * Specification, May 29, 1991, section 2.3.5.2.
	     */
	    tmp  = ((phy >> 29) & 0x01f);   /* bits 33:29 become bits 4:0 */
	    tmp |= ((phy & LDPBITS) << 3);  /* bits 28:17 become bits 31:20 */
	    LNREGWR32(sc->ldpaddr, tmp);

	    if (lndebug)
		printf("ln%d: sc->ldpaddr<0x%lx>\n",unit, tmp);
	  }
	  break;
	}
      	case DEC_2000_300:	/* Jensen, Culzean, Morgan/Mustang, Avanti? */
	    /* if the rom containing the station address can't be read or
	     * verified then punt
	     */
	    if (depca_read_rom(reg, unit) == 0) {
		if (depca_read_rom(reg, unit) == 0) {
		    printf("ln%d: lnprobe : Unable to read DEPCA station ROM.\n", unit);
		    return(0);
		}
	    }

	    sc->lnsw = eisa_lnsw;

	    /*
	     * Use the eisa get config call to find out how much memory was
	     * configured and where.
	     */
	    if (eisa_get_config(ctlr, EISA_MEM, NULL, &mem_info, 0) < 0) {
		printf("ln%d: lnprobe: failed to get memory info\n", unit);
		return(0);
	    }

            sc->ln_lrb = (void*)(mem_info.start_addr);
            sc->lrb_size = mem_info.size;

	    /* DEPCA's want to have data aligned in a particular way for
	     * efficiency's sake, so set a flag that others can see
	     */
	    sc->ln_is_depca = 1;

	    /* figure out if we're talking to a DE422; if we don't read a
	     * good looking ID then we're talking to a DE200 (note that we've
	     * already determined the existence of one of the two by reading
	     * and verifying the station prom)
	     */
	    slot_id = LNREGRD32(reg+ID_OFFSET);

	    if ((slot_id & DE422_MASK) == ID_DEC4220) {
		if (lndebug) {
                    printf("ln%d: DEC DE422 LANCE Ethernet Module\n", unit);
		    printf("local memory at 0x%x\n", sc->ln_lrb);
		}
	        sc->is_if.if_sysid_type = 182;
            } else {
	        if (lndebug) {
                    printf("ln%d: DEC DE200 LANCE Ethernet Module\n", unit);
		    printf("local memory at 0x%x\n", sc->ln_lrb);
		}
	        sc->is_if.if_sysid_type = 43;
	    }
            /* 
	     * Turn on the LED, allow interrupts to get off the board,
             * and turn on shadow RAM (to to make all 64K visible to
	     * the host).
             */
            LNREGWR16(reg+NICSR_OFFSET, NICSR_LED | NICSR_IEN | NICSR_SHE);

	    /* nuke the local buffer
	     */
            sc->lnsw->ln_bzero(sc->ln_lrb, sc->lrb_size);
	    break;

    default: 
	printf("ln%d: lnprobe : CPU type %d unknown\n", unit, cpu );
	return(0);
    }

    /*
     * Init io-handles for accessing the rdp and rap registers. Note that this
     * is common to all variations of the device/adapter.
     */
    sc->rdpaddr = reg + sc->lnsw->ln_rdp_offset;
    sc->rapaddr = reg + sc->lnsw->ln_rap_offset;

    return(sizeof(struct ln_softc)); 
}



ln_initbufs(unit)
/*
 * This allocates memory for - 
 * 1) The receive ring.
 * 2) The transmit ring.
 * 3) The init block.
 *
 * In Addition it initializes the init block and the multicast 
 * address array.
 *
 * Arguments:
 *	unit	The unit number of the interface.
 *
 * Return Value:
 *	Success:	Size of the ln_softc structure.
 *	Failure:	NULL.
 */
int unit;

{
    register struct ln_initb *initb; 	/* LRB initb ptr */
    int i, j, index;
    int prp;	 		 	/* physical addr ring pointer */
    unsigned int tmp, x;
    struct ln_softc *sc = ln_softc[unit];	

    /*
     * First the receive ring.
     * Allocate contiguous, 16-byte aligned (required)
     * space for both descriptor rings. "lnalloc" takes into account
     * the "alignment factor" for LRB sizing.
     */
    for (i = 0; i < nLNNRCV; i++) {
	sc->rring[i] = sc->lnsw->ln_alloc( sc,
					   sizeof(struct ln_ring),
					   (i==0 ? LN_QUAD_ALIGN : 0));
	if (sc->rring[i] == NULL) {
	    printf("ln%d: lnalloc: cannot alloc memory for recv descriptor rings\n", unit);
	    return(0);
	}
    }
    /*
     * Second the transmit ring.
     */
    for (i = 0; i < nLNNXMT; i++) {
	sc->tring[i] = sc->lnsw->ln_alloc( sc,
					   sizeof(struct ln_ring),
					   (i==0 ? LN_QUAD_ALIGN : 0));
	if (sc->tring[i] == NULL) {
	    printf("ln%d: lnalloc: cannot alloc memory for xmit descriptor rings\n", unit);
	    return(0);
	}
    }
    /*
     * Lastly, allocate local RAM buffer memory for the init block
     */
    sc->initbaddr = sc->lnsw->ln_alloc( sc,
					sizeof(struct ln_initb),
					LN_WORD_ALIGN);
    if (sc->initbaddr == NULL) {
	printf("ln%d: lnalloc: cannot alloc memory for init block\n", unit);
	return(0);
    }

    /* 
     * Initialize multicast address array. Number of active entries
     * is driven by number of "ADDMULTI" operations. (1, initially).
     */
    for (i=0; i < nLNMULTI; i++) {
	sc->muse[i] = 0;
	bcopy(ln_sunused_multi,&sc->multi[i],MULTISIZE);
    }
    sc->nmulti = 0;
    /*
     * Initialize Lance chip with init block, ln_initb
     *
     * ln_mode;				mode word
     * ln_sta_addr;			station address
     * ln_multi_mask;			multicast address mask
     * ln_rcvlist_lo, ln_rcvlist_hi;	rcv descriptor addr
     * ln_rcvlen;			rcv length
     * ln_xmtlist_lo, ln_xmtlist_hi;	xmt descriptor addr
     * ln_xmtlen;			xmt length
     */

    initb = &sc->ln_initb;
    initb->ln_mode = 0;		/* normal operation (mode==0) */

    if (sc->ln_is_depca) {
        for (i = j = 0; i < 3; i++, j += 2) {
             initb->ln_sta_addr[i] = (short)sc->is_int_addr[j];
             initb->ln_sta_addr[i] |= (short)sc->is_int_addr[j + 1] << 8;
        }
    } else {
        /*
         * fill out station address from the narom
         */
        for (i = j = 0; i < 3; i++, j += 2) {
            initb->ln_sta_addr[i] = 
	        (short)((sc->ln_narom[j]>>sc->lnsw->ln_na_align)&0xff);
	    initb->ln_sta_addr[i] |= 
	        (short)(((sc->ln_narom[j+1]>>sc->lnsw->ln_na_align)&0xff)<<8);
        }
    }

#if LNDEBUG
    if (lndebug)
	printf("ln%d: Station Address: %02x-%02x-%02x-%02x-%02x-%02x\n", unit,
	    (initb->ln_sta_addr[0] & 0xff),
	    (initb->ln_sta_addr[0] & 0xff00)>>8,
	    (initb->ln_sta_addr[1] & 0xff),
	    (initb->ln_sta_addr[1] & 0xff00)>>8,
	    (initb->ln_sta_addr[2] & 0xff),
	    (initb->ln_sta_addr[2] & 0xff00)>>8);
#endif

    /*
     * fill out multicast address mask
     */
    for (i = 0; i < 4; i++) {
	initb->ln_multi_mask[i] = 0x0000;
    }
    /*
     * initialize the multicast address CRC table
     */
    for (index = 0; !(unit) && index < 16; index++) {
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
     * Convert Virtual to LANCE relative
     */
    prp = sc->lnsw->ln_svtolance(sc, sc->rring[0]); 
    initb->ln_rcvlist_lo = (short)prp & 0xffff;
    initb->ln_rcvlist_hi = (char)(((int)prp >> 16) & 0xff);
    initb->ln_rcvlen = RLEN; 	/* Also clears rcvresv */

    prp = sc->lnsw->ln_svtolance(sc, sc->tring[0]);	
    initb->ln_xmtlist_lo = (short)prp & 0xffff;
    initb->ln_xmtlist_hi = (char)(((int)prp >> 16) & 0xff);
    initb->ln_xmtlen = TLEN; /* Also clears xmtresv */

    /*
     * Copy out the initblock info into the chip's address space.
     */
    sc->lnsw->ln_cpyout(initb, sc->initbaddr, (long)sizeof(struct ln_initb),(long)0);
    return(sizeof(struct ln_softc));
}


void
netnuke(unit)
/*
 * This routine shuts down the interface completly and prepares it
 * for starting it up again. 
 *
 * Arguments:
 *	unit:	The unit number to stop.
 *
 * Return Value:
 *	none.
 */
int unit;
{
    register int pi;		 	/* physical addr init block */
    struct ln_softc *sc;

    if ((unsigned)unit >= NLN)
	return;

    if ( ! (sc = ln_softc[unit]) )	/* punt if called before probe */
	return;

    /*
     * Set up CSR0
     */
    LNREGWR16(sc->rapaddr, LN_CSR0);
    LNREGWR16(sc->rdpaddr, LN_STOP);
    wbflush();
    DELAY(100);

    /*
     * Load LANCE control registers.
     * CSR3, CSR2 and CSR1
     */
    LNREGWR16(sc->rapaddr, LN_CSR3);

    if (sc->ln_is_depca) { 
        LNREGWR16(sc->rdpaddr, LN_ACON); 	/* ACON==1, BCON==BSWP==0 */
    } else {
        LNREGWR16(sc->rdpaddr, 0);		/* ACON==BCON==BSWP==0 */
    }
    LNIOSYNC();

    /*
     * Get the physical address for the init block for the
     * LANCE.
     */
    pi = sc->lnsw->ln_svtolance(sc, sc->initbaddr); 

    /* set-up CSR 1 */
    LNREGWR16(sc->rapaddr, LN_CSR1);
    LNREGWR16(sc->rdpaddr, ((u_short)(pi & 0xffff))); /* lo 8 bits */
    wbflush();
    
    /* set-up CSR 2 */
    LNREGWR16(sc->rapaddr, LN_CSR2);
    LNREGWR16(sc->rdpaddr, ((u_short)(((int)pi>>16) & 0xff)));/* hi 8 */
    wbflush();

    /*
     * Leave rap on CSR0
     */
    LNREGWR16(sc->rapaddr, LN_CSR0);
    wbflush();
    return;
}


lnattach(ctlr)
/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 *
 * Arguments:
 *	ctlr		struct controller containing the controller number.
 *
 * Return Value:
 *	None.
 */

struct controller *ctlr;
{
    register struct ln_softc *sc = ln_softc[ctlr->ctlr_num];
    register struct ifnet *ifp = &sc->is_if;
    register int i;
    register struct sockaddr_in *sin;

    sc->is_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;/* Bdcast address */
    sc->is_ac.ac_arphrd = ARPHRD_ETHER; 		  /* Ethernet addr  */

    ifp->if_unit = ctlr->ctlr_num;			/* Unit #	  */
    ifp->if_name = "ln";				/* Interface name */
    ifp->if_mtu = ETHERMTU; 
    ifp->if_mediamtu = ETHERMTU; 
    ifp->if_type = IFT_ETHER;				/* Ethernet	  */
    ifp->if_addrlen = 6;				/* Media address len*/
    ifp->if_hdrlen = sizeof(struct ether_header) + 8;   /* Media hdr len  
	                                                 * +8 for SNAP
							 */

    ifp->if_flags |= IFF_SIMPLEX | IFF_BROADCAST | 
		     IFF_MULTICAST | IFF_NOTRAILERS;	/* flags; IFF_SIMPLEX
							 * indicates we need to
							 * hear our own 
							 * broadcasts. The 
							 * looping is done at a
							 * higher level in 
							 * if_ethersubr.c
							 * /

    ((struct arpcom *)ifp)->ac_ipaddr.s_addr = 0;	

    /*
     * Read the address from the prom and save it.
     */
    if (sc->ln_is_depca)
        for ( i=0 ; i<6 ; i++ ) {
	    sc->is_addr[i] = sc->is_int_addr[i];
        }
    else
        for ( i=0 ; i<6 ; i++ ) {
	    sc->is_addr[i] = (u_char)((sc->ln_narom[i] >> sc->lnsw->ln_na_align) & 0xff);
        }
	
    sin = (struct sockaddr_in *)&ifp->if_addr;
    sin->sin_family = AF_INET;
    ifp->if_init = lninit;
    ifp->if_output = ether_output;
    ifp->if_start = lnoutput;	/* NOT lnstart! need ifp -> unit 1st */
    ifp->if_ioctl = lnioctl;
    ifp->if_timer = 0;
    ifp->if_watchdog = lnwatch; 
    ifp->if_reset = 0;
    ifp->if_version = "DEC LANCE Ethernet Interface";
    ifp->if_baudrate = ETHER_BANDWIDTH_10MB;
    printf("ln%d: %s, hardware address: %s \n", ctlr->ctlr_num,
	   ifp->if_version,ether_sprintf(sc->is_addr));
	
    attachpfilter(&(sc->is_ed));

    /* Attach the interface to the list of active
     * interfaces.
     */
    if_attach(ifp);
}


lninit(unit)
/*
 * Initialization of interface and allocation of mbufs for receive ring.
 *
 * Arguments:
 *	unit		The unit number of the interface
 *
 * Return Value:
 *	None. 
 */
int unit;
{
    register struct ln_softc *sc = ln_softc[unit];
    register struct ln_initb *initb = &sc->ln_initb;
    register struct lnsw *lnsw = sc->lnsw;
    register struct ifnet *ifp = &sc->is_if;
    int i,k;

    /* If the address is not known return */
    if (ifp->if_addrlist == (struct ifaddr *)0)
	return;

    /* If the interface is up and running do nothing */
    if (ifp->if_flags & IFF_RUNNING)
	return;

    /*
     * We only nail down our buffer addresses on the very
     * first call, then don't relinquish them again.
     */
    k = splimp();

    /*
     * Number of times this routine has been called.
     */
    sc->callno++;

    /*
     * Init the buffer descriptors and
     * indexes for each of the lists.
     */
    for ( i = 0 ; i < nLNNRCV ; i++ ) {
	lnsw->lninitdesc (sc, sc->rring[i], &sc->rlbuf[i],
			  (sc->callno == 1) ? LNALLOC : LNNOALLOC, i);
	/* give away rring */
	LNIOSYNC();			/* guarantee write ordering */
	lnsw->ln_setflag(sc->rring[i], LN_OWN);
    }
    for ( i = 0 ; i < nLNNXMT ; i++ ) {
	lninitdesc (sc, sc->tring[i], &sc->tlbuf[i],
		    (sc->callno == 1) ? LNALLOC : LNNOALLOC);
    }
    sc->nxmit = sc->otindex = sc->tindex = sc->rindex = 0;
    /*
     * Take the interface out of reset, program the vector, 
     * enable interrupts, and tell the world we are up.
     */

    LNREGWR16(sc->rapaddr, LN_CSR0);
    LNIOSYNC();			/* guarantee write ordering */

    if ( (ifp->if_flags & IFF_LOOPBACK)
	&& (initb->ln_mode & LN_LOOP) == 0) {
	/* if not in loopback mode, do loopback */
	initb->ln_mode &= ~LN_INTL;
	initb->ln_mode |= LN_LOOP;
	lnrestart(ifp);
	lninit(ifp->if_unit);
	splx(k);
	return;
    }

    /* start the Lance; enable interrupts, etc */
    if (sc->ln_is_depca) {
        LNREGWR16(sc->rapaddr, LN_CSR3);
        LNREGWR16(sc->rdpaddr, LN_ACON);
        LNREGWR16(sc->rapaddr, LN_CSR0);
    }
    LNREGWR16(sc->rdpaddr, (LN_START | LN_INEA));
    LNIOSYNC();			/* guarantee write ordering */

    ifp->if_flags |= IFF_RUNNING;
    ifp->if_flags &= ~IFF_OACTIVE;
    lnstart( unit );
    sc->ztime = time.tv_sec;
    splx(k);
}


lnintr(unit) 
/*
 * Ethernet interface interrupt processor. This routine
 * is called each time the network interface receives an
 * interrupt. After identifying what kind of an interrupt
 * was received - transmit or receive - it calls the 
 * respective routine to process the interrupt.
 *
 * Arguments:
 *	unit		The unit of the interrupted interface
 *
 * Return Value:
 *	None.
 */
int unit;
{
    register struct ln_softc *sc = ln_softc[unit];
    register struct ifnet *ifp;
    register unsigned int oldcsr;
    register unsigned short decsr;

    ifp = &sc->is_if;

    LNIOSYNC(); /* guarantee visibility of updated shared memory structures */

    /*
     * If a set-1-to-reset bit is 1, then writing a 1
     * back into the csr register will clear that bit.
     * This is OK; the intent is to clear the csr of all
     * errors/interrupts and then process the saved bits
     * in the old csr. We grab the "ln_flag" byte out of
     * the ring entry to check the ownership bit FIRST,
     * then grab the "ln_flag2" short word later to eliminate
     * a timing window in which the LANCE may update the
     * "ln_flag" before updating "ln_flag2".
     */
    oldcsr = LNREGRD16(sc->rdpaddr);	/* save the old csr */
    
    oldcsr &= ~LN_INEA;			/* clear interrupt enable */

#ifndef __alpha
    if(cpu == DS_5000) {  /* work around for R3000 bug */
	rdpptr=	(volatile unsigned int *)&sc->rdpaddr;
	*rdpptr = oldcsr;
    }else 
#endif
    	{
	LNREGWR16(sc->rdpaddr, oldcsr);
	LNIOSYNC();			/* flush to TURBOchannel */
    }

#if LNDEBUG
if (lndebug > 2)printf("ln%d: <0x%x>",unit, oldcsr);
#endif
    /*
     * check error bits
     */
    if ( oldcsr & LN_ERR ) {
	if (oldcsr & LN_MISS) {
	    /*
	     * LN_MISS is signaled when the LANCE receiver
	     * loses a packet because it doesn't own a
             * receive descriptor. Rev. D LANCE chips, which
	     * are no longer used, require a chip reset as
	     * described below.
	     */

	    lnmisscnt++;
#if LNDEBUG
	    if (lndebug)
		printf("ln%d: missed packet (MISS)\n",unit);
#endif
	    if(sc->ctrblk.est_sysbuf != 0xffff)
		sc->ctrblk.est_sysbuf++;

	    /* This is a toss-up.  We don't increment
	     * data overrun here because it's most likely
	     * the host's fault that packets were missed.
	     * No way to tell from here whether the LANCE
	     * is at fault.
	     *
	     *	 if (sc->ctrblk.est_overrun != 0xffff)
	     *		sc->ctrblk.est_overrun++;
	     */
	}
	if (oldcsr & LN_CERR) {
#if LNDEBUG
	    if (lndebug)
		printf("ln%d: collision (CERR)\n",unit);
#endif
	    if (sc->ctrblk.est_collis != 0xffff)
		sc->ctrblk.est_collis++;
	}
	if (oldcsr & LN_BABL) {
	    lnbablcnt++;
#if LNDEBUG
	    if (lndebug)
		printf("ln%d: transmitter timeout (BABL)\n",unit);
#endif
	}
	if (oldcsr & LN_MERR) {
	    lnmerrcnt++;
	    /* warn the user (printf on the terminal) */
	    printf("ln%d: memory error (MERR) \n", unit);
	    if (((oldcsr & LN_RXON) == 0)
		|| ((oldcsr & LN_TXON) == 0)) {
		lnrestart(ifp);
		lninit(ifp->if_unit);
		return;
	    }
	}
    }

    /*
     * We got a receive interrupt - process it
     */
    if ( oldcsr & LN_RINT )
	lnrint( unit );
	
    /* check for lance dma memory read error, happens on xmit only 
     * disables DMA, times out lance, interrupts processor
     */
    if (sc->lnsw->ln_dma) {
	vm_offset_t pa, pa1;
	struct tc_memerr_status status;

	if (LNREGRD32(sc->siraddr) & BIT16SET) { 
#ifdef __alpha
	    pa1 = (vm_offset_t)(LNREGRD32(sc->ldpaddr));
	    pa = (vm_offset_t)((pa1 & 0x01f) << 29L);/* bits 4:0->33:29 */
	    pa |= (vm_offset_t)((pa1 & ~0x01f) >> 3);/* bits 31:5->28:2 */
#else
	    pa = (vm_offset_t)(LNREGRD32(sc->ldpaddr) >> 3);
#endif
	    printf("ln%d: dma memory read error\n", unit);

	    status.pa = (caddr_t) pa;
	    status.va = (caddr_t) 0;	/* 0 means "don't know" */
	    status.log = TC_LOG_MEMERR;
	    status.blocksize = 16;	/* 16-byte chunks */
	    status.errtype = 0;		/* status filled in by routine */
	    tc_isolate_memerr(&status);

	    LNREGWR32(sc->siraddr, LNREGRD32(sc->siraddr) & ~BIT16SET);
	    LN_ENABLE_DMA(sc);		/* enable lance DMA in IOASIC */

	    /* in case this error was caused by referencing the currently
	     * tranmitting packet, get rid of it so lnrestart() won't
	     * requeue it for transmission.
	     */
	    if(sc->tmbuf[sc->otindex]) {
		m_freem(sc->tmbuf[sc->otindex]);
		sc->tmbuf[sc->otindex] = (struct mbuf *)NULL;
	    }
	    lnrestart(ifp);
	    lninit(ifp->if_unit);
	    lndmareaderr++;
	    return;
	}
    }

    /*
     * If it is a transmit interrupt call the
     * transmit interrupt routine.
     */
    if ( oldcsr & LN_TINT ) {
	lntint( unit );
    }

    /*
     * If it was neither it was a stray interrupt.
     */
    if (oldcsr == (LN_INTR|LN_RXON|LN_TXON))
	printf("ln%d: stray interrupt\n",unit); 

    LNREGWR16(sc->rdpaddr, LN_INEA);	/* set interrupt enable */
    LNIOSYNC();				/* flush to TURBOchannel */

}


lntint(unit)
/*
 * Ethernet interface transmit interrupt. This deals with
 * a transmit interrupt. We get this after we have finished
 * transmitting a packet.
 * 
 * Arguments:
 *	unit		The unit on which the transmit interrupt was rcvd.
 *
 * Return Value:
 *	None.
 */
int unit;
{
    register struct ln_softc *sc = ln_softc[unit];
    register int index, tlen;
    register struct ln_ring *rp = &sc->ln_ring;
    register struct lnsw *lnsw = sc->lnsw;

    struct mbuf *mp;
    struct ifnet *ifp = &sc->is_if;
    struct ether_header *eh;
#if LNDEBUG
if (lndebug > 2)printf("X: ");
#endif

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
    lnsw->ln_cpyin(sc, sc->tring[sc->otindex], &rp->ln_flag, sizeof(u_char),
	(u_long)((vm_offset_t)&rp->ln_flag - (vm_offset_t)rp));
    while ( (sc->otindex != sc->tindex)
	   && !(rp->ln_flag & LN_OWN)
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
	if (!(--sc->nxmit)) {
	    ifp->if_timer = 0;
	    ifp->if_flags &= ~IFF_OACTIVE;
	}
	sc->is_if.if_opackets++;
	
	lnsw->ln_cpyin(sc, sc->tring[sc->otindex],&rp->ln_flag2,sizeof(u_short),
	    (u_long)((vm_offset_t)&rp->ln_flag2 - (vm_offset_t)rp));
	/*
	 * Update statistics - also used by DECnet.
	 */

	/* exactly one collision? */
	if ((rp->ln_flag & LN_ONE) && !(rp->ln_flag2 & LN_LCOL)) {
		sc->is_if.if_collisions++;
		if (sc->ctrblk.est_single != (unsigned)0xffffffff)
			sc->ctrblk.est_single++;
	/* more than one collision? */
	} else if (rp->ln_flag & LN_MORE) {
		sc->is_if.if_collisions += 2;
		if (sc->ctrblk.est_multiple != (unsigned)0xffffffff)
			sc->ctrblk.est_multiple++;
	}
	if (rp->ln_flag & LN_DEF) {
		if (sc->ctrblk.est_deferred != (unsigned)0xffffffff)
			sc->ctrblk.est_deferred++;
	}

	/*
	 * Check for transmission errors.
	 * This assumes that transmission errors
	 * are always reported in the last packet.
	 */
	if (rp->ln_flag & LN_RT_ERR) {
	    sc->is_if.if_oerrors++;
	    if (sc->ctrblk.est_sendfail != 0xffff) {
		sc->ctrblk.est_sendfail++;
		if (rp->ln_flag2 & LN_RTRY) {
		    /* excessive collisions */
		    /* If the TDR is 0, assume it's just busy */
		    if (rp->ln_flag2 & LN_TDR) {
			register int tdr = (rp->ln_flag2 & LN_TDR);
			/* The TDR value ticks at 10MHz.
			 * Convert to distance using velocity
		         * of propagation using
		         * Distance = 1/2 x v x (ticks/freq).
		         * So thick 78% => 11.7m/tick
			 * thin  66% =>  9.9m/tick 
                         */
			tdr *= 100;
			if (lndebug)
			    printf("Ethernet: Warning: Excessive collisions on unit %d: TDR %dns (%d-%dm), Check transceiver\n", unit, tdr, (tdr*99)/1000, (tdr*117)/1000);
		    } else {
#if LNDEBUG
			if (lndebug)
			    printf("ln: packet not sent (RTRY)\n");
#endif
		    }
		}
		sc->ctrblk.est_sendfail_bm |= 1;

		if (rp->ln_flag2 & LN_LCOL) {
#if LNDEBUG
		    if (lndebug) 
			printf("ln%d: late transmit collision (LCOL)\n",unit);
#endif
		    ; /* not implemented */
		}
		if (rp->ln_flag2 & LN_LCAR) {
                    lncarcnt++;
                  if (lncarcnt == 1 || lncarcnt % 500)
                    printf("ln%d: lost carrier: check connector\n", unit);
#if LNDEBUG
		    if(lndebug) 
			printf("ln%d: lost carrier during transmit (LCAR)\n",unit);
#endif
		    sc->ctrblk.est_sendfail_bm |= 2;
		}
		if (rp->ln_flag2 & LN_UFLO) {
#if LNDEBUG
		    if (lndebug) 
			printf("ln%d: packet truncated (UFLO)\n,unit");
#endif
		}
		if (rp->ln_flag2 & LN_TBUFF) {
#if LNDEBUG
		    if (lndebug) 
			printf("ln%d: transmit buffer error (BUFF)\n,unit");
#endif
		}
	    }
	    /*
	     * Restart chip if transmitter got turned off
	     * due to transmit errors: UFLO, TBUFF or RTRY.
             */
	    if (rp->ln_flag2 & (LN_UFLO | LN_TBUFF | LN_RTRY)) {
		/*
		 * Need to free mp here, since we've
		 * already destroyed its copy in the
		 * "tmbuf" list.
		 */
		m_freem(mp);
		lnrestart(ifp);
		lninit(ifp->if_unit);
		return;
	    }
	} else {
		/*
		 * Accumulate statistics used by DECnet
		 */
		if (mp && (mp->m_flags & M_PKTHDR) ) { 
			tlen = mp->m_pkthdr.len;
			if ((sc->ctrblk.est_bytesent + tlen) > sc->ctrblk.est_bytesent)
				sc->ctrblk.est_bytesent += tlen;
			if (sc->ctrblk.est_bloksent != (unsigned)0xffffffff)
				sc->ctrblk.est_bloksent++;
			eh = mtod ( mp, struct ether_header *);
			if(eh->ether_dhost[0] & 1) {
				sc->ctrblk.est_mbytesent += tlen;
                        	if (sc->ctrblk.est_mbloksent != (unsigned) 0xffffffff) 
					sc->ctrblk.est_mbloksent++;
			}
		}
           /* once packet is sent, zero lncarcnt */
               lncarcnt = 0;
	}

  	m_freem(mp);
	/*
	 * Init the buffer descriptor
	 */
	lninitdesc(sc, sc->tring[index], &sc->tlbuf[index], LNNOALLOC);
	LNIOSYNC();			/* make descriptor visible to chip */
	sc->otindex = ++index % nLNNXMT;

	/* for the next time thru while */
	lnsw->ln_cpyin(sc, sc->tring[sc->otindex], &rp->ln_flag, sizeof(u_char),
	    (u_long)((vm_offset_t)&rp->ln_flag - (vm_offset_t)rp));
    }
    /*
     * Dequeue next transmit request, if any.
     */
    if (!(ifp->if_flags & IFF_OACTIVE))
	lnstart( unit );
}


lnrint(unit)
/*
 * Ethernet interface receiver interrupt.
 * If can't determine length from type, then have to drop packet.  
 * Othewise decapsulate packet based on type and pass to type specific 
 * higher-level input routine.
 *
 * Arguments:
 *	unit		The unit number which got the interrupt.
 *
 * Return Value:
 *	None.
 */

int unit;
{
    register struct ln_softc *sc = ln_softc[unit];
    register struct ln_ring *rp = &sc->ln_ring;
    register struct lnsw *lnsw = sc->lnsw;
    register int index, len;
    register struct ifnet *ifp = &sc->is_if;
    int first;
    u_char flag;	/* saved status flag */
    int ring_cnt;

int foobar=0;if (lndebug > 2)printf("R: ");
#ifdef lint
	unit = unit;
#endif
    /*
     * Traverse the receive ring looking for packets to pass back.
     * The search is complete when we find a descriptor that is in
     * use (owned by Lance). 
     */

    lnsw->ln_cpyin(sc, sc->rring[sc->rindex], &rp->ln_flag, sizeof(u_char),
	(u_long)((vm_offset_t)&rp->ln_flag - (vm_offset_t)rp));

    for ( ; !(rp->ln_flag & LN_OWN);
	 sc->rindex = ++index % nLNNRCV, 
	 lnsw->ln_cpyin(sc, sc->rring[sc->rindex], &rp->ln_flag, sizeof(u_char),
	    (u_long)((vm_offset_t)&rp->ln_flag - (vm_offset_t)rp))) {
++foobar;

	first = index = sc->rindex;

	/*
	 * If not the start of a packet, error
	 */
	if ( !(rp->ln_flag & LN_STP)) {
#if LNDEBUG
	    if (lndebug) {
	       printf("ln%d: recv: start of packet expected #%d, flag=0x%02x\n",
		    unit, index, (rp->ln_flag&0xff));
	    }
#endif
	    LNIOSYNC();	/* flush out ring updates before ownership update */
	    lnsw->ln_setflag(sc->rring[first], LN_OWN);
	    lnrestart(ifp);
	    lninit(ifp->if_unit);
	    return;
	}

	/*
	 * Find the index of the last descriptor in this
	 * packet (LN_OWN clear and LN_ENP set). If cannot
	 * find it then Lance is still working.
	 */
	ring_cnt=1;
	while (((rp->ln_flag &
		 (LN_RT_ERR|LN_OWN|LN_ENP)) == 0) && (ring_cnt++ <= nLNNRCV)){
	    index = ++index % nLNNRCV;
	    lnsw->ln_cpyin(sc, sc->rring[index], &rp->ln_flag, sizeof(u_char),
		(u_long)((vm_offset_t)&rp->ln_flag - (vm_offset_t)rp));
	}

	/*
	 * more than one; re-init the descriptors involved and
	 * ignore this packet. (May have to bump up DECnet counters).
	 */
	if (ring_cnt > 1) {
	    /*
	     * Went all the way around, wait
	     */
	    if (ring_cnt > nLNNRCV) {
#if LNDEBUG
		if (lndebug)
		    printf("ln%d: recv ring_cnt exceeded\n",unit);
#endif
		break;
	    }

	    /*
	     * "free" up the descriptors, but really just use
	     * them over again.  Give them back to the Lance
	     * using the same local RAM buffers. IF ERROR INFO.
	     * IS SET, WAIT!
	     */
	    if (!(rp->ln_flag & LN_RT_ERR)) {
		LNIOSYNC();/* flush out ring updates before ownership update */
		lnsw->ln_setflag(sc->rring[first], LN_OWN);
		while ( first != index ) {
		    first = ++first % nLNNRCV;
		    lnsw->ln_setflag(sc->rring[first], LN_OWN);
		}
		LNIOSYNC();	/* let LANCE see ownership ASAP */
#if LNDEBUG
		if (lndebug)
		    printf("ln%d: chained receive packet dropped\n",unit);
#endif
		continue;
	    }
if (lndebug) printf("CNT > 1\n");
	}

	/*
	 * Is it a valid descriptor, ie.
	 * we own the descriptor (LN_OWN==0),
	 * and end of packet (ENP) set?
	 */
	if ( (rp->ln_flag & ~LN_STP) == LN_ENP) {
	    lnsw->ln_cpyin(sc, sc->rring[index], &rp->ln_flag2, sizeof(u_short),
		(u_long)((vm_offset_t)&rp->ln_flag2 - (vm_offset_t)rp));
	    /* len here includes the 4 CRC bytes */
	    len = (rp->ln_flag2 & LN_MCNT);
	    lnread(sc, sc->rlbuf[index], len-4, index);
if (lndebug > 2)printf("%d.\n",len);

	    /*
	     * If we're on an architecture which provides DMA,
             * re-initialize the descriptor to get a new buf.
	     */
	    if (lnsw->ln_dodma)
		lnsw->lninitdesc (sc, sc->rring[index],
				  &sc->rlbuf[index], LNNOALLOC,index);

	    /*
	     * "free" up the descriptors. Turn ownership back
	     * back to LANCE.
             */
	    LNIOSYNC();			/* guarantee write ordering */
	    lnsw->ln_setflag(sc->rring[first], LN_OWN);
	    LNIOSYNC();			/* let LANCE see ownership ASAP */

	/*
	 * else Not a good packet sequence, check for receiver errors
	 */
	} else if ((flag = rp->ln_flag) & LN_RT_ERR){
	    sc->is_if.if_ierrors++;
	    
	    if (flag & (LN_OFLO | LN_CRC | LN_FRAM | LN_RBUFF)) {
#if LNDEBUG
		if (lndebug)
		    printf("ln%d: recv err 0x%02x\n", unit, flag&0xff);
#endif
		if (sc->ctrblk.est_recvfail != 0xffff) {
		    sc->ctrblk.est_recvfail++;
		    if (flag & LN_OFLO) {
			sc->ctrblk.est_recvfail_bm |= 4;
			sc->is_ed.ess_missed++;
		    }
		    if (flag & LN_CRC) {
			sc->ctrblk.est_recvfail_bm |= 1;
		    }
		    if (flag & LN_FRAM) {
			sc->ctrblk.est_recvfail_bm |= 2;
		    }
		    if (flag & LN_RBUFF) {
			; /* causes LN_OFLO to be set */
		    }
		}
	    } else {
#if LNDEBUG
		if (lndebug)
		    printf("ln%d: stray recv err 0x%02x\n",unit,flag&0xff);
#endif
	    }
	    /*
	     * "free" up the descriptors, but really just use
	     * them over again.  Give them back to the Lance
	     * using the same pre-allocated mbufs.
	     */
	    LNIOSYNC();	/* flush out ring updates before ownership update */
	    lnsw->ln_setflag(sc->rring[first], LN_OWN);
	    while ( first != index ) {
		first = ++first % nLNNRCV;
		/* give away */
		lnsw->ln_setflag(sc->rring[first], LN_OWN);
	    }
	    LNIOSYNC();	/* let LANCE see ownership ASAP */
	} else {
if (lndebug) printf("<TOO FAST>\n");
	    /*  else:
	     *   - not a good packet, 
	     *   - not an error situation,
	     *
	     * This can happen if we beat the Lance to the
	     * end of a valid list of receive buffers -- the
             * Lance hasn't relinquished the last buffer (one
	     * with ENP set) or we just found a buffer still
	     * owned by Lance, without finding the ENP bit.
             * In either case, just return.  We can pick up
	     * the unfinished chain on the next interrupt.
	     */
	    return;
	}
    }
if (lndebug > 2)if (! foobar)printf("  NONE\n");
}


lnoutput(ifp)
/*
 * Ethernet output routine. 
 * This routine calls lnstart to actually start the output.
 * 
 * Arguments:
 *	ifp		The ifnet pointer, pointing to the intf. to o/p on.
 *
 * Return Value:
 * 	None.
 */
register struct ifnet *ifp;
{
    lnstart(ifp->if_unit);
}


lnstart(unit)
/*
 * This starts output on the interface.
 * 
 * Argument:
 * 	unit		The unit number to send output on.
 *
 * Return Value:
 *	None.
 */
int	unit;
{
    register struct ln_softc *sc = ln_softc[unit];
    register struct mbuf *m;
    register int dp; 	/* data buffer pointer */
    register struct ln_ring *rp = &sc->ln_ring;
    register struct lnsw *lnsw = sc->lnsw;
    int tlen;
    int index, s;
    register struct ifnet *ifp = &sc->is_if;

    s = splimp();
    for (index = sc->tindex;
	 sc->nxmit < (nLNNXMT - 1);		/* TODO - WHY "-1" */
	 sc->tindex = index = ++index % nLNNXMT) {
	
	/*
	 * Dequeue the transmit request, if any.
	 */
	IF_DEQUEUE(&sc->is_if.if_snd, m);
	if (m == 0) {
	    break;		/* Nothing on the queue	*/
	    
	}
	
	/*
	 * "lnput" pads out anything less than "MINDATA" with NULL's
	 */
	tlen = lnput(sc, index, m);
	LNIOSYNC();			/* make DMA buffer visible to chip */
	
	dp = lnsw->ln_svtolance(sc, sc->tlbuf[index]);
#ifdef mips
	/*
	 * A very low ratio timing problem is happened on the 3max system 
	 * while updating the transmit descriptor on the lance. There is 
	 * a tiny window between copy the first long word and the
	 * second one to the lance. Because the first long word of the 
	 * descriptor includes the ownership bit, the lance could do the
	 * transmit polling within that tiny window. So, the lance
         * finds the host turn on the ownership bit and starts to transmit.
	 * But, the second long word which includes the byte counts
	 * has not been updated yet. For this reason, the lance uses the
	 * wrong length to transmit (1518 byte in lninitdesc routine)   
 	 *
	 * The fix is to update the ownership after write the byte
	 * conter to the adapter
         */
	rp->ln_flag = 0;
#else
	rp->ln_flag = ( LN_STP /**| LN_OWN**/ | LN_ENP );
#endif
	rp->ln_buf_len = -(tlen) | 0xf000;
	rp->ln_addr_lo = (short)dp & 0xffff;
	rp->ln_addr_hi = (short)(((int)dp >> 16) & 0xff);
	lnsw->ln_cpyout(rp,sc->tring[index],sizeof(struct ln_ring),0);

	LNIOSYNC();			/* make ring update visible to chip */
#ifdef mips
	lnsw->ln_setflag(sc->tring[index],( LN_STP | LN_ENP | LN_OWN ));
#else
	rp->ln_flag |= LN_OWN;
	lnsw->ln_setflag(sc->tring[index], (u_int)rp->ln_flag);
#endif

	LNREGWR16(sc->rapaddr, LN_CSR0);
	LNREGWR16(sc->rdpaddr, ( LN_TDMD | LN_INEA ));
	wbflush();

	sc->nxmit++;
	ifp->if_flags |= IFF_OACTIVE;
	sc->is_if.if_timer = 5;
    }
    splx(s);
}


lnioctl(ifp, cmd, data)
/*
 * This processes an ioctl request. In doing so, the interface is
 * restarted and reinitialized.
 *
 * Arguments:
 *	ifp		Pointer to the ifnet structure
 *	cmd		The ioctl to be performed
 *	data		The corresponding data for the ioctl.
 *
 */

	register struct ifnet *ifp;
	unsigned int cmd;
	caddr_t data;
{
	int ln_docrc();

	register struct ln_softc *sc = ln_softc[ifp->if_unit];
	register struct ln_initb *initb = &sc->ln_initb;
	register int i;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	struct ctrreq *ctr = (struct ctrreq *)data;
	/*struct protosw *pr;*/
	struct ifaddr *ifa = (struct ifaddr *)data;
	int bitpos;		/*top 6 bits of crc = bit in multicast mask */
	u_short newmask[4];	/*new value of multicast address mask */
	int j = -1, s, error=0;

	s = splimp();
	switch (cmd) {

	 case SIOCENABLBACK:
#if LNDEBUG
	    if (lndebug>1) printf("SIOCENABLBACK ");
#endif
	    printf("ln%d: internal loopback requested\n", ifp->if_unit);
	    
	    /* set external loopback */	
	    initb->ln_mode &= ~LN_INTL;
	    initb->ln_mode |= LN_LOOP;
	    lnrestart( ifp );
	    ifp->if_flags |= IFF_LOOPBACK;
	    lninit( ifp->if_unit );
	    break;
 
	case SIOCDISABLBACK:
#if LNDEBUG
	    if (lndebug>1) printf("SIOCDISABLBACK ");
#endif
	    printf("ln%d: internal loopback disable requested\n", ifp->if_unit);

	    /* disable external loopback */
	    initb->ln_mode &= ~LN_INTL;
	    initb->ln_mode &= ~LN_LOOP;
	    lnrestart( ifp );
	    ifp->if_flags &= ~IFF_LOOPBACK;
	    lninit( ifp->if_unit );
	    break;
 
	case SIOCRPHYSADDR:
	    /*
	     * read default hardware address.
	     */
#if LNDEBUG
	    if (lndebug>1) printf("SIOCRPHYSADDR ");
#endif
	    bcopy(sc->is_addr, ifd->current_pa, 6);
	    if (sc->ln_is_depca)
	        for ( i=0 ; i<6 ; i++ )
		    ifd->default_pa[i] = sc->is_int_addr[i];
	    else
	        for ( i=0 ; i<6 ; i++ )
		    ifd->default_pa[i] =
		        (u_char)((sc->ln_narom[i]>>sc->lnsw->ln_na_align)&0xff);

	    break;
 
	case SIOCSPHYSADDR:
#if LNDEBUG
	    if (lndebug>1) printf("SIOCSPHYSADDR ");
#endif
	    bcopy(ifr->ifr_addr.sa_data, sc->is_addr, 6);
	    pfilt_newaddress(sc->is_ed.ess_enetunit, sc->is_addr);

	    /*
	     * fill in the initb station address, and restart.
	     */
	    for (i=j=0; i < 3; i++, j += 2) {
		initb->ln_sta_addr[i] = 
		    (short)(sc->is_addr[j]&0xff);
		initb->ln_sta_addr[i] |= 
		    (short)((sc->is_addr[j+1]&0xff)<<8);
	    }

	    if (ifp->if_flags & IFF_RUNNING) {
		lnrestart(ifp);
		lninit(ifp->if_unit);
	    } else {
		lnrestart(ifp);
	    }
	    /* If an IP address has been configured then an ARP packet
	     * must be broadcast to tell all hosts which currently have
	     * our address in their ARP tables to update their information.
	     */
	    if(((struct arpcom *)ifp)->ac_ipaddr.s_addr)
		    rearpwhohas((struct arpcom *)ifp,
			  &((struct arpcom *)ifp)->ac_ipaddr);
	    break;

	case SIOCDELMULTI:			/* issue: if ln is in ALLMULTI mode, */
	case SIOCADDMULTI:			/* don't change the ln_multi_mask[] */
								/* array elements (which are all set */
								/* to ~0 by IFF_ALLMULTI), and don't */
								/* call lnrestart() and/or lninit(), */
								/* which reset the chip and undo the */
								/* ALLMULTI mode -bg */
	    if (cmd == SIOCDELMULTI) {
#if LNDEBUG
		if (lndebug>1) printf("SIOCDELMULTI ");
#endif
		for (i = 0; i < sc->nmulti; i++)
		    if (bcmp(&sc->multi[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
			if (--sc->muse[i] == 0) {
			    bcopy(ln_sunused_multi,&sc->multi[i],MULTISIZE);
			}
			if (lnshowmulti) printf("ln%d: %d deleted.\n", ifp->if_unit, i);
			
		    }
	    } else {				/* SIOCADDMULTI */
#if LNDEBUG
		if (lndebug>1) printf("SIOCADDMULTI ");
#endif
		for (i = 0; i < sc->nmulti; i++) {
		    if (bcmp(&sc->multi[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
			sc->muse[i]++;
			if (lnshowmulti) printf("ln%d: already using index %d\n", ifp->if_unit, i);
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
		    if (sc->nmulti == nLNMULTI) {
			printf("ln%d: SIOCADDMULTI failed, multicast list full: %d\n", ifp->if_unit, nLNMULTI);
			error = ENOBUFS;
			goto done;
		    } else {
			j = sc->nmulti++;
		    }

		}
		bcopy(ifr->ifr_addr.sa_data, &sc->multi[j], MULTISIZE);
		sc->muse[j]++;
		
		if (lnshowmulti)
		    printf("ln%d: added index %d.\n", ifp->if_unit, j);
	    }
		if (ifp->if_flags & IFF_ALLMULTI) break; /* do nothing more if */
												 /* IFF_ALLMULTI is set */
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
	    
	    for (i=0; i<sc->nmulti; i++) {
		if (sc->muse[i] == 0)
		    continue;
		/* returns 32-bit crc in global variable _ln_crc */
		ln_docrc(&sc->multi[i], 0, sc);
		bitpos = ((unsigned int)sc->ln_crc >> 26) & 0x3f;
		if (lnshowmulti)
		    printf("ln%d: crc=0x%x, bit=%d.\n", ifp->if_unit, sc->ln_crc,bitpos);
		
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
			printf("ln%d: bad crc, bitpos=%d.\n", ifp->if_unit, bitpos);
		}
	    }
	    i = 0;
	    for (i = 0; i < 4; i++)
		initb->ln_multi_mask[i] = newmask[i] & 0xffff;
	    if (lnshowmulti) {
		printf("ln%d: new 64-bit multimask= 0x%04x 0x%04x 0x%04x 0x%04x\n",
		       ifp->if_unit, newmask[0], newmask[1], newmask[2], newmask[3]);
	    }

	    if (ifp->if_flags & IFF_RUNNING) {
		lnrestart(ifp);
		lninit(ifp->if_unit);
	    } else {
		lnrestart(ifp);
	    }
	    break;

	case SIOCRDCTRS:
	case SIOCRDZCTRS:

#if LNDEBUG
	    if (lndebug>1) printf("SIOCRDCTRS ");
#endif
	    ctr->ctr_ether = sc->ctrblk;
	    ctr->ctr_type = CTR_ETHER;
	    ctr->ctr_ether.est_seconds = (time.tv_sec - sc->ztime) > 0xfffe ? 0xffff : (time.tv_sec - sc->ztime);
	    if (cmd == SIOCRDZCTRS) {
#if LNDEBUG
		if (lndebug>1) printf("SIOCRDZCTRS ");
#endif
		sc->ztime = time.tv_sec;
		bzero(&sc->ctrblk, sizeof(struct estat));
	    }
	    break;

	case SIOCSIFADDR:
#if LNDEBUG
	    if (lndebug>1) printf("SIOCSIFADDR ");
#endif
	    if (!(ifp->if_flags & IFF_RUNNING)) {
		lninit(ifp->if_unit);
		ifp->if_flags |= IFF_UP;
	    }
	    switch(ifa->ifa_addr->sa_family) {
	    case AF_INET:
		((struct arpcom *)ifp)->ac_ipaddr =
		    IA_SIN(ifa)->sin_addr;
		break;

	    default:
		/*if (pr=iffamily_to_proto(ifa->ifa_addr.sa_family)) {
		    error = (*pr->pr_ifioctl)(ifp, cmd, data);
		} */
#if LNDEBUG
		if (lndebug > 1)
		    printf("ln%d: Unknown Protocol received\n", ifp->if_unit);
#endif
		break;
	    }
	    break;
        case SIOCSIFFLAGS:
#if LNDEBUG
	    if (lndebug > 1) printf("SIOCSIFFLAGS ");
#endif
	    /*
	     * If promisuous mode is enabled/disabled restart
	     * to change LANCE mode
	     */
	    /* smp_lock(&sc->lk_ln_softc, LK_RETRY);*/
	    if ((ifp->if_flags & IFF_PROMISC) &&
		!(initb->ln_mode & LN_PROM)) {
		
		/* should be in promiscuous mode */
		initb->ln_mode |= LN_PROM;
	    } else if (!(ifp->if_flags & IFF_PROMISC)) {

		/* should not be in promiscuous mode */
		initb->ln_mode &= ~LN_PROM;
	    } 
	    /*
	    **  are we being asked to deal with all multicast packets
	    **  in some way or other? (i.e.enable or disable)
	    */
	    if (ifr->ifr_flags & IFF_ALLMULTI)   /* enable IFF_ALLMULTI */
	    {
		/*
		**  don't bother checking to see if we're already
		**  there...just do it.
		*/
		int     ix;

#if LNDEBUG
		if (lndebug) printf ("ln%d: setting all multicast bits\n", ifp->if_unit);
#endif
		for (ix = 0; ix < 4; ix++)
		{
#if LNDEBUG
		    if (lndebug) printf ("ln_multi_mask [%d] = 0x%x\n",
		        ix, initb->ln_multi_mask[ix]);
#endif
		    initb->ln_multi_mask [ix] = ~0; /* set ALL them bits */
		}
		ifp->if_flags |= IFF_ALLMULTI; /* this is redundant */
	    }
	    else	                        /* disable IFF_ALLMULTI */
	    {
#if LNDEBUBG
		if (lndebug) printf ("\trebuild multicast bit array\n");
#endif
            {
                /*
                **  rebuild the lance's multicast mask based
                **  on the multicast addresses stashed in
                **  the sc structure...
                */
                int ix;
                int bp, ndx;

                /*
                **  initialize temporary mask bits to all zeros
                */
                for (ix = 0; ix < 4; ix++)
                {
                    newmask [ix] = 0;
                }
                for  (ix = 0; ix < sc->nmulti; ix++)
                {
                    if (sc->muse[ix] == 0) continue;
                    ln_docrc (&sc->multi[ix], 0, sc);
                    bitpos = ((unsigned int)sc->ln_crc >> 26) & 0x3f;
                    if (lnshowmulti)
                        printf ("crc=0x%x, bit=%d.\n",sc->ln_crc,bitpos);
                    if (bitpos >= 64 || bitpos < 0)
                    {
                        if (lndebug || lnshowmulti)
                        printf("ln: bad crc, bitpos=%d.\n", bitpos);
                        continue;
                    }
                    /* don't break the pipeline with a bunch of if(...);
                     * else(...) conditionals
                     */
                    bp = bitpos & 15;   /* extract the 0..15 bit pos*/
                    ndx = (bitpos & ~15) >> 4; /* get index to mask array */
                    newmask[ndx] |= (1 << bp);
                }
                for (ix = 0; ix < 4; ix++)
                {
                    initb->ln_multi_mask [ix] = newmask [ix] & 0xffff;
                }
                if (lnshowmulti)
                {
                printf("new 64-bit multimask= 0x%04x 0x%04x 0x%04x 0x%04x\n",
                       newmask[0], newmask[1], newmask[2], newmask[3]);
                }
            }
        }
	    /* smp_unlock(&sc->lk_ln_softc); */
	    if (ifp->if_flags & IFF_RUNNING) {
		lnrestart(ifp);
		lninit(ifp->if_unit);
	    } else {
		lnrestart(ifp);
	    }
	    break;

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



static
ln_docrc(addr, flag, sc)
/*
 * Calculate 32-bit CRC (AUTODIN-II) for the given 48-bit
 * multicast address.  The CRC table must first be initialized
 * for the vax crc instruction to work.  The crc is returned in
 * variable ln_crc.
 * 
 * Arguments:
 *	addr		
 *	flag		The interface flags
 *	sc		The softc structure for the interface.
 *
 * Return Value:
 * 	NULL.
 */
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
    register int inicrc;		/* initial CRC */
    register int len;			/* length of multicast addr */
    register struct ln_multi *multi_addr;/* multicast address, 48 bits */
    register int *ln_crc = &sc->ln_crc;

    /* initialize polynomial table, only done once from lnprobe() */

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
/*
 * This function calculates the CRC and returns the value. 
 *
 * Arguments:
 *	addr		
 *	len		
 *
 * Return Value:
 *	crc
 */
u_char *addr;
int len;
{
    register unsigned int crc;
    
    crc = (u_int)0xffffffff;
    while(len--)
	crc = (crc >> 8) ^ crctbl[*addr++] ^ crctbl[crc&0xff];
    
    return(crc);
}


lnrestart( ifp )
/*
 * Restart the Lance chip -- this routine is called:
 *   - after changing the multicast address filter mask
 *   - on any loopback mode state change
 *   - on any error which disables the transmitter
 *   - on all missed packet errors
 *
 * The routine first halts the Lance chip, clears any previously
 * allocated mbufs, and then re-initializes the hardware (same as in
 * lnprobe() routine).  The lninit() routine is usually called next
 * to reallocate mbufs for the receive ring, and then actually
 * start the chip running again.
 *
 * Arguments:
 *	ifp		A pointer to the ifnet structure whose interface
 *			needs to be (re)started.
 *
 * Return Value:
 *	NULL.
 */
register struct ifnet *ifp;
{
    register struct ln_softc *sc = ln_softc[ifp->if_unit];
    register char *initb = sc->initbaddr;
    register int i, pi;
    register unsigned int index;
    int s;

    /*
     * stop the chip
     */
    s = splimp();

    /* disable DMA before setting STOP bit in case a
     * DMA transaction had been started.
     */
    if (sc->lnsw->ln_dma)
	LN_DISABLE_DMA(sc);		/* disable lance DMA in IOASIC */
    
    netnuke(ifp->if_unit);		/* physically stop the chip */

    if (sc->lnsw->ln_dma)
	LN_ENABLE_DMA(sc);		/* enable lance DMA in IOASIC */

    /*
     * stop network activity
     */
    ifp->if_flags &= ~IFF_RUNNING;
    lnrestarts++;

#if LNDEBUG
    if (lndebug)
	printf("lnrestart: restarted ln%d  %d\n",ifp->if_unit,lnrestarts);
#endif

    /* re-queue any packets that have not been transmitted.
     * Do this because normal operations such as setting the station or
     * multicast addresses will reset the chip.  This prevents 
     * untransmitted packets from being unecessarily discarded.
     * The ordering of the packets is maintained (as if this is important).
     * This fixes QAR 9217.
     */
    index = (unsigned int)sc->tindex;
    IFQ_LOCK(&sc->is_if.if_snd);
    while (index != (unsigned int)sc->otindex) {
	/* decrement first since tindex points to the next FREE slot */
	index--;
	index %= nLNNXMT;
	if(sc->tmbuf[index]) {
	    IF_PREPEND_NOLOCK(&sc->is_if.if_snd,sc->tmbuf[index]);
	    sc->tmbuf[index] = (struct mbuf *)NULL;
	}
    }
    IFQ_UNLOCK(&sc->is_if.if_snd);

    /*
     * free up any mbufs currently in use
     */
    for (i=0; i<nLNNXMT; i++) {
	if (sc->tmbuf[i])
	    m_freem(sc->tmbuf[i]);
	sc->tmbuf[i] = (struct mbuf *)NULL;
    }

    if ( sc->lnsw->ln_dodma ) {
	/*
	 * free up any mbufs currently in use
	 */
	for (i=0; i<nLNNRCV; i++) {
	    if (sc->rmbuf[i])
		m_freem(sc->rmbuf[i]);
	    sc->rmbuf[i] = (struct mbuf *)NULL;
	}
    }

    /*
     * reload Lance with init block
     */
    sc->lnsw->ln_cpyout(&sc->ln_initb,initb,sizeof(struct ln_initb),0);

    LNREGWR16(sc->rapaddr, LN_CSR1);
    pi = sc->lnsw->ln_svtolance(sc, initb);
    LNREGWR16(sc->rdpaddr, ((u_short)(pi & 0xffff)));

    LNREGWR16(sc->rapaddr, LN_CSR2);
    LNREGWR16(sc->rdpaddr, ((u_short)(((int)pi>>16) & 0xff)));

    if (sc->ln_is_depca) {
        LNREGWR16(sc->rapaddr, LN_CSR3);
        LNREGWR16(sc->rdpaddr, LN_ACON);
    }
    /*
     * clear IDON, and INIT the chip
     */
    LNREGWR16(sc->rapaddr, LN_CSR0);
    LNREGWR16(sc->rdpaddr, (LN_IDON | LN_INIT /***| LN_INEA***/));
    LNIOSYNC();		/* guarantee write preceeds polling below */

    i = 0;
    while (i++ < 100) {
	if ((LNREGRD16(sc->rdpaddr) & LN_IDON) != 0) {
	    break;
	}
	DELAY(10000);		/* this must not sleep */
    }

    /* make sure got out okay */
    if ((LNREGRD16(sc->rdpaddr) & LN_IDON) == 0) {
	if (LNREGRD16(sc->rdpaddr) & LN_ERR)
	    printf("lnrestart: initialization ln%d ERR csr0=0x%04x\n",
		   ifp->if_unit,LNREGRD16(sc->rdpaddr));
	else
	    printf("lnrestart: cannot initialize ln%d\n",ifp->if_unit);
    }

    if (sc->lnsw->ln_dma)
	LN_DISABLE_DMA(sc);		/* disable lance DMA in IOASIC */

    /* set STOP to clear INIT and IDON */
    LNREGWR16(sc->rdpaddr, LN_STOP);

    if (sc->lnsw->ln_dma)
	LN_ENABLE_DMA(sc);		/* reenable lance DMA in IOASIC */
	
    wbflush();

    splx(s);
    
    return(0);
}


lninitdesc (sc, desc, buf, option)
/*
 * Initialize a ring descriptor. Tie the buffer pointed to by
 * "dp" to the descriptor.
 * 
 * Arguments:
 *	sc		The softc structure pertaining to the interface.
 *	desc		Pointer to the ring.
 *	buf		Pointer to the buffer to be allocated.
 *	option		If buffer space needs to be allocated, this
 *			has a value of LNALLOC.
 *
 * Return Value:
 *	Failure:	-1
 *	Success: 	NULL
 */
register struct ln_softc *sc;
register struct ln_ring *desc;
register char **buf;
int option;
{
    register int dp;		/* data pointer */
    register struct lnsw *lnsw = sc->lnsw;
    struct ln_ring *ring = &sc->ln_ring;
    int align;

    align = (sc->ln_is_depca) ? 1 : 0;

    /*
     * clear the entire descriptor
     */
    lnsw->ln_bzero(desc, sizeof(struct ln_ring));
    
    /*
     * Perform the necessary allocation/deallocation
     */
    if (option == LNALLOC) {
	*buf = lnsw->ln_allocb (sc, LN_BUF_SIZE, align);
	if ( *buf == 0 ) {
#if LNDEBUG
	    if (lndebug)
		printf("ln%d: can't alloc space for buffers\n",
		    sc->is_if.if_unit);
#endif
	    return(-1);
	}
    }
    dp = lnsw->ln_svtolance(sc, *buf);
    /* +4 for CRC */
    ring->ln_flag = 0;
    ring->ln_flag2 =0;
    ring->ln_buf_len = -(ETHERMTU + sizeof(struct ether_header) + 4);
    ring->ln_addr_lo = (short)dp & 0xffff;
    ring->ln_addr_hi =
	(short)(((int)dp >> 16) & 0xff);
    lnsw->ln_cpyout(ring,desc,sizeof(struct ln_ring),0);
    return(0);
}


lnput(sc, index, m)
/*
 * This puts an mbuf chain into the appropriate RAM buffer.
 *
 * Arguments:
 *	sc		The softc structure for the interface.
 *	index		The index of the buffer
 *	m		The mbuf chain to be copied to the RAM buffer.
 *
 * Return Value:
 *	len		The total length copied.
 */
struct ln_softc *sc;
int index;
register struct mbuf *m;
{
    register caddr_t dp;
    register caddr_t lrbptr = sc->tlbuf[index];
    register struct lnsw *lnsw = sc->lnsw;
    int len = 0;
	
    sc->tmbuf[index] = m;	/* mbuf chain to free on xmit */
    while (m) {
	if (m->m_len == 0)
	    goto next_m;
	dp = mtod(m, char *);
	lrbptr = lnsw->ln_cpyoutb(dp,lrbptr,(unsigned)m->m_len,0);
next_m:	
	len += m->m_len;
	m = m->m_next;
    }

    if (len < MINDATA) {
	lnsw->ln_bzerob(lrbptr, MINDATA - len);
	len = MINDATA;
    }
    return (len);
}


struct mbuf *
lnget(sc,rlbuf, totlen, off, index)
/*
 * Pull read data off an interface.
 * Len is length of data, with local net header stripped.
 * Off is non-zero if a trailer protocol was used, and
 * gives the offset of the trailer information.
 * We first copy all the normal data into a cluster then deal 
 * with trailer. We deal with the trailer header in the lnread()
 * routine.
 */
struct ln_softc *sc;
caddr_t rlbuf;
int totlen;
register int off;
long index;
{
    struct mbuf *top, **mp, *m;
    int len;
    register caddr_t lrbptr = rlbuf;
    register struct lnsw *lnsw = sc->lnsw;

    top = 0;
    mp = &top;
   
    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == 0) {
if (lndebug) printf("NO MGETHDR ");
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
	MCLGET2(m, M_DONTWAIT);
	if ((m->m_flags & M_EXT) == 0) {
if (lndebug) printf("NO MCLGET2 ");
	    return(0);
	}
	len = MIN(off, MCL2KBYTES);
	m->m_len = len;
	lrbptr = lnsw->ln_cpyinb(sc, lrbptr,mtod(m,caddr_t),(u_long)len,0L);
	totlen -= len;
	*mp = m;
	mp = &m->m_next;
    } 
	
    while (totlen > 0) {
	if (top) {
	    MGET(m, M_DONTWAIT, MT_DATA);
	    if (m == 0) {
		m_freem(top);
if (lndebug) printf("NO MGET ");
		return(0);
	    }
	    m->m_len = MLEN;
	}
	len = totlen;
	
	if (len > MINCLSIZE) {
	    MCLGET2(m, M_DONTWAIT);
	    if (m->m_flags & M_EXT)
	    	m->m_len = MCL2KBYTES;
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

	lrbptr = lnsw->ln_cpyinb(sc, lrbptr,
				 mtod(m, caddr_t),(u_long)m->m_len, 0L);
	*mp = m;	
	mp = &m->m_next;
	totlen -= m->m_len;
    }
    wbflush();			/* Why is this needed here? */
    return (top);
}


lnread(sc, rlbuf, len, index)
/*
 * This passes a packet to the higher levels.
 * We deal with the trailer protocol here.
 *
 * Arguments:
 *	sc		Pointer to softc structure pertaining to the intf.
 *	rlbuf		The receive local RAM buffer
 *	len		The length of the data to be read
 *	index		Value for index the buffers
 *
 * Return value:
 *	None.
 */

register struct ln_softc *sc;
register char *rlbuf;
int len;
int index;
{
    register struct ether_header *eptr;
	     struct ether_header  eh;
    register struct lnsw *lnsw = sc->lnsw;	/* ptr to switch struct */
    struct mbuf *m;
    /*struct protosw *pr;*/
    int off, resid;
	
    /*
     * Deal with trailer protocol: if type is INET trailer
     * get true type from first 16-bit word past data.
     * Remember that type was trailer by setting off.
     */
    if ( lnsw->ln_dodma && (m= sc->rmbuf[index]) && sc->dma[index] ) {
	eptr = mtod(m, struct ether_header *);
	m->m_data += sizeof( struct ether_header );
    } else {
	if ( lnsw->ln_dodma && !m ) 
	    /* Couldn't get an mbuf */
	    return;
	eptr = &eh;
	rlbuf = lnsw->ln_cpyinb(sc,rlbuf,eptr,sizeof(struct ether_header), 0L);
    }

    /*
     * Done with ether_header; drop len
     */
    len -= sizeof(struct ether_header);

    if (!(sc->is_if.if_flags & (IFF_ALLMULTI | IFF_PROMISC)) ) { 
	/*
	 * Make sure our multicast address filter doesn't hand us
	 * up something that doesn't belong to us!
	 */
	if ((eptr->ether_dhost[0] & 1) && (bcmp ((caddr_t)eptr->ether_dhost,
	                           (caddr_t)etherbroadcastaddr,MULTISIZE))) {
	    int i;
	    for (i = 0; i < sc->nmulti; i++) {
		if (sc->muse[i] &&
		    !(bcmp (&sc->multi[i],eptr->ether_dhost,MULTISIZE)))
		    break;
	    }
	    if (i == sc->nmulti ) { 
#if LNDEBUG
if (lndebug) printf("DISCARD1 ");
#endif
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
#if LNDEBUG
if (lndebug) printf("DISCARD2 ");
#endif
	    if ( lnsw->ln_dodma ) 
		m_freem(sc->rmbuf[index]);
	    return;		/* sanity */
	}
	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; lnget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	if(m = lnsw->lnget(sc, rlbuf, len, off, (long)index)) {
	    eptr->ether_type = ntohs( *mtod(m, u_short *));
	    resid = ntohs( *(mtod(m, u_short *)+1));
	    if (off + resid > len) {
#if LNDEBUG
if (lndebug) printf("DISCARD3 ");
#endif
	        if ( lnsw->ln_dodma ) 
			m_freem(sc->rmbuf[index]);
	 	return;		/* sanity */
	    }
	    len = off + resid;
	} else { /* can't get mbuf */
#if LNDEBUG
if (lndebug) printf("DISCARD4 ");
#endif
	    return;
	}
    } else {
	off = 0;

	 if(lnsw->ln_dodma && sc->dma[index])
	     m->m_len = len;
	 else
	     m = lnsw->lnget(sc, rlbuf, len, off, (long)index);
    }

    if (m == 0) {
	return;
    }

    if (off) {
	m->m_data += 2 * sizeof (u_short);
	m->m_len -= 2 * sizeof (u_short);
    }

        /*
         * Bump up DECnet counters. Input packets for "netstat" include
         * ALL directed, multicast, and error inputs. For DECnet, only
         * error-free input packets are counted. See the DEUNA User's
         * Guide for a breakdown of the counters.
         */
        sc->ctrblk.est_bytercvd += len ;
        if (sc->ctrblk.est_blokrcvd != (unsigned) 0xffffffff)
                sc->ctrblk.est_blokrcvd++;

        if( eptr->ether_dhost[0] & 1 ) {
                sc->ctrblk.est_mbytercvd += len ;
                if (sc->ctrblk.est_mblokrcvd != (unsigned) 0xffffffff)
                sc->ctrblk.est_mblokrcvd++;
       }

    /* Dispatch this packet */
    ether_input(&(sc->is_ed), (struct ether_header *)eptr, m, (off != 0));
}


lnwatch(unit)
/*
 * If lntint hasn't been called in a while, we restart chip,
 * and reset timer.
 * 
 * Arguments:
 *	unit		The unit number to restart.
 *
 * Return Value:
 * 	None.
 */
int unit;
{
    register struct ln_softc *sc = ln_softc[unit];
    register struct ifnet *ifp = &sc->is_if;
    int s;

    s=splimp();
    ifp->if_timer = 0;
    sc->ln_debug.trfull++;
	
    lnrestart(ifp);
    lninit(ifp->if_unit);
    splx(s);
}


caddr_t
ln_alloc16 (sc, size, align)
/*
 * Allocate a chunk of local RAM buffer. Since we never
 * give Local RAM buffer memory back, we'll just step up the
 * byte-count on a per-unit basis. (16 bits on 32 bit boundary)
 *
 * Arguments:
 *	sc		The softc structure for the interface.
 *	size		The size of the data
 *	align		The alignment
 *
 * Return Value:
 *	Success:	The address of the buffer
 *	Failure: 	NULL.
 */
register struct ln_softc *sc;
int size;
int align;
{
    register int tmpoff = sc->lrb_offset;
    register int tmpoff1;
    /*
     * Start out on the first "align" boundary
     */
    if (align) {
	tmpoff1 = svtolance16(sc, sc->ln_lrb) + tmpoff;	
	while ( (tmpoff1 & ((align*2)+1)) && (tmpoff < sc->lrb_size) )
	{ 
	    tmpoff += ((tmpoff1&0x01) ? 3 : 1);
	    tmpoff1 += ((tmpoff1&0x01) ? 3 : 1);
	}
    }
    if ((int)LRBADDR(tmpoff,size) < sc->lrb_size) {
	sc->lrb_offset = (int)LRBADDR(tmpoff,size);
	return ((caddr_t)((vm_offset_t)sc->ln_lrb + tmpoff));
    }
    else
	return (0);
}


caddr_t
ln_alloc32 (sc, size, align)
/*
 * Allocate a chunk of local RAM buffer. Since we never
 * give Local RAM buffer memory back, we'll just step up the
 * byte-count on a per-unit basis. (32 bits wide)
 *
 *
 * Arguments:
 *	sc		The softc structure for the interface.
 *	size		The size of the data
 *	align		The alignment
 *
 * Return Value:
 *	Success:	The address of the buffer
 *	Failure: 	NULL.
 */
register struct ln_softc *sc;
int size;
int align;
{
    register int tmpoff = sc->lrb_offset;
    register int tmpoff1 ; 
    
    /*
     * Start out on the first "align" boundary
     */
    if (align) {
	tmpoff1 = svtolance32(sc, sc->ln_lrb) + tmpoff ;	
	while ((tmpoff1&(align)) && tmpoff < sc->lrb_size)
	    {
		tmpoff++;
		tmpoff1++ ;
	    }
    }
    if ((int)(tmpoff+size) < sc->lrb_size) {
	sc->lrb_offset = (int)(tmpoff+size);
	return ((caddr_t)((vm_offset_t)sc->ln_lrb + tmpoff));
    }
    else
	return (0);
}


caddr_t
ln_alloc_std (sc, size, align)
/*
 * Allocate a chunk of local RAM buffer. Since we never
 * give Local RAM buffer memory back, we'll just step up the
 * byte-count on a per-unit basis. (32 bits wide)
 *
 *
 * Arguments:
 *	sc		The softc structure for the interface.
 *	size		The size of the data
 *	align		The alignment
 *
 * Return Value:
 *	Success:	The address of the buffer
 *	Failure: 	NULL.
 */
register struct ln_softc *sc;
int size;
int align;
{
    register int tmpoff = sc->lrb_offset;
    register int tmpoff1 ; 
    
    /*
     * Start out on the first "align" boundary
     */
    if (align) {
	tmpoff1 = (int)sc->ln_lrb + tmpoff ;	
	while (((tmpoff1 & align) || !(tmpoff1 & (align << 1))) && 
		 tmpoff < sc->lrb_size)
	    {
		tmpoff++;
		tmpoff1++ ;
	    }
    }
    if ((int)(tmpoff+size) < sc->lrb_size) {
	sc->lrb_offset = (int)(tmpoff+size);
	return ((caddr_t)((vm_offset_t)sc->ln_lrb + tmpoff));
    }
    else
	return (0);
}



caddr_t
ln_alloc4x4 (sc, size, align)
register struct ln_softc *sc;
int size;
int align;
{
    register int tmpoff = sc->lrb_offset;
    register int tmpoff1 ; 
    
    
    /* Alloc routine for buffers only.  Each buffer has 4 good words
     * followed by 4 unused words, then 4 good, etc.  To save space,
     * use the 4 unused words for the next buffer to be alloc'd, thus
     * they will be alternating.
     */
    
    if (sc->rlbuf[0] == NULL)	  /* allocing the 1st one */
	align = LN_OCTA_ALIGN;
    
    /*
     * Start out on the first octaword aligned boundary
     */
    if (align) {
	tmpoff1 = sc->lnsw->ln_svtolance(sc, sc->ln_lrb) + tmpoff ;
	while ((tmpoff1&(align)) && tmpoff < sc->lrb_size)
	{
	    tmpoff++;
	    tmpoff1++ ;
	}
    }	
    
    if (((int)sc->ln_lrb + tmpoff) & 0x10)	/* 5th bit set */
	sc->lrb_offset = (int)(tmpoff+16);/* next time use altrn space */
    else
	if ((int)(tmpoff+(size * 2)) < sc->lrb_size) 
	    sc->lrb_offset = (int)(tmpoff + (size * 2) - 16); 
	else return (0);
    
    return ((caddr_t)((vm_offset_t)sc->ln_lrb + tmpoff));
}	


ln_bzero16 (addr, len)
/*
 * Zero "len" bytes of Local RAM buffer
 * 
 * Arguments:
 *	addr		The address to zero out
 *	len		The length to zeroed out.
 *
 * Return Value:
 *	None.
 */
register char *addr;
register int len;
{
    register int i;
    register caddr_t lrbptr =  addr;
    
    if (len) {
	if ((vm_offset_t)lrbptr & 0x01) {
	    *(u_int *)(lrbptr - 1) &= (u_int)0x000000ff;
	    lrbptr +=3;
	    len--;
	}
	for (i = 0; i < (len - 1); i += sizeof(u_short)) {
	    *(u_int *)lrbptr = (u_int)0;
	    lrbptr += sizeof(u_int);
	}
	if (i != len)
	    *(u_int *)lrbptr &= (u_int)0xffffff00;
    }	
}	
ln_bzero32 (addr, len)
/*
 * Zero "len" bytes of Local RAM buffer
 * 
 * Arguments:
 *	addr		The address to zero out
 *	len		The length to zeroed out.
 *
 * Return Value:
 *	None.
 */
register char *addr;
register int len;
{
    if (len) 
	bzero(addr, len);
}

ln_bzero_std (addr, len)
/*
 * Zero "len" bytes of Local RAM buffer .
 * 
 * Arguments:
 *	addr		The address to zero out
 *	len		The length to zeroed out.
 *
 * Return Value:
 *	None.
 */
register char *addr;
register int len;
{

    io_zero((io_handle_t)addr, (vm_offset_t)len);
    LNIOSYNC();
}


ln_bzero4x4 (addr, len )
/*
 * Zero "len" bytes of the ring buffers.
 * routine bzero's in 4 word boundaries, skipping every
 * words.
 * 
 * Arguments:
 *	addr		The address to zero out
 *	len		The length to zeroed out.
 *
 * Return Value:
 *	None.
 */
register char *addr;
register int len;
{
    register tmp1;
    register caddr_t lrbptr = addr;
    register end4word;
    int i;

    if ((vm_offset_t) lrbptr & 0xf) {
	/* start on a 4 word boundary */
	end4word = (0x10 - (u_int)((vm_offset_t)lrbptr & 0xf));
	tmp1 = ((len > end4word) ? end4word : len);
	bzero(lrbptr, tmp1);
	len -= tmp1;
	lrbptr += tmp1;
	if (((vm_offset_t)lrbptr & 0xf) == 0)
	    lrbptr += 0x10;
    }
    /* bzero in 4 word boundaries, skipping every 4 words */
    tmp1 = (len >> 4);
    for (i=0; i < tmp1; i++) {
	bzero(lrbptr,16);
	lrbptr += 32;
    }

    len -= (tmp1 << 4);
    
    /* now the left over */
    if (len) 
	bzero(lrbptr, len);
}	


svtolance32 (sc, virt)
/*
 * Convert a system virtual address to the appropriate addressing
 * scheme as seen by the LANCE chip for a 32 bit wide buffer. 
 * 
 * Arguments:
 *	sc		The softc structure for the interface.
 *	virt		The virtual address to be converted.
 *
 * Return Value:
 *	The physical address.
 */
register struct ln_softc *sc;
caddr_t	virt;
{
    register int off,base ;
    vm_offset_t phy;
    if((svatophys(virt, &phy)) == KERN_INVALID_ADDRESS)
	panic("svtolance32: Invalid physical address!\n");
    off = phy & (sc->lrb_size-1);	/* Byte offset from base of LRB */
    base = ((phy & ~(sc->lrb_size-1)) & 0xffffff); /* LRB base addr. */
    return (off+base);
}


svtolance_std(sc, virt)
/*
 * Convert a system virtual address to the appropriate addressing
 * scheme as seen by the LANCE chip.
 * 
 * Arguments:
 *	sc		The softc structure for the interface.
 *	virt		The virtual address to be converted.
 *
 * Return Value:
 *	The physical address.
 */
register struct ln_softc *sc;
caddr_t	virt;
{
    return ((ulong)virt & (u_int)sc->lrb_size-1);
}


svtolance16 (sc, virt)
/*
 * Convert a system virtual address to the appropriate addressing
 * scheme as seen by the LANCE chip for a 16 bit wide buffer.
 * 
 * Arguments:
 *	sc		The softc structure for the interface.
 *	virt		The virtual address to be converted.
 *
 * Return Value:
 *	The physical address.
 */
register struct ln_softc *sc;
caddr_t	virt;
{
    register int off, tmp;
    vm_offset_t phy;
    if((svatophys(virt, &phy)) == KERN_INVALID_ADDRESS)
	panic("svtolance16: Invalid physical address!\n");
    off = phy & (sc->lrb_size-1);	/* Byte offset from base of LRB */
    tmp = off/2;
    return ((off == tmp*2) ? tmp : tmp+1);
}


caddr_t
ln_cpyin16(sc,from,to,len,off)
/*
 * Specialized "bcopy" to move len bytes from
 * 16-bit wide by 32-bit aligned local RAM buffer.
 * Loop does up to 32 short-word moves in a single
 * shot until the buffer is depleted. Off is non-zero
 * if we wish to begin copying off-many bytes beyond
 * "from".
 * 
 * Arguments:
 *	sc		The softc structure for the interface.
 *	from		The address to copy from.
 *	to		The address to copy to.
 *	len		The number of bytes to copy.
 *	off		The offset, if any.
 *
 * Return Value:
 *	The local RAM buffer pointer (lrbptr).
 */
struct ln_softc *sc;
caddr_t from;
caddr_t to;
u_long len, off;
{
    register caddr_t lrbptr = LRBADDR(from, off);
    register caddr_t dp = to;
    register int tmp0, tmp1;
    vm_offset_t phy;
	
    tmp0 = (unsigned)len;
    
    if (sc->lnsw->ln_dma) { 	  /* no read thru caching */
#ifdef __alpha
	LNIOSYNC();/* acquire visibility of updated shared memory structures */
#else
	if((svatophys(lrbptr, &phy)) == KERN_INVALID_ADDRESS)
	    panic("ln_cpyin16: Invalid physical address!\n");
	clean_dcache(PHYS_TO_K0(phy),len*2);
#endif
    }
    if ((vm_offset_t)lrbptr & 0x01) {
	/* Start LRB on even short-word boundary */
	*dp++ = *lrbptr;
	lrbptr += 3;
	tmp0--;
    }
	
    while (tmp0) {
	switch (tmp1 = (tmp0 >> 1)) {
	default:
	case 32:
	    tmp1 = 32;
	    *(u_short *)(dp+62) = *(u_int *)(lrbptr+124);
	case 31:
	    *(u_short *)(dp+60) = *(u_int *)(lrbptr+120);
	case 30:
	    *(u_short *)(dp+58) = *(u_int *)(lrbptr+116);
	case 29:
	    *(u_short *)(dp+56) = *(u_int *)(lrbptr+112);
	case 28:
	    *(u_short *)(dp+54) = *(u_int *)(lrbptr+108);
	case 27:
	    *(u_short *)(dp+52) = *(u_int *)(lrbptr+104);
	case 26:
	    *(u_short *)(dp+50) = *(u_int *)(lrbptr+100);
	case 25:
	    *(u_short *)(dp+48) = *(u_int *)(lrbptr+96);
	case 24:
	    *(u_short *)(dp+46) = *(u_int *)(lrbptr+92);
	case 23:
	    *(u_short *)(dp+44) = *(u_int *)(lrbptr+88);
	case 22:
	    *(u_short *)(dp+42) = *(u_int *)(lrbptr+84);
	case 21:
	    *(u_short *)(dp+40) = *(u_int *)(lrbptr+80);
	case 20:
	    *(u_short *)(dp+38) = *(u_int *)(lrbptr+76);
	case 19:
	    *(u_short *)(dp+36) = *(u_int *)(lrbptr+72);
	case 18:
	    *(u_short *)(dp+34) = *(u_int *)(lrbptr+68);
	case 17:
	    *(u_short *)(dp+32) = *(u_int *)(lrbptr+64);
	case 16:
	    *(u_short *)(dp+30) = *(u_int *)(lrbptr+60);
	case 15:
	    *(u_short *)(dp+28) = *(u_int *)(lrbptr+56);
	case 14:
	    *(u_short *)(dp+26) = *(u_int *)(lrbptr+52);
	case 13:
	    *(u_short *)(dp+24) = *(u_int *)(lrbptr+48);
	case 12:
	    *(u_short *)(dp+22) = *(u_int *)(lrbptr+44);
	case 11:
	    *(u_short *)(dp+20) = *(u_int *)(lrbptr+40);
	case 10:
	    *(u_short *)(dp+18) = *(u_int *)(lrbptr+36);
	case  9:
	    *(u_short *)(dp+16) = *(u_int *)(lrbptr+32);
	case  8:
	    *(u_short *)(dp+14) = *(u_int *)(lrbptr+28);
	case  7:
	    *(u_short *)(dp+12) = *(u_int *)(lrbptr+24);
	case  6:
	    *(u_short *)(dp+10) = *(u_int *)(lrbptr+20);
	case  5:
	    *(u_short *)(dp+8) = *(u_int *)(lrbptr+16);
	case  4:
	    *(u_short *)(dp+6) = *(u_int *)(lrbptr+12);
	case  3:
	    *(u_short *)(dp+4) = *(u_int *)(lrbptr+8);
	case  2:
	    *(u_short *)(dp+2) = *(u_int *)(lrbptr+4);
	case  1:
	    *(u_short *)(dp) = *(u_int *)(lrbptr);
	    break;
	case 0:
	    if (tmp0 & 0x01) {
		/* One lousy byte left over! */
		*dp = *lrbptr++;
	    }
	    return(lrbptr);
	    
	}
	/* Actually did some word moves */
	lrbptr += (tmp1 << 2);
	dp += (tmp1 << 1);
	tmp0 -= (tmp1 << 1);
    }
    return(lrbptr);
}	


caddr_t
ln_cpyin32(sc,from,to,len,off)
/*
 * Specialized "bcopy" to move len bytes from
 * 32-bit wide by 32-bit aligned local RAM buffer.
 * 
 * Arguments:
 *	sc		The softc structure for the interface.
 *	from		The address to copy from.
 *	to		The address to copy to.
 *	len		The number of bytes to copy.
 *	off		The offset, if any.
 *
 * Return Value:
 *	The pointer to the last byte copied.
 */
struct ln_softc *sc;
register caddr_t from;
register caddr_t to;
u_long len, off;
{
	bcopy(from+off,to,len);
	return(from+len);
}


caddr_t
ln_cpyin_std(sc,from,to,len,off)
/*
 * Standard "io-bcopy" to move len bytes from system memory to local RAM buffer.
 * 
 * Arguments:
 *	sc		The softc structure for the interface.
 *	from		The address to copy from.
 *	to		The address to copy to.
 *	len		The number of bytes to copy.
 *	off		The offset, if any.
 *
 * Return Value:
 *	The pointer to the last byte copied.
 */
struct ln_softc *sc;
register caddr_t from;
register caddr_t to;
u_long len, off;
{
    io_copyin((io_handle_t)(from+off), (vm_offset_t)to, len);
    return(from+len);
}



/*
 * TODO - get rid of this extra level of copying, if possible
 */
#define ln_HACK_SIZE ((1500 + sizeof(struct ether_header) + sizeof(u_long) - 1)/sizeof(u_long))
static u_long ln_HACK[ln_HACK_SIZE];
caddr_t
ln_cpyin4x4(sc,from,to,len,off)
/*
 * Copy on a 4 longword boundary. 
 * 
 * Arguments:
 *	sc		The softc structure for the interface.
 *	from		The address to copy from.
 *	to		The address to copy to.
 *	len		The number of bytes to copy.
 *	off		The offset, if any.
 *
 * Return Value:
 *	The "from" pointer incremented by the number of bytes copied,
 *	plus offset, plus holes.
 */
struct ln_softc *sc;
caddr_t from;
caddr_t to;
u_long len, off;
{
#ifndef __alpha
    extern caddr_t ln_cpyin4x4s();

    register caddr_t lrbptr = (from+off);
    register caddr_t dp = to, k0_addr;
    register tmp1;
    vm_offset_t phy;

    /* call ln_clean_dcache using lrbptr on 4 word boundary
     * add 2 to tmp1 to account for fragments.
     */
    if((svatophys(((vm_offset_t)lrbptr & ~(0xf)),&phy)) == KERN_INVALID_ADDRESS)
	panic("ln_cpyin4x4: Invalid physical address!\n");

    k0_addr = (caddr_t)PHYS_TO_K0(phy);
    tmp1 = (len >> 4) + 2;
    ln_clean_dcache4x4(k0_addr, tmp1);

    return(ln_cpyin4x4s(lrbptr, dp, len));
#else
    register caddr_t lrbptr = (from+off);
    register caddr_t dp = to;
    register tmp1;
    int end4word;
    caddr_t saved_dp;

    LNIOSYNC();		/* acquire visibility of updated buffer (cache flush) */

    if ((vm_offset_t) lrbptr & 0xf) {
	/* copy up to boundary */
	end4word = (0x10 - (u_int)((vm_offset_t)lrbptr & 0xf));
	tmp1 = ((len > end4word) ? end4word : len);

	len -= tmp1;
	while (tmp1--)
	    *dp++ = *lrbptr++;

	if (((vm_offset_t)lrbptr & 0xf) == 0)
	    lrbptr += 0x10;
    }

    saved_dp = 0;
    if (len && ((vm_offset_t)dp & 0x7))	/* dp is unaligned */
    {
	saved_dp = dp;
	dp = (caddr_t)ln_HACK;
    }

    tmp1 = (len >> 4);

    len -= (tmp1 << 4);

    for (; tmp1; tmp1--) {
	/* copy 16-byte chunks */
       	register u_long temp0, temp1;

	temp0 = ((u_long *)lrbptr)[0];
	temp1 = ((u_long *)lrbptr)[1];

	((u_long *)dp)[0] = temp0;
	((u_long *)dp)[1] = temp1;

	lrbptr += 32;
	dp += 16;
    }

    if (len) {
	/* now the left over */
        tmp1 = len;		
	while (tmp1--)
	    *dp++ = *lrbptr++;	
    }

    if (saved_dp)
	bcopy((caddr_t)ln_HACK,saved_dp,dp-(caddr_t)ln_HACK);

    return(lrbptr);
#endif
}



u_short ln_hold[(1500 + sizeof(struct ether_header)) / 2];

caddr_t
ln_cpyout16(from,to,len,off)
/*
 * Specialized "bcopy" to move len bytes into
 * 16-bit wide by 32-bit aligned local RAM buffer.
 * Loop does up to 32 short-word moves in a single
 * shot until the buffer is depleted. "ln_hold" is
 * used for MIPS to short-word align buffers.
 * 
 * Arguments:
 *	from		The address to copy from.
 *	to		The address to copy to.
 *	len		The number of bytes to copy.
 *	off		The offset, if any.
 *
 * Return Value:
 *	The lrb pointer incremented by the number of bytes copied.
 */
caddr_t from;
caddr_t to;
int len, off;
{
	register int tmp0,tmp1;
	register caddr_t dp = from;
	register caddr_t lrbptr = LRBADDR(to, off);

#if LNDEBUG
if (lndebug > 2)printf("ln_cpyout16: ");
if (lndebug > 2)printf("from<0x%lx> to<0x%lx> len<%d.> ",from,to,len);
if (lndebug > 2)if (off) printf("off<%d.> ",off);
if (lndebug > 2)printf("\n");
#endif
	tmp0 = (unsigned)len;
	if ((vm_offset_t)lrbptr & 0x01) {
		/* Start LRB on even short-word boundary */
		*(u_int *)(lrbptr - 1) |=
		(u_short)((*dp++ & 0xff) << 8);
		lrbptr += 3;
		tmp0--;
	}
#if defined(mips) || defined(__alpha)
	/*
	 * If we're running on a MIPS, can't copy a short 
	 * from an odd boundary.  Align dp to an even boundary
	 * by using a known short-word aligned area.
	 */

	if (tmp0 && ((vm_offset_t)dp & 0x01)) {
	   bcopy(dp,(caddr_t)ln_hold, tmp0);
	   dp = (caddr_t)ln_hold;
	}
#endif /* mips or alpha */
	while (tmp0) {
	    switch (tmp1 = (tmp0 >> 1)) {
	    default:
	    case 32:
		tmp1 = 32;
		*(u_int *)(lrbptr+124) = *(u_short *)(dp+62);
	    case 31:
		*(u_int *)(lrbptr+120) = *(u_short *)(dp+60);
	    case 30:
		*(u_int *)(lrbptr+116) = *(u_short *)(dp+58);
	    case 29:
		*(u_int *)(lrbptr+112) = *(u_short *)(dp+56);
	    case 28:
		*(u_int *)(lrbptr+108) = *(u_short *)(dp+54);
	    case 27:
		*(u_int *)(lrbptr+104) = *(u_short *)(dp+52);
	    case 26:
		*(u_int *)(lrbptr+100) = *(u_short *)(dp+50);
	    case 25:
		*(u_int *)(lrbptr+96) = *(u_short *)(dp+48);
	    case 24:
		*(u_int *)(lrbptr+92) = *(u_short *)(dp+46);
	    case 23:
		*(u_int *)(lrbptr+88) = *(u_short *)(dp+44);
	    case 22:
		*(u_int *)(lrbptr+84) = *(u_short *)(dp+42);
	    case 21:
		*(u_int *)(lrbptr+80) = *(u_short *)(dp+40);
	    case 20:
		*(u_int *)(lrbptr+76) = *(u_short *)(dp+38);
	    case 19:
		*(u_int *)(lrbptr+72) = *(u_short *)(dp+36);
	    case 18:
		*(u_int *)(lrbptr+68) = *(u_short *)(dp+34);
	    case 17:
		*(u_int *)(lrbptr+64) = *(u_short *)(dp+32);
	    case 16:
		*(u_int *)(lrbptr+60) = *(u_short *)(dp+30);
	    case 15:
		*(u_int *)(lrbptr+56) = *(u_short *)(dp+28);
	    case 14:
		*(u_int *)(lrbptr+52) = *(u_short *)(dp+26);
	    case 13:
		*(u_int *)(lrbptr+48) = *(u_short *)(dp+24);
	    case 12:
		*(u_int *)(lrbptr+44) = *(u_short *)(dp+22);
	    case 11:
		*(u_int *)(lrbptr+40) = *(u_short *)(dp+20);
	    case 10:
		*(u_int *)(lrbptr+36) = *(u_short *)(dp+18);
	    case  9:
		*(u_int *)(lrbptr+32) = *(u_short *)(dp+16);
	    case  8:
		*(u_int *)(lrbptr+28) = *(u_short *)(dp+14);
	    case  7:
		*(u_int *)(lrbptr+24) = *(u_short *)(dp+12);
	    case  6:
		*(u_int *)(lrbptr+20) = *(u_short *)(dp+10);
	    case  5:
		*(u_int *)(lrbptr+16) = *(u_short *)(dp+8);
	    case  4:
		*(u_int *)(lrbptr+12) = *(u_short *)(dp+6);
	    case  3:
		*(u_int *)(lrbptr+8) = *(u_short *)(dp+4);
	    case  2:
		*(u_int *)(lrbptr+4) = *(u_short *)(dp+2);
	    case  1:
		*(u_int *)(lrbptr) = *(u_short *)(dp);
		break;
	    case 0:
		if (tmp0 & 0x01) {
			/* One lousy byte left over! */
			*(u_int *)(lrbptr) =
				(u_short)(*dp & 0xff);
		  	lrbptr++;
		}
		return(lrbptr);
	
	    }
	    /* Actually did some word moves */
	    lrbptr += (tmp1 << 2);
	    dp += (tmp1 << 1);
	    tmp0 -= (tmp1 << 1);
	}
	return(lrbptr);
}


caddr_t
ln_cpyout32(from,to,len,off)
/*
 * bcopy to move len bytes into
 * 32-bit wide by 32-bit aligned local RAM buffer.
 * 
 * Arguments:
 *	from		The address to copy from.
 *	to		The address to copy to.
 *	len		The number of bytes to copy.
 *	off		The offset, if any.
 *
 * Return Value:
 *	The "to" pointer incremented by the number of bytes copied.
 */
register caddr_t from;
register caddr_t to;
int len, off;
{
	bcopy (from, to+off, len);
	return(to + len);
}

caddr_t
ln_cpyout_std(from,to,len,off)
/*
 * Standard "io-bcopy" to move len bytes from local RAM buffer to system memory.
 *
 * Arguments:
 *	sc		The softc structure for the interface.
 *	from		The address to copy from.
 *	to		The address to copy to.
 *	len		The number of bytes to copy.
 *	off		The offset, if any.
 *
 * Return Value:
 *	The pointer to the last byte copied.
 */
register caddr_t from;
register caddr_t to;
u_long len, off;
{
    io_copyout((vm_offset_t)from, (io_handle_t)(to+off), len);
    return(to + len);
}


#ifdef __alpha
caddr_t
ln_cpyout4x4(from,to,len,off)
/*
 * Copy out 16-byte chunks each aligned on a 32-byte boundary.
 *
 * Arguments:
 *	from		The address to copy from.
 *	to		The address to copy to.
 *	len		The number of bytes to copy.
 *	off		The offset, if any.
 *
 * Return Value:
 *	The "to" pointer incremented by the number of bytes copied,
 *	plus offset, plus holes.
 */
caddr_t from;
caddr_t to;
int len, off;
{
    register caddr_t lrbptr = (to+off);
    register caddr_t dp = from;
    register tmp1;
    int end4word;

    if ((vm_offset_t) lrbptr & 0xf) {
	/* copy up to boundary */
	end4word = (0x10 - (u_int)((vm_offset_t)lrbptr & 0xf));
	tmp1 = ((len > end4word) ? end4word : len);

	len -= tmp1;
	while (tmp1--)
	    *lrbptr++ = *dp++;

	if (((vm_offset_t)lrbptr & 0xf) == 0)
	    lrbptr += 0x10;
    }

    tmp1 = (len >> 4);

    len -= (tmp1 << 4);

    for (; tmp1; tmp1--) {
	/* copy 16-byte chunks */
	bcopy(dp,lrbptr,16);

	lrbptr += 32;
	dp += 16;
    }

    if (len) {
	/* now the left over */
	bcopy(dp,lrbptr,len);

	lrbptr += len;
    }

    return(lrbptr);
}
#endif /* alpha */


ln_setflag16(rbuf,flg)
/* 
 * Set the flag field within the ln_ring structure.
 * 
 * Arguments:
 *	rbuf		The ring buffer.
 *	flg		The flag.
 *
 * Return Value:
 *	None.
 */
register caddr_t rbuf;
register int flg;
{	
    *(rbuf + 5) = flg;
}


ln_setflag32(rbuf,flg)
/* 
 * Set the flag field within the ln_ring structure.
 * 
 * Arguments:
 *	rbuf		The ring buffer.
 *	flg		The flag.
 *
 * Return Value:
 *	None.
 */
register caddr_t rbuf;
register int flg;
{	
    *(rbuf + 3) = flg;
}

ln_setflag_std(rbuf,flg)
/* 
 * Set the flag field within the ln_ring structure.
 * 
 * Arguments:
 *	rbuf		The ring buffer.
 *	flg		The flag.
 *
 * Return Value:
 *	None.
 */
register volatile caddr_t rbuf;
int flg;
{
    io_copyout((vm_offset_t)&flg, (io_handle_t)(rbuf+3), 1);
}

/*
 *		address Rom CRC
 * 
 * Compute the 16-bit 1's complement checksum on the ethernet address. 
 * This algorithm is used to determine if the device is really an 
 * ethernet device.
 *
 * Inputs
 *	addr - pointer to the address
 *
 * outputs
 *	chksm_1, chksm_2 - Return the checksum to the caller
 *
 * This algorithm is taken from the "The Ethernet", Local Area Network
 * Specification, V2.0, Appendix B.   -dcw
 *
 */
static void addressRomCRC(addr, chksm_1, chksm_2)
unsigned char addr[], *chksm_1, *chksm_2;
{
    register int i;
    unsigned int k = 0;

    for(i = 0; i < 3; i++) {
	k *= 2;

	if (k > 65535)
	    k -= 65535;

	k += (256 * addr[2 * i]) + (addr[(2 * i) + 1]);

	if (k > 65535)
	    k -= 65535;
    }

    if (k == 65535)
	k = 0;

    *chksm_1 = k / 256;
    *chksm_2 = k % 256;

    return;

}

/*
 * Read in the ethernet address from the DEPCA's ROM.  The ethernet address
 * is stored in a 32 byte rom.  It is read a byte at a time.  Each successive
 * byte read increments the offset within the 32 byte rom.  In addition to the
 * ethernet address there is a checksum and a test pattern.
 *
 * Returns 0 - failure - the station rom did not contain an appropriate
 *			 test pattern and checksum.
 *	   1 - success - verified test pattern & checksum.
 */
depca_read_rom(vm_offset_t reg, int unit)
{
	unsigned char sum1, sum2;
    	register struct ln_softc *sc = ln_softc[unit];
	unsigned char station[64];
	int i,j,k;
	int failure_reasons = 0;
	unsigned int null_address = 1;	/* have to show otherwise */
	unsigned int pattern_found = 0;	/* ditto */

 	/*
	 * Read the ROM twice since we don't know where the address is 
	 * pointing. 
	 */
	for (i=0; i < 64; i++)
		station[i] = LNREGRD8(reg+NI_ROM_OFFSET);	

	/*
	 * Perform a check of the test pattern.  First you have to 
	 * find it.  The test pattern is FF-00-55-AA-FF-00-55-AA.
	 * If things are OK, the it should be within the first half 
	 * of what we just read above.
	 */
	for (i=0; i<32; i++) {
		if (lntestpattern(&station[i])) {
			/*
			 * Pattern found; point past it and break out.
			 */
			pattern_found++;
			i += 8;
			break;
		} else {
			if (lndebug)
				printf("ROM[%d] = %02x\n",i,station[i]);
		}
	}

	/*
	 * Validate the address before we use it.
	 */
	if (pattern_found) {
	       if (lndebug)
			printf("ln%d: test pattern OK in station rom.\n", unit);

		/*
		 * Verify the checksum.  The computation of the checksum is some
	 	 * incomprehensible procedure. I just stole the routine from the
	 	 * PC depca driver. Once the checksum is computed verify that it
	 	 * equals the checksum presented in the rom.
	 	 */
		addressRomCRC(&station[i], &sum1, &sum2);

		/*
		 * The checksums are after the 6-byte address
		 */
		if ((station[i+6] == sum1) && (station[i+7] == sum2)) {
			if (lndebug)
				printf("ln%d: checksum verified.\n", unit);
			/*
	 		 * The station address is read in as bytes.  Later it 
			 * is accessed as shorts, so assemble the address.
			 * This is saved globally for access later via lnsw.
			 */
			for (j=0,k=i; j<6; j++,i++) {
				sc->is_int_addr[j] = (u_int)(station[i] & 0xFF);

				/* Check for null-address while we're at it. */
				if (station[i])
					null_address = 0;
			}

			if (null_address) {
				if (lndebug)
					printf("ln%d: Null ethernet address.\n",
						unit);
				failure_reasons++;
			}
		} else {
			if (lndebug) {
				printf("ln%d: checksum failed.\n", unit);
				printf("sum1 = 0x%2x, station[6] = 0x%2x\n",
					sum1, station[i+6]);
				printf("sum2 = 0x%2x, station[7] = 0x%2x\n",
					sum2, station[i+7]);
			}
			failure_reasons++;
		}
	} else {
		if (lndebug)
			printf("ln%d: Invalid ethernet test pattern.\n", unit);
		failure_reasons++;
	}

	/* The spec says bit 0 of station[0] must be 0.  Apparently if
	 * this bit is set it specifies a group address.
 	 */
	if ((failure_reasons == 0) && (station[k] & 0x1)) {
		if (lndebug)
			printf("ln%d: Group station address.\n", unit);
		failure_reasons++;
	}

	if (failure_reasons) {
		if (lndebug)
			printf("Failed read of station rom for %d tests.\n",
							failure_reasons);
		return(0);
	}
	else {
		if (lndebug)
		    printf("ln%d: Successfully read station rom.\n", unit);
		return(1);
	}
}

/*
 * lntestpattern()
 * 	Checks to see if the DEC STD ethernet pattern exists.
 */
static int lntestpattern(unsigned char *p)
{
	if ((*p++ == 0xFF) && (*p++ == 0) && (*p++ == 0x55) && (*p++ == 0xAA) &&
	    (*p++ == 0xFF) && (*p++ == 0) && (*p++ == 0x55) && (*p++ == 0xAA))
		return 1;
	else
		return 0;
}
#endif /* (NLN > 0) or (BINARY) */

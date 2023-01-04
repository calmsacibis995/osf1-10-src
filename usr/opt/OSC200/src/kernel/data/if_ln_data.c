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
/*	
 *	@(#)$RCSfile: if_ln_data.c,v $ $Revision: 1.2.19.7 $ (DEC) $Date: 1993/11/22 04:29:53 $
 */ 
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
 * derived from if_ln_data.c	1.1      (ULTRIX)  2/26/91"; 
 */

/************************************************************************
 *  Modification History:						*
 *									*
 * Nov-03-91 Fred Canter						*
 *	Add support for assembly language fast copy routines		*
 * 	developend by Vasu Subramanian.					*
 *									*
 * June-04-91 Dave Gerson                                               *
 *	Changed structure array name "ds3maxplussw" to "dskn03sw"	* 
 *      to apply more generic naming.					* 
 *                                                                      *
 * 10-May-91	Paul Grist						*
 *	Add support for 3MAX+/BIGMAX (DS_5000_300), uses same chip as	*
 *	3MIN, but different register locations.				*
 *                                                                      *
 * 19-May-91								*
 *	Statically allocated contiguous memory incase of a 3MIN.	*
 *									*
 * 06-Mar-91	Mark Parenti						*
 *	Modify to use new I/O data structures				*
 *									*
 *  6-Jul-90	Lea Gottfredsen						*
 *	3min support. Added additional fields to lnsw.	        	*
 *								        *
 *  29-Jun-89	Lea Gottfredsen						*
 *	Merge of isis and pu pools, added back in packet filter		*
 * 	changes and lock to softc; multi-unit support			*
 *									*
 *  1-Jun-89 -- Lea Gottfredsen						*
 *	Added lnsw structure in order to easily accommodate		* 
 *	new hardward support.					 	*
 *									*
 *  14-Dec-88 -- templin (Fred L. Templin)				*
 *	Hardwired NRCV and NXMT to 16 for all architectures		*
 *									*
 *   28-sep-88 -- jaw							*
 *      added lock field to softc struct				*
 *									*
 *   7-Jan-88 -- templin (Fred L. Templin)				*
 *	Created the if_ln_data.c module. This module is based upon	*
 *	a modified version of the if_se_data.c module.			*
 *									*
 ************************************************************************/

/*
 * Digital LANCE NI
 */

#include "ln.h"

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
/*#include "../h/protosw.h"*/
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/syslog.h>
#include <vm/vm_kern.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
#include <net/ether_driver.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <io/dec/netif/if_lnreg.h>
#include <io/dec/uba/ubavar.h>
#include <io/dec/tc/tc.h>
#include <io/dec/eisa/eisa.h>
#include <hal/kn121.h>
/* a dummy structure added to allow compilation. */
struct ni_regs
{
	u_int ni_rdp;		
	u_int ni_rap;	
	u_int *ni_sar;
	u_int *ni_nilrb;
};
						
#ifndef mips 
#define PHYS_TO_K1(x)	((x))
#define PHYS_TO_K0(x)	((x))
#endif 

#define RLEN	5	/* 2**5 = 32  receive descriptors */
#define TLEN	3	/* 2**3 = 8   transmit descriptors */
#define NRCV	(0x0001<<RLEN) 	/* Receive descriptors */
#define NXMT	(0x0001<<TLEN) 	/* Transmit descriptors	*/
#define NTOT	(NXMT + NRCV)
#define NMULTI	64		/* Size of multicast address table */
#define MINDATA 60

/*
 * Ethernet multicast address
 *
 * This is not exactly the same as an ethernet address,
 * although it does have the same size.  The way the
 * interface works is by 'hashing' the ethernet address
 * into a 6 byte value and passing along all packets
 * that match the assigned 'multicast addresses'.
 * The hash is computed by the crc() function.
 */

struct ln_multi {
	u_char	ln_multi_char[6];
};
#define MULTISIZE sizeof(struct ln_multi)

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * is_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
#define	is_if	is_ac.ac_if		/* network-visible interface 	*/
#define	is_addr	is_ac.ac_enaddr		/* hardware Ethernet address 	*/

struct	ln_softc {

        struct  ether_driver is_ed;     /* Ethernet driver common part    */
#define is_ac   is_ed.ess_ac            /* Ethernet common part */

	/*
	 * address of "lndevice" block for this unit, plus LRAM initb
	 */
	io_handle_t rdpaddr;		/* handle for data port register */
	io_handle_t rapaddr;		/* handle for addr port register */
	u_int	*ln_narom;		/* ptr to Network addr rom */
	volatile u_char *ln_lrb;	/* ptr to Local ram buffer */
	io_handle_t ldpaddr;		/* handle for Lance DMA register */
	io_handle_t ssraddr;		/* handle for System Support register */
	io_handle_t siraddr;		/* handle for Sys Interrupt register  */

	/* 
	 * our own copy of LRB structures
 	 */
	struct	ln_initb ln_initb;	/* init block */
	struct  ln_ring ln_ring;	/* ring entry */
	
	struct lnsw *lnsw;

	/*
	 * list of local RAM buffers 
	 */
	caddr_t	initbaddr;		/* Init block address		*/
	caddr_t	rring[NRCV];		/* Receive ring desc. addresses */
	caddr_t	tring[NXMT];		/* Transmit ring desc. addresses */
	caddr_t	rlbuf[NRCV];		/* Receive local RAM buffers	*/
	caddr_t	tlbuf[NXMT];		/* Transmit local RAM buffers	*/
	struct  mbuf *tmbuf[NXMT+1];	/* Xmt mbuf chains (freed on xmt) */
	struct  mbuf *rmbuf[NRCV+1];	/* Receive mbuf chains  */
	u_char	dma[NRCV];	        /* for DMA receiver architectures */ 

	struct	ln_multi multi[NMULTI];	/* Multicast address list	*/
#define ctrblk  is_ed.ess_ctrblk        /* Counter block		*/
 	u_char	muse[NMULTI];		/* Multicast address usage count*/
#define ztime   is_ed.ess_ztime         /* Time counters last zeroed      */
	int	rindex;			/* Receive index		*/
	int	tindex;			/* Transmit index		*/
	int	otindex;		/* Old transmit index		*/
 	int	nxmit;			/* Transmits in progress	*/
	int	nmulti;			/* Current # of multicast slots */
	int	lrb_size;		/* Size of Local Ram Buffer	*/
	int	lrb_offset;		/* Current allocation offset	*/
	int ln_crc;			/* crc, must be declared global */
	int callno;			/* lninit called */
	struct lndebug { 
		unsigned int trfull;	/* transmit side called lnwatch */
		int ln_showmulti; 	/* debug: show multicast add/delete */
		int ln_bablcnt;		/* transmitter timeout counter */
		int ln_misscnt;		/* missed packet counter */
		int ln_merrcnt;		/* memory error counter */
		int ln_restarts;	/* number of times chip was restarted */
		int ln_dmareaderr;	/* # dma read errors (ioasic parity) */
                int ln_carcnt;          /* lost carrier counter */
	} ln_debug;
    /*	struct lock_t lk_ln_softc; */	/* SMP lock */
	vm_offset_t csrbase;		/* Controller CSR base address */
	int 	ln_is_depca;		/* State flag set for DEPCA */
	u_int	is_int_addr[6];		/* Saved station address - as u_ints */
};

#define lnshowmulti sc->ln_debug.ln_showmulti
#define lnbablcnt sc->ln_debug.ln_bablcnt
#define lnmisscnt sc->ln_debug.ln_misscnt
#define lnmerrcnt sc->ln_debug.ln_merrcnt
#define lnrestarts sc->ln_debug.ln_restarts
#define lndmareaderr sc->ln_debug.ln_dmareaderr
#define lncarcnt sc->ln_debug.ln_carcnt


/*
 * LANCE "switch" structure. One structure PER ARCHITECTURE, PER UNIT.
 * Per-architecture tables are indexed by unit number.
 */
struct lnsw {
	vm_offset_t	ln_rdp_offset;	/* RDP offset from device-base addr */
	vm_offset_t	ln_rap_offset;	/* RAP offset from device-base addr */
	vm_offset_t	ln_phys_narom;	/* Phys. address of NA ROM */
	vm_offset_t	ln_phys_lrb;	/* Phys. address of Local RAM BuF. */
	vm_offset_t	ln_ldp_offset;	/* DMA Physical address offset */
	vm_offset_t	ln_ssr_offset;	/* System Support Register offset */
	vm_offset_t	ln_sir_offset;	/* System Interrupt Register offset */
	int		ln_na_align;	/* Byte offset for NA ROM */
	int		ln_dodma;	/* for lance DMA */
	int		ln_dma;		/* for strange lance DMA 3min style */
	caddr_t		(*ln_cpyin)();	/* Routine to copy desc/initb FROM LRB*/
	caddr_t		(*ln_cpyout)();	/* Routine to copy desc/initb TO LRB */
	caddr_t		(*ln_alloc)();	/* Routine to alloc d/i out of LRB */
	int		(*ln_bzero)();	/* Routine to zero d/i portions of LRB*/
	int		(*ln_svtolance)();/* Routine to map virtual to lance */
	caddr_t		(*ln_cpyinb)();/* Routine to copy buffer data FROM LRB*/
	caddr_t		(*ln_cpyoutb)();/* Routine to copy buffer data TO LRB */
	caddr_t		(*ln_allocb)();	/* Routine to alloc buffers out of LRB*/
	int		(*ln_bzerob)();	/* Routine to zero buf portions of LRB*/
	int		(*ln_setflag)();/* Routine to set ring ownership flag */
	int		(*lninitdesc)();/* Routine to initialize ring */
	struct mbuf *	(*lnget)();	/* Routine to do ln get */

};

#ifdef BINARY
extern struct lnsw mayfairsw[], ffoxsw[], pmaxsw[], ds5400sw[],
	vaxstarsw[], ds5000sw[], ds3minsw[], dsMaxinesw[], dskn03sw[], 
        kn15aa_lnsw[], kn16aa_lnsw[], eisa_lnsw[], pmadaa_lnsw[];

#else
extern int ln_bzero16(), ln_bzero32(), ln_bzero_std(), ln_setflag16(), ln_setflag32(), ln_setflag_std();
extern int svtolance16(), svtolance32(), svtolance_std(), lninitdesc(), lninitdesc_dma();
extern caddr_t ln_cpyout32(), ln_cpyout16(), ln_cpyin16(), ln_cpyin32(), ln_cpyin_std(), ln_cpyout_std();
extern caddr_t as_ln_cpyout16(), as_ln_cpyin16();
extern caddr_t ln_alloc16(), ln_alloc32(), ln_alloc4x4(), ln_alloc_std();
extern int ln_bzero4x4();
extern caddr_t ln_cpyout4x4(), ln_cpyin4x4();
extern struct mbuf *lnget(), *lnget_dma();

/*
 * Define USE_LN_FASTCOPY to use the RAM buffer copy routines
 * coded in assembly language for optimum performance. Otherwise,
 * the C language copy routines will be used.
 */
#ifdef mips
#define	USE_LN_FASTCOPY
#endif

/*
 * DecStation3100 original PMAX
 */
struct lnsw pmaxsw[] = {
/* Unit 0 */
{	0x18000000,	0x18000004,	0x1d000000,	0x19000000,
#ifdef	USE_LN_FASTCOPY
	0,	0,	0,	8,	LN_NONDMA_RCV,	0,	as_ln_cpyin16,
	as_ln_cpyout16,	ln_alloc16,	ln_bzero16,	svtolance16,
	as_ln_cpyin16,	as_ln_cpyout16,	ln_alloc16,	ln_bzero16,	
#else
	0,	0,	0,	8,	LN_NONDMA_RCV,	0,	ln_cpyin16,
	ln_cpyout16,	ln_alloc16,	ln_bzero16,	svtolance16,
	ln_cpyin16,	ln_cpyout16,	ln_alloc16,	ln_bzero16,	
#endif
	ln_setflag16,	lninitdesc,	lnget }
};

/*
 * DecStation5400 - Mipsfair
 */
struct lnsw ds5400sw[] = {
/* Unit 0 */
{	0x10084400,	0x10084404,	0x10084200,	0x10120000,
	0,	0,	0,	0,	LN_NONDMA_RCV,	0,	ln_cpyin32,
	ln_cpyout32,	ln_alloc32,	ln_bzero32,	svtolance32,	
	ln_cpyin32,	ln_cpyout32,	ln_alloc32,	ln_bzero32,	
	ln_setflag32,	lninitdesc,	lnget }
};

/*
 * DecStation5000 - 3MAX
 */
struct lnsw ds5000sw[] = {
/* Unit 0 */
{	0x00100000,   0x00100004,     0x001C0000,     0x00000000,
        0,	0,	0,	16,     LN_NONDMA_RCV,	0,	ln_cpyin32,
	ln_cpyout32,	ln_alloc32,	ln_bzero32,	svtolance32,
	ln_cpyin32,	ln_cpyout32,	ln_alloc32,	ln_bzero32,	
	ln_setflag32,	lninitdesc,	lnget }
};

/*
 * DecStation5000 Model 100 - 3MIN
 */
struct lnsw ds3minsw[] = {
/* Unit 0 */
{ 	0x1C0C0000,   0x1C0C0004,     0x1C080000,     0x00000000,
	0x1C040020,	0x1C040100,	0x1C040110,	0,     LN_NONDMA_RCV,
#ifdef	USE_LN_FASTCOPY
	LN_DMA_3MIN,	as_ln_cpyin16,	as_ln_cpyout16,	 ln_alloc16,	ln_bzero16,
#else
	LN_DMA_3MIN,	ln_cpyin16,	ln_cpyout16,	 ln_alloc16,	ln_bzero16,
#endif
	svtolance16,	ln_cpyin4x4,	ln_cpyout4x4,	ln_alloc4x4,
	ln_bzero4x4,	ln_setflag16,	lninitdesc,	lnget}
};

/*
 * TurboChannel Ethernet Card - Alpha version
 */
struct lnsw pmadaa_lnsw[] = {
/*
 * Specify regular/dense-space offsets for all register accesses except the
 * narom which is given as an absolute address (also in dense space).
 */
/* Unit 0 */
{	0x00100000,     0x00100004,     0x001C0000,     0x00000000,
	0,      0,      0,      16,     LN_NONDMA_RCV,  0,      ln_cpyin32,
	ln_cpyout32,    ln_alloc32,     ln_bzero32,     svtolance32,
	ln_cpyin32,     ln_cpyout32,    ln_alloc32,     ln_bzero32,
	ln_setflag32,   lninitdesc,     lnget }
};

/*
 * EISA bus LANCE option for JENSEN
 */
struct lnsw eisa_lnsw[] = {
{	/* Basically all the stuff we need is filled in at probe time */
	0xc04,			/* ln_rdp_offset*/     
	0xc06,			/* ln_rap_offset*/     
	0x0,			/* ln_phys_narom*/     
	0,			/* ln_phys_lrb	*/
	0,			/* ln_ldp_offset*/      
	0,			/* ln_ssr_offset*/      
	0,			/* ln_sir_offset*/      
	0,			/* ln_na_align	*/     
	LN_NONDMA_RCV,		/* ln_dodma	*/  
	0,			/* ln_dma	*/      
	ln_cpyin_std,		/* ln_cpyin	*/
	ln_cpyout_std,		/* ln_cpyout	*/    
	ln_alloc_std,		/* ln_alloc	*/     
	ln_bzero_std,		/* ln_bzero	*/     
	svtolance_std,		/* ln_svtolance	*/
	ln_cpyin_std,		/* ln_cpyinb	*/     
	ln_cpyout_std,		/* ln_cpyoutb	*/    
	ln_alloc_std,		/* ln_allocb	*/     
	ln_bzero_std,		/* ln_bzerob	*/
	ln_setflag_std,		/* ln_setflag	*/   
	lninitdesc,		/* lninitdesc	*/     
	lnget }			/* lnget	*/
};
/*
 * Personal DECstation xx (20/25/33) - MAXine
 */
struct lnsw dsMaxinesw[] = {
/* Unit 0 */
{       0x1C0C0000,   0x1C0C0004,     0x1C080000,     0x00000000,
        0x1C040020,   0x1C040100,     0x1C040110,     0,       LN_NONDMA_RCV,
#ifdef	USE_LN_FASTCOPY
	LN_DMA_3MIN,	as_ln_cpyin16,	as_ln_cpyout16,	 ln_alloc16,	ln_bzero16,
#else
        LN_DMA_3MIN,  ln_cpyin16,     ln_cpyout16,    ln_alloc16,   ln_bzero16,
#endif
        svtolance16,  ln_cpyin4x4,    ln_cpyout4x4,   ln_alloc4x4,
        ln_bzero4x4,  ln_setflag16,   lninitdesc,     lnget}
};

/*
 * DecStation5000 Model 300 - 3MAX-plus (Also BIGMAX)
 */
struct lnsw dskn03sw[] = {
/* Unit 0 */
{ 	0x1F8C0000,     0x1F8C0004,     0x1F880000,     0x00000000,
	0x1F840020,	0x1F840100,	0x1F840110,	0,     LN_NONDMA_RCV,
#ifdef	USE_LN_FASTCOPY
	LN_DMA_3MIN,	as_ln_cpyin16,	as_ln_cpyout16,	 ln_alloc16,	ln_bzero16,
#else
	LN_DMA_3MIN,	ln_cpyin16,	ln_cpyout16,	 ln_alloc16,	ln_bzero16,
#endif
	svtolance16,	ln_cpyin4x4,	ln_cpyout4x4,	ln_alloc4x4,
	ln_bzero4x4,	ln_setflag16,	lninitdesc,	lnget}
};

/*
 * Alpha AXP DEC 3000 (Uses same IO Controller ASIC as 3MIN and 3MAX-plus)
 */
struct lnsw kn15aa_lnsw[] = {
/*
 * Specify regular/dense-space offsets for all register accesses except the
 * narom which is given as an absolute address (also in dense space).
 */
/* Unit 0 */
{ 	0x0000C0000,	0x0000C0004,	0x1E0080000,	0x000000000,
	0x000040020,	0x000040100,	0x000040110,	0,     LN_NONDMA_RCV,
	LN_DMA_3MIN,	ln_cpyin16,	ln_cpyout16,	ln_alloc16,  ln_bzero16,
	svtolance16,	ln_cpyin4x4,	ln_cpyout4x4,	ln_alloc4x4,
	ln_bzero4x4,	ln_setflag16,	lninitdesc,	lnget}
};

/*
 * Alpha Pelican (Uses same IO Controller ASIC as 3MIN and 3MAX-plus)
 */
struct lnsw kn16aa_lnsw[] = {
/*
 * Specify regular/dense-space offsets for all register accesses except the
 * narom which is given as an absolute address (also in dense space).
 */
/* Unit 0 */
{ 	0x0000C0000,	0x0000C0004,	0x1A0080000,	0x000000000,
	0x000040020,	0x000040100,	0x000040110,	0,     LN_NONDMA_RCV,
	LN_DMA_3MIN,	ln_cpyin16,	ln_cpyout16,	ln_alloc16,  ln_bzero16,
	svtolance16,	ln_cpyin4x4,	ln_cpyout4x4,	ln_alloc4x4,
	ln_bzero4x4,	ln_setflag16,	lninitdesc,	lnget}
};

#endif



#ifdef BINARY

extern	struct	ln_softc *ln_softc[];
extern	struct	controller *lninfo[];
extern  int	nLNNRCV;
extern	int	nLNNXMT;
extern	int	nLNNTOT;
extern	int	nLNMULTI;

#if defined VAX420 || defined MVAX || DS5000_100 || DS5000_300 || DEC3000_500 || DEC3000_300
extern  char ln_lrb[][LN_LRB_SIZE];
#else
extern char ln_lrb[][1];
#endif 

#else

#if !defined(mips) && !defined(__alpha)
tc_addr_to_name(){}
tc_isolate_memerr(){}
clean_dcache(){}
#endif /* !mips and !alpha */

/*
 * added multiple support to softc. Also, the ln_initb blocks will be
 * directly accessed in the local RAM buffers
 */
struct	ln_softc  *ln_softc[NLN];
struct	controller *lninfo[NLN];
int	nLNNXMT = NXMT;
int 	nLNNRCV = NRCV;
int	nLNNTOT = NTOT;
int	nLNMULTI = NMULTI;

/* the following because of 24 bit addressing on lance */
#if defined VAX420 || defined MVAX
char    ln_lrb[NLN][LN_LRB_SIZE]; 
#elif defined DS5000_100 || defined DS5000_300 || defined DEC3000_500 || defined DEC3000_300
char 	ln_lrb[LN_LRB_SIZE];
#else
char	ln_lrb[NLN][1];
#endif

#endif

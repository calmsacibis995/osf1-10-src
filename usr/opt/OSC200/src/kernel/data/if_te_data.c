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
 * Digital TGEC NI
 */

#include "te.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/vmmac.h>
#include <vm/vm_kern.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/proc.h>  /* Needed for nonsymmetric drivers. us */

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h> 
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/if_ether.h>
#include <net/ether_driver.h>
#include <io/common/devdriver.h>
#include <hal/cpuconf.h>
#include <io/dec/mbox/mbox.h>
#include <io/dec/netif/if_tereg.h>

#include <hal/kn430.h>

#define NMULTI	16		/* Number of multicast addresses*/
#define MINDATA 60
struct te_multi {
	u_char	te_multi_char[6];
};
struct te_setup {
	u_char  setup_char[8];
};

#define MULTISIZE sizeof(struct te_multi)

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * is_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
#define	is_if	is_ac.ac_if		/* network-visible interface 	*/
#define	is_addr	is_ac.ac_enaddr		/* hardware Ethernet address 	*/


struct	te_softc {
	struct  ether_driver is_ed;	/* Ethernet driver common part 	*/
#define is_ac   is_ed.ess_ac    	/* Ethernet common part         */
#define ctrblk  is_ed.ess_ctrblk	/* Counter block	        */
#define ztime   is_ed.ess_ztime         /* Time counters last zeroed    */
	u_long	te_narom;		/* saddr of network addr rom   	*/
	u_long	te_csrs;		/* saddr of SGEC CSRS	       	*/
	u_long	te_lint;		/* mask for clearing lint bit	*/
	struct  tesw *tesw;		/* switch table 	       	*/
	TEDESC	*rring;			/* Receive ring desc. addresses */
	TEDESC	*tring;			/* Transmit ring desc. addresses*/
	u_long	rring_phys;		/* Receive ring desc. phys addr */
	u_long	tring_phys;		/* Transmit ring desc. phys addr*/
	vm_offset_t *tbufs;		/* Transmit buffers    		*/
	vm_offset_t *rbufs;		/* Receive buffers 		*/
	char    is_dpaddr[6];		/* Default phys. address */
	struct  mbuf **smbuf;		/* Xmt mbuf chains (freed on xmt)*/
	struct	te_multi multi[NMULTI];	/* Multicast address list	*/
	struct  te_setup *te_setup;	/* Setup frame buffer           */
	caddr_t	te_diagbf;		/* Diagnostic buffer	        */
	u_char	muse[NMULTI];		/* Multicast address usage count*/
	int	ntring;			/* number of xmt descriptors   	*/
	int	nrring;			/* number of rev descriptors    */
	int	sbr;			/* System Base Register         */
	int	setupqueued;		/* Setup packet queued          */ 
	int	ntdesc;			/* transmit descriptors         */
	int   	te_vec;			/* Interrupt Vector	        */
	int	rindex;			/* Receive index		*/
	int	tindex;			/* Transmit index		*/
	int	otindex;		/* Old transmit index		*/
	int	nxmit;			/* Transmits in progress	*/
	u_int	te_initcsr;		/* Info for the CSR0 	        */
	u_int	te_cmdcsr;		/* Info for the CSR6    	*/
	u_int	te_sbr;			/* VAX system base register     */
	int	te_hw_rev;		/* hardware revision number	*/
	int	te_fw_rev;		/* hardware revision number	*/
	int	te_setup_ic;		/* interrupt on completion flag */
	struct tedebug { 
		unsigned int trfull;	/* transmit side called newatch	*/
		int te_showmulti; 	/* debug: show multicast add/delete */
		int te_tbablcnt;	/* transmitter timeout counter	*/
		int te_rbablcnt;	/* receiver timeout counter	*/
		int te_misscnt;		/* missed packet counter	*/
		int te_merrcnt;		/* memory error counter		*/
		int te_resets;		/* number of times chip restarted */
		int te_nolmbuf;		/* can't allocate cluster mbuf  */
		int te_nosmbuf;		/* can't allocate small mbuf	*/
	} te_debug;
	    /*	struct lock_t lk_te_softc; */	/* SMP lock */
};

#define teshowmulti	sc->te_debug.te_showmulti
#define tetbablcnt	sc->te_debug.te_rbablcnt
#define terbablcnt	sc->te_debug.te_rbablcnt
#define temisscnt	sc->te_debug.te_misscnt
#define temerrcnt	sc->te_debug.te_merrcnt
#define teresets	sc->te_debug.te_resets
#define tenolmbuf	sc->te_debug.te_nolmbuf
#define tenosmbuf	sc->te_debug.te_nosmbuf

#define narompt		sc->te_narom
#define csrpt		sc->te_csrs
#define csr0_init	sc->te_initcsr
#define csr6_init	sc->te_cmdcsr

/*
 * TGEC "switch" structure. One structure PER ARCHITECTURE, PER UNIT.
 * Per-architecture tables are indexed by unit number.
 */
struct tesw {
	u_long	te_saddr_csr;	/* TGEC CSR Secondary Address for mail box*/
	u_long	te_saddr_narom;	/* Secondary address of NA ROM for mail box*/
	u_long	te_lint_mask;	/* mask for clearing lint bit */
	int	te_ipl;			/* CSR0 initialization value */
	int	te_vec;			/* CSR0 initialization value */
	int	te_mode;		/* CSR0 initialization value */
	int	te_burst;		/* Burst limit value */
	int	te_sigle;		/* single mode enable */	
	int	trdesc;			/* number of RCV descriptor */	
	int	ttdesc;			/* number of XMT descriptor */
};

#ifdef BINARY
extern struct tesw cobra_sw[];
#else
extern int vtophy(), vtosvapte(), teinitdesc();
extern struct mbuf *teget(), *teget_dma();

/*
 * Cobra switch structure
 */
struct tesw cobra_sw[]	 = 	
{
	{	   /* Unit 0 */
		TGEC_UNIT0_ADDR,	ETHADDR_BYTE0,	LINT_TGEC0,
		0,			0x10c,		1,		
		4,			0x00080000,	TE_nTRING,   		
		TE_nRRING
	},
	{	   /* Unit 1 */
		TGEC_UNIT1_ADDR,	ETHADDR_BYTE32,	LINT_TGEC1,
		0,			0x10c,		1,		
		4,			0x00080000,	TE_nTRING,   		
		TE_nRRING
	}
};

#endif



#ifdef BINARY
extern	nTE;
extern	struct	te_softc te_softc[NTE];
extern	struct	controller *teinfo[NTE];
#else

int nTE=NTE;

/*
 * added multiple support to softc. Also, the te_initb blocks will be
 * directly accessed in the local RAM buffers
 */
struct	te_softc  te_softc[NTE];
struct	controller *teinfo[NTE];

#endif

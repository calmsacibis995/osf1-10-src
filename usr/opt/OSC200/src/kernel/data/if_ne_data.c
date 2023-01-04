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
/************************************************************************
 *  Modification History:						*
 *									*
 *									*
 * 25-NOV-91  BJH							*
 *      Hercules h file  modifications					*
 *									*
 * 25-Sep-89  chc (Chran-Ham Chang)					*
 *	Created the if_ne_data.c module for the SGEC network interface  *
 *									*
 ************************************************************************/

/*
 * Digital SGEC NI
 */

#include "ne.h"

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
/* #include <sys/kmalloc.h> */
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
#include <arch/mips/cpu.h>
#include <arch/mips/hal/cpuconf.h>
#include <io/dec/netif/if_nereg.h>
/* #include <io/dec/netif/if_uba.h> */
#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>
#ifdef vax
#include <dec/machine/mtpr.h>
#endif vax
#ifndef mips 
#define PHYS_TO_K1(x)	((x))
#endif 

#define NMULTI	16		/* Number of multicast addresses*/
#define SETUPSIZE 128		/* setup frame size */
#define MINDATA 60
struct ne_multi {
	u_char	ne_multi_char[6];
};
struct ne_setup {
	u_char  setup_char[8];
};
#define MULTISIZE sizeof(struct ne_multi)

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * is_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
#define	is_if	is_ac.ac_if		/* network-visible interface 	*/
#define	is_addr	is_ac.ac_enaddr		/* hardware Ethernet address 	*/

struct ne_csrs {
	u_long csr0;
	u_long csr1;
	u_long csr2;
	u_long csr3;
	u_long csr4;
	u_long csr5;
	u_long csr6;
	u_long csr7;
	u_long csr8;
	u_long csr9;
	u_long csr10;
	u_long csr11;
	u_long csr12;
	u_long csr13;
	u_long csr14;
	u_long csr15;
};
#ifdef mips 
#define NECSRS volatile struct ne_csrs
#else
#define NECSRS struct ne_csrs
#endif
struct	ne_softc {
        struct  ether_driver is_ed;     /* Ethernet driver common part    */
#define is_ac   is_ed.ess_ac            /* Ethernet common part */
#define ctrblk  is_ed.ess_ctrblk        /* Counter block	*/
#define ztime   is_ed.ess_ztime         /* Time counters last zeroed     */
	u_char	*ne_narom;		/* ptr to Network addr rom */
	struct  nesw *nesw;		/* switch table 	   */	
	NECSRS	*ne_csrs;		/* prt to SGEC CSRS	*/
	NEDESC	*rring;			/* Receive ring desc. addresses */
	NEDESC	*tring;			/* Transmit ring desc. addresses */
	struct  mbuf **tmbuf;		/* Xmt mbuf       		*/
	struct  mbuf **rmbuf;		/* Receive mbuf chains  */
	struct  mbuf **smbuf;		/* Xmt mbuf chains (freed on xmt) */
	struct	ne_multi multi[NMULTI];	/* Multicast address list	*/
	struct  ne_setup *ne_setup;	/* Setup frame buffer           */
 	u_char	muse[NMULTI];		/* Multicast address usage count*/
	int	ntring;			/* number of xmt descriptors    */
	int	nrring;			/* number of rev descriptors    */
	int	sbr;			/* System Base Registe          */
	int     setupqueued;		/* Setup packet queued          */ 
	int	ntdesc;			/* transmit descriptors         */
	int     ne_vec;			/* Interrupt Vector	        */
	int	rindex;			/* Receive index		*/
	int	tindex;			/* Transmit index		*/
	int	otindex;		/* Old transmit index		*/
 	int	nxmit;			/* Transmits in progress	*/
	u_long	ne_initcsr;		/* Info for the CSR1            */
	u_long	ne_cmdcsr;		/* Info for the CSR6            */
	u_long	ne_sbr;			/* VAX system base register     */
	int	ne_rev;			/* revision number              */
	int	ne_setup_ic;		/* interrupt on completion flag */
	struct nedebug { 
		unsigned int trfull;	/* transmit side called newatch */
		int ne_showmulti; 	/* debug: show multicast add/delete */
		int ne_tbablcnt;	/* transmitter timeout counter */
		int ne_rbablcnt;	/* receiver  timeout counter */
		int ne_misscnt;		/* missed packet counter */
		int ne_merrcnt;		/* memory error counter */
		int ne_resets;		/* number of times chip was restarted */
		int ne_nolmbuf;		/* can not allocate cluster mbuf */
		int ne_nosmbuf;		/* can not allocate small mbuf */
	} ne_debug;
/*	struct lock_t lk_ne_softc; */	/* SMP lock */
};

#define neshowmulti sc->ne_debug.ne_showmulti
#define netbablcnt sc->ne_debug.ne_rbablcnt
#define nerbablcnt sc->ne_debug.ne_rbablcnt
#define nemisscnt sc->ne_debug.ne_misscnt
#define nemerrcnt sc->ne_debug.ne_merrcnt
#define neresets sc->ne_debug.ne_resets
#define nenolmbuf sc->ne_debug.ne_nolmbuf
#define nenosmbuf sc->ne_debug.ne_nosmbuf

/*
 * SGEC "switch" structure. One structure PER ARCHITECTURE, PER UNIT.
 * Per-architecture tables are indexed by unit number.
 */
struct nesw {
	u_long	ne_phys_csr;			/* SGEC CSR Physical Address */
	u_long	ne_phys_narom;		/* Phys. address of NA ROM */
	int	ne_ipl;			/* CSR0 initializton value */
	int	ne_vec;			/* CSR0 initializton value */
	int	ne_mode;		/* CSR0 initializton value */
	int	ne_burst;		/* Burst limit value */
	int	ne_sigle;		/* single mode enable */	
	int	nrdesc;			/* number of RCV descriptor */	
	int	ntdesc;			/* number of XMT descriptor */
};

#ifdef BINARY
extern struct nesw ds5500sw[];
#else
extern int vtophy(), vtosvapte(), neinitdesc();
extern struct mbuf *neget(), *neget_dma();

/*
 * DecStation5400 - Mipsfair2
 */
struct nesw ds5500sw[] = {
/* Unit 0 */
{	0x10008000,	0x10120000,	0,
	0x10c,		1,		4,
	0x00080000,		16,   	6	
	}
};

#endif



#ifdef BINARY

extern	struct	ne_softc ne_softc[];
extern	struct	controller *neinfo[];
#else

/*
 * added multiple support to softc. Also, the ne_initb blocks will be
 * directly accessed in the local RAM buffers
 */
struct	ne_softc  ne_softc[NNE];
struct	controller *neinfo[NNE];

#endif

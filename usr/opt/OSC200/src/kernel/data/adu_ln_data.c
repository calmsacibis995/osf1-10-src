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
 *	@(#)$RCSfile: adu_ln_data.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:29:45 $
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985-1991 by			*
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
 *   University    of   California,   Berkeley,   and   from   Bell	*
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


/************************************************************************
 *  Modification History:						*
 *									*
 *  04-Jan-92	Tim Hoskins						*
 *	Ported to OSF							*
 *									*
 *  04-Sep-90	Afd							*
 *	Added Alpha support						*
 *									*
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
 * Digital ADU NI
 */

#include "aduln.h"

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
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
#include <io/dec/netif/alpha/adulnreg.h>
#include <io/dec/uba/ubavar.h>
#include <io/dec/tc/tc.h>


#define NRCV	ADU_LN_RING_ENTRIE 	/* Receive descriptors */
#define NXMT	ADU_LN_RING_ENTRIE 	/* Transmit descriptors	*/
#define NTOT	(NXMT + NRCV)
#define NMULTI	12		/* Number of multicast addresses*/
#define MINDATA 60

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
	 * our own copy of LRB structures
 	 */
	struct	ln_initb ln_initb;	/* init block */
	
	struct lnsw *lnsw;

	/*
	 * list of local RAM buffers 
	 */
	caddr_t	initbaddr;		/* Init block address		*/
	struct ln_ring *rptr;		/* pointer to rings		*/
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
	} ln_debug;
	/* struct lock_t lk_ln_softc;	-- SMP lock */
};

#define ln_rdp	rdpaddr->reg
#define ln_rap	rapaddr->reg
#define lnshowmulti sc->ln_debug.ln_showmulti
#define lnbablcnt sc->ln_debug.ln_bablcnt
#define lnmisscnt sc->ln_debug.ln_misscnt
#define lnmerrcnt sc->ln_debug.ln_merrcnt
#define lnrestarts sc->ln_debug.ln_restarts

/*
 * LANCE "switch" structure. One structure PER ARCHITECTURE, PER UNIT.
 * Per-architecture tables are indexed by unit number.
 */
struct lnsw {
	u_int	ln_phys_rdp;		/* Phys. address of RDP */
	u_int	ln_phys_rap;		/* Phys. address of RAP */
	u_int	ln_phys_narom;		/* Phys. address of NA ROM */
	u_int	ln_phys_lrb;		/* Phys. address of Local RAM BuF. */
	int	ln_na_align;		/* Byte offset for NA ROM */
	int	ln_dodma;		/* for lance DMA */
	caddr_t	(*ln_cpyin)();		/* Routine to copy data TO LRB */
	caddr_t	(*ln_cpyout)();		/* Routine to copy data FROM LRB */
	int	(*ln_bzero)();		/* Routine to zero portions of LRB */
	int	(*ln_svtolance)();	/* Routine to map virtual to lance */
	int	(*ln_setflag)();	/* Routine to set ring ownership flag */
	caddr_t (*ln_alloc)();		/* Routine to alloc out of LRB */
	int     (*lninitdesc)();	/* Routine to initilize ring */
	struct mbuf * (*lnget)();	/* Routine to do ln get */

};

#ifdef BINARY
extern struct lnsw adusw[];
#else
extern struct mbuf *alnget();
extern alninitdesc();

/*
 * ADU - Alpha Development Unit
 */
struct lnsw adusw[] = {
/* Unit 0 */
{ 0x00100000,   0x00100004,     0x001C0000,     0x00000000,
        16,     LN_NONDMA_RCV,	0,     0,    0,
        0,    0,   0,	alninitdesc, 
	alnget }
};

#endif /* BINARY */



#ifdef BINARY

extern	struct	ln_softc *aln_softc[];
extern	struct	controller *alninfo[];
extern  int	nALNNRCV;
extern	int	nALNNXMT;
extern	int	nALNNTOT;

extern char aln_lrb[][1];

#else /* not BINARY */

/*
 * added multiple support to softc. Also, the ln_initb blocks will be
 * directly accessed in the local RAM buffers
 */
struct	ln_softc  *aln_softc[NADULN];
struct	controller *alninfo[NADULN];
int	nALNNXMT = NXMT;
int 	nALNNRCV = NRCV;
int	nALNNTOT = NTOT;

char	aln_lrb[NADULN][1];

#endif /* BINARY */

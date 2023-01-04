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
 * derived from:
 *	@(#)if_xna_data.c	5.1	(ULTRIX)	3/29/91
 */
/************************************************************************
 *			Modification History				*
 *									*
 *	11/91   - Laurie Murray 					*
 *		  Port to Alpha/OSF for Laser/Ruby DEMNA support	*
 *									*
 *	6/14/89 - Fred L. Templin (templin@decvax)			*
 *		  Packet Filter support					*
 *									*
 *	4/08/89 - Fred L. Templin (templin@decvax)			*
 *		  Chaged #include lines to reflect the new		*
 *		  pool hierarchy.					*
 *									*
 *	4/08/89 - Fred L. Templin (templin@decvax)			*
 *		  SMP changes to make XNA driver SMP-safe		*
 *									*
 *	8/24/88 - Fred L. Templin (templin@decvax)			*
 *		  Created this file					*
 *									*
 ************************************************************************/

#ifdef mips
#include <dec/machine/mips/pte.h>
#include <dec/io/bi/bireg.h>
#include <dec/io/uba/ubavar.h>
#endif

#include "xna.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/vmmac.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/types.h>
#include <sys/syslog.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/if_ether.h>
#include <net/ether_driver.h>

#include <machine/cpu.h>
#include <machine/scb.h>
#include <dec/binlog/errlog.h>

#include <io/dec/xmi/xmireg.h>
#include <io/dec/mbox/mbox.h>
#include <io/common/devdriver.h>
#include <io/dec/netif/if_xnareg.h>

/*
 * NRCV is a MAXIMUM of 64, but we won't map all 64 right off the bat due
 * to buffer allocations. The number of active receive descriptors will
 * grow dynamically as advised by the p_sbua counter in the port data
 * block. We'll start out with 8 active receive descriptors, and grow
 * between the range: XNANMIN <= nactv <= XNANMAX. NCMD depends on
 * XNA_XMIT_NBUFS.
 */
#define	NRCV		64
#define NACTV		8
#define XNANMAX		32
#define	XNANMIN		4
#define	NCMD		(1024 / ((XNA_XMIT_NBUFS + 1) * 8))

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ds_if, which the routing code uses to locate the interface.
 */
struct	xna_softc {
	struct	ether_driver is_ed;		/* Ethernet driver */
	struct	ifqueue if_cmd;			/* XNA command queue */
	struct	xnacmd_buf ctrblk;		/* Per-unit line counters */
	char	is_dpaddr[6];			/* Default phys. address */
	char	is_multi[NMULTI][8];
	int	is_muse[NMULTI];
	struct	xnadevice	xregs;		/* Addresses of xna reg's */
	struct	xnarecv_ring	*rring;		/* Receive ring addr. */
	struct	xnacmd_ring	*tring;		/* Cmd/xmit ring addr. */
	struct	xnapdb		*xpdb;		/* Port data block addr. */
	struct	xnactr_ent	xna_sbuacnt;	/* Sys buf unavail advice */
	struct	xnactr_ent	xna_ubuacnt;	/* User buf unavail ctr */
	int	nactv;				/* # recv desc. to activate */
	int	nrecv;				/* # active recv desc. */
	int	tindex;				/* Current cmd/xmit index */
	int	tlast;				/* Last cmd/xmit processed */
	int	nxmit;				/* Number of active xmits */
	int	rindex;				/* Index of last active desc. */
	int	rlast;				/* Last receive processed */
	int	flags;				/* To indicate reset */
	u_int	nproc;				/* # of entries processed */
#ifdef SMP_READY
	struct	lock_t	lk_xna_softc;		/* SMP lock for xna_softc */
#endif
	struct	controller *ctlr;		/* addr of controller struct */
	struct	mbuf	*mbuf_tofree[NCMD];	/* head of mbuf chain to free
						   for transmits	    */
	struct	mbuf	*mbuf_recv[NRCV];	/* head of mbuf chain for each
						   recv ring entry   	    */
};

#define	is_ac	is_ed.ess_ac			/* Ethernet common part */
#define	ztime	is_ed.ess_ztime			/* Time since last zeroed */
#define	is_if	is_ac.ac_if			/* network-visible interface */
#define	is_addr	is_ac.ac_enaddr			/* hardware Ethernet address */

#define XNA_RFLAG       0x01                    /* Interface is being reset */

#ifdef BINARY

extern	struct xna_softc xna_softc[];
extern	short xna_firsttime[];
extern	int nNXNARCV;
extern	int nNXNAACTV;
extern	int nNXNACMD;

#else BINARY

int xnaprobe(), xnaattach();

struct device *tmpxna_info[NXNA];
struct controller *tmpxnac_info[NXNA];
caddr_t xna_std[] = { 0 };
struct driver xnadriver =
	{ xnaprobe, 0, xnaattach, 0, 0, xna_std, "xna_ethernet",
		tmpxna_info, "xna", tmpxnac_info};

struct	xna_softc xna_softc[NXNA];
short	xna_firsttime[NXNA] = { 0 };
int	nNXNARCV = NRCV;
int	nNXNAACTV = NACTV;
int	nNXNACMD = NCMD;

#endif	BINARY

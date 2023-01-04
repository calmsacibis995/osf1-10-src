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
 *			Modification History				*
 *									*
 *	Jan 21, 1992	Chris Beute (cab@unx.dec.com)                   *
 *                      Modified Matt's base for use on Alpha/OSF       *
 *									*
 *	Dec 12, 1990	Matt Thomas (thomas@pa.dec.com)			*
 *		  	Created this file				*
 *									*
 ************************************************************************/

#include "mfa.h"

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
#include <sys/syslog.h>
#include <vm/vm_kern.h>
#include <dec/binlog/errlog.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/if_ether.h>
#include <netinet/if_fddi.h>
#include <net/ether_driver.h>

#include <machine/cpu.h>
#include <machine/scb.h>

#include <io/dec/xmi/xmireg.h>
#include <io/dec/mbox/mbox.h>
#include <io/common/devdriver.h>
#include <io/dec/netif/if_mfareg.h>

/*
 * The number of active receive descriptors will grow dynamically as
 * advised by the p_sbua counter in the port data block. We'll start out
 * with 8 active receive descriptors, and vary between the range:
 *	MFARCVMIN <= sc->rcv.active <= MFARCVMAX.
 */

#define	MFARINGSIZE	2048
#define MFAXMTMAX	64 		/* (1024 / sizeof(mfaxmt_ent_t)) */
#define MFAUNSOLMAX	(512 / sizeof(mfaunsol_ent_t))
#define MFARCVMAX	(1536 / sizeof(mfarcv_ent_t))
#define	MFARCVMIN	16
/*
 *  Command queue will be 512 bytes long
 */
#define	MFACMDMAX	64	


/*
 * FDDI software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ds_if, which the routing code uses to locate the interface.
 */
struct mfa_softc {
	struct ether_driver is_ed;		/* Ethernet driver */
	char		is_dpaddr[6];		/* Default phys. address */
	mfahwaddr_t 	is_multi[NMULTI];	/* Multicast address table */
	int 		is_muse[NMULTI];	/* Multicast field in use */
	struct mfadevice xregs;			/* Addresses of mfa reg's */
	mfaring_t	rcv;			/* Receive Ring */
	mfaring_t	xmt;			/* Transmit Ring */
	mfaring_t	cmd;			/* Command Ring */
	mfaring_t	unsol;			/* Unsolicited Ring */
	mfapdb_t	*xpdb;			/* Port data block addr. */
	struct controller *ctlr;		/* addr of controller struct */
	int		flags;			/* To indicate reset */
	int		dev_type;		/* Save XMI devtype contents */
	int		unit;			/* Save controller/unit # */
	int		callno;			/* Used by watch routine */
	long		mfa_ubuacntr;		/* User buf unavail ctr */
        caddr_t		omfarcv_hib_lo;		/* RCV ring last entry */
        caddr_t		omfaxmit_hib_lo;	/* XMT ring last entry */
#ifdef SMP_SAFE
	struct lock_t	lk_mfa_softc;		/* SMP lock for mfa_softc */

#endif
	mfaparam_t 	xparams;		/* saved copy of param block */
	mfactrblk_t 	xctrs;			/* saved copy of counters */
	mfastatus_t	xstat; 			/* saved copy of STATUS cmd */
	u_int		t_req;		/* FDDI settable parameters */
	u_int		rtoken;
	u_int		tvx;
	u_int		lem; 
	u_int		ring_purger;
};

#define	is_ac	is_ed.ess_ac			/* Ethernet common part */
#define	ztime	is_ed.ess_ztime			/* Time since last zeroed */
#define	is_if	is_ac.ac_if			/* network-visible interface */
#define	is_addr	is_ac.ac_enaddr			/* hardware Ethernet address */

#define MFA_RFLAG       0x01                    /* Interface is being reset */

#ifdef BINARY

extern struct mfa_softc	mfa_softc[];
extern  struct  controller *mfainfo[];
extern  int     mfa_fddi_units;


#else BINARY

int mfaprobe(), mfaattach();
#if NMFA > 0
struct mfa_softc	mfa_softc[NMFA];
struct  controller	*mfainfo[NMFA];
#else
struct mfa_softc	mfa_softc;
struct  controller	*mfainfo;
#endif
int     mfa_fddi_units = 0;

struct  driver mfadriver =
	{ mfaprobe, 0 ,mfaattach, 0, 0, 0, 0, 0, "mfa", mfainfo };


#endif	BINARY

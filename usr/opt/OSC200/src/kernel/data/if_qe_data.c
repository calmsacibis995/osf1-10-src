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

#include "qe.h"

/* #include "packetfilter.h"		/* NPACKETFILTER */
/*
 * Digital Q-BUS to NI Adapter
 */
#include <sys/map.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/vmmac.h>
#include <vm/vm_kern.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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
#ifdef vax
#include "../machine/mtpr.h"
#endif
#include <io/dec/netif/if_qereg.h>
#include <io/dec/netif/if_uba.h>
#include <io/dec/uba/ubareg.h>
#include <io/dec/uba/ubavar.h>

#define NRCV	25	 		/* Receive descriptors		*/
#define NXMT	5	 		/* Transmit descriptors		*/
#define NTOT	(NXMT + NRCV)
#define NMULTI	12			/* Number of multicast addresses*/
#define QBMSIZ	100			/* Q-bus map resource */
#define MAXDEQNA 4			/* 2 deqna / 2 deqla */

/*
 * This constant should really be 60 because the qna adds 4 bytes of crc.
 * However when set to 60 our packets are ignored by deuna's , 3coms are
 * okay ??????????????????????????????????????????
 *
 * Note: The bug was in the qe driver itself.  Whenever an odd number of
 *	 bytes less than the minimum packet size was sent, the driver
 *	 would set the "odd" bit in the descriptor and then reset the
 *	 length to the minimum packe size which happened to be "even."
 */
#define MINDATA 60

/*
 * The qeuba structures generalizes the ifuba structure
 * to an arbitrary number of receive and transmit buffers.
 */
struct	ifxmt {
	struct	ifrw x_ifrw;			/* mapping information */
	struct	pt_entry x_map[IF_MAXNUBAMR];	/* output base pages */
	short	x_xswapd;			/* mask of clusters swapped */
	struct	mbuf *x_xtofree;		/* pages being dma'ed out */
};

struct	qeuba {
	short	ifu_uban;		/* uba number */
	short	ifu_hlen;		/* local net heaqer length */
	struct	uba_regs *ifu_uba;	/* uba regs, in vm */
	struct	ifrw ifu_r[NRCV];	/* receive information */
	struct	ifxmt ifu_w[NXMT];	/* transmit information */
	short	ifu_flags;		/* used during uballoc's */
};
struct qe_multi {
	u_char	qm_char[6];
};
#define MULTISIZE sizeof(struct qe_multi)

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * is_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
#define	is_if	is_ac.ac_if		/* network-visible interface 	*/
#define	is_addr	is_ac.ac_enaddr		/* hardware Ethernet address 	*/

struct	qe_softc {
	struct	ether_driver qe_ed;	/* Ethernet driver common part	*/
#define	is_ac	qe_ed.ess_ac		/* Ethernet common part 	*/
#define ctrblk	qe_ed.ess_ctrblk	/* Counter block		*/
#define	ztime	qe_ed.ess_ztime		/* Time counters last zeroed	*/
	struct	qeuba qeuba;		/* Q-bus resources 		*/
	int	setupaddr;		/* mapping info for setup pkts  */
	struct	qe_ring *rringaddr;	/* mapping info for rings	*/
	struct	qe_ring *tringaddr;	/*       ""			*/
	struct	qe_ring rring[NRCV+1];	/* Receive ring descriptors 	*/
	struct	qe_ring tring[NXMT+1];	/* Transmit ring descriptors 	*/
	u_char	setup_pkt[16][8];	/* Setup packet			*/
	struct	qe_multi multi[NMULTI];	/* Multicast address list	*/
	u_char	muse[NMULTI];		/* Multicast address usage count*/
	int	rindex;			/* Receive index		*/
	int	tindex;			/* Transmit index		*/
	int	otindex;		/* Old transmit index		*/
	int	qe_intvec;		/* Interrupt vector 		*/
	struct	qedevice *addr;		/* device addr			*/
	int 	setupqueued;		/* setup packet queued		*/
	int	nxmit;			/* Transmits in progress	*/
	char	*buffers;		/* Buffers for packets		*/
	int	timeout;		/* watchdog			*/
	int	qe_rl_invalid;		/* recv buffer invalid		*/
	struct	map *qb_map;		/* Q bus resource map		*/
	struct	qmap_regs *qb_mregs;	/* Q bus map registers		*/
};

#ifdef BINARY

extern	struct	qe_softc *qe_softc[];
extern	struct	controller *qeinfo[];
extern	int	nNQE;
extern  int	nNRCV;
extern	int	nNXMT;
extern	int	nNTOT;

#else BINARY

struct	qe_softc  *qe_softc[MAXDEQNA];
struct	controller *qeinfo[MAXDEQNA];
int	nNQE = 0;
int	nNXMT = NXMT;
int 	nNRCV = NRCV;
int	nNTOT = NTOT;

#endif

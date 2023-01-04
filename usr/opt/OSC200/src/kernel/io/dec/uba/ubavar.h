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
 *	@(#)$RCSfile: ubavar.h,v $ $Revision: 1.2.6.3 $ (DEC) $Date: 1993/09/21 21:56:08 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from ubavar.h	4.1	(ULTRIX)	7/2/90	
 */

/*
 * Modification history:
 *
 * 23-Feb-90	Mark Parenti (map)
 *	Added structure for bus head structures.
 *	Added interrupt parameter field.
 *	Added defines for bus number.
 *
 * 20-Dec-89    Paul Grist
 *      Added VMEbus support (vba adapters). Had to add four new fields
 *      to the uba_ctlr and uba_device structures for the support of VMEbus 
 *      devices:
 *           1. addr2: address of the second csr
 *           2. bus_priority: VME device priority level 
 *           3. ivnum: 1st configured VMEbus device interrupt vector
 *           4. priority: the main bus request level of the VMEbus device
 *      The first three are initalized by config, the last is left to be
 *      initalized when the devices are auto-configured at boot time.
 *
 * 24-Jul-89    robin
 *	made tty_ubinfo and uba_hd be of size 1.  This will need to be changed
 *	to extern of size 0 when the "OTHER" code is merged into the V4.0 pool.
 *
 * 20-Jul-89	Mark A. Parenti
 *	Add define for UBAXMI for new SSP port.
 *
 * 26-Apr-89	Kong
 *	Changed some variables to have "volatile" attributes when
 *	compiled on a mips.
 *
 * 08-Jun-88	darrell for Ali
 *	Added VAX60 (Firefox LEGSS) support.
 *
 * 19-May-88    Fred Canter
 *	PTE maps for SCSI driver and extended I/O mode on CVAXstar/PVAX.
 *
 * 15-Feb-88	Fred Canter
 *	Added defines for VAX420 (CVAXstar/PVAX) cache size and address.
 *
 * 12-11-87	Robin L. and Larry C.
 *	Added portclass support to the system.
 *
 *  3-Aug-87 -- rafiey (Ali Rafieymehr)
 *	Added sgsys and SGSYSmap for VAXstation 2000.
 *
 * 23-Apr-87 -- darrell
 *	Added a vsdev (vaxstar device) structure and changed constant
 *	definitions in order to make vs_bufctl() in uba.c accept 
 *	a pointer to the routine to be called.
 *
 * 20-Apr-87 -- afd
 *	Moved Mayfair externals to ka650.h
 *
 * 06-Mar-87 -- afd
 *	Added external definitions for map names (& correponding virtual
 *	address names) for Mayfair/CVAX local register space, as per
 *	definitions in spt.s.
 *
 *  13-Dec-86 -- fred (Fred Canter)
 *	Added shmem and SHMEMmap for MicroVAX 2000 8 line SLU.
 *
 *   5-Aug-86 -- darrell (Darrell Dunnuck)
 *	Added definitions for VAXstar disk and TZK50 drivers
 *	sharing common disk buffer.
 *
 *   2-Jul-86 -- fred (Fred Canter)
 *	Added mapping for TEAMmate 8 line SLU registers.
 *
 * 18-Jun-86 -- fred (Fred Canter)
 *	Changes for VAXstar kernel support.
 *
 * 13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 18-mar-86  -- jaw     br/cvec changed to NOT use registers.
 *
 * 05-Mar-86 -- jaw  VAXBI device and controller config code added.
 *		     todr code put in cpusw.
 *
 * 08-Aug-85 -- darrell
 *	Add constants for zero vector timer fix, and an integer
 *	definition.
 *
 * 11-jul-85 -- jaw
 *	fix bua/bda map registers.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 06-Jun-85 -jaw
 *	added support for BDA.
 *
 * 13-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 */
/*
 * This file contains definitions related to the kernel structures
 * for dealing with the unibus adapters.
 *
 * Each uba has a uba_hd structure.
 * Each unibus controller which is not a device has a uba_ctlr structure.
 * Each unibus device has a uba_device structure.
 */

#ifndef _UBAVAR_H_
#define _UBAVAR_H_

#ifndef LOCORE
/* 

 */

struct bus_dispatch {
	int bus_num;
	int bus_vec;

};

struct ub_info {
        vm_offset_t vubp;           /* virt addr of bus */
        vm_offset_t pubp;           /* phys addr of bus */
        vm_offset_t vumem;
        vm_offset_t pumem;
        vm_size_t   umemsize;
        vm_offset_t pdevaddr;
        vm_size_t   haveubasr;
        vm_size_t   adpt_num;
        vm_size_t   nexus_num;
};

/*
 * Per-uba structure.
 *
 * This structure holds the interrupt vector for the uba,
 * and its address in physical and virtual space.  At boot time
 * we determine the devices attached to the uba's and their
 * interrupt vectors, filling in uh_vec.  We free the map
 * register and bdp resources of the uba into the structures
 * defined here.
 *
 * During normal operation, resources are allocated and returned
 * to the structures here.  We watch the number of passive releases
 * on each uba, and if the number is excessive may reset the uba.
 * 
 * When uba resources are needed and not available, or if a device
 * which can tolerate no other uba activity (rk07) gets on the bus,
 * then device drivers may have to wait to get to the bus and are
 * queued here.  It is also possible for processes to block in
 * the unibus driver in resource wait (mrwant, bdpwant); these
 * wait states are also recorded here.
 */
struct	uba_hd {
	int	uba_type;		/* see defines below. */
	struct	uba_regs *uh_uba;	/* virt addr of uba */
	struct	uba_regs *uh_physuba;	/* phys addr of uba */
	int	(**uh_vec)();		/* interrupt vector */
	struct	device *uh_actf;	/* head of queue to transfer */
	struct	device *uh_actl;	/* tail of queue to transfer */
	short	uh_mrwant;		/* someone is waiting for map reg */
	short	uh_bdpwant;		/* someone awaits bdp's */
	int	uh_bdpfree;		/* free bdp's */
	int	uh_hangcnt;		/* number of ticks hung */
	int	uh_zvcnt;		/* number of 0 vectors */
	int	uh_zvflg;		/* flag for timing zero vectors */
	int	uh_errcnt;		/* number of errors */
	int	uh_lastiv;		/* last free interrupt vector */
	short	uh_users;		/* transient bdp use count */
	short	uh_xclu;		/* an rk07 is using this uba! */
#define	UAMSIZ	100
	struct	map *uh_map;		/* buffered data path regs free */
#define	QAMSIZ	1000
	struct	map *uq_map;		/* Q22 bus data path regs free */
};

#define UBA780	0x1
#define UBA750	0x2
#define UBA730	0x4
#define UBAUVI	0x8
#define UBAUVII	0x10
#define UBABUA	0x20
#define UBABDA	0x40
#define UBABLA	0x80
#define UBAXMI	0x100

#define uba_hdp 	private[4]     /* this is in the bus struct */
#define um_ubinfo 	conn_priv[4]   /* this is in the controller struct */
#define um_tab 		conn_priv[3]   /* this is in the controller struct */

#endif /* !LOCORE */


/*
 * Flags to UBA map/bdp allocation routines
 */
#define	UBA_NEEDBDP	0x01		/* transfer needs a bdp */
#define	UBA_CANTWAIT	0x02		/* don't block me */
#define	UBA_NEED16	0x04		/* need 16 bit addresses only */
#define	UBA_HAVEBDP	0x08		/* use bdp specified in high bits */
#define UBA_MAPANYWAY	0x10		/* map anyway on MicroVAX I */

/*
 * Macros to bust return word from map allocation routines.
 */
#define	UBAI_BDP(i)	((int)(((unsigned)(i))>>28))
#define	UBAI_NMR(i)	((int)((i)>>18)&0x3ff)
#define	UBAI_MR(i)	((int)((i)>>9)&0x1ff)
#define	UBAI_BOFF(i)	((int)((i)&0x1ff))

#ifndef LOCORE
#ifdef KERNEL
/*
 * UBA related kernel variables
 */
int	numuba;					/* number of uba's */
/*
 * UBA related kernel variables
 */
extern struct	uba_hd uba_hd[];
extern int tty_ubinfo[];		     /* allocated unibus map for ttys */ 

/*
 * Since some VAXen vector their unibus interrupts
 * just adjacent to the system control block, we must
 * allocate space there when running on ``any'' cpu.  This space is
 * used for the vectors for uba0 and uba1 on all cpu's.
 */
extern	int (*UNIvec[])();			/* unibus vec for uba0 */

#ifdef __mips
extern volatile int cvec;
extern volatile int br;
#else
extern int cvec;
extern int br;
#endif /* __mips */
#endif /* KERNEL */
#endif /* !LOCORE */

/*
 *  definitions for the zero vector timer
 */
#define	ZVINTVL		300	/* zero vector interval in seconds */
#define ZVTHRESH	100000	/* zero vector reporting threshold */

/* Stuff for common I/O buffer sharing between vaxstar disk and TK50 */
struct vsbuf {
	u_char	vs_status;	/* vaxstar buffer is being used	(boolean)     */
	struct vsdev *vs_active;/* tape structure			      */
	struct vsdev *vs_wants; /* disk structure			      */
};

#define VS_IDLE		0	/* Buffer not being used		      */
#define VS_SDC		1	/* vaxstar disk driver			      */
#define VS_ST		2	/* vaxstar tape driver			      */

/*
 * Action Values used to call, and as return values.
 */
#define	VS_DEALLOC	0	/* Deallocate the vaxstar buffer	      */
#define VS_ALLOC	1	/* Allocate the vaxstar buffer		      */
#define VS_KEEP		2	/* In progress				      */
#define VS_WANTBACK	3	/* More requsts, want to be called back later */

struct vsdev {
	u_char	vsd_id;		/* id of the device			      */
	int	vsd_action;	/* desired action			      */
	int	(*vsd_funcptr)();/* Callback address for vs_wants vs_id */
};

#endif

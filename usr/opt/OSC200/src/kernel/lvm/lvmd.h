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
 *	@(#)$RCSfile: lvmd.h,v $ $Revision: 4.3.3.4 $ (DEC) $Date: 1992/11/06 14:04:02 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * lvmd.h
 *
 *	Revision History:
 *
 * 04-Nov-91	Tom Tierney
 *	Modified LVM strategy return type to be int like all others (one
 *	day all driver entrypoints that return no value will be "void").
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

#ifndef _LVMD_H_
#define _LVMD_H_

/*
 * This file is derived from:

 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager - dasd.h

 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - hd.h
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	Logical Volume Manager Device Driver data structures.
 */

#include <sys/param.h>
#include <kern/lock.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/buf.h>
#include <sys/lock_types.h>

#include <kern/threadcall.h>

#include <lvm/lvm.h>
#include <lvm/lv_q.h>
#include <lvm/lv_pvq.h>
#include <lvm/pvres.h>
#include <lvm/vgres.h>
#include <lvm/ltg.h>
#include <lvm/lv_defect.h>
#include <lvm/vgsa.h>

/* defines for top half of LVDD */
#define	LVM_MINBBPOOL	1	/* Minumum number of sectors for the bad    */
				/* block pool.				    */
#define PBSUBPOOLSIZE	64	/* size of LVs pbuf subpool alloc'd at open */

#define NOMIRROR	0	/* no mirrrors	   */
#define PRIMMIRROR	0	/* primary mirrror */
#define SINGMIRROR	1	/* one mirror	   */
#define DOUBMIRROR	2	/* two mirrors	   */
#define	ANYMIRROR	-1	/* pick a mirror   */

struct lv_crit {
	int lvc_priority;
	decl_simple_lock_data(,lvc_lock)
};
#define LOCK_INTERRUPT_INIT(LVCP)			\
	MACRO_BEGIN					\
		simple_lock_init(&(LVCP)->lvc_lock);	\
	MACRO_END

#define LOCK_INTERRUPT(LVCP)				\
	MACRO_BEGIN					\
		(LVCP)->lvc_priority=splbio();		\
		simple_lock(&((LVCP)->lvc_lock));	\
	MACRO_END

#define UNLOCK_INTERRUPT(LVCP)				\
	MACRO_BEGIN					\
		simple_unlock(&((LVCP)->lvc_lock));	\
		splx((LVCP)->lvc_priority);		\
		(LVCP)->lvc_priority = 0;		\
	MACRO_END
/*
 *  Physical request buf structure.
 *
 *	A 'pbuf' is a 'buf' structure with some additional fields used
 *	to track the status of the physical requests that correspond to
 *	each logical request.  A pool of pbuf's is allocated and
 *	managed by the device driver.  The size of this pool depends on
 *	the number of open logical volumes.
 */
struct pbuf {	
	/* This must come first, 'buf' pointers can be cast to 'pbuf' */
	struct buf	pb;	        /* imbedded buf for physical driver */

	struct buf	*pb_lbuf;	/* corresponding logical buf struct */
	void		(*pb_sched) ();	/* scheduler I/O done policy func   */
	struct pvol	*pb_pvol;	/* physical volume structure	    */
	struct lvol	*pb_lv;		/* logical volume being I/O'ed	    */
	lv_bblk_t	*pb_bad;	/* defects directory entry	    */
	daddr_t		pb_start;	/* starting physical block	    */
	caddr_t		pb_startaddr;	/* starting physical address	    */
	caddr_t		pb_endaddr;	/* ending physical address	    */
	int		pb_options;	/* logical buffer/volume options    */
	uchar_t		pb_mirror;	/* mirror this pbuf is accessing    */
	uchar_t		pb_miract;	/* active mirrors		    */
	uchar_t		pb_mirbad;	/* mask of broken mirrors	    */
	uchar_t		pb_mirdone;	/* mask of mirrors done		    */
	uchar_t		pb_miravoid;	/* mask of mirrors avoided	    */
	uchar_t		pb_swretry;	/* number of sw relocation retries  */
	uchar_t		pb_op;		/* Operation in progress	    */
	uchar_t		pb_type;	/* Type of pbuf			    */
	uchar_t		pb_vgsa_failed;	/* Update of the VGSA failed.	    */
	int		pb_seqnum;	/* The VGSA sequence number.	    */
};

extern struct lv_crit lv_pbuf_lock;

#define pb_addr	pb.b_un.b_addr		/* too ugly in its raw form	    */

/*
 *  Volume group structure.
 *
 *  Volume groups are implicitly open when any of their logical volumes are.
 */

#define MAXLVS		256		/* implementation limit on # LVs  */
#define MAXPVS		32		/* implementation limit on number */
					/* physical volumes per vg	  */
#define	MWCHSIZE	8		/* Number of mwc cache queues	  */
#define NBPI	(NBPB * sizeof(int))	/* Number of bits per int	  */
#define PVOLSALLOC	32		/* pvols growth amount on realloc */

struct volgrp {
	lock_data_t	vg_lock;	/* lock for all vg structures	    */
	struct lvol     **lvols;	/* logical volume struct array	    */
	uint_t		num_lvols;	/* size of lvols array		    */
	struct pvol	**pvols;	/* physical volume struct array	ptr */
	uint_t		size_pvols;	/* size of pvols array		    */
	uint_t		num_pvols;	/* number of pvol's in pvols array  */
	int		major_num;	/* major number of volume group     */
	lv_uniqueID_t	vg_id;		/* volume group id		    */
	short		vg_extshift;	/* log base 2 of extent size in blks*/
	short		vg_opencount;	/* count of open logical volumes    */
	uchar_t		vg_flags;	/* VG flags field		    */
	struct lv_crit	vg_intlock;	/* VG interrupt lock: */
					/* protects totalcount,requestcount */
	int		vg_totalcount;	/* cumulative */
	int 		vg_requestcount;/* instantaneous */

	/* Following used in write consistency cache management		    */
	struct lv_crit	vg_ca_intlock;	/* Mutual exclusion */
	struct buf	vg_mwc_lbuf;	/* logical buf used for MWC writes  */
	struct lv_queue	vg_cache_wait;	/* volume group cache wait queue    */
	struct lv_pvqueue vg_cache_write; /* queue of pvols w/cache I/O	    */
	struct mwc_rec  *vg_mwc_rec;	/* ptr to part 1 of cache - disk rec*/
	struct ca_mwc_mp *ca_part2;	/* ptr to part 2 of cache - memory  */
	struct ca_mwc_mp *ca_lst;	/* mru/lru cache list anchor	    */
	struct ca_mwc_mp *ca_hash[MWCHSIZE];/* write consistency hash anchors*/
	uchar_t		ca_free;	/* count of free entries (iocnt==0) */
	uchar_t		ca_size;	/* number of entries in cache	    */
	uchar_t		ca_chgcount;	/* number of changed entries	    */
	uchar_t		ca_flags;	/* flags to control cache states    */
	ushort_t	ca_clean_minor;	/* lvol to scrub */

	/* Following used in VGDA management.				    */
	char		*vg_vgda;	/* Pointer to the VGDA.		    */
	uint_t		vg_LVentry_off;	/* Offset of the LV entries in VGDA.*/
	uint_t		vg_PVentry_off;	/* Offset of the PV entries in VGDA.*/
	uint_t		vg_PVentry_len;	/* Lengt of a PV entry in the VGDA. */
	uint_t		vg_VGtrail_off;	/* Offset of the VG trailer in VGDA.*/

	/* The following are used for status area management.		    */
	struct lv_crit	vg_sa_intlock;	/* Status area mutual exclusion.    */
	uint_t		vg_sa_state;	/* Status area state.		    */
	struct vgsa	vg_sa_ptr;	/* pointers into the status area.   */
	struct lv_queue	vg_sa_hold;	/* The hold queue.		    */
	struct lv_queue	vg_sa_active;	/* The active list.		    */
	uint_t		vg_sa_wheel;	/* The current wheel index.	    */
	int		vg_sa_seqnum;	/* The current sequence number.	    */
	struct pbuf	vg_sa_pbuf;	/* Buffer to use to do SA updates.  */
	int		vg_sa_conf_op;	/* Configuration Operation	    */
	void		*vg_sa_conf_arg;/* Configuration Op argument	    */
	int		vg_sa_conf_seq;	/* Configuration Op final seq #     */

	ushort_t	vg_maxlvs;	/* volume group limit on # LVs	    */
	ushort_t	vg_maxpvs;	/* volume group limit on # PVs	    */
	ushort_t	vg_maxpxs;	/* volume group limit on # PXs	    */
	uint_t		vg_pxsize;	/* volume group physical extent size*/
	uint_t		vgda_len;	/* length of the VGDA.		    */
	uint_t		vgsa_len;	/* length of the VGSA.		    */
	uint_t		mcr_len;	/* length of the MCR.		    */
};

/*
 * Defines for vg_flags field in volgrp structure
 */
#define	VG_LOST_QUORUM		0x01	/* VG has lost quorum		    */
#define	VG_NOTCONFIGURED  	0x02	/* VG is not real - ignore it	    */
#define	VG_ACTIVATED		0x04	/* VG is not activated		    */
#define	VG_NOLVOPENS		0x08	/* Don't allow logical volume opens */

/*
 * Defines for ca_flags field in volgrp structure
 */
#define	CACHE_ACTIVATED	0x01	/* Cache has been activated */
#define	CACHE_INFLIGHT	0x02	/* Cache is being written */
#define CACHE_CHANGED	0x04	/* Cache has been modified */
#define	CACHE_CLEAN	0x08	/* Someone is sleeping for cache */

/*
 * Defines for vg_sa_state field in volgrp structure
 */
#define	SA_ACTIVE	1	/* The wheel is running.		    */

/*
 *  Logical volume structure. Created at volume group activate.
 */
struct lvol {
	struct h_anchor	*work_Q;	/* work in progress hash table      */
	struct lv_queue lv_ready_Q;	/* requests to pass to lower layers */
	struct lextent	*lv_lext;	/* the logical extent array	    */
	struct extent	*lv_exts[LVM_MAXCOPIES]; /* physical extent arrays  */
	struct lv_sched *lv_schedule;   /* the scheduling policy functions */
	lock_data_t	lv_lock;	/* locks the lv descriptions:	*/
				/* lv_exts contents	*/
				/* lv_ref		*/
				/* lv_rawavoid		*/
				/* lv_rawoptions	*/
				/* lv_curpxs/lv_maxlxs/lv_curlxs */
				/* lv_flags/lv_sched_strat/lv_maxmirrors */
				/* These fields are read-only below lv_block */
	struct lv_crit	lv_intlock;	/* locks lvol transient data:	*/
				/* work_Q contents 	*/
				/* lv_ready_Q contents	*/
				/* lv_lext contents	*/
				/* lv_complcnt		*/
				/* lv_totalcount/lv_requestcount */
				/* lv_status 		*/
	int		lv_complcnt;	/* completion count-used to quiesce */
	int		lv_totalcount;	/* cumulative */
	int		lv_requestcount;/* instantaneous */
	short		lv_status;	/* lv status: closed, closing, open */
	ushort_t	lv_ref;
	ushort_t	lv_rawavoid;
	ushort_t	lv_rawoptions;
		/* Fields from the LV_entry, or summary info */
	uint_t		lv_curpxs;	/* total # of phys. extents in LV   */
	ushort_t	lv_maxlxs;	/* Maximum size of the LV (extents) */
	ushort_t	lv_curlxs;	/* # of populated log. extents	    */
	ushort_t	lv_flags;	/* Logical volume flags.	    */
	uchar_t		lv_sched_strat;	/* The scheduling strategy.	    */
	uchar_t		lv_maxmirrors;	/* The maximum number of mirrors.   */

	struct buf	lv_rawbuf;	/* single struct buf for raw I/O    */
};

/* lv status:  */
#define	LV_CLOSED  	0x0000		/* logical volume is closed	  */
#define	LV_OPEN   	0x0001		/* logical volume is open	  */
#define	LV_PAUSING	0x0002		/* logical volume is pausing 	  */
#define	LV_PAUSED	0x0004		/* logical volume is paused	  */
#define	LV_RAWOPEN	0x0008		/* raw (character) device is open */
#define	LV_BLOCKOPEN	0x0010		/* cooked (block) device is open  */
#define	LV_FAKEOPEN	0x0020		/* admin cmd fake open		  */
#define	LV_EXCLUSIVE	0x0040		/* No more opens of this device   */

/*
 * Logical volume scheduling policy switch.
 */
struct	lv_sched {
	void	(*lv_schedule)(		/* vg, lv, lb, pb */ );
struct	pbuf *  (*lv_allocbuf)(		/* lb */ );
};

/*  Physical extent structure. */
struct extent {
	ushort_t	e_pxnum;	/* The physical extent number	*/
	uchar_t		e_pvnum;	/* The physical volume number	*/
	uchar_t		e_state;	/* The physical extent state	*/
};
typedef struct extent extent_t;

/* Null physical volume definition */
#define	PX_NOPV		0xff

/* Physical extent state definitions. */
#define PX_STALE	LVM_PXSTALE	/* The physical extent is stale.  */
#define	PX_CHANGING	0x02		/* The physical extent is stale,  */
					/*    but the VGSAs have not been */
					/*    completely updated.	  */
#define PX_NOMWC        0x04            /* Set when PX is in the process  */
                                        /* of being removed (reduced)     */

/* Logical extent structure. */
struct lextent {
	short	lx_synctrk;	/* Current LTG being resynced.	  */
	uchar_t	lx_syncmsk;	/* Current LTG sync mask.	  */
	uchar_t	lx_syncerr;	/* error behind synctrk */
};
typedef struct lextent lextent_t;
/* synctrk == -1 means no resync-in-progress */

/*
 *  Physical volume structure.
 *
 *	Contains defects directory hash anchor table.  The defects directory
 *	is hashed by track group within extent.  Entries within each defect
 *	table entry are sorted in ascending block addresses. 
 */

struct pvol {
	struct volgrp	*pv_vg;		/* volume group this pvol belongs to*/
	lv_lvmrec_t	*pv_lvmrec;	/* Pointer to the pvol's lvm record */
	lv_bblk_t	*pv_bbdir;	/* The volume defect directory.	    */
	uint_t		pv_maxdefects;	/* Max defects in defect directory  */
	struct vnode	*pv_vp;		/* The vnode of open pvols.	    */
	struct timeval	pv_vgdats[2];	/* The VGDA timestamps		    */
	uint_t		pv_vgra_psn;	/* PSN of the VGRA on this disk     */
	uint_t		pv_data_psn;	/* the start of the user data	    */
	uint_t		pv_pxspace;	/* space allocated for each extent  */
	ushort_t	pv_pxcount;	/* # of physical extents	    */
	ushort_t	pv_freepxs;	/* # of available physical extents  */
	daddr_t		pv_datapsn;	/* start of user data extents	    */
	struct lv_crit	pv_intlock;	/* lock for the following items:    */
	daddr_t		pv_armpos;	/* last requested arm position	    */
	struct lv_queue pv_ready_Q;	/* requests to pass to phys driver  */
	int		pv_totxf;	/* Total number of xfers to this pv */
	short		pv_curxfs;	/* Current number of active xfers   */
	ushort_t	pv_flags;	/* The physical volume flags	    */
	uchar_t		pv_num;		/* LVM PV number 0-31		    */

	int		pv_sa_seqnum[2];/* The current sequence number.	    */
	uint_t		pv_sa_psn[2];	/* The VGSA sector numbers.	    */
	struct timeval	pv_vgsats[2];	/* The timestamps on the VGSAs	    */

	struct lv_queue pv_cache_wait;	/* requests waiting on cache write  */
	struct pvol	*pv_cache_next;	/* next pvol with cache I/O	    */
	struct mwc_rec	*pv_mwc_rec;	/* (temporary) pointer to MWC record*/
	struct timeval	pv_mwc_latest;	/* time of newest MWC on this pvol  */
	int		pv_mwc_flags;	/* PVOL MWC flags		    */
	daddr_t		pv_mwc_loc[2];	/* location of the MWCs on this pvol*/

	daddr_t		altpool_psn;	/* first blkno in alternate pool    */
	daddr_t		altpool_next;	/* blkno of next unused alternate   */
					/* block in pool at end of PV	    */
	daddr_t		altpool_end;	/* last blkno in alternate pool     */
	lv_defect_t	*pv_defects[HASHSIZE];/* defect directory anchors   */
	lv_defect_t	*freelist;	/* list of free defect chain links  */
	uint_t		freelistsize;	/* size of freelist space	    */
	uint_t		bbdirsize;	/* size of pv_bbdir space	    */
	struct buf	pv_buf;		/* buf struct for PV reserved I/O   */
					/* (LVM rec & defect directory)	    */
};
/* pv_flags values (see vgres.h) */
/* #define LVM_PVMISSING   0x0001 */
/* #define LVM_PVRORELOC   0x0002 */
/* #define LVM_NOTATTACHED 0x0004 */
#define LVM_MWCMISSING	   0x8000

/* pv_mwc_flags values */
#define	PV_CACHE_TOGGLE	0x1
#define PV_CACHE_QUEUED	0x2

/*
 *  Macros for accessing these data structures.
 */
#define VG_DEV2LV(vg, dev)      ((minor(dev) >= (vg)->num_lvols) ? \
				 0 : ((vg)->lvols[minor(dev)]))
#define	VG_LVOL0(vg)		((vg)->lvols[0])
#define VG_DEV2PV(vg, pnum)	((vg)->pvols[(pnum)])

#define BLK2EXT(volgrp,lbn)	((uint_t)(lbn)>>((volgrp)->vg_extshift))
#define EXT2BLK(volgrp,x_no)	((x_no)        <<((volgrp)->vg_extshift))
#define	TRK2EXT(volgrp,trkno)	((trkno)>>(((volgrp)->vg_extshift)-LTGSHIFT))
#define EXT2TRK(volgrp,x_no)	((x_no) <<(((volgrp)->vg_extshift)-LTGSHIFT))
#define TRKPEREXT(volgrp)	(1<<(((volgrp)->vg_extshift)-LTGSHIFT))
#define TRKINEXT(volgrp,lbn)	(BLK2TRK(lbn)&(TRKPEREXT(volgrp)-1))

#define EXTENT(lv,x_no,mir)	((lv)->lv_exts[mir]+(x_no))
#define LEXTENT(lv,x_no)	((lv)->lv_lext+(x_no))

/*
 *  Mirror bit definitions
 */

#define PRIMARY_MIRROR		001	/* primary mirror mask		*/
#define SECONDARY_MIRROR	002	/* secondary mirror mask	*/
#define TERTIARY_MIRROR		004	/* tertiary mirror mask		*/
#define ALL_MIRRORS		007	/* mask of all mirror bits	*/

/* macro to extract mirror avoidance mask from ext parameter */
#define	X_AVOID(ext)		( ((ext) >> 4) & ALL_MIRRORS )

/*
 *  Macros to select mirrors using avoidance masks:
 *
 *	FIRST_MIRROR	returns first unmasked mirror (0 to 2); 3 if all masked
 *	FIRST_MASK	returns first masked mirror (0 to 2); 3 if none masked
 *	MIRROR_COUNT	returns number of unmasked mirrors (0 to 3)
 *	MIRROR_MASK	returns a mask to avoid a specific mirror (1, 2, 4)
 *	MIRROR_EXIST	returns a mask for non-existent mirrors (0, 4, 6, or 7)
 */
#define	FIRST_MIRROR(mask)	((0x30102010>>((mask)<<2))&0x0f)
#define	FIRST_MASK(mask)	((0x01020103>>((mask)<<2))&0x0f)
#define	MIRROR_COUNT(mask)	((0x01121223>>((mask)<<2))&0x0f)
#define	MIRROR_EXIST(nmirrors)	((0x00000467>>((nmirrors)<<2))&0x0f)
#define	MIRROR_MASK(mirror)	(1<<(mirror))

/*
 *  LVDD internal macros and extern staticly declared variables.  See
 *  hd_phys.c for definition.
 */

/* internal return codes: */
#define MAXGRABLV	16	/* Max number of LVs to grab pbuf structs */
#define MAXSYSVG	3	/* Max number of VGs to grab pbuf structs */

#define DEV2VG(dev)	((struct volgrp *)(cdevsw[major(dev)].d_ttys))

/* aliases for ``driver-reserved'' fields in struct buf */
#define b_options	b_driver_un_1.longvalue
/* Values for b_options:
 * From lv_rawavoid:
 * LVM_MIRAVOID 0x0007
 * From lv_flags & lv_rawoptions:
 * LVM_OPT_NORELOC  0x0100
 * From lv_flags, and set by MWC manager:
 * LVM_OPT_NOMWC    0x0200
 */
#define	LVM_OPT_NORELOC 0x00000100
#define	LVM_OPT_NOMWC	0x00000200
#define	LVM_RESYNC_OP	0x00010000
#define	LVM_REQ_WANTED	0x00020000
#define	LVM_RECOVERY	0x00040000
#define	LVM_VGSA_FAILED	0x00080000

/* #define b_work		b_driver_un_2.longvalue */

extern struct pbuf	*lv_freebuf;	/* free physical buf list	*/

/* Following are used for statistics gathering */
extern int lv_pbuf_inuse;	/* Number of pbufs currently in use	*/
extern int lv_pbuf_maxuse;	/* Maximum number of pbufs in use during*/
				/* this boot				*/

extern thread_callq_t lv_threadq;
/*
 *  LV_SCHED_DONE -- invoke scheduler policy routine for this request.
 *
 *	For physical requests it invokes the physical operation end policy.
 */
#ifdef __alpha
/* alpha jestabro - make the current compiler happy - ANSI problems?? */
#define LV_SCHED_DONE(pb) (((pb)->pb_sched)?((*(pb)->pb_sched) (pb)),0:0)
#else
#define LV_SCHED_DONE(pb) (((pb)->pb_sched)?((*(pb)->pb_sched) (pb)):(void)0)
#endif

/*
 * Write consistency cache structures and macros
 */

/* cache hash algorithms - returns index into cache hash table */
#define MWC_HASH(lb)	(BLK2TRK(lb->b_blkno) & (MWCHSIZE-1))
#define MWC_THASH(trk)	((trk) & (MWCHSIZE-1))

#define LVM_NUMMWC	32	/* size of MWC */
#define CACHE_MRU	0

/*
 * This structure will generally be referred to as part 2 of the cache
 */
struct ca_mwc_mp {	/* cache mirror write consistency memory only part  */
    struct ca_mwc_mp	*ca_hq_next;	/* ptr to next hash queue entry	    */
    struct pvol		*ca_pvol;	/* Ptr to pvol containing primary ? */
    struct ca_mwc_dp	*ca_mwc_ent;	/* Ptr to part 1 entry - ca_mwc_dp  */
    struct ca_mwc_mp	*ca_next;	/* Next memory part struct	    */
    struct ca_mwc_mp	*ca_prev;	/* Previous memory part struct	    */
    ushort_t 		ca_iocnt;
    ushort_t		ca_state;	/* State of entry		    */
};

/* ca_mwc_mp ca_state defines */
#define CACHE_ENTRY_CHANGING	0x1	/* Cache entry has changed since */
					/* last cache write operation    */
#define CACHE_ENTRY_FROZEN      0x2     /* VGSA update failed - cannot   */
                                        /* delete this cache entry       */

#define KFREE(PTR, SIZE)			\
MACRO_BEGIN					\
	if (PTR) {				\
		ASSERT((SIZE) != 0);		\
		kfree((PTR),(SIZE));		\
		(PTR)=NULL;			\
	}					\
MACRO_END
#define	NEW(TYPE)		((TYPE *)kalloc(sizeof(TYPE)))
#define	ROUNDUP(X, N)		((((X) + ((N)-1))/(N))*(N))

/* External Function Declarations */

/* lv_strategy.c: */
extern int  lv_strategy();
extern void lv_terminate();
extern void lv_pause();
extern void lv_continue();

/* lv_schedule.c: */
extern void lv_schedule();

/* lv_vgsa.c: */
extern void lv_sa_start();

/* lv_phys.c */
extern void lv_begin();
extern void lv_resume();
extern void lv_startpv();

/* Temporary - compatibility code */

#define LVM_QUERYVG_COMPAT
#ifdef LVM_QUERYVG_COMPAT
/*
 * lv_oqueryvg - retrieve information about a volume group. (COMPATIBILITY)
 */
struct lv_oqueryvg {
    lv_uniqueID_t vg_id;	/* Volume group ID.		   */
    ushort_t	  maxlvs;	/* Max # logical volumes allowed.  */
    ushort_t	  maxpvs;	/* Max # physical volumes allowed. */
    uint_t  	  pxsize;	/* Physical extent size.	   */
    ushort_t	  freepxs;	/* Number of free extents.	   */
    ushort_t	  cur_lvs;	/* Current logical volume count.   */
    ushort_t	  cur_pvs;	/* Current physical volume count.  */
    ushort_t	  status;	/* Status of the volume group.	   */
};
#ifdef __alpha
/* alpha jestabro - current compiler makes size same for {O,}QUERYVG structs */
#define	LVM_OQUERYVG		_IOWR('v',  99, struct lv_oqueryvg)
#else
#define	LVM_OQUERYVG		_IOWR('v',  18, struct lv_oqueryvg)
#endif

#endif /* LVM_QUERYVG_COMPAT */

#endif  /* _LVMD_H_ */

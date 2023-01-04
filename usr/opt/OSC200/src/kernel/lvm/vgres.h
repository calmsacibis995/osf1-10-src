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
 *	@(#)$RCSfile: vgres.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:23:21 $
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
#ifndef _LVM_VGRES_H_
#define _LVM_VGRES_H_

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
 *	Logical Volume Manager Volume Group Reserved Area data structures.
 */

#include <sys/types.h>
#include <sys/time.h>

/*
 * The volume group header and trailer.
 */
struct VG_header {
struct	timeval		vg_timestamp;	/* Time the VGDA was last written.  */
	lv_uniqueID_t	vg_id;		/* The volume group id		    */
	ushort_t	maxlvs;		/* The maximum number of LVs in VG. */
	ushort_t	numpvs;		/* The number of PV in the VG.	    */
	ushort_t	maxpxs;		/* The maximum number of PXs in VG. */
	ushort_t	reserved1;	/* RESERVED */
	uint_t		reserved2;	/* RESERVED */
	uint_t		reserved3;	/* RESERVED */
};
#define	VG_head(vg)	((struct VG_header *)(vg->vg_vgda))

struct VG_trailer {
struct	timeval		vg_timestamp;	/* Time the VGDA was last written.  */
	uint_t		reserved1;	/* RESERVED */
	uint_t		reserved2;	/* RESERVED */
	uint_t		reserved3;	/* RESERVED */
	uint_t		reserved4;	/* RESERVED */
	uint_t		reserved5;	/* RESERVED */
	uint_t		reserved6;	/* RESERVED */
};
#define	VG_trail(vg)	((struct VG_trailer *)(vg->vg_vgda + vg->vg_VGtrail_off))

/*
 * The logical volume entry in the VGDA.
 */
struct LV_entry {
	ushort_t	maxlxs;		/* Maximum size of the LV.	    */
	ushort_t	lv_flags;	/* Logical volume flags.	    */
	uchar_t		sched_strat;	/* The scheduling strategy.	    */
	uchar_t		maxmirrors;	/* The maximum number of mirrors.   */
	ushort_t	reserved1;	/* RESERVED */
	uint_t		reserved2;	/* RESERVED */
	uint_t		reserved3;	/* RESERVED */
};
#define	LV_ent(vg, i)	\
	((struct LV_entry *)(vg->vg_vgda + vg->vg_LVentry_off +	\
			     ((i-1) * sizeof(struct LV_entry))))

/*
 * The physical volume header and the physical extent descriptor in the VGDA.
 */
struct PV_header {
	lv_uniqueID_t	pv_id;		/* The physical volume ID.	    */
	ushort_t	px_count;	/* Number of physical extents.	    */
	ushort_t	pv_flags;	/* The physical volume flags.	    */
	ushort_t	pv_msize;	/* Size of PX entry map, in entries */
	ushort_t	pv_defectlim;	/* Max relocated defects allowed */
};
#define	PV_head(vg, i)	((struct PV_header *)				\
			 	(vg->vg_vgda + vg->vg_PVentry_off +	\
					      (vg->vg_PVentry_len * i)))

struct PX_entry {
	ushort_t	lv_index;	/* The logical volume index.	    */
	ushort_t	lx_num;		/* The logical extent number.	    */
};
#define	PX_ent(vg, i, px) (((struct PX_entry *)				      \
				(PV_head(vg, i) + 1)) + px)

/*
 * This structure will generally be referred to as part 1 of the cache
 */
struct ca_mwc_dp {      /* cache mirror write consistency disk part     */
	ushort_t	lv_minor;	/* LV minor number              */
	ushort_t	ltgshift;	/* LTG sector shift value       */
	uint_t		lv_ltg;		/* LV logical track group       */
};

/*
 * This structure must be maintained to be 1 block in length (DEV_BSIZE).
 * This also implies the maximum number of write consistency cache entries.
 */
#define MWC_SIZE ((DEV_BSIZE-2*sizeof(struct timeval))/sizeof(struct ca_mwc_dp))

struct mwc_rec {        /* mirror write consistency disk record         */
    struct timeval      b_tmstamp;      /* Time stamp at beginning of block */
    struct ca_mwc_dp    ca_p1[MWC_SIZE]; /* Reserve 62 part 1 structures    */
    struct timeval      e_tmstamp;      /* Time stamp at end of block       */
};

#endif  /* _LVM_VGRES_H_ */

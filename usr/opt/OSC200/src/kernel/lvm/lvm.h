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
 *	@(#)lvm.h	9.2	(ULTRIX/OSF)	10/18/91
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

#ifndef _LVM_H_
#define _LVM_H_

/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/ioctl.h>

/*
 * This file contains the data structures and definitions used to communicate
 * between user space code and the logical volume manager.
 */

/*
 * lxmap - logical extent to physical volume, physical extent map type.
 */
struct lxmap {
    ushort_t lx_num;		/* logical extent in question.     */
    ushort_t pv_key;		/* corresponding physical volume.  */
    ushort_t px_num;		/* corresponding physical extent.  */
    ushort_t status;		/* status of that physical extent. */
};
typedef struct lxmap lxmap_t;
#define LVM_PXSTALE	0x01	/* Physical copy is out of date. */
#define	LVM_PXMISSING	0x02	/* Physical copy is on a missing volume */

/*
 * pxmap - physical extent to logical volume, logical extent map type.
 */
struct pxmap {
    ushort_t lv_minor;		/* logical volume minor number.	  */
    ushort_t lv_extent;		/* logical extent on volume.	  */
    ushort_t status;		/* status of the physical extent. */
};
typedef struct pxmap pxmap_t;

/*
 * lv_uniqueID - unique identifier type for various structures.
 */
struct lv_uniqueID {
    uint_t id1;		/* First part of ID.  */
    uint_t id2;		/* Second part of ID. */
};
typedef struct lv_uniqueID lv_uniqueID_t;

/* lv_uniqueID macros */
#define	zeroID(p)	 ((p)->id1 == 0 && (p)->id2 == 0)
#define	clearID(p)	 ((p)->id1 = 0, (p)->id2 = 0)
#define equalID(p1, p2)  ((p1)->id1 == (p2)->id1 && (p1)->id2 == (p2)->id2)

/* Activatevg flags */
#define	LVM_ACTIVATE_LVS	0x01	/* Allow logical volume opens.	 */
#define	LVM_ALL_PVS_REQUIRED	0x04	/* All PVs must be present to	 */
					/*    activate.			 */
#define	LVM_NONMISSING_PVS_REQUIRED 0x08	/* All PVs which were not  */
					/* previously known as missing are */
					/* required in order to activate.  */

/*
 * lv_changepv - change the attributes of a physical volume.
 */
struct lv_changepv {
    ushort_t pv_key;		/* Physical volume identifier	     */
    ushort_t pv_flags;		/* Logical OR of following flags     */
    ushort_t maxdefects;	/* Maximum relocated defects allowed */
};

#define	LVM_PVDEFINED	0x01	/* This entry is used 		    */
#define	LVM_PVNOALLOC	0x02	/* No extent allocation is allowed. */
#define	LVM_NOVGDA	0x04	/* Physical volume contains a VGDA. */
#define	LVM_PVRORELOC	0x08	/* No new defects relocated 	    */
#define	LVM_PVMISSING	0x10	/* Physical volume is missing.	    */
#define	LVM_NOTATTACHED	0x20	/* Physical volume is not attached. */

/*
 * lv_createvg - create a volume group.
 */
struct lv_createvg {
    char	  *path;	/* Pathname of 1st physical volume.   */
    lv_uniqueID_t vg_id;	/* Volume group ID to set.	      */
    ushort_t	  pv_flags;	/* Logical OR of flags (see chart)    */
    ushort_t	  maxlvs;	/* Max # logical volumes allowed.     */
    ushort_t	  maxpvs;	/* Max # physical volumes allowed.    */
    ushort_t	  maxpxs;	/* Max # physical extents allowed.    */
    uint_t	  pxsize;	/* Physical extent size.	      */
    uint_t	  pxspace;	/* Space allocated for each extent.   */
    ushort_t	  maxdefects;	/* Maximum relocated defects allowed. */
};

/*
 * lv_installpv - install a physical volume in a volume group
 */
struct lv_installpv {
    char     *path;		/* The physical volume pathname.      */
    uint_t   pxspace;		/* Space to allocate for each extent. */
    ushort_t pv_flags;		/* Same as lv_statuspv.pv_flags.      */
    ushort_t maxdefects;	/* Maximum relocated defects allowed. */
};

/*
 * lv_lvsize - extend, reduce, or querymap a logical volume.
 */
struct lv_lvsize {
    ushort_t minor_num;		/* LV's device minor number.		    */
    uint_t   size;		/* The number of physical extents affected. */
    lxmap_t *extents;		/* Extents affected.			    */
};

/*
 * lv_querylv - query the attributes of a logical volume.
 */
struct lv_querylv {
    ushort_t minor_num;		/* LV's device minor number.	      */
    uint_t  numpxs;		/* Current # of physical extents.     */
    ushort_t numlxs;		/* Current # of logical extents.      */
    ushort_t maxlxs;		/* Maximum number if logical extents  */
    ushort_t lv_flags;		/* Logical flag word, see below.      */
    ushort_t sched_strat;	/* Mirror write scheduling strategy.  */
    ushort_t maxmirrors;	/* Maximum number of mirrors allowed. */
};

/*
 * lv_querypv - Query the status of a physical volume.
 */
struct lv_querypv {
    ushort_t pv_key;		/* Physical volume identifier.	       */
    ushort_t pv_flags;		/* Logical OR of the flags, see below. */
    ushort_t px_count;		/* Number of physical extents.	       */
    ushort_t px_free;		/* Number of free physical extents.    */
    uint_t   px_space;		/* Space allocated to each physical X. */
    dev_t    pv_rdev;		/* Current devno assigned to pvol.     */
    ushort_t maxdefects;	/* Maximum relocated defects allowed.  */
    ushort_t bbpool_len;	/* Limit of relocated defects.	       */
};

/*
 * lv_querypvmap - Query the extent map of a physical volume.
 */
struct lv_querypvmap {
    ushort_t pv_key;		/* Physical volume identifier. 	      */
    ushort_t numpxs;		/* number of physical extents. 	      */
    pxmap_t  *map;		/* map of volume's physical extents.  */
};

/*
 * lv_querypvpath - Query the status of a physical volume by path.
 */
struct lv_querypvpath {
    char     *path;		/* Physical volume pathname.	       */
    ushort_t pv_key;		/* Physical volume identifier.	       */
    ushort_t pv_flags;		/* Logical OR of the flags, see below. */
    ushort_t px_count;		/* Number of physical extents.	       */
    ushort_t px_free;		/* Number of free physical extents.    */
    uint_t   px_space;		/* Space allocated to each physical X. */
    dev_t    pv_rdev;		/* Current devno assigned to pvol.     */
    ushort_t maxdefects;	/* Maximum relocated defects allowed.  */
    ushort_t bbpool_len;	/* Limit of relocated defects.	       */
};

/*
 * lv_querypvs - query the physical volume list from the volume group.
 */
struct lv_querypvs {
    ushort_t numpvs;		/* Current physical volume count. */
    ushort_t *pv_keys;		/* Pointer to the key list.	  */
};

/*
 * lv_queryvg - retrieve information about a volume group.
 */
struct lv_queryvg {
    lv_uniqueID_t vg_id;	/* Volume group ID.		   */
    ushort_t	  maxlvs;	/* Max # logical volumes allowed.  */
    ushort_t	  maxpvs;	/* Max # physical volumes allowed. */
    ushort_t	  maxpxs;	/* Max # phys extents/phys volume. */
    uint_t	  pxsize;	/* Physical extent size.	   */
    ushort_t	  freepxs;	/* Number of free extents.	   */
    ushort_t	  cur_lvs;	/* Current logical volume count.   */
    ushort_t	  cur_pvs;	/* Current physical volume count.  */
    ushort_t	  status;	/* Status of the volume group.	   */
};

#define LVM_VGACTIVATED	0x01	/* Volume group has been activated. */
#define LVM_LVSACTIVATED 0x02	/* Logical Volumes have been activated. */

/*
 * lv_realloclv - reallocate logical extents
 */
struct lv_realloclv {
    ushort_t sourcelv;		/* The source logical volume.	*/
    ushort_t destlv;		/* The dest. logical volume.	*/
    uint_t   size;		/* The number of physical extents affected. */
    lxmap_t *extents;		/* The extents being realloc'ed */
};

/*
 * lv_resynclx - resynchronize a logical extent
 */
struct lv_resynclx {
    ushort_t minor_num;		/* Logical volume minor number	 */
    ushort_t lx_num;		/* Logical extent to resync	 */
};

/*
 * lv_statuslv - change the attributes of a logical volume.
 */
struct lv_statuslv {
    ushort_t minor_num;		/* LV's device minor number.	      */
    ushort_t maxlxs;		/* Maximum number of logical extents. */
    ushort_t lv_flags;		/* Logical flag word, see below.      */
    ushort_t sched_strat;	/* Mirror write scheduling strategy.  */
    ushort_t maxmirrors;	/* Maximum number of mirrors allowed. */
};

/* Logical Volume flags. */
#define	LVM_LVDEFINED	0x01	/* Logical volume entry defined.	  */
#define	LVM_DISABLED	0x02	/* Logical volume unavailable for use.	  */
#define	LVM_RDONLY	0x04	/* Read-only logical volume.		  */
#define	LVM_NORELOC	0x08	/* Bad blocks are not relocated.	  */
#define	LVM_VERIFY	0x10	/* Verify all writes.			  */
#define LVM_STRICT	0x20	/* Allocate mirrors on distinct PV's	  */
#define LVM_NOMWC	0x40	/* No mirror consistency on this LV	  */

/* LVM mirror write policies. */
#define	LVM_RESERVED	0	/* Write to the reserved area.		  */
#define	LVM_SEQUENTIAL	1	/* Write mirror copies sequentially.	  */
#define	LVM_PARALLEL	2	/* Write mirror copies in parallel.	  */

/* Maximum values - and why... */
#define	LVM_MAXLXS	65535	/* Bounded by lx_num in px entry.	  */
#define	LVM_MAXCOPIES	3	/* Maximum of 1 original copy and 2	  */
				/*    mirrors.				  */
#define	LVM_MAXPXS	65535	/* Bounded by size of px_count in PV_head */
#define	LVM_MAXPVS	255	/* Bounded by pv_num in PV header.	  */
#define	LVM_MAXLVS	255	/* Implementation limit.		  */

#define	LVM_MINPXSIZE	0x100000	/* Smallest physical extent size  */
#define	LVM_MAXPXSIZE	0x10000000	/* Largest physical extent size   */

struct lv_option {
    ushort_t opt_avoid;		/* Raw device mirror avoidance mask	*/
    ushort_t opt_options;	/* I/O options. see below. */
};

/*
 * Raw device I/O options - these do not persist after the raw device is
 * closed.  Avoid mask indicates mirrors not to eligible for reads. 0 means
 * all copies used, 7 means no copies eligible.
 * opt_options: LVM_VERIFY (0x10), LVM_NORELOC (0x08) 
 * Only valid against logical volume device, not volume group device.
 */
#define LVM_MIRAVOID ((1 << LVM_MAXCOPIES) - 1)

/*
 * LVM driver I/O control commands.
 */
#define	LVM_ACTIVATEVG		_IOW ('v',   1, int)
#define	LVM_ATTACHPV		_IO  ('v',   2) /* char *path */
#define	LVM_CHANGELV		_IOW ('v',   3, struct lv_statuslv)
#define	LVM_CHANGEPV		_IOW ('v',   4, struct lv_changepv)
#define	LVM_CREATELV		_IOW ('v',   5, struct lv_statuslv)
#define	LVM_CREATEVG		_IOW ('v',   6, struct lv_createvg)
#define	LVM_DEACTIVATEVG	_IO  ('v',   7)
#define	LVM_DELETELV		_IOW ('v',   8, int)
#define	LVM_DELETEPV		_IOW ('v',   9, int)
#define	LVM_EXTENDLV		_IOW ('v',  10, struct lv_lvsize)
#define	LVM_INSTALLPV		_IOW ('v',  11, struct lv_installpv)
#define	LVM_QUERYLV		_IOWR('v',  12, struct lv_querylv)
#define	LVM_QUERYLVMAP		_IOWR('v',  13, struct lv_lvsize)
#define	LVM_QUERYPV		_IOWR('v',  14, struct lv_querypv)
#define	LVM_QUERYPVMAP		_IOWR('v',  15, struct lv_querypvmap)
#define	LVM_QUERYPVPATH		_IOWR('v',  16, struct lv_querypvpath)
#define	LVM_QUERYPVS		_IOWR('v',  17, struct lv_querypvs)
#define	LVM_QUERYVG		_IOWR('v',  18, struct lv_queryvg)
#define	LVM_REDUCELV		_IOW ('v',  19, struct lv_lvsize)
#define	LVM_RESYNCLV		_IOW ('v',  20, int)
#define	LVM_RESYNCLX		_IOW ('v',  21, struct lv_resynclx)
#define	LVM_RESYNCPV		_IOW ('v',  22, int)
#define	LVM_SETVGID		_IOW ('v',  23, lv_uniqueID_t)
#define	LVM_OPTIONGET		_IOWR('v',  24, struct lv_option)
#define	LVM_OPTIONSET		_IOW ('v',  25, struct lv_option)
#define	LVM_REALLOCLV		_IOW ('v',  26, struct lv_realloclv)
#define	LVM_REMOVEPV		_IOW ('v',  27, int)

/* Debugging hooks */
#define LVM_DEBUG_STALEPX	_IOWR('v', 100, struct lv_lvsize)

#endif	/* _LVM_H_ */

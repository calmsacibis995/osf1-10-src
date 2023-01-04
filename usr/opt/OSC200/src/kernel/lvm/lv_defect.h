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
 *	@(#)$RCSfile: lv_defect.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:19:14 $
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

#ifndef _LV_DEFECT_H_
#define _LV_DEFECT_H_

/*
 * Defect management definitions
 */

/*
 * Defect handling defines
 */

#define MAX_SWRETRY	3		/* Maximum relocation attempts   */
#define HASHSIZE	64		/* number of defect hash classes */

/* Number of defect entries in a bad block directory block */
#define DEFECTPERBLK   64		/* Assumes sizeof(lv_bblk_t) = 8, */
					/* and DEV_BSIZE = 512		  */
#define DEFECTSHIFT     6               /* log2(DEFECTPERBLK)		  */

/* Values for pb_op used while attempting bad block relocation */
#define FIX_READ_ERROR	     1	/* request is a fix of a EMEDIA read error  */
#define FIX_ESOFT	     2	/* request is a fix of a ESOFT read error   */
#define BBDIR_UPDATE_PENDING 3	/* a defect directory update is in progress */

/* Bad block relocation status values: */
#define REL_DONE	0		/* software relocation completed    */
#define REL_PENDING	1		/* software relocation in progress  */
#define REL_DEVICE	2		/* device (HW) relocation requested */
#define REL_DESIRED	8		/* relocation desired - high bit on */

/* Bad block defect reason values: */
#define DEFECT_MFR	0x01		/* Manufacturer found defect	  */
#define DEFECT_DIAG	0x0A		/* Diagnostics found defect	  */
#define DEFECT_SYS	0x0B		/* System found defect		  */
#define DEFECT_MFRTST	0x0C		/* Manufacturer test found defect */

/*
 * Defect-related structure declarations
 */

/*
 *  Bad block directory entry.
 */
struct lv_bblk {
	uint_t	defect_reason;		/* Reason and defect PSN    */
	uint_t	alternate_status;	/* Status and alternate PSN */
};
typedef struct lv_bblk lv_bblk_t;

/*
 *  Defect hash table chain entries
 */
struct lv_defect {
	struct lv_defect *next;		/* Next entry in chain     */
	struct lv_bblk *defect;		/* Actual defect structure */
};
typedef struct lv_defect lv_defect_t;

/*
 * Various defect macros
 */

/* Macros to pull apart the bad block entries. */
#define	BB_STATUS(x)	((((x)->alternate_status) >> 28) & 0xf)
#define	BB_ALTERNATE(x)	((((x)->alternate_status) & 0x0fffffff))
#define	BB_REASON(x)	((((x)->defect_reason) >> 28) & 0xf)
#define	BB_DEFECT(x)	((((x)->defect_reason) & 0x0fffffff))

/* Macros to set areas in the bad block entries. */
#define BB_SET_HIGH(f, x, v) \
	{(x)->f &= 0x0fffffff; (x)->f |= ((v << 28) & 0xf0000000);}
#define BB_SET_LOW(f, x, v)  \
	{(x)->f &= 0xf0000000; (x)->f |= (v & 0x0fffffff);}
#define	SET_BB_STATUS(x,v)      BB_SET_HIGH(alternate_status, x, v)
#define	SET_BB_ALTERNATE(x,v)   BB_SET_LOW(alternate_status, x, v)
#define	SET_BB_REASON(x,v)      BB_SET_HIGH(defect_reason, x, v)
#define	SET_BB_DEFECT(x,v)      BB_SET_LOW(defect_reason, x, v)

/* Macro to return index into the bad block hash table for this block number */
#define BBHASHINX(blkno)  (BLK2TRK(blkno) & (HASHSIZE -1))

#define LV_QUEUEIO(PV,BP)				\
MACRO_BEGIN						\
	LOCK_INTERRUPT(&(PV)->pv_intlock);		\
	LV_QUEUE_APPEND(&(PV)->pv_ready_Q,(BP));	\
	(PV)->pv_totxf++;				\
	(PV)->pv_curxfs++;				\
	UNLOCK_INTERRUPT(&(PV)->pv_intlock);		\
MACRO_END

/*
 * Macro to return pointer to the defect table entry for the block number
 * specified in the pbuf pointer argument.
 */
#define BBHASHPTR(p)  ((p)->pb_pvol->pv_defects[BBHASHINX((p)->pb.b_blkno)])

/* Macro to determine if physical layer is being called for read error fixup */
#define IS_FIXUP(pb)	((pb)->pb_sched == lv_fixup)

#endif  /* _LV_DEFECT_H_ */

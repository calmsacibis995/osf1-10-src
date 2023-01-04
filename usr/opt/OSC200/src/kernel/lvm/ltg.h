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
 *	@(#)$RCSfile: ltg.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:18:33 $
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

#ifndef LVPAGESIZE
#include <sys/param.h>

/*
 * LVPAGESIZE and LVPGSHIFT were originally PAGESIZE and PGSHIFT from param.h.
 * There were renamed and moved to here to isolate LVM from the changable
 * system parameters that would have undesirable effects on LVM functionality.
 */
#define LVPAGESIZE	NBPG		  /* Page size in bytes, i.e. 4096 */
#define LVPGSHIFT	PGSHIFT		  /* log 2 of LVPAGESIZE i.e. 12 */

#define BPPG		(LVPAGESIZE/DEV_BSIZE) /* disk Blocks Per PaGe, i.e. 8*/
#define BPPGSHIFT	(LVPGSHIFT-DEV_BSHIFT) /* log 2 of BPPG, i.e. 3 */
#define PGPTRK		(NBBY*sizeof(int))  /* PaGes Per logical TRacK group */
						/* (32) */
#define TRKSHIFT	5		  /* log base 2 of PGPTRK */
#define LTGSHIFT	(TRKSHIFT+BPPGSHIFT)/* logical track group log base 2 */
#define BYTEPTRK	PGPTRK*LVPAGESIZE /* bytes per logical track group */
#define BLKPTRK		PGPTRK*BPPG	  /* blocks per logical track group */

#define BLK2BYTE(nblocks)	(((unsigned)(nblocks))<<DEV_BSHIFT)
#define BYTE2BLK(nbytes)	(((unsigned)(nbytes)) >>DEV_BSHIFT)
#define BLK2PG(blk)		(((unsigned)(blk))    >>BPPGSHIFT)
#define PG2BLK(pageno)		(         (pageno)    <<BPPGSHIFT)
#define BLK2TRK(blk)		(((unsigned)(blk))    >>LTGSHIFT)
#define TRK2BLK(t_no)		(((unsigned)(t_no))   <<LTGSHIFT)
#define PG2TRK(pageno)		(((unsigned)(pageno)) >>TRKSHIFT)

#if !defined(b_work)
#define b_work	b_driver_un_2.longvalue
#endif

/* define for b_error value (only used by lv_block.c) */
#define ELBBLOCKED 255		/* this logical request is blocked by	*/
				/* another one in progress		*/

#define WORKQ_SIZE	64	/* size of LVs work in progress queue	    */
/*
 * work_Q hash algorithm - must map requests in the same track
 * group to the same hash queue.
 */
#define LV_HASH(lb)	\
	(BLK2TRK(lb->b_blkno) & (WORKQ_SIZE-1))

struct h_anchor {
	struct buf *bp;
};

#endif /* LVPAGESIZE */

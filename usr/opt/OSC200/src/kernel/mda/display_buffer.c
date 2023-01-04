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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: display_buffer.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:58 $";
#endif 
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include "mmax_mp.h"

#include "mda.h"
#include "sys/types.h"
#include "sys/buf.h"
#include "sys/mount.h"
#include "sys/kernel.h"
#include "sys/vnode.h"

extern	char 	*truefalse();

static	char	*buf_flag_names[] = {
	"Read",
#if	!MMAX_MP
	"Done",
#else	!MMAX_MP
	"(Bad) Done",
#endif	!MMAX_MP
	"Error",
#if	!MMAX_MP
	"Busy",
#else	!MMAX_MP
	"(Bad) Busy",
#endif	!MMAX_MP
	"Phys",
#if	!MMAX_MP
	"Xxx",
#else	!MMAX_MP
	"(Bad) Xxx",
#endif	!MMAX_MP
#if	MMAX_MP
	"Wantfree",
#else	MMAX_MP
	"Wanted",
#endif	MMAX_MP
	"Age",
	"Async",
	"Delwri",
	"Tape",
#if	!MACH
	"Uarea",
	"Paget",
#else	!MACH
	"(Bad) Uarea",
	"(Bad) Paget",
#endif	!MACH
#if	!MMAX_MP
	"Dirty",
#else	!MMAX_MP
	"(Bad) Dirty",
#endif	!MMAX_MP
#if	!MACH
	"Pgin",
#else	!MACH
	"(Bad) Pgin",
#endif	!MACH
	"Cache",
	"Inval",
	"Locked",
	"Head",
	"Useless",
	"Bad",
#if	!MMAX_MP
	"Call",
#else	!MMAX_MP
	"(Bad) Call",
#endif	!MMAX_MP
	"Raw",
	"Nocache",
	"Private",
};

#define	SZOFBUFNAMES	(sizeof(buf_flag_names)/sizeof(char *))

/*
 * Which = 0	- From start of buffer
 *	   1	- From lock
 *	   2	- From event
 */

display_buffer(vbp, bp, which)
unsigned int vbp;
struct buf *bp;
int	which;
{
	int	i;
	struct	buf *fbp;
	char	*p, sbuf[64];

	if(which) {
#if	MMAX_MP
		fbp = (struct buf *)0;
		i = ((which == 1) ? (int)(&fbp->b_lock) :
		                     (int)(&fbp->b_iocomplete.ev_event));
		printf("Buffer @ %#x\n", vbp - i);
		bp = (struct buf *)((char *)bp - (char *)i);
#else	MMAX_MP
	printf("Non-mp kernels have no %s\n",
	       (which == 1) ? "lock" : "event");
#endif	MMAX_MP
	}

	bzero(sbuf, sizeof(sbuf));
	sdiagram(sbuf, bp->b_flags, buf_flag_names, SZOFBUFNAMES);
	printthree("sxx", "b_flags", "b_forw", "b_back",
		sbuf, bp->b_forw, bp->b_back);
	printthree("xxx", "av_forw", "av_back", "b_blockf",
		bp->av_forw, bp->av_back, bp->b_blockf);
	printthree("xxx", "b_blockb", "b_bcount", "b_bufsize",
		bp->b_blockb, bp->b_bcount, bp->b_bufsize);
	printthree("xxx", "b_error", "b_dev", "b_addr",
		bp->b_error, bp->b_dev, bp->b_un.b_addr);
	printthree("xxx", "b_lblkno", "b_blkno", "b_resid",
		bp->b_lblkno, bp->b_blkno, bp->b_resid);
#if	!MMAX_MP
	printthree("x  ", "b_proc", "", "",
		bp->b_proc, 0, 0);
#endif	!MMAX_MP
#if	MMAX_MP
	printthree("x  ", "hash_chain", "", "",
		bp->b_hash_chain, 0, 0);
	printf("b_lock:\n");
	display_lock(&bp->b_lock);
	printthree("sx ", "lock", "event", "",
		truefalse(bp->b_iocomplete.ev_slock.lock_data & 0xff),
		bp->b_iocomplete.ev_event, 0);
#else	MMAX_MP
	printthree("xx ", "b_iodone", "b_pfcent", "",
		   bp->b_iodone, bp->b_pfcent, 0);
#endif	MMAX_MP
	printthree("xxx", "b_vp", "b_rcred", "b_wcred",
		bp->b_vp, bp->b_rcred, bp->b_wcred);
	printthree("xx ", "b_dirtyoff", "b_dirtyend", "",
		   bp->b_dirtyoff, bp->b_dirtyend, 0);
}

display_bfreelist(vbp)
struct buf *vbp;
{
	struct buf *bp = vbp;

	do {
		if(phys(bp, &bp, ptb0) != SUCCESS)
			break;

		bp = (struct buf *)MAPPED(bp);
		printthree("xxx", "b_flags", "av_forw", "av_back",
			bp->b_flags, bp->av_forw, bp->av_back);
		bp = bp->av_forw;
	} while( bp != vbp);
}


display_bhash_chain(vbp)
struct buf *vbp;
{
	struct buf *bp = vbp;

	do {
		if(phys(bp, &bp, ptb0) != SUCCESS)
			break;

		bp = (struct buf *)MAPPED(bp);
		printthree("xxx", "b_flags", "b_forw", "b_back",
			bp->b_flags, bp->b_forw, bp->b_back);
		bp = bp->b_forw;
	} while( bp != vbp);
}

display_bvnode_list(vbp)
struct buf *vbp;
{
	struct buf *bp = vbp;

	do {
		if(phys(bp, &bp, ptb0) != SUCCESS)
			break;

		bp = (struct buf *)MAPPED(bp);
		printthree("xxx", "b_flags", "b_blockf", "b_blockb",
			bp->b_flags, bp->b_blockf, bp->b_blockb);
		bp = bp->b_blockf;
	} while( bp != (struct buf *)0);
}

display_vnode_clnlist(vp)
struct vnode *vp;
{
	struct buf *bp = vp->v_cleanblkhd;

	if (bp == (struct buf *)0) {
		printf("clean list is empty\n");
		return;
	}
	printthree("x  ", "v_cleanblkhd", "", "",
		vp->v_cleanblkhd, 0, 0);
	do {
		if(phys(bp, &bp, ptb0) != SUCCESS)
			break;

		bp = (struct buf *)MAPPED(bp);
		printthree("xxx", "b_flags", "b_blockf", "b_blockb",
			bp->b_flags, bp->b_blockf, bp->b_blockb);
		bp = bp->b_blockf;
	} while( bp != (struct buf *)0);
}

display_vnode_drtylist(vp)
struct vnode *vp;
{
	struct buf *bp = vp->v_dirtyblkhd;

	if (bp == (struct buf *)0) {
		printf("dirty list is empty\n");
		return;
	}
	printthree("x  ", "v_dirtyblkhd", "", "",
		vp->v_dirtyblkhd, 0, 0);
	do {
		if(phys(bp, &bp, ptb0) != SUCCESS)
			break;

		bp = (struct buf *)MAPPED(bp);
		printthree("xxx", "b_flags", "b_blockf", "b_blockb",
			bp->b_flags, bp->b_blockf, bp->b_blockb);
		bp = bp->b_blockf;
	} while( bp != (struct buf *)0);
}

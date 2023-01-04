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
static char	*sccsid = "@(#)$RCSfile: ufs_machdep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:20:24 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <sys/param.h>
#include <sys/systm.h>
#include <ufs/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/vm.h>

/*
 * Machine dependent handling of the buffer cache.
 */

/*
 * Expand or contract the actual memory allocated to a buffer.
 * If no memory is available, release buffer and take error exit
 */
allocbuf(tp, size)
	register struct buf *tp;
	int size;
{
	register struct buf *bp, *ep;
	int sizealloc, take;
	int s;

	sizealloc = roundup(size, CLBYTES);
	/*
	 * Buffer size does not change
	 */
	if (sizealloc == tp->b_bufsize)
		goto out;
	/*
	 * Buffer size is shrinking.
	 * Place excess space in a buffer header taken from the
	 * BQ_EMPTY buffer list and placed on the "most free" list.
	 * If no extra buffer headers are available, leave the
	 * extra space in the present buffer.
	 */
	if (sizealloc < tp->b_bufsize) {
		ep = bfreelist[BQ_EMPTY].av_forw;
		if (ep == &bfreelist[BQ_EMPTY])
			goto out;
		s = splbio();
		bremfree(ep);
		ep->b_flags |= B_BUSY;
		splx(s);
		pagemove(tp->b_un.b_addr + sizealloc, ep->b_un.b_addr,
		    (int)tp->b_bufsize - sizealloc);
		ep->b_bufsize = tp->b_bufsize - sizealloc;
		tp->b_bufsize = sizealloc;
		ep->b_flags |= B_INVAL;
		ep->b_bcount = 0;
		brelse(ep);
		goto out;
	}
	/*
	 * More buffer space is needed. Get it out of buffers on
	 * the "most free" list, placing the empty headers on the
	 * BQ_EMPTY buffer header list.
	 */
	while (tp->b_bufsize < sizealloc) {
		take = sizealloc - tp->b_bufsize;
		bp = getnewbuf();
		if (take >= bp->b_bufsize)
			take = bp->b_bufsize;
		pagemove(&bp->b_un.b_addr[bp->b_bufsize - take],
		    &tp->b_un.b_addr[tp->b_bufsize], take);
		tp->b_bufsize += take;
		bp->b_bufsize = bp->b_bufsize - take;
		if (bp->b_bcount > bp->b_bufsize)
			bp->b_bcount = bp->b_bufsize;
		if (bp->b_bufsize <= 0) {
			bremhash(bp);
			binshash(bp, &bfreelist[BQ_EMPTY]);
			bp->b_dev = (dev_t)NODEV;
			bp->b_error = 0;
			bp->b_flags |= B_INVAL;
		}
		brelse(bp);
	}
out:
	tp->b_bcount = size;
	return (1);
}

/*
 * Release space associated with a buffer.
 */
bfree(bp)
	struct buf *bp;
{

	bp->b_bcount = 0;
}

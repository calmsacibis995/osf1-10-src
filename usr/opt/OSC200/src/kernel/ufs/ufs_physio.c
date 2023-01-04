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
static char	*sccsid = "@(#)$RCSfile: ufs_physio.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/05/27 19:26:35 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/* 
 * OSF/1 Release 1.0.1
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
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)vm_swp.c	7.10 (Berkeley) 9/14/89
 */

/*
 * Major changes from Berkeley:
 *	Mach VM is very different... swapping code has been removed.
 */
/*
 * Revision History
 *
 * 8-Aug-91 -- prs
 *	Added back a kludge for tape drives which will break out of the physio
 *	loop after a partial transfer.
 */
 
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/vnode.h>

#include <kern/task.h>
#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <sys/dk.h>	/* for SAR counters */

/*
 * Raw I/O. The arguments are
 *	The strategy routine for the device
 *	A buffer, which will always be a special buffer
 *	  header owned exclusively by the device for this purpose
 *
 *	The device number
 *	Read/write flags (B_READ/B_WRITE, B_WRITEV)
 * Essentially all the work is computing physical addresses and
 * validating them.
 * If the user has the proper access privilidges, the process is
 * marked 'delayed unlock' and the pages involved in the I/O are
 * faulted and locked. After the completion of the I/O, the above pages
 * are unlocked.
 */
physio(strat, bp, dev, rw, mincnt, uio)
	int (*strat)(); 
	register struct buf *bp;
	dev_t dev;
	int rw;
	u_int (*mincnt)();
	struct uio *uio;
{
	register struct iovec *iov;
	register int requested, done;
	char *a;
	int error = 0;
	int once = 0;
	vm_offset_t	start, end;

	/*
	 * Check for a special AIO request. If so, service and return.
	 */
	if (uio->uio_rw == UIO_AIORW) {
		register struct iovec *vec = uio->uio_iov;
		vec->iov_base = (caddr_t) strat;
		(++vec)->iov_base = (caddr_t) mincnt;
		return EAIO;
	}

	LASSERT(BUF_LOCK_HOLDER(bp));

        /* global table() system call counter (see table.h) */
        if (rw)
                ts_phread++;
        else
                ts_phwrite++;

	for (; uio->uio_iovcnt; uio->uio_iov++, uio->uio_iovcnt--) {
		iov = uio->uio_iov;
		if (uio->uio_segflg == UIO_USERSPACE &&
		    !useracc(iov->iov_base, (u_int)iov->iov_len,
		    (rw & B_READ) ? B_WRITE : B_READ)) {
			error = EFAULT;
			break;
		}
		bp->b_proc = u.u_procp;
		bp->b_error = 0;
		bp->b_un.b_addr = iov->iov_base;
		while (iov->iov_len > 0) {
			LASSERT(BUF_LOCK_HOLDER(bp));
			bp->b_flags = B_PHYS | B_RAW | rw;
			event_clear(&bp->b_iocomplete);
			bp->b_dev = dev;
			bp->b_blkno = btodb(uio->uio_offset);
			bp->b_bcount = iov->iov_len;
			/*
			 * Set once to zero... this will allow 
			 * One request for each IOV for tapes.
			 */
			once = 0;
			(*mincnt)(bp);

			/*
			 * For devices that require only one time
			 * around while loop (TAPES ) check the 
			 * results from call to drivers minphys()
			 */
			if(( bp->b_flags & B_ERROR) != NULL){
			    /*
			     * If driver's minphys() doesn't want
			     * anthing to go it sets resid == b_bcount
			     * and b_error to proper value. (TAPES)
			     */
			    if( bp->b_bcount == bp->b_resid ){
				error = bp->b_error;
				break;
			    }
			    else {
				/*
				 * Driver has indicated it wants the loop
				 * done only once. Clear error flag.
				 */
				bp->b_flags &= ~B_ERROR;
				once = 1;
			    }
			}

			requested = bp->b_bcount;
			a = bp->b_un.b_addr;
			start = trunc_page(a);
			end = round_page(a + requested);
			/* Assume already wired if sys space */
			if (uio->uio_segflg != UIO_SYSSPACE &&
			    vm_map_pageable(current_task()->map, start, end,
				(rw & B_READ) ? VM_PROT_WRITE : VM_PROT_READ)
			    != KERN_SUCCESS) {
				error = EFAULT;
				break;
			}
			(*strat)(bp);
			error = biowait(bp);
			LASSERT(BUF_LOCK_HOLDER(bp));

			if (uio->uio_segflg == UIO_USERSPACE)
				(void)vm_map_pageable(current_task()->map,
						start, end, VM_PROT_NONE);

			done = bp->b_bcount - bp->b_resid;
			bp->b_un.b_addr += done;
			iov->iov_len -= done;
			uio->uio_resid -= done;
			uio->uio_offset += done;
			/*
			 * Once indicates only once thru while loop
			 */
			if ((done < requested) || (done == 0) || error || once)
				break;
		}
		bp->b_flags &= ~(B_PHYS | B_RAW);
		if (( done < requested ) || (done == 0) || error)
			break;
	}
	/* Note: rwuio will cause partial transfers to 
	 * be reported as "success", even though we return an error.
	 */
	return (error);
}

#ifdef	ibmrt
#include <sc.h>
#else
#define NSCC	0
#endif

#if	NSCC > 0
#define MAXPHYS	(64 * 512)	/* don't increase beyond NDMAXIO */
#else
#define MAXPHYS	(63 * 1024)
#endif

#if	EXL
#undef  MAXPHYS
#define MAXPHYS (32 * 4096)     /* EXL can take 32 pages for dma */
#endif

u_int
minphys(bp)
	struct buf *bp;
{
	LASSERT(BUF_LOCK_HOLDER(bp));
	if (bp->b_bcount > MAXPHYS)
		bp->b_bcount = MAXPHYS;
}

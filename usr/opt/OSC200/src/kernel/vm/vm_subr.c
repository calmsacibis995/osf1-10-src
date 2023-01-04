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
static char *rcsid = "@(#)$RCSfile: vm_subr.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/14 14:00:43 $";
#endif
#include <kern/lock.h>
#include <sys/buf.h>
#include <kern/sched_prim.h>
#include <sys/param.h>
#include <kern/parallel.h>
#include <sys/conf.h>
#include <sys/lwc.h>

udecl_simple_lock_data(extern,ubc_iodone_lock)
udecl_simple_lock_data(extern,vm_swap_swdone_lock)

extern struct buf *ubc_iodone_buf;
extern struct buf *swdone;
extern lwc_id_t ubc_lwc_id, vm_lwc_id;

vm_iodone(register struct buf *bp)
{
	register int s;
	register simple_lock_t l;
	register struct buf **bpp;
	register lwc_id_t *lwcidp;


	if ((bp->b_flags & (B_SWAP|B_READ|B_ASYNC)) == (B_SWAP|B_READ)) 
		event_post(&bp->b_iocomplete);
	else {
	
		if (bp->b_flags & B_UBC) { 
			l = simple_lock_addr(ubc_iodone_lock);
			bpp = &ubc_iodone_buf;
			lwcidp = &ubc_lwc_id;
		}
		else {
			l = simple_lock_addr(vm_swap_swdone_lock);
			bpp = &swdone;
			lwcidp = &vm_lwc_id;
		}

		s = splbio();
		usimple_lock(l);
		if (*bpp != BUF_NULL) binstailfree(bp, *bpp);
		else {
			*bpp = bp;
			bp->av_back = bp->av_forw = bp;
		}
		usimple_unlock(l);
		(void) splx(s);
		event_post(&bp->b_iocomplete);
		lwc_interrupt(*lwcidp);
	}
}

vm_iocheck(register struct buf *bp, 
	long low)
{
	long size;

	BDEVSW_PSIZE(major(bp->b_dev), bp->b_dev, size);
	if (size <= 0) goto failed;
	else if ((bp->b_blkno < btodb(low)) || 
		((bp->b_blkno + btodb(bp->b_bcount)) > size)) goto failed;
	return;
failed:
	printf("vm_iocheck: dev = 0x%x blk = 0x%x low = 0x%x psize = 0x%x\n",
		bp->b_dev, bp->b_blkno, low, size);
	panic("vm_iocheck: bad I/O");
}

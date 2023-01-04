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
static char	*sccsid = "@(#)$RCSfile: do_physio.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:08:19 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 */


#include  <sys/buf.h>
#include  <vm/vm_kern.h>

do_physio(bp, strat)
	struct buf *bp;
	int (*strat)();
{

	unsigned usraddr, addr, kvaddr;

	/* Disk access is faster via programmed I/O than by DMA on
	 * the AT. We map the user's pages into kernel space because
	 * we don't know what context we'll be running in when the
	 * disk operation completed interrupt occurs.
	 */
	usraddr = (unsigned) bp->b_un.b_addr;
	if (usraddr < VM_MIN_KERNEL_ADDRESS) { 
		/* if not on sector boundary */
		kvaddr = kmem_alloc(kernel_map, I386_PGBYTES);
		bp->b_un.b_addr = (caddr_t) kvaddr;
		if (bp->b_flags & B_READ) {
			(*strat)(bp);
			biowait(bp);
			copyout(kvaddr, usraddr, bp->b_bcount - bp->b_resid);
		} else {  /* B_WRITE */
			copyin(usraddr, kvaddr, bp->b_bcount);
			(*strat)(bp);
			biowait(bp);
		}
		bp->b_un.b_addr = (caddr_t) usraddr;
		kmem_free(kernel_map, kvaddr, I386_PGBYTES);
		return;
	}


	(*strat)(bp);

	biowait(bp);

}

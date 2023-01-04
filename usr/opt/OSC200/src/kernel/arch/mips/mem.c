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
static char	*sccsid = "@(#)$RCSfile: mem.c,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/04/15 08:05:49 $";
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
 * derived from mem.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/*
 * Memory special file
 */

#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/vmmac.h>
#include <sys/uio.h>

#include <machine/cpu.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <vm/vm_map.h>
#include <vm/pmap.h>

mmread(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	uio->uio_rw = UIO_READ;
	return (mmrw(dev, uio));
}

mmwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{

	uio->uio_rw = UIO_WRITE;
	return (mmrw(dev, uio));
}

mmmap(dev_t dev, vm_offset_t offset, vm_prot_t prot)
{
	int pf;

	if (minor(dev) != 0) return -1;
	pf = btop(offset);
	if (pf >= physmem) return -1;
	else return pf;
}

mmrw(dev, uio)
	dev_t dev;
	struct uio *uio;
{
	register int o;
	register u_int c, v;
	register struct iovec *iov;
	int error = 0;

	while (uio->uio_resid > 0 && error == 0) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("mmrw");
			continue;
		}
		switch (minor(dev)) {

		/*
		 * minor device 0 is physical memory
		 */
		case 0:
			c = (u_int)iov->iov_len;

			v = (u_int)uio->uio_offset + c;
			/*
			 * check for wrap around
			 */
			if (v < (u_int)uio->uio_offset)
				goto fault;
			/*
			 * make sure endpoint is legal
			 */
			if (btop(v) >= physmem || PHYS_TO_K0(v) >= K1BASE)
				goto fault;
			error = uiomove(PHYS_TO_K1(uio->uio_offset),c,uio);
			continue;

		/*
		 * minor device 1 is kernel memory
		 */
		case 1:
			c = iov->iov_len;
			if (!kernacc((caddr_t)uio->uio_offset, c, uio->uio_rw))
				goto fault;
			error = uiomove(uio->uio_offset, c, uio);
			continue;

		/*
		 * minor device 2 is EOF/RATHOLE
		 */
		case 2:
			if (uio->uio_rw == UIO_READ)
				return (0);
			c = iov->iov_len;
			break;

		/*
		 * unknown minor device
		 */
		default:
			error = ENXIO;
			break;
		}
		if (error)
			break;
		iov->iov_base += c;
		iov->iov_len -= c;
		uio->uio_offset += c;
		uio->uio_resid -= c;
	}
	return (error);
fault:
	return (EFAULT);
}

/*
 * kernacc -- Check for kernel access
 */
kernacc(base, len, rw)
	u_int base, len;
	enum uio_rw rw;
{
	u_int end;
	extern vm_map_t kernel_map;

	end = base + len - 1;

	/*
	 * check for wrap around
	 */
	if (end < base)
		return(0);

	if (IS_KSEG0(base)) {
		if (IS_KSEG0(end) && btop(K0_TO_PHYS(end)) < physmem)
			return(1);
	} else if (IS_KSEG1(base)) {
		if (IS_KSEG1(end) && btop(K1_TO_PHYS(end)) < physmem)
			return(1);
	} else if (IS_KSEG2(base)) {
		if (!IS_KSEG2(end))
			return(0);
		return(vm_map_check_protection(kernel_map, 
					       trunc_page(base), 
					       round_page(base+len), 
					       rw == UIO_READ ? 
					       VM_PROT_READ : VM_PROT_WRITE));
	}
	return(0);
}

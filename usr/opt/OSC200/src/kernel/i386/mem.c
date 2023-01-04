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
static char	*sccsid = "@(#)$RCSfile: mem.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:08 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#include <mach_load.h>
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

/*
 * Memory special file
 */

#include <sys/param.h>
#include <ufs/dir.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/systm.h>
#include <sys/vm.h>
#include <sys/uio.h>

#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>

extern int loadpt;

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

mmrw(dev,uio)
	dev_t dev;
	struct uio *uio;
{
	register int o;
	register u_int c, v;
	register struct iovec *iov;
	int error = 0;
	vm_offset_t	where;
	int		spl;


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

/* minor device 0 is physical memory */
		case 0:
#if	MACH_LOAD
			v = trunc_page(uio->uio_offset + loadpt);
#else	MACH_LOAD
			v = trunc_page(uio->uio_offset);
#endif	MACH_LOAD
			if (uio->uio_offset >= mem_size)
				goto fault;

			spl = splvm();
			where = vm_map_min(kernel_map);
			if (vm_map_find(kernel_map, VM_OBJECT_NULL,
				(vm_offset_t) 0, &where, PAGE_SIZE, TRUE) != KERN_SUCCESS) {
				splx(spl);
				goto fault;
			}
			pmap_enter(vm_map_pmap(kernel_map), where, v,
					VM_PROT_READ|VM_PROT_WRITE, FALSE);
			pmap_update();
#if	MACH_LOAD
			o = (uio->uio_offset + loadpt) - v;
#else	MACH_LOAD
			o = uio->uio_offset - v;
#endif	MACH_LOAD
			c = min(PAGE_SIZE - o, (u_int)iov->iov_len);
                        error = uiomove((caddr_t) (where + o), c, uio);
			vm_map_remove(kernel_map, where, where + PAGE_SIZE);
			splx(spl);
			continue;

/* minor device 1 is kernel memory */
		case 1:
			c = iov->iov_len;
			if (!kernacc((caddr_t)uio->uio_offset, c, 
				uio->uio_rw == UIO_READ ? B_READ : B_WRITE))
				goto fault;
			error = uiomove((caddr_t)uio->uio_offset, (int)c, uio);
			continue;

/* minor device 2 is EOF/RATHOLE */
		case 2:
			if(uio->uio_rw == UIO_READ)
				return (0);
			c = iov->iov_len;
			break;
		}
/* minor device 3 is unibus memory (addressed by shorts) */
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

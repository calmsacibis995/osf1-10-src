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
static char *rcsid = "@(#)$RCSfile: devz.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/08 20:22:04 $";
#endif
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
 * Name:
 *	devz.c
 *
 * Description:
 *	This is the SVR4 DDI/DKI compliant driver for /dev/zero.  It
 *	is partially derived from the OSF/1 driver mem.c, which
 *	controls /dev/mem, /dev/kmem and /dev/null. It also contains
 *	snipets of code that were "borrowed" from the mmap system call.
 * 
 *	devz.c differs from mem.c in that it has a DDI/DKI 
 *	compliant interface and supports a segmap entry point. 
 *	The segmap entry point is invoked from the mmap() system 
 *	call when a user attempts to map /dev/zero into	his/her 
 *	address space.  
 *
 *	THIS DRIVER CAN ONLY BE INSTALLED IN AN OSF/1 KERNEL THAT HAS
 *	THE DDI/DKI KERNEL HOOKS OF THE SVR4 COMPATIBILITY PACKAGE
 *	BUILT IN. IT MUST BE INSTALLED AS AN SVR4 STYLE DRIVER.  
 *	FAILURE TO FOLLOW THESE INSTRUCTIONS WILL CAUSE UNPREDICTABLE 
 *	RESULTS.
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/conf.h>
#include <sys/systm.h>
#include <sys/vm.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <sys/ucred.h>
#include <sys/mman.h>
#include <vm/vm_map.h>
#include <machine/cpu.h>
#include <mach/vm_param.h>
#include <sys/addrconf.h>

/*
 * OPEN
 */
int
dzopen(dev_t *devp, int flag, int otyp, struct ucred *ucredp)
{
#ifdef DEBUG
	printf("entered dzopen\n");
#endif
	return(ESUCCESS);
}

/*
 * CLOSE
 */
int
dzclose(dev_t dev, int flag, int otyp, struct ucred *ucredp)
{
#ifdef DEBUG
	printf("entered dzclose\n");
#endif
	return(ESUCCESS);
}

/*
 * 
 *
 * Name:
 *	dzsegmap
 *
 * Function:
 *	This entry point allocates a private, anonymous
 *	(MAP_PRIVATE|MAP_ANONYMOUS) area of zero-filled
 *	memory in the user's address space.  
 *
 * Called by:
 *	smmap() - the mmap system call 
 *
 * Calls:
 *	vm_map()
 *
 * 
 */
int
dzsegmap(dev, offset, map, addrp, len, prot, maxprot, flags, ucredp)

dev_t           dev;		/*dev #. don't need it really*/
off_t           offset;		/*offset where we start mapping*/
vm_map_t        map;		/*ptr to user's address space map*/
addr_t          *addrp;		/*ptr to ptr - where we store mapped
				  address*/
off_t           len;		/*len of area that user wants to map*/
unsigned int    prot;		/*protection flags from mmap*/
unsigned int    maxprot;	/*max protection flag possible*/ 
unsigned int    flags;		/*mmap flags*/
struct ucred    *ucredp;	/*credentials. don't really need 'em*/

{
	boolean_t       anywhere;
	int		error;
	unsigned int    cflags;		/*copy of mmap flags*/
	vm_offset_t	user_addr;

#ifdef DEBUG
	printf("entered dzsegmap\n");
	printf("dzsegmap: off = %d\n",offset);
	printf("dzsegmap: map = %x\n",map);
	printf("dzsegmap: len = %d\n",len);
	printf("dzsegmap: prot = %d\n",prot);
	printf("dzsegmap: maxprot = %d\n",maxprot);
	printf("dzsegmap: original flags = %x\n",flags);
#endif

	/*
	 * In SVR4, you get MAP_PRIVATE whether you specify
	 * MAP_PRIVATE or MAP_SHARED.  Make sure we do the
	 * same.  Also make sure that we map it as an anonymous
	 * area.
	 */
	cflags = ( (flags & (~MAP_SHARED)) | (MAP_PRIVATE|MAP_ANON) );
	
        anywhere = (flags & MAP_FIXED) ? FALSE : TRUE;
	
	/*
	 *If user didn't specify MAP_FIXED, he/she wants OS to pick.
	 *In OSF/1, anonmyous areas are mapped into the BSS section.
	 */
	if(anywhere) 
		*addrp = (caddr_t)(addressconf[AC_MMAP_BSS].ac_base);
#ifdef DEBUG
	printf("dzsegmap: cflags = %x\n",cflags);
	printf("dzsegmap: anywhere = %x\n",anywhere);
#endif

        /* Map the object */
        error = vm_map(map, addrp, len, 0, anywhere,
                       MEMORY_OBJECT_NULL, offset,
		       cflags, prot, maxprot, VM_INHERIT_COPY);

	/*
	 * The SVR4 way is to return an error code from errno.h
	 * from the driver entry point. So just do it.
	 */
	switch (error)
	{	
		case KERN_SUCCESS:
			break;

		case KERN_NO_SPACE:
		case KERN_INVALID_ADDRESS:
                	error = ENOMEM;
	                break;

		case KERN_MEMORY_FAILURE:
		case KERN_MEMORY_ERROR:
	                error = EIO;
        	        break;

		default:
	                error = EINVAL;
        	        break;
        }

#ifdef DEBUG
	printf("dzsegmap: returning to user. addr = %x. error = %d\n",*addrp,error);
#endif
	return(error);
	
}

/*
 * MMAP
 *
 * If /dev/zero is mmaped, what the user really gets is a private
 * anonymous data area that's part of the user address space.  Since
 * there isn't any real memory associated with this "device", dzsegmap() 
 * doesn't need an mmap function to validate offsets.  Therefore, in
 * the interest of preventing intrepid users from hanging themselves,
 * it was decided that this driver will not support an mmap entry point.
 */
int
dzmmap(dev_t dev, off_t off, int prot)
{
#ifdef DEBUG
	printf("entered dzmmap\n");
#endif
	return(ENOTSUP);

}
	
/*
 * READ
 */
int
dzread(dev_t dev, struct uio *uiop, struct ucred *ucredp)
{
#ifdef DEBUG
	printf("entered dzread\n");
#endif
	uiop->uio_rw = UIO_READ;
	return (dzrw(dev, uiop));
}

/*
 * WRITE
 */
int
dzwrite(dev_t dev, struct uio *uiop, struct ucred *ucredp)
{
#ifdef DEBUG
	printf("entered dzwrite\n");
#endif
	uiop->uio_rw = UIO_WRITE;
	return (dzrw(dev, uiop));
}

/*
 * Common code for read/write
 */
int
dzrw(dev_t dev, struct uio *uio)
{
	caddr_t		base;
	int 		len;
	register	struct iovec *iov;
	register	struct uio *uiop;
	int 		error = 0;

	uiop = uio;

#ifdef DEBUG
	printf("entered dzrw\n");
	printf("dzwr: uio_iovcnt = %d\n",uiop->uio_iovcnt);
	printf("dzwr: uio_offset = %d\n",uiop->uio_offset);
	printf("dzwr: uio_resid = %d\n",uiop->uio_resid);
	printf("dzwr: uio_rw = %d\n",uiop->uio_rw);
#endif
	while (uiop->uio_resid > 0 && error == 0)
	{
#ifdef DEBUG
		printf("dzrw: top of while. resid = %d\n",uiop->uio_resid);
#endif
		iov = uiop->uio_iov;
#ifdef DEBUG
		printf("dzrw: iov_base = %x\n",iov->iov_base);
		printf("dzrw: iov_len = %d\n",iov->iov_len);
#endif
		if (iov->iov_len == 0)
		{
			uiop->uio_iov++;
			uiop->uio_iovcnt--;
			if (uiop->uio_iovcnt < 0)
				panic("dzrw");
			continue;
		}

		switch (uiop->uio_rw)
		{

			/*
			 *  A read from /dev/zero is supposed to return, well
			 *  a bunch of zeros.
			 */
			case UIO_READ:
				base = iov->iov_base;
				len = iov->iov_len;
        			current_pcb->pcb_nofault = NF_COPYIO;
				error = bzero(base, len);
        			current_pcb->pcb_nofault = 0;
				break;

			/*
			 * Write to /dev/zero is equivalent to dumping
			 * bytes to /dev/null.  So, don't really need to
			 * to do anything except fall through to adjust 
			 * value of various uio fields.
			 */
			case UIO_WRITE:	
				len = iov->iov_len;
				break;

			/*
			 * unknow operation
			 */
			default:
				error = ENXIO;
				break;

		} /*end switch*/

		if (error)	/*should never have an error - yuk,yuk,yuk*/
			break;
		iov->iov_base += len;
		iov->iov_len -= len;
		uiop->uio_offset += len;
		uiop->uio_resid -= len;
#ifdef DEBUG
		printf("dzwr: bottom of while. resid = %d\n",uiop->uio_resid);
		printf("dzwr: base = %x\n",iov->iov_base);
		printf("dzwr: len = %d\n",iov->iov_len);
		printf("dzwr: offset = %d\n",uiop->uio_offset);
#endif


	}  /*end while*/

	return (error);
}

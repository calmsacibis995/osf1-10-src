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
 * @(#)$RCSfile: vm_mmap.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 08:52:58 $
 */
#ifndef	__VM_MMAP__
#define	__VM_MMAP__ 1
#include <sys/types.h>
#include <sys/mman.h>

struct vp_mmap_args {
	dev_t 		a_dev;
	int 		(*a_mapfunc)();
	vm_offset_t	a_offset;
	vm_offset_t	*a_vaddr;
	vm_size_t	a_size;
	vm_prot_t	a_prot;
	vm_prot_t	a_maxprot;
	int		a_flags;
};

#endif	/* !__VM_MMAP__ */

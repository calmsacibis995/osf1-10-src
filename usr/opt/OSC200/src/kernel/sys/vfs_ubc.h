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
 * @(#)$RCSfile: vfs_ubc.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 18:52:25 $
 */
#ifndef	__VFS_UBC__
#define __VFS_UBC__ 1

#ifdef	KERNEL
#include <machine/vm_ubc.h>

#ifndef	ubc_load
extern int ubc_load(vm_page_t pp, vm_offset_t offset, vm_size_t size);
#endif

#ifndef	ubc_unload
extern int ubc_unload(vm_page_t pp, vm_offset_t offset, vm_size_t size);
#endif

#ifndef	ubc_kmem_alloc
extern int ubc_kmem_alloc(vm_page_t pp);
#endif

#ifndef ubc_kmem_free
extern int ubc_kmem_free(vm_page_t pp);
#endif

#ifndef ubc_kmem_cache
extern boolean_t int ubc_kmem_cache(vm_page_t pp);
#endif

#ifndef ubc_page_zero
extern void ubc_page_zero(vm_page_t pp, vm_offset_t offset, vm_size_t len);
#endif

extern struct vm_page *ubc_dirty_kluster();
extern struct vm_page *ubc_kluster();

/*
 * ubc_kluster operation flags for hold condition
 */

#define	UBC_HNONE	0x0			/* Don't hold pages */
#define	UBC_HBCP	0x1			/* Hold before center page */
#define	UBC_HACP	0x2			/* Hold after center page */
#define	UBC_HALL	(UBC_HBCP|UBC_HACP)	/* Hold all pages */

#endif	/* KERNEL */

#endif /* !__VFS_UBC */

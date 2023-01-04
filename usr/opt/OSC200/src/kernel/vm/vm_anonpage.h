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
 * @(#)$RCSfile: vm_anonpage.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/12/11 15:39:01 $
 */
#ifndef	__VM_ANONPAGE__
#define	__VM_ANONPAGE__ 1


/*
 * Anonymous memory without an owner is known by the
 * pageout code to be in no object's page list.
 */

struct vm_anonpage {
	struct vm_anon_object *ap_owner;	/* Page owner */
	long	ap_roffset;			/* Relative into anon object */
};

#define	pg_owner	_upg._apg.ap_owner
#define pg_roffset	_upg._apg.ap_roffset

#ifdef	KERNEL
extern struct vm_page *vm_anon_kpage_alloc(/* struct vm_swap_object *sop, 
		vm_offset_t soffset */);
extern struct vm_page *vm_anon_page_alloc(/* struct vm_anon_object *aop, 
		vm_offset_t offset,  alock_t lp, 
		struct vm_anon *ap, boolean_t aplocked, boolean_t wait */);
extern struct vm_page *vm_anon_zeroed_page_alloc(/* struct vm_anon_object *aop, 
		vm_offset_t offset,  alock_t lp, 
		struct vm_anon *ap, boolean_t aplocked, boolean_t wait */);
extern vm_anon_page_free(/* register struct vm_anon_object *aop, 
		register vm_offset_t offset, struct vm_anon *nap */);
#endif	/* KERNEL */

#endif	/* !__VM_ANONPAGE__ */

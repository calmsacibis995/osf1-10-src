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
 * @(#)$RCSfile: vm_pagelru.h,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/08/20 19:24:57 $
 */
#ifndef	__VM_PAGELRU__
#define	__VM_PAGELRU__ 1

#ifdef	KERNEL
decl_simple_lock_data(extern,vm_page_queue_lock)/* lock on active and inactive
						   page queues */
decl_simple_lock_data(extern,vm_page_wire_count_lock)
#define vm_page_lock_queues()	simple_lock(&vm_page_queue_lock)
#define vm_page_unlock_queues()	simple_unlock(&vm_page_queue_lock)

/*
 * Page queue states
 */

#define	PG_NOQUEUE	0x0
#define	PG_ACTIVE	0x1
#define	PG_INACTIVE	0x2
#define	PG_STATE	0x3
#define	pg_state(PP)	((PP)->pg_reserved & PG_STATE)
#define	pg_clearstate(PP)						\
	((PP)->pg_reserved &= ~PG_STATE)
#define	pg_setstate(PP,STATE)						\
	(PP)->pg_reserved = ((PP)->pg_reserved & ~PG_STATE) | STATE

/*
 * Other reserved bits
 */

#define	PG_PREWRITE	0x4			/* Page being pre-written  */
#define	PG_PREWRITTEN	0x8			/* Page pre-written once */


#define VM_PAGE_QUEUES_REMOVE(PG)				\
	MACRO_BEGIN						\
	if (pg_state(PG) == PG_ACTIVE) {			\
		pgl_remove(vm_page_queue_active,(PG),p); 	\
		vm_page_active_count--;				\
	}							\
								\
	if (pg_state(PG) == PG_INACTIVE) {			\
		pgl_remove(vm_page_queue_inactive,(PG),p); 	\
		vm_page_inactive_count--;			\
	}							\
	pg_clearstate(PG);					\
	MACRO_END


extern vm_page_t vm_page_queue_active;	/* active memory queue */
extern vm_page_t vm_page_queue_inactive;/* inactive memory queue */
extern int vm_page_active_count;	/* How many pages are active? */
extern int vm_page_inactive_count;	/* How many pages are inactive? */
extern int vm_page_free_target;		/* How many do we want free? */
extern int vm_page_free_min;		/* When to wakeup pageout */
extern int vm_page_inactive_target;	/* How many do we want inactive? */
extern int vm_page_free_reserved;	/* How many pages reserved for pageout*/


#endif	/* KERNEL */

#endif	/* ! __VM_PAGELRU__ */

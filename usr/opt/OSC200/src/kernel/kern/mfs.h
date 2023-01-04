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
 *	@(#)$RCSfile: mfs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:26:13 $
 */ 
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
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	mfs.h
 *	Author:	Avadis Tevanian, Jr.
 *	Copyright (C) 1987, Avadis Tevanian, Jr.
 *
 *	Header file for mapped file system support.
 *
 */ 

#ifndef	_KERN_MFS_H_
#define _KERN_MFS_H_

#include <mach/memory_object.h>
#include <kern/lock.h>
#include <kern/queue.h>
#include <kern/zalloc.h>
#include <vm/vm_object.h>
#include <sys/types.h>

/*
 *	Associated with each mapped file is information about its
 *	corresponding VM window.  This information is kept in the following
 *	vm_info structure.
 */
struct vm_info {
	memory_object_t	pager;		/* memory object (global name) */
	memory_object_control_t
			pager_request;	/* control port (global name) */
	int		locker;		/* thread holding lock (debug) */
	short		map_count;	/* number of times mapped */
	short		use_count;	/* number of times in use */
	vm_offset_t	va;		/* mapped virtual address */
	vm_size_t	size;		/* mapped size */
	vm_offset_t	offset;		/* offset into file at va */
	vm_size_t	vnode_size;	/* vnode size (not reflected in vp) */
	lock_data_t	lock;		/* lock for changing window */
	vm_object_t	object;		/* object [for KERNEL flushing] */
	queue_chain_t	lru_links;	/* lru queue links */
	struct ucred	*cred;		/* vnode credentials */
	int		error;		/* holds error codes */
	int		queued:1,	/* on lru queue? */
			dirty:1,	/* range needs flushing? */
			close_flush:1,	/* flush on close */
			mapped:1;	/* mapped into KERNEL VM? */
};

extern zone_t	vm_info_zone;

#define VM_INFO_NULL	((struct vm_info *) 0)

#define	MFS_ABLE(vp)	(((vp)->v_mode&VFMT) == VREG)

#endif	/* _KERN_MFS_H_ */

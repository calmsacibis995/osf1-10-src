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
 *	@(#)$RCSfile: specdev.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/12 19:03:22 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _SYS_SPECDEV_H_
#define	_SYS_SPECDEV_H_

#ifdef	_KERNEL
/*
 * This file contains data structures that deal with special files and
 * aliases for special files.  Whenever a special file is activated
 * (via pathname translation), a struct specinfo is allocated to hold
 * device file-specific information.  An alias (struct specalias) is a 
 * structure that associates all vnodes for a given open device.  They 
 * are required because it is possible for more than one special file to 
 * represent the same device. 
 */

/*
 * This structure is allocated when a vnode for a special file is created
 * during pathname translation. 
 * Locking:
 * 	All fields in this structure are protected by the spec hashchain
 *	lock for the device.
 * NOTE:
 *	See IMPORTANT NOTE below about alignment of si_rdev.
 */

struct specinfo {
	dev_t		 si_rdev;
	struct vnode	 *si_nextalias;		/* next vnode on alias list */
	struct vnode	 *si_shadowvp;		/* shadow vnode for VBLK */
	struct specalias *si_alias;		/* our alias structure */
};

/*
 * This structure is allocated when a device is opened (after pathname
 * translation.  It serves as an alias for multiple vnodes referencing the
 * same device.  If it is a block device, it has an associated vnode that
 * is used for operation involving the buffer cache (read, write, close).
 *
 * IMPORTANT NOTE:
 *	The sa_rdev field of this structure MUST be first to make this 
 *	look like a struct specinfo.  Buffer cache functions need to
 *	access the v_rdev field of the vnode, and the shadow vnodes use
 *	one of these rather than a struct specinfo.
 */

struct specalias {
	dev_t		sa_rdev;
	u_long		sa_flag;
	enum vtype 	sa_type;	/* type of alias (VCHR, VBLK) */
	long		sa_usecount;	/* count of vnodes aliased */
	struct vnode	*sa_vlist;	/* next vnode on alias list */
	struct vnode 	*sa_vnode;	/* our vnode, if there is one */
	struct specalias *sa_next;      /* next struct on hash chain */
	void		*sa_private;	/* private pointer for spec ops */
};

/*
 * Data structures for aliasing special vnodes
 */

struct spechash {
	struct specalias *sh_alias;		/* the hash chain */
	udecl_simple_lock_data(,sh_lock)	/* a spin lock */
};

#define v_rdev v_specinfo->si_rdev

/*
 * flags for struct specinfo
 */
#define SA_MOUNTED	0x0001	/* file system mounted here */
#define SA_CLOSING	0x0002	/* close in progress */
#define SA_GOING	0x0004	/* clearalias in progress */
#define SA_WAIT		0x0008	/* someone waiting on one of above */

/*
 * Flags passed to setmount.
 */
#define SM_MOUNTED	0x0001		/* check to see if device is mounted */
#define SM_OPEN		0x0002		/* check to see if device is open */
#define SM_SETMOUNT	0x0004		/* mark the device as mounted */
#define SM_CLEARMOUNT	0x0008		/* clear the mounted flag on device */


#define MINSPECHSZ	16
extern int spechsz;
extern struct spechash *speclisth;
extern int	setmount();		/* manipulate mount-related aliases */

#define SPEC_DONTOPEN 00040000   /* dont open cloned device */

#endif	/* _KERNEL */
#endif	/* _SYS_SPECDEV_H_ */

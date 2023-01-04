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
 * @(#)$RCSfile: vm_lock.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 19:01:02 $
 */
#ifndef	__VM_LOCK__
#define	__VM_LOCK__ 1

/*
 * lockvas functionality
 */

#define	VML_LOCK_ALL		0x01		/* lock all address space */
#define	VML_LOCK_RANGE		0x02		/* lock a range */
#define	VML_UNLOCK_ALL		0x04		/* unlock all address space */
#define	VML_UNLOCK_RANGE	0x08		/* unlock a range */
#define	VML_FUTURE		0x10		/* lock all future VAS exp */
#define	VML_NOFUTURE		0x20		/* disable future VAS lock */

#define	VML_LOCKMASK	(						\
			VML_LOCK_ALL			|		\
			VML_LOCK_RANGE			|		\
			VML_UNLOCK_ALL			|		\
			VML_UNLOCK_RANGE		|		\
			VML_FUTURE			|		\
			VML_NOFUTURE)

#endif /* !__VM_LOCK__ */

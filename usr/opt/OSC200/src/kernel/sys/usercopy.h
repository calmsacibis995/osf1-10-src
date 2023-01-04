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
 * @(#)$RCSfile: usercopy.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 19:04:46 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * File:	sys/usercopy.h
 *
 * Purpose:	This file provides the interface definitions for the routines
 *		that copy data between user space and kernel space for the 
 *		currently executing task. These routines are implemented in
 *		machine-dependent code, but invoked from both machine-dependent
 *		and machine-independent routines.
 */

#ifndef _SYS_USERCOPY_H_
#define _SYS_USERCOPY_H_

#ifndef _KERNEL
#error This is a kernel-internal file, and defines kernel-internal functions.
#endif

#include <sys/types.h>

/*
 * Routines:	copyin/copyout
 * Purpose:	Copy data between user and kernel space
 *
 * Arguments:
 *	src	location to fetch data from
 *	dest	location to store data 
 *	count	number of bytes to transfer
 *
 * Return value:
 *	0	success
 *	EFAULT	inaccessible or invalid user-space address
 *
 * Notes:
 *	These routines have no alignment restrictions in either user space
 *	or kernel space.
 */
extern int copyin(caddr_t user_src, caddr_t kernel_dest, int count);
extern int copyout(caddr_t kernel_src, caddr_t user_dest, int count);

/*
 * Routines:	copyinstr/copyoutstr
 * Purpose:	Copy NULL-terminated strings in and out of the kernel.
 * 
 * Arguments:
 *	src	starting address of source string.
 *	dest	starting address of destination buffer.
 *	maxlength	maximum number of bytes to be transferred, including
 *		NULL termination byte.
 *	lengthcopied 	actual number of bytes transferred, including NULL
 *		termination byte.
 *
 * Return value:
 *	0	success
 *	EFAULT	inaccessible or invalid user-space address
 *	ENAMETOOLONG string exceeded the maxlength value. Note that in this case,
 *		*lengthcopied bytes have been transferred, but the resulting
 *		string is not NULL-terminated.
 */
extern int copyinstr(
		caddr_t user_src,
		caddr_t kernel_dest,
		int maxlength,
		int *lengthcopied);
extern int copyoutstr(
		caddr_t kernel_src,
		caddr_t user_dest,
		int maxlength,
		int *lengthcopied);
/*
 * Routines:	fubyte/fuword/subyte/suword:
 * Purpose:	Fetch or store a data item to/from user data space.
 * 
 * Routines:	fuibyte/fuiword/suibyte/suiword:
 * Purpose:	Fetch or store a data item to/from user data space.
 *
 * On systems that do not distinguish between instruction and data space,
 * the two sets of routines are completely interchangeable.
 *
 * The return value of the fetch functions is the data item from the user
 * address space. A -1 is returned on error. In the case of fuword() and
 * fuiword(), this restricts the range of values that may be retrieved by
 * this routine, since it is not possible to distinguish between the error
 * return and an equivalent retrieved value. Since fuword is used to fetch
 * pointers from user space, this effectively invalidates (caddr_t)-1 as a
 * pointer. [Note that the system interfaces to mmap() and sbrk() cause the
 * same problem.]
 *
 * The return value of the store functions is a 0 for success, or
 * -1 on failure.
 */
extern int fubyte(caddr_t user_src);
extern int fuibyte(caddr_t user_src);
extern int fuword(caddr_t user_src);
extern int fuiword(caddr_t user_src);

extern int subyte(caddr_t user_dest, int data);
extern int suibyte(caddr_t user_dest, int data);
extern int suword(caddr_t user_dest, int data);
extern int suiword(caddr_t user_dest, int data);

#endif

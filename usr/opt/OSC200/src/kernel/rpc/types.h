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
 * @(#)$RCSfile: types.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/18 21:07:14 $
 */
/*	@(#)types.h	1.6 90/07/19 4.1NFSSRC SMI	*/

/* 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *      @(#)types.h 1.20 88/02/08 SMI      
 */


/*
 * Rpc additions to <sys/types.h>
 */

#ifndef	__rpc_types_h
#define	__rpc_types_h

#define register

#define	bool_t	int
#define	enum_t	int
#define __dontcare__	-1

#ifndef FALSE
#	define	FALSE	(0)
#endif

#ifndef TRUE
#	define	TRUE	(1)
#endif

#ifndef NULL
#	define NULL 0
#endif

#ifdef sun
typedef u_longlong_t uint64;

typedef longlong_t int64;

typedef u_long uint32;

typedef long int32;

#endif /* sun */

#ifdef mips

typedef union {
	u_long l[2];
	double d;
} u_longlong_t;

typedef union {
	long l[2];
	double d;
} longlong_t;

typedef u_longlong_t uint64;

typedef longlong_t int64;

typedef u_long uint32;

typedef long int32;

#endif /* mips */

#ifdef __alpha
typedef unsigned long u_longlong_t;
typedef long longlong_t;
#endif /* __alpha */

#ifndef	KERNEL
extern char *malloc();
#define	mem_alloc(bsize)	malloc(bsize)
#define	mem_free(ptr, bsize)	free(ptr)
#else
extern char *kalloc();
#define	kmem_alloc(bsize)	kalloc((u_int)bsize)
#define	kmem_free(ptr, bsize)	kfree((caddr_t)(ptr), (u_int)(bsize))
#define	mem_alloc(bsize)	kalloc((u_int)bsize)
#define	mem_free(ptr, bsize)	kfree((caddr_t)(ptr), (u_int)(bsize))
#endif

#include <sys/types.h>

#ifndef _TIME_
#include <sys/time.h>
#endif

#endif	/* !__rpc_types_h */

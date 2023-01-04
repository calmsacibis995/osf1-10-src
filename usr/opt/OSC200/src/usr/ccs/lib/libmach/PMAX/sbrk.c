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
static char	*sccsid = "@(#)$RCSfile: sbrk.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:10:31 $";
#endif 
/*
 */
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
 *	File:	sbrk.c
 *
 *	Unix compatibility for sbrk system call.
 */

#define  EXPORT_BOOLEAN
#include <mach.h>		/* for vm_allocate, vm_offset_t */
#include <stdio.h>		/* for stderr */
#include <sys/types.h>		/* for caddr_t */
#include <mach_init.h>		/* for vm_page_size */

/* Will not find get "assembler" forms of cubrk, minbrk. */

extern char end;
#define curbrk _curbrk
#define minbrk _minbrk
caddr_t curbrk = &end;
caddr_t minbrk = &end;

#ifdef lint
   /* lint doesn't see asm stuff */
caddr_t	curbrk;
caddr_t	minbrk;
#else lint
extern caddr_t curbrk;
extern caddr_t minbrk;
#endif lint

#define	roundup(a,b)	((((a) + (b) - 1) / (b)) * (b))

static int sbrk_needs_init = FALSE;

caddr_t sbrk(size)
	int	size;
{
	vm_offset_t	addr;
	kern_return_t	ret;
	caddr_t		ocurbrk;

	if (sbrk_needs_init) {
		sbrk_needs_init = FALSE;
		/*
		 *	Get "curbrk"
		 */

	}
	
	if (size <= 0)
		return(curbrk);
	addr = (vm_offset_t) roundup((int)curbrk,vm_page_size);
	ocurbrk = curbrk;
	if (((int)curbrk+size) > addr)
	{	ret = vm_allocate(task_self(), &addr, 
			    size -((int)addr-(int)curbrk), FALSE);
		if (ret == KERN_NO_SPACE) {
			ret = vm_allocate(task_self(), &addr, size, TRUE);
			ocurbrk = (caddr_t)addr;
		}
		if (ret != KERN_SUCCESS) 
			return((caddr_t) -1);
	}

	curbrk = (caddr_t)ocurbrk + size;
	return(ocurbrk);

}

void brk(x)
	caddr_t x;
{
	fprintf(stderr, "brk: not implemented\n");
}

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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: blok.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 15:07:00 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *	1.13  com/cmd/sh/sh/blok.c, cmdsh, bos320, 9125320 6/6/91 23:10:09
 */

#include	"defs.h"


/*
 *	storage allocator
 *	(circular first fit strategy)
 */

#define BUSY 01

#define busy(x)	(Rcheat((x)->word) & BUSY)

unsigned	brkincr = BRKINCR;


	extern void	*end;
        uchar_t *brkbegin = (uchar_t *) &end;
        uchar_t *bloktop  = (uchar_t *) &end;
/* in the shared library version, the library malloc is used to allocate.
 * However, some shell routines free pointers which where not malloc'd and
 * expect this to be ignored.  We test by checking for addresses within
 * the heap.  To reduce the cost of the test, we only use sbrk to check
 * for the true end of heap when the check fails.  Eventually, blktop
 * should converge to the high water mark of the heap.
 */
void
alloc_free(uchar_t *ap)
{
	if ( ap >= (uchar_t *)brkbegin && ap <= (uchar_t *)bloktop )
		free(ap);
	else {
		bloktop = (uchar_t *)sbrk(0);
		if ( ap >=(uchar_t *) brkbegin && ap <=(uchar_t *) bloktop )
			free(ap);
	}
	
}

/* use only to init stak */
void
addblok(unsigned reqd)
{
	if (stakbot == 0)
	{
		growstak();
	}
}

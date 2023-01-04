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
static char	*sccsid = "@(#)$RCSfile: lfind.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:24:37 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: lfind
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * lfind.c	1.8  com/lib/c/gen,3.1,8943 9/8/89 09:03:20
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak lfind = __lfind
#endif
#endif
#include <stdio.h>			/* for NULL */


/*
 * NAME:	lfind
 *                                                                    
 * FUNCTION:	search a linear list for a key
 *                                                                    
 * NOTES:	Linear search algorithm, generalized from Knuth (6.1)
 *		Algorithm Q.  This version no longer has anything to
 *		do with Knuth's Algorithm Q, which first copies the new
 *		element into the table, then looks for it.  The assumption
 *		there was that the cost of checking for the end of the
 *		table before each comparison outweighed the cost of the
 *		comparison, which isn't true when an arbitrary comparison
 *		function must be called and when the copy itself takes a
 *		significant number of cycles.  Actually, it has now
 *		reverted to Algorithm S, which is "simpler."
 *
 *		'Compar' is a bsearch-type of function.
 *
 * RETURN VALUE DESCRIPTION:	a pointer to the table where 'key' was
 *		found.  NULL if it wasn't found.
 */  

/* 
 * Arguments to this function are now declared as specified by the XOPEN standard.
 */

typedef char *POINTER;			/* pointer type for lfind() */

POINTER
lfind(key, base, nelp, width, compar)
void * key;			/* Key to be located */
void * base;			/* Beginning of table */
size_t  *nelp;		/* Pointer to current table size */
size_t  width;		/* Width of an element (bytes) */
int (*compar)();		/* Comparison function */
{
	POINTER next = (char *)base + *nelp * width;	/* End of table */

	for ( ; (POINTER)base < next; base = (char *)base + width)
		if ((*compar)(key, base) == 0)
			return (base);	/* Key found */

	return (NULL);
}

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
static char	*sccsid = "@(#)$RCSfile: bsearch.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:26:30 $";
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
 * FUNCTIONS: bsearch
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
 * bsearch.c	1.11  com/lib/c/gen,3.1,8943 9/8/89 08:38:17
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h> /* for size_t and NULL */

/*
 * FUNCTION:	Binary search algorithm, generalized from Knuth
 *		(6.2.1) Algorithm B.
 *
 * NOTES:	Bsearch searches 'base', an array of 'nmemb' objects
 *		of size 'size', for a member that matches 'key'.  The
 *		function pointed to by 'compar' is used for comparing
 *		'key' to an element of 'base'.  'Compar' is called with
 *		2 arguments, the first of which is the 'key' and second
 *		of which is an array member.  It must return:
 *			< 0: if the key compares less than the member
 *			= 0: if the key compares equal than the member
 *			> 0: if the key compares greater than the member
 *
 * RETURN VALUE DESCRIPTION:	A pointer is returned to the element
 *		in 'base' matching 'key'.  If 'key' cannot be found
 *		in 'base', NULL is returned.
 *
 */  

void *
bsearch(const void *key, const void *base, size_t nmemb, size_t size,                   int(*compar)(const void *, const void *))
{
	size_t two_size = size + size;
	void *last = (char *)base + size * (nmemb - 1); /* Last element in table */

	while (last >= base) {
		void *p = (char *)base + size * (((char *)last - (char *)base)/two_size);
		int res = (*compar)(key, p);

		if (res == 0)
			return (p);	/* Key found */
		if (res < 0)
			last = (char *)p - size;
		else
			base = (char *)p + size;
	}
	return (NULL);		/* Key not found */
}

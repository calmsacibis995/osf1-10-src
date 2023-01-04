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
static char	*sccsid = "@(#)$RCSfile: memccpy.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/19 18:37:42 $";
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
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: memccpy
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
 * memccpy.c	1.7  com/lib/c/gen,3.1,8943 9/8/89 09:04:18
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_SHARED_LIBRARIES) || (defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak memccpy = __memccpy
#endif
#endif
#include <stdio.h>		/* for NULL	*/
#include <sys/types.h>		/* for size_t	*/

/*
 * NAME:	memccpy
 *                                                                    
 * FUNCTION:	Copy source to target, stopping if character c is copied.
 *		Copy no more than n bytes.
 *
 * RETURN VALUE DESCRIPTION:	Return a pointer to the byte after
 *		character c in the copy, or NULL if c is not found
 *		in the first n bytes.
 */  


void *
memccpy(void *target, const void *source, register int c, register size_t n)

{
	register unsigned char *t = target, *s = source;
	register int ch ;

	c = (unsigned char) c;
	 while (n > 0) {
		ch = *s ;
		*t = ch ;
		s++, t++;
		if (ch == c)
			return (t);
		n--;
	}
	return (NULL);
}

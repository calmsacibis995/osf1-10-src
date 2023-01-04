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
static char	*sccsid = "@(#)$RCSfile: strtok.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:09:36 $";
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
 * FUNCTIONS: strtok
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
 * strtok.c	1.9  com/lib/c/str,3.1,8943 9/13/89 16:32:08
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak strtok_r = __strtok_r
#endif
#endif
#include <string.h>

/*
 * FUNCTION: Returns a pointer to an occurrence of a text token in the
 *	     string pointed to by s1.  The string pointed to by s2
 *	     defines a set of token delimiters.  If s1 is anything other
 *	     than NULL, the string pointed to by s1 is read until one of
 *	     the delimiter characters is found.  A null character is
 *	     stored into the string, replacing the found delimiter, and
 *	     a pointer to the first character of the text token is
 *	     returned.  Subsequent calls with a NULL value in s1 step
 *	     through the string.  The delimiters pointed to by s2 can
 *	     be changed on subsequent calls.  A NULL pointer is returned
 *	     when no tokens remain in the string pointed to by s1.
 *                                                                    
 * PARAMETERS:
 *	     char *s1 - scanned for text tokens
 *	     char *s2 - set of token delimiters
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to a text token, or NULL,
 *	     as described above.
 */
/*LINTLIBRARY*/

/*
*	Thread-safe strtok
*	Changed to be re-entrant by not using
*	static data and passing in the data instead.
*/
#ifdef _THREAD_SAFE
#define SAVEPT	*savept
#else
#define SAVEPT	savept
#endif

#ifdef _THREAD_SAFE
char *
strtok_r(char *s1, const char *s2, char **savept )
#else /* _THREAD_SAFE */
char *
strtok(char *s1, const char *s2)
#endif /* _THREAD_SAFE */
{
	char	*p, *q, *r;
#ifndef _THREAD_SAFE
	static char	*savept;
#endif _THREAD_SAFE

	/*first or subsequent call*/
	p = (s1 == NULL)? SAVEPT: s1;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, s2);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = strpbrk(q, s2)) == NULL)	/* move past token */
		SAVEPT = 0;
	else {
		*r = '\0';
		SAVEPT = ++r;
	}
	return(q);
}

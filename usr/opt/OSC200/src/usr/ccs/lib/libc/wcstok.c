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
static char *rcsid = "@(#)$RCSfile: wcstok.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:09:03 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcstok
 *
 *
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
 * com/lib/c/str/wcstok.c, bos320 2/26/91 17:50:16
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak wcstok_r = __wcstok_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak wcstok = __wcstok
#endif
#endif
#include <wchar.h>

/*
 * FUNCTION: Returns a pointer to an occurrence of a text token in the wchar
 *	     string pointed to by ws1.  The wchar string pointed to by ws2
 *	     defines a set of token delimiters.  If ws1 is anything other
 *	     than NULL, the string pointed to by ws1 is read until one of
 *	     the delimiters is found.  A null wchar is
 *	     stored into the string, replacing the found delimiter, and
 *	     a pointer to the first wchar of the text token is
 *	     returned.  Subsequent calls with a NULL value in ws1 step
 *	     through the string.  The delimiters pointed to by ws2 can
 *	     be changed on subsequent calls.  A NULL pointer is returned
 *	     when no tokens remain in the string pointed to by ws1.
 *                                                                    
 * PARAMETERS:
 *	     wchar_t *ws1 - scanned for text tokens
 *	     wchar_t *ws2 - set of token delimiters
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to a text token, or NULL,
 *	     as described above.
 */
/*LINTLIBRARY*/

/*
 *	Thread-safe wcstok
 *	Changed to be re-entrant by not using
 *	static data and passing in the data instead.
 */

#ifdef _THREAD_SAFE
#   define SAVEPT	*savept
#else
#   define SAVEPT	savept
#endif


wchar_t *
#ifdef _THREAD_SAFE
wcstok_r(wchar_t *ws1, const wchar_t *ws2, wchar_t **savept)
#else
wcstok(wchar_t *ws1, const wchar_t *ws2)
#endif
{
	wchar_t	*p, *q, *r;
#ifndef _THREAD_SAFE
	static wchar_t	*savept;
#endif

	/*first or subsequent call*/
	p = (ws1 == NULL)? SAVEPT: ws1;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + wcsspn(p, ws2);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = (wchar_t *)wcspbrk(q, (wchar_t *)ws2)) == NULL)	/* move past token */
		SAVEPT = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		SAVEPT = ++r;
	}
	return(q);
}

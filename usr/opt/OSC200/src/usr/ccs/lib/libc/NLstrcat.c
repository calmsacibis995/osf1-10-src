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
static char	*sccsid = "@(#)$RCSfile: NLstrcat.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:17 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrcat, NCstrcat 
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
 * sccsid[] = "NLstrcat.c        1.11  com/lib/c/str,3.1,9021 2/11/90 17:30:25";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrcat = __NCstrcat
#pragma weak NLstrcat = __NLstrcat
#endif
#endif
#include <sys/types.h>
#include <NLchar.h>

/*
 *  Concatenate s2 on the end of s1.  S1's space must be large enough.
 *  Return s1.  Two versions here: NLstrcat (works on ASCII + NLS code
 *  points and is identical to strcat), and NCstrcat (works on NLchars).
 */

/*
 * NAME: NLstrcat()
 *
 * FUNCTION: concatonate two strings
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the beginning of the concatonated string.
 */
unsigned char *
#ifdef _NO_PROTO
NLstrcat(s1, s2)
register unsigned char *s1, *s2;
#else /* _NO_PROTO */
NLstrcat(register unsigned char *s1, register unsigned char *s2)
#endif /* _NO_PROTO */
{
	register unsigned char *os1;

	os1 = s1;
	while(*s1++)
		;
	--s1;
	while(*s1++ = *s2++)
		;
	return(os1);
}

/*
 * NAME: NCstrcat
 *
 * FUNCTION: concatonate two strings of type NCchar
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first character of the concatonated.
 */
wchar_t *
#ifdef _NO_PROTO
NCstrcat(s1, s2)
register wchar_t *s1, *s2;
#else /* _NO_PROTO */
NCstrcat(register wchar_t *s1, register wchar_t *s2)
#endif /* _NO_PROTO */
{
	register wchar_t *os1;

	os1 = s1;
	while(*s1++)
		;
	--s1;
	while(*s1++ = *s2++)
		;
	return(os1);
}

#ifdef KJI
#undef wstrcat (s1, s2)

wchar_t *
#ifdef _NO_PROTO
wstrcat(s1, s2)
register wchar_t *s1, *s2;
#else /* _NO_PROTO */
wstrcat(register wchar_t *s1, register wchar_t *s2)
#endif /* _NO_PROTO */
{
	return (NCstrcat(s1, s2));
}
#endif

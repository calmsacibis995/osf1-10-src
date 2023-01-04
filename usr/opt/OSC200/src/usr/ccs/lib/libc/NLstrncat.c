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
static char	*sccsid = "@(#)$RCSfile: NLstrncat.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:39 $";
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
 * FUNCTIONS: NLstrncat, NCstrncat
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
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
 * sccsid[] = "NLstrncat.c       1.10  com/lib/c/str,3.1,9021 1/18/90 09:52:10";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrncat = __NCstrncat
#pragma weak NLstrncat = __NLstrncat
#endif
#endif
#include <sys/types.h>
#include <NLchar.h>

/*
 *
 *  Concatenate s2 on the end of s1.  S1's space must be large enough.
 *  At most n elements (bytes or NLchars) are moved.  Return s1.  Two
 *  versions here:  NLstrncat (works with ASCII containing NLS code
 *  points, and is identical to strncat), and NCstrncat (works with
 *  NLchars).
 *
#ifdef KJI
 *  Change Activity:
 *	08/11/87 changed character processsing for SJIS.
#endif
 */

/*
 * NAME: NLstrncat
 *
 * FUNCTION: concatonate two strings, with a maximum length of n in the
 *	output string.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the result string.
 */
unsigned char *
#ifdef _NO_PROTO
NLstrncat(s1, s2, n)
register unsigned char *s1, *s2;
register n;
#else /* _NO_PROTO */
NLstrncat(register unsigned char *s1, register unsigned char *s2, register n)
#endif /* _NO_PROTO */
{
	register unsigned char *os1;

	os1 = s1;
	while (*s1++)
		;
	--s1;
	for (; *s1 = *s2++; ++s1) {
	    if (--n < 0) {
		*s1 = '\0';
		break;
	    } else if (NCisshift(*s1)) {
		if (--n >= 0) 
		    *(++s1) = *s2++;
		else {
		    *s1 = '\0';
		    break;
		}
	    }	
	}
	return(os1);
}

/*
 * NAME: NCstrncat
 *
 * FUNCTION: concatonate two strings of NCchars.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first NCchar in the
 *	resulting string.
 */
NLchar *
#ifdef _NO_PROTO
NCstrncat(s1, s2, n)
register NLchar *s1, *s2;
register n;
#else /* _NO_PROTO */
NCstrncat(register NLchar *s1, register NLchar *s2, register n)
#endif /* _NO_PROTO */
{
	register NLchar *os1;

	os1 = s1;
	while(*s1++)
		;
	--s1;
	while(*s1++ = *s2++)
		if(--n < 0) {
			*--s1 = 0;
			break;
		}
	return(os1);
}

#ifdef KJI
#include <wchar.h>
#undef wstrncat

wchar_t *
#ifdef _NO_PROTO
wstrncat(s1, s2, n)
register wchar_t *s1, *s2;
register n;
#else /* _NO_PROTO */
wstrncat(register wchar_t *s1, register wchar_t *s2, register n)
#endif /* _NO_PROTO */
{
	return (NCstrncat(s1, s2, n));
}
#endif

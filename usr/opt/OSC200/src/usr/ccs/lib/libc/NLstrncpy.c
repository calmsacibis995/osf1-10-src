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
static char	*sccsid = "@(#)$RCSfile: NLstrncpy.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:45 $";
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
 * FUNCTIONS: NLstrncpy, NCstrncpy
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
 * sccsid[] = "NLstrncpy.c       1.9  com/lib/c/str,3.1,9021 1/18/90 09:52:30";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrncpy = __NCstrncpy
#pragma weak NLstrncpy = __NLstrncpy
#endif
#endif
#include <sys/types.h>
#include <NLchar.h>

/*
 *
 *  Copy s2 to s1, truncating or null-padding to always copy n elements
 *  (bytes or NLchars).  Return s1.  Two versions here:  NLstrncpy (works
 *  with ASCII containing embedded NLS code points) and NCstrncpy (works
 *  with NLchars).
 *
#ifdef KJI
 *  Change Activity:
 *	08/11/87 changed character processsing for SJIS.
#endif
 */

unsigned char *
#ifdef _NO_PROTO
NLstrncpy(s1, s2, n)
register unsigned char *s1, *s2;
register int n;
#else /* _NO_PROTO */
NLstrncpy(register unsigned char *s1, register unsigned char *s2,register int n)
#endif /* _NO_PROTO */
{
	register unsigned char *os1 = s1;

	for (; --n >= 0; ++s1) {
	    if ((*s1 = *s2++) == '\0') {
		    ++n;
		    break;
		}
	    if (NCisshift(*s1))
		if (--n >= 0) {
		    *(++s1) = *s2++;
		} else {
		    n+=2;
		    break;
		}
	}
	while (--n >= 0)
		*s1++ = '\0';
	return (os1);
}

NLchar *
#ifdef _NO_PROTO
NCstrncpy(s1, s2, n)
register NLchar *s1, *s2;
register int n;
#else /* _NO_PROTO */
NCstrncpy(register NLchar *s1, register NLchar *s2, register int n)
#endif /* _NO_PROTO */
{
	register NLchar *os1 = s1;

	while (--n >= 0)
		if ((*s1++ = *s2++) == 0)
			while (--n >= 0)
				*s1++ = 0;
	return (os1);
}

#ifdef KJI
#include <wchar.h>
#undef wstrncpy

wchar_t *
#ifdef _NO_PROTO
wstrncpy(s1, s2, n)
register wchar_t *s1, *s2;
register int n;
#else /* _NO_PROTO */
wstrncpy(register wchar_t *s1, register wchar_t *s2, register int n)
#endif /* _NO_PROTO */
{
	return (NCstrncpy(s1, s2, n));
}
#endif

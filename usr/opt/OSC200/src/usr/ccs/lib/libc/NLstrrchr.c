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
static char	*sccsid = "@(#)$RCSfile: NLstrrchr.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:49 $";
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
 * FUNCTIONS: NLstrrchr, NCstrrchr
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
 * sccsid[] = "NLstrrchr.c       1.9  com/lib/c/str,3.1,9021 1/18/90 09:53:29";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrrchr = __NCstrrchr
#pragma weak NLstrrchr = __NLstrrchr
#endif
#endif
#include <sys/types.h>
#include <NLchar.h>

/*
 *  Return the ptr in sp at which the character c last appears; NULL
 *  if not found.  Two versions here:  NLstrrchr (works with ASCII
 *  containing NLS code points, takes NLchar as second argument), and
 *  NCstrrchr (works with NLchars).
 */

#define NULL	0

unsigned char *
#ifdef _NO_PROTO
NLstrrchr(sp, c)
register unsigned char *sp;
register NLchar c;
#else /* _NO_PROTO */
NLstrrchr(register unsigned char *sp, register NLchar c)
#endif /* _NO_PROTO */
{
	register unsigned char *r;
	register int i;

	r = NULL;
	do {
		if (1 < (i = NLchrlen(sp))) {
			if (NCdechr(sp) == c)
				r = sp;
			sp += i - 1;
		} else if(*sp == c)
			r = sp;
	} while(*sp++);
	return(r);
}

NLchar *
#ifdef _NO_PROTO
NCstrrchr(sp, c)
register NLchar *sp, c;
#else /* _NO_PROTO */
NCstrrchr(register NLchar *sp, register NLchar c)
#endif /* _NO_PROTO */
{
	register NLchar *r;

	r = NULL;
	do {
		if(*sp == c)
			r = sp;
	} while(*sp++);
	return(r);
}

#ifdef KJI
#include <wchar.h>

wchar_t *
#ifdef _NO_PROTO
wstrrchr(sp, c)
register wchar_t *sp;
register int c;
#else /* _NO_PROTO */
wstrrchr(register wchar_t *sp, register int c)
#endif /* _NO_PROTO */
{
	register wchar_t *r;

	r = NULL;
	do {
		if(*sp == c)
			r = sp;
	} while(*sp++);
	return(r);
}
#endif

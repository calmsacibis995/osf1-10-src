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
static char	*sccsid = "@(#)$RCSfile: NLstrpbrk.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:47 $";
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
 * FUNCTIONS: NLstrpbrk, NCstrpbrk
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
 * sccsid[] = "NLstrpbrk.c       1.9  com/lib/c/str,3.1,9021 1/18/90 09:53:06";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrpbrk = __NCstrpbrk
#pragma weak NLstrpbrk = __NLstrpbrk
#endif
#endif
#include <sys/types.h>
#include <NLchar.h>

/*
 *  Return ptr to first occurrence of any character from `brkset'
 *  in the character string `string'; NULL if none exists.  Two versions
 *  here:  NLstrpbrk (works with ASCII containing embedded NLS code
 *  points) and NCstrpbrk (works with NLchars).
 */

#define	NULL	0

unsigned char *
#ifdef _NO_PROTO
NLstrpbrk(string, brkset)
register unsigned char *string;
char *brkset;
#else /* _NO_PROTO */
NLstrpbrk(register unsigned char *string, char *brkset)
#endif /* _NO_PROTO */
{
	register NLchar *p;
	NLchar buf[NLCSETMAX], sc;
	NLchar *nlbrkset = _NCbufstr(brkset, buf, NLCSETMAX);
	register int i;

	for (; ; ) {
		if (!*string)
			break;
		i = NCdec(string, &sc);
		for(p = nlbrkset; *p != 0 && *p != sc; ++p)
			;
		if(*p != 0) {
			_NCfreebuf(nlbrkset);
			return(string);
		}
		string += i;
	}
	_NCfreebuf(nlbrkset);
	return(NULL);
}

NLchar *
#ifdef _NO_PROTO
NCstrpbrk(string, brkset)
register NLchar *string;
char *brkset;
#else /* _NO_PROTO */
NCstrpbrk(register NLchar *string, char *brkset)
#endif /* _NO_PROTO */
{
	register NLchar *p;
	NLchar buf[NLCSETMAX];
	NLchar *nlbrkset = _NCbufstr(brkset, buf, NLCSETMAX);

	do {
		for(p = nlbrkset; *p != 0 && *p != *string; ++p)
			;
		if(*p != 0) {
			_NCfreebuf(nlbrkset);
			return(string);
		}
	}
	while(*string++);
	_NCfreebuf(nlbrkset);
	return(NULL);
}

#ifdef KJI
#include <wchar.h>

wchar_t *
#ifdef _NO_PROTO
wstrpbrk(string, brkset)
register wchar_t *string, *brkset;
#else /* _NO_PROTO */
wstrpbrk(register wchar_t *string, register wchar_t *brkset)
#endif /* _NO_PROTO */
{
	register wchar_t *p;
	wchar_t buf[NLCSETMAX];
	wchar_t *nlbrkset = _NCbufstr(brkset, buf, NLCSETMAX);

	do {
		for(p = brkset; *p != 0 && *p != *string; ++p)
			;
		if(*p != 0) 
			return(string);
	}
	while(*string++);
	return(NULL);
}
#endif

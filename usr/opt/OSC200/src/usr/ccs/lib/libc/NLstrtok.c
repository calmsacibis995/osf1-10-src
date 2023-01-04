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
static char	*sccsid = "@(#)$RCSfile: NLstrtok.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:54 $";
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
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrtok, NCstrtok
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
 * sccsid[] = "NLstrtok.c	1.13  com/lib/c/str,3.1,9021 4/26/90 15:45:41";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrtok = __NCstrtok
#pragma weak NLstrtok = __NLstrtok
#endif
#endif
#include <string.h>
#include <NLchar.h>

/*
 *  Uses strpbrk and strspn to break string into tokens on sequentially
 *  subsequent calls.  Returns NULL when no non-separator characters
 *  remain.  `Subsequent' calls are calls with first argument NULL.
 *  Two versions here:  NLstrtok (works with ASCII containing embedded
 *  NLS code points) and NCstrtok (works with NLchars).
 */

#define	NULL	0

unsigned char *
#ifdef _NO_PROTO
NLstrtok(string, sepset)
unsigned char	*string, *sepset;
#else /* _NO_PROTO */
NLstrtok(unsigned char *string, unsigned char *sepset)
#endif /* _NO_PROTO */
{
	register unsigned char	*p, *q, *r;
	static unsigned char	*savept;

	/*first or subsequent call*/

	p = (string == NULL) ? savept : string;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + NLstrspn(p, sepset);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = NLstrpbrk(q, (char *)sepset)) == NULL)	/* move past token */
		savept = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept = ++r;
	}
	return(q);
}

NLchar *
#ifdef  _NO_PROTO
NCstrtok(string, sepset)
NLchar *string;
unsigned char *sepset;
#else /* _NO_PROTO */
NCstrtok(NLchar *string, unsigned char *sepset)
#endif /* _NO_PROTO */
{
	register NLchar	*p, *q, *r;
	static NLchar *savept;

	/*first or subsequent call*/

	p = (string == NULL) ? savept : string;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + NCstrspn(p, sepset);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = NCstrpbrk(q, (char *)sepset)) == NULL)	/* move past token */
		savept = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept = ++r;
	}
	return(q);
}

#ifdef _KJI
#include <wstring.h>

wchar_t *
wstrtok(string, sepset)
wchar_t *string, *sepset;
{
	register wchar_t	*p, *q, *r;
	static wchar_t *savept;

	/*first or subsequent call*/

	p = (string == NULL) ? savept : string;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + wstrspn(p, sepset);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = wstrpbrk(q, sepset)) == NULL)	/* move past token */
		savept = 0;	/* indicate this is last token */
	else {
		*r = '\0';
		savept = ++r;
	}
	return(q);
}
#endif

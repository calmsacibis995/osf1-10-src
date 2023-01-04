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
static char	*sccsid = "@(#)$RCSfile: NLstrcspn.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:33 $";
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
 * FUNCTIONS: NLstrcspn, NCstrcspn
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
 * sccsid[] = "NLstrcspn.c       1.10  com/lib/c/str,3.1,9021 1/18/90 09:51:26";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrcspn = __NCstrcspn
#pragma weak NLstrcspn = __NLstrcspn
#endif
#endif
#include <sys/types.h>
#include <NLchar.h>

/*
 *  Return the number of array elements (bytes or NLchars) in the
 *  longest leading segment of string that consists solely of characters
 *  NOT from charset.  Two versions here:  NLstrcspn (operates on ASCII
 *  with embedded NLS code points) and NCstrcspn (operates on NLchars).
 *  NLstrcspn returns length of prefix matched in bytes, NCstrcspn in
 *  NLchars.
 */

/*
 * NAME: NLstrcspn
 *
 * FUNCTION: Return the number of array elements (bytes or NLchars) in the
 *  longest leading segment of string that consists solely of characters
 *  NOT from charset.  Two versions here:  NLstrcspn (operates on ASCII
 *  with embedded NLS code points) and NCstrcspn (operates on NLchars).
 *  NLstrcspn returns length of prefix matched in bytes, NCstrcspn in
 *  NLchars.
 *
 * RETURN VALUE DESCRIPTION: see FUNCTION
 */
int
#ifdef _NO_PROTO
NLstrcspn(string, charset)
unsigned char	*string;
register char	*charset;
#else /* _NO_PROTO */
NLstrcspn(unsigned char *string, register char *charset)
#endif /* _NO_PROTO */
{
	register unsigned char *q;
	register NLchar *p;
	NLchar buf[NLCSETMAX], qc;
	NLchar *nlcharset = _NCbufstr(charset, buf, NLCSETMAX);
	register int i;

	for(q=string; *q != '\0'; ) {
		i = NCdec(q, &qc);
		for (p = nlcharset; *p != 0 && *p != qc; ++p)
			;
		if(*p != 0)
			break;
		q += i;
	}
	_NCfreebuf(nlcharset);
	return(q - string);
}

/*
 * NAME: NCstrcspn
 *
 * FUNCTION: Return the number of array elements (bytes or NLchars) in the
 *  longest leading segment of string that consists solely of characters
 *  NOT from charset.  Two versions here:  NLstrcspn (operates on ASCII
 *  with embedded NLS code points) and NCstrcspn (operates on NLchars).
 *  NLstrcspn returns length of prefix matched in bytes, NCstrcspn in
 *  NLchars.
 *
 * RETURN VALUE DESCRIPTION: see FUNCTION
 */
int
#ifdef _NO_PROTO
NCstrcspn(string, charset)
NLchar *string;
register unsigned char *charset;
#else /* _NO_PROTO */
NCstrcspn(NLchar *string, register unsigned char *charset)
#endif /* _NO_PROTO */
{
	register NLchar *p, *q;
	NLchar buf[NLCSETMAX];
	NLchar *nlcharset = _NCbufstr(charset, buf, NLCSETMAX);

	for(q=string; *q != 0; ++q) {
		for(p = nlcharset; *p != 0 && *p != *q; ++p)
			;
		if(*p != 0)
			break;
	}
	return(q - string);
}

#ifdef KJI
#include <wchar.h>

int
#ifdef _NO_PROTO
wstrcspn(string, charset)
wchar_t *string, *charset;
#else /* _NO_PROTO */
wstrcspn(wchar_t *string, wchar_t *charset)
#endif /* _NO_PROTO */
{
	register wchar_t *p, *q;
	wchar_t buf[NLCSETMAX];

	for(q=string; *q != 0; ++q) {
		for(p = charset; *p != 0 && *p != *q; ++p)
			;
		if(*p != 0)
			break;
	}
	return(q - string);
}
#endif

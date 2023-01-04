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
static char	*sccsid = "@(#)$RCSfile: NLctime.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:17:35 $";
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
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: NLctime, NLasctime 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sccsid[] = "NLctime.c 1.10  com/lib/c/time,3.1,9021 1/30/90 18:25:16";
 */

/*LINTLIBRARY*/
/*
 *  NLS-parameterized versions of ctime and asctime:
 *	NLctime(clk) just does the call NLasctime(localtime(clk)).
 *	NLasctime(tm) just makes the appropriate call to NLstrtime
 *	to get the current NLS equivalent of the traditional Unix
 *	date/time string.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NLasctime = __NLasctime
#pragma weak NLctime = __NLctime
#endif
#endif
#include <time.h>

/*  Length of buffer to hold return value of NLasctime()
 */
#define BLEN	64

unsigned char *
#ifdef _NO_PROTO
NLctime(t)
long *t;
#else /* _NO_PROTO */
NLctime(long *t)
#endif /* _NO_PROTO */
{
	return(NLasctime(localtime(t)));
}

unsigned char *
NLasctime(struct tm *t)
{
	static unsigned char strbuf[BLEN];

	strftime((char *)strbuf, (size_t) BLEN, "%a %sD %X %Y\n\0", t);
	return(strbuf);
}

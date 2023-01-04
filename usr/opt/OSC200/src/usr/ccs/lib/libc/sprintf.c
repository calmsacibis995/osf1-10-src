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
static char	*sccsid = "@(#)$RCSfile: sprintf.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/18 15:39:57 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * sprintf.c
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * FUNCTIONS: sprintf, wsprintf 
 *
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sprintf.c	1.12  com/lib/c/prnt,3.1,8943 9/13/89 11:44:42
 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <values.h>
#include <stdlib.h>

extern int _doprnt();

/*VARARGS2*/

/*                                                                    
 * FUNCTION: Writes output to the array specified by s, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: s      - array to be printed to
 *             format - format used to print arguments
 *	       ...    -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      Returns number of characters printed
 */                                                                   

#pragma weak NLsprintf = sprintf

#ifdef _BSD
char *
#else	/* _BSD */
int
#endif	/* _BSD */
sprintf(char *s, const char *format, ...) 
{
	int count;
	FILE siop;
	va_list ap;

	siop._lock = NULL;
	siop._cnt = INT_MAX;
	siop._base = siop._ptr = (unsigned char *)s;
	siop._flag = (_IOWRT|_IONOFD);
	va_start(ap,format);
	count = _doprnt(format, ap, &siop);
	va_end(ap);
	*siop._ptr = '\0'; /* plant terminating null character */
#ifdef _BSD
	return (s);
#else	/* _BSD */
	return(count);
#endif /* _BSD */
}

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
static char	*sccsid = "@(#)$RCSfile: vsprintf.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/18 15:40:31 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * FUNCTIONS: vsprintf 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * vsprintf.c	1.10  com/lib/c/prnt,3.1,8943 9/7/89 10:17:32
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <values.h>

extern int _doprnt();

/*                                                                    
 * FUNCTION: Writes output to to the array pointed to by s, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent argumnts are converted for output.
 *
 * PARAMETERS: s      - array to be printed to
 *             format - format used to print arguments
 *	       arg   - arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */                                                                   

#pragma weak NLvsprintf = vsprintf

int	
vsprintf(char *s, const char *format, va_list arg)
{
	FILE siop;
	int rc;

#ifdef _THREAD_SAFE
	siop._lock = NULL;
#endif
	siop._cnt = INT_MAX;
	siop._base = siop._ptr = (unsigned char *)s;
	siop._flag = (_IOWRT|_IONOFD);
	rc = _doprnt(format, arg, &siop);
	*siop._ptr = '\0';
	return (rc);
}

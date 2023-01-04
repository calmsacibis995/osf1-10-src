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
static char	*sccsid = "@(#)$RCSfile: printf.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/18 15:39:53 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * file
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
 * FUNCTIONS: printf 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * printf.c	1.10  com/lib/c/prnt,3.1,8943 9/7/89 10:16:10
 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include "ts_supp.h"
#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#include <stdarg.h>

extern int _doprnt();
/*VARARGS1*/

/*                                                                    
 * FUNCTION: writes output to stdout under control of the string pointed
 *           to by format that specifies how subsequent arguments are
 *           converted for output.
 *
 * PARAMETERS: format - format used to print arguments
 *	       ...    -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */

#pragma weak NLprintf = printf

int	
#ifndef	_NO_PROTO
printf(const char *format, ...) 
#else
printf(format, va_alist)
const char *format;
va_dcl
#endif
{
	int count;
	va_list ap;
	register int rc;
	TS_FDECLARELOCK(filelock)

	va_start(ap,format);
	TS_FLOCK(filelock, stdout);

	count = _doprnt(format, ap, stdout);
	rc = ferror(stdout) ? EOF : count;

	TS_FUNLOCK(filelock);
	va_end(ap);

	return(rc);
}

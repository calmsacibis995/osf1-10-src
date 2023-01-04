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
static char	*sccsid = "@(#)$RCSfile: vprintf.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:50:52 $";
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
 * FUNCTIONS: vprintf 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * vprintf.c	1.11  com/lib/c/prnt,3.1,8943 9/7/89 10:17:10
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdarg.h>
#include <stdio.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

/*
 * FUNCTION: Writes output to stdout under control of the string pointed
 *	     to by format that specifies how subsequent argumnts are
 *	     converted for output.
 *
 * PARAMETERS: format - format used to print arguments
 *	       arg   - arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 *	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */
int	
vprintf(const char *format, va_list arg) 
{
	int rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stdout);

	rc = _doprnt(format, arg, stdout);

	rc = ferror(stdout) ? EOF : rc;
	TS_FUNLOCK(filelock);
	return (rc);
}

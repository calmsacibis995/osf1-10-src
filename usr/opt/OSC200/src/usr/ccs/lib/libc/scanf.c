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
static char	*sccsid = "@(#)$RCSfile: scanf.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 19:43:59 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * scanf.c
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
 * FUNCTIONS: scanf, fscanf, sscanf 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.14  com/lib/c/io/scanf.c, libcio, bos320, 9125320 6/11/91 11:03:13
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#include <stdarg.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern int _doscan();

/*
 * FUNCTION:	The scanf function reads input from stdin, under control
 *		of the string pointed to by format that specifies the 
 *		admissible input sequences and how they are to be converted
 *		for assignment.
 *
 * RETURN VALUE DESCRIPTION:	
 *		returns the value of EOF if an input failure occurs before
 *		any conversion.  Otherwise, the fscanf function returns
 *		the number of input items assigned.
 *
 */
int	
scanf(const char *format, ...) 
{
	va_list ap;
	register int rc;
	TS_FDECLARELOCK(filelock)

	va_start(ap, format);
	TS_FLOCK(filelock, stdin);
	rc = _doscan(stdin, format, ap);
	TS_FUNLOCK(filelock);

	return (rc);
}


int	
fscanf(FILE *stream, const char *format, ...)
{
	va_list ap;
	register int rc;
	TS_FDECLARELOCK(filelock);

	va_start(ap, format);
	TS_FLOCK(filelock, stream);
	rc = _doscan(stream, format, ap);
	TS_FUNLOCK(filelock);

	return (rc);
}


int	
sscanf(const char *s, const char *format, ...) 
{
	va_list ap;
	FILE strbuf;

	va_start(ap, format);
	strbuf._flag = (_IOREAD|_IONOFD);
	strbuf.__newbase = (char *)(strbuf._ptr =
			   strbuf._base = (unsigned char *)s);
	strbuf._cnt = strlen(s);

#ifdef _THREAD_SAFE
	strbuf._lock = NULL;
#endif	/* _THREAD_SAFE */

	return (_doscan(&strbuf, format, ap));
}

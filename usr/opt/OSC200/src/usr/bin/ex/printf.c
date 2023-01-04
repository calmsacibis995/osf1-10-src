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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: printf.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 21:37:12 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDEDIT) printf.c
 *
 * FUNCTION: printf
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.7  com/cmd/edit/vi/printf.c, cmdedit, bos32 06/5/91 23:33:03
 */

/*LINTLIBRARY*/
#define VPRINTF
#include "ex.h"
#include <stdio.h>
#include <stdarg.h>
#ifndef VPRINTF
#include <values.h>
#endif
/*
 * ex_printf performs the equivalent of an sprintf and then calls vi's
 * putchar() to output string[]
 */
/*VARARGS1*/

int _doprnt(char *, va_list, FILE *);

int
ex_printf(char *format, ...)
{
	static char string[2*2048];	/* for at least 2048 2-byte chars */
	register int count;
#ifndef VPRINTF
	FILE siop;
#endif
	va_list ap;
	int char_len;

#ifndef VPRINTF
	siop._cnt = MAXINT;
	siop._base = siop._ptr = (unsigned char *)string;
	siop._flag = (_IOWRT|_IONOFD);
#endif
	va_start(ap, format);
	
#ifndef VPRINTF
	count = ((int)_doprnt(format, ap, &siop));
#else
	count = vsprintf(string, format, ap);
#endif
	va_end(ap);
#ifndef VPRINTF
	*siop._ptr = '\0'; /* plant terminating null character */
#endif
	if (count > 0) {
		register char *s = string;
		wchar_t nlc;
		int count2 = count;
		while ((count2 > 0) && ((char_len = mbtowc(&nlc, s, MB_CUR_MAX))> 0)){
			s += char_len;
			ex_putchar(nlc);
			count2--;
		}
	}
	return(count);
}

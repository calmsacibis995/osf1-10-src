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
static char rcsid[] = "@(#)$RCSfile: bsd_mvprintw.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:05:27 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * bsd_mvprintw.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint

#endif /* not lint */

#ifdef __STDC__
# include <stdarg.h>
#else
# include <varargs.h>
#endif
# include	"curses.ext"

/*
 * implement the mvprintw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Sigh....
 *
 */

#ifdef __STDC__
mvprintw(reg int y, reg int x, char *fmt, ...)
#else
mvprintw(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
	reg int	y, x;
	char	*fmt;
#endif
	va_list	ap;
	char	buf[512];

#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
	y = va_arg(ap, int);
	x = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#endif
	if (move(y, x) != OK)
		return ERR;
	(void) vsprintf(buf, fmt, ap);
	va_end(ap);
	return waddstr(stdscr, buf);
}

#ifdef __STDC__
mvwprintw(reg WINDOW *win, reg int y, reg int x, char *fmt, ...)
#else
mvwprintw(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
	reg WINDOW *win;
	reg int	y, x;
	char	*fmt;
#endif
	va_list	ap;
	char	buf[512];

#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
	win = va_arg(ap, WINDOW *);
	y = va_arg(ap, int);
	x = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#endif
	if (move(y, x) != OK)
		return ERR;
	(void) vsprintf(buf, fmt, ap);
	va_end(ap);
	return waddstr(win, buf);
}

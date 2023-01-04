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
/* $XConsortium: recode.c,v 5.1 91/02/16 09:45:57 rws Exp $ */

/***********************************************************
Copyright (c) 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/



/*
 * recode.escapes --
 * changes troff's funny control codes back into \x sequences,
 * where the control codes and their translations are as defined
 * in the switch statement below.
 *	Written by Henry McGilton.  April 1984
 */
#include	<stdio.h>

main ()
{
	int	c;

	while ((c = getchar()) != EOF)
		switch (c) {
		case '\000':
			putchar('\\');
			putchar('0');
			break;
		case '\001':
			putchar('\\');
			putchar('a');
			break;
		case '\020':
			putchar('\\');
			putchar('{');
			break;
		case '\021':
			putchar('\\');
			putchar('}');
			break;
		case '\024':
			putchar('\\');
			putchar('%');
			break;
		case '\025':
			putchar('\\');
			putchar('c');
			break;
		case '\026':
			putchar('\\');
			putchar('e');
			break;
		case '\027':
			putchar('\\');
			putchar(' ');
			break;
		case '\030':
			putchar('\\');
			putchar('!');
			break;
		case '\022':
			putchar('\\');
			putchar('&');
			break;
		case ('\327' & 0377):
			putchar('\\');
			putchar('^');
			break;
		case ('\332' & 0377):
			putchar('\\');
			putchar('|');
			break;
		case ('\326' & 0377):
			putchar('\\');
			putchar('-');
			break;
		case ('\334' & 0377):
			putchar('\\');
			putchar('\'');
			break;
		case ('\003'):
			putchar('\\');
			putchar('`');
			break;
		default:
			putchar(c);
			break;
		}
	return(0);
}

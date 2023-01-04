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
static char	*sccsid = "@(#)$RCSfile: gets.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:16 $";
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
/*
 * 5799-CGZ (C) COPYRIGHT IBM CORPORATION 1986,1988,1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef lint
#endif

int _noputchar;

char *gets(buf)
	char *buf;
{
	register char *lp;
	register c;

	lp = buf;
	for (;;) {
		c = getchar() & 0177;
store:
		switch (c) {
		case '\n':
		case '\r':
			c = '\n';      /* treat both as newline */
			*lp++ = '\0';
			return(buf);
		case '\b':
			putchar(' ');
			putchar('\b');
		case '#':
			lp--;
			if (lp < buf)
				lp = buf;
			continue;
		case '@':
		case 'u' & 037:
			lp = buf;
			putchar('\n');
			continue;
		case '\04':		/* ^D - return nul byte */
			return((char *) 0);
		case '\03':		/* ^C - exit */
			exit(1);
			continue;	/* paranoia */
		case 'o' & 037:		/* ^O - [no]suppress output */
			{
				_noputchar = ! _noputchar;
				continue;
			}
		case 'w' & 037:		/* ^W - wait for a second */
			delay(1000);
			continue;
		default:
			*lp++ = c;
		}
	}
}

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
static char	*sccsid = "@(#)$RCSfile: ttinit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:12:09 $";
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
 * Copyright (c) 1983 Regents of the University of California.
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
 * 
 * 	ttinit.c	3.20 (Berkeley) 6/29/88
 */


#include "ww.h"
#include "tt.h"
#ifdef POSIX_TTY
#include <sys/ioctl.h>
#endif

int tt_h19();
int tt_h29();
int tt_f100();
int tt_tvi925();
int tt_wyse75();
int tt_wyse60();
int tt_zentec();
int tt_generic();
struct tt_tab tt_tab[] = {
	{ "h19",	3, tt_h19 },
	{ "h29",	3, tt_h29 },
	{ "f100",	4, tt_f100 },
	{ "tvi925",	6, tt_tvi925 },
	{ "wyse75",	6, tt_wyse75 },
	{ "wyse60",	6, tt_wyse60 },
	{ "w60",	3, tt_wyse60 },
	{ "zentec",	6, tt_zentec },
	{ "generic",	0, tt_generic },
	0
};

ttinit()
{
	register struct tt_tab *tp;
	register char *p, *q;
	register char *t;
	struct winsize winsize;

	tt_strp = tt_strings;

	/*
	 * Set output buffer size to about 1 second of output time.
	 */
	tt_obp = tt_ob;
	tt_obe = tt_ob + MIN(wwbaud/10, sizeof tt_ob);

	/*
	 * Use the standard name of the terminal (i.e. the second
	 * name in termcap).
	 */
	for (p = wwtermcap; *p && *p != '|' && *p != ':'; p++)
		;
	if (*p == '|')
		p++;
	for (q = p; *q && *q != '|' && *q != ':'; q++)
		;
	if (q != p && (t = malloc((unsigned) (q - p + 1))) != 0) {
		wwterm = t;
		while (p < q)
			*t++ = *p++;
		*t = 0;
	}
	for (tp = tt_tab; tp->tt_name != 0; tp++)
		if (strncmp(tp->tt_name, wwterm, tp->tt_len) == 0)
			break;
	if (tp->tt_name == 0) {
		wwerrno = WWE_BADTERM;
		return -1;
	}
	if ((*tp->tt_func)() < 0) {
		wwerrno = WWE_CANTDO;
		return -1;
	}
	tt.tt_scroll_top = 0;
	tt.tt_scroll_bot = tt.tt_nrow - 1;
	if (ioctl(0, TIOCGWINSZ, (char *)&winsize) >= 0) {
		if (winsize.ws_row != 0)
			tt.tt_nrow = winsize.ws_row;
		if (winsize.ws_col != 0)
			tt.tt_ncol = winsize.ws_col;
	}
	return 0;
}

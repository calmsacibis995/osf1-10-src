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
static char	*sccsid = "@(#)$RCSfile: wwdelline.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:13:22 $";
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
 * wwdelline.c	3.10 (Berkeley) 6/29/88
 */


#include "ww.h"

wwdelline(w, row)
register struct ww *w;
int row;
{
	register i;
	register union ww_char **cpp, **cqq;
	register union ww_char *cp;
	int row1, row2;
	char deleted;
	int visible;

	/*
	 * Scroll first.
	 */
	if ((row1 = row) < w->ww_i.t) {
		row1 = w->ww_i.t;
	}
	if ((row2 = w->ww_b.b) > w->ww_i.b) {
		row2 = w->ww_i.b;
		visible = 0;
	} else
		visible = 1;
	deleted = wwscroll1(w, row1, row2, 1, visible);

	/*
	 * Fix the buffer.
	 * But leave clearing the last line for wwclreol().
	 */
	cpp = &w->ww_buf[row];
	cqq = cpp + 1;
	cp = *cpp;
	for (i = w->ww_b.b - row; --i > 0;)
		*cpp++ = *cqq++;
	*cpp = cp;

	/*
	 * Now clear the last line.
	 */
	if (visible)
		wwclreol1(w, w->ww_b.b - 1, w->ww_b.l, deleted);
	else {
		cp += w->ww_b.l;
		for (i = w->ww_b.nc; --i >= 0;)
			cp++->c_w = ' ';
	}
}

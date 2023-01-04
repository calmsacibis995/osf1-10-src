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
static char	*sccsid = "@(#)$RCSfile: wwmove.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:14:08 $";
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
 * wmove.c	3.9 (Berkeley) 6/29/88
 */


#include "ww.h"

/*
 * Move a window.  Should be unattached.
 */
wwmove(w, row, col)
register struct ww *w;
{
	register dr, dc;
	register i;

	dr = row - w->ww_w.t;
	dc = col - w->ww_w.l;

	w->ww_w.t += dr;
	w->ww_w.b += dr;
	w->ww_w.l += dc;
	w->ww_w.r += dc;

	w->ww_b.t += dr;
	w->ww_b.b += dr;
	w->ww_b.l += dc;
	w->ww_b.r += dc;

	w->ww_i.t = MAX(w->ww_w.t, 0);
	w->ww_i.b = MIN(w->ww_w.b, wwnrow);
	w->ww_i.nr = w->ww_i.b - w->ww_i.t;
	w->ww_i.l = MAX(w->ww_w.l, 0);
	w->ww_i.r = MIN(w->ww_w.r, wwncol);
	w->ww_i.nc = w->ww_i.r - w->ww_i.l;

	w->ww_cur.r += dr;
	w->ww_cur.c += dc;

	w->ww_win -= dr;
	for (i = w->ww_w.t; i < w->ww_w.b; i++)
		w->ww_win[i] -= dc;
	if (w->ww_fmap != 0) {
		w->ww_fmap -= dr;
		for (i = w->ww_w.t; i < w->ww_w.b; i++)
			w->ww_fmap[i] -= dc;
	}
	w->ww_nvis -= dr;
	for (i = w->ww_i.t; i < w->ww_i.b; i++) {
		register j = w->ww_i.l;
		register char *win = &w->ww_win[i][j];
		register char *smap = &wwsmap[i][j];
		int nvis = 0;

		for (; j < w->ww_i.r; j++, win++, smap++)
			if (*win == 0 && *smap == w->ww_index)
				nvis++;
		w->ww_nvis[i] = nvis;
	}
	w->ww_buf -= dr;
	for (i = w->ww_b.t; i < w->ww_b.b; i++)
		w->ww_buf[i] -= dc;
}

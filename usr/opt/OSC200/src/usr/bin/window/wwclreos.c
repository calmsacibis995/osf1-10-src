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
static char	*sccsid = "@(#)$RCSfile: wwclreos.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:13:06 $";
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
 * wwclreos.c	3.8 (Berkeley) 6/29/88
 */


#include "ww.h"
#include "tt.h"

wwclreos(w, row, col)
register struct ww *w;
{
	register i;
	int cleared = 0;

	/*
	 * Quick and dirty check for windows that cover the bottom
	 * portion of the screen.  Not meant to be complete.
	 */
	if (tt.tt_clreos && w->ww_i.b == wwnrow && w->ww_i.l == 0 &&
	    w->ww_i.r == wwncol && wwvisible(w)) {
		register j;
		register union ww_char *s;

		i = row;
		(*tt.tt_move)(i, col);
		(*tt.tt_clreos)();
		/*
		 * We have to fix wwos becuase wwclreol1 won't do that.
		 */
		s = &wwos[i][col];
		for (j = wwncol - col; --j >= 0;)
			s++->c_w = ' ';
		for (i++; i < wwnrow; i++) {
			s = wwos[i];
			for (j = wwncol; --j >= 0;)
				s++->c_w = ' ';
		}
		cleared = 1;
	}
	wwclreol1(w, row, col, cleared);
	for (i = row + 1; i < w->ww_b.b; i++)
		wwclreol1(w, i, w->ww_b.l, cleared);
}

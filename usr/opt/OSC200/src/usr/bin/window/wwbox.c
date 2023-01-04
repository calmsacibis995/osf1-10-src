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
static char	*sccsid = "@(#)$RCSfile: wwbox.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:12:51 $";
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
 * wwbox.c	3.6 (Berkeley) 6/29/88
 */


#include "ww.h"
#include "tt.h"

wwbox(w, r, c, nr, nc)
register struct ww *w;
register r, c;
int nr, nc;
{
	register r1, c1;
	register i;

	r1 = r + nr - 1;
	c1 = c + nc - 1;
	wwframec(w, r, c, WWF_D|WWF_R);
	for (i = c + 1; i < c1; i++)
		wwframec(w, r, i, WWF_L|WWF_R);
	wwframec(w, r, i, WWF_L|WWF_D);
	for (i = r + 1; i < r1; i++)
		wwframec(w, i, c1, WWF_U|WWF_D);
	wwframec(w, i, c1, WWF_U|WWF_L);
	for (i = c1 - 1; i > c; i--)
		wwframec(w, r1, i, WWF_R|WWF_L);
	wwframec(w, r1, i, WWF_R|WWF_U);
	for (i = r1 - 1; i > r; i--)
		wwframec(w, i, c, WWF_D|WWF_U);
}

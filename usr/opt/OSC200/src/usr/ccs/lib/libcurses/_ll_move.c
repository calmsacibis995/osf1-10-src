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
static char rcsid[] = "@(#)$RCSfile: _ll_move.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:42:31 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_ll_move.c	1.6  com/lib/curses,3.1,8943 10/16/89 22:58:24";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _ll_move
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

extern	struct	line	*_line_alloc();

/*
 * NAME:        _ll_move
 *
 * FUNCTION:
 *
 *      Position the cursor at position (row,col)
 *      in the virtual screen
 */

_ll_move (row, col)
register int row, col; 
{
	register struct line *p;
	register int l;
	register chtype *b1, *b2;
	register int rp1 = row+1;

#ifdef DEBUG
	if(outf) fprintf(outf, "_ll_move(%d, %d)\n", row, col);
#endif
	if (SP->virt_y >= 0 && (p=SP->std_body[SP->virt_y+1]) &&
		p->length < SP->virt_x)
		p->length = SP->virt_x >= columns ? columns : SP->virt_x;
	SP->virt_x = col;
	SP->virt_y = row;
	if (row < 0 || col < 0)
		return;
	if (!SP->std_body[rp1] || SP->std_body[rp1] == SP->cur_body[rp1]) {
		p = _line_alloc ();
		if (SP->cur_body[rp1]) {
			p->length = l = SP->cur_body[rp1]->length;
			b1 = &(p->body[0]);
			b2 = &(SP->cur_body[rp1]->body[0]);
			for ( ; l>0; l--)
				*b1++ = *b2++;
#ifdef PHASE2
			b1 = &(p->atr_body[0]);
			b2 = &(SP->cur_body[rp1]->atr_body[0]);
			for ( l = p->length; l>0; l--)
				*b1++ = *b2++;
#endif
		}
		SP->std_body[rp1] = p;
	}
	p = SP->std_body[rp1];
	p -> hash = 0;
	while (p -> length < col)
		p -> body[p -> length++] = ' ';
	SP->curptr = &(SP->std_body[rp1] -> body[col]);
#ifdef PHASE2
	SP->curatr = &(SP->std_body[rp1] -> atr_body[col]);
#endif
}

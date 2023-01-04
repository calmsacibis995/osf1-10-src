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
static char rcsid[] = "@(#)$RCSfile: _pos.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:44:28 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_pos.c  1.6  com/lib/curses,3.1,8943 10/16/89 22:59:15";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _pos
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

extern	int	_outch();

/*
 * NAME:        _pos
 *
 * FUNCTION:
 *
 *      Position the SP->curptr to (row, column) which start at 0.
 */

_pos(row, column)
int	row;
int	column;
{
#ifdef DEBUG
    if(outf) fprintf(outf, "_pos from row %d, col %d => row %d, col %d\n",
	    SP->phys_y, SP->phys_x, row, column);
#endif
	if( SP->phys_x == column && SP->phys_y == row )
	{
		return;	/* already there */
	}
	/*
	 * Many terminals can't move the cursor when in standout mode.
	 * We must be careful, however, because HP's and cookie terminals
	 * will drop a cookie when we do this.
	 */
	if( !move_standout_mode && SP->phys_gr && magic_cookie_glitch < 0 )
	{
		if( !ceol_standout_glitch )
		{
			_clearhl ();
		}
	}
	/* some terminals can't move in insert mode */
	if( SP->phys_irm == 1 && !move_insert_mode )
	{
		tputs(exit_insert_mode, 1, _outch);
		SP->phys_irm = 0;
	}
	/* If we try to move outside the scrolling region, widen it */
	if( row<SP->phys_top_mgn || row>SP->phys_bot_mgn )
	{
		_window(0, lines-1, 0, columns-1);
		_setwind();
	}
	mvcur(SP->phys_y, SP->phys_x, row, column);
	SP->phys_x = column;
	SP->phys_y = row;
}

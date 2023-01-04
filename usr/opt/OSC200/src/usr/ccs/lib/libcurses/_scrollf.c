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
static char rcsid[] = "@(#)$RCSfile: _scrollf.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:47:51 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_scrollf.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:00:06";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _scrollf
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
 * NAME:        _scrollf
 *
 * FUNCTION:
 *
 *      Scroll the terminal forward n lines, bringing up blank lines from
 *      bottom.  This only affects the current scrolling region.
 */

_scrollf(n)
int n;
{
	register int i;

	if( scroll_forward )
	{
		_setwind();
		_pos( SP->des_bot_mgn, 0 );
		for( i=0; i<n; i++ )
		{
			tputs(scroll_forward, 1, _outch);
		}
		SP->ml_above += n;
		if( SP->ml_above + lines > lines_of_memory )
		{
			SP->ml_above = lines_of_memory - lines;
		}
	}
	else
	{
		/* If terminal can't do it, try delete line. */
		_pos(0, 0);
		_dellines(n);
	}
}

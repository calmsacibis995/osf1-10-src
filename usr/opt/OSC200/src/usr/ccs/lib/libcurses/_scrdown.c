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
static char rcsid[] = "@(#)$RCSfile: _scrdown.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:47:02 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_scrdown.c  1.6  com/lib/curses,3.1,8943 10/16/89 22:59:47";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _scrdown
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

/*
 * NAME:        _scrdown
 *
 * FUNCTION:
 *
 *      Scroll the screen down (e.g. in the normal direction of text) one
 *      line physically, and update the internal notion of what's on the
 *      screen (SP->cur_body) to know about this.  Do it in such a way that
 *      we will realize this has been done later and take advantage of it.
 */

_scrdown()
{
	struct line *old_d, *old_p;
	register int i, l=lines;

#ifdef DEBUG
	if(outf)
	{
		fprintf(outf, "_scrdown()\n");
		fprintf( outf, "\tDoing _pos( %d, 0 )\n", lines - 1 );
		fprintf( outf, "\tDoing _scrollf(1)\n" );
	}
#endif

	/* physically... */
	_pos(lines-1, 0);
	_scrollf(1);

	/* internally */
	old_d = SP->std_body[1];
	old_p = SP->cur_body[1];
#ifdef	DEBUG
	if(outf)
	{
		fprintf( outf, "lines = l = %d\n", l );
	}
#endif	/* DEBUG */

	for( i=1; i<=l; i++ )
	{
		SP->std_body[i] = SP->std_body[i+1];
		SP->cur_body[i] = SP->cur_body[i+1];
	}
	SP->std_body[1] = NULL;
	SP->cur_body[1] = NULL;
	_line_free(old_d);
	if( old_d != old_p )
	{
		_line_free(old_p);
	}
}

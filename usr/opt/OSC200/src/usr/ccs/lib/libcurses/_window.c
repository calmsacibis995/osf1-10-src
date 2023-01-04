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
static char rcsid[] = "@(#)$RCSfile: _window.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:55:00 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_window.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:02:45";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _window
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
 * NAME:        _window
 *
 * FUNCTION:
 *
 *      Set the desired window to the box with the indicated boundaries.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      All scrolling should only affect the area inside the window.
 *      We currently ignore the last 2 args since we're only using this
 *      for scrolling and want to use the feature on vt100's as well as
 *      on concept 100's.  left and right are for future expansion someday.
 *
 *      Note that we currently assume cursor addressing within the window
 *      is relative to the screen, not the window.  This will have to be
 *      generalized if concept windows are to be used.
 */

_window(top, bottom, left, right)
int top, bottom, left, right;
{
#ifdef DEBUG
	if(outf) fprintf(outf,
		"_window old top=%d, bot %d; new top=%d, bot %d\n",
			SP->des_top_mgn, SP->des_bot_mgn, top, bottom);
#endif
	if (change_scroll_region || set_window) {
		SP->des_top_mgn = top;
		SP->des_bot_mgn = bottom;
	}
#ifdef DEBUG
	else
		if(outf) fprintf(outf, "window setting ignored\n");
#endif
}

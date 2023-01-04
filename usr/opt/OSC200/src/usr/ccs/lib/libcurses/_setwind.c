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
static char rcsid[] = "@(#)$RCSfile: _setwind.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:50:21 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_setwind.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:00:58";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _setwind
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

char *tparm();

extern	int	_outch();

/*
 * NAME:        _setwind
 *
 * FUNCTION:
 *
 *      Force the window to be as desired
 */

_setwind()
{
	if (	SP->phys_top_mgn == SP->des_top_mgn &&
		SP->phys_bot_mgn == SP->des_bot_mgn) {
#ifdef DEBUG
		if(outf) fprintf(outf,
			"_setwind, same values %d & %d, do nothing\n",
				SP->phys_top_mgn, SP->phys_bot_mgn);
#endif
		return;
	}
	if (set_window)
		tputs(tparm(set_window, SP->des_top_mgn,
			SP->des_bot_mgn, 0, columns-1), 1, _outch);
	else if (change_scroll_region) {
		/* Save & Restore SP->curptr since it becomes undefined */
		tputs(save_cursor, 1, _outch);
		tputs(tparm(change_scroll_region,
			SP->des_top_mgn, SP->des_bot_mgn), 1, _outch);
					/* put SP->curptr back */
		tputs(restore_cursor, 1, _outch);
	}
#ifdef DEBUG
	if(outf) fprintf(outf, "set phys window from (%d,%d) to (%d,%d)\n",
	SP->phys_top_mgn, SP->phys_bot_mgn, SP->des_top_mgn, SP->des_bot_mgn);
#endif
	SP->phys_top_mgn = SP->des_top_mgn;
	SP->phys_bot_mgn = SP->des_bot_mgn;
}

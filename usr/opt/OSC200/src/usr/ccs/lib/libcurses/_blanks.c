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
static char rcsid[] = "@(#)$RCSfile: _blanks.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:27:21 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_blanks.c	1.6  com/lib/curses,3.1,8943 10/16/89 22:53:12";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _blanks
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
extern	int	_sethl();
extern	int	_setmode();
extern	char	*tparm();
extern	int	tputs();

/*
 * NAME:        _blanks
 *
 * FUNCTION:    Output n blanks, or the equivalent.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is done to erase text, and
 *      also to insert blanks.  The semantics of this call do not define
 *      where it leaves the cursor - it might be where it was before, or
 *      it might be at the end of the blanks.  We will, of course, leave
 *      SP->phys_x properly updated.
 */

_blanks (n)
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_blanks(%d).\n", n);
#endif
	if (n == 0)
		return;
	_setmode ();
	_sethl ();
	if (SP->virt_irm==1 && parm_ich) {
		if (n == 1)
			tputs(insert_character, 1, _outch);
		else
			tputs(tparm(parm_ich, n), n, _outch);
		return;
	}
	if (erase_chars && SP->phys_irm != 1 && n > 5) {
		tputs(tparm(erase_chars, n), n, _outch);
		return;
	}
	if (repeat_char && SP->phys_irm != 1 && n > 5) {
		tputs(tparm(repeat_char, ' ', n), n, _outch);
		SP->phys_x += n;
		return;
	}
	while (--n >= 0) {
		if (SP->phys_irm == 1 && insert_character)
			tputs (insert_character,
				columns - SP->phys_x, _outch);
		if (++SP->phys_x >= columns && auto_right_margin) {
			if (SP->phys_y >= lines-1) {
				/*
				 * We attempted to put something in the last
				 * position of the last line.  Since this will
				 * cause a scroll (we only get here if the
				 * terminal has auto_right_margin) we refuse
				 * to put it out.
				 */
				SP->phys_x--;
				return;
			}
			SP->phys_x = 0;
			SP->phys_y++;
		}
		_outch (' ');
		if (SP->phys_irm == 1 && insert_padding)
			tputs (insert_padding, 1, _outch);
	}
}

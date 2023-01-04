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
static char rcsid[] = "@(#)$RCSfile: meta.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 21:57:13 $";
#endif
/*
 * HISTORY
 */
/*** 1.7  com/lib/curses/meta.c, , bos320, 9134320 8/12/91 09:30:32 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   meta
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
 * NAME:        meta
 *
 * EXECUTION ENVIRONMENT:
 *
 *      TRUE => all 8 bits of input character should be passed through.
 */

meta(win,bf)
WINDOW *win; int bf;
{
	int _outch();

	if (!has_meta_key)
		return ERR;
	/*
	 * Do the appropriate fiddling with the tty driver to make it send
	 * all 8 bits through.  On USG this means clearing ISTRIP, on
	 * V7 you have to resort to RAW mode.
	 */
#ifdef USG
	if (bf) {
		(cur_term->Nttyb).c_iflag &= ~ISTRIP;
		(cur_term->Nttyb).c_cflag &= ~CSIZE;
		(cur_term->Nttyb).c_cflag |= CS8;
		(cur_term->Nttyb).c_cflag &= ~PARENB;
	} else {
		(cur_term->Nttyb).c_iflag |= ISTRIP;
		(cur_term->Nttyb).c_cflag &= ~CSIZE;
		(cur_term->Nttyb).c_cflag |= CS7;
		(cur_term->Nttyb).c_cflag |= PARENB;
	}
#else
	if (bf)
		raw();
	else
		noraw();
#endif
	_reset_prog_mode();					/* 001 */

	/*
	 * Do whatever is needed to put the terminal into meta-mode.
	 */
	if (bf)
		tputs(meta_on, 1, _outch);
	else
		tputs(meta_off, 1, _outch);

	/* Keep track internally. */
	win->_use_meta = bf;

	return OK;
}

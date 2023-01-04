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
static char rcsid[] = "@(#)$RCSfile: _reset.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:46:08 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_reset.c  1.6  com/lib/curses,3.1,8943 10/16/89 22:59:30";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _reset
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
 * NAME:        _reset
 */

_reset ()
{
	extern int _nocursor;

	tputs(init_1string, 0, _outch);
#ifdef DEBUG
	if(outf) fprintf(outf, "_reset().\n");
#endif
	tputs(enter_ca_mode, 0, _outch);
	if (!_nocursor)
		tputs(cursor_visible, 0, _outch);
	tputs(exit_attribute_mode, 0, _outch);
	tputs(clear_screen, 0, _outch);
	SP->phys_x = 0;
	SP->phys_y = 0;
	SP->phys_irm = 1;
	SP->virt_irm = 0;
	SP->phys_top_mgn = 4;
	SP->phys_bot_mgn = 4;
	SP->des_top_mgn = 0;
	SP->des_bot_mgn = lines-1;
	SP->ml_above = 0;
	_setwind();
}

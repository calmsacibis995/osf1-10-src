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
static char rcsid[] = "@(#)$RCSfile: set_term.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:29:18 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "set_term.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:38:55"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   set_term
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
 * NAME:        set_term
 */

struct screen *
set_term(new)
struct screen *new;
{
	register struct screen *rv = SP;

#ifdef DEBUG
	if(outf) fprintf(outf, "setterm: old %x, new %x\n", rv, new);
#endif

#ifndef		NONSTANDARD
	SP = new;
#endif		/* NONSTANDARD */

	cur_term = SP->tcap;
	LINES = lines;
	COLS = columns;
	stdscr = SP->std_scr;
	curscr = SP->cur_scr;
	return rv;
}

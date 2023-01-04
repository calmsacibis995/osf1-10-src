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
static char rcsid[] = "@(#)$RCSfile: curses.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:10:40 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "curses.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:09:43";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   curses.c
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include       "cursesext.h"

/*
 * NAME:        curses.c
 *
 * FUNCTION:
 *
 *      Define global variables.
 */

char	*Def_term	= "unknown";	/* default terminal type	*/
WINDOW *stdscr, *curscr;
int	LINES, COLS;
struct screen *SP;

/* char *curses_version = "Packaged for USG UNIX 6.0, 3/6/83"; */
char *curses_version = " ";

# ifdef DEBUG
FILE	*outf;			/* debug output file			*/
# endif

struct	term _first_term;
struct	term *cur_term = &_first_term;

WINDOW *lwin;

int _endwin = FALSE;

int	tputs();

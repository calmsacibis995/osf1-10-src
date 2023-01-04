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
static char rcsid[] = "@(#)$RCSfile: restarttrm.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 22:24:00 $";
#endif
/*
 * HISTORY
 */
/*** "restarttrm.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:37:02"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   restartterm
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
#include <unistd.h>		/* for POSIX_VDISABLE */


extern	struct term *cur_term;


/*
 * NAME:        restartterm
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is useful after saving/restoring memory from a file (e.g. as
 *      in a rogue save game).  It assumes that the modes and windows are
 *      as wanted by the user, but the terminal type and baud rate may
 *      have changed.
 */

restartterm(term, filenum, errret)
char *term;
int filenum;	/* This is a UNIX file descriptor, not a stdio ptr. */
int *errret;
{
	int saveecho = SP->fl_echoit;
	int savecbreak = SP->fl_rawmode;
	int saveraw;
	int savenl;

#ifdef USG
	saveraw = (cur_term->Nttyb).c_cc[VINTR] == _POSIX_VDISABLE;
	savenl = (cur_term->Nttyb).c_iflag & ICRNL;
#else
	saveraw = (cur_term->Nttyb).sg_flags | RAW;
	savenl = (cur_term->Nttyb).sg_flags & CRMOD;
#endif

	setupterm(term, filenum, errret);

	/*
	 * Restore curses settable flags, leaving other stuff alone.
	 */
	if (saveecho)
		echo();
	else
		noecho();

	if (savecbreak)
		cbreak(), noraw();
	else if (saveraw)
		nocbreak(), raw();
	else
		nocbreak(), noraw();
	
	if (savenl)
		nl();
	else
		nonl();

	_reset_prog_mode();					/* 001 */

	LINES = lines;
	COLS = columns;
}

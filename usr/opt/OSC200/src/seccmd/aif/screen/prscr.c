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
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: prscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:25 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1989, 1990 SecureWare, Inc.  All rights reserved.
 */



/* print screen routines
 */

#include	<sys/stat.h>
#include	<stdio.h>
#include	<sys/security.h>
#include	<sys/audit.h>
#include	"userif.h"
#include	"kitch_sink.h"
#include	"logging.h"

#ifdef DEBUG
extern	FILE	*logfp;
#endif DEBUG


/* Help directory which stores the root of the help tree */
extern char *HelpDir;


/* routine that prints a screen on stdout */

extern int scr_pag;
static scr_cnt = 0;	/* count of screens this page so far */

#if defined(DEBUG) || defined(PRINTSCR)

printscreen (screenp)
struct	scrn_parms	*screenp;
{
	register int x, y;
	static FILE *psfp;
	FILE *fopen ();
	struct scrn_struct *sp, *buildscrn_struct();

	ENTERFUNC ("printscreen");
	DUMPVARS ("screen title='%s'", screenp->sh->title, NULL, NULL);
	if (!psfp)
		psfp = fopen ("SWscreens", "w");
	if (!psfp) {
		printf ("Could not open SWscreens for writing\n");
		exit (1);
	}
	if (!stdscr)
		initcurses ();
	if (!stdscr) {
		printf ("Could not initialize screen\n");
		exit (1);
	}
	if (scr_pag == 0)
		scr_pag = 1;
	scr_cnt++;
	if (scr_cnt == scr_pag) {
		scr_cnt = 0;
		fputc ('', psfp);
	}
	initscreen (screenp, 0);
	sp = buildscrn_struct (screenp);
	screenp->ss = sp;
	screenp->inuse = 1;
	headers (screenp);
	putscreen (screenp, sp, POP);
	for (y = 0; y < LINES; y++) {
		for (x = 0; x < COLS; x++) {
			if (screenp->w &&
				  y >= screenp->w->_begy &&
				  x >= screenp->w->_begx &&
				  y < screenp->w->_maxy + screenp->w->_begy &&
				  x < screenp->w->_maxx + screenp->w->_begx &&
				  (char) screenp->w->_y[y-screenp->w->_begy][x-screenp->w->_begx]) {
				fputc (((char)
				  screenp->w->_y[y-screenp->w->_begy][x-screenp->w->_begx] &
				  0x7f), psfp);
			} else if ((char) stdscr->_y[y][x]) {
				fputc (((char) stdscr->_y[y][x] & 0x7f), psfp);
			}
		}
		fputc ('\n', psfp);
		fflush (psfp);
	}
	freescrn_struct (sp);
	screenp->ss = NULL;
	screenp->inuse = screenp->si = 0;
	EXITFUNC ("printscreen");
}


/* for a screen, list FLD_BOTH or FLD_INPUT fields that don't
 * have help files defined.  Also list fields for which files do not exist.
 * The prefix argument is the pathname of the release directory.
 */


printhelp (prefix, scrn)
char *prefix;
struct scrn_parms *scrn;
{
	unsigned int i;
	struct scrn_desc *sd = scrn->sd;
	char filename[100];
	int len;
	struct stat sb;

	if (scrn->scrntype == SCR_NOCHANGE)
		return;
	/* build the file name prefix for all help screens */
	(void) sprintf (filename, "%s%s", prefix, HelpDir);
	len = strlen (filename);

	for (i = 0; i < scrn->ndescs; i++, sd++) {
		if (sd->inout == FLD_OUTPUT)
			continue;
		/* on menu prompt screens, choice fields with
		 * blanks don't have help, choice fields without
		 * blanks do.
		 */
		if (scrn->scrntype == SCR_MENUPROMPT &&
		    sd->type == FLD_CHOICE &&
		    i < scrn->ndescs - 1 &&
		    (sd+1)->type != FLD_CHOICE &&
		    (sd+1)->inout == FLD_INPUT)
			continue;
		else if (sd->help == (char *) 0)
			(void) printf ("Field %d, no help defined\n", i);
		else {
			(void) strcpy (&filename[len], sd->help);
			if (prefix && stat (filename, &sb) < 0)
				(void) printf ("%s not found\n", filename);
			else
				(void) puts (filename);
		}
	}
}


#endif /* DEBUG || PRINTSCR */

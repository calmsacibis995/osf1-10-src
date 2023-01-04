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
static char	*sccsid = "@(#)$RCSfile: helpscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:55 $";
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
 * Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
 */



/* This file contains all userif-based help routines
 */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/signal.h>
#include	<ctype.h>
#include	<stdio.h>
#include	<sys/security.h>
#include	<sys/audit.h>
#include	"userif.h"
#include	"curs_supp.h"
#include	"key_map.h"
#include	"Utils.h"

#include "kitch_sink.h"

void helpscrn();

/* Help directory which stores the root of the help tree */
extern char *HelpDir;


/*  "help" screen display */
void
help (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;

	sdp = sttosd (stp);
	if (sdp->help == NULL)
		message (stp, stp->window, stp->screenp, NOHELP, NO);
	else
		_HelpDisplayOpen (sdp->help);
	/* necessary becauses curses handles overlapped windows buggily */
	touchwin (stp->window);
	/* move back to original place on the screen */
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmove (stp->window, row, col + stp->columninfield);
	return;
}

#define HELPFILELEN	256

void
helpscrn (name)
char	*name;
{
	FILE	*fp;
	char	helpfile[HELPFILELEN];
	struct	stat	sb;
	char	buf[80];
	char	*cp;
	unsigned int	i;
	unsigned int	nlines;
	char	*strings = (char *) 0;
	char	**list_strings;

	/* This is a lower level screen, so pop up just ONE level */

	sprintf (helpfile, "%s%s", HelpDir, name);
	if (name == NULL || name[0] == '\0' ||
	    stat (helpfile, &sb) < 0 || sb.st_size == 0) {
		sprintf (buf, "Help file %s does not exist or is empty.", name);
		pop_msg (buf, "There is no help available for this item.");
		goto BOOGIE;
	}
	fp = fopen (helpfile, "r");
	if (fp == (FILE *) 0) {
		sprintf (buf,
		 "Help file %s is not readable.",
		 name);
		pop_msg (buf, "There is no help available for this item.");
		goto BOOGIE;
	}

	/* determine the file size & load it */

	strings = Malloc (sb.st_size);
	if (strings == (char *) 0) {
		fclose (fp);
		MemoryError ();	/* dies */
	}
	if (fread (strings, sb.st_size, 1, fp) != 1) {
		Free (strings);
		fclose (fp);
		MemoryError ();	/* dies */
	}
	fclose (fp);

	/* split each line out */

	nlines = 0;
	for (cp = strings, i = 0; i < sb.st_size; i++, cp++)
		if (*cp == '\n') {
			nlines++;
		}
	if (!(list_strings = (char **) Calloc (nlines, sizeof (char *))))
		MemoryError ();	/* dies */

	cp = strings;
	for (i = 0; i < nlines; i++) {
		list_strings[i] = cp;
		cp = strchr (cp, '\n');
		*cp++ = '\0';
	}

	HelpPopup (nlines, list_strings);

	Free (list_strings);
	Free (strings);
	strings = (char *) 0;
BOOGIE:
	return;
}



HelpPopup (nlines, list_strings)
int nlines;
char **list_strings;
{
	struct scrn_parms	scrn;
	int			i, widest, thiswide;
	struct scrn_desc	*sd = (struct scrn_desc *) 0;
	extern Scrn_hdrs	helpk_hdrs;

	sd = (struct scrn_desc *) Calloc (nlines, sizeof (*sd));
	if (sd == (struct scrn_desc *) 0) {
		MemoryError ();	/* dies */
	}

	for (i = 0; i < nlines; i++) {
		sd[i].row = i;
		sd[i].col = 0;
		sd[i].type = FLD_PROMPT;
		sd[i].inout = FLD_OUTPUT;
		sd[i].prompt = list_strings[i];
	}

	scrn.sh = &helpk_hdrs;
	widest = 0;
	for (i = 0; i < nlines; i++)
		if ((thiswide = strlen (sd[i].prompt)) > widest)
			widest = thiswide;
	scrn.nbrrows = MIN (nlines, MENU_ROWS);
	scrn.toprow = (LINES - scrn.nbrrows) / 2 - 1;
	scrn.nbrcols = MIN (widest, COLS);
	scrn.leftcol = (COLS - scrn.leftcol) / 2 - 1;
	adj_cols (&scrn);
	scrn.sd = sd;
	scrn.ndescs = nlines;
	scrn.scrntype = SCR_MSGONLY;
	scrn.w = newwin (scrn.nbrrows, scrn.nbrcols, scrn.toprow, scrn.leftcol);
	headers (&scrn);

	putscreen (&scrn, NULL, POP);
	scrn_enter_input_mode(NULL);
	(void) get_key_conv (scrn.w);
	scrn_exit_input_mode(NULL);
	delwin (scrn.w);

	Free (sd);
	sd = (struct scrn_desc *) 0;
}

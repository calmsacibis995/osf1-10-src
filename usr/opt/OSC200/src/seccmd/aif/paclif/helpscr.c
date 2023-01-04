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
static char *rcsid = "@(#)$RCSfile: helpscr.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:11:47 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	helpscr.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:04:10  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:59:04  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:38  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:16  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
 *
 * Based on OSF version:
 *	@(#)helpscr.c	1.11 16:31:04 5/17/91 SecureWare
 */

/* #ident "@(#)helpscr.c	1.1 11:16:42 11/8/91 SecureWare" */

/*
 * This file contains all terminfo interface help support routines
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

/* 
 * For help, an item might have its own help action that is accessible
 * when the user chooses that item, the specific item might have context-
 * sensitive help, or the screen may have help.  Help is checked in
 * that order.  If there is no help, a "no help available" message is
 * listed on the screen.
 */

void
help(stp)
register struct	state	*stp;
{
	Scrn_desc *sdp;
	register Menu_scrn *mp;
	register Scrn_parms *screenp = stp->screenp;
	uchar	row, col;
	int mi;			/* menu_desc index */
	int trav_ret;		/* traverse() return code */
	int handled = 0;	/* has help been taken care of yet? */

	sdp = sttosd(stp);

	if (screenp->ms != (Menu_scrn *) 0) {
		/*
		 * find menu structure associated with screen and see if
		 * it has associated help
		 */

		mi = sdtocn(screenp, &sdp, (sdp - screenp->sd));
		for (mp = screenp->ms; mp->choice != mi; mp++)
			;
		mi = mp - screenp->ms;
		if (screenp->ms[mi].help) {	/* "action" help */
			trav_ret = traverse(stp->screenp->ms[mi].help, -1);

			/* set return state to propagate if necessary */

			switch (trav_ret) {
			case QUIT:
				stp->ret.flags |= R_QUIT;
				break;
			case ABORT:
				stp->ret.flags |= R_ABORTED;
				break;
			default:
				stp->ret.flags |= R_ACTHELP;
				break;
			}
			stp->ret.item = stp->curfield;
			handled = 1;
		}
	}

	if (!handled) {
		/*
		 * Check for item-specific help specified in scrn_desc
		 */

		if (sdp->help != NULL)
			_HelpDisplayOpen(sdp->help);

		/*
		 * Check for help message for entire screen in scrn_parms
		 */

		else if (screenp->sh != NULL && screenp->sh->help != NULL)
			_HelpDisplayOpen(screenp->sh->help);

		/*
		 * Otherwise, print "no help available" message
		 */

		else {
			char *helpmsg[1];

			helpmsg[0] = Malloc(strlen(NOHELP) + 1);
			if (helpmsg[0] == NULL)
				MemoryError();
			strcpy(helpmsg[0], NOHELP);
			HelpPopup(1, helpmsg);
			free(helpmsg[0]);
		}
	}

	/* restore the cursor to its former location on the screen */

	touchwin(stp->window);

	/* move back to original place on the screen */

	reenterfield(stp, sdp);

	return;
}

/*
 * Display a list in the FLD_SCROLL item for the screen.
 * This is a scrolling region because the contents of the screen
 * may well exceed the screen size.
 */

struct helpfd_fill {
	char **list_strings;
	int nlines;
} help_buf, *helpfd_fill = &help_buf;

static Scrn_struct *helpfd_struct;

/***********************************************************

		FILE DISPLAY SCREEN SUPPORT

***********************************************************/

static Scrn_desc helpfd_desc[] = {

	{ -1, -1, FLD_SCROLL, 78, FLD_OUTPUT, NO, NULL, NULL,
		0, 0, 1 },		/* dynamically set */

};

Scrn_hdrs helpfd_hdrs = {
	"HELP MESSAGE", cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, "^D=Down scroll  ^U=Up scroll", 
        "RET=Return to screen",
	NULL
};

extern int helpfd_exit(), helpfd_init();

static Menu_scrn helpfd_menu[] = {
	ROUT_DESC(1, helpfd_exit, 0, NULL),
};

static
SCRN_PARMF(helpfd_scrn, SCR_NOCHANGE, helpfd_desc, helpfd_menu, NULL,
	&helpfd_hdrs,
	NULL, helpfd_init, NULL);

HelpPopup(nlines, list_strings)
int nlines;
char **list_strings;
{
	int len;
	int spaces;
	char *new_line0;
	int i;
	int widest_line;

	/* Determine widest line */

	widest_line = 0;
	for (i = 0; i < nlines; i++) {
		len = strlen(list_strings[i]);
		if (len > widest_line)
			widest_line = len;
	}

	/*
	 * need one space of margin on each side, plus two spaces for
	 * scroll indicators.  Truncate anything that's too long.
	 */

	if (widest_line > MENU_COLS - 4) {
		widest_line = MENU_COLS - 4;

		for (i = 0; i < nlines; i++)
			if (strlen(list_strings[i]) > MENU_COLS - 4)
				list_strings[i][MENU_COLS - 4] = '\0';
	}

	/* Center help contents */

	helpfd_desc[0].col = (MENU_COLS - widest_line) / 2;
	helpfd_desc[0].len = widest_line;
	helpfd_desc[0].inout = FLD_OUTPUT;
	
	/* Center first string */

	len = strlen(list_strings[0]);
	spaces = ((MENU_COLS - len) / 2) - helpfd_desc[0].col;
	new_line0 = Malloc(len + spaces + 1);
	if (new_line0 == NULL)
		MemoryError();

	/* put spaces, line contents, NULL into new line 0 */

	memset(new_line0, ' ', spaces);
	strcpy(new_line0 + spaces, list_strings[0]);

	/* de-allocate old and assign new line */

	Free(list_strings[0]);
	list_strings[0] = new_line0;
	
	/* Determine the first line and the number of lines */

	if (nlines > MENU_ROWS) {
		helpfd_desc[0].s_lines = MENU_ROWS;
		helpfd_desc[0].row = 0;
	} else {
		helpfd_desc[0].s_lines = nlines;
		helpfd_desc[0].row = (MENU_ROWS - nlines) / 2;
	}

	/* save off these values to plug into scrn_struct in helpfd_bstruct() */

	helpfd_fill->nlines = nlines;
	helpfd_fill->list_strings = list_strings;

	traverse(&helpfd_scrn, 0);

	/*
	 * Release the screen window, because next time it'll be a
	 * different size.
	 */

	if (helpfd_scrn.w) {
		delwin(helpfd_scrn.w);
		helpfd_scrn.w = NULL;
	}
}

#define PARMTEMPLATE	helpfd_scrn
#define STRUCTTEMPLATE	helpfd_struct
#define DESCTEMPLATE	helpfd_desc
#define FILLINSTRUCT	helpfd_fill
#define FILLIN		helpfd_fill

#define FIRSTDESC	0
#define NSCRNSTRUCT	1

/*
 * Function to initialize the scrn_struct for the screen to reference the list
 */

static int
helpfd_bstruct(fill, sptemplate)
	struct helpfd_fill *fill;
	struct scrn_struct **sptemplate;
{
	struct scrn_struct *sp;

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	sp->pointer = (char *) fill->list_strings;
	sp->filled = fill->nlines;
	sp->changed = 1;
	return 0;
}

static int
helpfd_action()
{
	return ABORT;
}

static int
helpfd_valid()
{
	return 0;
}

#define INITFUNC	helpfd_init
#define BUILDSTRUCT	helpfd_bstruct

#define ROUTFUNC	helpfd_exit
#define VALIDATE	helpfd_valid
#define SCREENACTION	helpfd_action

#include "stemplate.c"

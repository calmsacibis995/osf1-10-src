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
static char *rcsid = "@(#)$RCSfile: msgscr.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:12:22 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	msgscr.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:04:45  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:59:33  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:55  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:45  marquard]
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
 *	@(#)msgscr.c	1.9 16:31:12 5/17/91 SecureWare
 */

/* #ident "@(#)msgscr.c	1.1 11:17:20 11/8/91 SecureWare" */

/* msgscr.c - screen message routines
 * programs.  All routines which call into here use the header file "userif.h".
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

/*
 *	put a message in the window at the normal message row and column
 */

static WINDOW *msgwin = NULL;

void
message (stp, window, screenp, string, confirm)
struct state	*stp;
WINDOW	*window;		/* which window we're in */
struct	scrn_parms *screenp;	/* characteristics of window (row and col) */
char	*string;		/* message itself */
uchar	confirm;		/* whether to ask user to press return first */
{
	int	thischar;
	int	i, count, len;

	ENTERLFUNC ("message");
	DUMPLARGS ("string='%s' confirm=<%d>", string, confirm, NULL);
	len = strlen (string);
	if (WMSGLEN < len)
		len = WMSGLEN;
	if (msgwin)
		delwin (msgwin);
	msgwin = newwin(1, WMSGLEN, WMSGROW, WMSGCOL);

	if (confirm == YES)
		cursor (BLOCKCURSOR);
	WLEFTH (msgwin, WMSGROW, string);
	wrefresh (msgwin);

	if (confirm == YES) {
		scrn_enter_input_mode(stp);
		while (thischar = get_key_conv(msgwin))  {
			scrn_exit_input_mode(stp);
			switch (thischar) {
			case ENTER:
			case '\r':
			case '\n':
				goto out;
			case REDRAW:
				clearok (msgwin, TRUE);
				wrefresh (window);
				WLEFTH (msgwin, WMSGROW, string);
				wrefresh (msgwin);
				break;
			default:
				beep();
			}
		}
		scrn_exit_input_mode(stp);
		rm_message (window);
	}
out:
	if (stp && msgwin)
		stp->message = stp->curfield;
	EXITLFUNC ("message");
	return;
}



/*
 *	rm_message() - rmove a message() window && refresh the screen
 */
void
rm_message (window)
WINDOW *window;
{

	ENTERLFUNC ("rm_message");
	if (msgwin) {
		delwin (msgwin);
		msgwin = NULL;
		refresh ();
		wrefresh (window);
	} else {
		DUMPLDETI ("   (no message)", NULL, NULL, NULL);
	}
	EXITLFUNC ("rm_message");
	return;
}


/* message screen */

static
char *tp[3] = {
NULL, NULL, NULL
};

Scrn_hdrs hp = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, MT, MT,
		    "Press any key to continue"};

/*
 * DispMessage() - more general message display - uses char **
 */


Scrn_hdrs mp = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, MT, MT,
		    "Press any key to continue"};

static
struct	scrn_parms	Initial_Msgscrn = {
	SCR_TEXT,		/* scrntype */
	0, 0, 2, 0,		/* toprow, leftcol, nbrrows, nbrcols */
	0, 0, 0,		/* ndescs, inuse, si */
	NULL, NULL, NULL,	/* sd, ms, w */
	tp, &hp			/* text, sh */
};
static struct scrn_parms	Msgscrn;

/*
 * Display a message indicated by the index'th element into the
 * class_ptr array.
 */

void
DispMessage(class_ptr, index, char_ptr)
	char **class_ptr;
{
	char **temp_table;
	int i;
	int count;

	ENTERLFUNC ("DispMessage");

	/* 
	 * Yet another kml hack.  This one is necessary because the
	 * screen display code adjusts the leftcol, nbrrows, and other
	 * parameters each time the screen is displayed.  This results 
	 * in values that gradually creep, then overflow, then -- boom.  
	 * Core dump.  Ugh.
	 * 
	 * The fix -- we'll just reinitialize the screen parameters
	 * every time!
	 */

	Msgscrn = Initial_Msgscrn;

	/* determine the number of lines in the message */

	count = 0;
	for (i = index; class_ptr && class_ptr[i]
				&& class_ptr[i][0] != '\0'; i++)
		count++;

	if (count == 0) {
		char buf1[80], buf2[80], *cp;

		sprintf(buf1, MSGSTR(MSGSCR_1, "Internal error, no messages for index %d"),
				index);
		if (class_ptr == NULL || class_ptr[0] == NULL)
			cp = MSGSTR(MSGSCR_2, "NULL");
		else
			cp = class_ptr[0];
		sprintf(buf2, MSGSTR(MSGSCR_3, "first message is '%s'"), cp);
	}

	/* allocate a character pointer table with that many entries */

	temp_table = (char **) Calloc(sizeof(char *), count);
	if (temp_table == (char **) 0)
		MemoryError();

	/* fill the table with the message */

	for (i = 0; i < count; i++) {
		char *msg = class_ptr[index + i];

		/* If there's a format string, allocate maximum width */

		if (strchr(msg, '%') != NULL && char_ptr != NULL) {
			temp_table[i] = (char *) Malloc(COLS + 1);
			if (temp_table[i] == NULL)
				MemoryError();
			sprintf(temp_table[i], msg, char_ptr);
		} else {
			temp_table[i] = (char *) Malloc(strlen(msg) + 1);
			if (temp_table[i] == NULL)
				MemoryError();
			strcpy(temp_table[i], msg);
		}
	}

	Msgscrn.text = temp_table;
	Msgscrn.nbrrows = count;
	traverse(&Msgscrn, 0);

	/* free the memory and window allocated for the screen */

	for (i = 0; i < count; i++)
		free(temp_table[i]);
	free((char *) temp_table);

	if (Msgscrn.w) {
		delwin(Msgscrn.w);
		Msgscrn.w = NULL;
	}

	EXITLFUNC ("DispMessage");
	return;
}

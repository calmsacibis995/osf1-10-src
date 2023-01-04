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
static char	*sccsid = "@(#)$RCSfile: msgscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:19 $";
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



/* msgscr.c - screen message routines
 * programs.  All routines which call into here use the header file "userif.h".
 */

#include	<stdio.h>
#include	"userif.h"
#include	"curs_supp.h"
#include	"key_map.h"
#include	"kitch_sink.h"
#include	"logging.h"

/*
 *	put a message in the window at the normal message row and column
 */

WINDOW *msgwin = NULL;

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
	len = MIN (len, WMSGLEN);
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
		msgwin - NULL;
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

static Scrn_hdrs hp = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, MT, MT, cmds_anykey
};

static
struct	scrn_parms	msgscrn = {
	SCR_TEXT,
	0, 0, 2, 0,
	0, NULL, 0,
	NULL, NULL, NULL,
	tp, &hp
};

/*
 *	pop_msg() - pop up a message via SCR_TEXT screen
 */
void
pop_msg (string1, string2)
char	*string1, *string2;
{
	int	len1, len2;

	ENTERLFUNC ("pop_msg");
	DUMPLARGS ("msg: '%s' '%s'", string1, string2, NULL);
	len1 = strlen (string1);
	if (len1 > COLS - 4) {
		string1[COLS - 4] = '\0';
		len1 = COLS - 4;
	}
	len2 = strlen (string2);
	if (len2 > COLS - 4) {
		string2[COLS - 4] = '\0';
		len2 = COLS - 4;
	}
	msgscrn.nbrcols = MAX (len1, len2) + 4;
	tp[0] = string1;
	tp[1] = string2;
	traverse (&msgscrn, 0);
	endwin (msgscrn.w);
	EXITLFUNC ("pop_msg");
	return;
}

/*
 * DispMessage() - more general message display - uses char **
 */


static Scrn_hdrs mp = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, MT, MT, cmds_anykey
};

static
struct	scrn_parms	Msgscrn = {
	SCR_TEXT,		/* scrntype */
	0, 0, 2, 0,		/* toprow, leftcol, nbrrows, nbrcols */
	0, 0, 0,		/* ndescs, inuse, si */
	NULL, NULL, NULL,	/* sd, ms, w */
	tp, &hp			/* text, sh */
};

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

	/* determine the number of lines in the message */

	count = 0;
	for (i = index; class_ptr && class_ptr[i]
				&& class_ptr[i][0] != '\0'; i++)
		count++;

	if (count == 0) {
		char buf1[80], buf2[80], *cp;

		sprintf(buf1, "Internal error, no messages for index %d",
				index);
		if (class_ptr == NULL || class_ptr[0] == NULL)
			cp = "NULL";
		else
			cp = class_ptr[0];
		sprintf(buf2, "first message is '%s'", cp);
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
	traverse (&Msgscrn, 0);
	endwin(Msgscrn.w);

	/* free the memory allocated for the screen */

	for (i = 0; i < count; i++)
		free(temp_table[i]);
	free((char *) temp_table);

	EXITLFUNC ("DispMessage");
	return;
}

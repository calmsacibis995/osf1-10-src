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
static char	*sccsid = "@(#)$RCSfile: fillscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:44 $";
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



/*
 * This file contains all key response routines for FILLIN screens.
 * SCROLL, SCRTOG & TEXT-specific functions are in scrscr.c.
 *
 * The following keyboard handlers wrap around the screen when necessary:
 *
 *	downfill()	- move down to next field accepting input
 *	upfill()	- move up to previous field accepting input
 *	leftfill()	- move left one character or field
 *	rightfill()	- move right one character or field
 *	tabfill()	- move right to next field accepting input
 *	backtabfill()	- move left to previous field accepting input
 *
 * The following routines manipulate text inside fields that allow it:
 *
 *	delword()	- delete a "word" in s field
 *	backspace()	- handle backspace key (destructive)
 *	insfield()	- insert a blank "field" in a SCROLLing field
 *	delfield()	- delete a "field"
 *	instoggle()	- toggle insert mode
 *	fillinkey()	- handle "regular" key - text, selection, etc.
 *	ins_text_char()	- insert char in TEXT field
 *	del_text_char()	- delete char in TEXT field
 *	disp_text()	- redisplay changed TEXT field after ins_ or del_
 *
 * Miscellaneous routines:
 *
 *	reqfillin()	- check for required items in a fillin screen
 *	off_insert_ind()- turn INSert indicator on screen off
 *	on_insert_ind()	- turn INSert indicator on screen on
 */

#include	<stdio.h>
#include	<sys/security.h>
#include	<sys/audit.h>
#include	"userif.h"
#include	"curs_supp.h"
#include	"key_map.h"
#include	"kitch_sink.h"
#include	"logging.h"



/* "down" key in a fillin screen 
 * move to the first field on the next row that has fillin prompts
 */
void
downfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	int	i, row;

	/* if error return, stay in same field */
	if (leavefield (stp))
		return;
	sdp = sttosd (stp);
	/* if scrolling region, handle separately */
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	sdp->type == FLD_TEXT) {
		downscrollreg (stp, sdp);
		return;
	}
	/* move to the first item on the next line */
	if ((i = finddownitem(stp, sdp)) != -1) {
		stp->curfield = i;
		movetofield (stp);
		sdp = sttosd (stp);
	}
	else	
	/*  no later items -- another choice would be to wrap around to top */
		beep();
	return;
}

/* "up" key in a fillin screen */
void
upfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	int	i, row;

	/* stay in the same field if there was an error */
	if (leavefield (stp))
		return;

	sdp = sttosd (stp);
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	sdp->type == FLD_TEXT)  {
		upscrollreg (stp, sdp);
		return;
	}

	if ((i = findupitem (stp, sdp)) != -1)  {
		stp->curfield = i;
		movetofield (stp);
		sdp = sttosd (stp);
	}  else			/* the other choice would be to wrap */
		beep();
	return;
}

/* "left" key in a fillin screen */
void
leftfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;

	sdp = sttosd (stp);
	if (stp->columninfield == 0 || sdp->type == FLD_SCRTOG)  {
		beep();
		return;
	}
	stp->columninfield--;
	/* if there is a move left one character call, use it here */
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_TEXT)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmove (stp->window, row, col + stp->columninfield);
	return;
}

/* "right" key in a fillin screen */
void
rightfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;

	sdp = sttosd (stp);
	if (sdp->type == FLD_SCRTOG) {
		beep();
		return;
	}
	if (stp->columninfield == sdp->len - 1)  {  /* right edge */
		beep();
		return;
	}
	stp->columninfield++;
	/* if there is a move right one character call, use it here */
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_TEXT)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}
	wmove (stp->window, row, col + stp->columninfield);
	return;
}


/* "tab" key in a fillin screen
 * move to next field that is not OUTPUT only
 * wraps around to first field if on the last field
 */
void
tabfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register int	i;

	/* update the item as changed, if necessary */
	if (leavefield (stp))
		return;
	sdp = sttosd (stp);
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	sdp->type == FLD_TEXT) {
		tabscrollreg (stp, sdp);
		return;
	}
	/* move to next field that is input or both field
	 * wrap around to top of screen if necessary
	 */
	if (stp->curfield == stp->lastfield)
		stp->curfield = stp->firstfield;
	else {
		stp->curfield++;
		sdp++;
		for (i = stp->curfield; i <= stp->lastfield; i++, sdp++) {
			if (sdp->type == FLD_PROMPT)
				continue;
			if (sdp->inout == FLD_OUTPUT && sdp->type != FLD_SCROLL
			&& sdp->type != FLD_TEXT)
				continue;
			break;
		}
		stp->curfield = i;
	}
	movetofield (stp);
	sdp = sttosd (stp);
	return;
}


/* "back tab" key in a fillin screen */
void
backtabfill (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register int	i;

	/* update the item as changed, if necessary */
	if (leavefield (stp))
		return;
	sdp = sttosd (stp);
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	sdp->type == FLD_TEXT) {
		backtabscrollreg (stp, sdp);
		return;
	}
	/* move to previous field that is input or both (wrap) */
	if (stp->curfield == stp->firstfield)
		stp->curfield = stp->lastfield;
	else {
		sdp--;
		stp->curfield--;
		for (i = stp->curfield; i >= stp->firstfield; i--, sdp--) {
			if (sdp->type == FLD_PROMPT)
				continue;
			if (sdp->inout == FLD_OUTPUT)
				continue;
			break;
		}
		stp->curfield = i;
	}
	movetofield (stp);
	sdp = sttosd (stp);
	return;
}


/*
 * The following routines all manipulate character data within fields.
 * They handle, where applicable, overstriking, insertion and deletion
 * of characters and words. For TEXT fields, they handle wrapping. No
 * word-wrap is provided. Character shifting on insert/delete stops when
 * a blank line is found on a TEXT field.
 */

/*
 * "delete word" key - delete until SPACE or NULL found
 */
void
delword (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	char	*field;		/* first character in word to delete */
	register char
		*lastfield,	/* last position in the field */
		*lastword;	/* first position past what is to delete */
	uchar	breakout;	/* flag for inner loop */

	sdp = sttosd (stp);
	if (sdp->type == FLD_SCRTOG) {
		beep();
		return;
	}
	field = stp->scrnrep;
	lastfield = &field[sdp->len];
	field = &field[stp->columninfield];
	/* find last character past this position that isn't SPACE */
	breakout = 0;
	for (lastword = field; lastword < lastfield; lastword++) {
		while ((*lastword == SPACE || *lastword == '\0') &&
		   lastword < lastfield) {
			lastword++;
			breakout = 1;
		}
		if (breakout)
			break;
	}
	delchars (stp, sdp, lastword - field);
	/* reflect that item was changed */
	stp->itemchanged = 1;
	return;
}

/* "backspace" key
 * Delete character to the left of the cursor
 */

void
backspace (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;
	int del_state = 1;
	int i;

	sdp = sttosd (stp);
	if (stp->columninfield == 0 || sdp->type == FLD_SCRTOG)  {
		beep();
		return;
	}
	if (sdp->type != FLD_TEXT)
		stp->columninfield--;
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_TEXT)
		scr_rowcol (stp, sdp, &row, &col);
	else  {
		row = sdp->row;
		col = sdp->col;
	}

	if (sdp->type == FLD_TEXT) {
		if (row == 0 && stp->columninfield == 0) {
			if (stp->topscroll == 0) {
				beep ();
				return ;
			} else {
				leftfill (stp);
			}
		} else {
			stp->columninfield--;
		}
		scr_rowcol (stp, sdp, &row, &col);
		del_state = del_text_char (s_text (sdp->scrnstruct),
			stp->columninfield, (row + stp->topscroll),
			sdp->len, sdp->scrnstruct->filled);
		if (!del_state) {
			stp->itemchanged = 1;
			strcpy (stp->scrnrep,
			    (s_text(sdp->scrnstruct))[row + stp->topscroll]);
			for (i = strlen (stp->scrnrep);
			    i < sizeof (stp->scrnrep); i++)
				stp->scrnrep[i] = '\0';
			disp_text (s_text (sdp->scrnstruct),
				stp->columninfield, row,
				sdp->len, sdp->s_lines, stp);
		}
		scr_rowcol (stp, sdp, &row, &col);
		wmove (stp->window, row, col + stp->columninfield);
	} else {
		wmove (stp->window, row, col + stp->columninfield);
		delchars (stp, sdp, 1);
		stp->itemchanged = 1;
	}
	/* reflect that item was changed */
	return;
}

/* "insert field" key
 * only works in SCROLL fields
 */

void
insfield(stp)
struct state	*stp;
{
	struct scrn_desc	*sdp;

	sdp = sttosd (stp);
	if (sdp->type != FLD_SCROLL)
		beep ();
	else {
		scroll_rshift (stp, sdp);
		stp->itemchanged = 1;
	}
	return;
}

/* "delete field" key
 * only works in ALPHA or NUMBER fields
 */

void
delfield (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;
	
	sdp = sttosd (stp);
	switch (sdp->type) {
	case FLD_ALPHA:
	case FLD_NUMBER:
	case FLD_SCROLL:
	case FLD_TEXT:
		stp->columninfield = 0;
		if (sdp->type == FLD_SCROLL || sdp->type == FLD_TEXT)
			scr_rowcol (stp, sdp, &row, &col);
		else  {
			row = sdp->row;
			col = sdp->col;
		}
		wmove (stp->window, row, col);
		delchars (stp, sdp, sdp->len);
		stp->itemchanged = 1;
		break;
	default:
		beep();
		break;
	}
	return;
}


/*
 * "Insert toggle" key
 */

void
instoggle (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	uchar	row, col;
	int	i;

	sdp = sttosd (stp);
	if (sdp->type == FLD_CHOICE)  {
		beep();
		return;
	}
	if (stp->insert)  {	/* turn off indicator */
		off_insert_ind();
		stp->insert = 0;
	} else {		/* turn on indicator & insert a space */
		on_insert_ind();
		stp->insert = 1;
	}
	/* move back to where the cursor was */
	if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG ||
	    sdp->type == FLD_TEXT) {
		scr_rowcol (stp, sdp, &row, &col);
	} else {
		row = sdp->row;
		col = sdp->col;
	}
	wmove (stp->window, row, col + stp->columninfield);
	return;
}


/*
 * Entered a regular key in a fillin window
 *
 * if it's OUTPUT only, laugh at the user
 * if it's a TOGGLE field, toggle it on space
 * otherwise try to process it logically by the field type
 */
void
fillinkey (stp, input)
register struct	state	*stp;
int	input;
{
	register struct	scrn_desc	*sdp;
	uchar	key = input & 0177;
	uchar	row, col;

	sdp = sttosd (stp);
	if (sdp->inout == FLD_OUTPUT) {
		beep ();
		return;
	}
	if (sdp->type == FLD_TOGGLE || sdp->type == FLD_SCRTOG) {
		if (key == ' ' && sdp->type == FLD_TOGGLE) {
			if (*(s_toggle (sdp->scrnstruct))) {
				*(s_toggle (sdp->scrnstruct)) = 0;
				if (sdp->len == 1) {
					mvwaddch (stp->window, sdp->row,
						sdp->col, TOGGLEOFF);
					stp->scrnrep[0] = TOGGLEOFF;
					stp->scrnrep[1] = NULL;
					wmove(stp->window, sdp->row, sdp->col);
				} else {
					unsettoggle(stp->window,
						sdp->row, sdp->col,
						stp->scrnrep, sdp->len);
					wmove(stp->window, sdp->row,
					  sdp->col + sdp->len + 1);
				}
			} else {
				*(s_toggle (sdp->scrnstruct)) = 1;
				if (sdp->len == 1) {
					mvwaddch (stp->window, sdp->row,
						sdp->col, TOGGLEON);
					stp->scrnrep[0] = TOGGLEON;
					stp->scrnrep[1] = NULL;
					wmove(stp->window, sdp->row, sdp->col);
				} else {
					settoggle (stp->window,
						sdp->row, sdp->col,
						stp->scrnrep, sdp->len);
					wmove(stp->window, sdp->row,
					  sdp->col + sdp->len + 1);
				}
			}	
			stp->itemchanged = 1;
		} else if (key == ' ' && sdp->type == FLD_SCRTOG) {
			stp->scrollitem = sdp->index;
			scr_rowcol(stp, sdp, &row, &col);
			if (sdp->scrnstruct->state[sdp->index]) {
				sdp->scrnstruct->state[sdp->index] = 0;
				if (sdp->len == 1) {
					mvwaddch (stp->window, row,
						col, TOGGLEOFF);
					stp->scrnrep[0] = TOGGLEOFF;
					stp->scrnrep[1] = NULL;
					wmove(stp->window, row, col);
				} else {
					unsettoggle (stp->window,
						row, col,
						stp->scrnrep, sdp->len);
					wmove(stp->window, row,
						col + sdp->len + 1);
				}
			} else {
				sdp->scrnstruct->state[sdp->index] = 1;
				if (sdp->len == 1) {
					mvwaddch (stp->window, row,
						col, TOGGLEON);
					stp->scrnrep[0] = TOGGLEON;
					stp->scrnrep[1] = NULL;
					wmove(stp->window, row, col);
				} else {
					settoggle (stp->window,
						row, col,
						stp->scrnrep, sdp->len);
					wmove(stp->window, row,
						col + sdp->len + 1);
				}
			}	
			stp->itemchanged = 1;
		} else {
			beep ();
		}
	} else if (sdp->type == FLD_CHOICE) {
		beep ();
	} else if (sdp->type == FLD_TEXT) {

		int ins_state = 1;
		int wrapped = 0;

		if (stp->insert) {	/* insert mode */
			scr_rowcol (stp, sdp, &row, &col);
			ins_state = ins_text_char (s_text (sdp->scrnstruct),
				stp->columninfield, (row + stp->topscroll),
				sdp->len, sdp->scrnstruct->filled, key);
			if (ins_state)
				beep ();
		}
		stp->itemchanged = 1;
		if (stp->columninfield == sdp->len - 1)  {
			stp->scrnrep[stp->columninfield] = key;
			if (stp->insert && !ins_state)
				tabfill (stp);
			wrapped = 1;
		} else {
			if (stp->insert && !ins_state)
				moveright (stp, sdp, 1);
			stp->scrnrep[stp->columninfield] = key;
		}
		stp->itemchanged = 1;
		scr_rowcol (stp, sdp, &row, &col);
		if (stp->insert && !ins_state) {
			if (wrapped)
				disp_text (s_text (sdp->scrnstruct),
					0, 0,
					sdp->len, sdp->s_lines, stp);
			else
				disp_text (s_text (sdp->scrnstruct),
					stp->columninfield, row,
					sdp->len, sdp->s_lines, stp);
		} else {
			UNDERLINE (stp->window, ON);
			WADDCHAR (stp->window, key);
			UNDERLINE (stp->window, OFF);
		}
		if (stp->columninfield != sdp->len - 1) {  /* EOL */
			if ((!stp->insert || !ins_state) && !wrapped)
				stp->columninfield++;
		} else if (!stp->insert) {
				tabfill (stp);
		}
		scr_rowcol (stp, sdp, &row, &col);
		wmove (stp->window, row, col + stp->columninfield);
				
	} else {
		if (stp->insert)  { /* insert mode */
			/* if at rightmost column, don't allow going off end.
			 * Otherwise, move rest of field to right, truncating
			 * rightmost characters.
			 */
			if (stp->columninfield == sdp->len - 1)  {
				beep();
			} else {
				moveright (stp, sdp, 1);
			}
		}
		stp->scrnrep[stp->columninfield] = key;
		stp->itemchanged = 1;
		/* assumes adding a character moves cursor one position */
		UNDERLINE (stp->window, ON);
		WADDCHAR (stp->window, key);
		UNDERLINE (stp->window, OFF);
		if (stp->columninfield == sdp->len - 1) {  /* end of line */
			if (sdp->type == FLD_SCROLL)
				scr_rowcol (stp, sdp, &row, &col);
			else  {
				row = sdp->row;
				col = sdp->col;
			}
			wmove (stp->window, row, col + stp->columninfield);
		} else {
			stp->columninfield++;
		}
	}

	/*
	 * if the field has a validate on action function, call it.
	 * leavefield is called to save the screen representation.
	 * reenterfield is called to restore highlights, etc.
	 */

	if (sdp->scrnstruct->val_act) {
		leavefield(stp);
		if ((*sdp->scrnstruct->val_act) ())
			redraw_screen(stp);
		reenterfield(stp, sdp);
		wnoutrefresh(stdscr);
		wnoutrefresh(stp->window);
		doupdate();
	}
	return;
}



/* insert char in a TEXT field */

int
ins_text_char (t, col, row, w, h, c)
char **t;	/* text field */
int col;	/* col of text to start on */
int row;	/* row of text to start on */
int w;		/* width of text field */
int h;		/* height of text field */
char c;		/* character to insert */
{
	register char *s;
	int x = col,
	    y = row;
	char t1, t2;

	ENTERFUNC ("ins_text_char");
	DUMPARGS (" x=<%d> y=<%d> s='%s'", x, y, t[0]);
	DUMPARGS (" w=<%d> h=<%d> c='%c'", w, h, c);
	if (x < 0 || x >= w || y < 0 || y >= h) {
		ERRFUNC ("ins_text_char", "bad size or dimension");
		return (1);
	}

	t1 = *(t[y] + x);
	for (s = t[y]; y < h; s = t[++y]) {
		for (s += x; s < t[y] + w; s++) {
			t2 = *(s + 1);		/* save next char */
			if (!t2)		/* early EOL? */
				break;
			*(s + 1) = t1;		/* shift right on line*/
			t1 = t2;		/* next becomes current */
		}
		if (s < t[y] + (w - 1)) {	/* if row not full */
			*(++s) = t1;		/* move last char, */
			*(++s) = NULL;		/* terminate, */
			break;			/* and split */
		}
		if (y < h - 1) {		/* if not last line */
			t2 = *(t[y + 1]);	/* char. wrap */
			*(t[y + 1]) = t1;
			t1 = t2;		/* save for next pass */
		}
		x = 0;
	}
	*(t[row] + col) = c;			/* now do insert */
	EXITFUNC ("ins_text_char");
	return (0);
}



/* delete char in a TEXT field */

int
del_text_char (t, col, row, w, h)
char **t;	/* text field */
int col;	/* col of text to start on */
int row;	/* row of text to start on */
int w;		/* width of text field */
int h;		/* height of text field */
{
	register char *s;
	int x = col,
	    y = row;

	ENTERFUNC ("del_text_char");
	DUMPARGS (" x=<%d> y=<%d> s='%s'", x, y, t[0]);
	DUMPARGS (" w=<%d> h=<%d>", w, h, NULL);
	if (x < 0 || x >= w || y < 0 || y >= h) {
		ERRFUNC ("del_text_char", "bad size or dimension");
		return (1);
	}

	for (s = t[y]; y < h; s = t[++y]) {
		for (s += x; s < t[y] + w && *s; s++) {	/* shift left */
			*s = *(s + 1);			/* within line */
			if (!*s)
				break;
		}

		if (s < t[y] + (w - 1) && !*s) {	/* if row not full */
			break;			/* split */
		}
		if (y < h - 1)			/* if not last line */
			*s = *(t[y + 1]);	/* char. wrap */
		x = 0;
	}
	EXITFUNC ("del_text_char");
	return (0);
}



/* display mod. portion of TEXT field */

int
disp_text (t, col, row, w, h, stp)
char **t;	/* text field */
int col;	/* col of text to start on */
int row;	/* row of text to start on */
int w;		/* width of text field */
int h;		/* height of text field */
struct state *stp;
{
	register char *s;
	int x = col,
	    y = row;
	Scrn_desc *sdp = sttosd (stp);
	int i;

	ENTERFUNC ("disp_text");
	DUMPARGS (" x=<%d> y=<%d> s='%s'", x, y, t[0]);
	DUMPARGS (" w=<%d> h=<%d>", w, h, NULL);
	if (x < 0 || x >= w || y < 0 || y >= h) {
		beep ();
		ERRFUNC ("disp_text", "bad size or dimension");
		return (1);
	}

	for (s = t[y+stp->topscroll] + x; y < h; s = t[(++y)+stp->topscroll]) {
		wmove (stp->window, y, x + sdp->col);
		UNDERLINE (stp->window, ON);
		waddstr (stp->window, s);
		for (i = strlen (t[y+stp->topscroll]); i < w; i++) {
			waddch (stp->window, ' ');
		}
		UNDERLINE (stp->window, OFF);
		x = 0;
	}
	EXITFUNC ("disp_text");
	return (0);
}


/* check for required items in a fillin screen */

int
reqfillin (stp)
struct	state	*stp;
{
	struct	scrn_desc	*sdp;
	int	i;

	sdp = stp->screenp->sd;

	for (i = stp->firstfield; i <= stp->lastfield; i++) {
		if (sdp[i].inout == FLD_OUTPUT ||
		    sdp[i].type == FLD_CHOICE)
			continue;
		if (sdp[i].required)
			if ((sdp[i].scrnstruct->filled == 0 &&
			    sdp[i].scrnstruct->changed == 0)  ||
			   (sdp[i].type == FLD_ALPHA &&
			    sdp[i].scrnstruct->pointer[0] == '\0'))
			{
				stp->curfield = i;
				message (stp, stp->window, stp->screenp,
					 REQUIREDERROR, NO);
				movetofield (stp);
				return (1);
			}
	}
	return (0);
}


/*
 * turn off INS indicator for insert mode
 */

void
off_insert_ind()
{
	register int i;

	move (INSROW, INSCOL);
	for (i = 0; i < strlen(INSERTIND); i++)
		addch (' ');
}


/*
 * turn on INS indicator for insert mode
 */

void
on_insert_ind()
{
	highlight (stdscr, INSROW, INSCOL, INSERTIND);
}

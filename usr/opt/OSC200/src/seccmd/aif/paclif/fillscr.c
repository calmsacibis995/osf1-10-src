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
static char *rcsid = "@(#)$RCSfile: fillscr.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:11:32 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	fillscr.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:03:47  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:58:43  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:29  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:01  marquard]
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
 *	@(#)fillscr.c	1.20 16:30:56 5/17/91 SecureWare
 */

/* #ident "@(#)fillscr.c	1.1 11:16:21 11/8/91 SecureWare" */

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
 *	choicefill()	- interpret choice field on fillin screen
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
 *	del_text_word()	- delete word in TEXT field
 *	del_text_field()- delete entire TEXT field
 *	disp_text()	- redisplay changed TEXT field after ins_ or del_
 *
 * Miscellaneous routines:
 *
 *	reqfillin()	- check for required items in a fillin screen
 *	off_insert_ind()- turn INSert indicator on screen off
 *	on_insert_ind()	- turn INSert indicator on screen on
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

static void check_val_act();
static int ins_text_char();
static void del_text_chars();
static void disp_text();
static void del_text_word();
static void del_text_field();

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
	/* iftthere is a move right one character call, use it here */
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
wrap:		for (i = stp->curfield; i <= stp->lastfield; i++, sdp++) {
			if (sdp->type == FLD_PROMPT)
				continue;
			if (sdp->inout == FLD_OUTPUT && sdp->type != FLD_SCROLL
			&& sdp->type != FLD_TEXT)
				continue;
			break;
		}

		/* wrap to beginning if necessary */

		if (i > stp->lastfield) {
			stp->curfield = stp->firstfield;
			sdp = sttosd(stp);
			goto wrap;
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
wrap:		for (i = stp->curfield; i >= stp->firstfield; i--, sdp--) {
			if (sdp->type == FLD_PROMPT)
				continue;
			if (sdp->inout == FLD_OUTPUT)
				continue;
			break;
		}

		/* Wrap to end if necessary */

		if (i < stp->firstfield) {
			stp->curfield = stp->lastfield;
			sdp = sttosd(stp);
			goto wrap;
		}

		stp->curfield = i;
	}
	movetofield (stp);
	sdp = sttosd (stp);
	return;
}

/*
 * Selected a choice on the fillin screen
 * Perform action and interpret result
 * Return the ABORT or QUIT indicator if necessary
 */

int
choicefill(stp)
register struct	state	*stp;
{
	struct	scrn_desc *sdp = sttosd(stp);
	register struct scrn_parms *screenp = stp->screenp;
	unsigned int choice;
	int dummy;

	/* check for a "validate on leave" field and validate if necessary */

	if (sdp->valid_on_leave) {

		/* if there is a validate function and it fails, stay here */

		if (sdp->scrnstruct &&
		   (*sdp->scrnstruct->validate)(screenp->fillin)) {
			stp->ret.flags = 0;
			return 0;
		}
	}

	choice = sdtocn(screenp, &sdp, stp->curfield);
	return trav_choice(screenp, choice, &dummy, sdp, 0);
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
	if (sdp->type == FLD_TEXT) {
		uchar row, col;

		del_text_word(stp, sdp);
		scr_rowcol(stp, sdp, &row, &col);
		wmove(stp->window, row, col + stp->columninfield);
		stp->itemchanged = 1;
		check_val_act(stp, sdp);
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
	check_val_act(stp, sdp);
	return;
}

/* "backspace" key
 * Delete character to the left of the cursor
 */

void
backspace(stp)
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

	/*
	 * Text field backspace logic is in del_text_chars()
	 */

	if (sdp->type == FLD_TEXT) {
		del_text_chars(stp, sdp, 1);
		scr_rowcol(stp, sdp, &row, &col);
		wmove(stp->window, row, col + stp->columninfield);
	} else {
		if (sdp->type == FLD_SCROLL)
			scr_rowcol(stp, sdp, &row, &col);
		else  {
			row = sdp->row;
			col = sdp->col;
		}
		wmove(stp->window, row, col + stp->columninfield);
		delchars(stp, sdp, 1);
	}

	/* reflect that item was changed */

	stp->itemchanged = 1;
	check_val_act(stp, sdp);

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
		check_val_act(stp, sdp);
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

	case FLD_TEXT:
		del_text_field(stp, sdp);
		wmove(stp->window, sdp->row, sdp->col);
		stp->itemchanged = 1;
		check_val_act(stp, sdp);
		break;

	case FLD_ALPHA:
	case FLD_NUMBER:
	case FLD_SCROLL:
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
		check_val_act(stp, sdp);
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
 * if it's OUTPUT only, beep
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
	switch (sdp->type) {
	case FLD_TOGGLE:
	case FLD_SCRTOG:
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
		break;

	case FLD_CHOICE:

		beep ();
		break;

	case FLD_TEXT:

		/* if in insert mode, shift remainder of field to right */

		if (stp->insert) {

			/* beep if on last position of last field */

			if (ins_text_char(stp, sdp))
				beep();
			else {
				disp_text(stp, sdp);
				scr_rowcol(stp, sdp, &row, &col);
				wmove(stp->window, row,
				  col + stp->columninfield);
			}
		}

		/*
		 * insert keystroke into current position,
		 * and mark field changed
		 */

		stp->scrnrep[stp->columninfield] = key;
		stp->itemchanged = 1;

		/*
		 * show this character on the screen
		 */

		scr_rowcol(stp, sdp, &row, &col);
		wmove(stp->window, row, col + stp->columninfield);
		UNDERLINE(stp->window, ON);
		WADDCHAR(stp->window, key);
		UNDERLINE(stp->window, OFF);

		/* move to next blank on screen if at end */

		if (stp->columninfield == sdp->len - 1) {
			tabfill(stp);
			scr_rowcol(stp, sdp, &row, &col);
			wmove(stp->window, row, col);
		} else
			stp->columninfield++;

		break;		

	default:
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

	} /* end switch */

	if (stp->itemchanged)
		check_val_act(stp, sdp);
	return;
}

/* check whether this screen has a "validate on action" function and call it */

static void
check_val_act(stp, sdp)
	struct state *stp;
	struct scrn_desc *sdp;
{
	/*
	 * if the field has a validate on action function, call it.
	 * leavefield is called to save the screen representation.
	 * reenterfield is called to restore highlights, etc.
	 */

	if (sdp->scrnstruct->val_act) {
		leavefield(stp);
		if ((*sdp->scrnstruct->val_act) ()) {
			redraw_screen(stp);
			
			/* kml's hack to make it all work:
			 *  
			 * At this point, we have to make sure the the
			 * screen representation of the scroll area is
			 * the same as the backing-store representation.
			 * This is ensured by resetting the screen rep.
			 * This should be done by redraw_screen,
			 * or perhaps reenterfield, but this seems
			 * easiest.
			 */

			if (sdp->type == FLD_SCROLL) {
				char **stringtab = 
					s_scrollreg (sdp->scrnstruct);

				strcpy (stp->scrnrep, 
					stringtab[stp->scrollitem]);
			} else if (sdp->type == FLD_ALPHA) {
				strcpy (stp->scrnrep, 
					s_alpha (sdp->scrnstruct));
			}
		}
		reenterfield(stp, sdp);
		wnoutrefresh(stdscr);
		wnoutrefresh(stp->window);
		doupdate();
	}
	return;
}

/*
 * Insert a text character into the scrolled text field at the current
 * position.  If we are at the rightmost column on the last line of the
 * text field, beep.  Otherwise, shift the entire field one position
 * to the right and put the character in the current position
 * Returns 1 if the cursor is on the last position for this field.
 */

static int
ins_text_char(stp, sdp)
	struct state *stp;
	struct scrn_desc *sdp;
{
	char wrap_char;
	int length;
	char **strings;
	int i, j;

	/* return if at the end of the last line of the text region */

	if (stp->columninfield == sdp->len - 1 &&
	    stp->scrollitem == sdp->scrnstruct->filled - 1)
		return 1;

	/*
	 * Move the current line one position to the right.
	 * Save the last character if we need to wrap
	 * Set 'i' to place in string to start
	 */

	length = strlen(stp->scrnrep);

	if (length == sdp->len) {
		wrap_char = stp->scrnrep[sdp->len - 1];
		i = sdp->len - 1;
	} else {
		wrap_char = '\0';
		i = length;
	}

	for ( ; i > stp->columninfield; i--)
		stp->scrnrep[i] = stp->scrnrep[i-1];

	/*
	 * save this row into the text field
	 */

	strings = s_text(sdp->scrnstruct);
	strcpy(strings[stp->scrollitem], stp->scrnrep);

	/*
	 * If we need to wrap, move the character that fell off the end
	 * into the remaining lines.
	 */

	for (i = stp->scrollitem + 1;
	     wrap_char && i < sdp->scrnstruct->filled;
	     i++) {
		char *this_string = strings[i];
		char last_char;

		/* decide whether this string is going to wrap */

		length = strlen(this_string);
		if (length == sdp->len) {
			last_char = this_string[sdp->len - 1];
			j = sdp->len - 1;
		} else {
			last_char = '\0';
			j = length;
		}

		/* move the string over to the right */

		for ( ; j > 0; j--)
			this_string[j] = this_string[j - 1];

		/* insert the character that wrapped from the previous line */

		this_string[0] = wrap_char;

		/* Use the last character from this line for the next line */

		wrap_char = last_char;
	}

	return 0;
}


/* delete entire text field */

static void
del_text_field(stp, sdp)
	struct state *stp;
	struct scrn_desc *sdp;
{
	char **strings = s_text(sdp->scrnstruct);
	int i;
	int len;

	/* clear out the backing storage area */

	for (i = 0; i < sdp->scrnstruct->filled; i++) {

		len = strlen(strings[i]);
		if (len > 0)
			memset(strings[i], '\0', len);
	}

	/* clear out the screen representation */

	len = strlen(stp->scrnrep);
	if (len > 0)
		memset(stp->scrnrep, '\0', len);

	/* back up to the beginning of the scrolling region */

	stp->topscroll = 0;
	stp->columninfield = 0;

	/* redisplay text region */

	disp_text(stp, sdp);
	
}

/* delete a word in a text field.  The word may continue to the next line */

static void
del_text_word(stp, sdp)
	struct state *stp;
	struct scrn_desc *sdp;
{
	char *string = &stp->scrnrep[stp->columninfield];
	int len = strlen(stp->scrnrep);
	char *first_space;
	char **strings = s_text(sdp->scrnstruct);
	int spill_line;
	int chars_to_delete;
	int i;
	int space_item;

	/* delete on this line only or spill to the next? */

	first_space = strchr(string, ' ');

	/*
	 * there are more lines if the current line is full,
	 * there are more lines filled, and the next line has
	 * something in it.
	 */

	spill_line = (len == sdp->len &&
		      stp->scrollitem < sdp->scrnstruct->filled &&
		      strings[stp->scrollitem + 1][0] != '\0');
		
	/*
	 * determine how long it is until a word break
	 */

	if (first_space != NULL) {

		/* next space on this line */

		chars_to_delete = (unsigned long) first_space - 
				  (unsigned long) string;
		space_item = stp->scrollitem;

	} else if (spill_line) {

		chars_to_delete = sdp->len - stp->columninfield;

		/* find next space on succeeding lines */

		for (i = stp->scrollitem + 1;
		     i < sdp->scrnstruct->filled;
		     i++) {

			/* if this string's empty, we're done */

			if (strings[i][0] == '\0')
				break;

			first_space = strchr(strings[i], ' ');

			/*
			 * if no spaces, delete entire line.
			 * Otherwise, delete all including space
			 */

			if (first_space == NULL)
				chars_to_delete += strlen(strings[i]);
			else {
				chars_to_delete +=
				  (unsigned long) first_space -
				  (unsigned long) strings[i];
				space_item = i;
				break;
			}
		}

	} else {

		/* delete to end of line */

		chars_to_delete = len - stp->columninfield;
	}

	/* now scan past the spaces (including wrapped spaces) */

	if (first_space) {
		do {
			chars_to_delete++;
			first_space++;
			if (*first_space == '\0') {
				space_item++;
				if (space_item == sdp->scrnstruct->filled)
					break;
				first_space = strings[space_item];
			}
		} while (*first_space == ' ');
	}

	del_text_chars(stp, sdp, chars_to_delete);
	return;
}

/* delete the specified number of characters from the text region */

static void
del_text_chars(stp, sdp, chars_to_delete)
	struct state *stp;
	struct scrn_desc *sdp;
	int chars_to_delete;
{
	char *from_cp, *to_cp;
	int from_column, to_column;
	int from_index, to_index;
	int i;
	char **strings = s_text(sdp->scrnstruct);
	int chars_to_move;
	int chars_left;

	ENTERFUNC("del_text_chars");

	/*
	 * Move the requisite amount of characters forward.
	 * Save scrn_rep in the string to work entirely within the string
	 */

	DUMPARGS(
	  " stp->scrollitem=<%d> stp->scrnrep=<%s>, chars_to_delete=<%d>",
	  stp->scrollitem, stp->scrnrep, chars_to_delete);

	strcpy(strings[stp->scrollitem], stp->scrnrep);

	/*
	 * find the place in the text region where characters are to be
 	 * moved from (chars_to_delete from the current location)
	 */

	to_cp = &strings[stp->scrollitem][stp->columninfield];
	to_column = stp->columninfield;
	to_index = stp->scrollitem;

	/* determine the beginning of the data that needs moving up */

	from_cp = to_cp + chars_to_delete;
	from_column = stp->columninfield + chars_to_delete;
	from_index = to_index;

	chars_left = chars_to_delete - (sdp->len - stp->columninfield);

	if (chars_left > 0) {

		/* compute the number of full lines to skip */

		from_index = stp->scrollitem + (chars_left / sdp->len) + 1;
		from_cp = strings[from_index];
		
		/* compute the offset in that line */

		from_column = chars_left % sdp->len;
		from_cp += from_column;
	}

	/* determine how many characters need moving */

	chars_to_move = strlen(from_cp);

	DUMPVARS(
	  "Before for loop, from_index=<%d> filled=<%d> chars_to_move <%d>",
	  from_index, sdp->scrnstruct->filled, chars_to_move);

	for (i = from_index + 1; i < sdp->scrnstruct->filled; i++) {
		int len = strlen(strings[i]);

		if (len == 0)
			break;
		chars_to_move += len;
	}

	DUMPVARS("chars_to_move=<%d> from_cp=<%s> to_cp=<%s>",
	  chars_to_move, from_cp, to_cp);

	/* move characters forward */

	for (i = 0; i < chars_to_move; i++) {
		*to_cp++ = *from_cp;

		/*
		 * move from_cp forward if on end of current line and there
		 * is are more lines. o.w., leave on the last NULL
		 */

		if (from_index < sdp->scrnstruct->filled) {
			from_column++;
			if (from_column == sdp->len) {
				from_index++;
				from_cp = strings[from_index];
				from_column = 0;
			} else
				from_cp++;
		}

		/* move to_cp forward (it cannot overflow) */

		if (++to_column == sdp->len) {
			to_cp = strings[++to_index];
			to_column = 0;
		}
	}

	/* truncate off the end of the line */

	for (i = 0; i < chars_to_delete; i++) {
		*to_cp++ = '\0';
		if (++to_column == sdp->len) {
			to_cp = strings[++to_index];
			to_column = 0;
		}
	}

	/* copy the current line into scrnrep, and fill with nulls at end */

	strcpy(stp->scrnrep, strings[stp->scrollitem]);
	chars_to_move = strlen(stp->scrnrep);
	memset(&stp->scrnrep[chars_to_move], '\0',
			sizeof(stp->scrnrep) - chars_to_move);

	/* display the text region */

	disp_text(stp, sdp);
	return;
}

/* display modified portion of text field, from current position to end */

static void
disp_text(stp, sdp)
	struct state *stp;
	struct scrn_desc *sdp;
{
	char **strings = s_text(sdp->scrnstruct);
	int i, j;
	int first_item = stp->scrollitem - stp->topscroll;
	char *string;
	int scrollitem;

	ENTERFUNC("disp_text");
	DUMPARGS(" row=<%d> col=<%d> scrollitem=<%d>",
	  sdp->row, sdp->col, stp->scrollitem);
	DUMPARGS(" columninfield=<%d> string=<%s>",
	  stp->columninfield, &strings[stp->scrollitem][stp->columninfield],
	  NULL);

	/* Move to the current location in the region (changed from here) */

	wmove(stp->window, sdp->row + first_item,
			   sdp->col + stp->columninfield);
	scrollitem = stp->scrollitem;
	string = &strings[stp->scrollitem][stp->columninfield];

	for (i = first_item; i < sdp->s_lines; i++) {

		/* underline the contents of the current line, or blanks */

		UNDERLINE(stp->window, ON);

		waddstr(stp->window, string);
		for (j = strlen(strings[scrollitem]); j < sdp->len; j++)
				waddch(stp->window, ' ');

		UNDERLINE(stp->window, OFF);

		/* move to the beginning of the next line */

		wmove(stp->window, sdp->row + i + 1, sdp->col);

		/* determine the next string -- fake a NULL if off the end */

		if (++scrollitem > sdp->scrnstruct->filled)
			string = "";
		else
			string = strings[scrollitem];
	}
		
	EXITFUNC("disp_text");
	return;
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

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
static char	*sccsid = "@(#)$RCSfile: putscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:29 $";
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



/* putscr.c - put a screen on the terminal
 */

#include	<ctype.h>
#include	<stdio.h>
#include	"userif.h"
#include	"curs_supp.h"
#include	"key_map.h"
#include	"kitch_sink.h"
#include	"logging.h"

void draw_prompt(), draw_toggle(), draw_alpha(), draw_number(),
     draw_yn(), draw_scrollitems(), draw_scrtogl1items();

#define HEREAMI(Q) DUMPDECP(Q, NULL, NULL, NULL);

void
putscreen (screenp, structp, clear)
struct	scrn_parms	*screenp;	/* screen description */
struct	scrn_struct	*structp;	/* data values for screen */
uchar	clear;				/* clear screen? */
{
	register WINDOW	*window;
	register struct	scrn_desc	*sdp;
	register struct	scrn_struct	*sp;
	register unsigned int	i;

	static	int	first_time = 1;	/* for copyright message */

	ENTERFUNC("putscreen");
	sp = structp;
	if (clear)
		wclear (screenp->w);
	for (sdp = screenp->sd, i = 0; i < screenp->ndescs; i++, sdp++)  {
		/* find structure description for this field, if not
		   a CHOICE, PROMPT, BLANK, or SKIP field */
		if (sdp->type != FLD_CHOICE && sdp->type != FLD_PROMPT &&
				sdp->type != FLD_BLANK && sdp->type != FLD_SKIP)
			while (sp->desc != i)
				sp++;
		DUMPVARS("fld(i)=<%d> type=<%d>", i, sdp->type, NULL);
		switch (sdp->type)  {
		case	FLD_PROMPT:
		case	FLD_CHOICE:
			HEREAMI(" FLD_PROMPT or FLD_CHOICE");
			DUMPDETI (" prompt=<%s>",sdp->prompt,0,0);
			draw_prompt(screenp->w, sdp, sp);
			break;

		case	FLD_TOGGLE:
			HEREAMI(" FLD_TOGGLE");
			draw_toggle(screenp->w, sdp, sp);
			break;

		case	FLD_ALPHA:
			HEREAMI(" FLD_ALPHA");
			DUMPDETI (" alpha=<%s>",s_alpha(sp),0,0);
			draw_alpha(screenp->w, sdp, sp);
			break;

		case	FLD_NUMBER:
			HEREAMI(" FLD_NUMBER");
			DUMPDETI (" number=<%d>",*s_number(sp),0,0);
			draw_number(screenp->w, sdp, sp);
			break;

		case	FLD_YN:
		case	FLD_CONFIRM:
		case	FLD_POPUP:
			HEREAMI(" FLD_YN or FLD_CONFIRM or FLD_POPUP");
			draw_yn(screenp->w, sdp, sp);
			break;

		case	FLD_SCRTOG:
			if (sdp->len == 1) {
				HEREAMI("FLD_SCRTOG length 1");
				draw_scrtogl1items(screenp->w, sdp, sp, 0);
				break;
			}

			/* fall into . . . */

		case	FLD_SCROLL:
		case	FLD_TEXT:
			HEREAMI(" FLD_SCROLL or FLD_SCRTOG or SCR_TEXT");
			draw_scrollitems(screenp->w, sdp, sp, 0);
			break;

		case	FLD_SKIP:
		case	FLD_BLANK:
			HEREAMI(" FLD_SKIP or FLD_BLANK");
			break;
		}

		/* compute first desc to move to if not already computed */
		if (screenp->first_desc == (uchar) -1)
		switch (sdp->type) {
		case FLD_CHOICE:
		case FLD_TOGGLE:
		case FLD_ALPHA:
		case FLD_NUMBER:
		case FLD_YN:
		case FLD_CONFIRM:
		case FLD_POPUP:
		case FLD_SCROLL:
		case FLD_SCRTOG:
		case FLD_TEXT:
			screenp->first_desc = i;
		default:
			break;
		}
	}

	if (screenp->scrntype == SCR_TEXT) {
		for (i = 0; i < screenp->nbrrows; i++) {
			WCENTERS (screenp->w, i, screenp->text[i]);
		}
		parkcursor ();
	} else  {
		touchwin(stdscr);
		/* set and clear copyright string */
		switch (first_time) {
		default:
			break;
		case 1:
			mvaddstr (LINES-1, COLS-strlen(screen_copyr) - 3,
			 screen_copyr);
			first_time = 2;
			break;
		case 2:
			move (LINES - 1, COLS - strlen(screen_copyr) - 3);
			for (i = 0; i < strlen(screen_copyr); i++)
				addch (' ');
			first_time = 0;
			break;
		}
		wnoutrefresh (stdscr);
		wnoutrefresh (screenp->w);
		doupdate();
	}
	EXITFUNC("putscreen");
}

/*
 * Redraw the screen parts that have changed
 * Return 1 if anything changed
 */

int
redraw_screen(stp)
struct state *stp;
{
	register struct scrn_desc *sdp = stp->screenp->sd;
	int i;
	int changed = 0;

	/* for each field that's changed, redraw the field */

	for (i = 0; i < stp->screenp->ndescs; i++, sdp++) {
		register struct scrn_struct *sp = sdp->scrnstruct;

		if (sp && sp->changed) {
			WINDOW *w = stp->window;

			changed = 1;
			switch (sdp->type) {
			case FLD_SCRTOG:
				if (sdp->len == 1) {
					draw_scrtogl1items(w, sdp, sp, 
					  sdp->s_topleft);
					break;
				}

				/* fall into . . . */

			case FLD_SCROLL:
			case FLD_TEXT:
				draw_scrollitems(w, sdp, sp, sdp->s_topleft);
				break;

			case FLD_ALPHA:
				draw_alpha(w, sdp, sp);
				break;

			case FLD_NUMBER:
				draw_number(w, sdp, sp);
				break;

			case FLD_YN:
				draw_yn(w, sdp, sp);
				break;

			case FLD_TOGGLE:
				draw_toggle(w, sdp, sp);
				break;
			}
		}
	}
	return changed;
}


/*
 * Draw a toggle field
 */

void
draw_toggle(w, sdp, sp)
WINDOW *w;
struct scrn_desc *sdp;
struct scrn_struct *sp;
{
	if (sdp->len == 1) {
		DUMPDETI (" toggle=<%d>",*s_toggle(sp),0,0);
		if (*s_toggle(sp))
			mvwaddch (w, sdp->row, sdp->col, TOGGLEON);
		else
			mvwaddch (w, sdp->row, sdp->col, TOGGLEOFF);
	} else {
		DUMPDETI (" toggle=<%s>",sdp->prompt,0,0);
		if (*s_toggle(sp))
			settoggle(w, sdp->row, sdp->col,
					sdp->prompt, sdp->len);
		else
			unsettoggle(w, sdp->row, sdp->col,
					sdp->prompt, sdp->len);
	}
	return;
}

/*
 * Draw any of the scrolling region fields except scrolling toggle
 * of length 1.
 * first_item is the first item to display in the field.
 */

void
draw_scrollitems(w, sdp, sp, first_item)
WINDOW *w;
struct scrn_desc *sdp;
struct scrn_struct *sp;
int first_item;
{
	int	temprow, tempcol;
	int	i, j;
	int	which_item, in_region;
	char	**strings;

	temprow = sdp->row;
	strings = &(s_scrollreg(sp)[first_item]);
	for (i = 0; i < sdp->s_lines; i++)  {
		tempcol = sdp->col;
		for (j = 0; j < sdp->s_itemsperline; j++) {
		    which_item = (i * sdp->s_itemsperline) + j + first_item;
		    in_region = which_item < sp->filled;

		    switch (sdp->type) {
		    case FLD_SCROLL :
		    case FLD_TEXT :
			if (sdp->inout != FLD_OUTPUT) {
			    if (!has_underline) {
				wmove (w, temprow, tempcol-1);
				if (sdp->s_spaces == 1)
					WADDCHAR (w, SINGLE_BOUNDARY);
				else
					WADDCHAR (w, LEFT_BOUNDARY);
			    } else {
				wmove (w, temprow, tempcol);
				UNDERLINE (w, ON);
			    }
			} else {
				wmove (w, temprow, tempcol);
			}
			if (in_region && *strings)  {
				wpadstring (w, *strings, sdp->len);
				strings++;
			}
			else
				putspaces (w, sdp->len);
			if (sdp->inout != FLD_OUTPUT) {
			    if (!has_underline) {
				if (sdp->s_spaces == 1)
					WADDCHAR (w, SINGLE_BOUNDARY);
				else
					WADDCHAR (w, RIGHT_BOUNDARY);
			    } else {
				UNDERLINE (w, OFF);
			    }
			}
		    break;

		    case FLD_SCRTOG :
		  	if (in_region && *strings) {
			       	if (sp->state[which_item])
					settoggle(w, temprow, tempcol,
					  *strings, sdp->len);
				else
					unsettoggle(w, temprow, tempcol,
				  	  *strings, sdp->len);
				strings++;
			} else {
				wmove(w, temprow, tempcol);
				putspaces(w, sdp->len);
			}
		    break;
		    }
		    tempcol += sdp->len + sdp->s_spaces;
		}
		temprow++;
	}
	/*
	 * Adjust the above and below indicators for the scrolling region.
	 */

	if (sdp->s_lines * sdp->s_itemsperline + first_item < sp->filled &&
	     strings[0][0] != '\0')
		scr_below_ind(w, sdp, ON);
	else
		scr_below_ind(w, sdp, OFF);
	if (first_item)
		scr_above_ind(w, sdp, ON);
	else
		scr_above_ind(w, sdp, OFF);
}


/*
 * Draw one of the scrolling toggle of length 1 regions.
 * first_item is the first item to display in the field.
 */

void
draw_scrtogl1items(w, sdp, sp, first_item)
WINDOW *w;
struct scrn_desc *sdp;
struct scrn_struct *sp;
int first_item;
{
	int	temprow, tempcol;
	int	i, j;
	int	which_item, in_region;

	temprow = sdp->row;
	for (i = 0; i < sdp->s_lines; i++)  {
		tempcol = sdp->col;
		for (j = 0; j < sdp->s_itemsperline; j++) {
		    which_item = (i * sdp->s_itemsperline) + j + first_item;
		    in_region = which_item < sp->filled;
		    if (in_region) {
			if (sp->state[which_item])
				mvwaddch(w, temprow, tempcol, TOGGLEON);
			else
				mvwaddch(w, temprow, tempcol, TOGGLEOFF);
		    }
		    tempcol += sdp->len + sdp->s_spaces;
		}
		temprow++;
	}
	/*
	 * Adjust the above and below indicators for the scrolling region.
	 */

	if (sdp->s_lines * sdp->s_itemsperline + first_item < sp->filled)
		scr_below_ind(w, sdp, ON);
	else
		scr_below_ind(w, sdp, OFF);
	if (first_item)
		scr_above_ind(w, sdp, ON);
	else
		scr_above_ind(w, sdp, OFF);
}

/*
 * Draw an alphanumeric field
 */

void
draw_alpha(w, sdp, sp)
WINDOW *w;
struct scrn_desc *sdp;
struct scrn_struct *sp;
{
	if (!has_underline) {
		wmove (w, sdp->row, sdp->col - 1);
		WADDCHAR (w, LEFT_BOUNDARY);
	} else {
		wmove (w, sdp->row, sdp->col);
		UNDERLINE (w, ON);
	}
	if (sdp->inout == FLD_OUTPUT ||
	  (sdp->inout == FLD_BOTH && (sp->filled||sp->changed)))
		wpadstring (w, s_alpha(sp), sdp->len);
	else
		putspaces (w, sdp->len);
	if (!has_underline)
		WADDCHAR (w, RIGHT_BOUNDARY);
	else
		UNDERLINE (w, OFF);
}

/*
 * Draw a number field
 */

void draw_number(w, sdp, sp)
WINDOW *w;
struct scrn_desc *sdp;
struct scrn_struct *sp;
{
	if (!has_underline) {
		wmove (w, sdp->row, sdp->col - 1);
		WADDCHAR (w, LEFT_BOUNDARY);
	} else {
		wmove (w, sdp->row, sdp->col);
		UNDERLINE (w, ON);
	}
	if (sdp->inout == FLD_OUTPUT ||
	  (sdp->inout == FLD_BOTH && (sp->filled||sp->changed)))
		padnumber (w, *s_number(sp), sdp->len);
	else	putspaces (w, sdp->len);
	if (!has_underline)
		WADDCHAR (w, RIGHT_BOUNDARY);
	else
		UNDERLINE (w, OFF);
}

/*
 * Draw a yes/no field
 */

void
draw_yn(w, sdp, sp)
WINDOW *w;
struct scrn_desc *sdp;
struct scrn_struct *sp;
{
	if (!has_underline) {
		wmove (w, sdp->row, sdp->col - 1);
		WADDCHAR (w, LEFT_BOUNDARY);
	} else {
		wmove (w, sdp->row, sdp->col);
		UNDERLINE (w, ON);
	}
	DUMPDETI (" value=<%c>",*s_yesno(sp),0,0);
	if (sdp->inout == FLD_OUTPUT ||
	    (sdp->inout == FLD_BOTH && (sp->filled || sp->changed)))
		if (*s_yesno(sp))
			WADDCHAR (w, YESCHAR);
		else
			WADDCHAR (w, NOCHAR);
	else
		putspaces (w, 1);
	if (!has_underline)
		WADDCHAR (w, RIGHT_BOUNDARY);
	else
		UNDERLINE (w, OFF);
}

/*
 * Draw a prompt field
 */

void
draw_prompt(w, sdp, sp)
WINDOW *w;
struct scrn_desc *sdp;
struct scrn_struct *sp;
{
	mvwaddstr(w, sdp->row, sdp->col, sdp->prompt);
}

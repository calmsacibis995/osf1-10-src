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
static char	*sccsid = "@(#)$RCSfile: scrscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:39 $";
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



/*
 * This file contains SCROLL region-specific routines
 */

#include	<stdio.h>
#include	<sys/security.h>
#include	<sys/audit.h>
#include	"userif.h"
#include	"curs_supp.h"
#include	"key_map.h"
#include	"kitch_sink.h"

#ifdef DEBUG
extern	FILE	*logfp;
#endif DEBUG

/* test for scrolling toggle field of 1 character length */

#define SCRTOGL1(D) ((D)->type == FLD_SCRTOG && (D)->len == 1)


/* scrolling region routines */

/* move into a scrolling region -- to top left blank on screen */

void
movetoscrollreg (stp, sdp)
struct	state		*stp;
struct	scrn_desc	*sdp;
{
	/* saved copy of topleft blank is in sdp */
	stp->topscroll = sdp->s_topleft;
	stp->scrollitem = stp->topscroll;

	/* move to this field in the scrolling region */

	scr_moveto (stp, sdp);

	return;
}

/* tab key in a scrolling region
 * If the current field doesn't have anything in it but the next
 * one does, move other fields over to fill this field and stay
 * in the same field.
 * If the current field and next field are empty, don't allow.
 * Otherwise, move to next sequential field.
 * If on last item of last line, scroll the screen up.
 */

void
tabscrollreg (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{
	uchar	row, col;
	char	**stringtab;
	int	scrtogl1 = SCRTOGL1 (sdp);

	/* save off screen representation into string table */
	if (leavefield (stp))
		return;
	if (!scrtogl1)
		stringtab = s_scrollreg (sdp->scrnstruct);
	/* check for last item in table, or for blank item, and move to
	 * next field on screen.
	 */
	if (stp->scrollitem == sdp->scrnstruct->filled - 1 ||
	    (!scrtogl1 && stringtab[stp->scrollitem][0] == '\0'))  {
		sdp->s_topleft = stp->topscroll;
		if (stp->curfield == stp->lastfield)
			stp->curfield = stp->firstfield;
		else for (++stp->curfield, ++sdp; ; ++stp->curfield, ++sdp)
			if (sdp->inout != FLD_OUTPUT)
				break;
		movetofield (stp);
		return;
	}
	/* ok to move, check if next one is off screen */
	stp->scrollitem++;
	if (scr_offscreen (stp, sdp))  /* below screen */
		scroll_up (stp, sdp);
	scr_moveto (stp, sdp);
	return;
}

/* backtab key in a scrolling region.
 * if on first item, move out of scrolling region.
 * if this is the first item on the screen, scroll the screen down.
 */

void
backtabscrollreg (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{

	/* save off screen representation into string table */
	if (leavefield (stp))
		return;

	/* if on first item, move to previous field */
	if (stp->scrollitem == 0)  {
		/* save top item on screen */
		sdp->s_topleft = 0;
		if (stp->curfield == stp->firstfield)
			stp->curfield = stp->lastfield;
		else
			for (sdp--, stp->curfield--;
			     sdp->inout == FLD_OUTPUT;
			     sdp--)
				stp->curfield--;
#ifdef DEBUG
fprintf (logfp, "backtabscrollreg: new item %d firstfield %d lastfield %d.\n",
 stp->curfield, stp->firstfield, stp->lastfield);
#endif
		movetofield (stp);
		return;
	}
	/* Check for top of scrolling region */
	if (stp->scrollitem == stp->topscroll)
		scroll_down (stp, sdp);
	stp->scrollitem--;
	scr_moveto (stp, sdp);
	return;
}

/* scroll forward in a scrolling region */
void
scrolldownkey (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register char			**stringtab;
	register int			new_top;
	int				scrtogl1;

	sdp = sttosd(stp);
	scrtogl1 = SCRTOGL1(sdp);
	if (sdp->type != FLD_SCROLL && sdp->type != FLD_SCRTOG &&
	    sdp->type != FLD_TEXT) {
		beep();
		return;
	}
	/* check for moving outside range of string table */
	if (!scrtogl1)
		stringtab = s_scrollreg (sdp->scrnstruct);
	new_top = stp->topscroll + sdp->s_itemsperline * sdp->s_lines;
	if (new_top > sdp->scrnstruct->filled - 1 ||
		(!scrtogl1 && stringtab[new_top][0] == '\0')) {
		beep();
		return;
	}
	if (leavefield (stp))
		return;
	stp->topscroll = stp->scrollitem = new_top;
	drawscrollreg (stp, sdp);
	scr_moveto (stp, sdp);
	return;
}

/* scroll backward in a scrolling region */
void
scrollupkey (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd(stp);
	if (sdp->type != FLD_SCROLL && sdp->type != FLD_SCRTOG &&
	    sdp->type != FLD_TEXT) {
		beep();
		return;
	}
	if (stp->topscroll == 0) {
		beep();
		return;
	}
	if (leavefield (stp))
		return;
	/* check for moving outside range of string table */
	if (stp->topscroll < sdp->s_itemsperline * sdp->s_lines)
		stp->topscroll = 0;
	else	stp->topscroll -= sdp->s_itemsperline * sdp->s_lines;
	stp->scrollitem = stp->topscroll;
	drawscrollreg (stp, sdp);
	scr_moveto (stp, sdp);
	return;
}

/* down key in a scrolling region
 * check that the current field and all intervening fields aren't blank.
 * if so, move out of the scrolling region to the same column on the next line.
 * Otherwise, move down sdp->s_itemsperline fields
 */

void
downscrollreg (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{
	char		**stringtab;
	register int	i;
	int		scrtogl1 = SCRTOGL1 (sdp);

	if (stp->scrollitem == sdp->scrnstruct->filled - 1) {
		beep ();
		return;
	}
	if (leavefield (stp))
		return;

	/* check for moving outside range of string table */
	if (stp->scrollitem + sdp->s_itemsperline >
	     sdp->scrnstruct->filled - 1) {
		sdp->s_topleft = stp->topscroll;
		if (stp->curfield == stp->lastfield) {
			/* other choice would be to wrap */
			beep();
			stp->columninfield = 0;
			scr_moveto (stp, sdp);
			return;
		}
		/* find the next non-output field */
		if ((i = finddownitem (stp, sdp)) != -1) {
			stp->curfield = i;
			movetofield (stp);
		} else	beep();
		return;
	}
	/* check for intervening blank fields between this one and the end */
	if (!scrtogl1) {
		stringtab = s_scrollreg (sdp->scrnstruct);
		/* check for intervening blank fields */
		for (i = 0; i < sdp->s_itemsperline; i++)
			if (stringtab[stp->scrollitem + i][0] == '\0') {
				message (stp, stp->window, stp->screenp,
				  BLANKSCROLL, NO);
				stp->columninfield = 0;
				scr_moveto (stp, sdp);
				return;
			}
	}

	/* move to the field */
	stp->scrollitem += sdp->s_itemsperline;
	if (scr_offscreen (stp, sdp) == 1)
		scroll_up (stp, sdp);
	scr_moveto (stp, sdp);
	return;
}

/*  up key in a scrolling region */

void
upscrollreg (stp, sdp)
register struct	state	*stp;
register struct	scrn_desc	*sdp;
{
	int	i;

	if (stp->scrollitem == 0) {
		beep ();
		return;
	}
	if (leavefield (stp))
		return;

	/* check if on top line of scrolling region and on top row */
	if (stp->scrollitem < sdp->s_itemsperline) {
		sdp->s_topleft = stp->topscroll;
		if ((i = findupitem (stp, sdp)) != -1) {
			stp->curfield = i;
			movetofield (stp);
		}
		else	beep();
		return;
	}
	/* Check if on top row */
	if (stp->scrollitem - stp->topscroll < sdp->s_itemsperline) {
		scroll_down (stp, sdp);
	}
	stp->scrollitem -= sdp->s_itemsperline;
	scr_moveto (stp, sdp);
	return;
}

/* checks if a particular scroll item is off screen
 * returns:	-1 if off screen above
 *		 0 if on screen
 *		 1 if off screen below
 */

int
scr_offscreen (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	if (stp->scrollitem < stp->topscroll)
		return (-1);
	if (stp->scrollitem >=
	      stp->topscroll + (sdp->s_itemsperline * sdp->s_lines))
		return (1);
	return (0);
}

/* scroll the scrolling region down one line
 * adjusts stp->topscroll
 */

void
scroll_down (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	stp->topscroll -= sdp->s_itemsperline;
	drawscrollreg (stp, sdp);
	return;
}

/* scroll the region up one line
 * adjusts stp->topscroll
 */

void
scroll_up (stp, sdp)
struct	state		*stp;
struct	scrn_desc	*sdp;
{
	stp->topscroll += sdp->s_itemsperline;
	drawscrollreg (stp, sdp);
	return;
}

/* redraws the scrolling region, given stp->topscroll
 * deals with above and below arrow
 */

void
drawscrollreg (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	char	**stringtab;
	int	temprow, tempcol;
	register int	i, j;
	uchar	item;

	/*  keep sdp's indication of top of screen current */
	sdp->s_topleft = stp->topscroll;
	draw_scrollitems(stp->window, sdp, sdp->scrnstruct, stp->topscroll);
	return;
}

/* turn on or off the scroll above indicator
 */

void
scr_above_ind (w, sdp, onoff)
WINDOW *w;
struct scrn_desc *sdp;
uchar	onoff;
{
	uchar	col;
	int	i;

	col = sdp->col + (sdp->s_itemsperline * sdp->len) +
		((sdp->s_itemsperline - 1) * sdp->s_spaces) + 1;
	wmove (w, sdp->row, col);
	if (onoff == ON)
		waddstr (w, ABOVEIND);
	else {
		for (i = 0; i < strlen (ABOVEIND); i++)
			waddch (w, ' ');
	}
	return;
}

/* turn on or off the scroll below indicator
 */

void
scr_below_ind (w, sdp, onoff)
WINDOW *w;
struct scrn_desc *sdp;
uchar	onoff;
{
	uchar	row, col;
	int	i;

	col = sdp->col + (sdp->s_itemsperline * sdp->len) +
		((sdp->s_itemsperline - 1) * sdp->s_spaces) + 1;
	row = sdp->row + sdp->s_lines - 1;
	wmove (w, row, col);
	if (onoff == ON)
		waddstr (w, BELOWIND);
	else
		for (i = 0; i < strlen (BELOWIND); i++)
			waddch (w, ' ');
	return;
}

/* scr_rowcol reports the row and column associated with the
 * item found in stp->scrollitem
 */

void
scr_rowcol (stp, sdp, row, col)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
uchar	*row, *col;
{
	uchar	offset;

	offset = stp->scrollitem - stp->topscroll;
	*row = sdp->row + (offset / sdp->s_itemsperline);
	*col = sdp->col + (offset % sdp->s_itemsperline) *
				(sdp->len + sdp->s_spaces);
	return;
}

/* scr_moveto moves to a particular scrolling region item
 */

void
scr_moveto (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	char	**stringtab;
	uchar	row, col;
	register int	i;

	if (sdp->type == FLD_SCRTOG)
		unhighlighttoggle (stp, sdp);
	scr_rowcol (stp, sdp, &row, &col);
	wmove (stp->window, row, col);

	/* set up screen representation of item */
	if (sdp->type == FLD_SCRTOG) {
		sdp->index = stp->scrollitem;
		wmove (stp->window, sdp->row, sdp->col);
		stp->columninfield = 0;
		stp->itemchanged = 0;
		if (sdp->len == 1) {
			stp->scrnrep[0] = sdp->scrnstruct->state[sdp->index] ?
				TOGGLEON : TOGGLEOFF;
			stp->scrnrep[1] = '\0';
		} else {
			stringtab = s_scrtog(sdp->scrnstruct);
			strcpy(stp->scrnrep, stringtab[stp->scrollitem]);
		}
		highlighttoggle (stp, sdp);
	} else {	/* regular scrolling field */
		stringtab = s_scrollreg (sdp->scrnstruct);
		strcpy (stp->scrnrep, stringtab[stp->scrollitem]);
		for (i = strlen (stp->scrnrep); i < sizeof (stp->scrnrep); i++)
			stp->scrnrep[i] = '\0';
		stp->columninfield = 0;
	}
	return;
}

/* scrolling region right shift.
 * shift all items right one slot starting at the current item
 * in response to the INSFIELD character
 */

void
scroll_rshift (stp, sdp)
register struct state		*stp;
register struct scrn_desc	*sdp;
{
	register int	i, item;
	char		**stringtab;
	int		scrtogl1 = SCRTOGL1 (sdp);

	if (scrtogl1)
		return;		/* no point in this */

	stringtab = s_scrollreg (sdp->scrnstruct);
	/* make sure there is room */
	if (stringtab[sdp->scrnstruct->filled - 1][0]) {
		message (stp, stp->window, stp->screenp,
			FULLSCROLL, NO);
		stp->columninfield = 0;
		scr_moveto (stp, sdp);
		return;
	}
	item = stp->scrollitem;
	/* prevent insertion if field is empty */
	if (stringtab[item][0] == '\0') {
		message (stp, stp->window, stp->screenp,
			BLANKSCROLL, NO);
		stp->columninfield = 0;
		scr_moveto (stp, sdp);
		return;
	}
	for (i = sdp->scrnstruct->filled - 1; i >= item; --i) {
		if (stringtab[i][0] == '\0')
			continue;
		memcpy(stringtab[i+1], stringtab[i], sdp->len);
		stp->scrollitem = i + 1;
		if (!scr_offscreen (stp, sdp)) {
			scr_moveto (stp, sdp);
			UNDERLINE (stp->window, ON);
			wpadstring (stp->window, stringtab[i], sdp->len);
			UNDERLINE (stp->window, OFF);
		}
	}
	stp->scrollitem = item;
	memset (stringtab[item], 0, sdp->len);
	scr_moveto (stp, sdp);
	UNDERLINE (stp->window, ON);
	putspaces (stp->window, sdp->len);
	UNDERLINE (stp->window, OFF);
	/* adjust below indicator if necessary */
	item = stp->topscroll + sdp->s_lines * sdp->s_itemsperline;
	if (item < sdp->scrnstruct->filled && stringtab[item][0])
		scr_below_ind (stp, ON);
	else	scr_below_ind (stp, OFF);
	scr_moveto (stp, sdp);
	return;
}

/* scrolling region left shift.
 * typically done when the user clears a blank, causing the next
 * items in the region to be moved back
 */

void
scroll_lshift (stp, sdp)
register struct	state		*stp;
register struct	scrn_desc	*sdp;
{
	ushort		item;
	register int	i, j;
	char		**stringtab;
	int		scrtogl1 = SCRTOGL1 (sdp);

	if (scrtogl1)
		return;		/* no point in this */

	stringtab = s_scrollreg (sdp->scrnstruct);
	/* save off item while working on screen */
	item = stp->scrollitem;
	for (i = stp->scrollitem; ; i++, stp->scrollitem++) {
		/* last item is always null field */
		if (i == sdp->scrnstruct->filled - 1)
			stringtab[i][0] = '\0';
		else	strcpy (stringtab[i], stringtab[i+1]);
		/* if on screen, update that blank */
		if (!scr_offscreen (stp, sdp)) {
			scr_moveto (stp, sdp);
			UNDERLINE (stp->window, ON);
			wpadstring (stp->window, stringtab[i], sdp->len);
			UNDERLINE (stp->window, OFF);
		}
		/* pad rest of string to spaces */
		for (j = strlen(stringtab[i]); j < sdp->len; j++)
			stringtab[i][j] = '\0';
		/* if just created a blank space, that's the last one */
		if (stringtab[i][0] == '\0')
			break;
	}
	/* restore the current item of scrolling region and move to it */
	stp->scrollitem = item;
	/* adjust below indicator if necessary */
	item = stp->topscroll + sdp->s_lines * sdp->s_itemsperline;
	if (item < sdp->scrnstruct->filled && stringtab[item][0])
		scr_below_ind (stp, ON);
	else	scr_below_ind (stp, OFF);
	scr_moveto (stp, sdp);
	return;
}

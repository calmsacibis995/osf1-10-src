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
static char	*sccsid = "@(#)$RCSfile: mprscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:15 $";
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
 * This file contains the MENUPROMPT screen-specific routines
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




/* "down" key in a menu prompt screen */

void
downmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp, *tsdp;
	uchar	maxfield, tfield;

	sdp = sttosd (stp);
	maxfield = stp->screenp->ndescs - 1;
	/*  move back to choice field on this line */
	while (sdp->type != FLD_CHOICE)  {
		sdp--;
		stp->curfield--;
	}
	/* in all cases, we're on the choice prompt */
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	/* clear out non-choice input fields on the current line */
	for (tsdp = sdp + 1, tfield = stp->curfield + 1;
	     tfield <= maxfield &&
	       tsdp->type != FLD_CHOICE && tsdp->row == sdp->row;
	     tsdp++, tfield++)  {
		wmove (stp->window, tsdp->row, tsdp->col);
		UNDERLINE (stp->window, ON);
		putspaces (stp->window, tsdp->len);
		UNDERLINE (stp->window, OFF);
		clearfield (tsdp);
		tsdp->scrnstruct->changed = 0;
	}
	/* until select a menu item, screen hasn't changed */
	stp->ret.flags |= R_CHANGED;
	/* Move to the choice prompt on the next line, or wrap */
	if (stp->curfield == stp->lastfield)  {
		stp->curfield = stp->firstfield;
		sdp = sttosd (stp);
	} else
		for (sdp++, stp->curfield++; sdp->type != FLD_CHOICE; sdp++)
			stp->curfield++;
	/* we're on proper menu choice field, highlight and see if it has
	 * input fields
	 */
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	if (stp->curfield < maxfield && (++sdp)->type != FLD_CHOICE)  {
		stp->curfield++;
		movetofield (stp);
	} else
		cursor (INVISCURSOR);
	return;
}

/*  "up" key in a menu prompt screen */
void
upmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp, *tsdp;
	uchar	maxfield, tfield;

	sdp = sttosd (stp);
	maxfield = stp->screenp->ndescs - 1;
	/*  move back to choice field on this line */
	while (sdp->type != FLD_CHOICE)  {
		sdp--;
		stp->curfield--;
	}
	/* in all cases, we're on the choice prompt */
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	/* clear out non-choice input fields on the current line */
	for (tsdp = sdp + 1, tfield = stp->curfield + 1;
	     tfield <= maxfield &&
	       tsdp->type != FLD_CHOICE && tsdp->row == sdp->row;
	     tsdp++, tfield++)  {
		wmove (stp->window, tsdp->row, tsdp->col);
		UNDERLINE (stp->window, ON);
		putspaces (stp->window, tsdp->len);
		UNDERLINE (stp->window, OFF);
		clearfield (tsdp);
		tsdp->scrnstruct->changed = 0;
	}
	/* until select a menu item, screen hasn't changed */
	stp->ret.flags &= ~R_CHANGED;
	/* Move to the choice prompt on the previous line, or wrap */
	if (stp->curfield == stp->firstfield) {
		stp->curfield = stp->lastfield;
		sdp = sttosd (stp);
	} else
		for (sdp--, stp->curfield--; sdp->type != FLD_CHOICE; sdp--)
			stp->curfield--;
	/* we're on proper menu choice field, highlight and see if it has
	 * input fields
	 */
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	if (stp->curfield < maxfield && (++sdp)->type != FLD_CHOICE)  {
		stp->curfield++;
		movetofield (stp);
	} else
		cursor (INVISCURSOR);
	return;
}

/*  "backtab" key in a menu prompt screen */

void
backtabmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	if ((sdp-1)->type == FLD_CHOICE)  { /* leftmost fillin field */
		beep();
		return;
	}
	if (leavefield (stp))
		return;
	stp->curfield--;
	movetofield (stp);
	return;
}

/*  "tab" key in a menu prompt screen */

void
tabmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	/* check if rightmost fillin field */
	if ((sdp+1)->type == FLD_CHOICE || (sdp+1)->row != sdp->row)  {
		beep();
		return;
	}
	if (leavefield (stp))
		return;
	stp->curfield++;
	movetofield (stp);
	return;
}

/* "left" key in a menu prompt screen */

void
leftmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	if (sdp->type != FLD_CHOICE)
		leftfill (stp);
	else	beep();
	return;
}

/* "right" key in a menu prompt screen
 * works the same way as a left key
 */

void
rightmprompt (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	if (sdp->type != FLD_CHOICE)
		rightfill (stp);
	else	beep();
	return;
}
	


/* in a menu prompt field, need to assure that all required fields
 * filled in on that prompt line.
 */

int
reqmprompt (stp)
struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	register unsigned int	i;

	sdp = sttosd (stp);

	for (sdp++, i = stp->curfield + 1;
	     i < stp->screenp->ndescs && sdp->type != FLD_CHOICE;
	     i++, sdp++)
		if (sdp->required)
		    	if ((sdp->scrnstruct->filled == 0 &&
			     sdp->scrnstruct->changed == 0) ||
		    	    (sdp->type == FLD_ALPHA &&
		             sdp->scrnstruct->pointer[0] == '\0'))
			{
				stp->curfield = i;
				message (stp, stp->window, stp->screenp,
				 REQUIREDERROR, NO);
				movetofield (stp);
				return (1);
			}
	return (0);
}


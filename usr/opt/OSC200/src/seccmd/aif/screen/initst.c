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
static char	*sccsid = "@(#)$RCSfile: initst.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:02 $";
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
 * state initialization
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



/*
 * routine to initialize the state structure used by the routine
 */

void
initstates (screenp, structp, firstdesc, stp)
struct	scrn_parms	*screenp;
struct	scrn_struct	*structp;
register struct	state	*stp;
int	firstdesc;
{
	register struct	scrn_desc	*sdp;	/* for walking sd for screen */
	register unsigned int	i;		/* for counting structp */
	register int nstructp;

	stp->curfield = stp->firstfield = stp->lastfield = 0;
	stp->itemchanged = 0;
	stp->columninfield = 0;
	stp->insert = 0;
	stp->message = 0;
	stp->structp = structp;	/* point this at the right one?  */
	stp->screenp = screenp;
	stp->window = screenp->w;
	stp->ret.flags = 0;
	stp->ret.item = 0;
	stp->scrollitem = stp->topscroll = 0;
	stp->scrnrep[0] = 0;

	if (screenp->scrntype == SCR_TEXT)
		return;

	/*  look for first and last scrn_desc's that are input fields */
	/* just to flag that we haven't changed */
	stp->firstfield = (uchar) 0xff;
	nstructp = 0;
	for (sdp = screenp->sd, i = 0; i < screenp->ndescs; i++, sdp++) {
		switch (sdp->type) {
			case FLD_PROMPT:	 /* always output-only */
			case FLD_SKIP:		/* effectively FLD_NOTHING */
			case FLD_BLANK:		/* effectively FLD_NOTHING */
				sdp->inout = FLD_OUTPUT;
				continue;	/* that's all for these */

			case FLD_CHOICE:	/* always input-only */
				sdp->inout = FLD_INPUT;
				break;
		}
		if (sdp->prompt &&
		    sdp->type != FLD_PROMPT &&
		    sdp->type != FLD_CHOICE &&
		    sdp->type != FLD_TOGGLE) {
			restorescreen();
			printf (
			"Field %d had prompt \'%s\' but was not FLD_PROMPT.\n",
			  i, sdp->prompt);
			exit (1);
		}
		if (sdp->inout == FLD_OUTPUT && sdp->type != FLD_SCROLL &&
		    sdp->type != FLD_SCRTOG && sdp->type != FLD_TEXT) {
			nstructp++;	/* still have structure entry */
			continue;
		}
		if (stp->firstfield == (uchar) 0xff) /* set first desc */
			stp->firstfield = i;
		if (sdp->type == FLD_CHOICE) {
			stp->lastfield = i;
		} else {
			stp->lastfield = i;
			nstructp++;
		}
		if (sdp->type == FLD_SCROLL || sdp->type == FLD_SCRTOG)
			sdp->s_topleft = 0;
	}
	/*  on menu screens, have to start at the top menu
	 *  item, but in fillin screens, can start somewhere
	 *  in the middle */
	if (screenp->scrntype == SCR_MENUPROMPT ||
	     screenp->scrntype == SCR_MENU)
		stp->curfield = stp->firstfield;
	else
		if (firstdesc == 0)
			stp->curfield = stp->firstfield;
		else
			stp->curfield = firstdesc;

	/* initialize pointers from scrn_desc's to structp's  */
	if (structp && !screenp->si) {
		screenp->si = 1;
		for (i = 0; i < nstructp; i++) {
			screenp->sd[structp[i].desc].scrnstruct = &structp[i];
		}
	}
	stp->message = stp->curfield;
	sdp = sttosd (stp);
	if (sdp->prompt && sdp->type == FLD_TOGGLE) {
		strcpy (stp->scrnrep, sdp->prompt);
	}
#ifdef DEBUG
fprintf (logfp, "curfield %d lastfield %d firstfield %d.\n",
  stp->curfield, stp->lastfield, stp->firstfield);
#endif /* DEBUG */
	return;
}

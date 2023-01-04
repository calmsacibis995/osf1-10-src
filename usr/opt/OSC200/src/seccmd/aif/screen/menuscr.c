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
static char	*sccsid = "@(#)$RCSfile: menuscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:12 $";
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
 * This file contains the basic MENU screen-specific routines
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


/* "down" key in a menu screen */
void
downmenu (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	stp->curfield = finddownitem (stp, sdp);
	sdp = sttosd (stp);
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	return;
}

/* right key in a menu screen */

void
rightmenu (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	if (stp->curfield == stp->lastfield)  {
		stp->curfield = stp->firstfield;
		sdp = sttosd (stp);
	} else
		for (stp->curfield++, sdp++;
		     stp->curfield <= stp->lastfield;
		     sdp++, stp->curfield++)
			if (sdp->type == FLD_CHOICE)
				break;
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	return;
}

/* "up" key in a menu screen */
void
upmenu (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;

	sdp = sttosd (stp);
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	stp->curfield = findupitem (stp, sdp);
	sdp = sttosd (stp);
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	return;
}

/* left key in a menu screen */
void
leftmenu (stp)
register struct	state	*stp;
{
	register struct	scrn_desc	*sdp;
	sdp = sttosd (stp);
	unhighlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	if (stp->curfield == stp->firstfield)  {
		stp->curfield = stp->lastfield;
		sdp = sttosd (stp);
	} else
		for (stp->curfield--, sdp--;
		     stp->curfield >= stp->firstfield;
		     sdp--, stp->curfield--)
			if (sdp->type == FLD_CHOICE)
				break;
	highlight (stp->window, sdp->row, sdp->col, sdp->prompt);
	return;
}

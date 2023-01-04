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
static char *rcsid = "@(#)$RCSfile: menuscr.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:12:16 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	menuscr.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:04:37  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:59:27  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:51  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:40  marquard]
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
 * Copyright (c) 1989-90 SecureWare, Inc.
 *   All rights reserved.
 *
 * Based on OSF version:
 *	@(#)menuscr.c	1.4 16:31:11 5/17/91 SecureWare
 */

/* #ident "@(#)menuscr.c	1.1 11:17:14 11/8/91 SecureWare" */

/*
 * This file contains the basic MENU screen-specific routines
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

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

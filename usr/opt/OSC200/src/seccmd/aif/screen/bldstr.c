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
static char	*sccsid = "@(#)$RCSfile: bldstr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:25 $";
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
/* Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 */



/*
 *	bldstr.c
 *
 *	buildscrn_struct()	- allocate & set up pointers for scrn_struct
 *	rebuildscrn_struct()	- reset pointers for extant scrn_struct
 *	freescrn_struct()	- free a scrn_struct
 */

#include <sys/types.h>
#include "userif.h"
#include "kitch_sink.h"
#include "logging.h"

#ifdef DEBUG
#include <stdio.h>

FILE *logfp;
#define	LOGFILE "logfile"
#endif DEBUG



/* build the screen structure for a menuprompt screen.
 * need to allocate enough space such that all data
 * items have the proper amount reserved.
 */

struct	scrn_struct *
buildscrn_struct (screenp)
struct	scrn_parms	*screenp;
{
	struct	scrn_desc	*sd;
	struct	scrn_struct	*sp,	/* for walking through */
				*retsp;	/* return value of function */
	unsigned int	i;
	int	nstructs;

	ENTERFUNC ("buildscrn_struct");
	if (screenp->scrntype == SCR_MENU || screenp->scrntype == SCR_TEXT) {
		ERRFUNC ("buildscrn_struct", "screen type requires no struct");
		return (NULL);
	}

	nstructs = 0;

	/* count up the number of entries for fillin fields */

	for (i = 0, sd = screenp->sd; i < screenp->ndescs; i++, sd++) {

		switch (sd->type) {
		case	FLD_ALPHA:
		case	FLD_CONFIRM:
		case	FLD_NUMBER:
		case	FLD_POPUP:
		case	FLD_SCROLL:
		case	FLD_SCRTOG:
		case	FLD_TEXT:
		case	FLD_TOGGLE:
		case	FLD_YN:
			nstructs++;
			break;
		case	FLD_CHOICE:
		case	FLD_PROMPT:
			break;
		}
	}

	/*
	 * IF NOTHING TO DO, SPLIT
	 */
	if (nstructs == 0) {
		ERRFUNC ("buildscrn_struct", "nothing to do here");
		return (NULL);
	}

	/* allocate the table of scrn_structs and the memory to hold the
	 * values.  check success of Calloc().
	 */

	retsp = (struct scrn_struct *) Calloc (nstructs,
					sizeof (struct scrn_struct));
	if (retsp == NULL) {
		ERRFUNC ("buildscrn_struct", "out of memory");
		MemoryError();
	}

	/*
	 * Set up the desc fields in the scrn_struct values
	 */
	for (i = 0, sd = screenp->sd, sp = retsp;
	     i < screenp->ndescs;
	     i++, sd++) {
		switch (sd->type) {
		case	FLD_ALPHA:
		case	FLD_CONFIRM:
		case	FLD_NUMBER:
		case	FLD_POPUP:
		case	FLD_SCROLL:
		case	FLD_SCRTOG:
		case	FLD_TEXT:
		case	FLD_TOGGLE:
		case	FLD_YN:
			sp->desc = i;
			sd->scrnstruct = sp++;
			break;
		case	FLD_CHOICE:
		case	FLD_PROMPT:
			break;
		}
	}
	EXITFUNC ("buildscrn_struct");
	return (retsp);
}



/*
 *	freescrn_struct() - free the scrn_struct created by build_struct
 */

void
freescrn_struct (sp)
struct	scrn_struct	*sp;
{
	ENTERFUNC ("freescrn_struct");
	if (sp)
		Free (sp);
	EXITFUNC ("freescrn_struct");
	return;
}



/*
 *	rebuildscrn_struct() - rebuild pointers in a scrn_struct
 *	(probably because of a BUILDSTRUCT routine
 */

struct	scrn_struct *
rebuildscrn_struct (screenp)
struct	scrn_parms	*screenp;
{
	struct	scrn_desc	*sd;
	struct	scrn_struct	*sp;	/* for walking through */
	unsigned int	i;
	int	nstructs, bytes, nb;
	char	*cp;

	ENTERFUNC ("rebuildscrn_struct");
	if (screenp->scrntype == SCR_MENU || screenp->scrntype == SCR_TEXT) {
		ERRFUNC("rebuildscrn_struct","screen type requires no struct");
		return (NULL);
	}

	for (i = 0, sd = screenp->sd, sp = screenp->ss;
	     i < screenp->ndescs;
	     i++, sd++) {
		switch (sd->type) {
		case	FLD_ALPHA:
		case	FLD_CONFIRM:
		case	FLD_NUMBER:
		case	FLD_POPUP:
		case	FLD_SCROLL:
		case	FLD_SCRTOG:
		case	FLD_TEXT:
		case	FLD_TOGGLE:
		case	FLD_YN:
			sp->desc = i;
			sd->scrnstruct = sp++;
			break;
		case	FLD_CHOICE:
		case	FLD_PROMPT:
			break;
		}
	}
	EXITFUNC ("rebuildscrn_struct");
	return (screenp->ss);
}

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
static char *rcsid = "@(#)$RCSfile: bldstr.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:11:16 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	bldstr.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:03:24  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:58:20  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:19  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:51:44  marquard]
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
 * Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 *
 * Based on OSF version:
 *	@(#)bldstr.c	1.9 16:30:51 5/17/91 SecureWare
 */

/* #ident "@(#)bldstr.c	1.1 11:15:56 11/8/91 SecureWare" */

/*
 *	bldstr.c
 *
 *	buildscrn_struct()	- allocate & set up pointers for scrn_struct
 *	rebuildscrn_struct()	- reset pointers for extant scrn_struct
 *	freescrn_struct()	- free a scrn_struct
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

/*
 * Count the number of scrn_structs that will be necessary for a given
 * scrn_desc table.  Allocate the scrn_struct table and connect
 * its elements to the scrn_desc table, including setting the
 * reference in the scrn_struct table to the scrn_desc index.
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

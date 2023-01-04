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
static char	*sccsid = "@(#)$RCSfile: scrnfuncs.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:32 $";
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



/* scrnfuncs.c - screen setup & exit metafunctions:
 *
 *    scrnsetup() - one-time screen setup
 *    scrn_init() - screen setup each time in traverse() loop
 *    scrnexit() - validate & otherwise finish up a screen
 *    scrnfree() - free data structures when done with screen
 *
 * setup, _init, and free are per screen, while exit is per screen
 * item (scrn_desc). Any of them may be NULL. in which case they
 * will not be executed.
 */

#include	<stdio.h>
#include	"userif.h"
#include	"logging.h"


/* 
 * This function is called on traverse() entry if data needs
 * to be filled in first. It is executed only once per screen.
 */

scrnsetup (argv, fill, authfunc, bfillfunc)
char	**argv;
char *fill;
int (*authfunc)(), (*bfillfunc)();
{
	int status = 1;
	ENTERLFUNC ("scrnsetup");
	if (!(*authfunc) (argv, fill))
		status = (*bfillfunc) (fill);
	EXITLFUNC ("scrnsetup");
	return (status);
}



/* 
 * This function is called prior to putscreen() in traverse() 
 * if data needs to be filled in first. It is called every
 * time through the display loop.
 */

scrn_init (argv, fill, buildstruct, structtemplate, parmtemplate)
char	**argv;
char *fill;
int (*buildstruct) ();
struct  scrn_struct	**structtemplate;
struct	scrn_parms	*parmtemplate;
{
	parmtemplate->fillin = fill;
	return ((*buildstruct) (fill, structtemplate));
}




extern int _DoAction;	/* set in (extern) traverse() */

/* 
 * This function is called when a screen is executed. The
 * validatefunc() and actionfunc() are loosely defined.
 * Depending on various factors, validation and final
 * screen action may both occur in validatefunc(), or in
 * actionfunc(). Additionally, _DoAction may be reset in
 * traverse() to prevent actionfunc() from occurring.
 * Currently this is used when R_ACTHELP is returned by
 * a screen (indicating "active" help, such as a popup
 * selection box).
 */

scrnexit (argv, fill, validatefunc,
	  actionfunc, firstdesc, nscrnstruct,
	  parmtemplate, desctemplate, structtemplate)
char	**argv;
char *fill;
int                  (*validatefunc)();
struct scrn_struct * structtemplate;
struct scrn_desc *   desctemplate;
struct scrn_parms *  parmtemplate;
int                  (*actionfunc)();
int                  firstdesc;
int                  nscrnstruct;
{
	struct	scrn_struct	*sp, *tsp;
	struct	scrn_desc	*sd, *tsd;
	struct	scrn_parms	*screenp;
	int	i;
	int	ret;
	short	changed;	/* couldn't resist */
	int	scrn_changed;


	ENTERFUNC("scrnexit");

	sp = structtemplate; screenp = parmtemplate; sd = desctemplate;

	if (screenp->scrntype == SCR_MSGONLY)  {

	/* for message only screens, don't worry about entry and validating */
		traverse (screenp, firstdesc);
		ERRFUNC("scrnexit", "left MSG_ONLY screen");
		return (1);
	}

	if ((*validatefunc) (argv, fill)) {
		ERRFUNC("scrnexit", "validatefunc failed");
		return (CONTINUE);
	}
	
	scrn_changed = 0;
	/*
	 * for each changed field, change desc so it will
	 * display next time.  Also, need to propogate any
	 * "fixed" changes to action will continue.
	 */
	for (i = 0; i < nscrnstruct; i++)
		if (sp[i].changed) {
			if (!sp[i].filled)
				sp[i].filled = 1;
			sd[sp[i].desc].inout = FLD_BOTH;
			scrn_changed = 1;
		}
	/*
	 * now deal with changes to the screen, if any were reported
	 */
		if (!scrn_changed) { /* nothing previously changed */
			ERRFUNC("scrnexit", "no changes");
			return (1);
		}
		scrn_changed = 1;

	/* walk through and run validation routines */

	changed = 0;
	for (i = 0, tsp = sp; i < nscrnstruct; i++, tsp++)  {
		tsd = &sd[tsp->desc];
		if (tsp->changed && tsd->inout != FLD_OUTPUT)
			if (tsp->validate && (*tsp->validate) (fill)) {
				changed = 1;
				break;
			}
	}
	/*
	 * If _DoAction not set, do not run the final action proc.
	 * if changed != 0, not everything validated
	 */
	if (!_DoAction)
		return 1;
	if (changed)
		return CONTINUE;

	ret = (*actionfunc) (fill);

	EXITFUNC("scrnexit");
	return (ret);
}


/*
 * scrnfree() frees up screen structures when traverse() is
 * through with that screen.
 */

scrnfree (argv, fill, nscrnstruct, freefunc,
	  parmtemplate, desctemplate, structtemplate)
char               **argv;
char                *fill;
int                  nscrnstruct;
int                  (*freefunc)();
struct scrn_parms   *parmtemplate;
struct scrn_desc    *desctemplate;
struct scrn_struct  *structtemplate;
{
	int ret;

	ENTERFUNC ("scrnfree");
	ret = (*freefunc) (argv, fill, nscrnstruct,
		parmtemplate, desctemplate, structtemplate);
	EXITFUNC ("scrnfree");
	return ret;
}

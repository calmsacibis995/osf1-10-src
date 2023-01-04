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
static char *rcsid = "@(#)$RCSfile: traverse.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:14:19 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	traverse.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:05:46  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:00:29  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:52  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:54:22  marquard]
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
/* Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 *
 * Based on OSF version:
 *	@(#)traverse.c	1.21 16:31:39 5/17/91 SecureWare
 */

/* #ident "@(#)traverse.c	1.1 11:19:14 11/8/91 SecureWare" */

/*
 *	traverse.c
 *
 *	GLOBAL
 *	traverse()	- main screen traversal routine
 *
 *	LOCAL
 *	sdtocn()	- convert scrn_desc index to choice number
 *	call_routine()	- call a routine as result of choice
 *	make_arglist()	- make arglist from menu stuff
 *	free_arglist()	- free arglist
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

static int run_program ();
static call_routine (), free_arglist ();
static char **make_arglist ();

int _DoAction = 1;	/* set to 0 to not do final action, def = 1 */
			/* around call_routine() and (extern) scrnexit() */


/* go through the menu, displaying and doing appropriate things with
 * the choices.
 * returns:
 *	QUIT  - exit program
 *	ABORT - exit screen
 */

int TravLvl = 0;

traverse(screenp, firstdesc)
struct	scrn_parms *screenp;
int	firstdesc;
{
	struct	scrn_struct	*sp;
	int	ret;	/* returned from recursive traversal */
	struct	scrn_ret	scrn_ret;	/* return from getscreen */
	struct	menu_scrn	*mp;
	unsigned int	choice;	/* for mapping choice to proper screen_desc */
	struct	scrn_desc	*sdp;
	unsigned int	i;
	int backup = 0;

	ENTERFUNC ("traverse");
	TravLvl++;
	werase(overlay_win);
	touchwin(overlay_win);
	wrefresh(overlay_win);
	if (screenp->setup)
		if (call_routine(screenp->setup, -1, 0, NULL))
			return (1);
	DUMPVARS ("TravLvl UP to <%-2.2d>", TravLvl, NULL, NULL);
	if (screenp->sh && screenp->sh->title) {
		DUMPVARS ("Screen title = \'%s\'", screenp->sh->title, 0, 0);
	}
	do {
		/* display the screen. If not in use, allocate memory for a
		 * screenp structure.
		 */
	
		if (screenp->inuse) {
			sp = screenp->ss;
			rebuildscrn_struct(screenp);
		} else {
			initscreen(screenp, 0);
			sp = buildscrn_struct(screenp);
			screenp->ss = sp;
			screenp->inuse = TravLvl;
		}
		if (screenp->init)
			if (call_routine(screenp->init, -1, 0, NULL)) {
				ret = ABORT;
				break;
			}
		putscreen(screenp, sp, POP);
		headers(screenp);
		scrn_ret = getscreen(screenp, sp,
		  firstdesc == -1 ? screenp->first_desc : firstdesc);
	
		if (scrn_ret.flags & R_ABORTED)  {
			DUMPDECP ("getscreen returned aborted", NULL, NULL, 0);
			ret = ABORT;
		} else if (scrn_ret.flags & R_QUIT) {
			DUMPDECP ("getscreen returned quit", NULL, NULL, 0);
			 ret = QUIT;
		} else if (scrn_ret.flags & R_EXECUTE) {
			DUMPDECP ("getscreen returned item %d flags %d",
				scrn_ret.item, scrn_ret.flags, 0);
	
			choice = sdtocn (screenp, &sdp, scrn_ret.item);
			if (screenp->scrntype == SCR_FILLIN)
				sdp--; /* to fool run_ stuff below */

			/*
			 * call a routine, run a program, or recurse
			 * to another menu
			 */

			wclear (screenp->w);
			wrefresh (screenp->w);
			ret = trav_choice(screenp, choice, &backup, sdp, 0);

		} else if (scrn_ret.flags & R_ACTHELP) {
			/*
			 * help screens do screen management
			 */
			scrn_ret.flags &= ~R_ACTHELP;
			;

		} else if (scrn_ret.flags & R_POPUP) {
			DUMPDECP ("getscreen returned popup", NULL, NULL, 0);
			 ret = CONTINUE;
		} else {

		/* something's wrong */
		/* normally, user will have picked a choice */
	
			restorescreen();
			fprintf (stderr,
			  MSGSTR(TRAVERSE_1, "Returned from getscreen without choosing\n"));
			fprintf (stderr, MSGSTR(TRAVERSE_2, "item %d, changed %s\n"),
				scrn_ret.item,
				(scrn_ret.flags & R_CHANGED) ? MSGSTR(TRAVERSE_3, "yes") : MSGSTR(TRAVERSE_4, "no"));
			exit (1); /* PANIC! */
		}
	} while ((ret != QUIT && ret != ABORT) && (!(backup && screenp->skip)));
	if (ret == ABORT && screenp != TopScrn)
		ret = CONTINUE;
	if (screenp->inuse >= TravLvl) {
		freescrn_struct (sp);
		screenp->ss = NULL;
		screenp->inuse = screenp->si = 0;
		if (screenp->free)
			 call_routine (screenp->free, -1, 0, NULL);
	} else if (screenp->skip) {
		freescrn_struct (sp);
		screenp->ss = NULL;
		if (screenp->free)
			 call_routine (screenp->free, -1, 0, NULL);
	}
	if (ret == QUIT || !screenp->skip) {
		wclear(screenp->w);
		wrefresh(screenp->w);
	}
	TravLvl--;
	DUMPVARS ("TravLvl DOWN to <%-2.2d>", TravLvl, NULL, NULL);
	EXITFUNC ("traverse");
	return (ret);
}



/* convert the scrn_desc index to choice number */

int
sdtocn(sp, sdp, item)

struct scrn_parms *sp;
struct scrn_desc **sdp;
int item;
{
	int i, choice;

	*sdp = sp->sd;
	choice = 0;
	switch (sp->scrntype) {
	case SCR_FILLIN:
	case SCR_NOCHANGE:
		for (i = 0; i < sp->ndescs; i++)  {
			if ((*sdp)->type != FLD_PROMPT)  {
				choice++;
				if (i == item)
					break;
			}
			(*sdp)++;
		}
		break;
	default:
		for (i = 0; i < sp->ndescs; i++)  {
			if ((*sdp)->type == FLD_CHOICE)  {
				choice++;
				if (i == item)
					break;
			}
			(*sdp)++;
		}
		break;
	}
	return choice;
}



/*
 * perform the appropriate action based on the user selection.
 * Set appropriate global & traverse() flags (_DoAction & backup).
 */

int
trav_choice (sp, choice, backup, sdp, skip_action)
struct scrn_parms *sp;
unsigned int choice;	/* for mapping choice to proper screen_desc */
int *backup;
struct scrn_desc *sdp;
unsigned short skip_action;
{
	struct menu_scrn *mp;
	int ret;

	ENTERFUNC ("trav_choice");
	/* find the menu description assoc. with the choice */
	for (mp = sp->ms; mp->choice != choice; mp++)
		;
	DUMPDECP (" choice picked was <%d>, menu type <%d>",
		choice, mp->type, 0);
	DUMPDECP (" routine 0x%lx, nparams %d", mp->next_routine,
		mp->nparams, 0);
	
	switch (mp->type) {
		case	M_MENU:
			ret = traverse (mp->next_menu, -1);
			if (ret != ABORT && ret != CONTINUE)
				*backup = 1;
			break;
		case	M_ROUTINE:
			if (mp->next_routine) {
				/* call with sdp + 1 to get next field after
				 * choice
				 */
				if (skip_action)
					_DoAction = 0;
				ret = call_routine (mp->next_routine,
					choice,
					mp->nparams,
					sdp + 1);
				if (ret != ABORT && ret != CONTINUE)
					*backup = 1;
				_DoAction = 1;
			} else
				ret = ABORT;
			break;
	}
	EXITFUNC ("trav_choice");
	return (ret);
}

/* call a routine and pass it a list of parameters */

static
call_routine (routine, choice, nparams, sd)
int	(*routine)();
int	choice;
int	nparams;
struct	scrn_desc	*sd;
{
	char	**argv, **make_arglist();
	int	ret = 0;

	ENTERFUNC ("call_routine");
	if (routine) {
		argv = make_arglist (sd, choice, nparams, NULL);
		DUMPVARS ("call_routine", "choice was %s\n", argv[1], NULL);
		for (ret = 0; ret < nparams; ret++)
			DUMPVARS ("call_routine", "arg %d is %s\n",
				ret, argv[ret + 2]);
		/* first argument is program, which is NULL in this case */
		ret = (*routine) (&argv[1]);
		free_arglist (argv, nparams);
	}
	EXITFUNC ("call_routine");
	return (ret);
}



/* make an argument list from a menu prompt list */

static
char **
make_arglist (sd, choice, nparams, program)
struct	scrn_desc	*sd;
int	choice;
int	nparams;
char	*program;
{
	char	**argv;
	char	numbuf[11];	/* can't have bigger than this in 32 bits */

	ENTERFUNC ("make_arglist");
	/* allocate string table for args to program */
	argv = (char **) Malloc ((nparams + 3) * sizeof (char *));
	argv[0] = program;
	sprintf (numbuf, "%d", choice);
	argv[1] = (char *) Malloc (strlen (numbuf) + 1);
	strcpy (argv[1], numbuf);
	argv[2] = NULL;
	EXITFUNC ("make_arglist");
	return (argv);
}


/* free the argument list */

static
free_arglist (argv, nparams)
char	**argv;
int	nparams;
{
	int	i;

	ENTERFUNC ("free_arglist");
	Free ((char *) argv);
	EXITFUNC ("free_arglist");
	return;
}

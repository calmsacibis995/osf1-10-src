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
static char	*sccsid = "@(#)$RCSfile: traverse.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:45 $";
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
 *	traverse.c
 *
 *	GLOBAL
 *	traverse()	- main screen traversal routine
 *
 *	LOCAL
 *	sdtocn()	- convert scrn_desc index to choice number
 *	run_program()	- run a program as result of choice
 *	call_routine()	- call a routine as result of choice
 *	make_arglist()	- make arglist from menu stuff
 *	free_arglist()	- free arglist
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <grp.h>
#include <sys/security.h>
#include <sys/audit.h>
#include "userif.h"
#include "logging.h"

#ifdef DEBUG
#include <stdio.h>

FILE *logfp;
#define	LOGFILE "logfile"
#else DEBUG
#endif DEBUG

static int run_program ();
static call_routine (), free_arglist ();
static char **make_arglist ();

void freescrn_struct (), rebuildscrn_struct ();
struct scrn_struct *buildscrn_struct ();

int _DoAction = 1;	/* set to 0 to not do final action, def = 1 */
			/* around call_routine() and (extern) scrnexit() */


/* go through the menu, displaying and doing appropriate things with
 * the choices.
 * returns:
 *	QUIT  - exit program
 *	ABORT - exit screen
 */

int TravLvl = 0;

traverse (screenp, firstdesc)
struct	scrn_parms *screenp;
int	firstdesc;
{
	struct	scrn_struct	*sp, *buildscrn_struct();
	int	ret;	/* returned from recursive traversal */
	struct	scrn_ret	scrn_ret;	/* return from getscreen */
	struct	menu_scrn	*mp;
	unsigned int	choice;	/* for mapping choice to proper screen_desc */
	struct	scrn_desc	*sdp;
	unsigned int	i;
	int backup = 0;

	ENTERFUNC ("traverse");
	TravLvl++;
	touchwin (overlay_win);
	wrefresh (overlay_win);
	if (screenp->setup)
		if (call_routine (screenp->setup, -1, 0, NULL))
			return (1);
	DUMPVARS ("TravLvl UP to <%-2.2d>", TravLvl, NULL, NULL);
	if (screenp->sh && screenp->sh->title) {
		DUMPVARS ("Screen title = \'%s\'", screenp->sh->title, 0, 0);
	}
	do {
		/* display the screen. If a menuprompt, allocate memory for a
		 * screenp structure.
		 */
	
		if (screenp->inuse) {
			sp = screenp->ss;
			rebuildscrn_struct (screenp);
		} else {
			sp = (struct scrn_struct *) 0;
			initscreen (screenp, 0);
			sp = buildscrn_struct (screenp);
			screenp->ss = sp;
			screenp->inuse = TravLvl;
		}
		if (screenp->init)
			if (call_routine (screenp->init, -1, 0, NULL)) {
				ret = ABORT;
				goto BOOGIE;
			}
		putscreen (screenp, sp, POP);
		headers (screenp);
		scrn_ret = getscreen (screenp, sp,
		  firstdesc == -1 ? screenp->first_desc : firstdesc);
	
		if (scrn_ret.flags & R_ABORTED)  {
			DUMPDECP ("getscreen returned aborted", NULL, NULL, 0);
			ret = ABORT;
		} else if (scrn_ret.flags & R_QUIT) {
			DUMPDECP ("getscreen returned quit", NULL, NULL, 0);
			 ret = QUIT;
		} else if (scrn_ret.flags & R_EXECUTE ||
		  scrn_ret.flags & R_ACTHELP){
			DUMPDECP ("getscreen returned item %d flags %d",
				scrn_ret.item, scrn_ret.flags, 0);
	
			choice = sdtocn (screenp, &sdp, scrn_ret.item);
			if (screenp->scrntype == SCR_FILLIN)
				sdp--; /* to fool run_ stuff below */

			/* call a routine, run a program, or recurse to another menu */
			wclear (screenp->w);
			wrefresh (screenp->w);
			ret = trav_choice (screenp, choice, &backup, sdp,
				(scrn_ret.flags & R_ACTHELP));
		} else if (scrn_ret.flags & R_POPUP) {
			DUMPDECP ("getscreen returned popup", NULL, NULL, 0);
			 ret = CONTINUE;
		} else {

		/* something's wrong */
		/* normally, user will have picked a choice */
	
			restorescreen();
			fprintf (stderr,
			  "Returned from getscreen without choosing\n");
			fprintf (stderr, "item %d, changed %s\n",
				scrn_ret.item,
				(scrn_ret.flags & R_CHANGED) ? "yes" : "no");
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
	if (ret != QUIT && !screenp->skip) {
		wclear (screenp->w);
		wrefresh (screenp->w);
	}
BOOGIE:
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
		case	M_PROGRAM:
			/* call with sdp + 1 to get next field after
			 * choice
			 */
			ret = run_program (mp->next_program,
				choice,
				mp->nparams,
				sdp + 1);
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



/* run a program with an argument as listed on the menuprompt lines */

static int
run_program (program, choice, nparams, sd)
char	*program;		/* program to run */
int	choice;			/* which choice was picked */
int	nparams;		/* parameters to call it with */
struct	scrn_desc	*sd;	/* where to get params from */
{
	char	**argv, **make_arglist();
	int	ret;

	ENTERFUNC ("run_program");
	argv = make_arglist (sd, choice, nparams, program);

	ret = print_output (program, argv);

	free_arglist (argv, nparams);
	EXITFUNC ("run_program");
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
	int	i;
	char	*cp;
	char	numbuf[11];	/* can't have bigger than this in 32 bits */
	static	char yes[2] = "Y";
	static	char no[2] = "N";

	ENTERFUNC ("make_arglist");
	/* allocate string table for args to program */
	argv = (char **) Malloc ((nparams + 3) * sizeof (char *));
	argv[0] = program;
	sprintf (numbuf, "%d", choice);
	argv[1] = (char *) Malloc (strlen (numbuf) + 1);
	strcpy (argv[1], numbuf);

	for (i = 0; i < nparams; i++, sd++) {
		switch (sd->type) {
		case	FLD_ALPHA:
			cp = s_alpha(sd->scrnstruct);
			break;
		case	FLD_NUMBER:
			DUMPVARS ("make_arglist", "param %d number was %ld\n",
  				i, *s_number(sd->scrnstruct));
			sprintf (numbuf, "%ld", *s_number(sd->scrnstruct));
			cp = numbuf;
			break;
		case	FLD_YN:
			if (*s_yesno(sd->scrnstruct))
				cp = yes;
			else	cp = no;
			break;
		}
		argv[i + 2] = (char *) Malloc (strlen (cp) + 1);
		strcpy (argv[i + 2], cp);
	}
	argv[nparams + 2] = NULL;
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
	/* first argument is program, which is pointed to, not malloced */
	for (i = 1; i < nparams + 2; i++)
		Free (argv[i]);
	Free ((char *) argv);
	EXITFUNC ("free_arglist");
	return;
}

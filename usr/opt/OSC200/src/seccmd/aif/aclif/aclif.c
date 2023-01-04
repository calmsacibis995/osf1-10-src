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
static char	*sccsid = "@(#)$RCSfile: aclif.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:21 $";
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
 * Access Control List Editor
 *
 * Copyright (c) 1988-1990, SecureWare, Inc.
 *   All Rights Reserved
 */



#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <setjmp.h>
#include <signal.h>
#include <acl.h>
#include <sys/stat.h>
#include <sys/security.h>
#include <sys/audit.h>

#include "userif.h"

#define	ACL_CHUNK	24	/* alloc entries in multiples of this */
#define	ACL_MARGIN	8	/* alloc at least this many extra entries */

char		*HelpDir = "/usr/share/lib/sechelp/";

acle_t		*acl_buf;	/* working ACL buffer */
int		acl_size;	/* number of entry slots in acl_buf */
int		acl_ents;	/* number of valid entries in acl_buf */
char		**acl_scroll;	/* table for edit scrolling region */
char		**syn_scroll;	/* table for synonym list scrolling region */
int		syn_size;	/* size of syn_scroll */
int		acl_invalid;	/* flag to indicate validity of acl_buf */
struct stat	cur_stb;	/* stat buffer for access check */
char		cur_file[65];	/* current remembered file name */
char		file_buf[65];	/* buffer for file name fillin screen */
char		cur_syn[33];	/* current remembered synonym name */
char		syn_buf[33];	/* buffer for synonym name fillin screen */
char		cur_unam[9];	/* current user name for access check */
char		unam_buf[9];	/* buffer for access check fillin screen */
char		cur_gnam[9];	/* current group name for access check */
char		gnam_buf[9];	/* buffer for access check fillin screen */
ushort		cur_uid;	/* current user ID for access check */
ushort		cur_gid;	/* current group ID for access check */
uchar		edit_check;	/* flag to check access after edit */

static char	*perm_strings[] = {
	"null", "only execute", "only write", "write and execute",
	"only read", "read and execute", "read and write",
	"read, write and execute" };

int		loadacl(), loadsyn(), clearbuf(), editacl(), testaccess(),
		applyacl(), deleteacl(), definesyn(), deletesyn(), listsyns(),
		user_validate(), group_validate(), file_validate(),
		acl_validate(), edit_validate();

extern void	userif_void();
extern int	userif_one(), userif_zero();
extern char	*malloc(), **alloc_cw_table(), **expand_cw_table();
extern struct passwd	*getpwnam(), *getpwuid();
extern struct group	*getgrnam(), *getgrgid();

struct acl_fillin {
	int	ndescs;
};

extern int	errno;
extern char	*sys_errlist[];
extern char	*acl_dbfile;
extern aclsyn_t	acl_syns;

/******************************************************************************/

/*
 * Main Menu for Access Control List Editor
 */

struct scrn_desc	main_desc[] = {

/* ROW COL  TYPE       LEN  INOUT       REQ  PROMPT */
  {  3,  6, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Access Control List Editor" },

  {  6,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Load ACL Buffer from File",
						"aclif/top/1"	},
  {  7,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Load ACL Buffer from Synonym",
						"aclif/top/2"		},
  {  8,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Clear ACL Buffer",
						"aclif/top/3"		},
  {  9,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Edit ACL Buffer",
						"aclif/top/4"		},
  { 10,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Test Access",
						"aclif/top/5"		},

  { 12,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Apply an ACL to a File",
						"aclif/top/6"		},
  { 13,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Remove an ACL from a File",
						"aclif/top/7"		},
  { 14,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Define an ACL Synonym",
						"aclif/top/8"		},
  { 15,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "Delete an ACL Synonym",
						"aclif/top/9"		},
  { 16,  5, FLD_CHOICE,  0, FLD_INPUT,   NO, "List ACL Synonyms",
						"aclif/top/10"		},
};

struct menu_scrn	main_menu[] = {

/* CHOICE  TYPE  	NEXT_MENU	NEXT_ROUTINE	NEXT_PROGRAM    ARGS */
  {	 1, M_ROUTINE,	NULL,		loadacl,	NULL,		0 },
  {	 2, M_ROUTINE,	NULL,		loadsyn,	NULL,		0 },
  {	 3, M_ROUTINE,	NULL,		clearbuf,	NULL,		0 },
  {	 4, M_ROUTINE,	NULL,		editacl,	NULL,		0 },
  {	 5, M_ROUTINE,	NULL,		testaccess,	NULL,		0 },
  {	 6, M_ROUTINE,	NULL,		applyacl,	NULL,		0 },
  {	 7, M_ROUTINE,	NULL,		deleteacl,	NULL,		0 },
  {	 8, M_ROUTINE,	NULL,		definesyn,	NULL,		0 },
  {	 9, M_ROUTINE,	NULL,		deletesyn,	NULL,		0 },
  {	10, M_ROUTINE,	NULL,		listsyns,	NULL,		0 },
};

struct scrn_parms	main_parms = {

	SCR_MENU,		/* scrn_type  */
	2, 21, 19, 38,		/* top, left, rows, cols */
	NUMDESCS(main_desc),	/* number of scrn_descs */
	main_desc,		/* table of descs */
	main_menu		/* menu action descriptions */
};

struct scrn_parms	*TopScrn = &main_parms;

/******************************************************************************/

/*
 * "Enter File Name" Fillin Screen
 */

struct scrn_desc	file_desc[] = {

/* ROW COL  TYPE       LEN  INOUT       REQ  PROMPT */
  {  3,  5, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Enter File Name:"		},
  {  5,  5, FLD_ALPHA,  64, FLD_BOTH,   YES				},
};

struct scrn_struct	file_struct[] = {

    {	NULL,	0,	0,	1,	userif_zero }
};
#define	FILENAME	0
#define	FILENSTRUCT	1

struct scrn_parms	file_parms = {

	SCR_FILLIN,		/* scrn_type  */
	14, 3, 9, 74,		/* top, left, rows, cols */
	NUMDESCS(file_desc),	/* number of scrn_descs */
	file_desc,		/* table of descs */
};

/******************************************************************************/

/*
 * "Enter Synonym Name" Fillin Screen
 */

struct scrn_desc	syn_desc[] = {

/* ROW COL  TYPE       LEN  INOUT       REQ  PROMPT */
  {  3,  5, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Enter Synonym Name:"	},
  {  3, 25, FLD_ALPHA,  32, FLD_BOTH,   YES				},
};

struct scrn_struct	syn_struct[] = {

    {	NULL,	0,	0,	1,	userif_zero },
};
#define	SYNNAME		0
#define	SYNNSTRUCT	1

struct scrn_parms	syn_parms = {

	SCR_FILLIN,		/* scrn_type  */
	16, 9, 7, 62,		/* top, left, rows, cols */
	NUMDESCS(syn_desc),	/* number of scrn_descs */
	syn_desc,		/* table of descs */
};

/******************************************************************************/

/*
 * "Test Access" Fillin Screen
 */

struct scrn_desc	test_desc[] = {

/* ROW COL  TYPE       LEN  INOUT       REQ  PROMPT */
  {  3, 31, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Test Access"		},
  {  6, 21, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "User:"			},
  {  6, 27, FLD_ALPHA,   8, FLD_BOTH,   YES, NULL, "aclif/test/1"	},
  {  6, 39, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Group:"			},
  {  6, 46, FLD_ALPHA,   8, FLD_BOTH,   YES, NULL, "aclif/test/2"	},
  {  8,  5, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "File:"			},
  {  9,  5, FLD_ALPHA,  64, FLD_BOTH,   YES, NULL, "aclif/test/3"	},
};

struct scrn_struct	test_struct[] = {

    {	NULL,	0,	0,	2,	user_validate },
    {	NULL,	0,	0,	4,	group_validate },
    {	NULL,	0,	0,	6,	file_validate },
};
#define	TESTUSER	0
#define	TESTGROUP	1
#define	TESTFILE	2
#define	TESTNSTRUCT	3

struct scrn_parms	test_parms = {

	SCR_FILLIN,		/* scrn_type  */
	5, 3, 13, 74,		/* top, left, rows, cols */
	NUMDESCS(test_desc),	/* number of scrn_descs */
	test_desc,		/* table of descs */
};

/******************************************************************************/

/*
 * "Edit ACL Buffer" Fillin Screen
 */

struct scrn_desc	edit_desc[] = {

/* ROW COL  TYPE       LEN  INOUT       REQ  PROMPT */
  {  3, 23, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Edit ACL Buffer"		},
  {  6,  5, FLD_SCROLL, 24, FLD_BOTH,    NO, NULL, "aclif/edit/1",
						8, 4, 2			},
  { 15,  9, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
			"Do you want to test access with this ACL?"	},
  { 15, 52, FLD_YN,      1, FLD_BOTH,   YES, NULL, "aclif/edit/2"	},
  { 17, 14, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "User:"			},
  { 17, 20, FLD_ALPHA,   8, FLD_BOTH,   YES, NULL, "aclif/edit/3"	},
  { 17, 33, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Group:"			},
  { 17, 40, FLD_ALPHA,   8, FLD_BOTH,   YES, NULL, "aclif/edit/4"	},
};

struct scrn_struct	edit_struct[] = {

    {	NULL,	0,	0,	1,	edit_validate },
    {	NULL,	0,	0,	3,	userif_zero },
    {	NULL,	0,	0,	5,	user_validate },
    {	NULL,	0,	0,	7,	group_validate },
};
#define	EDITSCROLL	0
#define	EDITCHECK	1
#define	EDITUSER	2
#define	EDITGROUP	3
#define	EDITNSTRUCT	4

struct scrn_parms	edit_parms = {

	SCR_FILLIN,		/* scrn_type  */
	1, 9, 21, 62,		/* top, left, rows, cols */
	NUMDESCS(edit_desc),	/* number of scrn_descs */
	edit_desc,		/* table of descs */
};

/******************************************************************************/

/*
 * "List of Synonyms" Display Screen
 */

struct scrn_desc	synl_desc[] = {

/* ROW COL  TYPE       LEN  INOUT       REQ  PROMPT */
  {  3, 31, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "List of Synonyms"		},
  {  6,  5, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Synonym File:"		},
  {  6, 19, FLD_ALPHA,  53, FLD_OUTPUT,  NO				},
  {  8,  5, FLD_SCROLL, 32, FLD_OUTPUT,  NO, NULL, NULL, 10, 4, 2	},
};

struct scrn_struct	synl_struct[] = {

    {	NULL,	0,	0,	2,	userif_zero },
    {	NULL,	0,	0,	3,	userif_zero },
};
#define	SYNLFILE	0
#define	SYNLSCROLL	1
#define	SYNLNSTRUCT	2

struct scrn_parms	synl_parms = {

	SCR_NOCHANGE,		/* scrn_type  */
	1, 1, 21, 78,		/* top, left, rows, cols */
	NUMDESCS(synl_desc),	/* number of scrn_descs */
	synl_desc,		/* table of descs */
};

/******************************************************************************/

/*
 * Main routine for ACL editor
 */

main (argc, argv)
int	argc;
char	*argv[];
{
	extern	struct	scrn_parms *TopScrn;

	set_auth_parameters (argc, argv);

	initscreen();

	traverse (TopScrn);

	restorescreen();

	exit (0);
}

/* go through the menu, displaying and doing appropriate things with
 * the choices.  On entry to this routine, interrupts are off.
 * returns:
 *	INTERRUPT
 *	QUIT
 *	HANGUP
 */

traverse (screenp)
struct	scrn_parms *screenp;
{
	struct	scrn_struct	*sp, *buildscrn_struct();
	WINDOW	*window;
	int	ret;	/* returned from setjmp */
	struct	scrn_ret	scrn_ret;	/* return from getscreen */
	struct	menu_scrn	*mp;
	int	choice;	/* for mapping choice to proper screen_desc */
	struct	scrn_desc	*sdp;
	int	i;
	extern	jmp_buf	env;

	do {
		/* display the screen. If a menuprompt, allocate memory for a
		 * screenp structure.
		 */
	
		window = (WINDOW *) 0;
		sp = (struct scrn_struct *) 0;

		/* signal catching routines record what happened */
		if (ret = setjmp (env))  {
			if (sp != (struct scrn_struct *) 0)
				freescrn_struct (sp);
			if (window != (WINDOW *) 0)
				rm_window (window);
			if (ret == CONTINUE) {
				ret = INTERRUPT;
				continue;
			} else
				return (ret);
		}

		sp = buildscrn_struct (screenp);
	
		window = putscreen (screenp, sp, POP);
	
		scrn_ret = getscreen (window, screenp, sp, 1);
	
		/* in case of interrupt, window needs to be unset */

		window = (WINDOW *) 0;

		if (scrn_ret.flags & R_ABORTED)  {
			freescrn_struct (sp);
			rm_window (window);
			return (ABORT);
		}
	
		if (scrn_ret.flags & R_QUIT) {
			freescrn_struct (sp);
			rm_window (window);
			return (QUIT);
		}
	
		/* normally, user will have picked a choice */
	
		if (!(scrn_ret.flags & R_EXECUTE))  {  /* something wrong */
			restorescreen();
			fprintf (stderr,
			  "Returned from getscreen without choosing\n");
			fprintf (stderr, "item %d, changed %s\n",
				scrn_ret.item,
				(scrn_ret.flags & R_CHANGED) ? "yes" : "no");
			exit (1);
		}
	
		/* getscreen returns the scrn_desc index. convert to
		 * choice number
		 */
		sdp = screenp->sd;
		choice = 0;
		for (i = 0; i < screenp->ndescs; i++)  {
			if (sdp->type == FLD_CHOICE)  {
				choice++;
				if (i == scrn_ret.item)
					break;
			}
			sdp++;
		}

		/* find the menu description assoc. with the choice */
		for (mp = screenp->ms; mp->choice != choice; mp++)
			;
	
		/* call a routine, run a program, or recurse to another menu */
		switch (mp->type) {
		case	M_MENU:
			/* turn off signals until next level traverse can
			 * set up setjmp environment
			 */
			(void) signal (SIGINT, SIG_IGN);
			(void) signal (SIGQUIT, SIG_IGN);
			ret = traverse (mp->next_menu);
			break;
		case	M_PROGRAM:
			/* call with sdp + 1 to get next field after
			 * choice
			 */
			ret = run_program (mp->next_program,
					   choice,
					   mp->nparams,
					   sdp + 1);
			break;
		case	M_ROUTINE:
			/* call with sdp + 1 to get next field after
			 * choice
			 */
			ret = call_routine (mp->next_routine,
					    choice,
					    mp->nparams,
					    sdp + 1);
			break;
		}
		freescrn_struct (sp);
	} while (ret == INTERRUPT);

	return (ret);
}

/* run a program with an argument as listed on the menuprompt lines */

run_program (program, choice, nparams, sd)
char	*program;		/* program to run */
int	choice;			/* which choice was picked */
int	nparams;		/* parameters to call it with */
struct	scrn_desc	*sd;	/* where to get params from */
{
	char	**argv, **make_arglist();
	int	ret;

	argv = make_arglist (sd, choice, nparams, program);

	ret = print_output (program, argv);

	free_arglist (argv, nparams);
	return (ret);
}

/* call a routine and pass it a list of parameters */

call_routine (routine, choice, nparams, sd)
int	(*routine)();
int	choice;
int	nparams;
struct	scrn_desc	*sd;
{
	char	**argv, **make_arglist();
	int	ret;

	argv = make_arglist (sd, choice, nparams, NULL);
	/* first argument is program, which is NULL in this case */
	ret = (*routine) (&argv[1]);
	free_arglist (argv, nparams);
	return (ret);
}
	
/* make an argument list from a menu prompt list */
char **
make_arglist (sd, choice, nparams, program)
struct	scrn_desc	*sd;
int	choice;
int	nparams;
char	*program;
{
	char	**argv;
	int	i;
	char	*cp, *malloc();
	char	numbuf[11];	/* can't have bigger than this in 32 bits */
	static	char yes[2] = "Y";
	static	char no[2] = "N";

	/* allocate string table for args to program */
	argv = (char **) malloc ((nparams + 3) * sizeof (char *));
	argv[0] = program;
	sprintf (numbuf, "%d", choice);
	argv[1] = malloc (strlen (numbuf) + 1);
	strcpy (argv[1], numbuf);

	for (i = 0; i < nparams; i++, sd++) {
		switch (sd->type) {
		case	FLD_ALPHA:
			cp = s_alpha(sd->scrnstruct);
			break;
		case	FLD_NUMBER:
			sprintf (numbuf, "%ld", *s_number(sd->scrnstruct));
			cp = numbuf;
			break;
		case	FLD_YN:
			if (*s_yesno(sd->scrnstruct))
				cp = yes;
			else	cp = no;
			break;
		}
		argv[i + 2] = malloc (strlen (cp) + 1);
		strcpy (argv[i + 2], cp);
	}
	argv[nparams + 2] = NULL;
	return (argv);
}

/* free the argument list */
free_arglist (argv, nparams)
char	**argv;
int	nparams;
{
	int	i;

	/* first argument is program, which is pointed to, not malloced */
	for (i = 1; i < nparams + 2; i++)
		free (argv[i]);
	free ((char *) argv);
	return;
}

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
	int	i;
	int	nstructs, bytes;
	char	*space, *malloc(), *calloc();
	char	*cp;

	if (screenp->scrntype == SCR_MENU)
		return (NULL);

	nstructs = 0;
	bytes = 0;
	/* count up the number of entries and the total memory requirements */
	for (i = 0, sd = screenp->sd; i < screenp->ndescs; i++, sd++) {
		/* assume no scrolling regions in menuprompt screen */
		switch (sd->type) {
		case	FLD_ALPHA:
			bytes += sd->len + 1;
			nstructs++;
			break;
	 	case	FLD_NUMBER:
			bytes += sizeof (long);
			nstructs++;
			break;
		case	FLD_POPUP:
		case	FLD_YN:
		case	FLD_CONFIRM:
			bytes++;
			nstructs++;
			break;
		case	FLD_CHOICE:
		case	FLD_PROMPT:
			break;
		}
	}

	/* allocate the table of scrn_structs and the memory to hold the
	 * values.  check success of malloc().
	 */
	retsp = (struct scrn_struct *) calloc (nstructs,
					    sizeof (struct scrn_struct));
	space = malloc (bytes);
	if (retsp == NULL || space == NULL) {
		if (space == NULL)
			free (retsp);
		return (NULL);
	}

	/* zero out memory returned from malloc */
	for (i = 0, cp = space; i < bytes; i++)
		*cp++ = '\0';

	for (i = 0, sd = screenp->sd, sp = retsp;
	     i < screenp->ndescs;
	     i++, sd++) {
		switch (sd->type) {
		case	FLD_CHOICE:
		case	FLD_PROMPT:
			break;
		case	FLD_ALPHA:
			sp->pointer = space;
			sp->filled = 0;
			sp->desc = i;
			space += sd->len + 1;
			sp++;
			break;
		case	FLD_NUMBER:
			sp->pointer = space;
			sp->filled = 0;
			sp->desc = i;
			space += sizeof (long);
			sp++;
			break;
		case	FLD_POPUP:
		case	FLD_YN:
		case	FLD_CONFIRM:
			sp->pointer = space;
			sp->filled = 0;
			sp->desc = i;
			space += sizeof (uchar);
			sp++;
			break;
		}
	}
	return (retsp);
}

freescrn_struct (sp)
struct	scrn_struct	*sp;
{
	if (sp)  {
		free (sp->pointer);
		free (sp);
	}
	return;
}

/******************************************************************************/

/*
 * Auxilliary functions required by the screen template routine.
 * These functions just return pointers to the basic data
 * structures declared above.
 */

acl_bfill(fillp)
	struct acl_fillin	*fillp;
{
	return 0;
}

struct scrn_parms *
acl_copyscreen(fillp, parmp)
	struct acl_fillin	*fillp;
	struct scrn_parms	*parmp;
{
	return parmp;
}

struct scrn_desc *
acl_bdesc(fillp, descp, structp)
	struct acl_fillin	*fillp;
	struct scrn_desc	*descp;
	struct scrn_struct	*structp;
{
	return descp;
}

/******************************************************************************/

/*
 * Functions to initialize the fillin screen data structures.
 */

/*
 * Initialize for loadacl and applyacl
 */
struct scrn_struct *
loadacl_bstruct(fillp, sp)
	struct acl_fillin	*fillp;
	struct scrn_struct	*sp;
{
	sp[FILENAME].pointer = file_buf;
	sp[FILENAME].changed = 1;
	sp[FILENAME].filled = 1;
	strncpy(file_buf, cur_file, sizeof file_buf - 1);

	return sp;
}

/*
 * Initialize for loadsyn and definesyn
 */
struct scrn_struct *
loadsyn_bstruct(fillp, sp)
	struct acl_fillin	*fillp;
	struct scrn_struct	*sp;
{
	acl_load_syns(NULL);

	sp[SYNNAME].pointer = syn_buf;
	sp[SYNNAME].changed = 1;
	sp[SYNNAME].filled = 1;
	strncpy(syn_buf, cur_syn, sizeof syn_buf - 1);

	return sp;
}

/*
 * Initialize for editacl
 */
struct scrn_struct *
editacl_bstruct(fillp, sp)
	struct acl_fillin	*fillp;
	struct scrn_struct	*sp;
{
	struct passwd	*pw;
	struct group	*gr;

	/*
	 * Make sure buffer is allocated and
	 * that there is room for expansion.
	 */
	if (alloc_acl_buf(acl_ents) == ERR)
		return NULL;

	sp[EDITSCROLL].pointer = (char *)acl_scroll;
	sp[EDITSCROLL].changed = 0;
	sp[EDITSCROLL].filled = acl_size;
	convert_ir2er();

	/* Make NO the default choice for the edit check field */
	sp[EDITCHECK].pointer = (char *)&edit_check;
	sp[EDITCHECK].changed = 0;
	sp[EDITCHECK].filled = 1;
	edit_check = 0;

	sp[EDITUSER].pointer = unam_buf;
	sp[EDITUSER].changed = 0;
	sp[EDITUSER].filled = 0;
	if (cur_unam[0] == '\0' && (pw = getpwuid(getuid()))) {
		strncpy(cur_unam, pw->pw_name, sizeof cur_unam - 1);
		cur_uid = pw->pw_uid;
	}
	if (cur_unam[0]) {
		strncpy(unam_buf, cur_unam, sizeof unam_buf - 1);
		sp[EDITUSER].filled = 1;
	} else
		strncpy(unam_buf, "", sizeof unam_buf - 1);

	sp[EDITGROUP].pointer = gnam_buf;
	sp[EDITGROUP].changed = 0;
	sp[EDITGROUP].filled = 0;
	if (cur_gnam[0] == '\0' && (gr = getgrgid(getgid()))) {
		strncpy(cur_gnam, gr->gr_name, sizeof cur_gnam - 1);
		cur_gid = gr->gr_gid;
	}
	if (cur_gnam[0]) {
		strncpy(gnam_buf, cur_gnam, sizeof gnam_buf - 1);
		sp[EDITGROUP].filled = 1;
	} else
		strncpy(gnam_buf, "", sizeof gnam_buf - 1);

	return sp;
}

/*
 * Initialize for deleteacl
 */
struct scrn_struct *
deleteacl_bstruct(fillp, sp)
	struct acl_fillin	*fillp;
	struct scrn_struct	*sp;
{
	sp[FILENAME].pointer = file_buf;
	sp[FILENAME].changed = 0;
	sp[FILENAME].filled = 0;
	strncpy(file_buf, "", sizeof file_buf - 1);

	return sp;
}

/*
 * Initialize for deletesyn
 */
struct scrn_struct *
deletesyn_bstruct(fillp, sp)
	struct acl_fillin	*fillp;
	struct scrn_struct	*sp;
{
	acl_load_syns(NULL);

	sp[SYNNAME].pointer = syn_buf;
	sp[SYNNAME].changed = 0;
	sp[SYNNAME].filled = 0;
	strncpy(syn_buf, "", sizeof syn_buf - 1);

	return sp;
}

/*
 * Initialize for testaccess
 */
struct scrn_struct *
testacc_bstruct(fillp, sp)
	struct acl_fillin	*fillp;
	struct scrn_struct	*sp;
{
	struct passwd	*pw;
	struct group	*gr;

	sp[TESTUSER].pointer = unam_buf;
	sp[TESTUSER].changed = 0;
	sp[TESTUSER].filled = 0;
	if (cur_unam[0] == '\0' && (pw = getpwuid(getuid())))
		strncpy(cur_unam, pw->pw_name, sizeof cur_unam - 1);
	if (cur_unam[0]) {
		strncpy(unam_buf, cur_unam, sizeof unam_buf - 1);
		sp[TESTUSER].filled = 1;
	} else
		strncpy(unam_buf, "", sizeof unam_buf - 1);

	sp[TESTGROUP].pointer = gnam_buf;
	sp[TESTGROUP].changed = 0;
	sp[TESTGROUP].filled = 0;
	if (cur_gnam[0] == '\0' && (gr = getgrgid(getgid())))
		strncpy(cur_gnam, gr->gr_name, sizeof cur_gnam - 1);
	if (cur_gnam[0]) {
		strncpy(gnam_buf, cur_gnam, sizeof gnam_buf - 1);
		sp[TESTGROUP].filled = 1;
	} else
		strncpy(gnam_buf, "", sizeof gnam_buf - 1);

	sp[TESTFILE].pointer = file_buf;
	sp[TESTFILE].changed = 1;
	sp[TESTFILE].filled = 1;
	strncpy(file_buf, cur_file, sizeof file_buf - 1);

	return sp;
}

/*
 * Initialize for listsyns
 */
struct scrn_struct *
listsyns_bstruct(fillp, stp)
	struct acl_fillin	*fillp;
	struct scrn_struct	*stp;
{
	register struct aclsyn_t	*sp;
	register int			i;

	if (acl_load_syns(NULL) == ACL_ERR) {
		pop_msg("No ACL synonyms are defined.", "");
		return NULL;
	}
	stp[SYNLFILE].pointer = acl_dbfile ? acl_dbfile : "";
	stp[SYNLFILE].changed = 0;
	stp[SYNLFILE].filled = 1;

	/*
	 * Make sure scrolling buffer is allocated and has enough space
	 */
	if (syn_scroll == NULL)
		syn_scroll = alloc_cw_table(acl_syns.syn_count, 33);
	else if (acl_syns.syn_count > syn_size)
		syn_scroll = expand_cw_table(syn_scroll, syn_size,
				acl_syns.syn_count, 33);
	if (syn_scroll == NULL)
		return NULL;

	syn_size = acl_syns.syn_count;
	sp = acl_syns.syn_next;

	for (i = 0; i < syn_size; ++i) {
		if (sp)
			strcpy(syn_scroll[i], sp->syn_name);
		else
			break;
		sp = sp->syn_next;
	}
	if (i != syn_size) {
		pop_msg("Inconsistency in synonym database.",
			"Please report error and re-run program.");
		return NULL;
	}

	stp[SYNLSCROLL].pointer = (char *)syn_scroll;
	stp[SYNLSCROLL].changed = 0;
	stp[SYNLSCROLL].filled = syn_size;

	return stp;
}

/******************************************************************************/

/*
 * Screen validation routines
 */

/*
 * Check that the ACL buffer has valid contents before allowing
 * it to be applied to a file or used to define a synonym.
 */
acl_validate(argv, fillp)
	char			*argv[];
	struct acl_fillin	*fillp;
{
	if (acl_invalid) {
		pop_msg("The ACL buffer is invalid.",
			"Please create or load a valid ACL first.");
		return 1;
	}
	return 0;
}

/******************************************************************************/

/*
 * Field validation routines
 */

/*
 * Validate the ACL scroll field in the edit screen
 */
edit_validate(fillp)
	struct acl_fillin	*fillp;
{
	register acle_t		*ep;
	register int		i;
	int			ents;

	acl_ents = 0;
	for (i = 0; i < acl_size && acl_scroll[i][0]; ++i) {
		/*
		 * We are about to replace the contents of acl_buf
		 * with the converted contents of the edit scrolling
		 * region.  Mark acl_buf invalid until we succeed.
		 */
		acl_invalid = YES;
		ep = acl_er_to_ir(acl_scroll[i], &ents);
		if (ep == NULL) {
			pop_msg("The following entry is invalid:",
				acl_scroll[i]);
			acl_ents = 0;
			return 1;
		}
		if (alloc_acl_buf(acl_ents + ents) != OK) {
			acl_ents = 0;
			return 1;
		}
		memcpy((char *)&acl_buf[acl_ents], (char *)ep,
			ents * sizeof(acle_t));
		acl_ents += ents;
	}
	acl_invalid = NO;
	return 0;
}

/*
 * Validate the user name field in the test access screen
 */
user_validate(fillp)
	struct acl_fillin	*fillp;
{
	struct passwd	*pw;

	if ((pw = getpwnam(unam_buf)) == NULL) {
		pop_msg("The user name entered is undefined.",
			"Please enter a valid user name.");
		return 1;
	}
	strncpy(cur_unam, unam_buf, sizeof cur_unam - 1);
	cur_uid = pw->pw_uid;

	return 0;
}

/*
 * Validate the group name field in the test access screen
 */
group_validate(fillp)
	struct acl_fillin	*fillp;
{
	struct group	*gr;

	if ((gr = getgrnam(gnam_buf)) == NULL) {
		pop_msg("The group name entered is undefined.",
			"Please enter a valid group name.");
		return 1;
	}
	strncpy(cur_gnam, gnam_buf, sizeof cur_gnam - 1);
	cur_gid = gr->gr_gid;

	return 0;
}

/*
 * Validate the file name field in the test access screen
 */
file_validate(fillp)
	struct acl_fillin	*fillp;
{
	if (stat(file_buf, &cur_stb) < 0) {
		pop_msg("The specified file is inaccessible:",
			sys_errlist[errno]);
		return 1;
	}
	strncpy(cur_file, file_buf, sizeof cur_file - 1);
	return 0;
}

/******************************************************************************/

/*
 * Action routines for fillin screens and
 * screenless top level menu choices.
 */

/*
 * Fetch a file's ACL into the ACL buffer
 */
do_loadacl(fillp)
	struct acl_fillin	*fillp;
{
	int	ents;
	char	err[128];

	while ((ents = statacl(file_buf, acl_buf, acl_size)) > acl_size)
		if (alloc_acl_buf(ents) == ERR) {
			acl_ents = acl_size;
			return 1;
		}

	if (ents < 0) {
		sprintf(err, "Cannot load ACL from %s.", file_buf);
		pop_msg(err, (errno != EINVAL) ? sys_errlist[errno] :
			"File does not have an ACL.");
	} else {
		acl_ents = ents;
		strncpy(cur_file, file_buf, sizeof cur_file - 1);
		acl_invalid = NO;
	}
	return 1;
}

/*
 * Load the ACL buffer with a synonym
 */
do_loadsyn(fillp)
	struct acl_fillin	*fillp;
{
	acle_t	*ep;
	int	ents;

	ep = acl_er_to_ir(syn_buf, &ents);
	if (ep == NULL)
		pop_msg(syn_buf, "Undefined synonym.");
	else if (alloc_acl_buf(ents) == OK) {
		memcpy((char *)acl_buf, (char *)ep, ents * sizeof(acle_t));
		acl_ents = ents;
		strncpy(cur_syn, syn_buf, sizeof cur_syn - 1);
		acl_invalid = NO;
	}
	return 1;
}

/*
 * See if user wants to test access upon execution of edit screen
 */
do_editacl(fillp)
	struct acl_fillin	*fillp;
{
	int	perm;
	char	msg1[81], msg2[81];
	static char	*plurals[] = { "s", "", "", "s", "", "s", "s", "s" };

	if (edit_check) {
		perm = acl_access(cur_uid, cur_gid, cur_uid, cur_gid,
					acl_buf, acl_ents);
		sprintf(msg1, "This ACL gives %s.%s", cur_unam, cur_gnam);
		sprintf(msg2, "%s permission%s.", perm_strings[perm & 7],
				plurals[perm & 7]);
		pop_msg(msg1, msg2);
	}
	return 1;
}

/*
 * Clear the ACL buffer (called directly from top level screen)
 */
clearbuf(argv)
	char	*argv[];
{
	register int	i;

	acl_ents = 0;
	if (acl_scroll)
		for (i = 0; i < acl_size; ++i)
			acl_scroll[i][0] = '\0';
	acl_invalid = NO;
	return 1;
}

/*
 * Apply the current ACL to a file
 */
do_applyacl(fillp)
	struct acl_fillin	*fillp;
{
	char	err[128];

	if (chacl(file_buf, acl_buf, acl_ents) < 0) {
		sprintf(err, "Cannot apply ACL to %s.", file_buf);
		pop_msg(err, sys_errlist[errno]);
	}
	return 1;
}
	
/*
 * Remove the ACL from a file
 */
do_deleteacl(fillp)
	struct acl_fillin	*fillp;
{
	char	err[128];

	if (chacl(file_buf, ACL_DELETE, -1) < 0) {
		sprintf(err, "Cannot remove ACL from %s.", file_buf);
		pop_msg(err, sys_errlist[errno]);
	}
	return 1;
}

/*
 * Define an ACL synonym
 */
do_definesyn(fillp)
	struct acl_fillin	*fillp;
{
	register aclsyn_t	*sp;

	sp = acl_lookup_syn(syn_buf);
	if (sp) {	/* already defined */
		if (acl_expand_syn(sp, acl_ents - sp->syn_count) == ACL_OK) {
			if (acl_ents)
				memcpy((char *)sp->syn_ents, (char *)acl_buf,
					acl_ents * sizeof(acle_t));
			sp->syn_count = acl_ents;
			if (acl_store_syns(acl_dbfile) == ACL_ERR)
				pop_msg("Cannot write synonym database file.",
					"Change not saved permanently.");
		} else
			pop_msg("Cannot allocate memory for synonym.",
				"Please report error and re-run program.");
		return 1;
	}

	if ((sp = acl_alloc_syn(acl_ents))
			&& (sp->syn_name = malloc(strlen(syn_buf) + 1))) {
		sp->syn_count = acl_ents;
		strcpy(sp->syn_name, syn_buf);
		if (acl_ents)
			memcpy((char *)sp->syn_ents, (char *)acl_buf,
				acl_ents * sizeof(acle_t));
		acl_insert_syn(sp);
		if (acl_store_syns(acl_dbfile) == ACL_ERR)
			pop_msg("Cannot write synonym database file.",
				"Change not saved permanently.");
	} else {
		acl_free_syn(sp);
		pop_msg("Cannot allocate memory for synonym.",
			"Please report error and re-run program.");
	}
	return 1;
}

/*
 * Delete an ACL synonym
 */
do_deletesyn(fillp)
	struct acl_fillin	*fillp;
{
	if (acl_delete_syn(syn_buf) == ACL_ERR)
		pop_msg(syn_buf, "is not defined.");
	else if (acl_store_syns(acl_dbfile) == ACL_ERR)
		pop_msg("Cannot write synonym database file.",
			"Change not saved permanently.");
	return 1;
}

/*
 * Compute the access permissions of the specified user/group
 */
do_testaccess(fillp)
	struct acl_fillin	*fillp;
{
	int		acl_perm, perm_mask;
	char		msg1[81], msg2[81];

	acl_perm = acl_access(cur_uid, cur_gid, cur_stb.st_uid, cur_stb.st_gid,
				acl_buf, acl_ents);
	perm_mask = cur_stb.st_mode;
	if (cur_uid == cur_stb.st_uid)
		perm_mask >>= 6;
	else if (cur_gid == cur_stb.st_gid)
		perm_mask >>= 3;
	perm_mask &= 7;
	acl_perm &= perm_mask;
	sprintf(msg1, "With current ACL, %s.%s would have %s",
		cur_unam, cur_gnam, perm_strings[acl_perm]);
	sprintf(msg2, "access to %s.", cur_file);
	pop_msg(msg1, msg2);
	return 1;
}

/******************************************************************************/

/*
 * Utility routines
 */

/*
 * Allocate (or expand) space for the ACL buffer.
 * Allocate for both the internal representation and for the
 * scrolling region that will hold the external representation.
 */
alloc_acl_buf(ents)
	int	ents;
{
	acle_t	*newir;
	char	**newer;
	int	newsize;

	if (ents + ACL_MARGIN <= acl_size)
		return OK;
	
	/*
	 * Allocate in multiples of ACL_CHUNK, ensuring that
	 * there is room for at least ACL_MARGIN more entries
	 * than requested.
	 */
	newsize = (ents + ACL_MARGIN + ACL_CHUNK - 1) / ACL_CHUNK;
	newsize *= ACL_CHUNK;

	if (acl_buf) {
		newir = (acle_t *)realloc((char *)acl_buf,
					newsize * sizeof(acle_t));
		newer = expand_cw_table(acl_scroll, acl_size, newsize, 25);
	} else {
		newir = (acle_t *)malloc(newsize * sizeof(acle_t));
		newer = alloc_cw_table(newsize, 25);
	}

	if (newir == NULL || newer == NULL) {
		pop_msg("Cannot allocate memory for ACL buffer.",
			"Please report error and re-run program.");
		return ERR;
	}

	acl_buf = newir;
	acl_size = newsize;
	acl_scroll = newer;
	return OK;
}

/*
 * Simulate the ACL permission check mechanism
 */
acl_access(uid, gid, own_uid, own_gid, acl, ents)
	ushort		uid, gid, own_uid, own_gid;
	register acle_t	*acl;
	register int	ents;
{
	for (; ents > 0; --ents, ++acl) {
		if (acl->acl_uid == ACL_OWNER) {
			if (uid != own_uid)
				continue;
		} else if (acl->acl_uid != ACL_WILDCARD && acl->acl_uid != uid)
			continue;
		if (acl->acl_gid == ACL_OWNER) {
			if (gid != own_gid)
				continue;
		} else if (acl->acl_gid != ACL_WILDCARD && acl->acl_gid != gid)
			continue;
		return acl->acl_perm;
	}
	return 0;
}

/*
 * Convert the internal representations in acl_buf
 * to external representations in the edit scroll region.
 */
convert_ir2er()
{
	register int	i;
	register acle_t	*ep;
	for (i = 0, ep = acl_buf; i < acl_size; ++i, ++ep)
		if (i >= acl_ents)
			acl_scroll[i][0] = '\0';
		else
			acl_1ir_to_er(ep, acl_scroll[i]);
}

/*******************************************************************************
 * Fillin screen functions, automatically generated
 * from the template routine.
 ******************************************************************************/

/*
 * File name fillin screen for loading ACL buffer from a file
 */
#define	SCRNFUNC	loadacl
#define	SCREENACTION	do_loadacl
#define	BUILDSTRUCT	loadacl_bstruct
#define	FIRSTDESC	FILENAME
#define	NSCRNSTRUCT	FILENSTRUCT
#define	PARMTEMPLATE	file_parms
#define	DESCTEMPLATE	file_desc
#define	STRUCTTEMPLATE	file_struct
#define	FILLINSTRUCT	acl_fillin
#define	VALIDATE	userif_zero
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

/*
 * Synonym name fillin screen for loading ACL buffer from a synonym
 */
#define	SCRNFUNC	loadsyn
#define	SCREENACTION	do_loadsyn
#define	BUILDSTRUCT	loadsyn_bstruct
#define	FIRSTDESC	SYNNAME
#define	NSCRNSTRUCT	SYNNSTRUCT
#define	PARMTEMPLATE	syn_parms
#define	DESCTEMPLATE	syn_desc
#define	STRUCTTEMPLATE	syn_struct
#define	FILLINSTRUCT	acl_fillin
#define	VALIDATE	userif_zero
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

/*
 * ACL fillin screen for editing ACL buffer
 */
#define	SCRNFUNC	editacl
#define	SCREENACTION	do_editacl
#define	BUILDSTRUCT	editacl_bstruct
#define	FIRSTDESC	EDITSCROLL
#define	NSCRNSTRUCT	EDITNSTRUCT
#define	PARMTEMPLATE	edit_parms
#define	DESCTEMPLATE	edit_desc
#define	STRUCTTEMPLATE	edit_struct
#define	FILLINSTRUCT	acl_fillin
#define	VALIDATE	userif_zero
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

/*
 * File name fillin screen for applying ACL buffer to a file
 */
#define	SCRNFUNC	applyacl
#define	SCREENACTION	do_applyacl
#define	BUILDSTRUCT	loadacl_bstruct
#define	VALIDATE	acl_validate
#define	FIRSTDESC	FILENAME
#define	NSCRNSTRUCT	FILENSTRUCT
#define	PARMTEMPLATE	file_parms
#define	DESCTEMPLATE	file_desc
#define	STRUCTTEMPLATE	file_struct
#define	FILLINSTRUCT	acl_fillin
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

/*
 * File name fillin screen for deleting ACL from a file
 */
#define	SCRNFUNC	deleteacl
#define	SCREENACTION	do_deleteacl
#define	BUILDSTRUCT	deleteacl_bstruct
#define	FIRSTDESC	FILENAME
#define	NSCRNSTRUCT	FILENSTRUCT
#define	PARMTEMPLATE	file_parms
#define	DESCTEMPLATE	file_desc
#define	STRUCTTEMPLATE	file_struct
#define	FILLINSTRUCT	acl_fillin
#define	VALIDATE	userif_zero
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

/*
 * Synonym name fillin screen for defining an ACL synonym
 */
#define	SCRNFUNC	definesyn
#define	SCREENACTION	do_definesyn
#define	BUILDSTRUCT	loadsyn_bstruct
#define	VALIDATE	acl_validate
#define	FIRSTDESC	SYNNAME
#define	NSCRNSTRUCT	SYNNSTRUCT
#define	PARMTEMPLATE	syn_parms
#define	DESCTEMPLATE	syn_desc
#define	STRUCTTEMPLATE	syn_struct
#define	FILLINSTRUCT	acl_fillin
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

/*
 * Synonym name fillin screen for deleting an ACL synonym
 */
#define	SCRNFUNC	deletesyn
#define	SCREENACTION	do_deletesyn
#define	BUILDSTRUCT	deletesyn_bstruct
#define	FIRSTDESC	SYNNAME
#define	NSCRNSTRUCT	SYNNSTRUCT
#define	PARMTEMPLATE	syn_parms
#define	DESCTEMPLATE	syn_desc
#define	STRUCTTEMPLATE	syn_struct
#define	FILLINSTRUCT	acl_fillin
#define	VALIDATE	userif_zero
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

/*
 * User/Group name fillin screen for computing access permissions
 */
#define	SCRNFUNC	testaccess
#define	SCREENACTION	do_testaccess
#define	BUILDSTRUCT	testacc_bstruct
#define	FIRSTDESC	TESTUSER
#define	NSCRNSTRUCT	TESTNSTRUCT
#define	PARMTEMPLATE	test_parms
#define	DESCTEMPLATE	test_desc
#define	STRUCTTEMPLATE	test_struct
#define	FILLINSTRUCT	acl_fillin
#define	VALIDATE	userif_zero
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

/*
 * Synonym list display screen
 */
#define	SCRNFUNC	listsyns
#define	SCREENACTION	userif_one
#define	BUILDSTRUCT	listsyns_bstruct
#define	FIRSTDESC	SYNLSCROLL
#define	NSCRNSTRUCT	SYNLNSTRUCT
#define	PARMTEMPLATE	synl_parms
#define	DESCTEMPLATE	synl_desc
#define	STRUCTTEMPLATE	synl_struct
#define	FILLINSTRUCT	acl_fillin
#define	VALIDATE	userif_zero
#define	BUILDFILLIN	acl_bfill
#define	COPYSCREEN	acl_copyscreen
#define	BUILDDESC	acl_bdesc
#define	FREETABS	userif_void

#include "stemplate.c"

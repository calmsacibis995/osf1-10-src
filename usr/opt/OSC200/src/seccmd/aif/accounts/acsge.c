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
static char	*sccsid = "@(#)$RCSfile: acsge.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:45 $";
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
 *  Account Select Group (Extant) routines
 */

#include	<sys/stat.h>
#include	<sys/param.h>
#include	<stdio.h>
#include	<pwd.h>
#include	<grp.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/security.h>
#include	<sys/audit.h>
#include	<prot.h>
#include	"userif.h"
#include	"UIMain.h"
#include	"Utils.h"
#include	"valid.h"
#include	"logging.h"


/* routine definitions for multiply used routines */

static int acsge_auth();
static int acsge_bfill();
static int canaccgr();
static int valid_it();
static int save_it();

static int acsge_radio();


extern Scrn_parms	acsge_scrn;
extern Scrn_desc	acsge_desc[];
Scrn_struct		*acsge_struct;



/* Template for fill-in screen */

static struct	grp_fillin {
	char	**groups;
	int	ngroups;
	char	group[GNAMELEN + 1];
	char	*state;
	char	*old_state;
	int	nstructs;
} Scr_grp;


static struct grp_fillin *scr_grp = &Scr_grp;

/* offsets for scrn_struct */

#define	GROUPNAMES 	0
#define NGROUPS		1
#define GROUPNAME	2
#define PWNSCRNSTRUCT		5

#define PARMTEMPLATE	acsge_scrn
#define STRUCTTEMPLATE	acsge_struct
#define DESCTEMPLATE	acsge_desc
#define FILLINSTRUCT	grp_fillin
#define FILLIN		scr_grp

#define FIRSTDESC	GROUPNAMES
#define NSCRNSTRUCT	1


/* global routines used everywhere */

extern struct	group	*getgrnam(), *getgrent(), *getgrgid();

/*
 *  designate a group - argv[1] could be the group name or id
 */


/* see if authorized to do this
 * returns 0 if group can be added, 1 if not
 */

static int
acsge_auth (argv, gf)
char	**argv;
struct  grp_fillin	*gf;
{
	return (canaccgr (argv, gf) || acsge_bfill (gf));
}


/* returns 0 if group file can be read, 1 if not */

static int
canaccgr (argv, gf)
char	**argv;
struct	grp_fillin	*gf;
{
	extern	char	groupfn[];		/* group file name */

	ENTERFUNC("canaccgr");
	if (eaccess (groupfn, 1) != 0) {  /* need write and search */
		pop_msg ("This program cannot read the group file.",
"Please check the permissions of the group file's parent directory.");
		ERRFUNC ("canaccgr", "group access");
		return (1);
	}
	EXITFUNC("canaccgr");
	return (0);
}




/* fill in a new gf structure with empty entries */
static
int
acsge_bfill (gf)
register struct	grp_fillin	*gf;
{
	register int i;

	ENTERFUNC("acsge_bfill");
	gf->groups = NULL;
	gf->ngroups = 0L;
	memset (gf->group, '\0', sizeof (gf->group));
	GetAllGroups (&gf->ngroups, &gf->groups);
	if (!(gf->state = Calloc (gf->ngroups, sizeof (gf->group))))
		MemoryError ();	/* Dies */
	if (!(gf->old_state = Calloc (gf->ngroups, sizeof (gf->group))))
		MemoryError ();	/* Dies */
	EXITFUNC("acsge_bfill");
	return (0);
}




/*
 * fill in scrn_struct structure for the screen
 */

static
int
acsge_bstruct (gf, sptemplate)
struct	grp_fillin	*gf;
struct	scrn_struct	**sptemplate;
{
	register struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("acsge_bstruct");
	gf->nstructs = PWNSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp != (struct scrn_struct *) 0) {
		DUMPDECP (" sp being filled in", 0, 0, 0);
		sp[GROUPNAMES].pointer = (char *) gf->groups;
		sp[GROUPNAMES].filled = gf->ngroups;
		sp[GROUPNAMES].state = gf->state;
		sp[GROUPNAMES].val_act = acsge_radio;
	}
	EXITFUNC("acsge_bstruct");
	return (!sp);
}



static
int
acsge_radio ()
{
	register char *cp, *last, *op;

	cp = PARMTEMPLATE.ss[GROUPNAMES].state;
	last = cp + PARMTEMPLATE.ss[GROUPNAMES].filled;
	for (op = scr_grp->old_state; cp < last; cp++, op++)
		if (*cp)
			if (*op)
				*cp = *op = 0;
			else 
				*op = 1;
		else
			if (*op)
				*op = 0;
	return 1;
}



static
int
valid_it (argv, gf)
char **argv;
struct	grp_fillin	*gf;
{
	register char *first, *cp, *last;

	ENTERFUNC("valid_it");
	first = gf->state;
	last = first + gf->ngroups;
	for (cp = first; cp < last; cp++)
		if (*cp) {
			strcpy (gf->group, gf->groups[cp - first]);
			break;
	}
	DUMPDETI (" name=<%s>", gf->group, 0, 0);
	EXITFUNC("valid_it");
	return 0;
}


/*
 * save the chosen groupname, and it's GID, in the
 * global vars. don't bother if it's the same as the
 * current ones.
 */

static
int
save_it (gf)
struct	grp_fillin	*gf;
{
	struct group grp;

	ENTERFUNC("save_it");
	if (strcmp ((char *)gl_group, gf->group)) {
		strcpy ((char *)gl_group, gf->group);
		if (GetPWUserByName((char *)gl_group, &grp))
			gl_gid = grp.gr_gid;
		else
			gl_gid = BOGUS_ID;
	}
	EXITFUNC("save_it");
	return 0;
}



/* free malloc'd memory */

static int
do_free (argv, gf, nstructs, pp, dp, sp)
char	**argv;
struct	grp_fillin	*gf;
int	nstructs;
Scrn_parms	*pp;
Scrn_desc	*dp;
Scrn_struct	*sp;
{
	if (gf->groups)
		free_cw_table (gf->groups);
	Free (gf->state);
	Free (gf->old_state);
	return (1);
}

#define	SETUPFUNC	acsge_setup
#define	AUTHFUNC	acsge_auth
#define BUILDFILLIN	acsge_bfill

#define	INITFUNC	acsge_init
#define BUILDSTRUCT	acsge_bstruct

#define	ROUTFUNC	acsge_exit
#define VALIDATE	valid_it
#define	SCREENACTION	save_it

#define FREEFUNC	acsge_free
#define FREESTRUCT	do_free


#include "stemplate.c"


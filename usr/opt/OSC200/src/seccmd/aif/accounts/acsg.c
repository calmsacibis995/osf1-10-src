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
static char	*sccsid = "@(#)$RCSfile: acsg.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:42 $";
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
 *  Account Select Group routines
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
#include	"valid.h"
#include	"logging.h"

/* routine definitions for multiply used routines */

static int acsg_auth();
static int acsg_bfill();
static int canaccgr();
static int valid_it();
static int save_it();


extern Scrn_parms	acsg_scrn;
extern Scrn_desc	acsg_desc[];
Scrn_struct		*acsg_struct;


int InvalidUser();

/* Template for fill-in screen */

static struct	grp_fillin {
	char	group[GNAMELEN + 1];
	long	gid;
	int	nstructs;
} Scr_grp;


static struct grp_fillin *scr_grp = &Scr_grp;

/* offsets for scrn_struct */

#define	GROUPNAME 	0
#define GROUPID		1
#define GRNSCRNSTRUCT	2

#define PARMTEMPLATE	acsg_scrn
#define STRUCTTEMPLATE	acsg_struct
#define DESCTEMPLATE	acsg_desc
#define FILLINSTRUCT	grp_fillin
#define FILLIN		scr_grp

#define FIRSTDESC	-1
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
acsg_auth (argv, grfill)
char	**argv;
struct  grp_fillin	*grfill;
{
	return (canaccgr (argv, grfill));
}


/* returns 0 if group file can be read, 1 if not */

static int
canaccgr (argv, grfill)
char	**argv;
struct	grp_fillin	*grfill;
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




/* fill in a new grfill structure with empty entries */
static
int
acsg_bfill (grfill)
register struct	grp_fillin	*grfill;
{
	register int i;

	ENTERFUNC("acsg_bfill");
	memset (grfill->group, '\0', sizeof (grfill->group));
	grfill->gid = 0L;
	EXITFUNC("acsg_bfill");
	return (0);
}




/*
 * fill in scrn_struct structure for the screen
 */

static
int
acsg_bstruct (grfill, sptemplate)
struct	grp_fillin	*grfill;
struct	scrn_struct	**sptemplate;
{
	register struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("acsg_bstruct");
	grfill->nstructs = GRNSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp != (struct scrn_struct *) 0) {
		DUMPDECP (" sp being filled in", 0, 0, 0);
		DUMPDETI (" name=<%s> id=<%dl>",
			&(grfill->group[0]), grfill->gid, NULL);
		sp[GROUPNAME].pointer = grfill->group;
		sp[GROUPNAME].validate = InvalidUser;
		sp[GROUPNAME].filled = 1;
	}
	EXITFUNC("acsg_bstruct");
	return (!sp);
}




static
int
valid_it (argv, grfill)
char **argv;
struct	grp_fillin	*grfill;
{
	return 0;
}


static
int
save_it (grfill)
struct	grp_fillin	*grfill;
{
	struct group grp;

	if (isdigit(grfill->group[0])) {
		gl_gid = atoi (grfill->group);
		if (GetGRGroupByGID(gl_gid, &grp))
			strcpy ((char *)gl_group, grp.gr_name);
		else
			gl_group[0] = NULL;
	} else {
		if (strcmp ((char *)gl_group, grfill->group)) {
			strcpy ((char *)gl_group, grfill->group);
			if (GetGRGroupByName((char *)gl_group, &grp))
				gl_gid = grp.gr_gid;
			else
				gl_gid = BOGUS_ID;
		}
	}
	return 0;
}



/* free malloc'd memory */

static int
do_free (argv, grfill, nstructs, pp, dp, sp)
char	**argv;
struct	grp_fillin	*grfill;
int	nstructs;
Scrn_parms	*pp;
Scrn_desc	*dp;
Scrn_struct	*sp;
{
	return (1);
}

#define	SETUPFUNC	acsg_setup
#define	AUTHFUNC	acsg_auth
#define BUILDFILLIN	acsg_bfill

#define	INITFUNC	acsg_init
#define BUILDSTRUCT	acsg_bstruct

#define	ROUTFUNC	acsg_exit
#define VALIDATE	valid_it
#define	SCREENACTION	save_it

#define FREEFUNC	acsg_free
#define FREESTRUCT	do_free


#include "stemplate.c"

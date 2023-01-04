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
static char	*sccsid = "@(#)$RCSfile: accue.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:47 $";
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
 *  Account Check User (Names, existing) routines
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

static int accue_auth();
static int accue_bfill();
static int canaccpw();
static int no_valid_needed();
static int dont_save();


extern Scrn_parms	accue_scrn;
extern Scrn_desc	accue_desc[];
Scrn_struct		*accue_struct;


/* Template for fill-in screen */

static struct	pwd_fillin {
	char	**users;
	int	nusers;
	char	user[UNAMELEN + 1];
	char	*state;
	int	nstructs;
} Scr_pwd;


static struct pwd_fillin *scr_pwd = &Scr_pwd;

/* offsets for scrn_struct */

#define	USERNAMES 	0

#define PARMTEMPLATE	accue_scrn
#define STRUCTTEMPLATE	accue_struct
#define DESCTEMPLATE	accue_desc
#define FILLINSTRUCT	pwd_fillin
#define FILLIN		scr_pwd

#define FIRSTDESC	-1
#define NSCRNSTRUCT	1

/* see if authorized to do this
 * returns 0 if user can be added, 1 if not
 */

static int
accue_auth (argv, pf)
char	**argv;
struct  pwd_fillin	*pf;
{
	return canaccpw(argv, pf);
}


/* returns 0 if password file can be read, 1 if not */

static int
canaccpw (argv, pf)
char	**argv;
struct	pwd_fillin	*pf;
{
	extern	char	passwd[];		/* password file name */

	ENTERFUNC("canaccpw");
	if (eaccess (passwd, 1) != 0) {  /* need write and search */
		pop_msg ("This program cannot read the password file.",
"Please check the permissions of the password file's parent directory.");
		ERRFUNC ("canaccpw", "passwd access");
		return (1);
	}
	EXITFUNC("canaccpw");
	return (0);
}




/* fill in a new pf structure with empty entries */
static
int
accue_bfill (pf)
register struct	pwd_fillin	*pf;
{
	register int i;

	ENTERFUNC("accue_bfill");
	pf->users = NULL;
	pf->nusers = 0L;
	memset(pf->user, '\0', sizeof(pf->user));
	GetAllUsers (&pf->nusers, &pf->users);
	if (!(pf->state = Calloc (pf->nusers, sizeof (char *))))
		MemoryError (); /* Dies */
	EXITFUNC("accue_bfill");
	return (0);
}




/*
 * fill in scrn_struct structure for the screen
 */

static
int
accue_bstruct (pf, sptemplate)
struct	pwd_fillin	*pf;
struct	scrn_struct	**sptemplate;
{
	register struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("accue_bstruct");
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp != (struct scrn_struct *) 0) {
		DUMPDECP (" sp being filled in", 0, 0, 0);
		sp[USERNAMES].pointer = (char *) pf->users;
		sp[USERNAMES].filled = pf->nusers;
		sp[USERNAMES].state = pf->state;
	}
	EXITFUNC("accue_bstruct");
	return (!sp);
}




/* free malloc'd memory */

static int
do_free (argv, pf, nstructs, pp, dp, sp)
char	**argv;
struct	pwd_fillin	*pf;
int	nstructs;
Scrn_parms	*pp;
Scrn_desc	*dp;
Scrn_struct	*sp;
{
	ENTERFUNC("do_free");
	if (pf->users)
		free_cw_table (pf->users);
	Free (pf->state);
	EXITFUNC("do_free");
	return (1);
}

#define	SETUPFUNC	accue_setup
#define	AUTHFUNC	accue_auth
#define BUILDFILLIN	accue_bfill

#define	INITFUNC	accue_init
#define BUILDSTRUCT	accue_bstruct

#define FREEFUNC	accue_free
#define FREESTRUCT	do_free


#include "stemplate.c"


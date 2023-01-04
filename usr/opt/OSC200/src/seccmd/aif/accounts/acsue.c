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
static char	*sccsid = "@(#)$RCSfile: acsue.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:51 $";
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
 *  Account Select User routines
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

static int acsue_auth();
static int acsue_bfill();
static int canaccpw();
static int acsue_valid();
static int acsue_save();

static int acsue_radio();	/* radio button action */

int acsue_flag;			/* whether acsue_ menu changed things */


extern Scrn_parms	acsue_scrn;
extern Scrn_desc	acsue_desc[];
Scrn_struct		*acsue_struct;


/* Template for fill-in screen */

static struct	pwd_fillin {
	char	**users;
	int	nusers;
	char	user[UNAMELEN + 1];
	char	*state;
	char	*old_state;
	int	nstructs;
} Scr_pwd;


static struct pwd_fillin *scr_pwd = &Scr_pwd;

/* offsets for scrn_struct */

#define	USERNAMES 	0

#define PARMTEMPLATE	acsue_scrn
#define STRUCTTEMPLATE	acsue_struct
#define DESCTEMPLATE	acsue_desc
#define FILLINSTRUCT	pwd_fillin
#define FILLIN		scr_pwd

#define FIRSTDESC	USERNAMES
#define NSCRNSTRUCT	1

/* see if authorized to do this
 * returns 0 if user can be added, 1 if not
 */

static int
acsue_auth (argv, pf)
char	**argv;
struct  pwd_fillin	*pf;
{
	return (canaccpw (argv, pf));
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
acsue_bfill (pf)
register struct	pwd_fillin	*pf;
{
	register int i;

	ENTERFUNC("acsue_bfill");
	pf->users = NULL;
	pf->nusers = 0L;
	memset(pf->user, '\0', sizeof(pf->user));
	GetAllUsers (&pf->nusers, &pf->users);
	if (!(pf->state = Calloc (pf->nusers, sizeof (char *))))
		MemoryError ();	/* Dies */
	if (!(pf->old_state = Calloc (pf->nusers, sizeof (char *))))
		MemoryError ();	/* Dies */
	EXITFUNC("acsue_bfill");
	return (0);
}




/*
 * fill in scrn_struct structure for the screen
 */

static
int
acsue_bstruct (pf, sptemplate)
struct	pwd_fillin	*pf;
struct	scrn_struct	**sptemplate;
{
	register struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("acsue_bstruct");
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp != (struct scrn_struct *) 0) {
		DUMPDECP (" sp being filled in", 0, 0, 0);
		sp[USERNAMES].pointer = (char *) pf->users;
		sp[USERNAMES].filled = pf->nusers;
		sp[USERNAMES].state = pf->state;
		sp[USERNAMES].val_act = acsue_radio;
	}
	EXITFUNC("acsue_bstruct");
	return (!sp);
}



/*
	executed each selection - emulates radio button action
*/


static
int
acsue_radio ()
{
	register char *cp, *last, *op;

	cp = PARMTEMPLATE.ss[USERNAMES].state;
	last = cp + PARMTEMPLATE.ss[USERNAMES].filled;
	for (op = scr_pwd->old_state; cp < last; cp++, op++)
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
acsue_valid (argv, pf)
char **argv;
struct	pwd_fillin	*pf;
{
	register char *first, *cp, *last;

	ENTERFUNC("acsue_valid");
	first = pf->state;
	last = first + pf->nusers;
	for (cp = first; cp < last; cp++)
		if (*cp) {
			strcpy (pf->user, pf->users[cp - first]);
			break;
	}
	DUMPDETI (" name=<%s>", pf->user, 0, 0);
	EXITFUNC("acsue_valid");
	return 0;
}


/*
 * save the chosen username, and it's UID, in the
 * global vars. don't bother if it's the same as the
 * current ones.
 */

static
int
acsue_save (pf)
struct	pwd_fillin	*pf;
{
	struct passwd user;

	ENTERFUNC("acsue_save");
	if (strcmp ((char *)gl_user, pf->user)) {
		strcpy ((char *)gl_user, pf->user);
		if (GetPWUserByName((char *)gl_user, &user))
			gl_uid = user.pw_uid;
		else
			gl_uid = BOGUS_ID;
	}
	DUMPDETI (" uid=<%d>", gl_uid, 0, 0);
	acsue_flag = 1;
	EXITFUNC("acsue_save");
	return 0;
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
	Free (pf->old_state);
	EXITFUNC("do_free");
	return (1);
}

#define	SETUPFUNC	acsue_setup
#define	AUTHFUNC	acsue_auth
#define BUILDFILLIN	acsue_bfill

#define	INITFUNC	acsue_init
#define BUILDSTRUCT	acsue_bstruct

#define	ROUTFUNC	acsue_exit
#define VALIDATE	acsue_valid
#define	SCREENACTION	acsue_save

#define FREEFUNC	acsue_free
#define FREESTRUCT	do_free


#include "stemplate.c"


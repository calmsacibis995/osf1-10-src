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
static char	*sccsid = "@(#)$RCSfile: acsu.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:47 $";
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
#include	"valid.h"
#include	"logging.h"

/* routine definitions for multiply used routines */

static int acsu_auth();
static int acsu_bfill();
static int canaccpw();
static int valid_it();
static int save_it();


extern Scrn_parms	acsu_scrn;
extern Scrn_desc	acsu_desc[];
Scrn_struct		*acsu_struct;


int InvalidUser();

/* Template for fill-in screen */

static struct	pwd_fillin {
	char	user[UNAMELEN + 1];
	long	uid;
	int	nstructs;
} Scr_pwd;


static struct pwd_fillin *scr_pwd = &Scr_pwd;

/* offsets for scrn_struct */

#define	USERNAME 	0
#define USERID		1
#define PWNSCRNSTRUCT	2

#define PARMTEMPLATE	acsu_scrn
#define STRUCTTEMPLATE	acsu_struct
#define DESCTEMPLATE	acsu_desc
#define FILLINSTRUCT	pwd_fillin
#define FILLIN		scr_pwd

#define FIRSTDESC	USERNAME
#define NSCRNSTRUCT	1



/*
 *  designate a user - argv[1] could be the user name or id
 */


/* see if authorized to do this
 * returns 0 if user can be added, 1 if not
 */

static int
acsu_auth (argv, pwfill)
char	**argv;
struct  pwd_fillin	*pwfill;
{
	return (canaccpw (argv, pwfill));
}


/* returns 0 if password file can be read, 1 if not */

static int
canaccpw (argv, pwfill)
char	**argv;
struct	pwd_fillin	*pwfill;
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




/* fill in a new pwfill structure with empty entries */
static
int
acsu_bfill (pwfill)
register struct	pwd_fillin	*pwfill;
{
	register int i;

	ENTERFUNC("acsu_bfill");
	memset (pwfill->user, '\0', sizeof(pwfill->user));
	pwfill->uid = 0L;
	EXITFUNC("acsu_bfill");
	return (0);
}




/*
 * fill in scrn_struct structure for the screen
 */

static
int
acsu_bstruct (pwfill, sptemplate)
struct	pwd_fillin	*pwfill;
struct	scrn_struct	**sptemplate;
{
	register struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("acsu_bstruct");
	pwfill->nstructs = PWNSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp != (struct scrn_struct *) 0) {
		DUMPDECP (" sp being filled in", 0, 0, 0);
		DUMPDETI (" name=<%s> id=<%dl>",
			&(pwfill->user[0]), pwfill->uid, NULL);
		sp[USERNAME].pointer = pwfill->user;
		sp[USERNAME].validate = InvalidUser;
		sp[USERNAME].filled = 1;
	}
	EXITFUNC("acsu_bstruct");
	return (!sp);
}




static
int
valid_it (argv, pwfill)
char **argv;
struct	pwd_fillin	*pwfill;
{
	return 0;
}


static
int
save_it (pwfill)
struct	pwd_fillin	*pwfill;
{
	struct passwd user;

	if (isdigit(pwfill->user[0])) {
		gl_uid = atoi (pwfill->user);
		if (GetPWUserByUID(gl_uid, &user))
			strcpy ((char *)gl_user, user.pw_name);
		else
			gl_user[0] = NULL;
	} else {
		if (strcmp ((char *)gl_user, pwfill->user)) {
			strcpy ((char *)gl_user, pwfill->user);
			if (GetPWUserByName((char *)gl_user, &user))
				gl_uid = user.pw_uid;
			else
				gl_uid = BOGUS_ID;
		}
	}
	return 0;
}



/* free malloc'd memory */

static int
do_free (argv, pwfill, nstructs, pp, dp, sp)
char	**argv;
struct	pwd_fillin	*pwfill;
int	nstructs;
Scrn_parms	*pp;
Scrn_desc	*dp;
Scrn_struct	*sp;
{
	return (1);
}

#define	SETUPFUNC	acsu_setup
#define	AUTHFUNC	acsu_auth
#define BUILDFILLIN	acsu_bfill

#define	INITFUNC	acsu_init
#define BUILDSTRUCT	acsu_bstruct

#define	ROUTFUNC	acsu_rout
#define VALIDATE	valid_it
#define	SCREENACTION	save_it

#define FREEFUNC	acsu_free
#define FREESTRUCT	do_free


#include "stemplate.c"

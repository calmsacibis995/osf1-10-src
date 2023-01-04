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
static char	*sccsid = "@(#)$RCSfile: acdu.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:10 $";
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



/*  User account maintenance routines.
 *  Display a user.
 *  argv - not used very extensively
 */



#include	"ac.h"

/* routine definitions for multiply used routines */

static struct passwd upw;

static int acdu_auth();
static int acdu_new();
static int do_nothing();
static int do_free();

extern int checkexistusername();
extern int checkexistuserid();

extern Scrn_parms	acdu_scrn;
extern Scrn_desc	acdu_desc[];
Scrn_struct		*acdu_struct;


#define PARMTEMPLATE	acdu_scrn
#define STRUCTTEMPLATE	acdu_struct
#define DESCTEMPLATE	acdu_desc



/* see if authorized to do this
 * returns 0 if user can be added, 1 if not
 */

static
int
acdu_auth (argv, pwfill)
char	**argv;
struct  pwd_fillin	*pwfill;
{
	pwfill->new = acdu_new;
	return (ac_canadduser (argv, pwfill));
}


static
int
acdu_new (user, userid)
char *user;
long userid;
{
	ENTERFUNC("acdu_new");
	if (user && user[0] && pw_nametoid (user) == (uid_t) -1) {
		nosuchuser (user);
		ERRFUNC ("acdu_new", "user does not exist");
		return (1);
	}

	if (userid != BOGUS_ID && pw_idtoname (userid) == NULL) {
		nosuchuid (userid);
		ERRFUNC ("acdu_new", "uid does not exist");
		return (1);
	}
	if (user && user[0])
		GetPWUserByName (user, &upw);
	else if (userid != BOGUS_ID)
		GetPWUserByUID (userid, &upw);

	EXITFUNC("acdu_new");
	return 0;
}



/*
 * fill in scrn_struct structure for the screen
 */

static
int
acdu_bstruct (pwfill, sptemplate)
struct	pwd_fillin	*pwfill;
struct	scrn_struct	**sptemplate;
{
	struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("acdu_bstruct");
	pwfill->nstructs = PWNSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp) {
		if (upw.pw_name)
			pwd2pwf (&upw, pwfill);
		else {
			pwfill->groupname[0] = pwfill->homedir[0] =
				pwfill->shellname[0] = pwfill->gecos[0] = '\0';
			pwfill->groupid = 0;
		}
		DUMPDECP (" sp being filled in", 0, 0, 0);
		DUMPDETI (" name=<%s> id=<%dl>",
			&(pwfill->username[0]), pwfill->userid, NULL);
		sp[USERNAME].pointer = pwfill->username;
		sp[USERNAME].validate = checkexistusername;
		sp[USERNAME].filled = 1;
		sp[USERID].pointer = (char *) &pwfill->userid;
		sp[USERID].validate = checkexistuserid;
		sp[USERID].filled = pwfill->userid ? 1 : 0;
		sp[GROUPFNNAME].pointer = pwfill->groupname;
		sp[GROUPFNNAME].filled = pwfill->groupname[0] ? 1 : 0;
		sp[HOMEDIR].pointer = pwfill->homedir;
		sp[HOMEDIR].filled = pwfill->homedir[0] ? 1 : 0;
		sp[SHELLNAME].pointer = pwfill->shellname;
		sp[SHELLNAME].filled = pwfill->shellname[0] ? 1 : 0;
		sp[GECOS].pointer = pwfill->gecos;
		sp[GECOS].filled = pwfill->gecos[0] ? 1 : 0;
		getgroups (pwfill->username, pwfill->groupmem,
			pwfill->ngroupmem, sec_gr_mask);
		sp[GROUPFNMEM].pointer = (char *) pwfill->groupmem;
		sp[GROUPFNMEM].state = sec_gr_mask;
		sp[GROUPFNMEM].filled = pwfill->ngroupmem;
		DUMPDETI (" name=<%s> id=<%dl>",
			sp[USERNAME].pointer, (long) sp[USERID].pointer, NULL);
	}
	EXITFUNC("acdu_bstruct");
	return (!sp);
}


/* is it a new user? If so, return 0, else 1 */

static
int
valid_user (argv, pwfill)
char **argv;
struct	pwd_fillin	*pwfill;
{
	return 0;
}


/* does the screen action once everything is validated */
 
static
int
do_nothing (pwfill)
register struct	pwd_fillin	*pwfill;
{
	ENTERFUNC("do_nothing");
	strcpy ((char *)gl_user, pwfill->username);
	gl_uid = pwfill->userid;
	EXITFUNC("do_nothing");
	return (CONTINUE);
}



/* free malloc'd memory */

static
int
do_free (argv, pwfill, nstructs, pp, dp, sp)
char	**argv;
struct	pwd_fillin	*pwfill;
int	nstructs;
Scrn_parms	*pp;
Scrn_desc	*dp;
Scrn_struct	*sp;
{
	if (pwfill->groupmem)
		free_cw_table (pwfill->groupmem);
	pwfill->groupmem = NULL;
	Free (sec_groups);
		sec_groups = NULL;
	Free (sec_gr_mask);
		sec_gr_mask = NULL;
	return (1);
}

#define	SETUPFUNC	acdu_setup
#define	AUTHFUNC	acdu_auth
#define BUILDFILLIN	ac_bfill

#define	INITFUNC	acdu_init
#define BUILDSTRUCT	acdu_bstruct

#define	ROUTFUNC	acdu_exit
#define VALIDATE	valid_user
#define	SCREENACTION	do_nothing

#define FREEFUNC	acdu_free
#define FREESTRUCT	do_free



#include "stemplate.c"

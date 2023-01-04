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
static char	*sccsid = "@(#)$RCSfile: acau.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:27 $";
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
 *  add a user to the password file.
 *  argv - not used very extensively
 */



#define AC_ALLOCATE
#include	"ac.h"

extern char *Calloc();
extern char *Malloc();

/* routine definitions for multiply used routines */

static int _clear;

static int acau_auth();
static int checkusername();
static int checkuserid();
static int acau_new();
static int do_free();


extern Scrn_parms	acau_scrn;
extern Scrn_desc	acau_desc[];
Scrn_struct		*acau_struct;


#define PARMTEMPLATE	acau_scrn
#define STRUCTTEMPLATE	acau_struct
#define DESCTEMPLATE	acau_desc



/* see if authorized to do this
 * returns 0 if user can be added, 1 if not
 */

static
int
acau_auth (argv, pwfill)
char	**argv;
struct  pwd_fillin	*pwfill;
{
	pwfill->new = acau_new;
	_clear = 0;
	return (ac_canadduser (argv, pwfill));
}


static
int
acau_new (user, userid)
char *user;
long userid;
{
	ENTERFUNC("acau_new");
	if (user && user[0] && pw_nametoid (user) != (uid_t) -1) {
		UserExistsMsg (user);
		ERRFUNC ("acau_new", "user exists");
		return (1);
	}

	if (userid != BOGUS_ID && pw_idtoname (userid) != NULL) {
		UIDExistsMsg (userid);
		ERRFUNC ("acau_new", "uid exists");
		return (1);
	}
	EXITFUNC("acau_new");
	return 0;
}



/*
 * fill in scrn_struct structure for the screen
 */

static
int
acau_bstruct (pwfill, sptemplate)
struct	pwd_fillin	*pwfill;
struct	scrn_struct	**sptemplate;
{
	struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("acau_bstruct");
	pwfill->nstructs = PWNSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp) {
		if (_clear) {
			pwfill->groupname[0] = pwfill->homedir[0] =
				pwfill->shellname[0] = pwfill->gecos[0] = '\0';
			pwfill->groupid = 0;
			_clear = 0;
		}
		DUMPDECP (" sp being filled in", 0, 0, 0);
		DUMPDETI (" name=<%s> id=<%dl>",
			&(pwfill->username[0]), pwfill->userid, NULL);
		sp[USERNAME].pointer = pwfill->username;
		sp[USERNAME].validate = checkusername;
		sp[USERNAME].filled = 1;
		sp[USERID].pointer = (char *) &pwfill->userid;
		sp[USERID].validate = checkuserid;
		sp[USERID].filled = pwfill->userid ? 1 : 0;
		sp[GROUPFNNAME].pointer = pwfill->groupname;
		sp[GROUPFNNAME].validate = checkgroup;
		sp[HOMEDIR].pointer = pwfill->homedir;
		sp[HOMEDIR].validate = checkhomedir;
		sp[SHELLNAME].pointer = pwfill->shellname;
		sp[SHELLNAME].validate = checkshellname;
		sp[GECOS].pointer = pwfill->gecos;
		sp[GECOS].validate = checkgecos;
		sp[GROUPFNMEM].pointer = (char *) pwfill->groupmem;
		sp[GROUPFNMEM].state = sec_gr_mask;
		sp[GROUPFNMEM].validate = NULL;
		sp[GROUPFNMEM].filled = pwfill->ngroupmem;
		DUMPDETI (" name=<%s> id=<%dl>",
			sp[USERNAME].pointer, (long) sp[USERID].pointer, NULL);
	}
	EXITFUNC("acau_bstruct");
	return (!sp);
}


/*
 * check that a name was entered, and that it was unique.
 * return a 0 if conditions are met, 1 otherwise.
 */

static
int
checkusername (pwfill)
struct	pwd_fillin	*pwfill;
{

	ENTERFUNC("checkusername");
	if (!pwfill->username) {
		pop_msg ("No user name entered", "Please enter a name");
		ERRFUNC ("checkusername", "no user name");
		return (1);
	}
	if (InvalidUser (pwfill->username)) {
		InvUserMsg (pwfill->username);
		ERRFUNC ("checkusername", "bad user name");
		return (1);
	}
	if (pw_nametoid (pwfill->username) != (uid_t) -1) {
		UserExistsMsg (pwfill->username);
		ERRFUNC ("checkusername", "user name exists");
		return (1);
	}
	EXITFUNC("checkusername");
	return (0);
}



/* check uniqueness of user id.  If already assigned, don't allow.
 * if none entered, try to assign one.
 * return 0 if 0, assigned, or entered value is unique, 1 otherwise.
 */

static
int
checkuserid (pwfill)
struct pwd_fillin	*pwfill;
{
	ENTERFUNC("checkuserid");
	if (pwfill->userid == 0 || pwfill->userid == BOGUS_ID) {
		pwfill->userid = chooseuserid ();
		ERRFUNC ("checkuserid", "UID assigned");
		return (0);
	}
	if (pw_idtoname (pwfill->userid) != NULL) {
		UIDExistsMsg (pwfill->userid);
		ERRFUNC ("checkuserid", "UID exists");
		return (1);
	}
	EXITFUNC("checkuserid");
	return (0);
}



/* is it a new user? If so, return 0, else 1 */

static
int
valid_user (argv, pwfill)
char **argv;
struct	pwd_fillin	*pwfill;
{

	char	*user;
	long	userid;

	ENTERFUNC("valid_user");

	if (*pwfill->username)
		user = pwfill->username;
	else
		user = NULL;
	userid = pwfill->userid;

	if (pwfill->userid == 0 || pwfill->userid == BOGUS_ID) {
		pwfill->userid = chooseuserid ();
		ERRFUNC ("checkuserid", "UID assigned");
		return (0);
	}

	DUMPVARS ("valid_user: user='%s' uid=<%d>", (user?user:"NO NAME"),
		userid, NULL);
	/* check syntax of user name, assure that there are no ':'.
	 * getscreen checks for unprintable characters.
	 */

	if (user == NULL || InvalidUser (user)) {
		InvUserMsg (user);
		ERRFUNC ("valid_user", "bad user name");
		return (1);
	}

	if (acau_new (user, userid))
		return (1);

	if (user) {
		strcpy ((char *)gl_user, user);
	}
	gl_uid = userid;
	EXITFUNC("valid_user");
	return 0;
}


/* does the screen action once everything is validated */
 
static
int
do_adduser (pwfill)
register struct	pwd_fillin	*pwfill;
{
	int i, j, ret;
	struct passwd pwd;

	ENTERFUNC("do_adduser");

	memset((char *) &pwd, '\0', sizeof pwd);
	pwd.pw_name = pwfill->username;
	pwd.pw_passwd = "*";
	pwd.pw_uid = pwfill->userid;
	pwd.pw_gid = pwfill->groupid;
	pwd.pw_gecos = pwfill->gecos;
	pwd.pw_dir = pwfill->homedir;
	pwd.pw_shell = pwfill->shellname;
	ret = XCreateUserAccount (&pwd);
	if (ret == SUCCESS) {
		for (i = 0, j = 0; i < pwfill->ngroupmem; i++)
			if (sec_gr_mask[i])
				j++;
		if (j) {
			sec_groups = (char **)Calloc(j + 1, sizeof (char *));
			if (!sec_groups)
				MemoryError ();		/* dies */
			for (i = 0, j = 0; i < pwfill->ngroupmem; i++)
			    if (sec_gr_mask[i])
				sec_groups[j++] = &pwfill->groupmem[i][0];
		}
		ret = XModifyUserGroup (pwd.pw_name, pwd.pw_gid,
			sec_groups);
	}
	if (ret == SUCCESS) {
		for (i = 0; i < pwfill->ngroupmem; i++)
			sec_gr_mask[i] = 0;
		Free (sec_groups);
		sec_groups = NULL;
		_clear = 1;
	}
	EXITFUNC("do_adduser");
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
	if (sec_groups) {
		Free (sec_groups);
		sec_groups = NULL;
	}
	if (sec_gr_mask) {
		Free (sec_gr_mask);
		sec_gr_mask = NULL;
	}
	_clear = 1;
	return (1);
}

#define	SETUPFUNC	acau_setup
#define	AUTHFUNC	acau_auth
#define BUILDFILLIN	ac_bfill

#define	INITFUNC	acau_init
#define BUILDSTRUCT	acau_bstruct

#define	ROUTFUNC	acau_exit
#define VALIDATE	valid_user
#define	SCREENACTION	do_adduser

#define FREEFUNC	acau_free
#define FREESTRUCT	do_free



#include "stemplate.c"



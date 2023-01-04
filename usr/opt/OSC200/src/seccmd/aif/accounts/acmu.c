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
static char	*sccsid = "@(#)$RCSfile: acmu.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:29 $";
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
 *  modify and entry in the password file.
 *  argv - not used very extensively
 */



#include	"ac.h"
#include	"Utils.h"


/* routine definitions for multiply used routines */

static int acmu_auth();
static int acmu_new();
static int updatepwfile();
static int do_free();

int checkexistusername();
int checkexistuserid();

extern Scrn_parms	acmu_scrn;
extern Scrn_desc	acau_desc[];
Scrn_struct		*acmu_struct;

extern int acsue_flag;		/* whether acsue_ menu changed things */

#define PARMTEMPLATE	acmu_scrn
#define STRUCTTEMPLATE	acmu_struct
#define DESCTEMPLATE	acau_desc



/* see if authorized to do this
 * returns 0 if user can be added, 1 if not
 */

static
int
acmu_auth (argv, pf)
char	**argv;
struct  pwd_fillin	*pf;
{
	pf->new = acmu_new;
	return (ac_canadduser (argv, pf));
}


static
int
acmu_new (user, userid)
char *user;
long userid;
{
	ENTERFUNC("acmu_new");
	if (user && user[0] && pw_nametoid (user) == (uid_t) -1) {
		nosuchuser (user);
		ERRFUNC ("acmu_new", "user does not exist");
		return (1);
	}

	if (userid != BOGUS_ID && pw_idtoname (userid) == NULL) {
		nosuchuid (userid);
		ERRFUNC ("acmu_new", "uid does not exist");
		return (1);
	}
	if (user && user[0])
		strcpy(scr_pwd->username, user);
	else
		scr_pwd->username[0] = '\0';
	scr_pwd->userid = userid;

	EXITFUNC("acmu_new");
	return 0;
}



/*
 * fill in scrn_struct structure for the screen
 */

static
int
acmu_bstruct (pf, sptemplate)
struct	pwd_fillin	*pf;
struct	scrn_struct	**sptemplate;
{
	struct	scrn_struct	*sp;
	int	i;
	struct	passwd *pwd = (struct passwd *) 0;

	ENTERFUNC("acmu_bstruct");
	pf->nstructs = PWNSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp) {
		endpwent();
		if (pf->username[0])
			pwd = getpwnam(pf->username);
		else if (pf->userid != BOGUS_ID)
			pwd = getpwuid(pf->userid);
		if (pwd == (struct passwd *) 0) {
			if (pf->username[0])
				nosuchuser(pf->username);
			else
				nosuchuid(pf->userid);
			return 1;
		}
		pwd2pwf(pwd, pf);

		if (*pf->username)
			strcpy ((char *) gl_user, pf->username);
		if (pf->userid != BOGUS_ID)
			gl_uid = pf->userid;

		DUMPDECP (" sp being filled in", 0, 0, 0);
		DUMPDETI (" name=<%s> id=<%dl>",
			&(pf->username[0]), pf->userid, NULL);
		sp[USERNAME].pointer = pf->username;
		sp[USERNAME].validate = checkexistusername;
		sp[USERNAME].filled = 1;
		sp[USERID].pointer = (char *) &pf->userid;
		sp[USERID].validate = checkexistuserid;
		sp[USERID].filled = (pf->userid != BOGUS_ID) ? 1 : 0;
		sp[GROUPFNNAME].pointer = pf->groupname;
		sp[GROUPFNNAME].validate = checkgroup;
		sp[GROUPFNNAME].filled = pf->groupname[0] ? 1 : 0;
		sp[HOMEDIR].pointer = pf->homedir;
		sp[HOMEDIR].validate = checkhomedir;
		sp[HOMEDIR].filled = pf->homedir[0] ? 1 : 0;
		sp[SHELLNAME].pointer = pf->shellname;
		sp[SHELLNAME].validate = checkshellname;
		sp[SHELLNAME].filled = pf->shellname[0] ? 1 : 0;
		sp[GECOS].pointer = pf->gecos;
		sp[GECOS].validate = checkgecos;
		sp[GECOS].filled = pf->gecos[0] ? 1 : 0;
		getgroups (pf->username, pf->groupmem,
			pf->ngroupmem, sec_gr_mask);
		sp[GROUPFNMEM].pointer = (char *) pf->groupmem;
		sp[GROUPFNMEM].state = sec_gr_mask;
		sp[GROUPFNMEM].validate = NULL;
		sp[GROUPFNMEM].filled = pf->ngroupmem;
		DUMPDETI (" name=<%s> id=<%dl>",
			sp[USERNAME].pointer, (long) sp[USERID].pointer, NULL);
	}
	acsue_flag = 0;
	EXITFUNC("acmu_bstruct");
	return (!sp);
}


/*
 * check that a name was entered, and that it was unique.
 * return a 0 if conditions are met, 1 otherwise.
 */

int
checkexistusername (pf)
struct	pwd_fillin	*pf;
{

	ENTERFUNC("checkexistusername");
	if (!pf->username) {
		pop_msg ("No user name entered", "Please enter a name");
		ERRFUNC ("checkexistusername", "no user name");
		return (1);
	}
	if (InvalidUser (pf->username)) {
		InvUserMsg (pf->username);
		ERRFUNC ("checkexistusername", "bad user name");
		return (1);
	}
	if (pw_nametoid (pf->username) == (uid_t) -1) {
		nosuchuser (pf->username);
		ERRFUNC ("checkexistusername", "user name exists");
		return (1);
	}
	EXITFUNC("checkexistusername");
	return (0);
}



/* check uniqueness of user id.  If not already assigned, don't allow.
 * return 0 if not assigned, 1 otherwise.
 */

int
checkexistuserid (pf)
struct pwd_fillin	*pf;
{
	ENTERFUNC("checkexistuserid");
	if (pw_idtoname (pf->userid) == NULL) {
		nosuchuid (pf->userid);
		ERRFUNC ("checkexistuserid", "UID exists");
		return (1);
	}
	EXITFUNC("checkexistuserid");
	return (0);
}



/* is it an old user? If so, return 0, else 1 */

static
int
valid_user (argv, pf)
char **argv;
struct	pwd_fillin	*pf;
{

	char	*user;
	long	userid;

	ENTERFUNC("valid_user");

	if (*pf->username) {
		if (acsue_flag)
			if (strcmp((char *) gl_user, pf->username))
				strcpy((char *) gl_user, pf->username);
		user = pf->username;
	} else if (gl_user) {
		user = (char *) gl_user;
	} else {
		user = NULL;
	}
	if (pf->userid != BOGUS_ID) {
		if (acsue_flag)
			gl_uid = pf->userid;
		userid = pf->userid;
	} else {
		userid = gl_uid;
	}
	acsue_flag = 0;

	if ((!user || !*user) && userid == BOGUS_ID) {
		ERRFUNC ("valid_user", "no user or id");
		return (0);
	}

	DUMPVARS ("valid_user: user='%s' uid=<%d>", (user?user:"NO NAME"),
		userid, NULL);
	/* check syntax of user name, assure that there are no ':'.
	 * getscreen checks for unprintable characters.
	 */

	if (InvalidUser (user)) {
		InvUserMsg (user);
		ERRFUNC ("valid_user", "bad user name");
		return (1);
	}

	if (acmu_new (user, userid))
		return (1);

	if (user) {
		strcpy (pf->username, user);
		strcpy ((char *)gl_user, user);
	}
	gl_uid = userid;
	EXITFUNC("valid_user");
	return 0;
}


/* does the screen action once everything is validated */
 
static
int
do_moduser (pf)
register struct	pwd_fillin	*pf;
{
	int i, j, ret;
	struct passwd pwd;

	ENTERFUNC("do_moduser");

	pwd.pw_name = pf->username;
	pwd.pw_passwd = "";
	pwd.pw_uid = pf->userid;
	pwd.pw_gid = pf->groupid;
	pwd.pw_gecos = pf->gecos;
	pwd.pw_dir = pf->homedir;
	pwd.pw_shell = pf->shellname;
	ret = XModifyUserpwd (&pwd);
	if (ret == SUCCESS) {
		for (i = 0, j = 0; i < pf->ngroupmem; i++)
			if (sec_gr_mask[i])
				j++;
		if (j) {
			sec_groups = (char **)Calloc (j, sizeof (char *));
			if (!sec_groups)
				MemoryError ();		/* dies */
			for (i = 0, j = 0; i < pf->ngroupmem; i++)
			    if (sec_gr_mask[i])
				sec_groups[j++] = &pf->groupmem[i][0];
			sec_groups[j] = NULL;
		}
		ret = XModifyUserGroup (pwd.pw_name, pwd.pw_gid,
			sec_groups);
	}
	if (ret == SUCCESS) {
		for (i = 0; i < pf->ngroupmem; i++)
			sec_gr_mask[i] = 0;
		Free (sec_groups);
		sec_groups = NULL;
	}
	EXITFUNC("do_moduser");
	return (CONTINUE);
}



/* free malloc'd memory */

static
int
do_free (argv, pf, nstructs, pp, dp, sp)
char	**argv;
struct	pwd_fillin	*pf;
int	nstructs;
Scrn_parms	*pp;
Scrn_desc	*dp;
Scrn_struct	*sp;
{
	if (pf->groupmem)
		free_cw_table (pf->groupmem);
	pf->groupmem = NULL;
	Free (sec_groups);
		sec_groups = NULL;
	Free (sec_gr_mask);
		sec_gr_mask = NULL;
	return (1);
}

#define	SETUPFUNC	acmu_setup
#define	AUTHFUNC	acmu_auth
#define BUILDFILLIN	ac_bfill

#define	INITFUNC	acmu_init
#define BUILDSTRUCT	acmu_bstruct

#define	ROUTFUNC	acmu_exit
#define VALIDATE	valid_user
#define	SCREENACTION	do_moduser

#define FREEFUNC	acmu_free
#define FREESTRUCT	do_free



#include "stemplate.c"

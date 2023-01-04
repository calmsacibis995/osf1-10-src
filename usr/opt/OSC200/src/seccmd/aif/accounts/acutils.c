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
static char	*sccsid = "@(#)$RCSfile: acutils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:01 $";
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



/*  User account utility routines.
 *
 *	check & access routines return 0 for success
 *
 *	access_pw()		be sure passwd file read/writable
 *	checkgroup()		make sure primary group exists
 *	checkgecos()		check gecos field for valid characters
 *	checkhomedir()		be sure dir doesn't exist, parent writable
 *	checkshellname()	make sure shell name is legit
 *	getgroups()		get all groups a user is in
 *
 *	pwd2pwf()		copy passwd data to pwfill
 *	pwf2pwd()		copy pwfill pointers & data to passwd
 *
 *	ac_canadduser()		can user be added/modified/whatever?
 *	ac_isolduser()		is it a new/old user?
 *	ac_bfill()		fill in a new pwfill structure
 */

#include	"ac.h"

extern char *Malloc();
extern char *Calloc();


/* returns 0 if passwd file read/writable, else 1 */

int
access_pw ()
{
	extern	char	passwd[];		/* password file name */
	char	*pw_dir, *pw_slash;
	char	*strrchr();
	int	status = 0;
	int	n;

	ENTERLFUNC("access_pw");
	/* check that the program can write in the parent directory of
	 * the passwd file.
	 */
	
	pw_slash = strrchr (passwd, '/');
	n = pw_slash - passwd;
	pw_dir = Malloc (n + 1);
	if (!pw_dir)
		MemoryError ();
	strncpy (pw_dir, passwd, n);
	*(pw_dir + n) = NULL;

	if (eaccess (pw_dir, 3) != 0) {  /* need write and search */
		if (! msg_error_cant_access_passwd)
			LoadMessage("msg_accounts_make_user_cant_access_passwd",
				&msg_error_cant_access_passwd, 
				&msg_error_cant_access_passwd_text);
		ErrorMessageOpen(-1, msg_error_cant_access_passwd, 0, NULL);
		ERRLFUNC ("access_pw", "passwd access");
		status = 1;
	}
	Free (pw_dir);
	EXITLFUNC("access_pw");
	return (status);
}



/* check a user's group
 * returns 0 on success, 1 on failure
 */

int
checkgroup (pwfill)
struct	pwd_fillin	*pwfill;
{
	gid_t gid;
	char *gr_name;
	int ret;

	/* check if the user can change to this group */

	ENTERLFUNC("checkgroup");
	gid = gr_nametoid(pwfill->groupname);
	if (gid == (gid_t) -1) {
		char buf[80];
		sprintf(buf, "Group '%s' does not exist.", pwfill->groupname);
		pop_msg(buf,
"Please add the group using the Add Group screen or correct the entry.");

		/* restore old group */

		gr_name = gr_idtoname(pwfill->groupid);
		if (gr_name)
			strcpy(pwfill->groupname, gr_name);
		else
			strcpy(pwfill->groupname, "*UNKNOWN");
		ret = 1;
	}
	else {
		pwfill->groupid = gid;
		ret = 0;
	}
	EXITLFUNC("checkgroup");
	return ret;
}


/* check a user's gecos field
 * returns 0 on success, 1 on failure
 */

int
checkgecos (pwfill)
struct	pwd_fillin	*pwfill;
{

	ENTERLFUNC("checkgecos");
	/* check that there aren't any ':' characters */
	if (strchr (pwfill->gecos, ':'))  {
		if (! msg_error_invalid_comment)
			LoadMessage("msg_accounts_make_user_invalid_comment",
				&msg_error_invalid_comment, 
				&msg_error_invalid_comment_text);
		ErrorMessageOpen(-1, msg_error_invalid_comment, 0, NULL);
		ERRLFUNC("checkgecos", "Unprintable characters in gecos field");
		return (1);
	}
	EXITLFUNC("checkgecos");
	return (0);
}



/* return 0 if dir doesn't exist, parent writable, else 1 */

int
checkhomedir (pwfill)
struct	pwd_fillin	*pwfill;
{
	struct	stat	sb;
	register char	*cp;
	char	*thome;
	int	cantsearch;
	void	homedirexists(), badhomedir();

	ENTERLFUNC("checkhomedir");
	if (stat (pwfill->homedir, &sb) == 0) {  /* exists */
		homedirexists (pwfill->homedir);
		return (1);
	}
	thome = Malloc (strlen (pwfill->homedir) + 1);
	strcpy (thome, pwfill->homedir);
	cp = strrchr (thome, '/');
	if (cp == NULL)  {
		badhomedir (pwfill->homedir);
		Free (thome);
		return (1);
	}

	/* must be able to write home directory parent */
	*cp = '\0';
	if (eaccess (thome, 2) != 0) {
		pop_msg (
		"To create home directory, this program must have write",
		"permission to the directory's parent.");
		Free (thome);
		return (1);
	}
	*cp = '/';

	/* if user or group not assigned yet, return OK */
	if (pwfill->userid == 0 || pwfill->groupid == 0)  {
		Free (thome);
		return (0);
	}

	cantsearch = 0;
	do {
		*cp = '\0';
		if (stat (thome, &sb) != 0) {
			badhomedir (pwfill->homedir);
			Free (thome);
			return (1);
		}
		if (pwfill->userid == sb.st_uid)
			if ((sb.st_mode & S_IEXEC) == 0)
				cantsearch = 1;
			else	cantsearch = 0;
		else if (pwfill->groupid == sb.st_gid)
			if ((sb.st_mode & (S_IEXEC>>3)) == 0)
				cantsearch = 1;
			else	cantsearch = 0;
		else if ((sb.st_mode & (S_IEXEC >> 6)) == 0)
			cantsearch = 1;
		else	cantsearch = 0;
	} while (cantsearch == 0 && (cp = strrchr (thome, '/')) && cp != thome);
	
	Free (thome);

	if (cantsearch)  {
		badhomedir (pwfill->homedir);
		return (1);
	}
	EXITLFUNC("checkhomedir");
	return (0);
}



/* check that the shell name for the user is valid
 * returns 0 on success, 1 on failure
 */

int
checkshellname (pwfill)
struct	pwd_fillin	*pwfill;
{
	struct	stat	sb;
	void	noshell();

	ENTERLFUNC("checkshellname");
	DUMPLDETI(" name='%s'", pwfill->shellname, NULL, NULL);
	/* NULL shell field means just use the default */
	if (*pwfill->shellname == '\0') {
		ERRLFUNC("checkshellname", "Using default shell");
		return (0);
	}

	/* check that shell exists */
	/* check that shell is a regular file */
	/* check that shell is executable by someone */
	if (stat (pwfill->shellname, &sb) < 0 ||
	    (sb.st_mode & S_IFMT) != S_IFREG ||
	    (sb.st_mode & (S_IEXEC | (S_IEXEC >> 3) | (S_IEXEC >> 6))) == 0) {
		if (! msg_error_invalid_shell)
			LoadMessage("msg_accounts_make_user_invalid_shell",
				&msg_error_invalid_shell, 
				&msg_error_invalid_shell_text);
		ErrorMessageOpen(-1, msg_error_invalid_shell, 0, NULL);
		ERRLFUNC("checkshellname", "no shell");
		return (1);
	}
	EXITLFUNC("checkshellname");
	return (0);
}




void
getgroups (name, groups, ngroups, mask)
char *name;
char **groups;
int ngroups;
char *mask;
{
	struct	group	*grp, *getgrent();
	char *ug, *user, *pmask;
	int i;

	ENTERLFUNC("getgroups");
	setgrent();
	while (grp = getgrent()) {
		for (user = grp->gr_mem[i = 0]; user; user = grp->gr_mem[++i])
			if (!strcmp (name, user))
				break;
		if (!user)
			continue;
		for (ug = groups[i = 0], pmask = mask;
		    i < ngroups;
		    ug = groups[++i], pmask++)
			if (!strcmp (ug, grp->gr_name)) {
				*pmask = 1;
				break;
			}
	}

	EXITLFUNC("getgroups");
	return;
}



/*
 * pwf2pwd() - copy pwfill pointers & data to passwd
 */

void
pwf2pwd (pwfill, pwd)
register struct pwd_fillin *pwfill;
register struct passwd *pwd;
{
	ENTERLFUNC("pwf2pwd");
	pwd->pw_name = pwfill->username;
	pwd->pw_passwd = "";		/* set elsewhere */
	pwd->pw_uid = pwfill->userid;
	pwd->pw_gid = pwfill->groupid;
	pwd->pw_comment = "";		/* actually, we don't care yet */
	pwd->pw_gecos = pwfill->gecos;
	pwd->pw_dir = pwfill->homedir;
	pwd->pw_shell = pwfill->shellname;
	EXITLFUNC("pwf2pwd");
	return;
}

/*
 * pwd2pwf() - copy passwd data to pwfill
 * age, comment, fall into a black hole
 */

void
pwd2pwf (pwd, pwfill)
register struct passwd *pwd;
register struct pwd_fillin *pwfill;
{
	char *s;

	ENTERLFUNC("pwf2pwd");
	strcpy (pwfill->username, pwd->pw_name);
	pwfill->userid = pwd->pw_uid;
	pwfill->groupid = pwd->pw_gid;
	s = (char *)GetGrName (pwd->pw_gid);
	if (s) {
		strcpy (pwfill->groupname, s);
		Free (s);
	}
	strcpy (pwfill->gecos, pwd->pw_gecos);
	strcpy (pwfill->homedir, pwd->pw_dir);
	strcpy (pwfill->shellname, pwd->pw_shell);
	EXITLFUNC("pwf2pwd");
	return;
}


/* returns 0 if user can be added, 1 if not */

int
ac_canadduser (argv, pwfill)
char	**argv;
struct	pwd_fillin	*pwfill;
{
	ENTERLFUNC("ac_canadduser");
	if (access_pw ()) {
		ERRLFUNC ("ac_canadduser", "passwd access");
		return (1);
	}
	return (ac_isolduser (argv, pwfill));
}


/* is it a new/old user? If so, return 0, else 1 */

int
ac_isolduser (argv, pwf)
char **argv;
struct	pwd_fillin	*pwf;
{

	char	*user;
	long	userid;

	ENTERLFUNC("ac_isolduser");

	if (*gl_user)
		user = (char *)gl_user;
	else
		user = NULL;
	userid = gl_uid;
	if ((!user || !*user) && userid == BOGUS_ID) {
		ERRLFUNC ("ac_isolduser", "no user or id");
		return 0;
	}

	DUMPLVARS ("ac_isolduser: user='%s' uid=<%d>", (user?user:"NO NAME"),
		userid, NULL);
	if (pwf->new && (*pwf->new)(user, userid))
		return (1);

	if (user)
		strcpy (pwf->username, user);
	pwf->userid = userid;
	pwf->groupid = 0;
	EXITLFUNC("ac_isolduser");
	return 0;
}




/* fill in a new pwfill structure with empty entries */
int
ac_bfill (pwfill)
register struct	pwd_fillin	*pwfill;
{
	int i;

	ENTERLFUNC("ac_bfill");
	pwfill->groupname[0] = pwfill->homedir[0] =
	  pwfill->shellname[0] = pwfill->gecos[0] = '\0';
	pwfill->groupid = 0;
	GetAllGroups (&pwfill->ngroupmem, &pwfill->groupmem);
	if (!pwfill->ngroupmem ||
	    !(sec_gr_mask = Calloc (pwfill->ngroupmem, sizeof (char))))
		MemoryError ();		/* dies */
	DUMPLDETI (" ngrp=<%d> &grp=<%x> &state=<%x>", pwfill->ngroupmem,
		pwfill->groupmem, sec_gr_mask);
	EXITLFUNC("ac_bfill");
	return (0);
}

int checkmemgroup () {}

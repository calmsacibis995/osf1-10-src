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
static char *rcsid = "@(#)$RCSfile: dec_verify.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/29 21:39:41 $";
#endif
/*
 * xdm - display manager daemon
 *
 * $XConsortium: verify.c,v 1.24 91/07/18 22:22:45 rws Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * verify.c
 *
 * typical unix verification routine.
 */

# include	"dm.h"

#ifdef	SEC_BASE
#include <sys/security.h>
#include <sys/secdefines.h>
#include <prot.h>
#define	MESSAGE(x)
#endif	/* SEC_BASE */

#if defined(USE_MOTIF)
# include "gpi.h"
#endif /*USE_MOTIF*/

#if defined(CAPACITY_LIMIT) && defined(__osf__)
# include <sys/sysinfo.h>
#endif /*CAPACITY_LIMIT*/

#ifdef USE_SIA
#include <sia.h>
#include <paths.h>
#endif /* USE_SIA */

# include	<pwd.h>
# ifdef NGROUPS_MAX
# include	<grp.h>
# endif
#ifdef USESHADOW
# include	<shadow.h>
#endif
#ifdef X_NOT_STDC_ENV
char *getenv();
#endif

struct passwd joeblow = {
	"Nobody", "***************"
};

#ifdef USESHADOW
struct spwd spjoeblow = {
	"Nobody", "**************"
};
#endif
static char *envvars[] = {
#if defined(sony) && !defined(SYSTYPE_SYSV)
    "bootdev",
    "boothowto",
    "cputype",
    "ioptype",
    "machine",
    "model",
    "CONSDEVTYPE",
    "SYS_LANGUAGE",
    "SYS_CODE",
    "TZ",
#endif
    NULL
};

#ifdef	SEC_BASE
struct pr_passwd joebleau = {
	"Nobody", -2, ":::::::::::::", "Nobody", 0
};
#endif	/* SEC_BASE */

#ifdef USE_SIA

Verify (d, greet, verify, pwd)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
struct passwd *pwd;
{
	struct passwd	*p, *result;
	char		*crypt ();
	char		**userEnv (), **systemEnv (), **parseArgs ();
	char		*shell, *home;
	char		**argv;

	Debug("Verify %s\n" , greet->name);
        if (pwd == NULL) pwd = getpwnam (greet->name);
        p = pwd;
	if (!p || strlen (greet->name) == 0)
		p = &joeblow;

#if defined(CAPACITY_LIMIT) && defined(__osf__)
	Debug( "Capacity limits active.\n" );
	if (p->pw_uid) {
	  Debug( "User not root, capacity limit being checked.\n" );
	  if(setsysinfo(SSI_ULIMIT,0,0,0,0) != 0) {
	    Debug( "Capacity limit exceeed.\n" );
	    LogError( "Can't start session on %s, too many users.\n",
		      d->name );
	    return VERIFY_CAPACITY_LIMIT_EXCEEDED;
	  }
	}
#endif /*CAPACITY_LIMIT*/
    
	Debug ("verify succeeded\n");
	bzero(greet->password, strlen(greet->password));

	verify->uid = p->pw_uid;
#ifdef NGROUPS
	getGroups (greet->name, verify, p->pw_gid);
#else
	verify->gid = p->pw_gid;
#endif
	home = p->pw_dir;
	shell = p->pw_shell;

if (!shell || (strlen(shell) == 0))
    shell = _PATH_BSHELL;

	argv = 0;
	if (d->session)
		argv = parseArgs (argv, d->session);
	if (greet->string)
		argv = parseArgs (argv, greet->string);
	if (!argv)
		argv = parseArgs (argv, "xsession");
	verify->argv = argv;
	verify->userEnviron = userEnv (d, p->pw_uid == 0,
				       greet->name, home, shell);
	Debug ("user environment:\n");
	printEnv (verify->userEnviron);
	verify->systemEnviron = systemEnv (d, greet->name, home);
	Debug ("system environment:\n");
	printEnv (verify->systemEnviron);
	Debug ("end of environments\n");
	return VERIFY_SUCCESSFUL;
}

#else	/* USE_SIA */

Verify (d, greet, verify)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
{
	struct passwd	*p, *result;
#ifdef USESHADOW
	struct spwd	*sp;
#endif
	char		*crypt ();
	char		**userEnv (), **systemEnv (), **parseArgs ();
	char		*shell, *home;
	char		**argv;
#ifdef	SEC_BASE
	struct pr_passwd *ppwd, save_ppwd;
	char		*bigcrypt(), buf[256];
	time_t		last_slogin=0, last_ulogin=0;
	int		num_ulogins=0;
#endif	/* SEC_BASE */

#ifndef	SEC_BASE
	Debug ("Verify %s ...\n", greet->name);
	p = getpwnam (greet->name);
	if (!p || strlen (greet->name) == 0)
		p = &joeblow;


#ifdef USESHADOW
	sp = getspnam(greet->name);
	if (sp == NULL) {
		sp = &spjoeblow;
		Debug ("getspnam() failed.  Are you root?\n");
	}
	endspent();

#ifdef MORE_CORRECT
	/* The following tests should be made, but are skipped for now
	   to insure compatibility with older mechanisms */
	if (strlen(sp->sp_pwdp) == 0) {
		if (strlen(greet->password) != 0) {
			Debug ("More correct verify failed\n");
			bzero(greet->password, strlen(greet->password));
			return VERIFY_FAILED;
		}
		/* Else both == 0, so verify didn't fail, yet */
	}
#endif /* MORE_CORRECT */
	if (strcmp (crypt (greet->password, sp->sp_pwdp), sp->sp_pwdp))
#else /* USE_SHADOW */
#ifdef MORE_CORRECT
	/* The following tests should be made, but are skipped for now
	   to insure compatibility with older mechanisms */
	if (strlen(p->pw_passwd) == 0) {
		if (strlen(greet->password) != 0) {
			Debug ("More correct verify failed\n");
			bzero(greet->password, strlen(greet->password));
			return VERIFY_FAILED;
		}
		/* Else both == 0, so verify didn't fail, yet */
	}
#endif /* MORE_CORRECT */
	if (strcmp (crypt (greet->password, p->pw_passwd), p->pw_passwd))
#endif /* USE_SHADOW */
	{
		Debug ("verify failed\n");
		bzero(greet->password, strlen(greet->password));
		return 0;
	}
#else /* SEC_BASE */
	Debug ("Secure verify active\n");
	ppwd = getprpwent(greet->name);
	p = getpwnam(greet->name);
	if(!p || !ppwd || strlen(greet->name) == 0) {
		p = &joeblow;
		ppwd = &joebleau;
	}
	if(ppwd->uflg.fg_pwchanger) {
/* Notify user that password was changed by idtoname(ppwd->uflg.fd_pwchanger)
   or by the UID number in the same location if idtoname returns NULL, at the
   time given by ctime(&ppwd->ufld.fd_schange).
 */
	}

#ifdef MORE_CORRECT
	/* The following tests should be made, but are skipped for now
	   to insure compatibility with older mechanisms */
	if (strlen(ppwd->ufld.fd_encrypt) == 0) {
		if (strlen(greet->password) != 0) {
			Debug ("More correct secure verify failed\n");
			bzero(greet->password, strlen(greet->password));
			MESSAGE ("Wait for login retry...");
#ifndef BADLOGIN_DELAY
#define	BADLOGIN_DELAY	4
#endif
			sleep(BADLOGIN_DELAY);
			return VERIFY_FAILED;
		}
		/* Else both == 0, so verify didn't fail, yet */
	}
	else /* ppwd->ufld.fd_encrypt is not empty so use bigcrypt() */
#endif

	if(strcmp(ppwd->ufld.fd_encrypt,
	    bigcrypt(greet->password, ppwd->ufld.fd_encrypt))) {
		Debug ("verify failed\n");
		bzero(greet->password, strlen(greet->password));
		MESSAGE ("Wait for login retry...");
#ifndef BADLOGIN_DELAY
#define	BADLOGIN_DELAY	4
#endif
		sleep(BADLOGIN_DELAY);
		return VERIFY_FAILED;
	}
#endif /* SEC_BASE */

#if defined(CAPACITY_LIMIT) && defined(__osf__)
	Debug( "Capacity limits active.\n" );
	if (p->pw_uid) {
	  Debug( "User not root, capacity limit being checked.\n" );
	  if(setsysinfo(SSI_ULIMIT,0,0,0,0) != 0) {
	    Debug( "Capacity limit exceeed.\n" );
	    LogError( "Can't start session on %s, too many users.\n",
		      d->name );
	    return VERIFY_CAPACITY_LIMIT_EXCEEDED;
	  }
	}
#endif /*CAPACITY_LIMIT*/

	Debug ("verify succeeded\n");
/*	bzero(greet->password, strlen(greet->password)); */

#ifdef	SEC_BASE
	if(locked_out(ppwd)) {
		Debug ("user locked out\n");
		MESSAGE(
		"Account is disabled -- see Account Administrator.\n");
		return VERIFY_FAILED;
	}
	if(ppwd->uflg.fg_retired && ppwd->ufld.fd_retired) {
		Debug ("account retired\n");
		MESSAGE(
		"Account has been retired -- logins are no longer allowed.\n");
		return VERIFY_FAILED;
	}
	if(time_lock(ppwd)) {
		Debug ("wrong time for account\n");
		MESSAGE(
		"Wrong time period to log into this account.\n");
		return VERIFY_FAILED;
	}
	if(ppwd->uflg.fg_slogin)
		last_slogin = ppwd->ufld.fd_slogin;
	if(ppwd->uflg.fg_ulogin)
		last_ulogin = ppwd->ufld.fd_ulogin;
	if(ppwd->uflg.fg_nlogins)
		num_ulogins = ppwd->ufld.fd_nlogins;
	save_ppwd = *ppwd;
	ppwd = &save_ppwd;
	ppwd->uflg.fg_slogin = 1;		/* Succesful login */
	ppwd->ufld.fd_slogin = time((long *)0);	/* Time of login */
	ppwd->uflg.fg_nlogins = 0;		/* No bad logins */
	ppwd->uflg.fg_suctty = 0;		/* No tty device for xdm
						   for now */
	ppwd->uflg.fg_pwchanger = 0;		/* Password unchanged */
	if(!putprpwnam(ppwd->ufld.fd_name, ppwd)) {
		Debug ("can't update protected password entry\n");
		return VERIFY_FAILED;
	}
/*
 * At this point the following information has been saved for display to the user:
 *	last_slogin	- Time of last successful login (if non-zero)
 *	last_ulogin	- Time of last unsucessful login (if non-zero)
 *	num_ulogins	- Count of unsucessful logins since last successful login
 *
 * All of this information needs to be displayed to the user at some point.  It
 * cannot be reconstructed later because a successful login resets all these values
 * in the authcap file.
 */
	if(last_ulogin) {
		sprintf(buf, "Last successful login for %s: %s from %s\n",
			p->pw_name, ctime(last_ulogin), "???");
		MESSAGE(buf);
	}
	if(last_slogin) {
		sprintf(buf, "Last unsuccessful login for %s: %s from %s\n",
			p->pw_name, ctime(last_ulogin), "???");
		MESSAGE(buf);
	}
/*
 * At this point substantial code needs to be added to force a passord change if
 * required (expired password, empty password).
 */
#endif	/* SEC_BASE */

	verify->uid = p->pw_uid;
#ifdef NGROUPS_MAX
	getGroups (greet->name, verify, p->pw_gid);
#else
	verify->gid = p->pw_gid;
#endif
	home = p->pw_dir;
	shell = p->pw_shell;
	argv = 0;
	if (d->session)
		argv = parseArgs (argv, d->session);
	if (greet->string)
		argv = parseArgs (argv, greet->string);
	if (!argv)
		argv = parseArgs (argv, "xsession");
	verify->argv = argv;
	verify->userEnviron = userEnv (d, p->pw_uid == 0,
				       greet->name, home, shell);
	Debug ("user environment:\n");
	printEnv (verify->userEnviron);
	verify->systemEnviron = systemEnv (d, greet->name, home);
	Debug ("system environment:\n");
	printEnv (verify->systemEnviron);
	Debug ("end of environments\n");
	Debug("Calling getpwuid_r \n");
	result = getpwuid( verify->uid );
	Debug("setting login via setlogin \n");
	setlogin(result->pw_name);
	
	Debug("Returning VERIFY_SUCCESSFUL\n");
	return VERIFY_SUCCESSFUL;
}
#endif /* USE_SIA */

extern char **setEnv ();

char **
defaultEnv ()
{
    char    **env, **exp, *value;

    env = 0;
    for (exp = exportList; exp && *exp; ++exp)
    {
	value = getenv (*exp);
	if (value)
	    env = setEnv (env, *exp, value);
    }
    return env;
}

char **
userEnv (d, useSystemPath, user, home, shell)
struct display	*d;
int	useSystemPath;
char	*user, *home, *shell;
{
    char	**env;
    char	**envvar;
    char	*str;
    
    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "USER", user);
    env = setEnv (env, "LOGNAME", user);
    env = setEnv (env, "PATH", useSystemPath ? d->systemPath : d->userPath);
    env = setEnv (env, "SHELL", shell);
    for (envvar = envvars; *envvar; envvar++)
    {
	if (str = getenv(*envvar))
	    env = setEnv (env, *envvar, str);
    }
    return env;
}

char **
systemEnv (d, user, home)
struct display	*d;
char	*user, *home;
{
    char	**env;
    
    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    if (home)
	env = setEnv (env, "HOME", home);
    if (user) {
	env = setEnv (env, "USER", user);
	env = setEnv (env, "LOGNAME", user);
    }
    env = setEnv (env, "PATH", d->systemPath);
    env = setEnv (env, "SHELL", d->systemShell);
    if (d->authFile)
	    env = setEnv (env, "XAUTHORITY", d->authFile);
    return env;
}

#ifdef NGROUPS_MAX
groupMember (name, members)
char	*name;
char	**members;
{
	while (*members) {
		if (!strcmp (name, *members))
			return 1;
		++members;
	}
	return 0;
}

getGroups (name, verify, gid)
char			*name;
struct verify_info	*verify;
int			gid;
{
	int		ngroups;
	struct group	*g;
	int		i;

	ngroups = 0;
	verify->groups[ngroups++] = gid;
	setgrent ();
	/* SUPPRESS 560 */
	while (g = getgrent()) {
		/*
		 * make the list unique
		 */
		for (i = 0; i < ngroups; i++)
			if (verify->groups[i] == g->gr_gid)
				break;
		if (i != ngroups)
			continue;
		if (groupMember (name, g->gr_mem)) {
			if (ngroups >= NGROUPS_MAX)
				LogError ("%s belongs to more than %d groups, %s ignored\n",
					name, NGROUPS_MAX, g->gr_name);
			else
				verify->groups[ngroups++] = g->gr_gid;
		}
	}
	verify->ngroups = ngroups;
	endgrent ();
}
#endif

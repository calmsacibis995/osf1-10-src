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
static char	*sccsid = "@(#)$RCSfile: AuthUtils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:11 $";
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
#if SEC_BASE


/*
    filename:
        AuthUtils.c
        
    copyright:
        Copyright (c) 1989-1990 SKM, L.P.
        Copyright (c) 1989-1990 SecureWare Inc.
        Copyright (c) 1989-1990 MetaMedia Inc.
        ALL RIGHTS RESERVED

    function:
	Various non-X utilities to help accounts and devices
	Writes and reads the protected password/device databases
        
*/

#include <sys/types.h>

#include "UIMain.h"
#include "Accounts.h"
#include "AuthUtils.h"
#include "logging.h"

/* extern routines */
extern int
	IsPrIsso();

/* This should not be called in production version. Left in for future
 * debug */
void
DebugUser (pr)
	struct prpw_if pr;
{
}

int
GetUserInfo (user, pr)
	char *user;
	struct prpw_if *pr;
{
	struct passwd *pw;
	struct pr_passwd *prpwd;
	int	id;
	int 	i;

	if (user == (char *) NULL) {
		return (NULL_USER);
	}

	id = pw_nametoid(user);
	if (id == -1)
		return (NOT_USER);

	if ( (prpwd = getprpwnam(user)) == NULL) {
		audit_security_failure(OT_PRPWD, user,
		"Password entry exists, protected password entry doesn't",
		ET_SYS_ADMIN);
		return (NO_PRDB_ENTRY);
	}

	if (prpwd->uflg.fg_retired && prpwd->ufld.fd_retired)
		return (ACCT_RETIRED);

	if (!pr)
		return (NULL_PTR);
	pr->prpw = *prpwd;

	/* An ISSO cannot modify another ISSO account.
	   Only a sys admin can modify an ISSO account */
	if (role_program == ISSO) {
		if (IsPrIsso(prpwd))
			return (IS_ISSO);
	}
	else if (role_program == SYS_ADMIN) {
		if (! IsPrIsso(prpwd))
			return (NOT_ISSO);
	}
	/* otherwise they must have both privs and can do anything */

	return (SUCCESS);
}

int
GetDeviceInfo (device_name, dv)
	char *device_name;
	struct dev_if	*dv;
{
	struct dev_asg	*prdev1;
	struct dev_asg	*prdev;
	struct pr_term	*prterm;
	int 	i;

	if (device_name == (char *) NULL) {
		return (NULL_DEVICE);
	}

	if ( (prdev1 = getdvagnam(device_name)) == NULL) {
		audit_security_failure(OT_DEV_ASG, device_name,
		"No device entry exists in the database",
		ET_SYS_ADMIN);
		return (NO_DEVICE_ENTRY);
	}

	/* Have to copy this for safety */
	/* This should really be freed after use, currently this is not done */
	prdev = copydvagent(prdev1);

	/* If it is a terminal or host then get the other info */
#if SEC_NET_TTY
	if (prdev->uflg.fg_type && 
	      ( ISBITSET(prdev->ufld.fd_type, AUTH_DEV_TERMINAL) ||
		ISBITSET(prdev->ufld.fd_type, AUTH_DEV_REMOTE) ) ) {
#else
	if (prdev->uflg.fg_type && 
	      ISBITSET(prdev->ufld.fd_type, AUTH_DEV_TERMINAL) ) {
#endif

		if ( (prterm = getprtcnam(device_name)) == NULL) {
			audit_security_failure(OT_TERM_CNTL, device_name, 
		"Device assignment entry exists but corresponding terminal control entry missing",
			ET_SYS_ADMIN);

			return (NO_TERMINAL_ENTRY);
		}
		dv->tm  = *prterm;
	}
	else {
		/* Flag no terminal data available */
		memset(&dv->tm, '\0', sizeof(dv->tm));
	}
    
	/* This works for now but we should be more careful ! */
	dv->dev = *prdev;
	
	return (SUCCESS);
}

int
GetTerminalInfo (terminal_name, dv)
	char *terminal_name;
	struct dev_if	*dv;
{
	struct pr_term	*prterm;

	if (terminal_name == (char *) NULL) {
		return (NULL_TERMINAL);
	}

	if ( (prterm = getprtcnam(terminal_name)) == NULL) {
		audit_security_failure(OT_TERM_CNTL, terminal_name, 
		"Terminal control entry missing", ET_SYS_ADMIN);
		return (NO_TERMINAL_ENTRY);
	}

	/* This works for now but we should be more careful ! */
	dv->tm  = *prterm;
	
	return (SUCCESS);
}

int
GetSystemInfo (sd)
	struct sdef_if *sd;
{
	struct pr_default *df;
	int	id;
	int i;

	/* Get all system info */
	if ( (df = getprdfnam("default")) == NULL) {
		audit_security_failure(OT_DFLT_CNTL, "default", 
		"No system default entry", ET_SYS_ADMIN);
		return (NO_PRDB_ENTRY);
	}

	sd->df = *df;

	return (SUCCESS);
}

int
WriteUserInfo (pr)
	struct prpw_if *pr;
{
	struct pr_passwd *prpwd = &pr->prpw;

	ENTERLFUNC("WriteUserInfo");
	/* At this point we do not need to check if the user is an
	 * isso or not. If we have been able to read this account
	 * then we can also write it */
	/* Sanity check */
	if ( (! prpwd->ufld.fd_name) || (! prpwd->ufld.fd_name[0]) ) {
		puts("WriteUserInfo() is missing a user name");
		ERRLFUNC("WriteUserInfo", "missing user name");
		exit(1);
	}

	DUMPLVARS (" egid=<%d> gid=<%d>", getegid(), getgid(), NULL);
	if (putprpwnam (prpwd->ufld.fd_name, prpwd)) {
		audit_subsystem(prpwd->ufld.fd_name,
			"Successful update of protected password entry",
			ET_SYS_ADMIN);
		EXITLFUNC("WriteUserInfo");
		return(SUCCESS);
	}
	else {
		audit_security_failure(OT_PRPWD, prpwd->ufld.fd_name,
			"Unable to update protected password entry",
			ET_SYS_ADMIN);
		ERRLFUNC("WriteUserInfo", "can't update prot. passwd entry");
	    	return (CANT_UPDATE);
	}
}

int
WriteDeviceInfo (dv)
	struct dev_if	*dv;
{
	struct dev_asg *prdev;
	int 	i;

	prdev = &dv->dev;

	/* Write the device database */
	/* Sanity check */
	if ( (! prdev->ufld.fd_name) || (! prdev->ufld.fd_name[0]) ) {
		puts("WriteDeviceInfo() is missing a device name");
		exit(1);
	}

	if (putdvagnam (prdev->ufld.fd_name, prdev)) {
		audit_subsystem(prdev->ufld.fd_name,
			"Successful update of device assignment entry",
			ET_SYS_ADMIN);
		return(SUCCESS);
	}
	else {
		audit_security_failure(OT_DEV_ASG,prdev->ufld.fd_name,
			"Unable to update device assignment entry",
			ET_SYS_ADMIN);
	    	return (CANT_UPDATE_DEVICES);
	}
}

int
WriteTerminalInfo (dv)
	struct dev_if	*dv;
{
	struct pr_term	*prterm;
	int 	i;

	prterm = &dv->tm;
	setprtcent();

	/* Sanity check */
	if (! prterm->uflg.fg_devname) {
		puts("WriteTerminalInfo() is missing a terminal name");
		exit(1);
	}

	/* If a terminal or host then we need to update the ttys database */
	if (putprtcnam (prterm->ufld.fd_devname, prterm)) {
		audit_subsystem(prterm->ufld.fd_devname,
			"Successful update of terminal control entry",
			ET_SYS_ADMIN);
		return(SUCCESS);
	}
	else {
		audit_security_failure(OT_TERM_CNTL, prterm->ufld.fd_devname,
			"Unable to update terminal control entry",
			ET_SYS_ADMIN);
	    	return (CANT_UPDATE_TERMINAL);
	}
}

int
WriteSystemInfo (sd)
	struct sdef_if *sd;
{
	struct pr_default *sdef = &sd->df;

	if (putprdfnam ("default", sdef)) {
		audit_subsystem("default",
			"Successful update of default database entry",
			ET_SYS_ADMIN);
		return(SUCCESS);
	}
	else {
		audit_security_failure(OT_DFLT_CNTL, "default",
			"Unable to update default database entry",
			ET_SYS_ADMIN);
	    	return (CANT_UPDATE);
	}
}

#endif /* SEC_BASE */

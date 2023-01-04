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
static char	*sccsid = "@(#)$RCSfile: XAuthUtils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:01:47 $";
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
#include <sys/secdefines.h>
#if SEC_BASE
/* XXXX */
#define MAXUID 33333

/*
    filename:
        XAuthUtils.c
        
    copyright:
        Copyright (c) 1989-1990 SKM, L.P.
        Copyright (c) 1989-1990 SecureWare Inc.
        Copyright (c) 1989-1990 MetaMedia Inc.
        ALL RIGHTS RESERVED

    function:
	Various X based utilities to help accounts/ devices
	Handles writing to /from databases with error display

*/

/* Common C include files */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <stdio.h>

#ifndef rewinddir
#if defined(AUX)
#include	<sys/dir.h>
#define		dirent	direct
#else
#include	<dirent.h>
#endif
#endif /* rewinddir */
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#if AUX
#include <string.h>
#else
#include <strings.h>
#endif

#if SEC_NET_TTY
#include <netdb.h>
#endif

#include "UIMain.h"
#include "Accounts.h"
#include "Devices.h"

#ifdef X11
# include <Xm/Text.h>
# include <Xm/ToggleBG.h>
#else /* X11 */
# ifndef True
#  define True TRUE
# endif /* True */
# ifndef False
#  define False FALSE
# endif /* False */
#endif /* X11 */
#include "logging.h"

/* XXX */

/* file names for skeleton startup files for new user accounts. */

#ifdef SCO
	/* On the SCO platform there are two skeleton directories */
#define SKEL_DIR	"/usr/lib/mkuser/sh/"
#define SKEL_DIR_1	"/usr/lib/mkuser/csh/"
#else
#define SKEL_DIR	"/tcb/files/skel/"
#endif /* SCO */

/* programs that need to be exec'ed from this file */

#define COPY_PROGRAM	"/bin/cp"

/* modes on files that this program creates. */

#define HOMEDIRMODE	0750

/* limits for user account fields */


/* extern routines */
extern FILE	
	*popen_all_output(),
	*pclose_all_output();

#ifdef X11
extern void 
	WorkingClose();
#else /* X11 */
void 
	WorkingClose() {}
#endif /* X11 */

extern void 
	ErrorMessageOpen();

extern char
	*Calloc(),
	*Malloc(),
	*Realloc();

extern int 
	strcmp(),
	chooseuserid(),
	choosegroupid(),
	GetUserInfo(),
	WriteUserInfo(),
	GetDeviceInfo(),
	WriteDeviceInfo(),
	GetTerminalInfo(),
	WriteTerminalInfo(),
	GetSystemInfo();

/* local routines */
static FILE	*lockpwfile();

static int	updatepwfile(),
		mkhomedir(),
		ResetProcessSL(),
		SetProcessSLFromHomeDir(),
		XIsDeviceAlreadyListed(),
		XIsDeviceCharSpecial(),
		XIsNameNew();


#define GL_ALLOCATE
#include	"Messages.h"
#undef GL_ALLOCATE

int
XGetUserInfo (user, pr)
	char *user;
	struct prpw_if *pr;
{
	int 	ret;

	/* Get all user info and handles all the errors */
	switch (ret = GetUserInfo (user, pr) ) 
	{
	case NOT_USER:
		WorkingClose();
		no_form_present = True;
		if (! msg_not_user)
			LoadMessage ("msg_accounts_not_user",
				&msg_not_user, &msg_not_user_text);
		ErrorMessageOpen (-1, msg_not_user, 0, NULL);
		break;

	case NO_PRDB_ENTRY:
		WorkingClose();
		no_form_present = True;
		if (! msg_no_prdb_entry)
			LoadMessage ("msg_accounts_no_prdb_entry",
			&msg_no_prdb_entry, &msg_no_prdb_entry_text);
		ErrorMessageOpen (-1, msg_no_prdb_entry, 0, NULL);
		break;

	case ACCT_RETIRED:
		WorkingClose();
		no_form_present = True;
		if (! msg_acct_retired)
			LoadMessage ("msg_accounts_acct_retired",
			&msg_acct_retired, &msg_acct_retired_text);
		ErrorMessageOpen (-1, msg_acct_retired, 0, NULL);
		break;

	case IS_ISSO:
		WorkingClose();
		no_form_present = True;
		if (! msg_is_isso)
			LoadMessage ("msg_accounts_is_isso",
			&msg_is_isso, &msg_is_isso_text);
		ErrorMessageOpen (-1, msg_is_isso, 0, NULL);
		break;

	case NOT_ISSO:
		WorkingClose();
		no_form_present = True;
		if (! msg_not_isso)
			LoadMessage ("msg_accounts_not_isso",
			&msg_not_isso, &msg_not_isso_text);
		ErrorMessageOpen (-1, msg_not_isso, 0, NULL);
		break;

	case NULL_USER:
		WorkingClose();
		no_form_present = True;
		if (! msg_null_user)
			LoadMessage ("msg_accounts_null_user",
			&msg_null_user, &msg_null_user_text);
		ErrorMessageOpen (-1, msg_null_user, 0, NULL);
		break;

	case CANT_READ_AUDIT:
		WorkingClose();
		no_form_present = True;
		if (! msg_cant_read_audit)
			LoadMessage ("msg_accounts_cant_read_audit",
			&msg_cant_read_audit, &msg_cant_read_audit_text);
		ErrorMessageOpen (-1, msg_cant_read_audit, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Program Error \n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}

int
XGetDeviceInfo (device, dv)
	char *device;
	struct dev_if *dv;
{
	int 	ret;

	/* Get all user info and handles all the errors */
	switch (ret = GetDeviceInfo (device, dv) ) 
	{
	case NO_DEVICE_ENTRY:
		WorkingClose();
		no_form_present = True;
		if (! msg_no_device_entry)
			LoadMessage ("msg_devices_no_device_entry",
			&msg_no_device_entry, &msg_no_device_entry_text);
		ErrorMessageOpen (-1, msg_no_device_entry, 0, NULL);
		break;

	case NO_TERMINAL_ENTRY:
		WorkingClose();
		no_form_present = True;
		if (! msg_no_terminal_entry)
			LoadMessage ("msg_devices_no_terminal_entry",
			&msg_no_device_entry, &msg_no_terminal_entry_text);
		ErrorMessageOpen (-1, msg_no_terminal_entry, 0, NULL);
		break;

	case NULL_DEVICE:
		WorkingClose();
		no_form_present = True;
		if (! msg_null_device)
			LoadMessage ("msg_devices_null_device",
			&msg_null_device, &msg_null_device_text);
		ErrorMessageOpen (-1, msg_null_device, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Program Error \n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}

int
XGetTerminalInfo (terminal, dv)
	char *terminal;
	struct dev_if *dv;
{
	int 	ret;

	/* Get all user info and handles all the errors */
	switch (ret = GetTerminalInfo (terminal, dv) ) 
	{
	case NO_TERMINAL_ENTRY:
		WorkingClose();
		no_form_present = True;
		if (! msg_no_terminal_entry)
			LoadMessage ("msg_devices_no_terminal_entry",
			&msg_no_device_entry, &msg_no_terminal_entry_text);
		ErrorMessageOpen (-1, msg_no_terminal_entry, 0, NULL);
		break;

	case NULL_TERMINAL:
		WorkingClose();
		no_form_present = True;
		if (! msg_null_terminal)
			LoadMessage ("msg_devices_null_terminal",
			&msg_null_terminal, &msg_null_terminal_text);
		ErrorMessageOpen (-1, msg_null_terminal, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Program Error \n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}

#if SEC_NET_TTY
int
XGetHostInfo (host, dv)
	char *host;
	struct dev_if *dv;
{
	int 	ret;

	/* Get all host info and handles all the errors */
	/* Piggy back on the GetTerminalInfo routine for Host Info */
	switch (ret = GetTerminalInfo (host, dv) ) 
	{
	case NO_TERMINAL_ENTRY:
		WorkingClose();
		no_form_present = True;
		if (! msg_no_host_entry)
			LoadMessage ("msg_devices_no_host_entry",
			&msg_no_device_entry, &msg_no_host_entry_text);
		ErrorMessageOpen (-1, msg_no_host_entry, 0, NULL);
		break;

	case NULL_TERMINAL:
		WorkingClose();
		no_form_present = True;
		if (! msg_null_host)
			LoadMessage ("msg_devices_null_host",
			&msg_null_host, &msg_null_host_text);
		ErrorMessageOpen (-1, msg_null_host, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Program Error \n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}
#endif /* SEC_NET_TTY */

int
XGetSystemInfo (sd)
	struct sdef_if *sd;
{
	int 	ret;

	/* Get all system info and handle all errors */
	switch (ret = GetSystemInfo (sd) ) 
	{
	case NO_PRDB_ENTRY:
		WorkingClose();
		no_form_present = True;
		if (! msg_no_default_entry)
			LoadMessage ("msg_accounts_no_default_entry",
					&msg_no_default_entry, 
					&msg_no_default_entry_text);
		ErrorMessageOpen (-1, msg_no_default_entry, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Error in switch statement for user\n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}
	return (ret);
}

int
XWriteUserInfo (pr)
	struct prpw_if *pr;
{
	int 	ret;

	switch (ret = WriteUserInfo(pr) ) 
	{
	case CANT_UPDATE:
		WorkingClose();
		if (! msg_cant_update_user)
			LoadMessage ("msg_accounts_cant_update_user",
					&msg_cant_update_user, 
					&msg_cant_update_user_text);
		ErrorMessageOpen (-1, msg_cant_update_user, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Error in switch statement for user\n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}

int
XWriteDeviceAndTerminalInfo (dv)
	struct dev_if *dv;
{
	int 	ret;

	switch (ret = WriteDeviceInfo(dv) ) 
	{
	case CANT_UPDATE_DEVICES:
		WorkingClose();
		if (! msg_cant_update_devices)
			LoadMessage ("msg_devices_cant_update_devices",
					&msg_cant_update_devices, 
					&msg_cant_update_devices_text);
		ErrorMessageOpen (-1, msg_cant_update_devices, 0, NULL);
		break;

	case SUCCESS:
		ret = XWriteTerminalInfo(dv);
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Error in switch statement for user\n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}

int
XWriteDeviceInfo (dv)
	struct dev_if *dv;
{
	int 	ret;

	/* Write the device database */
	switch (ret = WriteDeviceInfo(dv) ) 
	{
	case CANT_UPDATE_DEVICES:
		WorkingClose();
		if (! msg_cant_update_devices)
			LoadMessage ("msg_devices_cant_update_devices",
					&msg_cant_update_devices, 
					&msg_cant_update_devices_text);
		ErrorMessageOpen (-1, msg_cant_update_devices, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Error in switch statement for user\n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}

int
XWriteTerminalInfo (dv)
	struct dev_if *dv;
{
	int 	ret;

	switch (ret = WriteTerminalInfo(dv) ) 
	{
	case CANT_UPDATE_TERMINAL:
		WorkingClose();
		if (! msg_cant_update_terminal)
			LoadMessage ("msg_devices_cant_update_terminal",
					&msg_cant_update_terminal, 
					&msg_cant_update_terminal_text);
		ErrorMessageOpen (-1, msg_cant_update_terminal, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Error in switch statement for user\n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}

#if SEC_NET_TTY
int
XWriteHostInfo (dv)
	struct dev_if *dv;
{
	int 	ret;

	/* Use the WriteTerminalInfo routine - it uses same code as
	 * a host routine would */
	switch (ret = WriteTerminalInfo(dv) ) 
	{
	case CANT_UPDATE_TERMINAL:
		WorkingClose();
		if (! msg_cant_update_host)
			LoadMessage ("msg_devices_cant_update_host",
					&msg_cant_update_host, 
					&msg_cant_update_host_text);
		ErrorMessageOpen (-1, msg_cant_update_host, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Error in switch statement for user\n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}

	return (ret); /* Return value */
}
#endif /* SEC_NET_TTY */

int
XWriteSystemInfo (sd)
	struct sdef_if *sd;
{
	int 	ret;

	/* Write system info back to database. Handle all errors */

	switch (ret = WriteSystemInfo(sd) ) 
	{
	case CANT_UPDATE:
		WorkingClose();
		if (! msg_cant_update)
			LoadMessage ("msg_accounts_cant_update",
					&msg_cant_update, 
					&msg_cant_update_text);
		ErrorMessageOpen (-1, msg_cant_update, 0, NULL);
		break;

	case SUCCESS:
		break;

	default:
		/* Should never get here */
		fprintf (_SWlogfp, "Error in switch statement for user\n");
		fprintf (_SWlogfp, "Contact your supplier\n");
		restorescreen ();
		exit(1);
	}
	return (ret);
}

/* Returns true if it is a special device */
static int
XIsDeviceCharSpecial (device_name)
	char *device_name;
{
	struct stat sb;

#ifdef DEBUG
	fprintf (_SWlogfp, "XIsDeviceCharSpecial: name= %s\n", device_name);
#endif
	/* Check for null name */
	if (! device_name[0]) {
		if (! msg_devices_error_1)
			LoadMessage ("msg_devices_add_error_null_device",
			       &msg_devices_error_1, &msg_devices_error_1_text);
		ErrorMessageOpen (-1, msg_devices_error_1, 0, NULL);
		return (0);
	}

	/* Check we can stat the file */
	if (stat(device_name, &sb) < 0) {
		if (! msg_devices_error_2)
			LoadMessage ("msg_devices_add_error_cant_stat",
			       &msg_devices_error_2, &msg_devices_error_2_text);
		ErrorMessageOpen (-1, msg_devices_error_2, 0, device_name);
		return (0);
	}
	
	/* Check it is a special character or block device */
	if ( !(S_ISCHR(sb.st_mode)) && !(S_ISBLK(sb.st_mode)) ) {
		if (! msg_devices_error_3)
			LoadMessage ("msg_devices_add_error_not_special",
			       &msg_devices_error_3, &msg_devices_error_3_text);
		ErrorMessageOpen (-1, msg_devices_error_3, 0, device_name);
		return (0);
	}
	return(1);
}

/* Returns true if this is a host name */
static int
XIsDeviceHost (device_name)
	char *device_name;
{
	return (1);
}

static int
XIsNameNew (dev_name)
	char	*dev_name;
{
	struct dev_asg	*dv;

	setdvagent();
	while ( (dv=getdvagent()) != (struct dev_asg *) 0) {
	    if (dv->uflg.fg_name && (strcmp(dv->ufld.fd_name,dev_name) == 0)) {
			    if (! msg_devices_error_5)
			       LoadMessage ("msg_devices_add_error_old_name",
      			       &msg_devices_error_5, &msg_devices_error_5_text);
			    ErrorMessageOpen (-1, msg_devices_error_5, 0, NULL);
			    return(1);
	    }
	}
	return(0);
}

static int
XIsDeviceAlreadyListed (dev_name, dev_list, dev_list_count)
	char	*dev_name;
	char **dev_list;
	int	dev_list_count;
{
	struct dev_asg	*dv;
	int i,j;
	char	*cp;

#ifdef DEBUG
	fprintf (_SWlogfp, "XIsDeviceAlreadyListed\n
Device name %s count %d\n", dev_name, dev_list_count);
#endif
	setdvagent();
	while ( (dv=getdvagent()) != (struct dev_asg *) 0) {
	    fprintf (_SWlogfp, "a\n");
	    if (dv->uflg.fg_devs && strcmp(dv->ufld.fd_name,dev_name) ) {
	    fprintf (_SWlogfp, "b\n");
		for (j=0; j<dev_list_count; j++) {
	    fprintf (_SWlogfp, "c\n");
		    for (i=0; cp = dv->ufld.fd_devs[i]; i++) {
	    fprintf (_SWlogfp, "d\n");
			if (strcmp(dev_list[j], cp) == 0) {
	    fprintf (_SWlogfp, "e\n");
			    if (! msg_devices_error_4)
			       LoadMessage ("msg_devices_add_error_is_listed",
      			       &msg_devices_error_4, &msg_devices_error_4_text);
			    ErrorMessageOpen (-1, msg_devices_error_4, 0, 
					dev_list[j]);
			    return(1);
			}
		    }
		}
	    }
	}
	return(0);
}

/* 
 * Start of routines for creating/maintaining a user account that
 * are non-security related
 */

int
CheckPasswordAccess()
{
	char	*user;
	char	passwd_copy[PASSWD_LEN + 1];
	char	*strrchr();

	/* check that the program can write in the parent directory of
	 * the password file */
	strcpy (passwd_copy, passwd);
	user = strrchr (passwd_copy, '/');
	*user = '\0';
	if (eaccess (passwd_copy, 3) != 0) {  /* need write and search */
		audit_no_resource (passwd, OT_PWD,
		  "No write permission in parent directory", ET_SYS_ADMIN);
		return (FAILURE);
	}
	return(SUCCESS);
}


/* check that it's a valid user name: no ':', no upper case as 1st char,
 * no digit as first char (messes up chmod and chown), no '/' (messes up
 * home directory).
 */
int
CheckValidName(user)
char	*user;
{
	if (! user)
		return (FAILURE);
	if (strchr (user, ':') ||
	    isupper (user[0])  ||
	    isdigit (user[0])  ||
	    strchr (user, '/'))
		return (FAILURE);
	return (SUCCESS);
}

int
UserNameExists(user)
char	*user;
{
	if (getpwnam (user) != (struct passwd *) 0)
		return (FAILURE);
	else
		return (SUCCESS);
}


int
CheckValidUid(userid)
int	userid;
{
	if (userid <= 0 || userid >= MAXUID)
		return (FAILURE);
	else
		return (SUCCESS);
}
int
UserUidExists(userid)
int	userid;
{
	if (userid && (getpwuid (userid) != (struct passwd *) 0) )
		return (FAILURE);
	else
		return (SUCCESS);
}

/* check the home directory for parent searchable, home dir doesn't exist */
int
CheckValidHomeDir(newpwd)
struct passwd	*newpwd;
{
	struct	stat	sb;
	register char	*cp;
	char	*thome;
	int	cantsearch;
	char	*homedir;
	int	userid;
	int	groupid;

	/*
	void	homedirexists(), badhomedir();
	*/
	homedir = newpwd->pw_dir;
	userid  = (int) newpwd->pw_uid;
	groupid = (int) newpwd->pw_gid;
	
	if (stat (homedir, &sb) == 0) {  /* exists */
		return (FAILURE);
	}

	/* cannot contain a : */
	if (strchr (homedir, ':') )
		return (FAILURE);

	thome = (char *) Malloc (strlen (homedir) + 1);
	if (! thome)
		MemoryError();
	strcpy (thome, homedir);
	cp = strrchr (thome, '/');
	if (cp == NULL)  {
		Free (thome);
		return (FAILURE);
	}

	/* must be able to write home directory parent */
	*cp = '\0';
	if (eaccess (thome, 2) != 0) {
		/*
		pop_msg (
		"To create home directory, this program must have write",
		"permission to the directory's parent.");
		*/
		Free (thome);
		return (FAILURE);
	}
	*cp = '/';

	/* if user or group not assigned yet, return OK */
	if (userid == 0 || groupid == 0)  {
		Free (thome);
		return (SUCCESS);
	}

	cantsearch = 0;
	do {
		*cp = '\0';
		if (stat (thome, &sb) != 0) {
			/* badhomedir (homedir); */
			Free (thome);
			return (FAILURE);
		}
		if (userid == sb.st_uid)
			if ((sb.st_mode & S_IEXEC) == 0)
				cantsearch = 1;
			else	cantsearch = 0;
		else if (groupid == sb.st_gid)
			if ((sb.st_mode & (S_IEXEC>>3)) == 0)
				cantsearch = 1;
			else	cantsearch = 0;
		else if ((sb.st_mode & (S_IEXEC >> 6)) == 0)
			cantsearch = 1;
		else	cantsearch = 0;
	} while (cantsearch == 0 && (cp = strrchr (thome, '/')) && cp != thome);
	
	Free (thome);

	if (cantsearch)  {
		/* badhomedir (homedir); */
		return (FAILURE);
	}
	return (SUCCESS);
}


/* check a user's shell
 */

int
CheckValidShell(shellname)
char	*shellname;
{
	struct	stat	sb;

	/* NULL shell field means just use the default */
	if (shellname == '\0')
		return (FAILURE);

	/* check that shell exists */
	/* check that shell is a regular file */
	/* check that shell is executable by someone */
	if (stat (shellname, &sb) < 0 ||
	    (sb.st_mode & S_IFMT) != S_IFREG ||
	    (sb.st_mode & (S_IEXEC | (S_IEXEC >> 3) | (S_IEXEC >> 6))) == 0) {
		return (FAILURE);
	}

	/* Shell cannot contain a : */
	if (strchr (shellname, ':') )
		return (FAILURE);

	return (SUCCESS);
}

/* check a user's gecos field
 * returns 1 on success, 0 on failure
 */

int
CheckValidComment (gecos)
char	*gecos;
{

	/* check that there aren't any ':' characters */
	if (strchr (gecos, ':'))  {
		return (FAILURE);
	}
	return (SUCCESS);
}

int
XModifyUserpwd (newpwd)
struct	passwd	*newpwd;
{
	FILE	*fp;
	struct  pr_passwd *pr;
	int	ret;

	ENTERLFUNC("XModifyUserpwd");
	/* lock the password file */

	if ((fp = lockpwfile ()) == NULL)  {
		audit_no_resource (passwd, OT_PWD,
		  "Cannot lock password file", ET_SYS_ADMIN);
		if (! msg_error_lock_passwd_file)
			LoadMessage("msg_accounts_make_user_lock_passwd_file",
				&msg_error_lock_passwd_file, 
				&msg_error_lock_passwd_file_text);
		ErrorMessageOpen(-1, msg_error_lock_passwd_file, 0, NULL);
		ERRLFUNC("XModifyUserpwd", "passwd lock failed");
		return (FAILURE);
	}

	if (updatepwfile (newpwd, fp) != SUCCESS) {
		audit_security_failure (OT_PWD, newpwd->pw_name,
		  "Password file update unsuccessful", ET_SYS_ADMIN);
		if (! msg_error_cant_update_passwd)
			LoadMessage("msg_accounts_make_user_cant_update_passwd",
				&msg_error_cant_update_passwd, 
				&msg_error_cant_update_passwd_text);
		ErrorMessageOpen(-1, msg_error_cant_update_passwd, 0, NULL);
		ERRLFUNC("XModifyUserpwd", "passwd update failed");
		return (FAILURE);
	}

	EXITLFUNC("XModifyUserpwd");
	return (SUCCESS);
}

int
XCreateUserAccount (newpwd)
struct	passwd	*newpwd;
{
	FILE	*fp;
	char	buf1[80];
	char	buf2[80];
	struct	stat	sb;
	char	*targv[4];
	struct  dirent	*dp;
	int	good_stat;
	DIR	*skel_dir;
	struct	prpw_if  prpwd;
	struct  pr_passwd *pr;
	int	ret;
#if SEC_MAC_OB || SEC_CMW
	mand_ir_t	*old_process_sl_ir;
	int	process_sl_changed;
#endif /* SEC_MAC_OB || SEC_CMW */
	int i;

#ifdef DEBUG
	fprintf (_SWlogfp, "XCreateUserAccount\n");
#endif

	/* lock the password file */
	ret = XModifyUserpwd(newpwd);
	if (ret != SUCCESS)
		return (ret);

#if SEC_MAC_OB || SEC_CMW
	/* Allocate space to hold the old process sl ir if changed */
	old_process_sl_ir = mand_alloc_ir();
	if (! old_process_sl_ir)
		MemoryError();
	ret = SetProcessSLFromHomeDir (newpwd->pw_dir, 
		&process_sl_changed, old_process_sl_ir);
	fprintf (_SWlogfp, "process_sl_4changed = %d\n", process_sl_changed);
	outsl(old_process_sl_ir);
	if (ret != SUCCESS)
		return (ret);
#endif /* SEC_MAC_OB || SEC_CMW */

	/* Make home directory, change permissions */
	if (mkhomedir (newpwd->pw_dir, geteuid(), getegid()) != SUCCESS) {
		audit_no_resource (newpwd->pw_dir, OT_DIRECTORY,
		  "Unsuccessful create of user home directory", ET_SYS_ADMIN);
		if (! msg_error_cant_create_home_dir)
			LoadMessage("msg_accounts_make_user_cant_create_home_dir",
				&msg_error_cant_create_home_dir, 
				&msg_error_cant_create_home_dir_text);
		ErrorMessageOpen(-1, msg_error_cant_create_home_dir, 0, NULL);
		/* Should remove entry from file .... (later versions) */
		ret = FAILURE;
		goto no_home_dir;
	}

	if (stat (SKEL_DIR, &sb) < 0) {
		if (! msg_error_cant_stat_skel)
			LoadMessage("msg_accounts_make_user_cant_stat_skel",
				&msg_error_cant_stat_skel, 
				&msg_error_cant_stat_skel_text);
		ErrorMessageOpen(-1, msg_error_cant_stat_skel, 0, NULL);
		/* Should remove entry from file .... (later versions) */
		ret = FAILURE;
		goto no_home_dir;
	}

	/*
	 * Copy all files from SKEL_DIR to the user's home directory.
	 * In the process, prepend a `.' to each name.  The new files
	 * are owned by the user, with owner permissions inherited
	 * from the original file (the other permissions are dropped).
	 */
	skel_dir = (DIR *) opendir(SKEL_DIR);
	if (skel_dir == (DIR *) 0)  {
		chown (newpwd->pw_dir, newpwd->pw_uid, newpwd->pw_gid);
		if (! msg_error_cant_open_skel)
			LoadMessage("msg_accounts_make_user_cant_open_skel",
				&msg_error_cant_open_skel, 
				&msg_error_cant_open_skel_text);
		ErrorMessageOpen(-1, msg_error_cant_open_skel, 0, NULL);
		/* Should remove entry from file .... (later versions) */
		ret = FAILURE;
		goto no_home_dir;
	}

	targv[0] = strrchr (COPY_PROGRAM, '/') + 1;
	targv[1] = buf1;
	targv[2] = buf2;
	targv[3] = (char *) 0;

	while ((dp = readdir(skel_dir)) != (struct dirent *) 0)
		if ((strcmp(dp->d_name, ".") != 0) &&
		    (strcmp(dp->d_name, "..") != 0))  {
			(void) sprintf (buf1, "%s%s", SKEL_DIR, dp->d_name);
			good_stat = stat(buf1, &sb);
			(void) sprintf (buf2, "%s/.%s",
					newpwd->pw_dir, dp->d_name);
			fp = popen_all_output (COPY_PROGRAM, targv);
			if (fp != (FILE *) 0)
				pclose_all_output (fp);
			if (good_stat == 0)
				chmod (targv[2], 0700 & sb.st_mode);
			else
				chmod (targv[2], 0600);
			chown (targv[2], newpwd->pw_uid, newpwd->pw_gid);
		}

	chmod (newpwd->pw_dir, 0700);
	chown (newpwd->pw_dir, newpwd->pw_uid, newpwd->pw_gid);
	closedir(skel_dir);

#ifdef SCO
	/* There are two skeleton directories on the SCO platform */
	if (stat (SKEL_DIR_1, &sb) < 0) {
		if (! msg_error_cant_stat_skel)
			LoadMessage("msg_accounts_make_user_cant_stat_skel",
				&msg_error_cant_stat_skel, 
				&msg_error_cant_stat_skel_text);
		ErrorMessageOpen(-1, msg_error_cant_stat_skel, 0, NULL);
		/* Should remove entry from file .... (later versions) */
		ret = FAILURE;
		goto no_home_dir;
	}

	/*
	 * Copy all files from SKEL_DIR_1 to the user's home directory.
	 * In the process, prepend a `.' to each name.  The new files
	 * are owned by the user, with owner permissions inherited
	 * from the original file (the other permissions are dropped).
	 */
	skel_dir = opendir(SKEL_DIR_1);
	if (skel_dir == (DIR *) 0)  {
		chown (newpwd->pw_dir, newpwd->pw_uid, newpwd->pw_gid);
		if (! msg_error_cant_open_skel)
			LoadMessage("msg_accounts_make_user_cant_open_skel",
				&msg_error_cant_open_skel, 
				&msg_error_cant_open_skel_text);
		ErrorMessageOpen(-1, msg_error_cant_open_skel, 0, NULL);
		/* Should remove entry from file .... (later versions) */
		ret = FAILURE;
		goto no_home_dir;
	}

	targv[0] = strrchr (COPY_PROGRAM, '/') + 1;
	targv[1] = buf1;
	targv[2] = buf2;
	targv[3] = (char *) 0;

	while ((dp = readdir(skel_dir)) != (struct dirent *) 0)
		if ((strcmp(dp->d_name, ".") != 0) &&
		    (strcmp(dp->d_name, "..") != 0))  {
			(void) sprintf (buf1, "%s%s", SKEL_DIR_1, dp->d_name);
			good_stat = stat(buf1, &sb);
			(void) sprintf (buf2, "%s/.%s",
					newpwd->pw_dir, dp->d_name);
			fp = popen_all_output (COPY_PROGRAM, targv);
			if (fp != (FILE *) 0)
				pclose_all_output (fp);
			if (good_stat == 0)
				chmod (targv[2], 0700 & sb.st_mode);
			else
				chmod (targv[2], 0600);
			chown (targv[2], newpwd->pw_uid, newpwd->pw_gid);
		}

	chmod (newpwd->pw_dir, 0700);
	chown (newpwd->pw_dir, newpwd->pw_uid, newpwd->pw_gid);
	closedir(skel_dir);
#endif /* SCO */

	/* create a protected password entry for that user (all defaults). */
	/* clear out the structure to all zeros (defaults) */
	pr = &prpwd.prpw;
	memset((char *) pr, '\0', sizeof(*pr));

	(void) strcpy (pr->ufld.fd_name, newpwd->pw_name);
	pr->uflg.fg_name = 1;
	pr->ufld.fd_uid = newpwd->pw_uid;
	pr->uflg.fg_uid = 1;

	ret = XWriteUserInfo (&prpwd);

no_home_dir:
#if SEC_MAC_OB || SEC_CMW
	fprintf (_SWlogfp, "reset stuff \n");
	fprintf (_SWlogfp, "process_sl_6changed = %d\n", process_sl_changed);
	fprintf (_SWlogfp, "True = %d\n", True);
		outsl(old_process_sl_ir);
	if (process_sl_changed == True) {
		outsl(old_process_sl_ir);
		 i = ResetProcessSL (old_process_sl_ir);
	}
	mand_free_ir(old_process_sl_ir);
#endif /* SEC_MAC_OB || SEC_CMW */

	return (ret);
}

#if SEC_MAC_OB || SEC_CMW
/* 
 * When creating the user's home directory we want to set the SL of the
 * directory to a good value. We choose this as follows :
 *   If the process SL dominates the SL of the directory containing the
 *   user's home directory then do nothing.
 *
 *   If the directory containing the user's home directory dominates the
 *   process SL then change the process SL to the level of the user's
 *   home directory parent.
 */

outsl(ir)
	mand_ir_t       *ir;
{
	char    *s;

	s=mand_ir_to_er(ir);
	fprintf (_SWlogfp, "SL=%s\n", s);
}

static int
SetProcessSLFromHomeDir (dirname, process_sl_changed, old_process_sl_ir)
	char	*dirname;
	int	*process_sl_changed;
	mand_ir_t	*old_process_sl_ir;
{
	mand_ir_t	*process_sl_ir;
	mand_ir_t	*parent_sl_ir;
	mand_ir_t	*ir;
	char		*parent;
	char		*c;
	int		i,j,k;

#ifdef DEBUG
fprintf (_SWlogfp, "SetProcessSLFromHomeDir \n");
#endif
	*process_sl_changed = False;
	fprintf (_SWlogfp, "process_sl_1changed = %d\n", *process_sl_changed);
	fprintf (_SWlogfp, "Set SL From Home Dir  = %s\n", dirname);
	c = strrchr (dirname, '/');
	fprintf (_SWlogfp, "strelns %d %d\n" ,strlen(dirname), strlen(c));
	/* If there is a '/' and it is not the only one */
	if (c && (strlen(c) != strlen(dirname) ) ) {
		i = strlen(dirname) - strlen(c);
		parent = (char *) Malloc (i + 1);
		if (! parent)
			MemoryError();
		strncpy (parent, dirname, i);
		parent[i] = '\0';
		fprintf (_SWlogfp, "parent= %s len=%d dirname=%s i=%d\n", parent,
			strlen(parent) , dirname, i);
	}
	else {
		i = 1;
		parent = (char *) Malloc (i + 1);
		if (! parent)
			MemoryError();
		strncpy (parent, "/", i);
		parent[i] = '\0';
		fprintf (_SWlogfp, "PARENT= %s len=%d dirname=%s i=%d\n", parent,
			strlen(parent) , dirname, i);
	}

	fprintf (_SWlogfp, "i c %d %s\n", i,c);
	fprintf (_SWlogfp, "parent %s\n", parent);
	fprintf (_SWlogfp, "strlen(arn) %d\n", strlen(parent));
	/* Get the SL of the parent directory */
	parent_sl_ir = mand_alloc_ir();
	if (! parent_sl_ir)
		MemoryError();
	
	j = statslabel (parent, parent_sl_ir);
	outsl (parent_sl_ir);
	Free (parent);
	/* Check for errors in setting the process sl */
	if (j != 0) {
		switch (errno) {

		case EACCES:
			if (! msg_error_mkdir_eacces)
				LoadMessage ("msg_accounts_error_mkdir_eacces",
					&msg_error_mkdir_eacces,
					&msg_error_mkdir_eacces_text);
			ErrorMessageOpen(-1, msg_error_mkdir_eacces, 0, NULL);
			break;

		case EFAULT:
			if (! msg_error_mkdir_efault)
				LoadMessage ("msg_accounts_error_mkdir_efault",
					&msg_error_mkdir_efault,
					&msg_error_mkdir_efault_text);
			ErrorMessageOpen(-1, msg_error_mkdir_efault, 0, NULL);
			break;

		case EINTR:
			if (! msg_error_mkdir_eintr)
				LoadMessage ("msg_accounts_error_mkdir_eintr",
					&msg_error_mkdir_eintr,
					&msg_error_mkdir_eintr_text);
			ErrorMessageOpen(-1, msg_error_mkdir_eintr, 0, NULL);
			break;

		case EINVAL:
			if (! msg_error_mkdir_einval)
				LoadMessage ("msg_accounts_error_mkdir_einval",
					&msg_error_mkdir_einval,
					&msg_error_mkdir_einval_text);
			ErrorMessageOpen(-1, msg_error_mkdir_einval, 0, NULL);
			break;

		case ENOTDIR:
			if (! msg_error_mkdir_enotdir)
				LoadMessage ("msg_accounts_error_mkdir_enotdir",
					&msg_error_mkdir_enotdir,
					&msg_error_mkdir_enotdir_text);
			ErrorMessageOpen(-1, msg_error_mkdir_enotdir, 0, NULL);
			break;

		case ENOENT:
			if (! msg_error_mkdir_enoent)
				LoadMessage ("msg_accounts_error_mkdir_enoent",
					&msg_error_mkdir_enoent,
					&msg_error_mkdir_enoent_text);
			ErrorMessageOpen(-1, msg_error_mkdir_enoent, 0, NULL);
			break;

		case EREMOTE:
			if (! msg_error_mkdir_eremote)
				LoadMessage ("msg_accounts_error_mkdir_eremote",
					&msg_error_mkdir_eremote,
					&msg_error_mkdir_eremote_text);
			ErrorMessageOpen(-1, msg_error_mkdir_eremote, 0, NULL);
			break;

		default:
			if (! msg_error_mkdir_error)
				LoadMessage ("msg_accounts_error_mkdir_error",
					&msg_error_mkdir_error,
					&msg_error_mkdir_error_text);
			ErrorMessageOpen(-1, msg_error_mkdir_error, 0, NULL);
		}
		mand_free_ir (parent_sl_ir);
		return (FAILURE);
	}

	/* Get the process SL */
	process_sl_ir = mand_alloc_ir();
	if (! process_sl_ir)
		MemoryError();
	
	j = getslabel(process_sl_ir);
	if (j != 0) {
		if (! msg_error_mkdir_cant_getslabel)
			LoadMessage ("msg_accounts_error_mkdir_cant_getslabel",
				&msg_error_mkdir_cant_getslabel,
				&msg_error_mkdir_cant_getslabel_text);
		ErrorMessageOpen(-1, msg_error_mkdir_cant_getslabel, 0, NULL);
		return (FAILURE);
	}

	/* Determine relationship between the process SL and the parent SL */
	fprintf (_SWlogfp, "mand_ir_relat\n");
	outsl (process_sl_ir);
	outsl(parent_sl_ir);
	k = mand_ir_relationship(process_sl_ir, parent_sl_ir);
	fprintf (_SWlogfp, "k %d\n", k);
	fprintf (_SWlogfp, "dominances %d %d\n", MAND_ODOM, MAND_INCOMP);

	/* If the directory dominates or is incomparable change process SL
	 * so we can create the user's home directory */
	if (k & (MAND_ODOM | MAND_INCOMP) ) {
		fprintf (_SWlogfp, "HI\n");
		mand_copy_ir (process_sl_ir, old_process_sl_ir);
		fprintf (_SWlogfp, "Copied process away\n");
		outsl(old_process_sl_ir);
		/* Change process SL */
		j = setslabel (parent_sl_ir);
		fprintf (_SWlogfp, "j from setslabel %d\n", j);
		if (j != 0) {
			fprintf (_SWlogfp, "errors so freeing stuff\n");
			mand_free_ir(old_process_sl_ir);
			mand_free_ir(parent_sl_ir);
			if (! msg_error_mkdir_change_sl)
				LoadMessage ("msg_accounts_error_mkdir_change_sl",
					&msg_error_mkdir_change_sl,
					&msg_error_mkdir_change_sl_text);
			ErrorMessageOpen(-1, msg_error_mkdir_change_sl, 
				0, NULL);
			return (FAILURE);
		}
		fprintf (_SWlogfp, "Changed process sl\n");
		*process_sl_changed = True;
	fprintf (_SWlogfp, "process_sl_2changed = %d\n", *process_sl_changed);
		outsl (old_process_sl_ir);
	}
	fprintf (_SWlogfp, "process_sl_3changed = %d\n", *process_sl_changed);
	return (SUCCESS);
}

/* Reset the process SL if it was changed during the makedir */
static int
ResetProcessSL(old_process_sl_ir)
	mand_ir_t	*old_process_sl_ir;
{
	int i;

	fprintf (_SWlogfp, "reset process sl\n");
	outsl(old_process_sl_ir);
	i = setslabel (old_process_sl_ir);
	fprintf (_SWlogfp, "i from setslabel %d\n", i);
	if (i != 0) {
		if (! msg_error_mkdir_reset_sl)
			LoadMessage ("msg_accounts_error_mkdir_reset_sl",
				&msg_error_mkdir_reset_sl,
				&msg_error_mkdir_reset_sl_text);
		ErrorMessageOpen(-1, msg_error_mkdir_reset_sl, 0, NULL);
		return (FAILURE);
	}
	return (SUCCESS);
}
#endif /* SEC_MAC_OB || SEC_CMW */


/* lock the password file.
 * Return a file pointer to the open temp file on success, NULL on failure.
 */

static
FILE	*
lockpwfile ()
{
	int	i = 0;
	FILE	*tf;
#ifdef DEBUG
fprintf (_SWlogfp, "lockpwfile\n");
#endif
	while (eaccess (temp, 0) >= 0 && i < 30) {
		sleep (1);
		i++;
	}
	if (i == 30 || (tf = fopen (temp, "w")) == (FILE *) 0) {
		return ((FILE *) 0);
	}
#ifdef DEBUG
fprintf (_SWlogfp, "lockpwfile successful\n");
#endif
	return (tf);
}

/* update password file with all changes */

static int
updatepwfile (newpwd, tf)
struct	passwd	*newpwd;
FILE	*tf;
{
	struct	passwd	*pwd;
	int	found;
	struct	stat	sb, dsb, psb;

	ENTERLFUNC("updatepwfile");
	setpwent();
	found = 0;
	while (pwd = getpwent())  {
		if (strcmp (pwd->pw_name, newpwd->pw_name) == 0) {
			found = 1;
			putpwent (newpwd, tf);
		}
		else {
			putpwent (pwd, tf);
		}
	}

	/* if this was an addition, add it at the end */
	if (!found) {
		putpwent (newpwd, tf);
	}
	endpwent ();
	fclose (tf);
	DUMPLDETI(" passwd file rewritten", 0, 0, 0);

	if ((unlink (opasswd) && eaccess (opasswd, 0) == 0) ||
	    (unlink (opasswd_dir) && eaccess (opasswd_dir, 0) == 0) ||
	    (unlink (opasswd_pag) && eaccess (opasswd_pag, 0) == 0))
		return (FAILURE);
	else {
		stat (passwd, &sb);
		stat (passwd_dir, &dsb);
		stat (passwd_pag, &psb);
		/*
		 * try to rebuild the passwd database with temp
		 */
		if (makedb (temp))
			return (FAILURE);
		DUMPLDETI(" passwd database recreated", 0, 0, 0);
		/*
		 * try to move/backup the passwd file
		 */
		if (rename (passwd, opasswd))
			return (FAILURE);
		DUMPLDETI(" passwd -> opasswd", 0, 0, 0);
		if (rename (passwd_dir, opasswd_dir)) {
			rename (opasswd, passwd);
			return (FAILURE);
		}
		DUMPLDETI(" passwd.dir -> opasswd.dir", 0, 0, 0);
		if (rename (passwd_pag, opasswd_pag)) {
			rename (opasswd, passwd);
			rename (opasswd_dir, passwd_dir);
			return (FAILURE);
		}
		DUMPLDETI(" passwd.pag -> opasswd.pag", 0, 0, 0);
		/*
		 * now try to update with new files
		 */
		if (rename (temp_pag, passwd_pag)) {
			rename (opasswd, passwd);
			rename (opasswd_dir, passwd_dir);
			rename (opasswd_pag, passwd_pag);
			return (FAILURE);
		}
		DUMPLDETI(" temp -> passwd", 0, 0, 0);
		if (rename (temp_dir, passwd_dir)) {
			unlink (passwd_pag);
			rename (opasswd, passwd);
			rename (opasswd_dir, passwd_dir);
			rename (opasswd_pag, passwd_pag);
			return (FAILURE);
		}
		DUMPLDETI(" temp.dir -> passwd.dir", 0, 0, 0);
		if (rename (temp, passwd)) {
			unlink (passwd_dir);
			unlink (passwd_pag);
			rename (opasswd, passwd);
			rename (opasswd_dir, passwd_dir);
			rename (opasswd_pag, passwd_pag);
			return (FAILURE);
		}
		DUMPLDETI(" temp.pag -> passwd.pag", 0, 0, 0);

		chmod (passwd, sb.st_mode);
		chown (passwd, sb.st_uid, sb.st_gid);
		chmod (passwd, dsb.st_mode);
		chown (passwd, dsb.st_uid, dsb.st_gid);
		chmod (passwd, psb.st_mode);
		chown (passwd, psb.st_uid, psb.st_gid);
	}
	EXITLFUNC("updatepwfile");
	return (SUCCESS);
}


makedb (file)
char *file;
{
	int stat, procid, w;

	ENTERLFUNC ("makedb");
	if ((procid = vfork()) == 0) {
		execl("/usr/sbin/mkpasswd", "mkpasswd", file, 0);
		exit(1);
	}
	while ((w = wait(&stat)) != procid && w != -1) {
	}
	sleep (3);
	DUMPLDETI(" wait returned <%d> status=<%d> pid=<%d>", w, stat, procid);
	EXITLFUNC ("makedb");
	return ((stat || w == -1) ? -1 : stat);
}



/* make home directory
 * give proper user and group permissions.
 * set mode to 750.
 * returns 0 if mkdir command was successful, 1 if not.
 */

static int
mkhomedir (dir, userid, groupid)
char	*dir;
long	userid, groupid;
{
	int	ret;

#ifdef DEBUG
fprintf (_SWlogfp, "mkhomedir\n");
#endif
	ret = mkdir (dir, 0750);
	if (ret == 0) {
		chown (dir, (int) userid, (int) groupid);
		return (SUCCESS);
	}
	return (FAILURE);
}

#define UIDTHRESHHOLD 100

/* choose a user id
 * Just allocate one that's not allocated that's past a certain
 * threshhold.  The maximum user id is stored in <sys/param.h>.
 */

int
chooseuserid ()
{
	struct	passwd	*pwd;
	struct a {
		uid_t uid;
		struct a *next;
	} *ap, *first = NULL;
	uid_t ret_uid = (uid_t) -1;

	/* build a linked list of user ID's */
	setpwent();
	while (pwd = getpwent()) {
		ap = (struct a *) Malloc(sizeof(*ap));
		if (ap == (struct a *) 0)
			MemoryError();
		ap->uid = pwd->pw_uid;
		if (first == NULL) {
			/* empty list */
			first = ap;
			ap->next = NULL;
		} else {
			struct a *follow_ap = NULL;
			struct a *this_ap = first;

			/* sort into list */

			while (this_ap) {
				/* check for duplicate */
				if (ap->uid == this_ap->uid)
					goto error;
				if (ap->uid < this_ap->uid) {
					if (follow_ap == NULL)
						first = ap;
					else
						follow_ap->next = ap;
					ap->next = this_ap;
					break;
				}
				follow_ap = this_ap;
				this_ap = this_ap->next;
			}
			/* goes at the end of the list */
			if (this_ap == NULL) {
				follow_ap->next = ap;
				ap->next = NULL;
			}
		}
	}

	/* skip over the ones under the thresshold */

	for (ap = first; ap && ap->uid < UIDTHRESHHOLD; ap = ap->next)
		;

	ret_uid = UIDTHRESHHOLD;

	while (ap != NULL) {
		if (ap->uid == ret_uid) {
			ap = ap->next;
			ret_uid++;
		} else
			break;
	}

error:
	/* free the list */

	for (ap = first; ap != NULL; ) {
		first = first->next;
		free((char *) ap);
		ap = first;
	}

	return ret_uid;
}

/* ZZZZ */ 
int
CheckGroupAccess()
{
	char	*user;
	char	group_copy[GROUP_LEN + 1];
	char	*strrchr();

	/* check that the program can write in the parent directory of
	 * the password file */
	strcpy (group_copy, groupfn);
	user = strrchr (group_copy, '/');
	*user = '\0';
	if (eaccess (group_copy, 3) != 0) {  /* need write and search */
		audit_no_resource (groupfn, OT_PWD,
		  "No write permission in parent directory", ET_SYS_ADMIN);
		return (FAILURE);
	}
	return(SUCCESS);
}


/* check that it's a valid group name: no ':', 
 * no digit as first char (messes up chgrp), no '/' (messes up
 * home directory).
 */
int
CheckValidGroup(group)
char	*group;
{
	if (strlen(group) == 0)
		return (FAILURE);
	if (strchr (group, ':') ||
	    isdigit (group[0])  ||
	    strchr (group, '/'))
		return (FAILURE);
	return (SUCCESS);
}

int
GroupNameExists(group)
char	*group;
{
	if (getgrnam (group) != (struct group *) 0)
		return (FAILURE);
	else
		return (SUCCESS);
}


int
CheckValidGid(groupid)
int	groupid;
{
	/* Picked an arbitrary limit of MAXGID for group limit */
	if (groupid < 0 || groupid > MAXGID)
		return (FAILURE);
	else
		return (SUCCESS);
}
int
GroupIdExists(groupid)
int	groupid;
{
	if (getgrgid (groupid) != (struct group *) 0)
		return (FAILURE);
	else
		return (SUCCESS);
}

int
XCreateGroup (newgrp)
struct	group	*newgrp;
{
	char	group_name[NUSERNAME + 1];
	long	group_id;
	struct	group	*grp, *getgrent();
	FILE	*ngrpfp, *ogrpfp;
	int	c;			/* for copying */
	int i;
	privvec_t s;
	int	ret;

	endgrent();

	forceprivs(privvec(SEC_CHOWN, SEC_ALLOWDACACCESS,
#if SEC_MAC
			   SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			   SEC_ILNOFLOAT,
#endif
			   -1), s);
	ret = create_file_securely(groupnewfn, AUTH_VERBOSE);
	if (ret != CFS_GOOD_RETURN) {
		seteffprivs(s, NULL);
		return FAILURE;
	}
	ngrpfp = fopen (groupnewfn, "w");
	ogrpfp = fopen (groupfn, "r");
	while ((c = getc (ogrpfp)) != EOF)
		putc (c, ngrpfp);
	fprintf(ngrpfp, "%s::%ld:", newgrp->gr_name, newgrp->gr_gid);
	if (newgrp->gr_mem != (char **) 0)
		for (i = 0; newgrp->gr_mem[i] != NULL; i++)
			if (i != 0)
				putc(',', ngrpfp);
			fputs(newgrp->gr_mem[i], ngrpfp);
	putc('\n', ngrpfp);
			
	fclose (ngrpfp);
	fclose (ogrpfp);
	unlink (groupfn);
	rename(groupnewfn, groupfn);
	seteffprivs(s, NULL);
	audit_subsystem ("Add group to /etc/group file",
	  newgrp->gr_name, ET_SYS_ADMIN);
	return (SUCCESS);
}

#define GIDTHRESHHOLD 100

/* choose a group id
 * Just allocate one that's not allocated that's past a certain
 * threshhold.  
 */

int
choosegroupid ()
{
	struct	group	*grp;
	struct a {
		gid_t gid;
		struct a *next;
	} *ap, *first = NULL;
	gid_t ret_gid = (gid_t) -1;

	/* build a linked list of user ID's */
	setgrent();
	while (grp = getgrent()) {
		ap = (struct a *) Malloc(sizeof(*ap));
		if (ap == (struct a *) 0)
			MemoryError();
		ap->gid = grp->gr_gid;
		if (first == NULL) {
			/* empty list */
			first = ap;
			ap->next = NULL;
		} else {
			struct a *follow_ap = NULL;
			struct a *this_ap = first;

			/* sort into list */

			while (this_ap) {
				/* check for duplicate */
				if (ap->gid == this_ap->gid)
					goto error;
				if (ap->gid < this_ap->gid) {
					if (follow_ap == NULL)
						first = ap;
					else
						follow_ap->next = ap;
					ap->next = this_ap;
					break;
				}
				follow_ap = this_ap;
				this_ap = this_ap->next;
			}
			/* goes at the end of the list */
			if (this_ap == NULL) {
				follow_ap->next = ap;
				ap->next = NULL;
			}
		}
	}

	/* skip over the ones under the thresshold */

	for (ap = first; ap && ap->gid < GIDTHRESHHOLD; ap = ap->next)
		;

	ret_gid = GIDTHRESHHOLD;

	while (ap != NULL) {
		if (ap->gid == ret_gid) {
			ap = ap->next;
			ret_gid++;
		} else
			break;
	}

error:
	/* free the list */

	for (ap = first; ap != NULL; ) {
		first = first->next;
		free((char *) ap);
		ap = first;
	}

	return ret_gid;
}


/* change the group memberships for the user.
 * returns 0 on success, 1 on failure
 * Corresponds to changememgroup from auth_user.c
 */

int 
XModifyUserGroup(username, groupid, groupmem)
char	*username;
int	groupid;
char	**groupmem;
{
	static 	char
		**msg_err_cant_update_grp_list,
		*msg_err_cant_update_grp_list_text;
	struct	group	*grp;
	register char	**gcp;	/* for walking group list for user */
	register char	**ucp;	/* for walking user list in group file */
	int	UserOnGroupList;	/* user is on group list */
	int	GroupOnUserList;	/* group is on user list */
	FILE	*outfp;
	struct	stat	sb;
	int	first;	/* is first in group list printing? */

#ifdef DEBUG
	fprintf (_SWlogfp, "Modify user group\n");
	fprintf (_SWlogfp, "Name %s\n", username);
	fprintf (_SWlogfp, "Gid  %d\n", groupid);
#endif

	/* output file for groups */
	if ((outfp = fopen (groupnewfn, "w")) == (FILE *) 0) {
		audit_no_resource (groupnewfn, OT_REGULAR,
		  "unsuccessful create of new group file", ET_SYS_ADMIN);
		if (! msg_err_cant_update_grp_list)
		    LoadMessage("msg_accounts_make_user_cant_update_grp_list",
				&msg_err_cant_update_grp_list, 
				&msg_err_cant_update_grp_list_text);
		ErrorMessageOpen(-1, msg_err_cant_update_grp_list, 0, NULL);
		return (FAILURE);
	}

	stat (groupfn, &sb);
	chmod (groupnewfn, sb.st_mode);
	chown (groupnewfn, sb.st_uid, sb.st_gid);

	/* for each of groups in group file,
	 * if user is a member in the file
	 * and no longer has that group on the list, remove him from the group
	 * if the user is not a member in the file and is on the list,
	 * add him to the group.
	 */
	setgrent();
	while (grp = getgrent())  {
		GroupOnUserList = 0; /* group not on user list */
		for (gcp = groupmem; gcp && (*gcp) && (*gcp)[0]; gcp++) {
			if (strcmp (grp->gr_name, *gcp) == 0) {
				GroupOnUserList = 1; /* group on user's list */
				break;
			}
		}
		if (grp->gr_gid == groupid)
			GroupOnUserList = 1;
		UserOnGroupList = 0; /* user not on group list */
		for (ucp = grp->gr_mem; *ucp; ucp++)
			if (strcmp (username, *ucp) == 0) { /*yes */
				UserOnGroupList = 1;
				break;
			}
		if (grp->gr_passwd == NULL)
			fprintf (outfp, "%s::%d:", grp->gr_name, grp->gr_gid);
		else	fprintf (outfp, "%s:%s:%d:", grp->gr_name,
					     grp->gr_passwd,
					     grp->gr_gid);
		/* if user on group list and group not on user list */
		if (UserOnGroupList == 1 && GroupOnUserList == 0) {
			/* remove user from the group */
			first = 1;
			for (ucp = grp->gr_mem; *ucp; ucp++)
				if (strcmp (*ucp, username) == 0)
					continue;
				else if (first) {
					fprintf (outfp, "%s", *ucp);
					first = 0;
				}
				else	fprintf (outfp, ",%s", *ucp);
			fprintf (outfp, "\n");
		}

		/* if user not on group list and group on user list */
		else if (UserOnGroupList == 0 && GroupOnUserList == 1)  {
			/* add user to the group */
			first = 1;
			for (ucp = grp->gr_mem; *ucp; ucp++)
				if (first) {
					fprintf (outfp, "%s", *ucp);
					first = 0;
				}
				else	fprintf (outfp, ",%s", *ucp);
			if (first)
				fprintf (outfp, "%s\n", username);
			else	fprintf (outfp, ",%s\n", username);
		}
		else  {
			/* write out group as is */
			first = 1;
			for (ucp = grp->gr_mem; *ucp; ucp++)
				if (first) {
					fprintf (outfp, "%s", *ucp);
					first = 0;
				}
				else	fprintf (outfp, ",%s", *ucp);
			fprintf (outfp, "\n");
		}
	}
	endgrent();
	fclose (outfp);
	if (unlink (groupfn) < 0)  {
		audit_security_failure (OT_GRP, "unlink",
		  "unsuccessful group file update", ET_SYS_ADMIN);
		unlink (groupnewfn);
		if (! msg_err_cant_update_grp_list)
		    LoadMessage("msg_accounts_make_user_cant_update_grp_list",
				&msg_err_cant_update_grp_list, 
				&msg_err_cant_update_grp_list_text);
		ErrorMessageOpen(-1, msg_err_cant_update_grp_list, 0, NULL);
		return (FAILURE);
	}
	link (groupnewfn, groupfn);
	unlink (groupnewfn);
	return (SUCCESS);
}

#ifdef X11
/* Check the device against others */
	/* b11, b21 refer to the single level / multi level IL/SL toggle
	 * widgets. On a non-SEC_MAC system these do not exist and therefore
	 * come in as NULL widgets */
int
XValidateDeviceEntry (name, device_list_widget, device_list_widget_count, 
		b11, b21, dv, adding, device_type)
	char	*name;
	Widget	*device_list_widget;
	int	device_list_widget_count;
	Widget	b11;
	Widget	b21;
	struct  dev_if	*dv;
	Boolean	adding;
	int	device_type;
{
	char 	*buf;
	char	**device_list_names;
	struct dev_asg *dev;
	int	device_list_name_count;
	int	len;
	int	ret;
	int 	i;

#ifdef DEBUG
	fprintf (_SWlogfp, "XValidateDeviceEntry\n");
	fprintf (_SWlogfp, "name %s\n", name);
	fprintf (_SWlogfp, "device_list 0 widget =  %d \n", device_list_widget[0]);
	fprintf (_SWlogfp, "device_list_widget_count %d\n", device_list_widget_count);
	fprintf (_SWlogfp, "b11, b21 %d %d\n", b11, b21);
	fprintf (_SWlogfp, "adding %d\n", adding);
	fprintf (_SWlogfp, "device_type %d\n", device_type);
#endif
	/* If adding a device then check the name is new */
	if (adding) {
		ret = GetDeviceInfo(name, dv);
		if (ret == SUCCESS) {
			/* The name already exists ! */
			if (! msg_err_dev_already_exists)
			    	LoadMessage("msg_accounts_dev_already_exists",
					&msg_err_dev_already_exists, 
					&msg_err_dev_already_exists_text);
			ErrorMessageOpen(-1, msg_err_dev_already_exists, 
					0, NULL);
			return (FAILURE);
		}
	}

	ret = SUCCESS;
	device_list_name_count = 0;
	device_list_names = (char **) Malloc ( sizeof(char *) * 
			device_list_widget_count);
	if (! device_list_names)
		MemoryError();
	for (i=0; i<device_list_widget_count; i++) {
		buf = XmTextGetString(device_list_widget[i]);
		fprintf (_SWlogfp, "Device list number and name %d %s\n", i, buf);
		if (buf[0] != '\0') {
			device_list_names[device_list_name_count] =
				(char *) Malloc(strlen(buf) + 1);
			if (! device_list_names[device_list_name_count])
				MemoryError();

			strcpy(device_list_names[device_list_name_count], buf);
			device_list_name_count ++;
		}
		XtFree(buf);
	}
					
	/* If none listed then get a device name */
	if (device_list_name_count == 0) {
		/* Not for hosts */
		if (device_type == AUTH_DEV_REMOTE) {
			/* Don't know what to do yet */
			fprintf (_SWlogfp, "Hostname \n");
		}
		else {
			len = strlen ("/dev/");
			device_list_names[0] = (char *) 
				Malloc(strlen(name) + len + 1);
			if (! device_list_names[0])
				MemoryError();
			sprintf (device_list_names[0], "/dev/%s", name);
			device_list_name_count = 1;
		}
	}

	/* Check that all the device lists are valid devices */
	if (device_type != AUTH_DEV_REMOTE) {
		for (i=0;i<device_list_name_count;i++) {
			if (! XIsDeviceCharSpecial(device_list_names[i])) {
				fprintf (_SWlogfp, "sdf\n");
				ret = FAILURE;
				goto clean_up;
			}
		}
	}

	fprintf (_SWlogfp, "HIHIH\n");

	/* Check that the device names are unique */
	if (XIsDeviceAlreadyListed (name, device_list_names, 
		device_list_name_count)) {
		fprintf (_SWlogfp, "Error\n");
		ret = FAILURE;
		goto clean_up;
	}

	fprintf (_SWlogfp, "Hithere\n");
	/* If modifying an existing entry then get that entry so we can 
	 * modify it */
	if (! adding) {
		ret = XGetDeviceInfo (name, dv);
		fprintf (_SWlogfp, "ret is %d\n", ret);
		if (ret)
			goto clean_up;
	}

	dev = &dv->dev;

	/* If adding then we need to null the structure */
	if (adding) {
		strncpy (dev, "", sizeof(*dev));

		/* Everything looks good - save the name and other parameters */
		dev->uflg.fg_name = 1;
		dev->ufld.fd_name = (char *) Malloc (strlen(name) + 1);
		if (! dev->ufld.fd_name)
			MemoryError();

		strcpy (dev->ufld.fd_name, name);
#if SEC_MAC
		dev->uflg.fg_max_sl = 0;
		dev->uflg.fg_min_sl = 0;
		dev->uflg.fg_cur_sl = 0;
#if SEC_ILB
		dev->uflg.fg_cur_il = 0;
#endif
#endif
	}

	/* Should add some code here to free the old device list but I
	 * think the data is stored funny */

	/* Copy the device list into the structure */
	dev->uflg.fg_devs = 1;
	dev->ufld.fd_devs = (char **) Malloc 
			(sizeof(char *) * (device_list_name_count + 1) );
	if (! dev->ufld.fd_devs)
		MemoryError();
	for (i=0; i<device_list_name_count; i++) {
		dev->ufld.fd_devs[i] = (char *) Malloc 
				(strlen(device_list_names[i]) + 1);
		if (! dev->ufld.fd_devs[i])
			MemoryError();
		strcpy(dev->ufld.fd_devs[i], device_list_names[i]);
	}
	dev->ufld.fd_devs[device_list_name_count] = '\0';

	dev->uflg.fg_type = 1;
	/* Clear type field and then set to the type */
	for (i = 0; i<= AUTH_MAX_DEV_TYPE; i++) 
		RMBIT (dev->ufld.fd_type, i);
	ADDBIT (dev->ufld.fd_type, device_type);

#if SEC_MAC
	/* Depending on multi level bits we set these */
	dev->uflg.fg_assign = 1;
	for (i = 0; i<= AUTH_MAX_DEV_ASSIGN; i++) 
		RMBIT (dev->ufld.fd_assign, i);

#if ! SEC_SHW
	if (XmToggleButtonGadgetGetState (b11))
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_SINGLE);
	else
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_MULTI);
#endif /* ! SEC_SHW */
#if SEC_ILB
	if (XmToggleButtonGadgetGetState (b21))
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_ILSINGLE);
	else
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_ILMULTI);
#endif /* SEC_ILB */
#endif /* SEC_MAC */

clean_up:
	/* Save space */
	if (device_list_names) {
		for (i=0; i<device_list_name_count; i++)
			Free (device_list_names[i]);
		Free (device_list_names);
	}

	return (ret);
}

int
XValidateTerminalEntry (name, text_default_widget, text_widget,
		default_lock, lock, dv, adding)
	char *name;
	Widget	*text_default_widget;
	Widget	*text_widget;
	Boolean 	default_lock;
	Boolean		lock;
	struct  dev_if	*dv;
	Boolean	adding;
{
	char 	*buf;
	struct pr_term *tm;
	int	i;
	long 	t;
	int	ret;
	
	/* If modifying an existing entry then get that entry 
	 * so we can modify it */
	if (! adding) {
		ret = GetTerminalInfo (name, dv);
		if (ret)
			return (FAILURE);
	}

	tm = &dv->tm;

	/* If adding then we need to null the structure */
	if (adding) {
		strncpy (tm, "", sizeof(*tm));
		tm->uflg.fg_devname = 1;
		strcpy (tm->ufld.fd_devname, name);
		tm->uflg.fg_uid = 0;
		tm->uflg.fg_slogin = 0;
		tm->uflg.fg_uuid = 0;
		tm->uflg.fg_ulogin = 0;
		tm->uflg.fg_loutuid = 0;
		tm->uflg.fg_louttime = 0;
		tm->uflg.fg_nlogins = 0;
	}

	/* Check the text input fields */
	if (XmToggleButtonGadgetGetState(text_default_widget[0])) {
		tm->uflg.fg_max_tries = 0;
	}
	else {
		tm->uflg.fg_max_tries = 1;
		buf = XmTextGetString(text_widget[0]);
		i = atoi (buf);
		XtFree(buf);
		if (i >= 0 && i <= 999)
			tm->ufld.fd_max_tries = (ushort) (i);
		else
			return (FAILURE);
	}

	if (XmToggleButtonGadgetGetState(text_default_widget[1])) {
		tm->uflg.fg_login_timeout = 0;
	}
	else {
		tm->uflg.fg_login_timeout = 1;
		buf = XmTextGetString(text_widget[1]);
		i = atoi (buf);
		XtFree(buf);
		if (i >= 0 && i <= 999)
			tm->ufld.fd_login_timeout = (ushort) i;
		else
			return (FAILURE);
	}

	if (XmToggleButtonGadgetGetState(text_default_widget[2])) {
		tm->uflg.fg_logdelay = 0;
	}
	else {
		tm->uflg.fg_logdelay = 1;
		buf = XmTextGetString(text_widget[2]);
		t = atol (buf);
		XtFree(buf);
		if (t >= 0 && t <= 999)
			tm->ufld.fd_logdelay = t;
		else
			return (FAILURE);
	}

	/* Lock toggle */
	if (default_lock) {
		tm->uflg.fg_lock = 0;
		tm->ufld.fd_lock = 1;
	}
	else {
		tm->uflg.fg_lock = 1;
		if (lock)
			 tm->ufld.fd_lock = 1;
		else     tm->ufld.fd_lock = 0;
	}

	return (SUCCESS);
}
#endif /* X11 */

#endif /* SEC_BASE */

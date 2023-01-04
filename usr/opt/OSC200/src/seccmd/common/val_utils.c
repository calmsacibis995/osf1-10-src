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
static char	*sccsid = "@(#)$RCSfile: val_utils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:44 $";
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



/*  User account validation utilities
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
#include	"valid.h"
#include	"Messages.h"


/* check that it's a valid group name: no ':', no upper case as 1st char,
 * no digit as first char (messes up chmod and chown), no '/' (messes up
 * home directory).
 * returns 0 on success (user name ok), 1 if not ok.
 */

int
InvalidGroup (user)
char	*user;
{
	if (strchr (user, ':') ||
	    isupper (user[0])  ||
	    isdigit (user[0])  ||
	    strchr (user, '/'))
		return (1);
	return (0);
}


void
baduser (user)
char	*user;
{
	pop_msg ("User names must not have imbedded colons (:) or",
	  "spaces.  All characters must be printable.");
	return;
}

/* bad group name or id */

static void
badgroup (gname, gid)
char	*gname;
long	gid;
{
	char	buf[80];

	(void) sprintf (buf, "Group name \'%s\' or group id \'%ld\' already",
	  gname, gid);
	pop_msg (buf,
	  "in the group database.  Choose a new one.");
	return;
}

/* trying to add a user who already exists */

void
userexists (user)
char	*user;
{
	char	buf[80];

	(void) sprintf (buf, "User \'%s\' exists in the password file.", user);
	pop_msg (buf, "Please use the update option to change that entry");
	return;
}

/* trying to access a user who doesn't exist */

void
nosuchuser (user)
char	*user;
{
	char	buf[80];

	(void) sprintf (buf, "User \'%s\' does not exist in the password file.", user);
	pop_msg (buf, "Please use the add option to add that entry");
	return;
}

/* trying to access a uid which doesn't exist */

void
nosuchuid (uid)
uid_t uid;
{
	char	buf[80];

	(void) sprintf (buf, "UID \'%d\' does not exist in the password file.", uid);
	pop_msg (buf, "Please use the add option to add that entry");
	return;
}

/* trying to access a group that doesn't exist */

void
nosuchgroup (group)
char	*group;
{
	char	buf[80];

	(void) sprintf (buf, "Group \'%s\' does not exist in the group file.",
		group);
	pop_msg (buf, "Please use the add option to add that entry");
	return;
}

/* trying to access a gid which doesn't exist */

void
nosuchgid (gid)
gid_t gid;
{
	char	buf[80];

	(void) sprintf (buf, "GID \'%d\' does not exist in the group file.",
		gid);
	pop_msg (buf, "Please use the add option to add that entry");
	return;
}

/* write a message only screen telling there is no such shell */

void
noshell (shell)
char	*shell;
{
	char	buf[80];

	(void) sprintf (buf, 
	"There is no executable file \'%s\'.", shell);
	pop_msg (buf, "Please choose an executable program.");
	return;
}


FILE	*
lockpwfile ()
{
	int	i = 0;
	FILE	*tf;
	while (eaccess (temp, 0) >= 0 && i < 30) {
		sleep (1);
		i++;
	}
	if (i == 30 || (tf = fopen (temp, "w")) == (FILE *) 0) {
		pop_msg ("Could not open password file for writing.",
		  "Check existence of password lock file.");
		return ((FILE *) 0);
	}
	return (tf);
}



void InvGroupMsg(group)
char *group;
{
	if (! msg_invalid_group)
		LoadMessage("msg_accounts_make_group_invalid_group",
			&msg_invalid_group,
			&msg_invalid_group_text);
	ErrorMessageOpen(-1, msg_invalid_group, 0, NULL);
}


void GIDExistsMsg(gid)
gid_t gid;
{
	if (! msg_gid_exists)
		LoadMessage("msg_accounts_make_group_uid_exists",
			&msg_gid_exists,
			&msg_gid_exists_text);
	ErrorMessageOpen(-1, msg_gid_exists, 0, NULL);
}


void GroupExistsMsg(group)
char *group;
{
	if (! msg_group_exists)
		LoadMessage("msg_accounts_make_group_exists",
			&msg_group_exists,
			&msg_group_exists_text);
	ErrorMessageOpen(-1, msg_group_exists, 0, NULL);
}


void CantUpdateGroupMsg()
{
	if (! msg_cant_update_group)
		LoadMessage("msg_accounts_cant_update_group",
			&msg_cant_update_group,
			&msg_cant_update_group_text);
		ErrorMessageOpen (-1, msg_cant_update_group, 0, NULL);
}


void InvUserMsg(user)
char *user;
{
	if (! msg_error_invalid_name)
		LoadMessage("msg_accounts_make_user_invalid_name",
			&msg_error_invalid_name,
			&msg_error_invalid_name_text);
	ErrorMessageOpen(-1, msg_error_invalid_name, 0, NULL);
}


void UIDExistsMsg(uid)
uid_t uid;
{
	if (! msg_error_uid_exists)
		LoadMessage("msg_accounts_make_user_uid_exists",
			&msg_error_uid_exists,
			&msg_error_uid_exists_text);
	ErrorMessageOpen(-1, msg_error_uid_exists, 0, NULL);
}


void UserExistsMsg(user)
char *user;
{
	if (! msg_error_name_exists)
		LoadMessage("msg_accounts_make_user_exists",
			&msg_error_name_exists,
			&msg_error_name_exists_text);
	ErrorMessageOpen(-1, msg_error_name_exists, 0, NULL);
}


void CantUpdateUserMsg()
{
	if (! msg_cant_update_user)
		LoadMessage("msg_accounts_cant_update_user",
			&msg_cant_update_user,
			&msg_cant_update_user_text);
		ErrorMessageOpen (-1, msg_cant_update_user, 0, NULL);
}

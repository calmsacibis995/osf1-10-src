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
static char	*sccsid = "@(#)$RCSfile: userutil.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:39 $";
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
/* Copyright (c) 1988 SecureWare, Inc.
 *   All rights reserved
 */



/*
 * userutil.c - utility routines related to user accounts
 */

#include <grp.h>
#include <sys/security.h>
#include <sys/audit.h>



/* make home directory
 * give proper user and group permissions.
 * set mode to 750.
 * returns 0 if mkdir command was successful, 1 if not.
 */

int
mkhomedir (dir, userid, groupid)
char	*dir;
long	userid, groupid;
{
	char	buf[80];

	sprintf (buf, "/bin/mkdir %s", dir);
	if (system (buf))
		return (1);
	chmod (dir, 0750);
	chown (dir, (int) userid, (int) groupid);
	return (0);
}

/* write a message only screen telling that there is no such user */

void
nouser (user)
char	*user;
{
	char	buf[80];

	sprintf (buf, "There is no user \'%s\' in the password file.", user);
	pop_msg (buf,
	  "Please choose a valid user name that has a valid entry.");
	audit_auth_entry(user, OT_PWD, "there is no entry for this user",
		ET_SYS_ADMIN);	
	return;
}

void
noprpwentry (user)
char	*user;
{
	char	buf[80];

	sprintf (buf, "There is no protected password entry for user \'%s\'.",
	  user);
	pop_msg (buf, "Please create one with the Create User Entry screen.");
	audit_auth_entry(user, OT_PRPWD, "there is no entry for this user",
		ET_SYS_ADMIN);
	return;
}

void
makehomedir (homedir)
char	*homedir;
{
	char	buf[80];
	sprintf (buf, "Cannot create home directory \'%s\'.", homedir);
	pop_msg (buf, "Please correct problem and make directory manually.");
	return;
}

/* password file is locked. */

void
pwdinuse ()
{
	pop_msg ("The password file is in use.",
	  "The operation was aborted.");
	return;
}

void
badhomedir (homedir)
char	*homedir;
{
	char	buf[80];
	sprintf (buf, "Home directory requested \'%s\' cannot be",
	homedir);
	pop_msg (buf,
		"accessed. Please check permissions on that directory.");
	return;
}

/* moving to home directory that already exists */

void
homedirexists (homedir)
char	*homedir;
{
	char	buf[80];
	sprintf (buf, "Home directory requested \'%s\'",
	homedir);
	pop_msg (buf, "exists. Please choose one that doesn\'t.");
	return;
}

print_output (s)
char *s;
{
}
print_help_scrns (s1, s2)
char *s1, *s2;
{
}

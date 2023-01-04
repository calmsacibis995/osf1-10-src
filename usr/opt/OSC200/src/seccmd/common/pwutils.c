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
static char	*sccsid = "@(#)$RCSfile: pwutils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:31 $";
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
/*
 * Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
 */



/*
 * pwutils.c - passwd && group file manipulation utils
 *
 * TEMPS FOR WHILE OSF PROBLEMS OCCUR:
 * putpwent ()        - replacement for broken osf/AIX putpwent()
 *
 * GetPWUserByName()  - get a passwd entry based on user name
 * GetPWUserByUID()   - get a passwd entry based on UID
 *
 * GetGrName()        - return a group name corresponding to a GID
 * GetGRGroupByName() - get a group entry based on group name
 * GetGRGroupByGID()  - get a group entry based on GID
 */

#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include "logging.h"

char *strdup();


#ifndef FIXED_PASSWD
/*
 * putpwent() put a passwd entry at the end of the passwd file
 */


int
putpwent (p, f)
struct passwd *p;
FILE *f;
{
	if (p && f && fprintf (f, "%s:%s:%d:%d:%s:%s:%s\n",
	p->pw_name, p->pw_passwd, p->pw_uid, p->pw_gid,
	p->pw_gecos, p->pw_dir, p->pw_shell) >= 0)
		return 0;
	else
		return 1;
}
#endif /* FIXED_PASSWD */


/*
 * GetPWUserByName() - return 1 (and the user entry) if user exists, else 0
 */

int
GetPWUserByName(name, upw)
	char *name;
	struct passwd *upw;
{
	int ret;
	struct passwd *user;

	ENTERLFUNC ("GetPWUserByName");
	user = getpwnam(name);
		if (user != (struct passwd *) NULL &&
		!strcmp (name, user->pw_name)) {
			DUMPLDETI (" Found <%s>", name, 0, 0);
			bcopy (user, upw, sizeof (struct passwd));
			ret = 1;
		} else {
			DUMPLDETI (" Could not find <%s>", name, 0, 0);
			bzero (upw, sizeof (struct passwd));
			ret = 0;
		}
	EXITLFUNC ("GetPWUserByName");
	return (ret);
}


/*
 * GetPWUserByUID() - return 1 (and the user entry) if UID exists, else 0
 */

int
GetPWUserByUID(uid, upw)
	long uid;
	struct passwd *upw;
{
	int ret;
	struct passwd *user;

	ENTERLFUNC ("GetPWUserByUID");
	user = getpwuid(uid);
		if (user != (struct passwd *) NULL &&
		uid ==  user->pw_uid) {
			DUMPLDETI (" Found <%d>", uid, 0, 0);
			bcopy (user, upw, sizeof (struct passwd));
			ret = 1;
		} else {
			DUMPLDETI (" Could not find <%d>", uid, 0, 0);
			bzero (upw, sizeof (struct passwd));
			ret = 0;
		}
	EXITLFUNC ("GetPWUserByUID");
	return (ret);
}



/*
 * GetGrName() - return a group name corresponding to a GID
 */

char *
GetGrName (gid)
int gid;
{
	struct group *gp;
	char *s = NULL;

	setgrent ();
	if ((gp = getgrgid (gid)) != NULL)
		s = strdup (gp->gr_name);
	endgrent ();
	return s;
}



addgroup (q)
char *q;
{
	return 0;
}


/*
 * GetGRGroupByName() - return 1 (and the user entry) if user exists, else 0
 */

int
GetGRGroupByName(name, grp)
	char *name;
	struct group *grp;
{
	struct group *this1;

	this1 = getgrnam(name);
		if (this1 != (struct group *) NULL &&
		!strcmp (name, this1->gr_name)) {
			bcopy (this1, grp, sizeof (struct group));
			return 1;
		} else {
			bzero (grp, sizeof (struct group));
			return 0;
		}
}


/*
 * GetGRGroupByGID() - return 1 (and the this1 entry) if GID exists, else 0
 */

int
GetGRGroupByGID(gid, grp)
	long gid;
	struct group *grp;
{
	struct group *this1;

	this1 = getgrgid(gid);
		if (this1 != (struct group *) NULL &&
		gid ==  this1->gr_gid) {
			bcopy (this1, grp, sizeof (struct group));
			return 1;
		} else {
			bzero (grp, sizeof (struct group));
			return 0;
		}
}



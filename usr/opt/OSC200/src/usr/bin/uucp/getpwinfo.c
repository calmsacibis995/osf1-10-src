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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getpwinfo.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:05:44 $";
#endif
/* 
 * COMPONENT_NAME: UUCP getpwinfo.c
 * 
 * FUNCTIONS: gninfo, guinfo 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/*
getpwinfo.c	1.5  com/cmd/uucp,3.1,9013 11/30/89 14:01:22";
*/
/*	uucp:getpwinfo.c	1.2
*/
#include "uucp.h"
/* VERSION( getpwinfo.c	5.2 -  -  ); */

#include <pwd.h>
/* extern struct passwd *getpwuid(), *getpwnam(); */
extern char	*getlogin();

/*
 * get passwd file info for logname or uid
 *	uid	-> uid #	
 *	name	-> address of buffer to return ascii user name
 *		This will be set to pw->pw_name.
 *
 * return:
 *	0	-> success
 *	FAIL	-> failure (logname and uid not found)
 */
guinfo(uid, name)
int uid;
char *name;
{
	register struct passwd *pwd;
	char	*login_name;

	/* look for this user as logged in utmp */
	if ((login_name = getlogin()) != NULL) {
		pwd = getpwnam(login_name);
		if (pwd != NULL && pwd->pw_uid == uid)
			goto uid_found;
	}

	/* no dice on utmp -- get first from passwd file */
	if ((pwd = getpwuid(uid)) == NULL) {
	    if ((pwd = getpwuid(UUCPUID)) == NULL)
		/* can not find uid in passwd file */
		return(FAIL);
	}

uid_found:
	(void) strcpy(name, pwd->pw_name);
	return(0);
}

/*
 * get passwd file info for name
 *	name	-> ascii user name
 *	uid	-> address of integer to return uid # in
 *	path	-> address of buffer to return working directory in
 * returns:
 *	0	-> success
 *	FAIL	-> failure
 */
gninfo(name, uid, path)
char *path, *name;
int *uid;
{
	register struct passwd *pwd;

	if ((pwd = getpwnam(name)) == NULL) {
		/* can not find name in passwd file */
		*path = '\0';
		return(FAIL);
	}

	(void) strcpy(path, pwd->pw_dir);
	*uid = pwd->pw_uid;
	return(0);
}



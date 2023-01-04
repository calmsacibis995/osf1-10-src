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
static char	*sccsid = "@(#)$RCSfile: namtouid.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:58:55 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 *	namtouid converts login names to uids
 *	maintains ulist for speed only
 */
#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <pwd.h>
extern char *strncpy();
static	usize;
static	struct ulist {
	char	uname[NSZ];
	uid_t	uuid;
} ul[A_USIZE];
char	ntmp[NSZ+1];

uid_t
namtouid(name)
char	name[NSZ];
{
	register struct ulist *up;
	register uid_t tuid;
	extern struct passwd *getpwnam();
	register struct passwd *pp;

	for (up = ul; up < &ul[usize]; up++)
		if (EQN(up->uname, name))
			return(up->uuid);
	(void)strncpy(ntmp, name, NSZ);
	(void)setpwent();
	if ((pp = getpwnam(ntmp)) == NULL)
		tuid = -1;
	else {
		tuid = pp->pw_uid;
		if (usize < A_USIZE) {
			(void)CPYN(up->uname, name);
			up->uuid = tuid;
			usize++;
		}
	}
	return(tuid);
}

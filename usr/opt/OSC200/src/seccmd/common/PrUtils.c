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
static char	*sccsid = "@(#)$RCSfile: PrUtils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:00:13 $";
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
#ifdef SEC_BASE
/*
 *	Copyright 1990, SecureWare, Inc. Atlanta, GA. All righse reserved
 */

/*
	filename:
		PrUtils.c
	
	copyright:
		Copyright (c) 1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	functions:

	GetAllLockedUsers() - Returns list of locked users
	GetAllLockedIssoUsers() - Returns list of locked ISSOs
	GetAllLockedNonIssoUsers() - Returns list of locked non ISSOs

	GetAllUnlockedUsers() - Returns list of unlocked users
	GetAllUnlockedIssoUsers() - Returns list of unlocked ISSOs
	GetAllUnlockedNonIssoUsers() - Returns list of unlocked non ISSOs

	GetAllNonretiredUsers() - Returns list of non-retired users
	GetAllNonretiredIssoUsers() - Returns list of non-retired ISSOs
	GetAllNonretiredNonIssoUsers() - Returns list of non-retired non ISSOs
		
	notes:
		This file contains general protected password utilities
*/


#include <sys/types.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#include <stdio.h>
#include <grp.h>
#include <pwd.h>

#include "UIMain.h"
#include "kitch_sink.h"
#include "logging.h"
#include "Utils.h"

/* External routines */ 

extern int strcmp();

struct list {
	char name[UNAMELEN + 1];
	struct list *next;
} *first = NULL, *last, *lp;



static
void
AddAUser(user)
	struct pr_passwd *user;
{
	struct list *lp;

	lp = (struct list *) Calloc(sizeof(struct list), 1);
	if (lp == (struct list *) 0)
		MemoryError();
	strncpy(lp->name, user->ufld.fd_name, UNAMELEN);
	if (first == NULL)
		first = last = lp;
	else {
		last->next = lp;
		last = lp;
	}
}


static
void
CopyAllUsers(numb_users, pitems)
	int	numb_users;
	char    ***pitems;
{
	struct list *olp;

	*pitems = alloc_cw_table(numb_users, UNAMELEN + 1);

	for (lp = first, numb_users = 0; lp; numb_users++ ) {
		strcpy((*pitems)[numb_users], lp->name);
		olp = lp;
		lp = lp->next;
		Free(olp);
	}
	first = NULL;
}



void 
GetAllLockedUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	/* Rewind the file */
	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (IsPrRetired(user) || !IsPrLocked(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllLockedIssoUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (!IsPrIsso(user) || IsPrRetired(user) || !IsPrLocked(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllLockedNonIssoUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (IsPrIsso(user) || IsPrRetired(user) || !IsPrLocked(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllUnlockedUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	/* Rewind the file */
	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (IsPrRetired(user) || IsPrLocked(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllUnlockedIssoUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (!IsPrIsso(user) || IsPrRetired(user) || IsPrLocked(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllUnlockedNonIssoUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (IsPrIsso(user) || IsPrRetired(user) || IsPrLocked(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);

	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllNonretiredUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	/* Rewind the file */
	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (IsPrRetired(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllNonretiredIssoUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (!IsPrIsso(user) || IsPrRetired(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}

void 
GetAllNonretiredNonIssoUsers(pnitems, pitems)
	int     *pnitems;
	char    ***pitems;
{
	int     count;
	struct pr_passwd *user;

	setprpwent();

	for (count = 0; user = getprpwent(); ) {
		if (IsPrIsso(user) || IsPrRetired(user))
			continue;
		AddAUser(user);
		count++;
	}
	*pnitems = count;
	CopyAllUsers(count, pitems);
	
	/* Alpha sort entries */
	sort_cw_table(*pitems, UNAMELEN + 1, *pnitems);
}
#endif /* SEC_BASE */

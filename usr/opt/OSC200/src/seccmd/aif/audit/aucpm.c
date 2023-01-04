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
static char	*sccsid = "@(#)$RCSfile: aucpm.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:59 $";
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
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



/*
 * Routines to implement the selection of users and groups for audit collection.
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "gl_defs.h"
#include "IfAudit.h"
#include "userif.h"
#include "UIMain.h"
#include "valid.h"
#include "logging.h"

/* static routine definitions */

static int aucpm_auth();
static int aucpm_bfill();
static int aucpm_valid();
static int aucpm_exit();

/* structures defined in au_scrns.c */

extern Scrn_parms	aucpm_scrn;
extern Scrn_desc	aucpm_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aucpm_scrn
#define STRUCTTEMPLATE	aucpm_struct
#define DESCTEMPLATE	aucpm_desc
#define FILLINSTRUCT	audit_users_groups_struct
#define FILLIN		aucpm_fill
#define TRAVERSERW	TRAV_RW

#define USER_TITLE_DESC		0
#define GROUP_TITLE_DESC	1
#define USER_UNDER_DESC		2
#define GROUP_UNDER_DESC	3
#define USER_SCRTOG_DESC	4
#define GROUP_SCRTOG_DESC	5
#define CURRENT_DESC		6
#define FUTURE_DESC		7

#define USER_SCRTOG_STRUCT	0
#define GROUP_SCRTOG_STRUCT	1
#define CURRENT_STRUCT		2
#define FUTURE_STRUCT		3
#define NSCRNSTRUCT		4

#define FIRSTDESC	USER_SCRTOG_DESC

static Scrn_struct	*aucpm_struct;
static AUDIT_USERS_GROUPS_STRUCT au_buf, *aucpm_fill = &au_buf;

static char *UserState;
static char *GroupState;

static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

aucpm_auth(argv, aufill)
	char **argv;
	AudUserGroup_fillin *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aucpm_auth");
	if (first_time) {
		AuditUsersGroupsStart();
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aucpm_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * Builds the fillin structure by retrieving the user and group lists from the
 * audit_parms file.  Returns 0 on success.
 */

aucpm_bfill(aufill)
	AudUserGroup_fillin *aufill;
{
	int ret;
	char **users_table;
	char **groups_table;
	int nusers;
	int ngroups;
	int i, j;

	ENTERFUNC("aucpm_bfill");

	/* Get the users and groups from the control file. */

	ret = AuditUsersGroupsGet(aufill);

	/*
	 * Get the total list of users and groups from /etc/group and
	 * the protected password database.
	 */

	GetAllGroups(&ngroups, &groups_table);
	if (ngroups == 0) {
		pop_msg("Unexpected error.",
		  "There are no groups in the authentication databases.");
		goto out;
	}
	GetAllUsers(&nusers, &users_table);
	if (nusers == 0) {
		free_cw_table(groups_table);
		pop_msg("Unexpected error.",
		  "There are no users in the authentication databases.");
		goto out;
	}

	/* Allocate the state tables */

	UserState = calloc(nusers, 1);
	GroupState = calloc(ngroups, 1);

	if (UserState == NULL || GroupState == NULL)
		MemoryError();

	/* Mark the state tables to match the lists in the control file */

	for (i = 0; i < aufill->nusers; i++)
		for (j = 0; j < nusers; j++)
			if (strcmp(aufill->users[i], users_table[j]) == 0)
				UserState[j] = (char) 1;

	for (i = 0; i < aufill->ngroups; i++)
		for (j = 0; j < ngroups; j++)
			if (strcmp(aufill->groups[i], groups_table[j]) == 0)
				GroupState[j] = (char) 1;

	/* Free the tables returned from AuditUsersGroupsGet() */

	AuditUsersGroupsFree(aufill);

	/* Connect the new tables into the structure */

	aufill->users = users_table;
	aufill->nusers = nusers;
	aufill->groups = groups_table;
	aufill->ngroups = ngroups;

	/* turn off the ability to update current if audit not enabled */

	switch (CheckAuditEnabled()) {
	case -1:	/* audit is running but there is no daemon */
	case 0:		/* audit is not enabled */
		aucpm_desc[CURRENT_DESC].inout = FLD_OUTPUT;
		break;
	case 1:		/* audit is enabled and daemon is running */
		aucpm_desc[CURRENT_DESC].inout = FLD_BOTH;
		break;
	}

out:
	EXITFUNC("aucpm_bfill");
	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

static
struct scrn_struct *
aucpm_bstruct(aufill, sptemplate)
	AudUserGroup_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	int i;
	int widest_type = 0;
	struct scrn_struct *sp;

	ENTERFUNC("aucpm_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[USER_SCRTOG_STRUCT].pointer = (char *) aufill->users;
	sp[USER_SCRTOG_STRUCT].filled = aufill->nusers;
	sp[USER_SCRTOG_STRUCT].validate = NULL;
	sp[USER_SCRTOG_STRUCT].state = UserState;

	sp[GROUP_SCRTOG_STRUCT].pointer = (char *) aufill->groups;
	sp[GROUP_SCRTOG_STRUCT].filled = aufill->ngroups;
	sp[GROUP_SCRTOG_STRUCT].validate = NULL;
	sp[GROUP_SCRTOG_STRUCT].state = GroupState;

	sp[CURRENT_STRUCT].pointer = &aufill->this;
	sp[CURRENT_STRUCT].filled = 1;
	sp[CURRENT_STRUCT].validate = NULL;
	sp[FUTURE_STRUCT].pointer = &aufill->future;
	sp[FUTURE_STRUCT].filled = 1;
	sp[FUTURE_STRUCT].validate = NULL;

	EXITFUNC("aucpm_bstruct");
	return 0;
}

/*
 * action routine.
 * Unload the state fields to the mask structure and update audit state
 */

static int
aucpm_action(aufill)
	AudUserGroup_fillin *aufill;
{
	int i;
	int nusers = 0, ngroups = 0;
	char **users_table, **groups_table;
	int count;

	ENTERFUNC("aucpm_action");
	/* Build new tables of users and groups that contain ones selected */
	for (i = 0; i < aufill->nusers; i++)
		if (UserState[i])
			nusers++;

	for (i = 0; i < aufill->ngroups; i++)
		if (GroupState[i])
			ngroups++;

	/* allocate tables if any users and/or groups are selected */

	if (nusers > 0) {
		users_table = alloc_cw_table(nusers, UNAMELEN + 1);
		if (users_table == (char **) 0)
			MemoryError();
	} else
		users_table = (char **) 0;

	if (ngroups > 0) {
		groups_table = alloc_cw_table(ngroups, NGROUPNAME + 1);
		if (groups_table == (char **) 0)
			MemoryError();
	} else
		groups_table = (char **) 0;
	
	/* set the new table to the users that have been selected */

	count = 0;
	if (nusers > 0)
		for (i = 0; i < aufill->nusers; i++)
			if (UserState[i])
				strcpy(users_table[count++],
					aufill->users[i]);

	/* set the new table to the groups that have been selected */

	count = 0;
	if (ngroups > 0)
		for (i = 0; i < aufill->ngroups; i++)
			if (GroupState[i])
				strcpy(groups_table[count++],
					aufill->groups[i]);

	/* free the tables that were used on the screen */

	free_cw_table(aufill->users);
	free_cw_table(aufill->groups);

	/* set the tables in the structures to the new selected users */

	aufill->users = users_table;
	aufill->nusers = nusers;
	aufill->groups = groups_table;
	aufill->ngroups = ngroups;

	/* Do the action */

	AuditUsersGroupsPut(aufill);

	EXITFUNC("aucpm_action");
	return 1;
}


static void
cpm_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	AudUserGroup_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("cpm_free");
	if (aufill->users) {
		free_cw_table(aufill->users);
		aufill->users = (char **) 0;
	}
	if (aufill->groups) {
		free_cw_table(aufill->groups);
		aufill->groups = (char **) 0;
	}
	if (UserState) {
		free(UserState);
		UserState = NULL;
	}
	if (GroupState) {
		free(GroupState);
		GroupState = NULL;
	}
	EXITFUNC("cpm_free");
	return;
}

/*
 * validate the structure -- make sure at least one of this/future set
 */

static int
aucpm_valid(argv, aufill)
	char **argv;
	AudUserGroup_fillin *aufill;
{
	int ret = 0;

	ENTERFUNC("aucpm_valid");
	if (aufill->this == (char) 0 && aufill->future == (char) 0) {
		pop_msg("At least one of 'This Session' and",
		  "'Future Sessions' must be selected");
		ERRFUNC("aucpm_valid", "neither this nor future selected");
		ret = 1;
	}
	EXITFUNC("aucpm_valid");
	return ret;
}

#define SETUPFUNC	aucpm_setup	/* defined by stemplate.c */
#define AUTHFUNC	aucpm_auth
#define BUILDFILLIN	aucpm_bfill

#define INITFUNC	aucpm_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aucpm_bstruct

#define ROUTFUNC	aucpm_exit		/* defined by stemplate.c */
#define VALIDATE	aucpm_valid
#define SCREENACTION	aucpm_action

#define FREEFUNC	aucpm_free		/* defined by stemplate.c */
#define FREESTRUCT	cpm_free

#include "stemplate.c"

#endif /* SEC_BASE */

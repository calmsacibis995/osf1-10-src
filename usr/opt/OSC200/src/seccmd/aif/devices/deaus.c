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
static char	*sccsid = "@(#)$RCSfile: deaus.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:02 $";
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
 * Routines to implement the authorized user list screen
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "gl_defs.h"
#include "IfDevices.h"
#include "userif.h"
#include "UIMain.h"
#include "valid.h"
#include "logging.h"

/* static routine definitions */

static int deaus_auth();
static int deaus_bfill();
static int deaus_valid();
static int deaus_exit();
static int AuthAllUserToggle();
static int AuthUserToggle();

/* structures defined in de_scrns.c */

extern Scrn_parms	deaus_scrn;
extern Scrn_desc	deaus_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	deaus_scrn
#define STRUCTTEMPLATE	deaus_struct
#define DESCTEMPLATE	deaus_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		deaus_fill

#define AUS_ALL_USERS_DESC	0
#define AUS_USERS_TITLE_DESC	1
#define AUS_USERS_DESC		2

#define AUS_ALL_USERS_STRUCT	0
#define AUS_USERS_STRUCT	1
#define NSCRNSTRUCT		2

#define FIRSTDESC	AUS_ALL_USERS_DESC

static Scrn_struct	*deaus_struct;
static struct device_fillin de_buf, *deaus_fill = &de_buf;

static char **UserTable;
static char *UserState;
static int NUsers;
static char no_auth_users;
static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

deaus_auth(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	static int first_time = 1;

	ENTERFUNC("deaus_auth");

	if (!IsDeviceSelected) {
		pop_msg("You must select a device before setting its"
		  "authorized user list.");
		return 1;
	}

	if (first_time) {
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("deaus_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * There is only one field.  Nothing to do.
 */

static int
deaus_bfill(defill)
	struct device_fillin *defill;
{
	int ret;
	int i, j;

	ENTERFUNC("deaus_bfill");

	GetAllUsers(&NUsers, &UserTable);
	if (NUsers == 0 || UserTable == (char **) 0)
		MemoryError();

	UserState = (char *) Calloc(NUsers, 1);
	if (UserState == NULL)
		MemoryError();

	ret = DevGetFillin(DevSelected, defill);

	if (ret == 0) {
		if (defill->auth_users == (char **) 0)
			no_auth_users = 1;
		else for (i = 0; i < defill->nauth_users; i++) {
			for (j = 0; j < NUsers; j++) {
				if (!strcmp(defill->auth_users[i],
					    UserTable[j]))
					UserState[j] = 1;
				break;
			}
		}
	}

	EXITFUNC("deaus_bfill");

	return 0;
}

/*
 * When toggling the All Users Allowed button, deselect all selected users.
 */
static int
AuthAllUserToggle()
{
	int ret;

	if (no_auth_users) {
		memset(UserState, '\0', NUsers);
		ret = 1;
	} else
		ret = 0;

	return ret;
}

/*
 * When toggling any user, remove the all users button
 */

static int
AuthUserToggle()
{
	int ret;
	struct scrn_struct *sp = PARMTEMPLATE.ss;

	if (no_auth_users) {
		no_auth_users = 0;
		sp[AUS_ALL_USERS_STRUCT].changed = 1;
		ret = 1;
	} else
		ret = 0;

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
deaus_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("deaus_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[AUS_ALL_USERS_STRUCT].pointer = &no_auth_users;
	sp[AUS_ALL_USERS_STRUCT].filled = 1;
	sp[AUS_ALL_USERS_STRUCT].val_act = AuthAllUserToggle;

	sp[AUS_USERS_STRUCT].pointer = (char *) UserTable;
	sp[AUS_USERS_STRUCT].filled = NUsers;
	sp[AUS_USERS_STRUCT].val_act = AuthUserToggle;
	sp[AUS_USERS_STRUCT].state = UserState;

	EXITFUNC("deaus_bstruct");
	return 0;
}

/*
 * action routine.
 * Rewrite default database
 */

static int
deaus_action(defill)
	struct device_fillin *defill;
{
	int ret;
	int i;

	ENTERFUNC("deaus_action");

	/* make a list from the number of users selected */

	if (no_auth_users) {
		defill->auth_users = (char **) 0;
		defill->nauth_users = 0;
	}
	else {
		int count;

		/* count number of users selected */

		count = 0;
		for (i = 0; i < NUsers; i++)
			if (UserState[i])
				count++;

		/* allocate a table for that many users */

		defill->auth_users = alloc_cw_table(count, 9);
		if (defill->auth_users == (char **) 0)
			MemoryError();
		defill->nauth_users = count;

		count = 0;
		for (i = 0; i < NUsers; i++)
			if (UserState[i])
				strcpy(defill->auth_users[count++],
				       UserTable[i]);
	}

	ret = DevChangeUserList(defill);

	EXITFUNC("deaus_action");
	return INTERRUPT;
}


static void
aus_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("aus_free");

	if (UserTable != (char **) 0) {
		free_cw_table(UserTable);
		UserTable = (char **) 0;
	}

	if (UserState != NULL) {
		free(UserState);
		UserState = NULL;
	}

	DevFreeFillin(defill);

	EXITFUNC("aus_free");
	return;
}

/*
 * validate the structure
 */

static int
deaus_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("deaus_valid");

	EXITFUNC("deaus_valid");
	return ret;
}

#define SETUPFUNC	deaus_setup	/* defined by stemplate.c */
#define AUTHFUNC	deaus_auth
#define BUILDFILLIN	deaus_bfill

#define INITFUNC	deaus_init		/* defined by stemplate.c */
#define BUILDSTRUCT	deaus_bstruct

#define ROUTFUNC	deaus_exit		/* defined by stemplate.c */
#define VALIDATE	deaus_valid
#define SCREENACTION	deaus_action

#define FREEFUNC	deaus_free		/* defined by stemplate.c */
#define FREESTRUCT	aus_free

#include "stemplate.c"

#endif /* SEC_BASE */

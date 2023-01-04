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
static char	*sccsid = "@(#)$RCSfile: aumb.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:03 $";
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
 * Routines to implement backup and deletion of audit sessions
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

static int aumb_auth();
static int aumb_bfill();
static int aumb_valid();
static int aumb_exit();
static int SessionValidate();

/* structures defined in au_scrns.c */

extern Scrn_parms	aumb_scrn;
extern Scrn_desc	aumb_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aumb_scrn
#define STRUCTTEMPLATE	aumb_struct
#define DESCTEMPLATE	aumb_desc
#define FILLINSTRUCT	ls_fillin
#define FILLIN		aumb_fill
#define TRAVERSERW	TRAV_RW

#define MB_SESSION_TITLE_DESC	0
#define MB_SESSION_UNDER_DESC	1
#define MB_SESSION_SCROLL_DESC	2
#define MB_BACKUP_TOGGLE_DESC	3
#define MB_DELETE_TOGGLE_DESC	4
#define MB_BACKDEV_TITLE_DESC	5
#define MB_BACKDEV_FILL_DESC	6

#define MB_SESSIONS_STRUCT	0
#define MB_BACKUP_STRUCT	1
#define MB_DELETE_STRUCT	2
#define MB_DEVICE_STRUCT	3
#define NSCRNSTRUCT		4

#define FIRSTDESC	MB_SESSION_SCROLL_DESC

static Scrn_struct	*aumb_struct;
static struct ls_fillin au_buf, *aumb_fill = &au_buf;

/* Button state variables */

static char Backup;
static char Delete;

/* Device name */

char DeviceName[21];

/* state variable for scrolling toggle session field */

char *SessionState;

static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

aumb_auth(argv, aufill)
	char **argv;
	struct ls_fillin *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aumb_auth");
	if (first_time) {
		BackupDeleteStart();
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aumb_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * Builds the fillin structure by retrieving the sessions that are
 * represented by log files in the log directory
 * audit_parms file.  Returns 0 on success.
 */

static int
aumb_bfill(aufill)
	struct ls_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumb_bfill");

	/* Get the audit sessions from the control file. */

	ret = AuditListSessionsGet(aufill);

	if (ret || aufill->nsessions == 0) {
		pop_msg("No audit sessions available for backup/delete",
			"This option cannot be exercised at this time");
		return 1;
	}

	/* allocate the state structure */

	SessionState = calloc(aufill->nsessions, 1);

	if (SessionState == NULL)
		MemoryError();

	/* Set up the other values */

	Backup = Delete = (char) 0;

	memset(DeviceName, '\0', sizeof DeviceName);

	EXITFUNC("aumb_bfill");
	return 0;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
aumb_bstruct(aufill, sptemplate)
	struct ls_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("aumb_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[MB_SESSIONS_STRUCT].pointer = (char *) aufill->sessions;
	sp[MB_SESSIONS_STRUCT].filled = aufill->nsessions;
	sp[MB_SESSIONS_STRUCT].validate = NULL;
	sp[MB_SESSIONS_STRUCT].state = SessionState;

	sp[MB_BACKUP_STRUCT].pointer = &Backup;
	sp[MB_BACKUP_STRUCT].filled = 1;
	sp[MB_BACKUP_STRUCT].validate = NULL;

	sp[MB_DELETE_STRUCT].pointer = &Delete;
	sp[MB_DELETE_STRUCT].filled = 1;
	sp[MB_DELETE_STRUCT].validate = NULL;

	sp[MB_DEVICE_STRUCT].pointer = DeviceName;
	sp[MB_DEVICE_STRUCT].filled = 1;
	sp[MB_DEVICE_STRUCT].validate = NULL;

	EXITFUNC("aumb_bstruct");
	return 0;
}

/*
 * action routine.
 * Perform backup and/or deletion of audit session.
 */

static int
aumb_action(aufill)
	struct ls_fillin *aufill;
{
	char **file_list = (char **) 0;
	int nfiles;
	int ret;
	int redo_ret;

	ENTERFUNC("aumb_action");

	/* get a file list of files affected by these sessions */

	ret = GetSessionFiles(aufill, &file_list, &nfiles, SessionState);

	/* If sessions to be backed up, back up files in the file list */

	if (ret == 0 && Backup)
		ret = BackupFiles(file_list, nfiles, DeviceName);

	/* If sessions to be deleted, delete files in the file list */

	if (ret == 0 && Delete)
		ret = DeleteFiles(file_list, nfiles);

	if (file_list != (char **) 0)
		FreeSessionFiles(file_list, nfiles);

	/*
	 * Now that action is complete, re-build the screen with current
	 * snapshot of audit sessions
	 */

	AuditListSessionsFree(aufill);

	redo_ret = AuditListSessionsGet(aufill);

	if (SessionState)
		free(SessionState);

	/* allocate the state structure */

	SessionState = calloc(aufill->nsessions, 1);
	if (SessionState == NULL)
		MemoryError();

	/*
	 * If we successfully did the function but couldn't re-initialize,
	 * quit from the screen.  Otherwise, leave it right here.
	 */

	if (redo_ret != 0)
		ret = QUIT;
	else
		ret = INTERRUPT;

	EXITFUNC("aumb_action");
	return ret;
}


static void
mb_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	struct ls_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("mb_free");
	AuditListSessionsFree(aufill);
	if (SessionState != NULL) {
		free(SessionState);
		SessionState = NULL;
	}
	EXITFUNC("mb_free");
	return;
}

/*
 * validate the structure
 * -- at least one of backup/delete set
 * -- at least one session is selected
 * -- the device is in the device assignment database (SEC_ARCH).
 * -- not trying to delete the current session.
 */

static int
aumb_valid(argv, aufill)
	char **argv;
	struct ls_fillin *aufill;
{
	int ret = 0;
	int i;
	int count = 0;
	int session;

	ENTERFUNC("aumb_valid");

	/* Check at least one of backup/delete set */

	if (Backup == (char) 0 && Delete == (char) 0) {
		pop_msg("You did not specify an action for this screen.",
			"Please select Backup and/or Delete.");
		ERRFUNC("aumb_valid", "neither backup nor delete selected");
		ret = 1;
	}

	/*
	 * Count the number of sessions and make sure not trying to delete
	 * the current one
	 */

	if (ret == 0) {

	    if (Delete)
		session = CurrentAuditSession();

	    for (i = 0; i < aufill->nsessions; i++) {
		if (SessionState[i])
			count++;
		if (Delete) {
			if (SessionState[i] &&
			    session != -1 &&
			    session == atol(aufill->sessions[i])) {
				pop_msg(
			  "You may not delete the current session.",
			  "The selection of that session has been removed.");
				SessionState[i] = 0;
				ret = 1;
			}
		}
	}
	
	/* Check that at least one session was selected */

	if (ret == 0 && count == 0) {
		pop_msg(
		  "You must select at least one session for Backup/Delete.",
		  "Please select the session(s) to backup and/or delete.");
		ret = 1;
	    }
	}

#if SEC_ARCH
	if (ret == 0 && Backup) {
		struct dev_asg *dv, *getdvagent();
		int found = 0;

		/* Check that the device is in the device assignment database */

		setdvagent();
		while ((dv = getdvagent()) && !found) {
			int i;
			char *cp;

			if (strcmp(dv->ufld.fd_name, DeviceName) == 0)
				break;

			if (dv->uflg.fg_devs)
				for (i = 0; cp = dv->ufld.fd_devs[i]; i++)
					if (strcmp(cp, DeviceName) == 0) {
						found = 1;
						break;
					}
		}
		if (dv == (struct dev_asg *) 0) {
			pop_msg(
			  "The device you specified is not listed in",
			  "the device assignment database.");
			ret = 1;
		}
	}
#endif

	EXITFUNC("aumb_valid");
	return ret;
}

#define SETUPFUNC	aumb_setup	/* defined by stemplate.c */
#define AUTHFUNC	aumb_auth
#define BUILDFILLIN	aumb_bfill

#define INITFUNC	aumb_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aumb_bstruct

#define ROUTFUNC	aumb_exit		/* defined by stemplate.c */
#define VALIDATE	aumb_valid
#define SCREENACTION	aumb_action

#define FREEFUNC	aumb_free		/* defined by stemplate.c */
#define FREESTRUCT	mb_free

#include "stemplate.c"

#endif /* SEC_BASE */

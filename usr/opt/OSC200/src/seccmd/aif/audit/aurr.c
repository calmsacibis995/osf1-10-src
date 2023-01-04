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
static char	*sccsid = "@(#)$RCSfile: aurr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:28 $";
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
 * Routines to implement the generating reports for audit sessions
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

static int aurr_auth();
static int aurr_bfill();
static int aurr_valid();
static int aurr_exit();
static int SessionToggle();
static int SelFileToggle();

/* structures defined in au_scrns.c */

extern Scrn_parms	aurr_scrn;
extern Scrn_desc	aurr_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aurr_scrn
#define STRUCTTEMPLATE	aurr_struct
#define DESCTEMPLATE	aurr_desc
#define FILLINSTRUCT	ls_fillin
#define FILLIN		aurr_fill
#define TRAVERSERW	TRAV_RW

#define RR_MAIN_TITLE_DESC	0
#define RR_SESSION_TITLE_DESC	1
#define RR_SESSION_UNDER_DESC	2
#define RR_SESSION_DESC		3
#define RR_SELFILE_TITLE_DESC	4
#define RR_RPTNAME_TITLE_DESC	5
#define RR_SELFILE_DESC		6
#define RR_RPTNAME_DESC		7

#define RR_SESSION_STRUCT	0
#define RR_SELFILE_STRUCT	1
#define RR_RPTNAME_STRUCT	2
#define NSCRNSTRUCT		3

#define FIRSTDESC	RR_SESSION_DESC

static Scrn_struct	*aurr_struct;
static AUDIT_LS_STRUCT au_buf, *aurr_fill = &au_buf;

static int IsISSO;

static int NumberSelFiles = 0;
static char **SelFileTable = (char **) 0;
static char *SelFileState = NULL;
static char *SessionState = NULL;
static char ReportFile[ENTSIZ + 1];
static int SessionNumber;
static int SelFileNumber;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

aurr_auth(argv, aufill)
	char **argv;
	AUDIT_LS_STRUCT *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aurr_auth");
	if (first_time) {
		AuditListSessionsStart();
		SelectionFileStart();
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aurr_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * Builds the fillin structure by retrieving the sessions and
 * report filenames from the appropriate place.
 * Returns 0 on success.
 */

aurr_bfill(aufill)
	AUDIT_LS_STRUCT *aufill;
{
	int ret;

	ENTERFUNC("aurr_bfill");

	ret = SelectionFileGet(&NumberSelFiles, &SelFileTable);
	if (ret == 0 && NumberSelFiles == 0) {
		pop_msg(
		  "There are no report selection files.",
		  "You must create one to produce an audit report.");
		ret = 1;
	}

	if (ret == 0) {
		ret = AuditListSessionsGet(aufill);
		if (ret == 0) {
			if (aufill->nsessions == 0) {
				pop_msg(
		"There are no audit sessions on the system.",
		"You may not create a report without audit data.");
				ret = 1;
			}
		}

		if (ret == 0) {
			SelFileState = Calloc(NumberSelFiles, 1);
			SessionState = Calloc(aufill->nsessions, 1);
			if (SelFileState == NULL || SessionState == NULL)
				MemoryError();
			memset(ReportFile, '\0', sizeof ReportFile);

			/* Initialize fields (keep track) */

			SelFileState[0] = (char) 1;
			SessionState[0] = (char) 1;
			SelFileNumber = 0;
			SessionNumber = 0;
		}
		else {
			free_cw_table(SelFileTable);
			SelFileTable = (char **) 0;
		}
	}
		
	EXITFUNC("aurr_bfill");
	return ret;
}

/*
* Builds the scrn_struct for this screen.
*/

static int
aurr_bstruct(aufill, sptemplate)
	AUDIT_LS_STRUCT *aufill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("aurr_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;
	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[RR_SESSION_STRUCT].pointer = (char *) aufill->sessions;
	sp[RR_SESSION_STRUCT].state = SessionState;
	sp[RR_SESSION_STRUCT].filled = aufill->nsessions;
	sp[RR_SESSION_STRUCT].val_act = SessionToggle;

	sp[RR_SELFILE_STRUCT].pointer = (char *) SelFileTable;
	sp[RR_SELFILE_STRUCT].state = SelFileState;
	sp[RR_SELFILE_STRUCT].filled = NumberSelFiles;
	sp[RR_SELFILE_STRUCT].val_act = SelFileToggle;

	sp[RR_RPTNAME_STRUCT].pointer = ReportFile;
	sp[RR_RPTNAME_STRUCT].filled = 1;

	EXITFUNC("aurr_bstruct");
	return 0;
}

/*
 * action routine.
 * set up the appropriate parameters to the ReduceProgram function.
 */

static int
aurr_action(aufill)
	AUDIT_LS_STRUCT *aufill;
{
	int i;
	int session;
	char *selfile;
	int ret;

	ENTERFUNC("aurr_action");
	for (i = 0; i < aufill->nsessions; i++)
		if (SessionState[i])
			session = atol(aufill->sessions[i]);

	for (i = 0; i < NumberSelFiles; i++)
		if (SelFileState[i])
			selfile = SelFileTable[i];

	ret = ReduceProgram(session, selfile, ReportFile);

	EXITFUNC("aurr_action");
	return 1;
}


static void
rr_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	AUDIT_LS_STRUCT *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("rr_free");
	AuditListSessionsFree(aufill);
	if (SelFileTable != (char **) 0) {
		free_cw_table(SelFileTable);
		SelFileTable = (char **) 0;
	}
	if (SelFileState != NULL) {
		free(SelFileState);
		SelFileState = NULL;
	}
	if (SessionState != NULL) {
		free(SessionState);
		SessionState = NULL;
	}
	EXITFUNC("rr_free");
	return;
}

/*
 * validate the structure
 * -- require that file name is filled in
 */

static int
aurr_valid(argv, aufill)
	char **argv;
	AUDIT_LS_STRUCT *aufill;
{
	int ret = 0;

	ENTERFUNC("aurr_valid");
	
	if (ReportFile[0] == '\0') {
		pop_msg(
		  "You must specify an output file name."
		  "Please fill in the 'Report Name:' field.");
		ret = 1;
	}
	if (strrchr(ReportFile, '/')) {
		pop_msg(
		  "The report name may not contain a '/' character.",
		  "Please re-enter the report name.");
		ret = 1;
	}
	if (!strcmp(ReportFile, ".") || !strcmp(ReportFile, "..")) {
		pop_msg(
		  "The report name may not be '.' or '..'.",
		  "Please re-enter the report name.");
		ret = 1;
	}

	EXITFUNC("aurr_valid");
	return ret;
}

/*
 * Routine called when session is toggled.
 * Implements session radio buttons.
 * If push down the one that's currently selected, make it stay down.
 * If push down another one, de-select the old one.
 */

static int
SessionToggle()
{
	int i;

	ENTERFUNC("SessionToggle");

	for (i = 0; i < aurr_fill->nsessions; i++)
		if (SessionState[i] == 0 && i == SessionNumber) {
			/* turned off the one we had selected */
			SessionState[i] = 1;
			break;
		}
		else if (SessionState[i] && i != SessionNumber) {
			/* turned on another one */
			SessionState[SessionNumber] = 0;
			SessionNumber = i;
			break;
		}
	EXITFUNC("SessionToggle");
	return 1;
		
}

/*
 * Routine called when selection file is toggled.
 * Implements selection file radio buttons.
 * If push down the one that's currently selected, make it stay down.
 * If push down another one, de-select the old one.
 */

static int
SelFileToggle()
{
	int i;

	ENTERFUNC("SelFileToggle");

	for (i = 0; i < NumberSelFiles; i++)
		if (SelFileState[i] == 0 && i == SelFileNumber) {
			/* turned off the one we had selected */
			SelFileState[i] = 1;
			break;
		}
		else if (SelFileState[i] && i != SelFileNumber) {
			/* turned on another one */
			SelFileState[SelFileNumber] = 0;
			SelFileNumber = i;
			break;
		}
	EXITFUNC("SelFileToggle");
	return 1;
		
}

#define SETUPFUNC	aurr_setup		/* defined by stemplate.c */
#define AUTHFUNC	aurr_auth
#define BUILDFILLIN	aurr_bfill

#define INITFUNC	aurr_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aurr_bstruct

#define ROUTFUNC	aurr_exit		/* defined by stemplate.c */
#define VALIDATE	aurr_valid
#define SCREENACTION	aurr_action

#define FREEFUNC	aurr_free		/* defined by stemplate.c */
#define FREESTRUCT	rr_free

#include "stemplate.c"

#endif /* SEC_BASE */

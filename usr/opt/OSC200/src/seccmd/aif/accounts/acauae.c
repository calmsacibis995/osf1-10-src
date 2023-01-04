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
static char	*sccsid = "@(#)$RCSfile: acauae.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:31 $";
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
 * Routines to implement modification of per-user audit events
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "gl_defs.h"
#include "userif.h"
#include "UIMain.h"
#include "valid.h"
#include "logging.h"
#include "Accounts.h"
#include "IfAudit.h"

/* static routine definitions */

static int acauae_auth();
static int acauae_bfill();
static int acauae_valid();
static int acauae_exit();
static int StateToggle();
static int acauae_local_rout();
static int acauae();
static int acauae_setup(), acauae_init(), acauae_free();

/* structures defined in au_scrns.c */

Scrn_parms	acauae_scrn;

#define LEFT_EVENT_COL 1
#define LEFT_SET_COL 30
#define LEFT_DFLT_COL 34
#define RIGHT_EVENT_COL 40
#define RIGHT_SET_COL 71
#define RIGHT_DFLT_COL 75
#define EVENT_LEN 28

/*
 * Template screen desc for the dynamically allocated screen
 */

Scrn_desc acauae_templ_desc[] = {
	{ 0, LEFT_EVENT_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Event Type" },
	{ 0, LEFT_SET_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
	{ 0, LEFT_DFLT_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Dflt" },
	{ 0, RIGHT_EVENT_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Event Type" },
	{ 0, RIGHT_SET_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
	{ 0, RIGHT_DFLT_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Dflt" },
	{ 1, LEFT_EVENT_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----- ----" },
	{ 1, LEFT_SET_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, LEFT_DFLT_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },
	{ 1, RIGHT_EVENT_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----- ----" },
	{ 1, RIGHT_SET_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, RIGHT_DFLT_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },
};
#define FIRSTDESC (NUMDESCS(acauae_templ_desc) + 1)

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	acauae_scrn
#define STRUCTTEMPLATE	acauae_struct
#define DESCTEMPLATE	acauae_desc
#define FILLINSTRUCT	prpw_if
#define FILLIN		pwfill
#define NSCRNSTRUCT	(AUDIT_MAX_EVENT * 2)

static Scrn_struct	*acauae_struct;
static struct prpw_if pwbuf, *pwfill = &pwbuf;
static Scrn_desc	*acauae_desc;
static Menu_scrn *acauae_menu;

static char **AcaueEventTable;	/* Event Table names */
static int AcaueNEvents;	/* size of event table */
static char *DefEventState;	/* system default event state */
static char *UserCtrlState;	/* user control mask */
static char *UserDispState;	/* user disposition mask */
static char *ScrnUserCtrlState;	/* user control mask */
static char *ScrnUserDispState;	/* user disposition mask */

uchar acauae_title[] = "USER AUDIT EVENTS";
Scrn_hdrs acauae_hdrs = {
	acauae_title,
	cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	NULL, cmds_titem, cmds_line1
};

/* NOTE: menu_scrn, scrn_descs, and even type get filled in dynamically */

SKIP_PARMF(acauae_scrn, SCR_FILLIN, acauae_templ_desc, NULL, NULL, &acauae_hdrs,
	acauae_setup, acauae_init, acauae_free);

static int IsISSO;

extern char *Calloc(), *Malloc();

int
do_acauae()
{
	return acauae(1);
}

int
do_acduae()
{
	return acauae(0);
}

/*
 * called from top screen -- dynamically build the data structures
 * to describe the event table if it is the first time we are called.
 */

static int
acauae(allow_update)
	int allow_update;
{
	static int first_time = 1;
	int desc_cols;		/* how many columns of descs */
	int odd_number;		/* is there an odd number of columns? */
	int desc_items;		/* number of descs for the Events (3 per) */
	int numdescs;		/* number of descs for the whole table */
	int i;
	int row;
	Scrn_desc *dp;

	if (first_time) {

		/* allocate tables according to the number of events */

		AuditAllocEventMaskTable(&AcaueNEvents, &AcaueEventTable);
		if (AcaueNEvents == 0) {
			pop_msg("This screen is not available",
			  "There are no events recorded on the system.");
			return 1;
		}
		DefEventState = Calloc(AcaueNEvents, 1);
		UserCtrlState = Calloc(AcaueNEvents, 1);
		UserDispState = Calloc(AcaueNEvents, 1);
		ScrnUserCtrlState = Calloc(AcaueNEvents, 1);
		ScrnUserDispState = Calloc(AcaueNEvents, 1);

		if (AcaueEventTable == (char **) 0 ||
		    DefEventState == NULL ||
		    UserCtrlState == NULL ||
		    UserDispState == NULL ||
		    ScrnUserCtrlState == NULL ||
		    ScrnUserDispState == NULL)
			MemoryError();

		/* dynamically allocate the desc table */

		desc_cols = (AcaueNEvents + 1) / 2;

		odd_number = (AcaueNEvents & 1);
		desc_items = desc_cols * 2 - odd_number;
		numdescs = NUMDESCS(acauae_templ_desc) + (3 * desc_items);
	
		acauae_desc = (Scrn_desc *) Calloc(numdescs, sizeof(Scrn_desc));
		if (acauae_desc == (Scrn_desc *) 0)
			MemoryError();

		/* Copy the template */

		for (i = 0; i < NUMDESCS(acauae_templ_desc); i++)
			acauae_desc[i] = acauae_templ_desc[i];

		row = acauae_desc[NUMDESCS(acauae_templ_desc) - 1].row;

		dp = &acauae_desc[NUMDESCS(acauae_templ_desc)];

		/* set up the desc table */

		for (i = 0; i < desc_items; i++, dp += 3) {
			if ((i % 2) == 0) { /* left column */
				dp[0].col = LEFT_EVENT_COL;
				dp[1].col = LEFT_SET_COL + 1;
				dp[2].col = LEFT_DFLT_COL + 1;
				row++;
			} else {
				dp[0].col = RIGHT_EVENT_COL;
				dp[1].col = RIGHT_SET_COL + 1;
				dp[2].col = RIGHT_DFLT_COL + 1;
			}
			dp[0].row = dp[1].row = dp[2].row = row;
			dp[0].type = FLD_PROMPT;
			dp[1].type = dp[2].type = FLD_TOGGLE;
			dp[0].inout = FLD_OUTPUT;
			dp[1].inout = dp[2].inout = FLD_BOTH;
			dp[1].len = dp[2].len = 1;
			dp[0].required = dp[1].required = dp[2].required = NO;

			dp[0].prompt = Malloc(EVENT_LEN + 1);
			if (dp[0].prompt == NULL)
				MemoryError();

			/* strip off last word (or more) if won't fit */

			if (strlen(AcaueEventTable[i]) > EVENT_LEN) {
				char *cp;
				int len;

				cp = strrchr(AcaueEventTable[i], ' ');
				if (cp == NULL)
					cp =
					 &AcaueEventTable[i][strlen(AcaueEventTable[i])];

				if ((uint)cp - (uint)AcaueEventTable[i] > EVENT_LEN)
					len = EVENT_LEN;
				else
					len = (uint) cp - (uint) AcaueEventTable[i];
				strncpy(dp[0].prompt, AcaueEventTable[i], len);
			} else
				strcpy(dp[0].prompt, AcaueEventTable[i]);

		}

		/* allocate the menu screen */

		acauae_menu = (Menu_scrn *)
			Calloc(2 * desc_items, sizeof(Menu_scrn));

		for (i = 0; i < 2 * desc_items; i++) {
			acauae_menu[i].choice = i + 1;
			acauae_menu[i].type = M_ROUTINE;
			acauae_menu[i].next_routine = acauae_local_rout;
			acauae_menu[i].nparams = 0;
		}

		/* fill in the screen parameters */

		acauae_scrn.ndescs = numdescs;
		acauae_scrn.sd = acauae_desc;
		acauae_scrn.ms = acauae_menu;

		/* whew! all done */

		first_time = 0;
	}

	/* set up for update if modifying user */

	acauae_scrn.scrntype = allow_update ? SCR_FILLIN : SCR_NOCHANGE;

	return traverse(&acauae_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

acauae_auth(argv, pwfill)
	char **argv;
	struct prpw_if *pwfill;
{
	static int first_time = 1;

	ENTERFUNC("acauae_auth");
	EXITFUNC("acauae_auth");
	return 0;
}

/*
 * Grab the root directory and directory list from the parameters file
 * Allocate a structure for the screen that is one larger than the
 * current table.  Expand dynamically as the user adds new directories.
 */

static int
acauae_bfill(pwfill)
	struct prpw_if *pwfill;
{
	int ret;
	struct pr_passwd *prpwd = &pwfill->prpw;
	audmask_fillin aubuf, *aufill = &aubuf;
	int i;
	static int first_time = 1;

	ENTERFUNC("acauae_bfill");

	ret = XGetUserInfo(gl_user, pwfill);

	if (ret == 0) {

	    /* Get the default events */

	    if (first_time) {
		AuditEvMaskStart();
		first_time = 0;
	    }

	    if (AuditGetMaskStructure(aufill)) {
			pop_msg("System default audit events are not set yet.",
			  "They must be set before this screen may be run.");
			return 1;
	    }

	    for (i = 0; i < AcaueNEvents; i++) {

		DefEventState[i] = ISBITSET(aufill->au.event_mask, i + 1);

		if (!prpwd->uflg.fg_auditcntl ||
		    !ISBITSET(prpwd->ufld.fd_auditcntl, i + 1)) {
			UserCtrlState[i] = 1;
			UserDispState[i] = DefEventState[i];
		} else {
			UserCtrlState[i] = 0;
			UserDispState[i] =
				ISBITSET(prpwd->ufld.fd_auditdisp, i + 1);
		}
	    }
	}
	EXITFUNC("acauae_bfill");
	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
acauae_bstruct(pwfill, sptemplate)
	struct prpw_if *pwfill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;
	int i;

	ENTERFUNC("acauae_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	memcpy(ScrnUserCtrlState, UserCtrlState, AcaueNEvents);
	memcpy(ScrnUserDispState, UserDispState, AcaueNEvents);

	for (i = 0; i < AcaueNEvents; i++, sp += 2) {
		sp[0].pointer = &ScrnUserDispState[i];
		sp[1].pointer = &ScrnUserCtrlState[i];
		sp[0].filled = sp[1].filled = 1;
		sp[0].changed = sp[1].changed = 1;
		sp[0].val_act = sp[1].val_act = StateToggle;
		sp[1].filled = sp[1].filled = 1;
		sp[1].changed = sp[1].changed = 1;
		sp[1].val_act = sp[1].val_act = StateToggle;
	}

	EXITFUNC("acauae_bstruct");
	return 0;
}

/*
 * Routine called when something changed.
 * If the user toggled the Default ON, change the set to whatever the default is
 * If the user toggled the Default OFF, leave it alone.
 * If the user toggled the Set on, turn off the default if it is on.
 * If the user toggled the Set off, turn off the default if it is on.
 */

static int
StateToggle()
{
	int i;
	int ret = 0;

	for (i = 0; i < AcaueNEvents && !ret; i++) {
		if (ScrnUserCtrlState[i] && !UserCtrlState[i]) {

			/* user toggled the default on */

			UserCtrlState[i] = 1;
			UserDispState[i] = ScrnUserDispState[i] =
			  DefEventState[i];
			ret = 1;

		} else if (!ScrnUserCtrlState[i] && UserCtrlState[i]) {

			/* user toggled the default off */

			UserCtrlState[i] = 0;
			ret = 1;

		} else if (ScrnUserDispState[i] && !UserDispState[i]) {

			/* User toggled the Set on */

			UserDispState[i] = 1;
			ScrnUserCtrlState[i] = UserCtrlState[i] = 0;
			ret = 1;

		} else if (!ScrnUserDispState[i] && UserDispState[i]) {

			/* User toggled the Set off */

			UserDispState[i] = 0;
			ScrnUserCtrlState[i] = UserCtrlState[i] = 0;
			ret = 1;
		}
	}
	return ret;
}


/*
 * action routine.
 * Perform restore of audit session.
 */

static int
acauae_action(pwfill)
	struct prpw_if *pwfill;
{
	int ret;
	int i;
	int all_default;
	struct pr_passwd *prpwd = &pwfill->prpw;


	ENTERFUNC("acauae_action");

	for (i = 0; i < AcaueNEvents; i++)
		if (!UserCtrlState[i])
			/* something is not default */
			break;

	if (i == AcaueNEvents) {  /* all defaults */
		prpwd->uflg.fg_auditcntl = 0;
		prpwd->uflg.fg_auditdisp = 0;
	} else {
		prpwd->uflg.fg_auditcntl = 1;
		prpwd->uflg.fg_auditdisp = 1;

		for (i = 0; i < AcaueNEvents; i++) {
			if (UserCtrlState[i]) /* use default */
				RMBIT(prpwd->ufld.fd_auditcntl, i + 1);
			else {
				ADDBIT(prpwd->ufld.fd_auditcntl, i + 1);
				if (UserDispState[i])
					ADDBIT(prpwd->ufld.fd_auditdisp, i + 1);
				else
					RMBIT(prpwd->ufld.fd_auditdisp, i + 1);
			}
		}
	}
	ret = XWriteUserInfo(pwfill);

	EXITFUNC("acauae_action");

	return INTERRUPT;
}


static void
auae_free(argv, pwfill, nstructs, pp, dp, sp)
	char **argv;
	struct prpw_if *pwfill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("auae_free");
	EXITFUNC("auae_free");
	return;
}

/*
 * validate the structure -- the logic is in AuditParametersCheck().
 */

static int
acauae_valid(argv, pwfill)
	char **argv;
	struct prpw_if *pwfill;
{
	ENTERFUNC("acauae_valid");
	EXITFUNC("acauae_valid");
	return 0;
}

#define SETUPFUNC	acauae_setup		/* defined by stemplate.c */
#define AUTHFUNC	acauae_auth
#define BUILDFILLIN	acauae_bfill

#define INITFUNC	acauae_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acauae_bstruct

#define ROUTFUNC	acaue_rout		/* defined by stemplate.c */
#define VALIDATE	acauae_valid
#define SCREENACTION	acauae_action

#define FREEFUNC	acauae_free		/* defined by stemplate.c */
#define FREESTRUCT	auae_free

/*
 * Since the number of scrn_structs is variable, here is a local copy of
 * the route function that calls with our local copy of the number of
 * structs
 */

acauae_local_rout(argv)
	char **argv;
{
        return (scrnexit (argv, (char *) FILLIN,
          VALIDATE, SCREENACTION, FIRSTDESC, AcaueNEvents * 2,
          &PARMTEMPLATE, DESCTEMPLATE, STRUCTTEMPLATE)
        );
}

#include "stemplate.c"

#endif /* SEC_BASE */

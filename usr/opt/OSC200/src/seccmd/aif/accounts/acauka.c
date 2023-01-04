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
static char	*sccsid = "@(#)$RCSfile: acauka.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:38 $";
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
 * Routines to implement modification of per-user kernel auths/base privs
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

static int acauka_auth();
static int acauka_bfill();
static int acauka_valid();
static int acauka_exit();
static int StateToggle();
static int acauka_local_rout();
static int acauka();
static int acauka_setup(), acauka_init(), acauka_free();

Scrn_parms	acauka_scrn;

#define LEFT_AUTH_COL	0
#define LEFT_KER_COL	18
#define LEFT_BASE_COL	22
#define MIDDLE_AUTH_COL	27
#define MIDDLE_KER_COL	45
#define MIDDLE_BASE_COL	49
#define RIGHT_AUTH_COL	54
#define RIGHT_KER_COL	72
#define RIGHT_BASE_COL	76

#define AUTH_LEN 17
/*
 * Template screen desc for the dynamically allocated screen
 */

Scrn_desc acauka_templ_desc[] = {
	{ 0, LEFT_AUTH_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Authorization" },
	{ 0, LEFT_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Ker" },
	{ 0, LEFT_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Base" },

	{ 0, MIDDLE_AUTH_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Authorization" },
	{ 0, MIDDLE_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Ker" },
	{ 0, MIDDLE_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Base" },

	{ 0, RIGHT_AUTH_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Authorization" },
	{ 0, RIGHT_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Ker" },
	{ 0, RIGHT_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Base" },

	{ 1, LEFT_AUTH_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "-------------" },
	{ 1, LEFT_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, LEFT_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },

	{ 1, MIDDLE_AUTH_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"-------------" },
	{ 1, MIDDLE_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, MIDDLE_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },

	{ 1, RIGHT_AUTH_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "-------------" },
	{ 1, RIGHT_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, RIGHT_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },

	{ 2, LEFT_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "S D" },
	{ 2, LEFT_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "S D" },
	{ 2, MIDDLE_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "S D" },
	{ 2, MIDDLE_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "S D" },
	{ 2, RIGHT_KER_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "S D" },
	{ 2, RIGHT_BASE_COL, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "S D" },
};
#define FIRSTDESC (NUMDESCS(acauka_templ_desc) + 1)

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	acauka_scrn
#define STRUCTTEMPLATE	acauka_struct
#define DESCTEMPLATE	acauka_desc
#define FILLINSTRUCT	prpw_if
#define FILLIN		pwfill
#define NSCRNSTRUCT	(AUDIT_MAX_EVENT * 2)

static Scrn_struct	*acauka_struct;
static struct prpw_if pwbuf, *pwfill = &pwbuf;
static Scrn_desc	*acauka_desc;
static Menu_scrn *acauka_menu;

static char **AcaukaAuthTable;	/* Authorization Table names */
static int AcaukaNAuths;	/* size of auth table */
static char *DefKAState;	/* system default kernel auth state */
static char *DefBPState;	/* system default base priv state */
static char UseDefKAs;		/* whether to use default KAs */
static char UseDefBPs;		/* whether to use default BPs */
static char ScrnUseDefKAs;	/* whether to use default KAs (on screen) */
static char ScrnUseDefBPs;	/* whether to use default BPs (on screen) */
static char *UserKAState;	/* user kernel auths (saved) */
static char *UserBPState;	/* user base privs (saved) */
static char *ScrnUserKAState;	/* user kernel auths (as on screen) */
static char *ScrnUserBPState;	/* user base privs (as on screen) */

uchar acauka_title[] = "USER KERNEL AUTHORIZATIONS/BASE PRIVILEGES";
Scrn_hdrs acauka_hdrs = {
	acauka_title,
	cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	NULL, cmds_titem, cmds_line1
};

/* NOTE: menu_scrn, scrn_descs, and even type get filled in dynamically */

SKIP_PARMF(acauka_scrn, SCR_FILLIN, acauka_templ_desc, NULL, NULL, &acauka_hdrs,
	acauka_setup, acauka_init, acauka_free);

static int IsISSO;

extern char *Calloc(), *Malloc();

int
do_acauka()
{
	return acauka(1);
}

int
do_acduka()
{
	return acauka(0);
}

/*
 * called from top screen -- dynamically build the data structures
 * to describe the event table if it is the first time we are called.
 */

static int
acauka(allow_update)
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
	int priv_number = 0;

	if (first_time) {

		/* allocate tables according to the number of auths */

		AcaukaNAuths = 0;
		for (i = 0; i < SEC_MAX_SPRIV; i++)
			if (sys_priv[i].name && sys_priv[i].name[0])
				AcaukaNAuths++;
				
		AcaukaAuthTable = (char **) alloc_table(sys_priv, AcaukaNAuths);

		DefKAState = Calloc(AcaukaNAuths, 1);
		DefBPState = Calloc(AcaukaNAuths, 1);
		UserKAState = Calloc(AcaukaNAuths, 1);
		UserBPState = Calloc(AcaukaNAuths, 1);
		ScrnUserKAState = Calloc(AcaukaNAuths, 1);
		ScrnUserBPState = Calloc(AcaukaNAuths, 1);

		if (AcaukaAuthTable == (char **) 0 ||
		    DefKAState == NULL ||
		    DefBPState == NULL ||
		    UserKAState == NULL ||
		    UserBPState == NULL ||
		    ScrnUserKAState == NULL ||
		    ScrnUserBPState == NULL)
			MemoryError();

		/* dynamically allocate the desc table */

		desc_cols = (AcaukaNAuths + 2) / 3;

		odd_number = 3 - (AcaukaNAuths % 3);
		if (odd_number == 3)
			odd_number = 0;

		desc_items = AcaukaNAuths * 5; /* prompt, ker/base s & d */

		numdescs = NUMDESCS(acauka_templ_desc) + AcaukaNAuths * 5;
	
		acauka_desc = (Scrn_desc *) Calloc(numdescs, sizeof(Scrn_desc));
		if (acauka_desc == (Scrn_desc *) 0)
			MemoryError();

		/* Copy the template */

		for (i = 0; i < NUMDESCS(acauka_templ_desc); i++)
			acauka_desc[i] = acauka_templ_desc[i];

		row = acauka_desc[NUMDESCS(acauka_templ_desc) - 1].row;

		dp = &acauka_desc[NUMDESCS(acauka_templ_desc)];

		/* set up the desc table */

		for (i = 0; i < AcaukaNAuths; i++, dp += 5, priv_number++) {

			/* find the next privilege in the list */

			while (sys_priv[priv_number].name == NULL ||
			       sys_priv[priv_number].name[0] == '\0')
				priv_number++;

			switch (i % 3) {
			case 0: /* left column */
				dp[0].col = LEFT_AUTH_COL;
				dp[1].col = LEFT_KER_COL;
				dp[2].col = LEFT_KER_COL + 2;
				dp[3].col = LEFT_BASE_COL;
				dp[4].col = LEFT_BASE_COL + 2;
				row++;
				break;
			case 1: /* middle column */
				dp[0].col = MIDDLE_AUTH_COL;
				dp[1].col = MIDDLE_KER_COL;
				dp[2].col = MIDDLE_KER_COL + 2;
				dp[3].col = MIDDLE_BASE_COL;
				dp[4].col = MIDDLE_BASE_COL + 2;
				break;
			case 2: /* right column */
				dp[0].col = RIGHT_AUTH_COL;
				dp[1].col = RIGHT_KER_COL;
				dp[2].col = RIGHT_KER_COL + 2;
				dp[3].col = RIGHT_BASE_COL;
				dp[4].col = RIGHT_BASE_COL + 2;
				break;
			}
			dp[0].row = dp[1].row = dp[2].row =
			dp[3].row = dp[4].row = row;
			dp[0].type = FLD_PROMPT;
			dp[1].type = dp[2].type = FLD_TOGGLE;
			dp[3].type = dp[4].type = FLD_TOGGLE;
			dp[0].inout = FLD_OUTPUT;
			dp[1].inout = dp[2].inout = FLD_BOTH;
			dp[3].inout = dp[4].inout = FLD_BOTH;
			dp[1].len = dp[2].len = 1;
			dp[3].len = dp[4].len = 1;
			dp[0].required = dp[1].required = dp[2].required =
			dp[3].required = dp[4].required = NO;

			dp[0].prompt = Malloc(AUTH_LEN + 1);
			if (dp[0].prompt == NULL)
				MemoryError();

			/* strip off last word (or more) if won't fit */

			strncpy(dp[0].prompt, sys_priv[priv_number].name,
			  AUTH_LEN);

		}

		/* allocate the menu screen */

		acauka_menu = (Menu_scrn *)
			Calloc(4 * AcaukaNAuths, sizeof(Menu_scrn));

		for (i = 0; i < 4 * AcaukaNAuths; i++) {
			acauka_menu[i].choice = i + 1;
			acauka_menu[i].type = M_ROUTINE;
			acauka_menu[i].next_routine = acauka_local_rout;
			acauka_menu[i].nparams = 0;
		}

		/* fill in the screen parameters */

		acauka_scrn.ndescs = numdescs;
		acauka_scrn.sd = acauka_desc;
		acauka_scrn.ms = acauka_menu;

		first_time = 0;
	}

	/* set up for update if modifying user */

	acauka_scrn.scrntype = allow_update ? SCR_FILLIN : SCR_NOCHANGE;

	return traverse(&acauka_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 */

acauka_auth(argv, pwfill)
	char **argv;
	struct prpw_if *pwfill;
{
	static int first_time = 1;

	ENTERFUNC("acauka_auth");
	EXITFUNC("acauka_auth");
	return 0;
}

/*
 * Grab the root directory and directory list from the parameters file
 * Allocate a structure for the screen that is one larger than the
 * current table.  Expand dynamically as the user adds new directories.
 */

static int
acauka_bfill(pwfill)
	struct prpw_if *pwfill;
{
	int ret;
	struct pr_passwd *prpwd = &pwfill->prpw;
	audmask_fillin aubuf, *aufill = &aubuf;
	int i;
	static int first_time = 1;
	int priv_number;

	ENTERFUNC("acauka_bfill");

	ret = XGetUserInfo(gl_user, pwfill);

	if (ret == 0) {
		for (i = 0; i <= SEC_MAX_SPRIV; i++) {
			DefKAState[i] = ISBITSET(prpwd->sfld.fd_sprivs, i);
			DefBPState[i] = ISBITSET(prpwd->sfld.fd_bprivs, i);
			if (prpwd->uflg.fg_sprivs)
				UserKAState[i] =
					ISBITSET(prpwd->ufld.fd_sprivs, i);
			else
				UserKAState[i] = DefKAState[i];
			if (prpwd->uflg.fg_bprivs)
				UserBPState[i] =
					ISBITSET(prpwd->ufld.fd_bprivs, i);
			else
				UserBPState[i] = DefBPState[i];
		}
		if (prpwd->uflg.fg_sprivs)
			UseDefKAs = (char) 0;
		else		
			UseDefKAs = (char) 1;
		if (prpwd->uflg.fg_bprivs)
			UseDefBPs = (char) 0;
		else		
			UseDefBPs = (char) 1;
	}
	EXITFUNC("acauka_bfill");
	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
acauka_bstruct(pwfill, sptemplate)
	struct prpw_if *pwfill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;
	int i;

	ENTERFUNC("acauka_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	memcpy(ScrnUserKAState, UserKAState, AcaukaNAuths);
	memcpy(ScrnUserBPState, UserBPState, AcaukaNAuths);
	ScrnUseDefKAs = UseDefKAs;
	ScrnUseDefBPs = UseDefBPs;

	for (i = 0; i < AcaukaNAuths; i++, sp += 4) {
		sp[0].pointer = &ScrnUserKAState[i];
		sp[1].pointer = &ScrnUseDefKAs;
		sp[2].pointer = &ScrnUserBPState[i];
		sp[3].pointer = &ScrnUseDefBPs;
		sp[0].filled = sp[1].filled =
		sp[2].filled = sp[3].filled = 1;
		sp[0].changed = sp[1].changed =
		sp[2].changed = sp[3].changed = 1;
		sp[0].val_act = sp[1].val_act =
		sp[2].val_act = sp[3].val_act = StateToggle;
	}

	EXITFUNC("acauka_bstruct");
	return 0;
}

/*
 * Routine called when something changed.
 * If the user toggled the Default ON, change all "sets" to default
 * If the user toggled the Default OFF, turn off all defaults.
 * If the user toggled the Set on, turn off all defaults
 * If the user toggled the Set off, turn on all defaults and set to default
 *   values.
 */

static int
StateToggle()
{
	int i;
	int ret = 0;

	/* Check if changed state of Default KAs */
	if (ScrnUseDefKAs != UseDefKAs) {
		/* turned it on -> reset to defaults */
		if (ScrnUseDefKAs && !UseDefKAs) {
			memcpy(UserKAState, DefKAState, AcaukaNAuths);
			memcpy(ScrnUserKAState, DefKAState, AcaukaNAuths);
		}
		UseDefKAs = ScrnUseDefKAs;
		ret = 1;
	}

	/* Check if changed state of Default BPs */
	else if (ScrnUseDefBPs != UseDefBPs) {
		/* turned it on -> reset to defaults */
		if (ScrnUseDefBPs && !UseDefBPs) {
			memcpy(UserBPState, DefBPState, AcaukaNAuths);
			memcpy(ScrnUserBPState, DefBPState, AcaukaNAuths);
		}
		UseDefBPs = ScrnUseDefBPs;
		ret = 1;
	}

	/* check if toggled BPs or KAs in "set" column */

	for (i = 0; i < AcaukaNAuths && !ret; i++) {

		if (ScrnUserKAState[i] != UserKAState[i]) {

			/* user changed kernel auth state */

			/* Not using defaults anymore */
			if (UseDefKAs) {
				UseDefKAs = ScrnUseDefKAs = 0;
				ret = 1;
			}
			UserKAState[i] = ScrnUserKAState[i];
			break;

		}

		if (ScrnUserBPState[i] != UserBPState[i]) {

			/* user changed base priv state */

			/* Not using defaults anymore */
			if (UseDefBPs) {
				UseDefBPs = ScrnUseDefBPs = 0;
				ret = 1;
			}
			UserBPState[i] = ScrnUserBPState[i];
			break;

		}
	}

	return ret;
}


/*
 * action routine.
 * Perform restore of audit session.
 */

static int
acauka_action(pwfill)
	struct prpw_if *pwfill;
{
	int ret;
	int i;
	int all_default;
	struct pr_passwd *prpwd = &pwfill->prpw;


	ENTERFUNC("acauka_action");

	if (UseDefKAs)
		prpwd->uflg.fg_sprivs = 0;
	else {
		prpwd->uflg.fg_sprivs = 1;
		for (i = 0; i < AcaukaNAuths; i++)
			if (UserKAState[i])
				ADDBIT(prpwd->ufld.fd_sprivs, i);
			else
				RMBIT(prpwd->ufld.fd_sprivs, i);
	}
	if (UseDefBPs)
		prpwd->uflg.fg_bprivs = 0;
	else {
		prpwd->uflg.fg_bprivs = 1;
		for (i = 0; i < AcaukaNAuths; i++)
			if (UserBPState[i])
				ADDBIT(prpwd->ufld.fd_bprivs, i);
			else
				RMBIT(prpwd->ufld.fd_bprivs, i);
	}
	ret = XWriteUserInfo(pwfill);

	EXITFUNC("acauka_action");

	return INTERRUPT;
}


static void
auka_free(argv, pwfill, nstructs, pp, dp, sp)
	char **argv;
	struct prpw_if *pwfill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("auka_free");
	EXITFUNC("auka_free");
	return;
}

/*
 * validate the structure -- the logic is in AuditParametersCheck().
 */

static int
acauka_valid(argv, pwfill)
	char **argv;
	struct prpw_if *pwfill;
{
	ENTERFUNC("acauka_valid");
	EXITFUNC("acauka_valid");
	return 0;
}

#define SETUPFUNC	acauka_setup		/* defined by stemplate.c */
#define AUTHFUNC	acauka_auth
#define BUILDFILLIN	acauka_bfill

#define INITFUNC	acauka_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acauka_bstruct

#define ROUTFUNC	acauka_rout		/* defined by stemplate.c */
#define VALIDATE	acauka_valid
#define SCREENACTION	acauka_action

#define FREEFUNC	acauka_free		/* defined by stemplate.c */
#define FREESTRUCT	auka_free

/*
 * Since the number of scrn_structs is variable, here is a local copy of
 * the route function that calls with our local copy of the number of
 * structs
 */

acauka_local_rout(argv)
	char **argv;
{
        return (scrnexit (argv, (char *) FILLIN,
          VALIDATE, SCREENACTION, FIRSTDESC, AcaukaNAuths * 4,
          &PARMTEMPLATE, DESCTEMPLATE, STRUCTTEMPLATE)
        );
}

#include "stemplate.c"

#endif /* SEC_BASE */

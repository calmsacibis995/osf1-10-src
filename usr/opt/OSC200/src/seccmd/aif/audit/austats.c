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
static char	*sccsid = "@(#)$RCSfile: austats.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:38 $";
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
 * Routines to display audit statistics
 * The scrn_desc is built internally in this routine.
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

static int austats_auth();
static int austats_bfill();
static int austats_valid();
static int austats_exit();
static int DisplayButtonValidate();
static int PrintButtonValidate();
static int DeleteButtonValidate();
static int ReportNameToggle();

static int austats_setup(), austats_init(), austats_free();

#define LEFT_COL 5

#define SESSION_LINE	0
#define BLANK_LINE	1
#define BYTES_LINE	2
#define RECS_LINE	3
#define APPL_RECS_LINE	4
#define SYSC_RECS_LINE	5
#define KERN_RECS_LINE	6
#define SYSC_NSEL_LINE	7
#define PCT_SYSC_LINE	8
#define DEV_READS_LINE	9
#define DEV_WRITES_LINE	10
#define BUF_USED_LINE	11
#define BUF_SLEEPS_LINE	12
#define NUMBER_LINES	(BUF_SLEEPS_LINE + 1)

#define FIRSTDESC	0
#define NSCRNSTRUCT	0

uchar austats_title[] = "AUDIT STATISTICS";

Scrn_hdrs austats_hdrs = {
	austats_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SKIP_PARMF(austats_scrn, SCR_TEXT, NULL, NULL, NULL,
	&austats_hdrs,
	austats_setup, austats_init, austats_free);

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	austats_scrn
#define STRUCTTEMPLATE	austats_struct
#define DESCTEMPLATE	austats_desc
#define FILLINSTRUCT	audir_fillin /* not used this screen */
#define FILLIN		austats_fill
#define TRAVERSERW	TRAV_RW

static Scrn_struct	*austats_struct;
static struct audir_fillin au_buf, *austats_fill = &au_buf; /* not used */

static Scrn_desc austats_desc[1];	/* dummy scrn_desc */

static int IsISSO;

/*
 * Table for the screen
 */

static char *StatsTable[NUMBER_LINES];

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

static int
austats_auth(argv, aufill)
	char **argv;
	struct audir_fillin *aufill;
{
	static int first_time = 1;
	int ret = 0;

	ENTERFUNC("austats_auth");
	if (first_time) {
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	if (CheckAuditEnabled() != 1) {
		pop_msg("Audit is not currently enabled.",
		"Audit must be enabled to report statistics.");
		ret = 1;
	}

	EXITFUNC("austats_auth");
	if (IsISSO && !ret)
		return 0;
	else
		return 1;
}

/*
 * Create a new element for the table, padding to the desired length
 * with spaces where the internal field is separated by a '-' or '\t'
 */

#define LINESIZE 50
static int
AssignDesc(buffer,desc)
	char *buffer;
	int desc;
{
	char *cp;
	char *endcp;
	char *new_line;
	int endlen;
	int i, j;
	int blanks;

	/* strip leading tabs */
	cp = buffer + strspn(buffer, "\t");

	/* zap trailing newline */
	cp[strlen(cp) - 1] = '\0';

	/* find last - or tab */
	endcp = strrchr(cp, '-');
	if (endcp == NULL)
		endcp = strrchr(cp, '\t');

	/* allocate a new buffer */
	new_line = Calloc(LINESIZE + 1, 1);
	if (new_line == NULL)
		MemoryError();

	/* copy the part of the string before the tab or - */
	strncpy(new_line, cp, endcp - cp);

	/* compute the length of the string after the tab or -  */
	endlen = strlen(cp) - (endcp - cp) - 1;

	/* compute how many blanks to insert, and put them in */
	blanks = LINESIZE - endlen - strlen(new_line);
	for (i = strlen(new_line), j = 0; j < blanks; i++, j++)
		new_line[i] = ' ';

	/* tack on the ending part */
	strcat(new_line, endcp + 1);

	/* assign the string as the prompt */
	StatsTable[desc] = new_line;
}
	
static int
austats_bfill(aufill)
        struct audir_fillin *aufill;
{
	int ret = 0;
	char *argv[3];
	FILE *fp;
	char buf[80];
	char *cp;
	int i;

	ENTERFUNC("austats_bfill");

	austats_scrn.nbrrows = NUMBER_LINES;

	/* run the audit command to process its output */

	argv[0] = strrchr(AUDIT_COMMAND, '/');
	if (argv[0] != NULL)
		argv[0]++;
	else
		argv[0] = AUDIT_COMMAND;
	argv[1] = "-c";
	argv[2] = NULL;

	fp = popen_all_output(AUDIT_COMMAND, argv);

	if (fp == (FILE *) 0) {
		pop_msg("Cannot run auditcmd -c.",
	"The program is not installed properly or the system is too busy.");
		return 1;
	}

	/* retrieve the first line.  If "Auditing is not enabled", abort */

	cp = fgets(buf, sizeof(buf), fp);

	if (cp == NULL || strcmp(buf, "Auditing is not enabled\n") == 0) {
		pop_msg("Audit is not enabled.",
		"This option must be run when audit is enabled.");
		return 1;
	}

	/* read the next blank line, the title line, and the blank line */
	for (i = 0; i < 3; i++)	{
		cp = fgets(buf, sizeof(buf), fp);
		if (cp == NULL)
			goto bad;
	}

	/* read the audit session line */

	cp = fgets(buf, sizeof buf, fp);
	if (cp == NULL)
		goto bad;

	AssignDesc(cp, SESSION_LINE);

	/* read the blank line after that */

	cp = fgets(buf, sizeof buf, fp);
	if (cp == NULL)
		goto bad;
	AssignDesc(" \t ", BLANK_LINE);

	for (i = 2; i <= BUF_SLEEPS_LINE; i++) {
		cp = fgets(buf, sizeof buf, fp);
		if (cp == NULL)
			goto bad;
		AssignDesc(buf, i);
	}
	pclose_all_output(fp);

	austats_scrn.text = StatsTable;

	EXITFUNC("austats_bfill");

	return 0;
bad:
	for (i = SESSION_LINE; i <= BUF_SLEEPS_LINE; i++)
		if (StatsTable[i]) {
			free(StatsTable[i]);
			StatsTable[i] = NULL;
		}
	pclose_all_output(fp);
	EXITFUNC("austats_bfill");
	return 1;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
austats_bstruct(aufill, sptemplate)
	struct audir_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	ENTERFUNC("austats_bstruct");
	EXITFUNC("austats_bstruct");
	return 0;
}

/*
 * action routine.
 * Depending on action, call the lower level screen traversal function.
 * Put the filename into the "aurc_fill" structure that the lower level
 * screen uses.
 */

static int
austats_action(aufill)
	struct audir_fillin *aufill;
{
	ENTERFUNC("austats_action");
	EXITFUNC("austats_action");
	return 1;
}


static void
stats_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	struct audir_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	int i;

	ENTERFUNC("stats_free");
	for (i = SESSION_LINE; i <= BUF_SLEEPS_LINE; i++)
		if (StatsTable[i]) {
			free(StatsTable[i]);
			StatsTable[i] = NULL;
		}
	EXITFUNC("stats_free");
	return;
}

/*
 * Validate that only one selection file was selected if Print or Display.
 * Validate at least one selection file was selected if Delete
 */

static int
austats_valid(argv, aufill)
	char **argv;
	struct audir_fillin *aufill;
{
	ENTERFUNC("austats_valid");
	EXITFUNC("austats_valid");
	return 0;
}


#define SETUPFUNC	austats_setup	/* defined by stemplate.c */
#define AUTHFUNC	austats_auth
#define BUILDFILLIN	austats_bfill

#define INITFUNC	austats_init		/* defined by stemplate.c */
#define BUILDSTRUCT	austats_bstruct

#define ROUTFUNC	austats_exit		/* defined by stemplate.c */
#define VALIDATE	austats_valid
#define SCREENACTION	austats_action

#define FREEFUNC	austats_free		/* defined by stemplate.c */
#define FREESTRUCT	stats_free

#include "stemplate.c"

#endif /* SEC_BASE */

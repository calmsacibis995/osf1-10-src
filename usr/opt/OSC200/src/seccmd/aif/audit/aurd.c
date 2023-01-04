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
static char	*sccsid = "@(#)$RCSfile: aurd.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:25 $";
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
 * Routines to implement display/print/delete audit reports
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

static int aurd_auth();
static int aurd_bfill();
static int aurd_valid();
static int aurd_exit();
static int DisplayButtonValidate();
static int PrintButtonValidate();
static int DeleteButtonValidate();
static int ReportNameToggle();

/* structures defined in au_scrns.c */

extern Scrn_parms	aurd_scrn;
extern Scrn_desc	aurd_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aurd_scrn
#define STRUCTTEMPLATE	aurd_struct
#define DESCTEMPLATE	aurd_desc
#define FILLINSTRUCT	audir_fillin /* not used this screen */
#define FILLIN		aurd_fill
#define TRAVERSERW	TRAV_RW

/*
 * buffers for file parameters
 */

static char **ReportTable;
static int Nreports;
static char *ReportState;
static int SelectedReport;
static char Display, Print, Delete;

#define RD_DISPLAY_BUTTON_DESC		0
#define RD_PRINT_BUTTON_DESC		1
#define RD_DELETE_BUTTON_DESC		2
#define RS_REPORT_TITLE_DESC		3
#define RS_REPORT_DESC			4

#define FIRSTDESC	RD_DISPLAY_BUTTON_DESC

#define RD_DISPLAY_BUTTON_STRUCT	0
#define RD_PRINT_BUTTON_STRUCT		1
#define RD_DELETE_BUTTON_STRUCT		2
#define RD_REPORT_STRUCT		3
#define NSCRNSTRUCT			4

static Scrn_struct	*aurd_struct;
static struct audir_fillin au_buf, *aurd_fill = &au_buf; /* not used */

static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

static int
aurd_auth(argv, aufill)
	char **argv;
	struct audir_fillin *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aurd_auth");
	if (first_time) {
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aurd_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * Build the report file table
 */

static int
aurd_bfill(aufill)
        struct audir_fillin *aufill;
{
	ENTERFUNC("aurd_bfill");
	EXITFUNC("aurd_bfill");
	return 0;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
aurd_bstruct(aufill, sptemplate)
	struct audir_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;
	int ret;

	ENTERFUNC("aurd_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	if (ReportTable != (char **) 0) {
		ReportNamesFree(ReportTable, Nreports);
		ReportTable = (char **) 0;
		free(ReportState);
	}

	/* retrieve reports each time through the display loop */

	ret = ReportNamesGet(&Nreports, &ReportTable);

	if (ret == 0) {
		ReportState = Calloc(Nreports, 1);
		if (ReportState == NULL) {
			ReportNamesFree(ReportTable, Nreports);
			ret = 1;
		}
		ReportState[0] = 1;
		SelectedReport = 0;
		Display = 1;
		Delete = 0;
		Print = 0;
		
		sp[RD_DISPLAY_BUTTON_STRUCT].pointer = &Display;
		sp[RD_DISPLAY_BUTTON_STRUCT].filled = 1;
		sp[RD_DISPLAY_BUTTON_STRUCT].val_act = DisplayButtonValidate;
		sp[RD_DISPLAY_BUTTON_STRUCT].changed = 1;

		sp[RD_PRINT_BUTTON_STRUCT].pointer = &Print;
		sp[RD_PRINT_BUTTON_STRUCT].filled = 1;
		sp[RD_PRINT_BUTTON_STRUCT].val_act = PrintButtonValidate;

		sp[RD_DELETE_BUTTON_STRUCT].pointer = &Delete;
		sp[RD_DELETE_BUTTON_STRUCT].filled = 1;
		sp[RD_DELETE_BUTTON_STRUCT].val_act = DeleteButtonValidate;

		sp[RD_REPORT_STRUCT].pointer = (char *) ReportTable;
		sp[RD_REPORT_STRUCT].filled = Nreports;
		sp[RD_REPORT_STRUCT].state = ReportState;
		sp[RD_REPORT_STRUCT].val_act = ReportNameToggle;
	}

	EXITFUNC("aurd_bstruct");
	return ret;
}

/*
 * Validation routine selected when the user toggles the Display button
 * If it's off, turn it back on
 * If it's on, turn the others back off
 * return 1 if the screen needs to be updated
 */

static int
DisplayButtonValidate()
{
	int i;

	ENTERFUNC("CreateButtonValidate");
	Display = 1;

	/* 
	 * If had selected delete, need to drop back to a single selection
	 */

	if (Delete) {
		int first = -1;

		for (i = 0; i < Nreports; i++)
			if (ReportState[i]) {
				if (first == -1) {
					SelectedReport = i;
					first = i;
				}
				else
					ReportState[i] = 0;
			}

		/* if none selected, choose one arbitrarily as default */

		if (first == -1) {
			ReportState[0] = 1;
			SelectedReport = 0;
		}
	}
	Print = 0;
	Delete = 0;
	EXITFUNC("CreateButtonValidate");
	return 1;
}

/*
 * Validation routine selected when the user toggles the Print button
 * If it's off, turn it back on
 * If it's on, turn the others back off
 * return 1 if the screen needs to be updated
 */

static int
PrintButtonValidate()
{
	int i;

	ENTERFUNC("ModifyButtonValidate");
	Print = 1;

	/* 
	 * If had selected delete, need to drop back to a single selection
	 */

	if (Delete) {
		int first = -1;

		for (i = 0; i < Nreports; i++)
			if (ReportState[i]) {
				if (first == -1) {
					SelectedReport = i;
					first = i;
				}
				else
					ReportState[i] = 0;
			}

		/* if none selected, choose one arbitrarily as default */

		if (first == -1) {
			ReportState[0] = 1;
			SelectedReport = 0;
		}
	}
					
	Display = 0;
	Delete = 0;
	EXITFUNC("ModifyButtonValidate");
	return 1;
}

/*
 * Validation routine selected when the user toggles the delete button
 * If it's off, turn it back on
 * If it's on, turn the others back off
 * return 1 if the screen needs to be updated
 */

static int
DeleteButtonValidate()
{
	ENTERFUNC("DeleteButtonValidate");
	Delete = 1;
	Display = 0;
	Print = 0;
	EXITFUNC("DeleteButtonValidate");
	return 1;
}

/*
 * action routine.
 * Depending on action, call the lower level screen traversal function.
 * Put the filename into the "aurc_fill" structure that the lower level
 * screen uses.
 */

static int
aurd_action(aufill)
	struct audir_fillin *aufill;
{
	int i;
	int ret;
	char path_buf[PATH_MAX];

	ENTERFUNC("aurd_action");

	if (Display) {
		for (i = 0; i < Nreports; i++)
			if (ReportState[i])
				break;
		sprintf(path_buf, "%s%s", REDUCE_REPORT_PATH, ReportTable[i]);
		ret = DisplayFile(path_buf);
		headers(&aurd_scrn);
	}

	else if (Print) {
		for (i = 0; i < Nreports; i++)
			if (ReportState[i])
				break;
		sprintf(path_buf, "%s%s", REDUCE_REPORT_PATH, ReportTable[i]);
		ret = PrintFile(path_buf);
		headers(&aurd_scrn);
	}

	else if (Delete) {
		for (i = 0; i < Nreports; i++) {
			if (ReportState[i])
				ret = ReportFileDelete(ReportTable[i]);
			if (ret) {
				char buf[80];

				sprintf(buf,
				  "Unable to delete report file %s.",
				  ReportTable[i]);
				pop_msg(buf,
					"Please re-select and try again.");
				ret = 0;
			}
		}
	}

	EXITFUNC("aurd_action");
	return 1;
}


static void
rd_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	struct audir_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("rd_free");
	ReportNamesFree(ReportTable, Nreports);
	ReportTable = (char **) 0;
	EXITFUNC("rd_free");
	return;
}

/*
 * Validate that only one selection file was selected if Print or Display.
 * Validate at least one selection file was selected if Delete
 */

static int
aurd_valid(argv, aufill)
	char **argv;
	struct audir_fillin *aufill;
{
	int ret = 0;
	int i;
	int count;

	ENTERFUNC("aurd_valid");

	/* count the number of files selected */

	count = 0;
	for (i = 0; i < Nreports; i++)
		if (ReportState[i])
			count++;

	if (Display || Print) {

		if (count != 1) {
			pop_msg(
			  "Please choose one report file to display/print",
			  "You must choose and may not choose more than one.");
			ret = 1;
		}


	} else if (Delete) {

		if (count == 0) {
			pop_msg(
		"Please specify at least one report to delete.",
		"Otherwise, there's nothing to do for this request!");
			ret = 1;
		}

	}

	EXITFUNC("aurd_valid");
	return ret;
}

/*
 * Called when a report file name is toggled.
 * If deleting, allow more than one selection.
 * If displaying or printing, allow only one selection.
 */

static int
ReportNameToggle()
{
	int i;
	int ret = 0;

	/* allow usual behavior if Delete */

	if (Delete)
		return 0;

	for (i = 0; i < Nreports; i++) {

		/* selected another one -- turn old one off */

		if (ReportState[i] && i != SelectedReport) {
			ReportState[SelectedReport] = 0;
			SelectedReport = i;
			ret = 1;
			break;
		}

		/* selected the same one -- leave it on */

		else if (!ReportState[i] && i == SelectedReport) {
			ReportState[i] = 1;
			ret = 1;
			break;
		}
	}
	return ret;
}

#define SETUPFUNC	aurd_setup	/* defined by stemplate.c */
#define AUTHFUNC	aurd_auth
#define BUILDFILLIN	aurd_bfill

#define INITFUNC	aurd_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aurd_bstruct

#define ROUTFUNC	aurd_exit		/* defined by stemplate.c */
#define VALIDATE	aurd_valid
#define SCREENACTION	aurd_action

#define FREEFUNC	aurd_free		/* defined by stemplate.c */
#define FREESTRUCT	rd_free

#include "stemplate.c"

#endif /* SEC_BASE */

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
static char	*sccsid = "@(#)$RCSfile: aurs.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:31 $";
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
 * Routines to implement create/modify/delete audit report selection files
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

static int aurs_auth();
static int aurs_bfill();
static int aurs_valid();
static int aurs_exit();
static int CreateButtonValidate();
static int ModifyButtonValidate();
static int DeleteButtonValidate();
static int SelFileToggle();

/* structures defined in au_scrns.c */

extern Scrn_parms	aurs_scrn;
extern Scrn_desc	aurs_desc[];
extern Scrn_parms	aurc_scrn;

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aurs_scrn
#define STRUCTTEMPLATE	aurs_struct
#define DESCTEMPLATE	aurs_desc
#define FILLINSTRUCT	audir_fillin /* not used this screen */
#define FILLIN		aurs_fill
#define TRAVERSERW	TRAV_RW

/*
 * buffers for file parameters
 */

static char **sel_files;
static int nsel_files;
static char *sel_state;
static int file_selected;

char SelFileCreate, SelFileModify, SelFileDelete;
char SelFileName[ENTSIZ + 1];

static char FileToCreate[ENTSIZ + 1];

#define RS_CREATE_BUTTON_DESC	0
#define RS_MODIFY_BUTTON_DESC	1
#define RS_DELETE_BUTTON_DESC	2
#define RS_CREATE_TITLE_DESC	3
#define RS_CREATE_FILE_DESC	4
#define RS_SELECT_TITLE_DESC	5
#define RS_SELECT_FILES_DESC	6

#define FIRSTDESC		RS_CREATE_BUTTON_DESC

#define RS_CREATE_BUTTON_STRUCT	0
#define RS_MODIFY_BUTTON_STRUCT	1
#define RS_DELETE_BUTTON_STRUCT	2
#define RS_CREATE_FILE_STRUCT	3
#define RS_SELECT_FILE_STRUCT	4
#define NSCRNSTRUCT		5

static Scrn_struct	*aurs_struct;
static struct audir_fillin au_buf, *aurs_fill = &au_buf; /* not used */

/*
 * This is defined here for communication with SelectionDeleteFile()
 */

static struct audit_selection_struct aurc_buf, *aurc_fill = &aurc_buf;

static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

static int
aurs_auth(argv, aufill)
	char **argv;
	struct audir_fillin *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aurs_auth");
	if (first_time) {
		SelectionFileStart();
		ASelectionOpen();
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aurs_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * Set up initial values of prompt fields
 */

static int
aurs_bfill(aufill)
        struct audir_fillin *aufill;
{
	ENTERFUNC("aurs_bfill");

	SelFileModify = SelFileDelete = (char) 0;
	SelFileCreate = 1;

	memset(FileToCreate, '\0', sizeof(FileToCreate));
	aurs_desc[RS_CREATE_FILE_DESC].inout = FLD_BOTH;
	file_selected = -1;

	EXITFUNC("aurs_bfill");

	return 0;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
aurs_bstruct(aufill, sptemplate)
	struct audir_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	int ret;
	struct scrn_struct *sp;

	ENTERFUNC("aurs_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	/* delete old selection file table if there's one there */

	if (sel_files != (char **) 0) {
		free_cw_table(sel_files);
		sel_files = (char **) 0;
	}
	if (sel_state) {
		free(sel_state);
		sel_state = NULL;
	}

	/* get a current copy of the selection files */

	ret = SelectionFileGet(&nsel_files, &sel_files);

	if (ret == 0) {
		/* allocate a one-element table if no files */

		if (nsel_files == 0) {
			sel_files = alloc_cw_table(1, FILEWIDTH+1);
			if (sel_files == (char **) 0)
				MemoryError();
			nsel_files = 1;
		}

		sel_state = Calloc(nsel_files, 1);
		if (sel_state == NULL) {
			free_cw_table(sel_files);
			ret = 1;
		}
	}

	sp[RS_CREATE_BUTTON_STRUCT].pointer = &SelFileCreate;
	sp[RS_CREATE_BUTTON_STRUCT].filled = 1;
	sp[RS_CREATE_BUTTON_STRUCT].val_act = CreateButtonValidate;
	sp[RS_CREATE_BUTTON_STRUCT].changed = 1;

	sp[RS_MODIFY_BUTTON_STRUCT].pointer = &SelFileModify;
	sp[RS_MODIFY_BUTTON_STRUCT].filled = 1;
	sp[RS_MODIFY_BUTTON_STRUCT].val_act = ModifyButtonValidate;

	sp[RS_DELETE_BUTTON_STRUCT].pointer = &SelFileDelete;
	sp[RS_DELETE_BUTTON_STRUCT].filled = 1;
	sp[RS_DELETE_BUTTON_STRUCT].val_act = DeleteButtonValidate;

	sp[RS_CREATE_FILE_STRUCT].pointer = FileToCreate;
	sp[RS_CREATE_FILE_STRUCT].filled = 1;
	sp[RS_CREATE_FILE_STRUCT].changed = 1;

	sp[RS_SELECT_FILE_STRUCT].pointer = (char *) sel_files;
	sp[RS_SELECT_FILE_STRUCT].filled = nsel_files;
	sp[RS_SELECT_FILE_STRUCT].state = sel_state;
	sp[RS_SELECT_FILE_STRUCT].changed = 1;
	sp[RS_SELECT_FILE_STRUCT].val_act = SelFileToggle;

	EXITFUNC("aurs_bstruct");
	return ret;
}

/*
 * Validation routine selected when the user toggles the create button
 * If it's off, turn it back on
 * If it's on, turn the others back off
 * return 1 if the screen needs to be updated
 */

static int
CreateButtonValidate()
{
	ENTERFUNC("CreateButtonValidate");
	SelFileCreate = 1;
	SelFileModify = 0;
	SelFileDelete = 0;
	aurs_desc[RS_CREATE_FILE_DESC].inout = FLD_BOTH;
	memset(sel_state, '\0', nsel_files);
	file_selected = -1;
	EXITFUNC("CreateButtonValidate");
	return 1;
}

/*
 * Validation routine selected when the user toggles the modify button
 * If it's off, turn it back on
 * If it's on, turn the others back off
 * return 1 if the screen needs to be updated
 */

static int
ModifyButtonValidate()
{
	ENTERFUNC("ModifyButtonValidate");
	SelFileModify = 1;
	SelFileCreate = 0;
	SelFileDelete = 0;
	memset(FileToCreate, '\0', sizeof FileToCreate);
	aurs_desc[RS_CREATE_FILE_DESC].inout = FLD_OUTPUT;
	if (file_selected == -1) {
		file_selected = 0;
		sel_state[0] = 1;
		memset(&sel_state[1], '\0', nsel_files - 1);
	}
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
	SelFileDelete = 1;
	SelFileCreate = 0;
	SelFileModify = 0;
	memset(FileToCreate, '\0', sizeof FileToCreate);
	aurs_desc[RS_CREATE_FILE_DESC].inout = FLD_OUTPUT;
	file_selected = -1;
	memset(sel_state, '\0', nsel_files);
	EXITFUNC("DeleteButtonValidate");
	return 1;
}

/*
 * Validation routine when toggling a selection file.
 * Modify only allows selecting one.
 * Delete allows selecting multiple
 * If creating, can't select any
 */

SelFileToggle()
{
	int ret = 0;
	int i;

	ENTERFUNC("SelFileToggle");

	if (SelFileDelete) {

		/* nothing to do; toggle to your heart's content */

	} else if (SelFileCreate) {
		
		/* don't allow selecting files in here */

		beep();
		memset(sel_state, '\0', nsel_files);
		ret = 1;

	} else {		/* SelFileModify */

		/* logic for only selecting one file */

		for (i = 0; i < nsel_files; i++)

			if (sel_state[i] && i != file_selected) {

				/* selected another one */

				sel_state[file_selected] = 0;
				file_selected = i;
				ret = 1;
				break;

			} else if (!sel_state[i] && i == file_selected) {

				/* turned of the current one */

				sel_state[i] = 1;
				ret = 1;
				break;
			}
	}

	EXITFUNC("SelFileToggle");

	return ret;
}

/*
 * action routine.
 * Depending on action, call the lower level screen traversal function.
 * Put the filename into the "aurc_fill" structure that the lower level
 * screen uses.
 */

static int
aurs_action(aufill)
	struct audir_fillin *aufill;
{
	int i;
	int ret;

	ENTERFUNC("aurs_action");

	if (SelFileCreate) {
		strcpy(SelFileName, FileToCreate);
		ret = traverse(&aurc_scrn, RS_CREATE_BUTTON_DESC);
		memset(FileToCreate, '\0', sizeof FileToCreate);
	}

	else if (SelFileModify) {

		for (i = 0; i < nsel_files; i++)
			if (sel_state[i]) {
				strcpy(SelFileName, sel_files[i]);
				ret = traverse(&aurc_scrn,
						RS_CREATE_BUTTON_DESC);
			}
	}

	else if (SelFileDelete) {
		for (i = 0; i < nsel_files; i++)
			if (sel_state[i]) {
				strcpy(aurc_fill->filename, sel_files[i]);
				SelectionDeleteFile(aurc_fill);
			}
		ret = 1;
	}

	EXITFUNC("aurs_action");
	return ret;
}


static void
rs_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	struct audir_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("rs_free");
	if (sel_files != (char **) 0) {
		free_cw_table(sel_files);
		sel_files = (char **) 0;
	}
	if (sel_state != NULL) {
		free(sel_state);
		sel_state = NULL;
	}
	EXITFUNC("rs_free");
	return;
}

/*
 * Validate that only one selection file was selected if Modify.
 * Validate at least one selection file was selected if Delete
 * Validate that no selection files were selected if Create.
 * Validate that filename is only filled in if Create, and that it
 * doesn't exist (is not in the scrolled list).
 */

static int
aurs_valid(argv, aufill)
	char **argv;
	struct audir_fillin *aufill;
{
	int ret = 0;
	int i;
	int count;

	ENTERFUNC("aurs_valid");

	/* count the number of files selected */

	count = 0;
	for (i = 0; i < nsel_files; i++)
		if (sel_state[i])
			count++;

	if (SelFileModify) {

		if (count != 1) {
			pop_msg(
			  "Please choose one selection file to modify",
			  "You must choose and may not choose more than one.");
			return 1;
		}


	} else if (SelFileCreate) {

		if (count != 0) {
			pop_msg(
		"Please do not select any files if creating a new one.",
			  "De-select the ones you selected and re-execute.");
			return 1;
		}
		if (FileToCreate[0] == '\0') {
			pop_msg(
			"To create a new file, you must specify a name.",
			"Please fill in the 'Create Selection File:' field.");
			return 1;
		}
		for (i = 0; i < nsel_files; i++) {
			if (strcmp(sel_files[i], FileToCreate) == 0) {
				pop_msg(
			"There is already a file with that name."
			"Please choose one that is unique.");
				return 1;
			}
		}
		if (strchr(FileToCreate, '/')) {
			pop_msg(
			  "The selection filename may not include a '/'.",
			  "Please re-enter the file name.");
			return 1;
		}
		if (!strcmp(FileToCreate, ".") ||
		    !strcmp(FileToCreate, "..")) {
			pop_msg(
			  "You may not specify '.' or '..' as a file name.",
			  "Please re-enter the file name.");
			return 1;
		}

	} else if (SelFileDelete) {

		if (count == 0) {
			pop_msg(
		"Please specify at least one report selection file to delete.",
		"Otherwise, there's nothing to do for this request!");
			return 1;
		}

	}

	if (SelFileModify || SelFileDelete) {
		if (FileToCreate[0] != '\0') {
			pop_msg(
			  "Do not specify a file to create when selecting"
			  "Modify or Delete.  Please clear that field.");
			return 1;
		}
	}
	EXITFUNC("aurs_valid");
	return ret;
}

#define SETUPFUNC	aurs_setup		/* defined by stemplate.c */
#define AUTHFUNC	aurs_auth
#define BUILDFILLIN	aurs_bfill

#define INITFUNC	aurs_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aurs_bstruct

#define ROUTFUNC	aurs_exit		/* defined by stemplate.c */
#define VALIDATE	aurs_valid
#define SCREENACTION	aurs_action

#define FREEFUNC	aurs_free		/* defined by stemplate.c */
#define FREESTRUCT	rs_free

#include "stemplate.c"

#endif /* SEC_BASE */

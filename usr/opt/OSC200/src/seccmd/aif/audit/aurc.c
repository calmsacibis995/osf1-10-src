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
static char	*sccsid = "@(#)$RCSfile: aurc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:21 $";
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
 * Routines to implement create/update audit report selection file
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

/*
 * These are defined here as communication with the upper level screen
 */

extern char SelFileName[];
extern char SelFileCreate;

static struct audit_selection_struct aurc_buf;
struct audit_selection_struct *aurc_fill = &aurc_buf;

/*
 * The rest of the file implements the complex screen for the creation
 * or update of a report selection file.  Since it is called from the
 * screen implemented in this file, rather than being called from the
 * main screen driver directly, and it needs to share data with the
 * upper level screen, it is included in the same file.  Note that
 * stemplate.c undefines all of the earlier definitions and therefore
 * can be included in the same file twice.
 */

/* static routine definitions */

static int aurc_auth();
static int aurc_bfill();
static int aurc_valid();
static int aurc_exit();
static void AllocTables();
static int FileTableExpand();
static int SubjMinSLValAct();
static int SubjMaxSLValAct();
static int ObjMinSLValAct();
static int ObjMaxSLValAct();

/* structures defined in au_scrns.c */

extern Scrn_parms	aurc_scrn;
extern Scrn_parms	aurs_scrn;
extern Scrn_desc	aurc_desc[];
extern uchar aurc_create_title[];
extern uchar aurc_modify_title[];

#if SEC_MAC

/* define macro to raise and lower privileges to query MAC daemon */

#if SEC_ILB
#define RAISE_PRIVVEC	privvec(SEC_ALLOWMACACCESS, SEC_ILNOFLOAT, -1)
#else
#define RAISE_PRIVVEC	privvec(SEC_ALLOWMACACCESS, -1)
#endif

#define RAISE_PRIVS(old_privs)	forceprivs(RAISE_PRIVVEC, old_privs)

#endif

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aurc_scrn
#define STRUCTTEMPLATE	aurc_struct
#define DESCTEMPLATE	aurc_desc
#define FILLINSTRUCT	audit_selection_struct
#define FILLIN		aurc_fill
#define TRAVERSERW	TRAV_RW

#define AURC_FILE_TITLE_DESC	0
#define AURC_FILE_DESC		1
#define AURC_STIME_TITLE_DESC	2
#define AURC_STIME_DD_DESC	3
#define AURC_STIME_MMM_DESC	4
#define AURC_STIME_YY_DESC	5
#define AURC_STIME_HH_DESC	6
#define AURC_STIME_COLON_DESC	7
#define AURC_STIME_MM_DESC	8
#define AURC_ETIME_TITLE_DESC	9
#define AURC_ETIME_DD_DESC	10
#define AURC_ETIME_MMM_DESC	11
#define AURC_ETIME_YY_DESC	12
#define AURC_ETIME_HH_DESC	13
#define AURC_ETIME_COLON_DESC	14
#define AURC_ETIME_MM_DESC	15
#define AURC_STIME_UNDER_DESC	16
#define AURC_ETIME_UNDER_DESC	17
#define AURC_EVENT_TITLE_DESC	18
#define AURC_FILETAB_TITLE_DESC	19
#define AURC_EVENT_DESC		20
#define AURC_FILETAB_DESC	21
#define AURC_USER_TITLE_DESC	22
#define AURC_GROUP_TITLE_DESC	23
#define AURC_SL_TITLE_DESC	24
#define AURC_USER_DESC		25
#define AURC_GROUP_DESC		26
#define AURC_SUBJ_TITLE_DESC	27
#define AURC_SUBJ_MINSL_DESC	28
#define AURC_SUBJ_MAXSL_DESC	29
#define AURC_OBJ_TITLE_DESC	30
#define AURC_OBJ_MINSL_DESC	31
#define AURC_OBJ_MAXSL_DESC	32

#define FIRSTDESC		AURC_STIME_DD_DESC

#define AURC_FILE_STRUCT	0
#define AURC_STIME_DD_STRUCT	1
#define AURC_STIME_MMM_STRUCT	2
#define AURC_STIME_YY_STRUCT	3
#define AURC_STIME_HH_STRUCT	4
#define AURC_STIME_MM_STRUCT	5
#define AURC_ETIME_DD_STRUCT	6
#define AURC_ETIME_MMM_STRUCT	7
#define AURC_ETIME_YY_STRUCT	8
#define AURC_ETIME_HH_STRUCT	9
#define AURC_ETIME_MM_STRUCT	10
#define AURC_EVENT_STRUCT	11
#define AURC_FILETAB_STRUCT	12
#define AURC_USER_STRUCT	13
#define AURC_GROUP_STRUCT	14
#define AURC_SUBJ_MINSL_STRUCT	15
#define AURC_SUBJ_MAXSL_STRUCT	16
#define AURC_OBJ_MINSL_STRUCT	17
#define AURC_OBJ_MAXSL_STRUCT	18

#define NSCRNSTRUCT		19

static Scrn_struct	*aurc_struct;

/* Additional items on the screen not in the structure */

static char SubjMinSLFlag;
static char SubjMaxSLFlag;
static char ObjMinSLFlag;
static char ObjMaxSLFlag;
static char *UserState;
static char *GroupState;
static char *EventState;
static char **EventTable;
static char **UserTable;
static char **GroupTable;
static char **FileTable;
static int Nusers;
static int Ngroups;
static int Nevents;
static int Nfiles;

/*
 * No need for an authorization function; user authorization has been checked.
 */

static int
aurc_auth(argv, aufill)
	char **argv;
	struct audit_selection_struct *aufill;
{
	ENTERFUNC("aurc_auth");
	EXITFUNC("aurc_auth");
	return 0;
}

/*
 * allocate tables for the screen.  Call MemoryError() on failure.
 */

static void
AllocTables()
{
	int i;

	GetAllUsers(&Nusers, &UserTable);
	UserState = Calloc(Nusers, 1);
	if (UserState == NULL)
		MemoryError();

	GetAllGroups(&Ngroups, &GroupTable);
	GroupState = Calloc(Ngroups, 1);
	if (GroupState == NULL)
		MemoryError();

	AuditAllocEventMaskTable(&Nevents, &EventTable);
	EventState = Calloc(Nevents, 1);
	if (EventState == NULL)
		MemoryError();

	/*
	 * allocate a file table that's one larger than the number of files
	 * to allow for dynamic expansion
	 */

	Nfiles = aurc_fill->nfiles + 1;
	FileTable = alloc_cw_table(Nfiles, FILEWIDTH + 1);
	if (FileTable == (char **) 0)
		MemoryError();
	for (i = 0; i < aurc_fill->nfiles; i++)
		strcpy(FileTable[i], aurc_fill->files[i]);

}

/*
 * The fillin structure needs to initialize from an existing selection file
 * or initialize all fields to create a new one.
 */

static int
aurc_bfill(aufill)
	struct audit_selection_struct *aufill;
{
	int i, j;

	ENTERFUNC("aurc_bfill");

	/* copy the file name from the higher level routine */
	strcpy(aufill->filename, SelFileName);

	if (SelFileCreate) {

		aurc_scrn.sh->title = aurc_create_title;

		if (SelectionCreateFill(aufill))
			return 1;

		/* Indicate no sensitivity levels set yet */

		SubjMinSLFlag = SubjMaxSLFlag =
		 ObjMinSLFlag =  ObjMaxSLFlag = 0;

		/* allocate user and group lists and state tables */

		AllocTables();

	} else { /* update */

		aurc_scrn.sh->title = aurc_modify_title;

		if (SelectionDisplayFill(aufill))
			return 1;

		/* allocate user and group lists and state tables */

		AllocTables();

		/* update the user and group lists to reflect those selected */

		for (i = 0; i < aufill->nusers; i++)
			for (j = 0; j < Nusers; j++)
				if (!strcmp(aufill->users[i], UserTable[j]))
					UserState[j] = 1;

		for (i = 0; i < aufill->ngroups; i++)
			for (j = 0; j < Ngroups; j++)
				if (!strcmp(aufill->groups[i], GroupTable[j]))
					GroupState[j] = 1;

		/* update the events selected to reflect those selected */

		for (i = 0; i < AUDIT_MAX_EVENT; i++)
			if (aufill->events[i][0] == YESCHAR)
				EventState[i] = 1;
	}
	EXITFUNC("aurc_bfill");
	return 0;
}

/*
 * fill in the structure pointers for this massive screen.
 */

static int
aurc_bstruct(aufill, sptemplate)
	struct audit_selection_struct *aufill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("aurc_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

#if SEC_MAC
	/* set sensitivity level toggles */

	if (aufill->as.slevel_min != (tag_t) 0)
		SubjMinSLFlag = 1;
	else
		SubjMinSLFlag = 0;

	if (aufill->as.slevel_max != (tag_t) 0)
		SubjMaxSLFlag = 1;
	else
		SubjMaxSLFlag = 0;

	if (aufill->as.olevel_min != (tag_t) 0)
		ObjMinSLFlag = 1;
	else
		ObjMinSLFlag = 0;

	if (aufill->as.olevel_max != (tag_t) 0)
		ObjMaxSLFlag = 1;
	else
		ObjMaxSLFlag = 0;
#endif
		
	sp[AURC_FILE_STRUCT].pointer = aufill->filename;
	sp[AURC_FILE_STRUCT].filled = 1;

	sp[AURC_STIME_DD_STRUCT].pointer = aufill->s_day;
	sp[AURC_STIME_DD_STRUCT].filled = 1;
	sp[AURC_STIME_MMM_STRUCT].pointer = aufill->s_month;
	sp[AURC_STIME_MMM_STRUCT].filled = 1;
	sp[AURC_STIME_YY_STRUCT].pointer = aufill->s_year;
	sp[AURC_STIME_YY_STRUCT].filled = 1;
	sp[AURC_STIME_HH_STRUCT].pointer = aufill->s_hour;
	sp[AURC_STIME_HH_STRUCT].filled = 1;
	sp[AURC_STIME_MM_STRUCT].pointer = aufill->s_min;
	sp[AURC_STIME_MM_STRUCT].filled = 1;

	sp[AURC_ETIME_DD_STRUCT].pointer = aufill->e_day;
	sp[AURC_ETIME_DD_STRUCT].filled = 1;
	sp[AURC_ETIME_MMM_STRUCT].pointer = aufill->e_month;
	sp[AURC_ETIME_MMM_STRUCT].filled = 1;
	sp[AURC_ETIME_YY_STRUCT].pointer = aufill->e_year;
	sp[AURC_ETIME_YY_STRUCT].filled = 1;
	sp[AURC_ETIME_HH_STRUCT].pointer = aufill->e_hour;
	sp[AURC_ETIME_HH_STRUCT].filled = 1;
	sp[AURC_ETIME_MM_STRUCT].pointer = aufill->e_min;
	sp[AURC_ETIME_MM_STRUCT].filled = 1;

	sp[AURC_EVENT_STRUCT].pointer = (char *) EventTable;
	sp[AURC_EVENT_STRUCT].state = EventState;
	sp[AURC_EVENT_STRUCT].filled = Nevents;

	sp[AURC_FILETAB_STRUCT].pointer = (char *) FileTable;
	sp[AURC_FILETAB_STRUCT].filled = Nfiles;
	sp[AURC_FILETAB_STRUCT].val_act = FileTableExpand;

	sp[AURC_USER_STRUCT].pointer = (char *) UserTable;
	sp[AURC_USER_STRUCT].filled = Nusers;
	sp[AURC_USER_STRUCT].state = UserState;

	sp[AURC_GROUP_STRUCT].pointer = (char *) GroupTable;
	sp[AURC_GROUP_STRUCT].filled = Ngroups;
	sp[AURC_GROUP_STRUCT].state = GroupState;

#if SEC_MAC
	sp[AURC_SUBJ_MINSL_STRUCT].pointer = &SubjMinSLFlag;
	sp[AURC_SUBJ_MINSL_STRUCT].filled = 1;
	sp[AURC_SUBJ_MINSL_STRUCT].val_act = SubjMinSLValAct;
	sp[AURC_SUBJ_MAXSL_STRUCT].pointer = &SubjMaxSLFlag;
	sp[AURC_SUBJ_MAXSL_STRUCT].filled = 1;
	sp[AURC_SUBJ_MAXSL_STRUCT].val_act = SubjMaxSLValAct;
	sp[AURC_OBJ_MINSL_STRUCT].pointer = &ObjMinSLFlag;
	sp[AURC_OBJ_MINSL_STRUCT].filled = 1;
	sp[AURC_OBJ_MINSL_STRUCT].val_act = ObjMinSLValAct;
	sp[AURC_OBJ_MAXSL_STRUCT].pointer = &ObjMaxSLFlag;
	sp[AURC_OBJ_MAXSL_STRUCT].filled = 1;
	sp[AURC_OBJ_MAXSL_STRUCT].val_act = ObjMaxSLValAct;
#endif

	EXITFUNC("aurc_bstruct");
	return 0;
}

#if SEC_MAC
/*
 * validation routine for the subject min SL.
 * if the user turns it off, clear the tag.
 */

static int
SubjMinSLValAct()
{
	privvec_t s;

	if (SubjMinSLFlag) {
		RAISE_PRIVS(s);
		mand_ir_to_tag(aurc_fill->slevel_min,
					&aurc_fill->as.slevel_min);
		seteffprivs(s, NULL);
	}
	else
		aurc_fill->as.slevel_min = (tag_t) 0;
	return 0;
}

/*
 * validation routine for the subject max SL.
 * if the user turns it off, clear the tag.
 */

static int
SubjMaxSLValAct()
{
	privvec_t s;

	if (SubjMaxSLFlag) {
		RAISE_PRIVS(s);
		mand_ir_to_tag(aurc_fill->slevel_max,
					&aurc_fill->as.slevel_max);
		seteffprivs(s, NULL);
	}
	else
		aurc_fill->as.slevel_max = (tag_t) 0;
	return 0;
}

/*
 * validation routine for the object min SL.
 * if the user turns it off, clear the tag.
 */

static int
ObjMinSLValAct()
{
	privvec_t s;

	if (ObjMinSLFlag) {
		RAISE_PRIVS(s);
		mand_ir_to_tag(aurc_fill->olevel_min,
					&aurc_fill->as.olevel_min);
		seteffprivs(s, NULL);
	}
	else
		aurc_fill->as.olevel_min = (tag_t) 0;
	return 0;
}

/*
 * validation routine for the object max SL.
 * if the user turns it off, clear the tag.
 */

static int
ObjMaxSLValAct()
{
	privvec_t s;

	if (ObjMaxSLFlag) {
		RAISE_PRIVS(s);
		mand_ir_to_tag(aurc_fill->olevel_max,
					&aurc_fill->as.olevel_max);
		seteffprivs(s, NULL);
	}
	else
		aurc_fill->as.olevel_max = (tag_t) 0;
	return 0;
}
#endif /* SEC_MAC */

/*
 * Validate routine.
 * Make sure that there are no spaces in filenames and that the dates parse
 */

static int
aurc_valid(argv, aufill)
	char **argv;
	struct audit_selection_struct *aufill;
{
	int ret = 0;

	ENTERFUNC("aurc_valid");

	if (SelectionEditTime(aufill->s_hour, aufill->s_min, aufill->s_day,
			      aufill->s_month, aufill->s_year))  {
		ret = 1;
		goto out;
	}

	if (SelectionEditTime(aufill->e_hour, aufill->e_min, aufill->e_day,
			      aufill->e_month, aufill->e_year)) {
		ret = 1;
		goto out;
	}

	if (SelectionEditFiles(aufill)) {
		ret = 1;
		goto out;
	}

#if SEC_MAC
	/*  If the subj SL is set, make sure that max dominates min */

	if (aufill->as.slevel_min && aufill->as.slevel_max) {
		privvec_t s;

		RAISE_PRIVS(s);

		switch(mand_tag_relationship(aufill->as.slevel_min,
					     aufill->as.slevel_max)) {

		seteffprivs(s, NULL);

		case MAND_ODOM:
		case MAND_EQUAL:
			break;
		default:
			pop_msg("Maximimum subject sensitivity label does not",
			"dominate minimum subject sensitivity label");
			ret = 1;
			goto out;
		}
	}

	/*  If the obj SL is set, make sure that max dominates min */

	if (aufill->as.olevel_min && aufill->as.olevel_max) {
		privvec_t s;

		RAISE_PRIVS(s);

		switch(mand_tag_relationship(aufill->as.olevel_min,
					     aufill->as.olevel_max)) {

		seteffprivs(s, NULL);

		case MAND_ODOM:
		case MAND_EQUAL:
			break;
		default:
			pop_msg("Maximimum object sensitivity label does not",
			"dominate minimum object sensitivity label");
			ret = 1;
			goto out;
		}
	}
#endif

out:
	EXITFUNC("aurc_valid");

	return ret;
}

/*
 * Expand the file table, if necessary.
 * This happens if the user types in the last entry on the screen.
 * Returns 1 if the screen needs to be redrawn.
 */

static int
FileTableExpand()
{
	int i;
	char **ntable;
	struct scrn_struct *sp;
	int ret;

	ENTERFUNC("FileTableExpand");
	if (FileTable[Nfiles - 1][0] != '\0')  {

		ntable = expand_cw_table(FileTable,
				Nfiles, Nfiles + 1, FILEWIDTH + 1);
		if (ntable == (char **) 0)
			MemoryError();

		Nfiles++;
		FileTable = ntable;

		/* update the screen field */

		sp = PARMTEMPLATE.ss;
		sp[AURC_FILETAB_STRUCT].pointer = (char *) FileTable;
		sp[AURC_FILETAB_STRUCT].filled = Nfiles;
		ret = 1;
	}
	else
		ret = 0;
			
	EXITFUNC("FileTableExpand");
	return ret;
}

/*
 * Action routine
 * For toggles that are off, set the tag values to 0.
 * Prepare data in the format required by SelectionWriteFile.
 * Call the appropriate action routine.
 */

static int
aurc_action(aufill)
	struct audit_selection_struct *aufill;
{
	int i;
	int count;
	char **table;
	int ret;
	char **old_filetab;
	int old_nfiles;

	ENTERFUNC("aurc_action");

	/* create the user array consisting of only those selected */

	count = 0;
	for (i = 0; i < Nusers; i++)
		if (UserState[i])
			count++;

	if (count > aufill->nusers) {
		if (aufill->users == (char **) 0)
			table = alloc_cw_table(count, 9);
		else
			table = expand_cw_table(aufill->users,
						aufill->nusers, count, 9);
		if (table == (char **) 0)
			MemoryError();
		aufill->users = table;
		aufill->nusers = count;
	}

	for (i = 0; i < count; i++)
		if (UserState[i])
			strcpy(aufill->users[i], UserTable[i]);
	
	/* create the group array consisting of only those selected */

	count = 0;
	for (i = 0; i < Ngroups; i++)
		if (GroupState[i])
			count++;

	if (count > aufill->ngroups) {
		if (aufill->groups == (char **) 0)
			table = alloc_cw_table(count, NGROUPNAME);
		else
			table = expand_cw_table(aufill->groups,
					aufill->ngroups, count, NGROUPNAME);
		if (table == (char **) 0)
			MemoryError();
		aufill->groups = table;
		aufill->ngroups = count;
	}

	for (i = 0; i < count; i++)
		if (GroupState[i])
			strcpy(aufill->groups[i], GroupTable[i]);
	

	/* create the yes/no characters for the event table */

	for (i = 0; i < AUDIT_MAX_EVENT; i++)
		if (EventState[i])
			aufill->events[i][0] = YESCHAR;
		else
			aufill->events[i][0] = NOCHAR;

	/* use our file table */

	old_filetab = aufill->files;
	old_nfiles = aufill->nfiles;

	aufill->files = FileTable;
	aufill->nfiles = Nfiles - 1; /* note we keep an extra one */

#if SEC_MAC
	/* sensitivity labels */

	if (!SubjMinSLFlag) {
		mand_free_ir(aufill->slevel_min);
		aufill->as.slevel_min = (tag_t) 0;
		aufill->slevel_min = (mand_ir_t *) 0;
	}

	if (!SubjMaxSLFlag) {
		mand_free_ir(aufill->slevel_max);
		aufill->as.slevel_max = (tag_t) 0;
		aufill->slevel_max = (mand_ir_t *) 0;
	}

	if (!ObjMinSLFlag) {
		mand_free_ir(aufill->olevel_min);
		aufill->as.olevel_min = (tag_t) 0;
		aufill->olevel_min = (mand_ir_t *) 0;
	}

	if (!ObjMaxSLFlag) {
		mand_free_ir(aufill->olevel_max);
		aufill->as.olevel_max = (tag_t) 0;
		aufill->olevel_max = (mand_ir_t *) 0;
	}

#endif /* SEC_MAC */

	ret = SelectionWriteFile(aufill);

	aufill->files = old_filetab;
	aufill->nfiles = old_nfiles;

	/*
	 * Go ahead and pass back through to the upper level screen.
	 */

	aurc_scrn.skip = 1;

	EXITFUNC("aurc_action");
	return INTERRUPT;
}

/*
 * Free all the tables and state structures
 */

static void
rc_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	struct audit_selection_struct *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("rc_free");
	if (UserTable != (char **) 0) {
		free_cw_table(UserTable);
		UserTable = (char **) 0;
	}
	if (UserState != NULL) {
		free(UserState);
		UserState = NULL;
	}
	if (GroupTable != (char **) 0) {
		free_cw_table(GroupTable);
		GroupTable = (char **) 0;
	}
	if (GroupState != NULL) {
		free(GroupState);
		GroupState = NULL;
	}
	if (EventTable != (char **) 0) {
		free_cw_table(EventTable);
		EventTable = (char **) 0;
	}
	if (EventState != NULL) {
		free(EventState);
		EventState = NULL;
	}
	if (FileTable != (char **) 0) {
		free_cw_table(FileTable);
		FileTable = (char **) 0;
	}

#if SEC_MAC
	if (aufill->slevel_min != (mand_ir_t *) 0) {
		mand_free_ir(aufill->slevel_min);
		aufill->slevel_min = (mand_ir_t *) 0;
	}
	if (aufill->slevel_max != (mand_ir_t *) 0) {
		mand_free_ir(aufill->slevel_max);
		aufill->slevel_max = (mand_ir_t *) 0;
	}
	if (aufill->olevel_min != (mand_ir_t *) 0) {
		mand_free_ir(aufill->olevel_min);
		aufill->olevel_min = (mand_ir_t *) 0;
	}
	if (aufill->olevel_max != (mand_ir_t *) 0) {
		mand_free_ir(aufill->olevel_max);
		aufill->olevel_max = (mand_ir_t *) 0;
	}
#endif
	SelectionFreeTables(aufill);
	EXITFUNC("rc_free");
}

#if SEC_MAC

/*
 * Subject minimum sensitivity label
 */

int
aurc_nssl()
{
	int ret;
	mand_ir_t *ir;

	ENTERFUNC("aurc_nssl");
	ir = mand_alloc_ir();
	if (ir == (mand_ir_t *) 0)
		MemoryError();
	mand_copy_ir(aurc_fill->slevel_min, ir);
	ret = aif_label(
	  "AUDIT SELECTION FILE MINIMUM SUBJECT SENSITIVITY LABEL",
	  ir, NULL, NULL, NULL, NULL);
	if (ret == 0) {
		privvec_t s;
		struct scrn_struct *sp;

		if (aurc_fill->slevel_min == (mand_ir_t *) 0)
			aurc_fill->slevel_min = ir;
		else {
			mand_copy_ir(ir, aurc_fill->slevel_min);
			mand_free_ir(ir);
		}

		/* compute tag value */

		RAISE_PRIVS(s);

		/*
		 * the sensitivity label was already compared to
		 * the maximum sensitivity label
		 */

		mand_ir_to_tag(aurc_fill->slevel_min,
					&aurc_fill->as.slevel_min);
		seteffprivs(s, NULL);
	
		/* mark the subject min SL as having been set */

		SubjMinSLFlag = 1;
	}
	else
		mand_free_ir(ir);
	
	/* don't pass through next level screen on the way up */

	aurc_scrn.skip = 0;

	EXITFUNC("aurc_nssl");
	return ret;
}

/*
 * Subject maximum sensitivity label
 */

int
aurc_xssl()
{
	int ret;
	mand_ir_t *ir;

	ENTERFUNC("aurc_xssl");
	ir = mand_alloc_ir();
	if (ir == (mand_ir_t *) 0)
		MemoryError();
	mand_copy_ir(aurc_fill->slevel_max, ir);
	ret = aif_label(
	  "AUDIT SELECTION FILE MAXIMUM SUBJECT SENSITIVITY LABEL",
	  ir, NULL, NULL, NULL, NULL);
	if (ret == 0) {
		privvec_t s;

		if (aurc_fill->slevel_max == (mand_ir_t *) 0)
			aurc_fill->slevel_max = ir;
		else {
			mand_copy_ir(ir, aurc_fill->slevel_max);
			mand_free_ir(ir);
		}

		/* compute tag value */

		RAISE_PRIVS(s);

		/*
		 * the sensitivity label was already compared to
		 * the minimum sensitivity label
		 */
		mand_ir_to_tag(aurc_fill->slevel_max,
					&aurc_fill->as.slevel_max);
		seteffprivs(s, NULL);
	
		/* mark the subject max SL as having been set */

		SubjMaxSLFlag = 1;
	}
	else
		mand_free_ir(ir);
	
	/* don't pass through next level screen on the way up */

	aurc_scrn.skip = 0;

	EXITFUNC("aurc_xssl");
	return ret;
}

/*
 * Object minimum sensitivity label
 */

int
aurc_nosl()
{
	int ret;
	mand_ir_t *ir;

	ENTERFUNC("aurc_nosl");
	ir = mand_alloc_ir();
	if (ir == (mand_ir_t *) 0)
		MemoryError();
	mand_copy_ir(aurc_fill->olevel_min, ir);
	ret = aif_label(
	  "AUDIT SELECTION FILE MINIMUM OBJECT SENSITIVITY LABEL",
	  ir, NULL, NULL, NULL, NULL);
	if (ret == 0) {
		privvec_t s;

		if (aurc_fill->olevel_min == (mand_ir_t *) 0)
			aurc_fill->olevel_min = ir;
		else {
			mand_copy_ir(ir, aurc_fill->olevel_min);
			mand_free_ir(ir);
		}

		/* compute tag value */

		RAISE_PRIVS(s);

		/*
		 * the sensitivity label was already compared to
		 * the maximum sensitivity label
		 */

		mand_ir_to_tag(aurc_fill->olevel_min,
					&aurc_fill->as.olevel_min);
		seteffprivs(s, NULL);
	
		/* mark the object min SL as having been set */

		ObjMinSLFlag = 1;
	} else
		mand_free_ir(ir);
	
	/* don't pass through next level screen on the way up */

	aurc_scrn.skip = 0;

	EXITFUNC("aurc_nosl");
	return ret;
}

/*
 * Object maximum sensitivity label
 */

int
aurc_xosl()
{
	int ret;
	mand_ir_t *ir;

	ENTERFUNC("aurc_xosl");
	ir = mand_alloc_ir();
	if (ir == (mand_ir_t *) 0)
		MemoryError();

	mand_copy_ir(aurc_fill->olevel_max, ir);
	ret = aif_label(
	  "AUDIT SELECTION FILE MAXIMUM OBJECT SENSITIVITY LABEL",
	  ir, NULL, NULL, NULL, NULL);
	if (ret == 0) {
		privvec_t s;

		if (aurc_fill->olevel_max == (mand_ir_t *) 0)
			aurc_fill->olevel_max = ir;
		else {
			mand_copy_ir(ir, aurc_fill->olevel_max);
			mand_free_ir(ir);
		}

		/* compute tag value */

		RAISE_PRIVS(s);

		/*
		 * the sensitivity label was already compared to
		 * the minimum sensitivity label
		 */
		mand_ir_to_tag(aurc_fill->olevel_max,
					&aurc_fill->as.olevel_max);
		seteffprivs(s, NULL);
	
		/* mark the object max SL as having been set */

		ObjMaxSLFlag = 1;
	}
	else
		mand_free_ir(ir);
	
	/* don't pass through next level screen on the way up */

	aurc_scrn.skip = 0;

	EXITFUNC("aurc_xosl");
	return ret;
}

#endif /* SEC_MAC */

#define SETUPFUNC	aurc_setup		/* defined by stemplate.c */
#define AUTHFUNC	aurc_auth
#define BUILDFILLIN	aurc_bfill

#define INITFUNC	aurc_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aurc_bstruct

#define ROUTFUNC	aurc_exit		/* defined by stemplate.c */
#define VALIDATE	aurc_valid
#define SCREENACTION	aurc_action

#define FREEFUNC	aurc_free		/* defined by stemplate.c */
#define FREESTRUCT	rc_free

#include "stemplate.c"

#endif /* SEC_BASE */

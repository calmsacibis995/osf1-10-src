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
static char	*sccsid = "@(#)$RCSfile: XMakeUser.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:37 $";
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

#if SEC_BASE


/*
	filename:
		XMakeUser.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for creating a user account
		
	entry points:
		MakeUserStart()
		MakeUserOpen()
		MakeUserClose()
		MakeUserStop()

*/

/* Common C include files */
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#ifdef AUX
#include <string.h>
#else
#include <strings.h>
#endif

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/List.h>
#include <Xm/Text.h>

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XMacros.h"

/* External routines */
extern Widget
	CreateAttachedForm(),
	CreateScrolledLabelledList();

extern char
	*malloc();

extern int
	chooseuserid(),
	CheckPasswordAccess(),
	CheckValidName(),
	UserNameExists(),
	CheckValidUid(),
	UserUidExists(),
	CheckValidHomeDir(),
	CheckValidShell(),
	CheckValidComment(),
	XModifyUserGroup();

extern void
	SetWidgetHeightAndWidth(),
	GetAllGroups();

/* Local routines */
static void
	SingleCallback(),
	MultipleCallback();

/* Local variables */
static XmString
	*groups_xmstring;

static Widget
	primary_group,
	secondary_group;

static int
	user_gid,
	group_list_count,
	ngroups;

static char
	**group_list,
	**groups,
	**msg_make_user_value,
	**msg_make_user_toggle,
	**msg_error_must_list_group,
	**msg_error_cant_access_passwd,
	**msg_error_invalid_name,
	**msg_error_name_exists,
	**msg_error_invalid_uid,
	**msg_error_uid_exists,
	**msg_error_invalid_home_dir,
	**msg_error_invalid_shell,
	**msg_error_invalid_comment,
	**msg_error_cant_update_password,
	**msg_error_cant_create_home,
	*msg_make_user_value_text,
	*msg_make_user_toggle_text,
	*msg_error_must_list_group_text,
	*msg_error_cant_access_passwd_text,	/*  1 */
	*msg_error_invalid_name_text,		/*  2 */
	*msg_error_name_exists_text,		/*  3 */
	*msg_error_invalid_uid_text,		/*  4 */
	*msg_error_uid_exists_text,		/*  5 */
	*msg_error_invalid_home_dir_text,	/*  6 */
	*msg_error_invalid_shell_text,		/*  7 */
	*msg_error_invalid_comment_text,	/*  8 */
	*msg_error_cant_update_password_text,	/*  9 */
	*msg_error_cant_create_home_text;	/* 10 */

static struct passwd	newpwd;

/* Local routines */

/* Definitions */
#define NUM_USER_MAKE_VALUES	5

/* Define the size of the text widgets */ 
static int make_user_text_col [NUM_USER_MAKE_VALUES] = 
	{8,5,39,39,39,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (MakeUserStart, MakeUserOpen, MakeUserClose, 
			MakeUserStop)

static void 
MakeWidgets() 
{
	Widget      	work_area1_frame,
			work_area1_widget,
			work_area2_form,
			work_area3_form,
			work_area2_frame,
			work_area3_frame,
			ok_button,
			cancel_button,
			help_button,
			w,w1;
	Dimension   	max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "MakeUser");
	XtAddCallback(form_widget, XmNhelpCallback,
			HelpDisplayOpen, "accounts,MakeUser");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage("msg_accounts_make_user", &msg_header
			,&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* First create the form work area widget                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	
	max_label_width = (Dimension) 0;

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	value     	 = MallocInt(NUM_USER_MAKE_VALUES);
	text_widget 	 = MallocWidget(NUM_USER_MAKE_VALUES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_make_user_value)
		LoadMessage("msg_accounts_make_user_value", 
			&msg_make_user_value, &msg_make_user_value_text);
	CreateItemsText(work_area1_widget, 0, NUM_USER_MAKE_VALUES,
		msg_make_user_value, &max_label_width, make_user_text_col, 
		text_widget);

	/**********************************************************************/
	/* Create the primary and secondary groups                            */
	/**********************************************************************/
	work_area2_frame = CreateFrame(form_widget, work_area1_frame, 
		True, NULL);
	work_area2_form = CreateSecondaryForm(work_area2_frame);
	SetWidgetWidth(work_area2_form, (Dimension) 182);
	primary_group = CreateScrolledLabelledList(work_area2_form,
		msg_header[4], "PrimaryGroup", True, NULL, True, NULL,
		(unsigned char) XmSINGLE_SELECT);
	XtAddCallback(primary_group, XmNsingleSelectionCallback, 
		SingleCallback, NULL);

	work_area3_frame = CreateFrame(form_widget, work_area1_frame, 
		False, work_area2_frame);
	work_area3_form = CreateSecondaryForm(work_area3_frame);
	SetWidgetWidth(work_area3_form, (Dimension) 182);
	secondary_group = CreateScrolledLabelledList(work_area3_form,
		msg_header[5], "SecondaryGroup", True, NULL, True, NULL,
		(unsigned char) XmMULTIPLE_SELECT);
	XtAddCallback(secondary_group, XmNmultipleSelectionCallback, 
		MultipleCallback, NULL);

	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons                            */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
				&ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				HelpDisplayOpen, "accounts,MakeUser");
}


CREATE_CALLBACKS(msg_header[3], MakeUserClose)

static int
LoadVariables()
{
	char 	buf[10];
	int	i, n;
	Arg	args[10];
	int 	id;

	/*  0  - User name
	    1  - Group id
	    2  - Home dir
	    3  - Shell
	    4  - Comment
	    */
	XmTextSetString (text_widget[0], NULL);
	id = chooseuserid();
	if (id != 0) {
		sprintf (buf, "%d", id);
		XmTextSetString (text_widget[1], buf);
	}
	else
		XmTextSetString (text_widget[1], NULL);
	XmTextSetString (text_widget[2], "/users/");
	XmTextSetString (text_widget[3], "/bin/sh");
	XmTextSetString (text_widget[4], NULL);

	GetAllGroups (&ngroups, &groups);

	/* Malloc XmString arrays for the lists of all users and groups */
	groups_xmstring = (XmString *) Malloc(sizeof(XmString) * ngroups);
	if (! groups_xmstring)
	        MemoryError();
        
	/* Load XmString arrays with values of valid groups and groups */
	for (i = 0; i < ngroups; i++) {
		groups_xmstring[i] = XmStringCreate(groups[i], charset);
		if (! groups_xmstring[i])
			MemoryError();
	}
		
	/* Load in group values */
	n = 0;
	XtSetArg(args[n], XmNitemCount,                     ngroups); n++;
	XtSetArg(args[n], XmNitems,                 groups_xmstring); n++;
	XtSetValues(primary_group, args, n);
	XtSetValues(secondary_group, args, n);
	XmListDeselectAllItems(primary_group);
	XmListDeselectAllItems(secondary_group);
	user_gid = -1;
	group_list_count = -1;
	group_list = (char **) Malloc (sizeof(char *));
	if (! group_list)
		MemoryError();
	group_list[0] = (char *) Malloc (sizeof(char *));
	if (! group_list[0])
		MemoryError();
	*group_list[0] = '\0';

	in_change_mode = True;

	/* free the memory used by XmString */
	for (i = 0; i < ngroups; i++)
		XmStringFree(groups_xmstring[i]);
	free(groups_xmstring);

	return (SUCCESS);
}

/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	char *buf;

	/* Check that a group is listed */
	if (user_gid == -1) {
		if (! msg_error_must_list_group)
			LoadMessage("msg_accounts_make_user_must_list_group",
				&msg_error_must_list_group, 
				&msg_error_must_list_group_text);
		ErrorMessageOpen(-1, msg_error_must_list_group, 0, NULL);
		return (FAILURE);
	}

	/* Must be able to access the password file */
	if (CheckPasswordAccess() != SUCCESS) {
		if (! msg_error_cant_access_passwd)
			LoadMessage("msg_accounts_make_user_cant_access_passwd",
				&msg_error_cant_access_passwd, 
				&msg_error_cant_access_passwd_text);
		ErrorMessageOpen(-1, msg_error_cant_access_passwd, 0, NULL);
		return (FAILURE);
	}

	/* Check user name is valid. No ":" */
	newpwd.pw_name = XmTextGetString(text_widget[0]);
	if (CheckValidName(newpwd.pw_name) != SUCCESS) {
		if (! msg_error_invalid_name)
			LoadMessage("msg_accounts_make_user_invalid_name",
				&msg_error_invalid_name, 
				&msg_error_invalid_name_text);
		ErrorMessageOpen(-1, msg_error_invalid_name, 0, NULL);
		XtFree(newpwd.pw_name);
		return (FAILURE);
	}

	/* Check to see if name already exists */
	if (UserNameExists(newpwd.pw_name) != SUCCESS) {
		if (! msg_error_name_exists)
			LoadMessage("msg_accounts_make_user_exists",
				&msg_error_name_exists, 
				&msg_error_name_exists_text);
		ErrorMessageOpen(-1, msg_error_name_exists, 0, NULL);
		XtFree(newpwd.pw_name);
		return (FAILURE);
	}

	/* Check the user id is valid */
	buf = XmTextGetString(text_widget[1]);
	newpwd.pw_uid = atoi(buf);
	XtFree(buf);
	if (CheckValidUid(newpwd.pw_uid) != SUCCESS) {
		if (! msg_error_invalid_uid)
			LoadMessage("msg_accounts_make_user_invalid_uid",
				&msg_error_invalid_uid, 
				&msg_error_invalid_uid_text);
		ErrorMessageOpen(-1, msg_error_invalid_uid, 0, NULL);
		XtFree(newpwd.pw_name);
		return (FAILURE);
	}

	/* Check the uid doesn't already exist */
	if (UserUidExists(newpwd.pw_uid) != SUCCESS) {
		if (! msg_error_uid_exists)
			LoadMessage("msg_accounts_make_user_uid_exists",
				&msg_error_uid_exists, 
				&msg_error_uid_exists_text);
		ErrorMessageOpen(-1, msg_error_uid_exists, 0, NULL);
		XtFree(newpwd.pw_name);
		return (FAILURE);
	}

	newpwd.pw_dir = XmTextGetString(text_widget[2]);
#ifdef OLD_CODE
	/* Leave this code out - too confusing - should make sys admin
	 * explicitly specify a home directory
	 */
	if (! newpwd.pw_dir) {
		newpwd.pw_dir = (char *) Malloc (strlen("/users/") + 
			strlen (newpwd.pw_name) + 1);
		if (! newpwd.pw_dir)
			MemoryError();
		sprintf (newpwd.pw_dir, "/users/%s", newpwd.pw_name);
	}
#endif

	/* Check the home directory is valid */
	if (CheckValidHomeDir(&newpwd) != SUCCESS) {
		if (! msg_error_invalid_home_dir)
			LoadMessage("msg_accounts_make_user_invalid_home_dir",
				&msg_error_invalid_home_dir, 
				&msg_error_invalid_home_dir_text);
		ErrorMessageOpen(-1, msg_error_invalid_home_dir, 0, NULL);
		XtFree(newpwd.pw_dir);
		XtFree(newpwd.pw_name);
		return (FAILURE);
	}

	/* Check shell name is valid, no ":", is executable */
	newpwd.pw_shell = XmTextGetString(text_widget[3]);
	if (CheckValidShell(newpwd.pw_shell) != SUCCESS) {
		if (! msg_error_invalid_shell)
			LoadMessage("msg_accounts_make_user_invalid_shell",
				&msg_error_invalid_shell, 
				&msg_error_invalid_shell_text);
		ErrorMessageOpen(-1, msg_error_invalid_shell, 0, NULL);
		XtFree(newpwd.pw_shell);
		XtFree(newpwd.pw_dir);
		XtFree(newpwd.pw_name);
		return (FAILURE);
	}

	/* Check comment field is valid */
	newpwd.pw_gecos = XmTextGetString(text_widget[4]);
	if (CheckValidComment(newpwd.pw_gecos) != SUCCESS) {
		if (! msg_error_invalid_comment)
			LoadMessage("msg_accounts_make_user_invalid_comment",
				&msg_error_invalid_comment, 
				&msg_error_invalid_comment_text);
		ErrorMessageOpen(-1, msg_error_invalid_comment, 0, NULL);
		XtFree(newpwd.pw_gecos);
		XtFree(newpwd.pw_shell);
		XtFree(newpwd.pw_dir);
		XtFree(newpwd.pw_name);
		return (FAILURE);
	}

	return (SUCCESS);
}

static void
SingleCallback(w, ptr, info)
	Widget                  w;
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	struct 	group 	*grp;
	int	i;

#ifdef DEBUG
	printf ("Single callback\n");
	printf ("Reason %d\n", info->reason);
	printf ("item p %d\n", info->item_position);
	printf ("Group name is %s\n", 
		groups[info->item_position - 1]);
#endif
	grp      = getgrnam(groups[info->item_position - 1]);
#ifdef DEBUG
	printf ("Got the gid call\n");
	printf ("Name is %s\n", grp->gr_name);
	printf ("id is %d\n", grp->gr_gid);
#endif
	user_gid = grp->gr_gid;
}


static void
MultipleCallback(w, ptr, info)
	Widget                  w;
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	int	i;

	/* Save space from last time */
	if (group_list) {
		for (i=0; i<group_list_count; i++)
			free(group_list[i]);
		free(group_list);
	}

	group_list_count = info->selected_item_count;

	if (group_list_count == 0) {
		group_list = (char **) Malloc (sizeof(char *));
		if (! group_list)
			MemoryError();
		group_list[0] = (char *) Malloc (sizeof(char *));
		if (! group_list[0])
			MemoryError();
		*group_list[0] = '\0';
	}
	else {
		group_list = (char **) Malloc (sizeof(char *) * 
			(info->selected_item_count + 1) );
		if (! group_list)
			MemoryError();
		for (i=0; i<group_list_count; i++) {
			group_list[i] = (char *) strdup(extract_normal_string(
				info->selected_items[i]) );
			if (! group_list[i])
				MemoryError();
		}
		group_list[group_list_count] = (char *) strdup("\0");
	}

#ifdef DEBUG
	printf ("Multiple callback\n");
	printf ("Reason %d\n", info->reason);
	printf ("item c %d\n", info->selected_item_count);
	for (i=0; i<info->selected_item_count; i++)
		printf ("Selectedlist %d %s\n", i,
			extract_normal_string(info->selected_items[i]) );
#endif
}


static int
WriteInformation ()
{
	int ret;

	/* Create a new user */
	newpwd.pw_gid = user_gid;
	newpwd.pw_passwd = "";
#ifdef AUX
	newpwd.pw_age = "";
#endif
	newpwd.pw_comment = "";
	ret = XCreateUserAccount(&newpwd);
	if (ret == SUCCESS)
		ret = XModifyUserGroup(newpwd.pw_name, newpwd.pw_gid, 
			group_list);
	XtFree(newpwd.pw_gecos);
	XtFree(newpwd.pw_shell);
	XtFree(newpwd.pw_dir);
	XtFree(newpwd.pw_name);
	return (ret);
}

#endif /* SEC_BASE **/

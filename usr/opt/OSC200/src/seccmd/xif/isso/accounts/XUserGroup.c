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
static char	*sccsid = "@(#)$RCSfile: XUserGroup.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:16 $";
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
		XUserGroup.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting group membership for user account
		
	entry points:
		UserGroupStart()
		UserGroupOpen()
		UserGroupClose()
		UserGroupStop()

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
#include <Xm/Label.h>
#include <Xm/Text.h>

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XMacros.h"

/* External routines */
extern Widget
	CreateAttachedForm(),
	CreateScrolledLabelledList();

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
	XModifyUserpwd(),
	XModifyUserGroup();

extern void
	SetWidgetHeightAndWidth(),
	GetAllGroups();

/* Local routines */
static void
	SetUserUid(),
	SingleCallback(),
	MultipleCallback();

static Widget
	CreateUserUid();

/* Local variables */
static struct passwd
	newpwd;

static XmString
	*groups_xmstring;

static Widget
	user_uid_widget,
	primary_group,
	secondary_group;

static int
	user_uid,
	user_gid,
	group_list_count,
	ngroups;

static char
	**group_list,
	**groups,
	**msg_user_group_value,
	**msg_user_group_toggle,
	**msg_error_must_list_group,
	**msg_error_cant_read_pwd, 
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
	*msg_user_group_value_text,
	*msg_user_group_toggle_text,
	*msg_error_must_list_group_text,
	*msg_error_cant_read_pwd_text, 
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

/* Local routines */

/* Definitions */
#define NUM_USER_GROUP_VALUES	3

/* Define the size of the text widgets */ 
static int make_user_text_col [NUM_USER_GROUP_VALUES] = 
	{39,39,39,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (UserGroupStart, UserGroupOpen, UserGroupClose, 
			UserGroupStop)

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
	int         	i;
	Dimension   	max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "UserGroup");
	XtAddCallback(form_widget, XmNhelpCallback,
			HelpDisplayOpen, "accounts,UserGroup");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage("msg_accounts_user_group", 
			&msg_header,&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* User name                                                          */
	/**********************************************************************/
	user_name_widget = CreateUserName (form_widget, w);
	user_uid_widget  = CreateUserUid  (form_widget, user_name_widget);

	/**********************************************************************/
	/* First create the form work area widget                             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	work_area1_frame  = CreateFrame(form_widget, user_uid_widget, 
		True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	max_label_width = (Dimension) 0;

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	value     	 = MallocInt(NUM_USER_GROUP_VALUES);
	text_widget 	 = MallocWidget(NUM_USER_GROUP_VALUES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_user_group_value)
		LoadMessage("msg_accounts_user_group_value", 
			&msg_user_group_value, &msg_user_group_value_text);
	CreateItemsText(work_area1_widget, 0, NUM_USER_GROUP_VALUES,
		msg_user_group_value, &max_label_width, make_user_text_col, 
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
	/* Create the OK, Cancel and Help buttons
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
				&ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				HelpDisplayOpen, "accounts,UserGroup");
}


CREATE_CALLBACKS(msg_header[3], UserGroupClose)

static int
LoadVariables()
{
	struct passwd	*pwd;
	struct group 	*grp;
	XmString	xmstring;
	char 	buf[10];
	char    *group_name;
	int	i, n;
	Arg	args[10];
	int	primary_selected;
	int 	id;
	int	count;	/* total # times a user appears */
	char	**cp;

	/*  2  - Home dir
	    3  - Shell
	    4  - Comment
	    */
	pwd = getpwnam (chosen_user_name);
	if (pwd == (struct passwd *) 0) {
		if (! msg_error_cant_read_pwd)
			LoadMessage("msg_accounts_make_user_cant_read_pwd",
				&msg_error_cant_read_pwd, 
				&msg_error_cant_read_pwd_text);
		ErrorMessageOpen(-1, msg_error_cant_read_pwd, 0, NULL);
		return (FAILURE);
	}
	user_uid = (int) pwd->pw_uid;
	user_gid = (int) pwd->pw_gid;
	grp = getgrgid (pwd->pw_gid);
#ifdef DEBUG
	printf ("User id ,gid %d %d \n", user_uid, user_gid);
	printf ("Group name %s\n", grp->gr_name);
#endif
	group_name = (char *) strdup (grp->gr_name);

	XmTextSetString (text_widget[0], pwd->pw_dir);
	XmTextSetString (text_widget[1], pwd->pw_shell);
	XmTextSetString (text_widget[2], pwd->pw_gecos);

	GetAllGroups (&ngroups, &groups);

	/* Malloc XmString arrays for the lists of all users and groups */
	groups_xmstring = (XmString *) Malloc(sizeof(XmString) * ngroups);
	if (! groups_xmstring)
	        MemoryError();
        
	/* Load XmString arrays with values of valid groups and groups */
	primary_selected = -1;
	for (i = 0; i < ngroups; i++) {
		if (strcmp(groups[i], group_name) == 0)
			primary_selected = i;
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
	user_gid = (int) pwd->pw_gid;
	/* If this is false then we have an error situation. Should handle
	 * better than this however this line should never be false */
	if (primary_selected != -1)
		XmListSelectItem(primary_group, 
			groups_xmstring[primary_selected], False);

	/* Loop through the list of groups and set the groups list */
	group_list_count = -1;
	group_list = (char **) Malloc (sizeof(char *));
	group_list[0] = (char *) Malloc (sizeof(char *));
	*group_list[0] = '\0';
	setgrent();
	while (grp = getgrent()) {
		for (cp = grp->gr_mem; *cp; cp++) {
			if (strcmp (chosen_user_name, *cp) == 0) {
				xmstring = XmStringLtoRCreate(
					grp->gr_name, charset);
				if (! xmstring)
					MemoryError();
				XmListSelectItem(secondary_group, xmstring, 
					True);
				XmStringFree(xmstring);
			}
		}
	}

	in_change_mode = True;
	SetUserName(user_name_widget, chosen_user_name);
	SetUserUid(user_uid_widget, user_uid);
	return (SUCCESS);
}

/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	char *buf;
	int i;
	int nerrs = 0;

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

	newpwd.pw_name = (char *) strdup(chosen_user_name);
	newpwd.pw_dir = XmTextGetString(text_widget[0]);
	/* Leave this code out - too confusing - should make sys admin
	 * explicitly specify a home directory
	 */
	/*
	if (! newpwd.pw_dir) {
		newpwd.pw_dir = (char *) Malloc (strlen("/users/") + 
			strlen (newpwd.pw_name) );
		sprintf (newpwd.pw_dir, "/users/%s", newpwd.pw_name);
	}
	*/

	newpwd.pw_shell = XmTextGetString(text_widget[1]);
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

	newpwd.pw_gecos = XmTextGetString(text_widget[2]);
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
	user_gid = (int) grp->gr_gid;
}


static void
MultipleCallback(w, ptr, info)
	Widget                  w;
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	char	group_name;
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
		group_list[0] = (char *) Malloc (sizeof(char *));
		*group_list[0] = '\0';
	}
	else {
		group_list = (char **) Malloc (sizeof(char *) * 
			(info->selected_item_count + 1) );
		for (i=0; i<group_list_count; i++) {
			/*
			group_name = extract_normal_string(
				info->selected_items[i]);
			printf ("Group_name %d %s\n", i, group_name);
			*/
			group_list[i] = (char *) strdup(extract_normal_string(
				info->selected_items[i]) );
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
	newpwd.pw_uid = (uid_t) user_uid;
	newpwd.pw_gid = (gid_t) user_gid;
	newpwd.pw_passwd = "*";
#ifdef AUX
	newpwd.pw_age = "";
#endif
	newpwd.pw_comment = "";
	ret = XModifyUserpwd(&newpwd);
	XtFree(newpwd.pw_gecos);
	XtFree(newpwd.pw_shell);
	XtFree(newpwd.pw_dir);
	if (ret == SUCCESS)
		ret = XModifyUserGroup(newpwd.pw_name, (int) newpwd.pw_gid, 
			group_list);
	XtFree(newpwd.pw_name);
	return (ret);
}

/***********************************************************************/
/* Create the user name widget                                         */
/***********************************************************************/
static Widget 
CreateUserUid (Parent, topw)
	Widget Parent;
	Widget topw;
{ 
	Arg        args[6];
	Cardinal   n;
	Widget     w;
	XmString   xmstring;

	/* This string never gets used anyway so don't need a LoadMessage */
	xmstring = XmStringLtoRCreate("User id :", charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                         topw); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	w = XmCreateLabel(Parent, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);                          
	return (w);
}

static void
SetUserUid (w, user_uid)
	Widget    w;
	int       user_uid;
{
	Arg        args[2];
	Cardinal   n;
	XmString   xmstring;
	char       user_name_buffer[100];	/* Big enough I hope */
	static char **msg_user_uid,
		*msg_user_uid_text;

	if (! msg_user_uid)
		LoadMessage("msg_user_uid", &msg_user_uid, &msg_user_uid_text);
	sprintf (user_name_buffer, "%s : %d", msg_user_uid[0], user_uid);

	n = 0;
	xmstring = XmStringLtoRCreate(user_name_buffer, charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNlabelString,  xmstring); n++;
	XtSetValues(w, args, n);
	XmStringFree(xmstring);                          
}


#endif /* SEC_BASE **/

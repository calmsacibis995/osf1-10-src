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
static char	*sccsid = "@(#)$RCSfile: XMakeGroup.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:33 $";
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
		XMakeGroup.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for MakeGroup function in role programs.
		
	entry points:
		MakeGroupStart()
		MakeGroupOpen()
		MakeGroupClose()
		MakeGroupStop()

*/

/* Common C include files */
#include <sys/types.h>
#include <grp.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XMacros.h"

/* External routines */
extern int
	choosegroupid(),
	CheckGroupAccess(),
	CheckValidGroup(),
	GroupNameExists(),
	CheckValidGid(),
	GroupIdExists(),
	XCreateGroup();

/* Local variables */
static struct group newgroup;

static char
	**msg_make_group_value,
	**msg_error_cant_access_group,
	**msg_error_invalid_group,
	**msg_error_group_exists,
	**msg_error_invalid_gid,
	**msg_error_gid_exists,
	*msg_make_group_value_text,
	*msg_error_cant_access_group_text,
	*msg_error_invalid_group_text,
	*msg_error_group_exists_text,
	*msg_error_invalid_gid_text,
	*msg_error_gid_exists_text;


/* Local routines */

/* Definitions */
#define NUM_GROUP_MAKE_VALUES	2

/* Define the size of the text widgets */ 
static int make_group_text_col [NUM_GROUP_MAKE_VALUES] = 
	{8,5,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (MakeGroupStart, MakeGroupOpen, MakeGroupClose, 
			MakeGroupStop)

static void 
MakeWidgets() 
{
	Widget      	work_area1_frame,
			work_area1_widget,
			ok_button,
			cancel_button,
			help_button,
			w,w1;
	Dimension   	max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "MakeGroup");
	XtAddCallback(form_widget, XmNhelpCallback,
			HelpDisplayOpen, "accounts,MakeGroup");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage("msg_accounts_make_group", &msg_header,
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* First create the form work area widget                             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/* Want enough room to display three buttons */
	max_label_width = (Dimension) 150;

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	value     	 = MallocInt(NUM_GROUP_MAKE_VALUES);
	text_widget 	 = MallocWidget(NUM_GROUP_MAKE_VALUES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_make_group_value)
		LoadMessage("msg_accounts_make_group_value", 
			&msg_make_group_value, &msg_make_group_value_text);
	CreateItemsText(work_area1_widget, 0, NUM_GROUP_MAKE_VALUES,
		msg_make_group_value, &max_label_width, make_group_text_col, 
		text_widget);

	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area1_frame,
				&ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				HelpDisplayOpen, "accounts,MakeGroup");
}


CREATE_CALLBACKS(msg_header[3], MakeGroupClose)

static int
LoadVariables()
{
	char 	buf[10];
	int 	id;

	/*  0  - Group name
	    1  - Group id
	    */
	XmTextSetString (text_widget[0], NULL);
	id = choosegroupid();
	if (id != 0) {
		sprintf (buf, "%d", id);
		XmTextSetString (text_widget[1], buf);
	}
	else
		XmTextSetString (text_widget[1], NULL);
	in_change_mode = True;
	return (SUCCESS);
}

/************************************************************************/
/* Validates data 						        */
/************************************************************************/
static int
ValidateEntries ()
{
	char *buf;
	int i;
	int nerrs = 0;

	/* Must be able to access the password file */
	if (CheckGroupAccess() != SUCCESS) {
		if (! msg_error_cant_access_group)
			LoadMessage("msg_accounts_make_group_cant_access_group",
				&msg_error_cant_access_group, 
				&msg_error_cant_access_group_text);
		ErrorMessageOpen(-1, msg_error_cant_access_group, 0, NULL);
		return (FAILURE);
	}

	/* Check name is valid, no ":" */
	newgroup.gr_name = XmTextGetString(text_widget[0]);
	if (CheckValidGroup(newgroup.gr_name) != SUCCESS) {
		if (! msg_error_invalid_group)
			LoadMessage("msg_accounts_make_group_invalid_group",
				&msg_error_invalid_group, 
				&msg_error_invalid_group_text);
		ErrorMessageOpen(-1, msg_error_invalid_group, 0, NULL);
		XtFree(newgroup.gr_name);
		return (FAILURE);
	}

	/* Check if name already exists */
	if (GroupNameExists(newgroup.gr_name) != SUCCESS) {
		if (! msg_error_group_exists)
			LoadMessage("msg_accounts_make_group_exists",
				&msg_error_group_exists, 
				&msg_error_group_exists_text);
		ErrorMessageOpen(-1, msg_error_group_exists, 0, NULL);
		XtFree(newgroup.gr_name);
		return (FAILURE);
	}

	/* Check the group number is in a valid range */
	buf = XmTextGetString(text_widget[1]);
	newgroup.gr_gid = atoi(buf);
	if (buf)
		XtFree(buf);
	if (CheckValidGid(newgroup.gr_gid) != SUCCESS) {
		if (! msg_error_invalid_gid)
			LoadMessage("msg_accounts_make_group_invalid_gid",
				&msg_error_invalid_gid, 
				&msg_error_invalid_gid_text);
		ErrorMessageOpen(-1, msg_error_invalid_gid, 0, NULL);
		XtFree(newgroup.gr_name);
		return (FAILURE);
	}

	/* Check the id is unique */
	if (GroupIdExists(newgroup.gr_gid) != SUCCESS) {
		if (! msg_error_gid_exists)
			LoadMessage("msg_accounts_make_group_gid_exists",
				&msg_error_gid_exists, 
				&msg_error_gid_exists_text);
		ErrorMessageOpen(-1, msg_error_gid_exists, 0, NULL);
		XtFree(newgroup.gr_name);
		return (FAILURE);
	}

	return (SUCCESS);
}

static int
WriteInformation ()
{
	int ret;

	/* Create a new group */
	newgroup.gr_passwd = "";
	newgroup.gr_mem = (char **) 0;
	ret = XCreateGroup(&newgroup);
	XtFree(newgroup.gr_name);
	return (ret);
}

#endif /* SEC_BASE **/

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
static char	*sccsid = "@(#)$RCSfile: XUsrGrp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:09:07 $";
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

#ifdef SEC_BASE

/*
	filename:
		XUsrGrp.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for Users Groups display
		
	entry points:
		UsersGroupsStart()
		UsersGroupsOpen()
		UsersGroupsClose()
		UsersGroupsStop()
*/

#include "XAudit.h"
#include "Xm/Form.h"
#include "Xm/LabelG.h"
#include "Xm/List.h"
#include "Xm/MessageB.h"
#include "Xm/RowColumn.h"
#include "Xm/ScrolledW.h"
#include "Xm/ToggleB.h"

int     CheckAuditEnabled();

static Widget
		confirmation_widget,             
		confirm_this_session_widget,
		confirm_next_session_widget,
		form_widget,
		groups_list_widget,
		users_list_widget;
		
static int 
		nusers,                /* total number of users */
		ngroups,               /* total number of groups */
		
		confirmation_open,     /* toggle to initialize widgets*/
				       /* if first time confirmed */
				                         
		usersgroups_open;      /* toggle to initialize widgets*/
				       /* if first time usersgroups called*/
static char 
		**users,               /* list of all users */
		**groups,              /* list of all groups */
		**msg,                 /* message pointers */
		*msg_text;             /* message text space */
		
static AUDIT_USERS_GROUPS_STRUCT 
		valid_usrgrp;          /* entered user group data to be*/
				       /* validated and entered into */
				       /* database */
static XmString
	 *users_xmstring,              /* XmStrings for all users */
	 *groups_xmstring;             /* XmStrings for all groups*/
				       /* These are displayed in the*/
				       /* list widgets in the display*/
static void 
	 CancelCallback(),
	 ConfirmCancelCallback(),
	 ConfirmOKCallback(),
	 MakeWidgets(),
	 OKCallback();

/* Initialize variables that need to be initialized;
 * called once at startup time
 */
void 
UsersGroupsStart() 
{
	confirmation_open = FALSE;
	usersgroups_open = FALSE;
}

/* Main entry point for Xusersgroups.c - causes users/groups screen
 * to be displayed, user & group values to be loaded, etc.
 */
 
void 
UsersGroupsOpen(parent, ptr, info)
	Widget parent;
	char  *ptr;
	XmAnyCallbackStruct *info;
{
	Cardinal    n;
	
	Arg         args[20];
	int         i,
				j,
				audit_active,
				fd,
				rc;
	XmString    xmstring;
	AUDIT_USERS_GROUPS_STRUCT selected_usrgrp;
				              
	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);        
	
	/* Load Message Strings */
	if (! msg)
		LoadMessage("msg_isso_audit_usersgroups", &msg, &msg_text);
	
	/* Load in all users and groups on the system */
	GetAllUsers(&nusers, &users);
	GetAllGroups(&ngroups, &groups);

	/* Malloc XmString arrays for the lists of all users and groups */
	users_xmstring = (XmString *) Malloc(sizeof(XmString) * nusers);
	if (! users_xmstring)
		MemoryError();
		/* Dies */
		
	groups_xmstring = (XmString *) Malloc(sizeof(XmString) * ngroups);
	if (! groups_xmstring)
		MemoryError();   
		/* Dies */
	
	/* Load XmString arrays with values of valid users and groups */
	for (i = 0; i < nusers; i++) 
		users_xmstring[i] = XmStringCreate(users[i], charset);
		
	for (i = 0; i < ngroups; i++)
		groups_xmstring[i] = XmStringCreate(groups[i], charset);
		
	/* Load currently audited users and groups */
	rc = AuditUsersGroupsGet(&selected_usrgrp);
	if (rc == 1)
		ErrorMessageOpen(3010, msg, 12, NULL);
	/* if (rc == 2) another message has already been put up; simply return */
	if (rc == 2)
		return;
		
	/* Need to save general audit params data for case of writing
	*  data back to audit_parms file
	*/
	memcpy(&valid_usrgrp.au, &selected_usrgrp.au, sizeof(selected_usrgrp.au));
		
	/* First time -- create the Users/Groups window */
	if (! usersgroups_open) {
		MakeWidgets();
		usersgroups_open = TRUE;
	}
	
	/* Load in user and group values */
	n = 0;
	XtSetArg(args[n], XmNitemCount,                         nusers); n++;
	XtSetArg(args[n], XmNitems,                     users_xmstring); n++;
	XtSetValues(users_list_widget, args, n);
	
	n = 0;
	XtSetArg(args[n], XmNitemCount,                        ngroups); n++;
	XtSetArg(args[n], XmNitems,                    groups_xmstring); n++;
	XtSetValues(groups_list_widget, args, n);
	
	/* Deselect all items in both the users and the groups list */ 
	XmListDeselectAllItems(users_list_widget);
	XmListDeselectAllItems(groups_list_widget);
	
	/* Select items in the current audit list */
	for( i = 0; i < selected_usrgrp.nusers; i++) {
		xmstring = XmStringCreate(selected_usrgrp.users[i], charset);
		XmListSelectItem(users_list_widget, xmstring, False);
		XmStringFree(xmstring);
	}
	for ( i = 0; i < selected_usrgrp.ngroups; i++) {
		xmstring = XmStringCreate(selected_usrgrp.groups[i], charset);
		XmListSelectItem(groups_list_widget, xmstring, False);
		XmStringFree(xmstring);
	}            
	
	/* Free selected data structure */
	AuditUsersGroupsFree(&selected_usrgrp);
	
	/* Flag to say that when the this/next toggle is changed */
	/* next will be the one being selected                   */
	CenterForm(form_widget);
	
	/* Determine audit state to set sensitivity on buttons */
	audit_active = CheckAuditEnabled();
	
	/* Set toggles for confirm this seession / next session widget */
	XmToggleButtonSetState(confirm_this_session_widget, FALSE, FALSE);
	XtSetSensitive(confirm_this_session_widget, audit_active);
	XmToggleButtonSetState(confirm_next_session_widget, TRUE,  FALSE);
	WorkingClose();
}

/* Clean up after each invocation of UsersGroupsOpen;
 * called from the callbacks to UsersGroupsOpen;
 * frees transient memory, gets rid of screen and returns control 
 * to the menubar.
 */
void 
UsersGroupsClose() 
{

	int     i;
	
	/* Free various memory */
	if (users) { 
		free_cw_table(users);
		users = NULL;
	}
	if (groups) { 
		free_cw_table(groups);
		groups = NULL;
	}
	if (users_xmstring) {
		for (i = 0; i < nusers; i++)
			XmStringFree(users_xmstring[i]);
		free(users_xmstring);
		users_xmstring = NULL;
	}
	if (groups_xmstring) {
		for (i = 0; i < ngroups; i++)
			XmStringFree(groups_xmstring[i]);
		free(groups_xmstring);
		groups_xmstring = NULL;
	}
	
	nusers = 0;
	ngroups = 0;
	
	AuditUsersGroupsFree(&valid_usrgrp);
	
	XtSetSensitive(main_menubar, True);
	XtUnmanageChild(form_widget);
	
	if (save_memory) {
		if (usersgroups_open) {
			XtDestroyWidget(form_widget);
			usersgroups_open = FALSE;
		}
		if (confirmation_open) {
			XtDestroyWidget(confirmation_widget);
			confirmation_open = FALSE;
		}
	}
}

/* Called at exit for final clean up */
void 
UsersGroupsStop() 
{

	if (usersgroups_open) {
		XtDestroyWidget(form_widget);
		usersgroups_open = FALSE;
	}
	if (confirmation_open) {
		XtDestroyWidget(confirmation_widget);
		confirmation_open = FALSE;
	}
}

/* Makes the widgets for the collection mask screen; only called first
 * time UsersGroupsOpen is called.
 */
static void 
MakeWidgets() 
{
	Widget      action_button_widget,
		cancel_button,
		help_button,
		ok_button,
				groups_scrolled_window_widget,
				groups_scrollbar_widget,
				label_widget,
				list_label_widget,
				this_or_next_widget,
				users_scrollbar_widget,
				users_scrolled_window_widget;
	Cardinal    n;
	Arg         args[20];
	XmString    xmstring;

	/**********************************************************************/
	/* Form                                                               */
	/**********************************************************************/
	
	n = 0;
	XtSetArg(args[n], XmNautoUnmanage,                     False); n++;
	XtSetArg(args[n], XmNrubberPositioning,                 True); n++;
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,           XmATTACH_FORM); n++;
	form_widget = XtCreateWidget("UsersGroups", 
			xmFormWidgetClass, mother_form, args, n);
	XtAddCallback(form_widget, XmNhelpCallback,
				          HelpDisplayOpen, "audit,UsersGroups");
				                 
	/* Title */
	xmstring =  XmStringCreate(msg[0], charset);
	if (! xmstring)
	MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,              XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	label_widget = XmCreateLabelGadget(form_widget, "Title", args, n);
	XtManageChild(label_widget);
	XmStringFree(xmstring);                          

	/**********************************************************************/
	/* Users list in scrolled window, titled                              */
	/**********************************************************************/
	
	/* Label for users */
	xmstring = XmStringCreate(msg[1], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,                  xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,         XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftAttachment,          XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopWidget,                label_widget); n++;
	XtSetArg(args[n], XmNalignment,          XmALIGNMENT_CENTER); n++;
	list_label_widget = XmCreateLabelGadget(form_widget, 
				        "UsersLabel", args, n);
	XtManageChild(list_label_widget);
	XmStringFree(xmstring);
		
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                 list_label_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                                 3); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++; 
	users_scrolled_window_widget = XtCreateManagedWidget(
			"Users", xmScrolledWindowWidgetClass, 
			form_widget, args, n);

	n = 0;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                            True); n++;
#endif
	XtSetArg(args[n], XmNselectionPolicy,           XmMULTIPLE_SELECT); n++; 
	users_list_widget = XtCreateManagedWidget("List",
			xmListWidgetClass, users_scrolled_window_widget, args, n);

#ifdef TRAVERSAL
	n = 0;
	XtSetArg(args[n], XmNverticalScrollBar,      &users_scrollbar_widget); n++;
	XtGetValues(users_scrolled_window_widget, args, n);
	if (users_scrollbar_widget) {
		n = 0;
		XtSetArg(args[n], XmNtraversalOn,      True); n++;
		XtSetValues(users_scrollbar_widget, args, n);
		XmAddTabGroup(users_scrollbar_widget);
	}
	XmAddTabGroup(users_list_widget);
#endif

	/**********************************************************************/
	/* Group list in scrolled window, titled                              */
	/**********************************************************************/
	
	/* Build group listing title */
	xmstring = XmStringCreate(msg[2], charset);
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                   label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,  users_scrolled_window_widget); n++;
	XtSetArg(args[n], XmNlabelString,                     xmstring); n++;
	XtSetArg(args[n], XmNalignment,             XmALIGNMENT_CENTER); n++;
	list_label_widget = XmCreateLabelGadget(form_widget, "GroupsLabel", args, n);
	XtManageChild(list_label_widget);
	XmStringFree(xmstring);

	n = 0;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                 list_label_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                                 3); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_WIDGET); n++; 
	XtSetArg(args[n], XmNleftWidget,     users_scrolled_window_widget); n++;
	groups_scrolled_window_widget = XtCreateManagedWidget(
			"Groups", xmScrolledWindowWidgetClass, 
			form_widget, args, n);

	n = 0;
	XtSetArg(args[n], XmNselectionPolicy,           XmMULTIPLE_SELECT); n++; 
	groups_list_widget = XtCreateManagedWidget("List",
			xmListWidgetClass, groups_scrolled_window_widget, args, n);

#ifdef TRAVERSAL
	n = 0;
	XtSetArg(args[n], XmNverticalScrollBar, &groups_scrollbar_widget); n++;
	XtGetValues(groups_scrolled_window_widget, args, n);
	if (groups_scrollbar_widget) {
		n = 0;
		XtSetArg(args[n], XmNtraversalOn,      True); n++;
		XtSetValues(groups_scrollbar_widget, args, n);
		
		XmAddTabGroup(groups_scrollbar_widget);
	}
	XmAddTabGroup(groups_list_widget);
#endif

	/* Row column widget to handle this/next buttons */
	n = 0;
	XtSetArg(args[n], XmNpacking,                   XmPACK_COLUMN); n++;
	XtSetArg(args[n], XmNentryClass,    xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNorientation,                XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, groups_scrolled_window_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	this_or_next_widget = XmCreateRowColumn(form_widget, "CurrentFuture",
			args, n);
	XtManageChild(this_or_next_widget);

	/* Button to confirm for this session */
	n = 0;
	xmstring = XmStringCreate(msg[3], charset);
	XtSetArg(args[n], XmNlabelString, xmstring); n++; 
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	confirm_this_session_widget = XmCreateToggleButton(this_or_next_widget,
				     "ToggleButton", args, n);
	XtManageChild(confirm_this_session_widget);
	XmStringFree(xmstring);
#ifdef TRAVERSAL
	XmAddTabGroup(confirm_this_session_widget);
#endif
	
	/* Button to confirm for next session */
	n = 0;
	xmstring = XmStringCreate(msg[4], charset);
	XtSetArg(args[n], XmNlabelString, xmstring); n++; 
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	confirm_next_session_widget = XmCreateToggleButton(
			this_or_next_widget, "ToggleButton", args, n);
	XtManageChild(confirm_next_session_widget);
	XmStringFree(xmstring);
#ifdef TRAVERSAL
	XmAddTabGroup(confirm_next_session_widget);
#endif

	/**********************************************************************/
	/* Create the OK, CANCEL, HELP buttons                                */
	/**********************************************************************/
	CreateThreeButtons (form_widget, this_or_next_widget,
				 &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "audit,UsersGroups");
}                                     

/* Verifies that the information on the screen is valid;
 * displays a confirmation widget to make sure the user really wants to
 * enter the values.
 */
static void 
OKCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	Arg         args[20];
	XmString    xmstring,
				(*selected_users)[],
				(*selected_groups)[];
	Cardinal    user_count,
				group_count;
	int         i,
				j,
				n;
	Widget      w1;
	
	/* Get information on selected items */
	n = 0;
	XtSetArg(args[n], XmNselectedItems,           &selected_users); n++;
	XtSetArg(args[n], XmNselectedItemCount,           &user_count); n++;
	XtGetValues(users_list_widget, args, n);

	n = 0;
	XtSetArg(args[n], XmNselectedItems,          &selected_groups); n++;
	XtSetArg(args[n], XmNselectedItemCount,          &group_count); n++;
	XtGetValues(groups_list_widget, args, n);
	
	/* Allocate memory for valid_usrgrp */
	valid_usrgrp.users = alloc_cw_table(user_count, NGROUPNAME+1);
	valid_usrgrp.nusers = user_count;
	valid_usrgrp.groups = alloc_cw_table(group_count, NGROUPNAME+1);
	valid_usrgrp.ngroups = group_count;
	
	/* Load selected users into valid_usrgrp */
	for (j = 0; j < nusers; j++)
		for (i = 0; i < user_count; i++)
			if (XmStringCompare(users_xmstring[j], (*selected_users)[i])) 
				strcpy(valid_usrgrp.users[i], users[j]);
				
	/* Load selected groups into valid_usrgrp */
	for (j = 0; j < ngroups; j++)
		for (i = 0; i < group_count; i++)
			if (XmStringCompare(groups_xmstring[j], (*selected_groups)[i]))
				strcpy(valid_usrgrp.groups[i], groups[j]);
				
	/* Load information on this or next session into valid_usrgrp */
	valid_usrgrp.future = XmToggleButtonGetState(confirm_next_session_widget);
	valid_usrgrp.this   = XmToggleButtonGetState(confirm_this_session_widget);
		
	/* The user must choose to implement the parameters some time,
	*  or an error will occur 
	*/
	if (! valid_usrgrp.future && !valid_usrgrp.this) {
		ErrorMessageOpen(3020, msg, 15, NULL);
		return;
	}
	
	/* Validate users and groups */
	if (AuditUsersValidate(&valid_usrgrp) 
	||  AuditGroupsValidate(&valid_usrgrp)
	)
		return;
	
	XtSetSensitive(form_widget, False);
	if (! confirmation_open) {
		confirmation_open = TRUE;
		
		/************************************************************/
		/*   Create Confirmation Widget                             */
		/************************************************************/
		n = 0;
		xmstring = XmStringCreate(msg[8], charset);
		XtSetArg(args[n], XmNmessageString,            xmstring); n++;
		XtSetArg(args[n], XmNborderWidth,                     1); n++;
		XtSetArg(args[n], XmNtopAttachment,       XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,      XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNdialogStyle,    XmDIALOG_WORK_AREA); n++;
		XtSetArg(args[n], XmNdialogType,      XmDIALOG_QUESTION); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
	 	XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
#endif
		confirmation_widget = XmCreateMessageBox(mother_form, "Confirm", args, n);
		XmStringFree(xmstring);
		
		XtAddCallback(confirmation_widget, XmNokCallback, 
				ConfirmOKCallback, NULL);
		XtAddCallback(confirmation_widget, XmNcancelCallback,
				ConfirmCancelCallback, NULL);
				
		w1 = XmMessageBoxGetChild(confirmation_widget,
			   XmDIALOG_OK_BUTTON);
		n = 0;
		xmstring = XmStringCreate(msg[10], charset);
		XtSetArg(args[n], XmNlabelString, xmstring); n++;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,     True); n++;
#endif
		XtSetValues(w1, args, n);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(w1);
#endif
		
		w1 = XmMessageBoxGetChild(confirmation_widget,
			   XmDIALOG_CANCEL_BUTTON);
		n = 0;
		xmstring = XmStringCreate(msg[11], charset);
		XtSetArg(args[n], XmNlabelString, xmstring); n++;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,     True); n++;
#endif
		XtSetValues(w1, args, n);
		XmStringFree(xmstring);
			
#ifdef TRAVERSAL
		XmAddTabGroup(w1);
#endif
		
		w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_HELP_BUTTON);
		XtUnmanageChild(w1);
		XtDestroyWidget(w1);
	}

	XtManageChild(confirmation_widget);
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	UsersGroupsClose();  
}


/* Writes info and unmaps the confirmation widget */
static void 
ConfirmOKCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	/* Write users, groups audit data out */
	AuditUsersGroupsPut(&valid_usrgrp);

	/* resensitize form widget, then close out */
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
	UsersGroupsClose();  
}

/* Called if user presses cancel from confirmation widget
 * returns focus to the main Users/Groups widget
 */
static void 
ConfirmCancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
}
#endif /* SEC_BASE */

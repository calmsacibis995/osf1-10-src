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
static char	*sccsid = "@(#)$RCSfile: XRetire.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:45 $";
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
		XRetire.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for retiring a user account
		
	entry points:
		RetireStart()
		RetireOpen()
		RetireClose()
		RetireStop()
*/

/* Common C include files */
#include <sys/types.h>
#ifdef AUX
#include <string.h>
#else
#include <strings.h>
#endif

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

/* ISSO include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XMacros.h"

/* Definitions */

/* External routines */
extern Widget
	CreateSelectionBox();

extern void 
	GetAllUsers();

extern char 
	*strdup(),
	*extract_normal_string();

/* Local routines */
static void 
	NoMatchCallback();

/* Local variables */
static char 
	*selected_user_name,
	**msg_error_cant_read,
	**msg_error_no_match,
	*msg_error_cant_read_text,
	*msg_error_no_match_text;

static Widget
        selection_widget;
        
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (RetireStart, RetireOpen, RetireClose, RetireStop)

/* Makes the widgets */
static void 
MakeWidgets() 
{
	Widget      w;

	/**********************************************************************/
	/* Form                                                               */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "Retire");
	XtAddCallback(form_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "accounts,Retire");
		                         
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_retire_account", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Users list in scrolled window, titled                              */
	/**********************************************************************/
	selection_widget = CreateSelectionBox (form_widget, "UserSelection",
		w, msg_header[1], msg_header[2], True);
	   
	XtAddCallback(selection_widget, XmNnoMatchCallback, 
			NoMatchCallback, NULL);
	XtAddCallback(selection_widget, XmNokCallback, OKCallback, NULL);
	XtAddCallback(selection_widget, XmNcancelCallback, CancelCallback, 
			NULL);
	XtAddCallback(selection_widget, XmNhelpCallback,  
		            HelpDisplayOpen, "accounts,Retire");
}

static int 
LoadVariables()
{
	Cardinal	n;
	Arg		args[10];
	int		i;
	int		nusers;
	char		**users;
	XmString	*users_xmstring;
                              
	/* Load in all users and groups on the system */
	GetAllUsers(&nusers, &users);

	/* Malloc XmString arrays for the lists of all users and groups */
	users_xmstring = (XmString *) Malloc(sizeof(XmString) * nusers);
	if (! users_xmstring)
	        MemoryError();
        
	/* Load XmString arrays with values of valid users and groups */
	for (i = 0; i < nusers; i++) {
		users_xmstring[i] = XmStringCreate(users[i], charset);
		if (! users_xmstring[i])
			MemoryError();
	}
		
	/* Load in user and group values */
	n = 0;
	XtSetArg(args[n], XmNlistItemCount,                     nusers); n++;
	XtSetArg(args[n], XmNlistItems,                 users_xmstring); n++;
	XtSetValues(selection_widget, args, n);

	/* Free various memory */
	if (users) { 
		free_cw_table(users);
		users = NULL;
	}
	if (users_xmstring) {
		for (i = 0; i < nusers; i++) {
		    XmStringFree(users_xmstring[i]);
		}
		free(users_xmstring);
		users_xmstring = NULL;
	}
	
	return (SUCCESS);
}


/* Verifies that the information on the screen is valid;
 * displays a confirmation widget to make sure the user really wants to
 * enter the values.
 */
static void 
OKCallback(w, ptr, cd) 
	Widget                  w; 
	char                    *ptr;
	caddr_t		cd;
{
	XmSelectionBoxCallbackStruct 
		*info = (XmSelectionBoxCallbackStruct *) cd;

	/* Save the user name */
	if (selected_user_name)
		free(selected_user_name);
	selected_user_name = strdup(extract_normal_string (info->value));
	if (! selected_user_name)
		MemoryError();

	/* Create the confirmation box */
	XtSetSensitive (form_widget, False);
	if (! confirmation_open) {
		confirmation_open = TRUE;
		confirmation_widget = CreateConfirmationBox(mother_form,
				msg_header[3]);
		XtAddCallback(confirmation_widget, XmNokCallback,
				ConfirmOKCallback,NULL);
		XtAddCallback(confirmation_widget, XmNcancelCallback,
				ConfirmCancelCallback,NULL);
	}
	else
        	XtManageChild(confirmation_widget);
}

static void
ConfirmOKCallback(w, ptr, info)
	Widget                  w;
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	int ret;

	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
	/* Read the users protected password database entry */
	ret = GetUserInfo(selected_user_name, &pr);
	/* No error if we have a SUCCESS or we are told this is a non ISSO 
	 * accounts because the sysadmin can retire anyone */
	if ( (ret != SUCCESS) && (ret != NOT_ISSO) ) {
		if (! msg_error_cant_read)
			LoadMessage("msg_accounts_cant_read", 
				&msg_error_cant_read, 
				&msg_error_cant_read_text);
		ErrorMessageOpen (-1, msg_error_cant_read, 0, NULL);
		return;
	}

	/* Retire the account */
	pr.prpw.uflg.fg_retired = 1;
	pr.prpw.ufld.fd_retired = 1;

	/* Write the information back */
	ret = XWriteUserInfo(&pr);
	if (ret != SUCCESS)
		return;
	sa_audit_lock(ES_SET_USER_LOCK, pr.prpw.ufld.fd_name);
	audit_subsystem(pr.prpw.ufld.fd_name, 
		"Account is being retired", ET_SYS_ADMIN);

	RetireClose();
}

static void
ConfirmCancelCallback(w, ptr, info)
	Widget                  w;
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
}

static void 
NoMatchCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	if (! msg_error_no_match)
		LoadMessage("msg_accounts_retire_account_no_match", 
			&msg_error_no_match, &msg_error_no_match_text);
	ErrorMessageOpen (-1, msg_error_no_match, 0, NULL);
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	RetireClose();  
}

#endif /* SEC_BASE **/

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
static char	*sccsid = "@(#)$RCSfile: XGetUser.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:24 $";
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
		XGetUser.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for selecting the user name
		
	entry points:
		GetUserStart()
		GetUserOpen()
		GetUserClose()
		GetUserStop()
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
	GetAllUsers(),
	GetAllNonIssoUsers(),
	GetAllIssoUsers();

extern char 
	*strdup(),
	*extract_normal_string();

/* Local routines */
static void 
	NoMatchCallback();

/* Local variables */
static char 
	**msg_error_no_match,
	*msg_error_no_match_text;

static Widget
	selection_widget;

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (GetUserStart, GetUserOpen, GetUserClose, GetUserStop)

/* Makes the widgets for the screen */
static void
MakeWidgets() 
{
	Widget      w;

	/**********************************************************************/
	/* Form                                                               */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "GetUser");
	if (role_program == ISSO)
		XtAddCallback(form_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "accounts,GetIssoName");
	else if (role_program == SYS_ADMIN)
		XtAddCallback(form_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "accounts,GetSysName");
	else 
		XtAddCallback(form_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "accounts,GetName");
		                         
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_get_user", &msg_header, 
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
	if (role_program == ISSO)
		XtAddCallback(selection_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "accounts,GetIssoName");
	else if (role_program == SYS_ADMIN)
		XtAddCallback(selection_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "accounts,GetSysName");
	else 
		XtAddCallback(selection_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "accounts,GetName");
}		                             

static int
LoadVariables ()
{
	Cardinal	n;
	Arg		args[5];
	int		i;
	XmString	xmstring;
	char		**users;
	int 		nusers;			/* total number of users */
	XmString	*users_xmstring;	/* XmStrings for all users */
	                          
	/* Load in all users and groups on the system */
	nusers = 0;
	if (role_program == ISSO)
		GetAllNonIssoUsers(&nusers, &users);
	else {
	if (role_program == SYS_ADMIN)
		GetAllIssoUsers(&nusers, &users);
	else
		GetAllUsers (&nusers, &users);
	}

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
	if (chosen_user_name) {
		xmstring = XmStringCreate (chosen_user_name, charset);
		if (! xmstring)
			MemoryError();
		XtSetArg(args[n], XmNtextString,      xmstring); n++;
	}
	XtSetValues(selection_widget, args, n);

	/* Free the memory */
	if (users) {
		for (i = 0; i < nusers; i++) {
		    free(users[i]);
		    XmStringFree(users_xmstring[i]);
		}
		free(users);
		free(users_xmstring);
	}

	/* If the next line is if (xmstring) and this is the first time in 
	 * we have a lovely memory bug hence the use of chosen_user_name */
	if (chosen_user_name)
		XmStringFree(xmstring);

	return (SUCCESS);
}


/* Store the name in global memory for access to all screens */
static void 
OKCallback(w, ptr, cd) 
	Widget                  w; 
	char                    *ptr;
	caddr_t		cd;
{
	XmSelectionBoxCallbackStruct 
		*info = (XmSelectionBoxCallbackStruct *) cd;

	if (chosen_user_name)
		free (chosen_user_name);
	chosen_user_name = strdup(extract_normal_string (info->value));
	if (! chosen_user_name)
		MemoryError();
	GetUserClose();  
}

static void 
NoMatchCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	if (! msg_error_no_match)
		LoadMessage("msg_accounts_get_user_no_match", 
			&msg_error_no_match, &msg_error_no_match_text);
	ErrorMessageOpen (-1, msg_error_no_match, 0, NULL);
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	GetUserClose();  
}
#endif /* SEC_BASE **/

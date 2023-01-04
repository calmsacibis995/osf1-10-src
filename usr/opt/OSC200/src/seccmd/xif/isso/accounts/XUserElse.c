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
static char	*sccsid = "@(#)$RCSfile: XUserElse.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:13 $";
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
		XUserLogin.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Login Parameters for user account
		
	entry points:
		UserLoginStart()
		UserLoginOpen()
		UserLoginClose()
		UserLoginStop()

*/

/* Common C include files */
#include <sys/types.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XMacros.h"

/* External routines */
extern void ErrorMessageOpen();

/* Local variables */
static char
	**msg_error,
	**msg_user_login_value,
	**msg_user_login_toggle,
	*msg_user_login_toggle_text,
	*msg_user_login_value_text,
	*msg_error_text;

/* Definitions */
#define NUM_USER_LOGIN_VALUES		2
#define NUM_USER_LOGIN_TOGGLES		1

static int user_login_text_col[NUM_USER_LOGIN_VALUES] = 
	{3,3,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (UserLoginStart, UserLoginOpen, UserLoginClose,
	UserLoginStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
		    work_area1_widget,
		    work_area2_frame,
		    work_area2_widget,
		    ok_button,
		    cancel_button,
		    help_button,
		    w,w1;
	int         i;
	Dimension   max_label_width;


	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "UserLogin");
	XtAddCallback(form_widget, XmNhelpCallback,
					 HelpDisplayOpen, "accounts,UserLogin");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_user_login", 
			&msg_header, &msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* User name                                                          */
	/**********************************************************************/
	user_name_widget = CreateUserName (form_widget, w);

	/**********************************************************************/
	/* First create the form work area widget                             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	work_area1_frame  = CreateFrame(form_widget, user_name_widget, 
				True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	max_label_width = (Dimension) 0;

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	def_value     	 = MallocInt(NUM_USER_LOGIN_VALUES);
	value     	 = MallocInt(NUM_USER_LOGIN_VALUES);
	value_state      = MallocChar(NUM_USER_LOGIN_VALUES);
	toggle_state     = MallocChar(NUM_USER_LOGIN_TOGGLES);
	def_toggle_state = MallocChar(NUM_USER_LOGIN_TOGGLES);
	text_default_widget = MallocWidget(NUM_USER_LOGIN_VALUES);
	text_widget 	 = MallocWidget(NUM_USER_LOGIN_VALUES);
	yes_widget 	 = MallocWidget(NUM_USER_LOGIN_TOGGLES);
	default_widget   = MallocWidget(NUM_USER_LOGIN_TOGGLES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_user_login_value)
		LoadMessage("msg_accounts_user_login_value", 
			&msg_user_login_value, &msg_user_login_value_text);
	CreateItemsTextD(work_area1_widget, 0, NUM_USER_LOGIN_VALUES,
		msg_user_login_value, &max_label_width, user_login_text_col, 
	text_widget, text_default_widget);

	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_USER_LOGIN_VALUES; i++) {
		XtAddCallback (text_widget[i], XmNvalueChangedCallback, 
				 TextCallback, i);
		XtAddCallback (text_default_widget[i], XmNvalueChangedCallback, 
				 TextDefaultCallback, i);
	}

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], 
		work_area1_frame, True, user_name_widget);

	/**********************************************************************/
	/*
	/* Create all the Toggle button widgets                               */
	/*
	/**********************************************************************/
	work_area2_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/*
	max_label_width = (Dimension) 0;
	*/
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	/* There is only one - the lock widget, we do not use the default */
	if (! msg_user_login_toggle)
		LoadMessage("msg_accounts_user_login_toggle", 
			&msg_user_login_toggle, &msg_user_login_toggle_text);
	CreateItemsYND(work_area2_widget, 0, NUM_USER_LOGIN_TOGGLES,
		msg_user_login_toggle, &max_label_width, yes_widget,
		default_widget);

	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_USER_LOGIN_TOGGLES; i++) {
		XtAddCallback (yes_widget[i], XmNvalueChangedCallback, 
				OnCallback, i);
		XtAddCallback (default_widget[i], XmNvalueChangedCallback, 
				DefaultCallback, i);
	}

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader (form_widget, msg_header[2], work_area1_frame, 
		False, w);

	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons			     */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
			     &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "accounts,UserLogin");
}


CREATE_TEXT_CALLBACKS (def_value)
CREATE_ON_CALLBACKS (def_toggle_state, False, NUM_USER_LOGIN_TOGGLES)
CREATE_CALLBACKS (msg_header[3],  UserLoginClose)

static int
LoadVariables ()
{
	int i;
	int ret;
	struct pr_passwd *prpwd = &pr.prpw;

	ret = XGetUserInfo(chosen_user_name, &pr);
	if (ret)
		return (ret);

	/* First get the text values */
	if (prpwd->uflg.fg_max_tries) {
		value      [0] = (int) prpwd->ufld.fd_max_tries;
		value_state[0] = NO_CHAR;
	}
	else 
		value_state[0] = YES_CHAR;

	if (prpwd->uflg.fg_nice) {
		value      [1] = prpwd->ufld.fd_nice;
		value_state[1] = NO_CHAR;
	}
	else 
		value_state[1] = YES_CHAR;

	if (prpwd->sflg.fg_max_tries)
		def_value      [0] = (int) prpwd->sfld.fd_max_tries;
	else 	def_value      [0] = 0;

	if (prpwd->sflg.fg_nice)
		def_value      [1] = (int) prpwd->sfld.fd_nice;
	else 	def_value      [1] = 0;

	/* Toggles */
	if (prpwd->uflg.fg_lock)
		if (prpwd->ufld.fd_lock)
			toggle_state[0] = YES_CHAR;
		else	toggle_state[0] = NO_CHAR;
	else		toggle_state[0] = DEFAULT_CHAR;

	if (prpwd->sflg.fg_lock && prpwd->sfld.fd_lock)
		def_toggle_state[0] = YES_CHAR;
	else	def_toggle_state[0] = NO_CHAR;

	in_change_mode = False;
	for (i=0; i<NUM_USER_LOGIN_VALUES; i++) {
		SetTextD(value[i], value_state[i], def_value[i], 
			text_widget[i], text_default_widget[i]);
	}
	in_change_mode = True;

	for (i=0; i<NUM_USER_LOGIN_TOGGLES; i++) {
		SetYND(toggle_state[i], def_toggle_state[i],
			yes_widget[i], default_widget[i]);
	}

	/* Set the user name */
	SetUserName(user_name_widget, pr.prpw.ufld.fd_name);
	return (SUCCESS);
}

static int
ValidateEntries ()
{
	char *buf;
	struct pr_passwd *prpwd = &pr.prpw;
	int i;
	int nerrs = 0;

	/* Check the text fields */
	if (XmToggleButtonGadgetGetState(text_default_widget[0])) {
		prpwd->uflg.fg_max_tries = 0;
		prpwd->ufld.fd_max_tries = (ushort) 1;
	}
	else {
		prpwd->uflg.fg_max_tries = 1;
		buf = XmTextGetString(text_widget[0]);
		i = atoi (buf);
		if (i >= 0 && i <= 999)
			prpwd->ufld.fd_max_tries = (ushort) i;
		else
			nerrs ++;

		XtFree(buf);
	}

	if (XmToggleButtonGadgetGetState(text_default_widget[1])) {
		prpwd->uflg.fg_nice = 0;
		prpwd->ufld.fd_nice = 1;
	}
	else {
		prpwd->uflg.fg_nice = 1;
		buf = XmTextGetString(text_widget[1]);
		i = atoi (buf);
		if (i >= -99 && i <= 999)
			prpwd->ufld.fd_nice = i;
		else
			nerrs ++;

		XtFree(buf);
	}

	/* Display an error if not all numbers
	 * are non-zero integers
	 */
	if (nerrs) {
		if (! msg_error)
			LoadMessage("msg_accounts_user_login_error_1",
				&msg_error, &msg_error_text);
		ErrorMessageOpen(-1, msg_error, 0, NULL);
		return (FAILURE);
	}

	return (SUCCESS);
}

static int
WriteInformation ()
{
	struct pr_passwd *prpwd = &pr.prpw;

	/* Now read the toggles */
	if (XmToggleButtonGadgetGetState(default_widget[0])) {
		prpwd->uflg.fg_lock = 0;
		prpwd->ufld.fd_lock = 1;
	}
	else {
		prpwd->uflg.fg_lock = 1;
		if (XmToggleButtonGadgetGetState(yes_widget[0]))
			 prpwd->ufld.fd_lock = 1;
		else     prpwd->ufld.fd_lock = 0;
	}

	return (XWriteUserInfo (&pr));
}

#endif /* SEC_BASE **/

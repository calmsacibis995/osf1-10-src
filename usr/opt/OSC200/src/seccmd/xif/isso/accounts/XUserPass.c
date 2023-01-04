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
static char	*sccsid = "@(#)$RCSfile: XUserPass.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:20 $";
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
		XUserPass.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Password Parameters for user account
		
	entry points:
		UserPassStart()
		UserPassOpen()
		UserPassClose()
		UserPassStop()

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
extern int
	TimeToWeeks(),
	TimeToDays();

/* Local variables */
static char
	**msg_user_pass_value,
	**msg_user_pass_toggle,
	**msg_error_1,
	**msg_error_2,
	**msg_error_3,
	**msg_user_pass_value_text,
	**msg_user_pass_toggle_text,
	*msg_error_text_1,
	*msg_error_text_2,
	*msg_error_text_3;

/* Local routines */

/* Definitions */
#define NUM_USER_PASSWORD_VALUES	4
#define NUM_USER_PASSWORD_TOGGLES	6

/* Define the size of the text widgets */ 
static int user_pass_text_col [NUM_USER_PASSWORD_VALUES] = 
	{4,4,4,4,}; 

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (UserPassStart, UserPassOpen, UserPassClose, 
			UserPassStop)

static void 
MakeWidgets() 
{
	Widget      	work_area1_frame,
			work_area1_widget,
			work_area2_frame,
			work_area2_widget,
			ok_button,
			cancel_button,
			help_button,
			w,w1;
	int         	i;
	Dimension   	max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "UserPass");
	XtAddCallback(form_widget, XmNhelpCallback,
			HelpDisplayOpen, "accounts,UserPass");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage("msg_accounts_user_password", 
			&msg_header, &msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* User name                                                          */
	/**********************************************************************/
	user_name_widget = CreateUserName (form_widget, w);

	/**********************************************************************/
	/*
	/* Create all the value setting widgets                               */
	/*
	/**********************************************************************/

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
	def_value     	 = MallocInt(NUM_USER_PASSWORD_VALUES);
	value     	 = MallocInt(NUM_USER_PASSWORD_VALUES);
	value_state      = MallocChar(NUM_USER_PASSWORD_VALUES);
	toggle_state     = MallocChar(NUM_USER_PASSWORD_TOGGLES);
	def_toggle_state = MallocChar(NUM_USER_PASSWORD_TOGGLES);
	text_widget 	 = MallocWidget(NUM_USER_PASSWORD_VALUES);
	text_default_widget = MallocWidget(NUM_USER_PASSWORD_VALUES);
	yes_widget 	 = MallocWidget(NUM_USER_PASSWORD_TOGGLES);
	default_widget   = MallocWidget(NUM_USER_PASSWORD_TOGGLES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_user_pass_value)
		LoadMessage("msg_accounts_user_password_value", 
			&msg_user_pass_value, &msg_user_pass_value_text);
	CreateItemsTextD(work_area1_widget, 0, NUM_USER_PASSWORD_VALUES,
		msg_user_pass_value, &max_label_width, user_pass_text_col, 
		text_widget, text_default_widget);

	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_USER_PASSWORD_VALUES; i++) {
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
	if (! msg_user_pass_toggle)
		LoadMessage("msg_accounts_user_password_toggle_b1", 
			&msg_user_pass_toggle, &msg_user_pass_toggle_text);
	CreateItemsYND(work_area2_widget, 0, NUM_USER_PASSWORD_TOGGLES,
		msg_user_pass_toggle, &max_label_width, yes_widget, 
		default_widget);

	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_USER_PASSWORD_TOGGLES; i++) {
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
	/* Create the OK, Cancel and Help buttons
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
				&ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
			HelpDisplayOpen, "accounts,UserPass");
}


CREATE_TEXT_CALLBACKS(def_value)
CREATE_ON_CALLBACKS(def_toggle_state, False, NUM_USER_PASSWORD_TOGGLES)
CREATE_CALLBACKS(msg_header[3], UserPassClose)

static int
LoadVariables()
{
	struct pr_passwd *prpwd = &pr.prpw;
	int i;
	int ret;

	/* Load the password value defaults */
	ret = XGetUserInfo(chosen_user_name, &pr);
	if (ret)
		return (ret);

	/* First get the text values */
	if (prpwd->uflg.fg_min) {
		value      [0] = TimeToWeeks(prpwd->ufld.fd_min);
		value_state[0] = NO_CHAR;
	}
	else 
		value_state[0] = YES_CHAR;

	if (prpwd->uflg.fg_expire) {
		value      [1] = TimeToWeeks(prpwd->ufld.fd_expire);
		value_state[1] = NO_CHAR;
	}
	else 
		value_state[1] = YES_CHAR;

	if (prpwd->uflg.fg_lifetime) {
		value      [2] = TimeToWeeks(prpwd->ufld.fd_lifetime);
		value_state[2] = NO_CHAR;
	}
	else 
		value_state[2] = YES_CHAR;

	if (prpwd->uflg.fg_maxlen) {
		value      [3] = prpwd->ufld.fd_maxlen;
		value_state[3] = NO_CHAR;
	}
	else 
		value_state[3] = YES_CHAR;

	/* Toggles */
	/* Note that these are backwards, because we use the term
	   "password required" which is opposite of null password
	*/
	if (prpwd->uflg.fg_nullpw)
		if (prpwd->ufld.fd_nullpw)
			toggle_state[0] = NO_CHAR;
		else    toggle_state[0] = YES_CHAR;
	else        toggle_state[0] = DEFAULT_CHAR;

	if (prpwd->uflg.fg_pick_pwd)
		if (prpwd->ufld.fd_pick_pwd)
			toggle_state[1] = YES_CHAR;
		else    toggle_state[1] = NO_CHAR;
	else        toggle_state[1] = DEFAULT_CHAR;

	if (prpwd->uflg.fg_gen_pwd)
		if (prpwd->ufld.fd_gen_pwd)
			toggle_state[2] = YES_CHAR;
		else    toggle_state[2] = NO_CHAR;
	else        toggle_state[2] = DEFAULT_CHAR;

	if (prpwd->uflg.fg_gen_chars)
		if (prpwd->ufld.fd_gen_chars)
			toggle_state[3] = YES_CHAR;
		else    toggle_state[3] = NO_CHAR;
	else        toggle_state[3] = DEFAULT_CHAR;

	if (prpwd->uflg.fg_gen_letters)
		if (prpwd->ufld.fd_gen_letters)
			toggle_state[4] = YES_CHAR;
		else    toggle_state[4] = NO_CHAR;
	else        toggle_state[4] = DEFAULT_CHAR;

	if (prpwd->uflg.fg_restrict)
		if (prpwd->ufld.fd_restrict)
			toggle_state[5] = YES_CHAR;
		else    toggle_state[5] = NO_CHAR;
	else        toggle_state[5] = DEFAULT_CHAR;

	/* Get the system default values */
	if (prpwd->sflg.fg_min)
		def_value    [0] = TimeToWeeks(prpwd->sfld.fd_min);
	else 
		def_value    [0] = 0;

	if (prpwd->sflg.fg_expire)
		def_value    [1] = TimeToWeeks(prpwd->sfld.fd_expire);
	else 
		def_value    [1] = 0;

	if (prpwd->sflg.fg_lifetime)
		def_value    [2] = TimeToWeeks(prpwd->sfld.fd_lifetime);
	else 
		def_value    [2] = 0;

	if (prpwd->sflg.fg_maxlen)
		def_value    [3] = prpwd->sfld.fd_maxlen;
	else 
		def_value    [3] = 0;

	/* Toggles */
	/* Note these are other way round because we want password required */
	if (prpwd->sflg.fg_nullpw && prpwd->sfld.fd_nullpw)
		   def_toggle_state[0] = NO_CHAR;
	else   def_toggle_state[0] = YES_CHAR;

	if (prpwd->sflg.fg_pick_pwd && prpwd->sfld.fd_pick_pwd)
		def_toggle_state[1] = YES_CHAR;
	else    def_toggle_state[1] = NO_CHAR;

	if (prpwd->sflg.fg_gen_pwd && prpwd->sfld.fd_gen_pwd)
		def_toggle_state[2] = YES_CHAR;
	else    def_toggle_state[2] = NO_CHAR;

	if (prpwd->sflg.fg_gen_chars && prpwd->sfld.fd_gen_chars)
		def_toggle_state[3] = YES_CHAR;
	else    def_toggle_state[3] = NO_CHAR;

	if (prpwd->sflg.fg_gen_letters && prpwd->sfld.fd_gen_letters)
		def_toggle_state[4] = YES_CHAR;
	else    def_toggle_state[4] = NO_CHAR;

	if (prpwd->sflg.fg_restrict && prpwd->sfld.fd_restrict)
		def_toggle_state[5] = YES_CHAR;
	else    def_toggle_state[5] = NO_CHAR;

	in_change_mode = False; /* Avoid the callbacks */

	for (i=0; i<NUM_USER_PASSWORD_VALUES; i++) {
		SetTextD(value[i], value_state[i], def_value[i], 
				text_widget[i], text_default_widget[i]);
	}
	
	for (i=0; i<NUM_USER_PASSWORD_TOGGLES; i++) {
		SetYND(toggle_state[i], def_toggle_state[i], yes_widget[i], 
			default_widget[i]);
	}

	SetUserName(user_name_widget, pr.prpw.ufld.fd_name);
	in_change_mode = True;
	return (ret);
}

/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	char *buf;
	struct pr_passwd *prpwd = &pr.prpw;
	int i;
	int nerrs = 0;

	/* Check the text fields */
	if (XmToggleButtonGadgetGetState(text_default_widget[0])) {
		prpwd->uflg.fg_min = 0;
		if (prpwd->sflg.fg_min)
			prpwd->ufld.fd_min = prpwd->sfld.fd_min; /* for check */
		else
			prpwd->ufld.fd_min = 0; /* Used for check */
	}
	else {
		prpwd->uflg.fg_min = 1;
		buf = XmTextGetString(text_widget[0]);
		i = atoi (buf);
		if (i >= 0 && i <= 1103)
			prpwd->ufld.fd_min = (time_t) (i * SECINWEEK);
		else
			nerrs ++;

		XtFree(buf);
	}

	if (XmToggleButtonGadgetGetState(text_default_widget[1])) {
		prpwd->uflg.fg_expire = 0;
		if (prpwd->sflg.fg_expire)
			prpwd->ufld.fd_expire = prpwd->sfld.fd_expire; 
				/* for check */
		else
			prpwd->ufld.fd_expire = 0; /* Used for check */
	}
	else {
		buf = XmTextGetString(text_widget[1]);
		prpwd->uflg.fg_expire = 1;
		i = atoi (buf);
		if (i >= 0 && i <= 1103)
			prpwd->ufld.fd_expire = (time_t) (i * SECINWEEK);
		else
			nerrs ++;
		XtFree(buf);
	}

	if (XmToggleButtonGadgetGetState(text_default_widget[2])) {
		prpwd->uflg.fg_lifetime = 0;
		if (prpwd->sflg.fg_lifetime)
			prpwd->ufld.fd_lifetime = prpwd->sfld.fd_lifetime; 
				/* for check */
		else
			prpwd->ufld.fd_lifetime = 0; /* Used for check */
	}
	else {
		buf = XmTextGetString(text_widget[2]);
		prpwd->uflg.fg_lifetime = 1;
		i = atoi (buf);
		if (i >= 0 && i <= 1103)
			prpwd->ufld.fd_lifetime = (time_t) (i * SECINWEEK);
		else
			nerrs ++;
		XtFree(buf);
	}

	if (XmToggleButtonGadgetGetState(text_default_widget[3])) {
		prpwd->uflg.fg_maxlen = 0;
		prpwd->ufld.fd_maxlen = 1;
	}
	else {
		buf = XmTextGetString(text_widget[3]);
		prpwd->uflg.fg_maxlen = 1;
		i = atoi (buf);
		if (i >= 0 && i <= AUTH_MAX_PASSWD_LENGTH)
			prpwd->ufld.fd_maxlen = i;
		else
			nerrs ++;
		XtFree(buf);
	}

	/* Display an error if not all numbers
	 * are non-zero integers
	 */
	if (nerrs) {
		if (! msg_error_1)
			LoadMessage("msg_accounts_user_password_error_1",
				&msg_error_1, &msg_error_text_1);
		ErrorMessageOpen(-1, msg_error_1, 0, NULL);
		return (FAILURE);
	}

	/* Must check for values set to 0 */
	if ( ( (prpwd->ufld.fd_lifetime < prpwd->ufld.fd_expire ) &&
	       (prpwd->ufld.fd_lifetime != (time_t) 0) ) ||
	     ( (prpwd->ufld.fd_expire   < prpwd->ufld.fd_min) &&
	       (prpwd->ufld.fd_expire   != (time_t) 0) ) ) {
		if (! msg_error_2)
			LoadMessage("msg_accounts_user_password_error_2",
				&msg_error_2, &msg_error_text_2);
		ErrorMessageOpen(-1, msg_error_2, 0, NULL);
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
		prpwd->uflg.fg_nullpw = 0;
		prpwd->ufld.fd_nullpw = 1;
	}
	else {
		prpwd->uflg.fg_nullpw = 1;
	/* Note they are the other way round */
		if (XmToggleButtonGadgetGetState(yes_widget[0]))
			prpwd->ufld.fd_nullpw = 0;
		else    prpwd->ufld.fd_nullpw = 1;
	}

	if (XmToggleButtonGadgetGetState(default_widget[1])) {
		prpwd->uflg.fg_pick_pwd = 0;
		prpwd->ufld.fd_pick_pwd = 1;
	}
	else {
		prpwd->uflg.fg_pick_pwd = 1;
		if (XmToggleButtonGadgetGetState(yes_widget[1]))
			prpwd->ufld.fd_pick_pwd = 1;
		else    prpwd->ufld.fd_pick_pwd = 0;
	}

	if (XmToggleButtonGadgetGetState(default_widget[2])) {
		prpwd->uflg.fg_gen_pwd = 0;
		prpwd->ufld.fd_gen_pwd = 1;
	}
	else {
		prpwd->uflg.fg_gen_pwd = 1;
		if (XmToggleButtonGadgetGetState(yes_widget[2]))
			prpwd->ufld.fd_gen_pwd = 1;
		else    prpwd->ufld.fd_gen_pwd = 0;
	}

	if (XmToggleButtonGadgetGetState(default_widget[3])) {
		prpwd->uflg.fg_gen_chars = 0;
		prpwd->ufld.fd_gen_chars = 1;
	}
	else {
		prpwd->uflg.fg_gen_chars = 1;
		if (XmToggleButtonGadgetGetState(yes_widget[3]))
			prpwd->ufld.fd_gen_chars = 1;
		else    prpwd->ufld.fd_gen_chars = 0;
	}

	if (XmToggleButtonGadgetGetState(default_widget[4])) {
		prpwd->uflg.fg_gen_letters = 0;
		prpwd->ufld.fd_gen_letters = 1;
	}
	else {
		prpwd->uflg.fg_gen_letters = 1;
		if (XmToggleButtonGadgetGetState(yes_widget[4]))
			prpwd->ufld.fd_gen_letters = 1;
		else    prpwd->ufld.fd_gen_letters = 0;
	}

	if (XmToggleButtonGadgetGetState(default_widget[5])) {
		prpwd->uflg.fg_restrict = 0;
		prpwd->ufld.fd_restrict = 1;
	}
	else {
		prpwd->uflg.fg_restrict = 1;
		if (XmToggleButtonGadgetGetState(yes_widget[5]))
			prpwd->ufld.fd_restrict = 1;
		else    prpwd->ufld.fd_restrict = 0;
	}
	return (XWriteUserInfo (&pr));
}

#endif /* SEC_BASE **/

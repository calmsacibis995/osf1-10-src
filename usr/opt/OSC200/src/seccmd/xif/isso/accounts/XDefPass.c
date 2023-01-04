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
static char	*sccsid = "@(#)$RCSfile: XDefPass.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:20 $";
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
		XDefPass.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Default Password Parameters
		
	entry points:
		DefPassStart()
		DefPassOpen()
		DefPassClose()
		DefPassStop()

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
	**msg_system_pass_value,
	**msg_system_pass_toggle,
	**msg_error_1,
	**msg_error_2,
	**msg_error_3,
	*msg_system_pass_value_text,
	*msg_system_pass_toggle_text,
	*msg_error_text_1,
	*msg_error_text_2,
	*msg_error_text_3;

/* Definitions */

#define NUM_SYSTEM_PASSWORD_VALUES	5
#define NUM_SYSTEM_PASSWORD_TOGGLES	6

static int system_pass_text_col[NUM_SYSTEM_PASSWORD_VALUES] = 
	{4,4,4,4,4,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DefPassStart, DefPassOpen, DefPassClose,
			DefPassStop)

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
		    title,
		    w,w1;
	int         i;
	Dimension   max_label_width;


	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DefPass");
	XtAddCallback(form_widget, XmNhelpCallback,
					HelpDisplayOpen, "accounts,DefPass");
	
	/**********************************************************************/
	/* title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_default_password", &msg_header, 
			&msg_header_text);
	title = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* First create the form work area widget                             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	work_area1_frame  = CreateFrame(form_widget, title, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	max_label_width = (Dimension) 120;

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	value     	 = MallocInt(NUM_SYSTEM_PASSWORD_VALUES);
	toggle_state     = MallocChar(NUM_SYSTEM_PASSWORD_TOGGLES);
	text_widget 	 = MallocWidget(NUM_SYSTEM_PASSWORD_VALUES);
	yes_widget 	 = MallocWidget(NUM_SYSTEM_PASSWORD_TOGGLES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_system_pass_value)
		LoadMessage ("msg_accounts_default_password_value_sw_3000", 
			&msg_system_pass_value, &msg_system_pass_value_text);
	CreateItemsText(work_area1_widget, 0, NUM_SYSTEM_PASSWORD_VALUES,
		msg_system_pass_value, &max_label_width, system_pass_text_col,
		text_widget);

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], work_area1_frame, 
	True, title);

	/**********************************************************************/
	/*
	/* Create all the Toggle button widgets                               */
	/*
	/**********************************************************************/
	work_area2_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/* Make same size
	max_label_width = 0;
	*/
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_system_pass_toggle)
		LoadMessage ("msg_accounts_default_password_toggle_b1", 
		&msg_system_pass_toggle, &msg_system_pass_toggle_text);
	CreateItemsYN(work_area2_widget, 0, NUM_SYSTEM_PASSWORD_TOGGLES,
		msg_system_pass_toggle, &max_label_width, yes_widget);

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader (form_widget, msg_header[2], work_area1_frame, 
		False, w);

	/*********************************************************************/
	/* RowColumn widget for action buttons                               */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
			&ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
			HelpDisplayOpen, "accounts,DefPass");
}

/* We only use OK, CANCEL and HELP callbacks */
CREATE_TEXT_CALLBACKS(value)
CREATE_CALLBACKS(msg_header[3], DefPassClose)

static int
LoadVariables()
{
	int i;
	int ret;
	char buffer[20];
	struct pr_default *df = &sd.df;

	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);

	/* First get the text values */
	if (df->prg.fg_min)
		value    [0] = TimeToWeeks(df->prd.fd_min);
	else 	value    [0] = 0;

	if (df->prg.fg_expire)
		value    [1] = TimeToWeeks(df->prd.fd_expire);
	else	value    [1] = 0;

	if (df->prg.fg_lifetime)
		value    [2] = TimeToWeeks(df->prd.fd_lifetime);
	else 	value    [2] = 0;

	if (df->sflg.fg_pw_expire_warning)
		value    [3] = TimeToDays (df->sfld.fd_pw_expire_warning);
	else 	value    [3] = 0;

	if (df->prg.fg_maxlen)
		value    [4] = (int) df->prd.fd_maxlen;
	else 	value    [4] = 0;

	/* Toggles */
	/* Note they are the other way round */
	if (df->prg.fg_nullpw && df->prd.fd_nullpw)
		toggle_state[0] = NO_CHAR;
	else    toggle_state[0] = YES_CHAR;

	if (df->prg.fg_pick_pwd && df->prd.fd_pick_pwd)
		toggle_state[1] = YES_CHAR;
	else    toggle_state[1] = NO_CHAR;

	if (df->prg.fg_gen_pwd && df->prd.fd_gen_pwd)
		toggle_state[2] = YES_CHAR;
	else    toggle_state[2] = NO_CHAR;

	if (df->prg.fg_gen_chars && df->prd.fd_gen_chars)
		toggle_state[3] = YES_CHAR;
	else    toggle_state[3] = NO_CHAR;

	if (df->prg.fg_gen_letters && df->prd.fd_gen_letters)
		toggle_state[4] = YES_CHAR;
	else    toggle_state[4] = NO_CHAR;

	if (df->prg.fg_restrict && df->prd.fd_restrict)
		toggle_state[5] = YES_CHAR;
	else    toggle_state[5] = NO_CHAR;

	/* Load the password value defaults */
	for (i=0; i<NUM_SYSTEM_PASSWORD_VALUES; i++) {
		sprintf (buffer, "%d", value[i]);
		XmTextSetString (text_widget[i], buffer);
	}

	for (i=0; i<NUM_SYSTEM_PASSWORD_TOGGLES; i++) {
		SetToggle(yes_widget[i], (toggle_state[i] == YES_CHAR));
	}

	return (ret);
}

/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	char *buf;
	struct pr_default *sdef = &sd.df;
	int i;
	int nerrs = 0;

	/* Check the text fields */
	sdef->prg.fg_min = 1;
	buf = XmTextGetString(text_widget[0]);
	i = atoi (buf);
	if (i >= 0 && i <= 1103)
		sdef->prd.fd_min = (time_t) (i * SECINWEEK);
	else
		nerrs ++;
	XtFree(buf);

	buf = XmTextGetString(text_widget[1]);
	sdef->prg.fg_expire = 1;
	i = atoi (buf);
	if (i >= 0 && i <= 1103)
		sdef->prd.fd_expire = (time_t) (i * SECINWEEK);
	else
		nerrs ++;
	XtFree(buf);

	buf = XmTextGetString(text_widget[2]);
	sdef->prg.fg_lifetime = 1;
	i = atoi (buf);
	if (i >= 0 && i <= 1103)
		sdef->prd.fd_lifetime = (time_t) (i * SECINWEEK);
	else
		nerrs ++;
	XtFree(buf);

	buf = XmTextGetString(text_widget[3]);
	sdef->sflg.fg_pw_expire_warning = 1;
	i = atoi (buf);
	if (i >= 0 && i <= 999)
		 sdef->sfld.fd_pw_expire_warning = (time_t) (i * SECINDAY);
	else
		nerrs ++;
	XtFree(buf);

	buf = XmTextGetString(text_widget[4]);
	sdef->prg.fg_maxlen = 1;
	i = atoi (buf);
	if (i >= 0 && i <= AUTH_MAX_PASSWD_LENGTH)
		sdef->prd.fd_maxlen = i;
	else
		nerrs ++;
	XtFree(buf);

	/* Display an error if not all numbers
	 * are non-zero integers
	 */
	if (nerrs) {
		if (! msg_error_1)
			LoadMessage ("msg_accounts_default_password_error_1",
				&msg_error_1, &msg_error_text_1);
		ErrorMessageOpen(-1, msg_error_1, 1, NULL);
		return (FAILURE);
	}

	/* Error if lifetime < expiration
	 * or expiry time < min change time ,
	 * however must check for 0 values
	 */
	if ( (  (sdef->prd.fd_lifetime < sdef->prd.fd_expire ) &&
		(sdef->prd.fd_lifetime != (time_t) 0) ) ||
	     (	(sdef->prd.fd_expire   < sdef->prd.fd_min)  &&
		(sdef->prd.fd_expire   != (time_t) 0) ) ) {
		if (! msg_error_2)
			LoadMessage ("msg_accounts_default_password_error_2",
				&msg_error_2, &msg_error_text_2);
		ErrorMessageOpen(-1, msg_error_2, 0, NULL);
		return (FAILURE);
	}

	if ( (sdef->sfld.fd_pw_expire_warning > sdef->prd.fd_expire) &&
	     (sdef->prd.fd_expire != (time_t) 0) ) {
		if (! msg_error_3)
			LoadMessage ("msg_accounts_default_password_error_3",
				&msg_error_3, &msg_error_text_3);
		ErrorMessageOpen(-1, msg_error_3, 0, NULL);
		return (FAILURE);
	}
	return (SUCCESS);
}

static int
WriteInformation ()
{
	struct pr_default *sdef = &sd.df;

	/* Toggles */
	sdef->prg.fg_nullpw = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[0]))
		sdef->prd.fd_nullpw = 0;
	else	sdef->prd.fd_nullpw = 1;

	sdef->prg.fg_pick_pwd = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[1]))
		sdef->prd.fd_pick_pwd = 1;
	else	sdef->prd.fd_pick_pwd = 0;

	sdef->prg.fg_gen_pwd = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[2]))
		sdef->prd.fd_gen_pwd = 1;
	else  	sdef->prd.fd_gen_pwd = 0;

	sdef->prg.fg_gen_chars = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[3]))
		sdef->prd.fd_gen_chars = 1;
	else  	sdef->prd.fd_gen_chars = 0;

	sdef->prg.fg_gen_letters = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[4]))
		sdef->prd.fd_gen_letters = 1;
	else  	sdef->prd.fd_gen_letters = 0;

	sdef->prg.fg_restrict = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[5]))
		sdef->prd.fd_restrict = 1;
	else	sdef->prd.fd_restrict = 0;

	return (XWriteSystemInfo (&sd));
}

#endif /* SEC_BASE **/

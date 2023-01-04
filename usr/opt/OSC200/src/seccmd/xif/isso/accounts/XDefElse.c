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
static char	*sccsid = "@(#)$RCSfile: XDefElse.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:15 $";
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
		XDefLogin.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Default Login Parameters
		
	entry points:
		DefLoginStart()
		DefLoginOpen()
		DefLoginClose()
		DefLoginStop()

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
	**msg_system_login_value,
	**msg_system_login_toggle,
	**msg_error,
	*msg_system_login_value_text,
	*msg_system_login_toggle_text,
	*msg_error_text;

/* Definitions */
#define NUM_SYSTEM_LOGIN_VALUES		3
#define NUM_SYSTEM_LOGIN_TOGGLES	2

static system_login_text_col[NUM_SYSTEM_LOGIN_VALUES] = 
	{3,3,3,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DefLoginStart, DefLoginOpen, DefLoginClose,
			DefLoginStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
		    work_area1_widget,
		    work_area2_frame,
		    work_area2_widget,
		    title,
		    ok_button,
		    cancel_button,
		    help_button,
		    w,w1;
	int         i;
	Dimension   max_label_width;


	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DefLogin");
	XtAddCallback(form_widget, XmNhelpCallback,
				    HelpDisplayOpen, "accounts,DefLogin");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_default_login", &msg_header, 
			&msg_header_text);
	title = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* First create the form work area widget                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, title, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	
	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	value 	 	 = MallocInt(NUM_SYSTEM_LOGIN_VALUES);
	toggle_state     = MallocChar(NUM_SYSTEM_LOGIN_TOGGLES);
	yes_widget 	 = MallocWidget(NUM_SYSTEM_LOGIN_TOGGLES);
	text_widget 	 = MallocWidget(NUM_SYSTEM_LOGIN_VALUES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_system_login_value)
		LoadMessage ("msg_accounts_default_login_value", 
			&msg_system_login_value, &msg_system_login_value_text);
	max_label_width = (Dimension) 0;
	CreateItemsText(work_area1_widget, 0, NUM_SYSTEM_LOGIN_VALUES,
		msg_system_login_value, &max_label_width, 
		system_login_text_col, text_widget);

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], 
		work_area1_frame, True, title);

	/**********************************************************************/
	/*
	/* Create all the Toggle button widgets                               */
	/*
	/**********************************************************************/
	work_area2_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_system_login_toggle)
		LoadMessage ("msg_accounts_default_login_toggle", 
		&msg_system_login_toggle, &msg_system_login_toggle_text);
	CreateItemsYN(work_area2_widget, 0, NUM_SYSTEM_LOGIN_TOGGLES,
		msg_system_login_toggle, &max_label_width, yes_widget);

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
			HelpDisplayOpen, "accounts,DefLogin");
	XSync(XtDisplay(main_shell), FALSE);
}

/* We only use OK, CANCEL and HELP callbacks */
CREATE_TEXT_CALLBACKS(value)

CREATE_CALLBACKS(msg_header[3], DefLoginClose)

static int
LoadVariables()
{
	int i;
	int ret;
	char buffer[20];
	struct pr_default *df = &sd.df;

	/* Load the  value defaults */
	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);

	/* First get the text values */
	if (df->prg.fg_max_tries)
		value    [0] = (int) df->prd.fd_max_tries;
	else 	value    [0] = 0;

	if (df->sflg.fg_inactivity_timeout)
		value    [1] = (int) df->sfld.fd_inactivity_timeout / 60;
	else 	value    [1] = 0;

	if (df->prg.fg_nice)
		value    [2] = (int) df->prd.fd_nice;
	else 	value    [2] = 0;

	/* Toggles */
	if (df->sflg.fg_boot_authenticate && df->sfld.fd_boot_authenticate)
		toggle_state[0] = YES_CHAR;
	else	toggle_state[0] = NO_CHAR;

	if (df->prg.fg_lock && df->prd.fd_lock)
		toggle_state[1] = YES_CHAR;
	else	toggle_state[1] = NO_CHAR;

	for (i=0; i<NUM_SYSTEM_LOGIN_VALUES; i++) {
		sprintf (buffer, "%d", value[i]);
		XmTextSetString (text_widget[i], buffer);
	}

	for (i=0; i<NUM_SYSTEM_LOGIN_TOGGLES; i++) {
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
	ulong l;
	time_t t;
	int nerrs = 0;

	/* Check the text fields */
	sdef->prg.fg_max_tries = 1;
	buf = XmTextGetString(text_widget[0]);
	i = atoi (buf);
	if (i >= 0 && i <= 999)
		sdef->prd.fd_max_tries = (ushort) (i);
	else
		nerrs ++;
	XtFree(buf);

	buf = XmTextGetString(text_widget[1]);
	sdef->sflg.fg_inactivity_timeout = 1;
	i = atoi (buf);
	if (i >= 0 && i <= 999)
		sdef->sfld.fd_inactivity_timeout = (time_t) (i * 60);
	else
		nerrs ++;
	XtFree(buf);

	buf = XmTextGetString(text_widget[2]);
	sdef->prg.fg_nice = 1;
	i = atoi (buf);
	if (i >= -20 && i <= 20)
		sdef->prd.fd_nice = i;
	else
		nerrs ++;
	XtFree(buf);

	/* Display an error if not all numbers
	 * are non-zero integers
	 */
	if (nerrs) {
		if (! msg_error) 
			LoadMessage ("msg_accounts_default_login_error",
				&msg_error, &msg_error_text);
		ErrorMessageOpen(-1, msg_error, 1, NULL);
		return (FAILURE);
	}

	return (SUCCESS);
}

static int
WriteInformation ()
{
	struct pr_default *sdef = &sd.df;

	/* Toggles */
	sdef->sflg.fg_boot_authenticate = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[0]))
		  sdef->sfld.fd_boot_authenticate = 1;
	else  sdef->sfld.fd_boot_authenticate = 0;

	sdef->prg.fg_lock = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[1]))
		  sdef->prd.fd_lock = 1;
	else  sdef->prd.fd_lock = 0;

	return (XWriteSystemInfo (&sd));
}

#endif /* SEC_BASE **/

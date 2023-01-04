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
static char	*sccsid = "@(#)$RCSfile: XDevDefLog.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:09:34 $";
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
		XDevDefLog.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevDefLog function in role programs.
		
	entry points:
		DevDefLogStart()
		DevDefLogOpen()
		DevDefLogClose()
		DevDefLogStop()

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
#include "XDevices.h"
#include "XMacros.h"

/* External routines */
extern void ErrorMessageOpen();

/* Local variables */
static char 
	**msg_def_dev_login_value,
	**msg_def_dev_login_toggle,
	**msg_error,
	*msg_def_dev_login_value_text,
	*msg_def_dev_login_toggle_text,
	*msg_error_text;

/* Definitions */
#define NUM_DEFAULT_DEVICES_LOGIN_VALUES		3
#define NUM_DEFAULT_DEVICES_LOGIN_TOGGLES		1

static def_dev_login_text_col[NUM_DEFAULT_DEVICES_LOGIN_VALUES] = 
	{3,3,3,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevDefLogStart, DevDefLogOpen, DevDefLogClose,
			DevDefLogStop)

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
	form_widget = CreateForm(mother_form, "DevDefLog");
	XtAddCallback(form_widget, XmNhelpCallback,
				    HelpDisplayOpen, "devices,DevDefLog");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_default_login", &msg_header, 
			&msg_header_text);
	title = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/*
	/* Create all the value setting widgets                               */
	/*
	/**********************************************************************/

	/**********************************************************************/
	/* First create the form work area widget                             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	work_area1_frame  = CreateFrame(form_widget, title, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	max_label_width = (Dimension) 0;

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	value 	 	 = MallocInt(NUM_DEFAULT_DEVICES_LOGIN_VALUES);
	toggle_state     = MallocChar(NUM_DEFAULT_DEVICES_LOGIN_TOGGLES);
	yes_widget 	 = MallocWidget(NUM_DEFAULT_DEVICES_LOGIN_TOGGLES);
	text_widget 	 = MallocWidget(NUM_DEFAULT_DEVICES_LOGIN_VALUES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	LoadMessage ("msg_devices_default_login_value", 
		&msg_def_dev_login_value, &msg_def_dev_login_value_text);
	CreateItemsText(work_area1_widget, 0, NUM_DEFAULT_DEVICES_LOGIN_VALUES,
		msg_def_dev_login_value, &max_label_width, 
		def_dev_login_text_col, text_widget);

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
	
	/*
	max_label_width = (Dimension) 0;
	*/
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
#if SEC_NET_TTY
	/* If remote host then should read "Lock Terminal/Host" otherwise
	 * it reads "Lock Host" */
	LoadMessage ("msg_devices_default_login_toggle", 
#else
	LoadMessage ("msg_devices_default_login_sec_net_tty_toggle", 
#endif
		&msg_def_dev_login_toggle, &msg_def_dev_login_toggle_text);

	CreateItemsYN(work_area2_widget, 0, NUM_DEFAULT_DEVICES_LOGIN_TOGGLES,
		msg_def_dev_login_toggle, &max_label_width, yes_widget);

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
			HelpDisplayOpen, "devices,DevDefLog");
}

/* We only use OK, CANCEL and HELP callbacks */
CREATE_TEXT_CALLBACKS(value)

CREATE_CALLBACKS(msg_header[3], DevDefLogClose)

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
	if (df->tcg.fg_max_tries)
		value    [0] = (int) df->tcd.fd_max_tries;
	else	value    [0] = 0;

	if (df->tcg.fg_login_timeout)
		value    [1] = (int) df->tcd.fd_login_timeout;
	else 	value    [1] = 0;

	if (df->tcg.fg_logdelay)
		value    [2] = (int) df->tcd.fd_logdelay;
	else 	value    [2] = 0;

	/* Toggles */
	if (df->tcg.fg_lock && df->tcd.fd_lock)
		toggle_state[0] = YES_CHAR;
	else	toggle_state[0] = NO_CHAR;

	for (i=0; i<NUM_DEFAULT_DEVICES_LOGIN_VALUES; i++) {
		sprintf (buffer, "%d", value[i]);
		XmTextSetString (text_widget[i], buffer);
	}

	for (i=0; i<NUM_DEFAULT_DEVICES_LOGIN_TOGGLES; i++) {
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
	sdef->tcg.fg_max_tries = 1;
	buf = XmTextGetString(text_widget[0]);
	i = atoi (buf);
	if (i >= 0 && i <= 999)
		sdef->tcd.fd_max_tries = (ushort) (i);
	else
		nerrs ++;
	XtFree(buf);

	buf = XmTextGetString(text_widget[1]);
	sdef->tcg.fg_login_timeout = 1;
	i = atoi (buf);
	if (i >= 0 && i < 1000)
		sdef->tcd.fd_login_timeout = (ushort) i;
	else
		nerrs ++;
	XtFree(buf);

	sdef->tcg.fg_logdelay = 1;
	buf = XmTextGetString(text_widget[2]);
	t = atol (buf);
	if (t >= 0 && t <= 999)
		sdef->tcd.fd_logdelay = t;
	else
		nerrs ++;
	XtFree(buf);

	/* Display an error if not all numbers
	 * are non-zero integers
	 */
	if (nerrs) {
		if (! msg_error) 
			LoadMessage ("msg_devices_default_login_error",
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
	sdef->tcg.fg_lock = 1;
	if (XmToggleButtonGadgetGetState(yes_widget[0]))
		  sdef->tcd.fd_lock = 1;
	else  sdef->tcd.fd_lock = 0;

	return (XWriteSystemInfo (&sd));
}

#endif /* SEC_BASE **/

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
static char	*sccsid = "@(#)$RCSfile: XDevAddTer.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:09:30 $";
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
		XDevAddTer.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevAddTer function in role programs.
		
	entry points:
		DevAddTerStart()
		DevAddTerOpen()
		DevAddTerClose()
		DevAddTerStop()

*/

/* Common C include files */
#include <sys/types.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/ToggleBG.h>

/* Role program  include files */
#include "XMain.h"
#include "XDevices.h"
#include "XMacros.h"

/* External routines */
extern Widget 
#if SEC_MAC
	CreateSingleMultiLevel(),
#endif
	CreateScrolledWindow(),
	CreateScrolledWindowForm(),
	CreateTexts(),
	CreateItemText();

extern int
	XValidateDeviceEntry(),
	XValidateTerminalEntry();

extern Dimension
	GetWidth();

extern void 
#if SEC_MAC
	SetAddMultiLevelToggle(),
#endif
	ErrorMessageOpen();

/* Local variables */
static Widget
	b11, b12, b21, b22,
	device_list_form,
	device_text_widget,
	*device_list_widget;

static int
	device_list_created = False,
	device_list_widget_count;

static char
	**msg_device_login_value,
	**msg_device_login_toggle,
	**msg_error,
	*msg_device_login_toggle_text,
	*msg_device_login_value_text,
	*msg_error_text;

/* Definitions */
#define NUM_DEVICE_ADD_TERMINAL_VALUES		3
#define NUM_DEVICE_ADD_TERMINAL_TOGGLES		1

static int device_login_text_col[NUM_DEVICE_ADD_TERMINAL_VALUES] = 
	{3,3,3,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevAddTerStart, DevAddTerOpen, DevAddTerClose,
	DevAddTerStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
		    work_area1_widget,
		    work_area2_frame,
		    work_area2_widget,
		    work_area3_frame,
		    work_area3_widget,
		    work_area4_frame,
		    work_area4_widget,
		    ok_button,
		    cancel_button,
		    help_button,
		    w,w1,w2;
	int         i;
	Dimension   max_label_width;


	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevAddTer");
	XtAddCallback(form_widget, XmNhelpCallback,
				 HelpDisplayOpen, "devices,DevAddTer");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_add_terminal", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* First create the form work area widget                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Device name                                                        */
	/**********************************************************************/
	max_label_width = (Dimension) 0;
	device_name_widget = CreateItemText(work_area1_widget, True, NULL, 
		msg_header[4], &max_label_width, TERMINAL_NAME_LEN,
		&device_text_widget);

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[5], 
		work_area1_frame, True, NULL);

	XSync(XtDisplay(main_shell), FALSE);
	/**********************************************************************/
	/* Device list                                                        */
	/**********************************************************************/
	w1 = CreateScrolledWindow(form_widget, work_area1_frame, w);
	device_list_form = CreateScrolledWindowForm(w1);

	/**********************************************************************/
	/* Create the form work area widget                                   */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	work_area2_frame  = CreateFrame(form_widget, w1, True, NULL);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	def_value     	 = MallocInt(NUM_DEVICE_ADD_TERMINAL_VALUES);
	value     	 = MallocInt(NUM_DEVICE_ADD_TERMINAL_VALUES);
	value_state      = MallocChar(NUM_DEVICE_ADD_TERMINAL_VALUES);
	toggle_state     = MallocChar(NUM_DEVICE_ADD_TERMINAL_TOGGLES);
	def_toggle_state = MallocChar(NUM_DEVICE_ADD_TERMINAL_TOGGLES);
	text_default_widget = MallocWidget(NUM_DEVICE_ADD_TERMINAL_VALUES);
	text_widget 	 = MallocWidget(NUM_DEVICE_ADD_TERMINAL_VALUES);
	yes_widget 	 = MallocWidget(NUM_DEVICE_ADD_TERMINAL_TOGGLES);
	default_widget   = MallocWidget(NUM_DEVICE_ADD_TERMINAL_TOGGLES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_device_login_value)
		LoadMessage("msg_devices_add_terminal_value", 
			&msg_device_login_value, 
			&msg_device_login_value_text);
	max_label_width = (Dimension) 0;

	CreateItemsTextD(work_area2_widget, 0, NUM_DEVICE_ADD_TERMINAL_VALUES,
		msg_device_login_value, &max_label_width, 
		device_login_text_col, text_widget, text_default_widget);

	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_DEVICE_ADD_TERMINAL_VALUES; i++) {
		XtAddCallback (text_widget[i], XmNvalueChangedCallback, 
				 TextCallback, i);
		XtAddCallback (text_default_widget[i], XmNvalueChangedCallback, 
				 TextDefaultCallback, i);
	}
	/**********************************************************************/
	/* Create the single/multi level widgets                              */
	/**********************************************************************/
#if SEC_MAC && ! SEC_SHW
	work_area3_frame = CreateSingleMultiLevel (form_widget, 
		work_area2_frame, True, NULL, False,
		&b11, &b12, &b21, &b22);
#endif

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
#if SEC_MAC && ! SEC_SHW
	w = CreateHeader (form_widget, msg_header[1], 
		work_area2_frame, False, work_area3_frame);
#else
	w = CreateHeader (form_widget, msg_header[1], 
		work_area2_frame, True, NULL);
#endif

	max_label_width = GetWidth(w) + (Dimension) 10;
	/**********************************************************************/
	/*
	/* Create all the Toggle button widgets                               */
	/*
	/**********************************************************************/
#if SEC_MAC && ! SEC_SHW
	work_area4_frame  = CreateFrame(form_widget, w, False, work_area3_frame);
#else
	work_area4_frame  = CreateFrame(form_widget, w, True, NULL);
#endif
	work_area4_widget = CreateSecondaryForm(work_area4_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	if (! msg_device_login_toggle)
		LoadMessage("msg_devices_add_terminal_toggle", 
			&msg_device_login_toggle, 
			&msg_device_login_toggle_text);

	CreateItemsYND(work_area4_widget, 0, 
		NUM_DEVICE_ADD_TERMINAL_TOGGLES,
		msg_device_login_toggle, &max_label_width, yes_widget,
		default_widget);

	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_DEVICE_ADD_TERMINAL_TOGGLES; i++) {
		XtAddCallback (yes_widget[i], XmNvalueChangedCallback, 
				OnCallback, i);
		XtAddCallback (default_widget[i], XmNvalueChangedCallback, 
				DefaultCallback, i);
	}

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader (form_widget, msg_header[2], work_area2_frame, 
		False, w);

	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons			     */
	/*********************************************************************/
#if SEC_MAC && ! SEC_SHW
	CreateThreeButtons (form_widget, work_area3_frame,
			     &ok_button, &cancel_button, &help_button);
#else
	CreateThreeButtons (form_widget, work_area4_frame,
			     &ok_button, &cancel_button, &help_button);
#endif
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "devices,DevAddTer");
}


CREATE_TEXT_CALLBACKS (def_value)
CREATE_ON_CALLBACKS (def_toggle_state, False, NUM_DEVICE_ADD_TERMINAL_TOGGLES)
CREATE_CALLBACKS (msg_header[3],  DevAddTerClose)

static int
LoadVariables ()
{
	int i;
	int ret;
	struct pr_default *df = &sd.df;

	/* Load the  value defaults */
	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);

	/* Set all values/buttons to default */
	for (i=0; i<NUM_DEVICE_ADD_TERMINAL_VALUES; i++) {
		value_state[i] = YES_CHAR;
	}

	for (i=0; i<NUM_DEVICE_ADD_TERMINAL_TOGGLES; i++) {
		toggle_state[i] = DEFAULT_CHAR;
	}

	/* Get the default text values */
	if (df->tcg.fg_max_tries)
		def_value [0] = (int) df->tcd.fd_max_tries;
	else	def_value [0] = 0;

	if (df->tcg.fg_login_timeout)
		def_value [1] = (int) df->tcd.fd_login_timeout;
	else 	def_value [1] = 0;

	if (df->tcg.fg_logdelay)
		def_value [2] = (int) df->tcd.fd_logdelay;
	else 	def_value [2] = 0;

	/* Toggles */
	if (df->tcg.fg_lock && df->tcd.fd_lock)
		def_toggle_state[0] = YES_CHAR;
	else	def_toggle_state[0] = NO_CHAR;


	/* All values should be default first time in */
	in_change_mode = False;
	for (i=0; i<NUM_DEVICE_ADD_TERMINAL_VALUES; i++) {
		SetTextD(value[i], value_state[i], def_value[i], 
			text_widget[i], text_default_widget[i]);
	}
	in_change_mode = True;

	for (i=0; i<NUM_DEVICE_ADD_TERMINAL_TOGGLES; i++) {
		SetYND(toggle_state[i], def_toggle_state[i],
			yes_widget[i], default_widget[i]);
	}

#if SEC_MAC
	/* Set the single level, multi level toggles */
	SetAddMultiLevelToggle(b11, b12, b21, b22, False, df);
#endif

	/* If not the first time in then save some memory */
	if (device_list_created) {
		for (i=0; i<device_list_widget_count; i++) {
			XtDestroyWidget(device_list_widget[i]);
		}
	}
	else
		device_list_created = True;

	/* Put in some blank values in the scrolled text widget */
	device_list_widget_count = DEVICE_LIST_CHUNK;
	device_list_widget = (Widget *) Malloc(sizeof(Widget) *
		device_list_widget_count);
	if (! device_list_widget)
		MemoryError();

	CreateTexts(device_list_form, 0, device_list_widget_count,
			DEVICE_LIST_MAX_WIDTH, device_list_widget);

	/* Clear the device name */
	XmTextSetString (device_text_widget, NULL);
	return (ret);
}

static int
ValidateEntries ()
{
	char	*name;
	int ret;
	
	name = XmTextGetString (device_text_widget);
	ret = XValidateDeviceEntry (name, device_list_widget,
		device_list_widget_count, b11, b21, &dv, 
		True, AUTH_DEV_TERMINAL);

	/* Validate the terminal part */
	if (ret == SUCCESS) {
		if (ret = XValidateTerminalEntry (name, text_default_widget, 
			text_widget,
			XmToggleButtonGadgetGetState(default_widget[0]),
			XmToggleButtonGadgetGetState(yes_widget[0]),
			&dv, True) ) {
			if (! msg_error) 
				LoadMessage ("msg_devices_default_login_error",
					&msg_error, &msg_error_text);
			ErrorMessageOpen(-1, msg_error, 1, NULL);
		}
	}

	XtFree (name);
	return (ret);
}

static int
WriteInformation ()
{
	int ret;

	ret = XWriteDeviceAndTerminalInfo(&dv);
	return (ret);
}

#endif /* SEC_BASE **/

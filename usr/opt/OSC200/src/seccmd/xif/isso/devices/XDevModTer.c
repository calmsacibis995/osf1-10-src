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
static char	*sccsid = "@(#)$RCSfile: XDevModTer.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:30 $";
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
		XDevModTer.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevModTer function in role programs.
		
	entry points:
		DevModTerStart()
		DevModTerOpen()
		DevModTerClose()
		DevModTerStop()

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

extern Dimension
	GetWidth();

extern int
	XValidateDeviceEntry(),
	XValidateTerminalEntry();

extern void 
#if SEC_MAC
	SetModifyMultiLevelToggle(),
#endif
	ErrorMessageOpen();

/* Local variables */
static Widget
	b11, b12, b21, b22,
	device_list_form,
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

CREATE_SCREEN_ROUTINES (DevModTerStart, DevModTerOpen, DevModTerClose,
	DevModTerStop)

static void 
MakeWidgets() 
{
	Widget      work_area2_frame,
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
	form_widget = CreateForm(mother_form, "DevModTer");
	XtAddCallback(form_widget, XmNhelpCallback,
				 HelpDisplayOpen, "devices,DevModTer");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_modify_terminal", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Device name                                                        */
	/**********************************************************************/
	device_name_widget = CreateDeviceName(form_widget, w);

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[5], 
		device_name_widget, True, NULL);

	XSync(XtDisplay(main_shell), FALSE);
	/**********************************************************************/
	/* Device list                                                        */
	/**********************************************************************/
	w1 = CreateScrolledWindow(form_widget, device_name_widget, w);
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
	LoadMessage("msg_devices_add_terminal_value", &msg_device_login_value, 
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
	/* Create the single/multi SL widget. IL is not supported for terms   */
	/**********************************************************************/
#if SEC_MAC && ! defined (SEC_SHW)
	work_area3_frame = CreateSingleMultiLevel (form_widget, 
		work_area2_frame, True, NULL, False,
		&b11, &b12, &b21, &b22);
#else
	work_area3_frame = work_area2_frame;
#endif

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
#if SEC_MAC && ! defined (SEC_SHW)
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
#if SEC_MAC && ! defined (SEC_SHW)
	work_area4_frame  = CreateFrame(form_widget, w, False, work_area3_frame);
#else
	work_area4_frame  = CreateFrame(form_widget, w, True, NULL);
#endif
	work_area4_widget = CreateSecondaryForm(work_area4_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	LoadMessage("msg_devices_add_terminal_toggle", 
		&msg_device_login_toggle, &msg_device_login_toggle_text);

	CreateItemsYND(work_area4_widget, 0, 
		NUM_DEVICE_ADD_TERMINAL_TOGGLES,
		msg_device_login_toggle, &max_label_width, yes_widget,
		default_widget);

	/* Import/Export have no default toggles */
	/* None for terminal */
	/*
	XtUnmanageChild(default_widget[1]);
	XtUnmanageChild(default_widget[2]);
	*/

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
#if SEC_MAC && ! defined (SEC_SHW)
	CreateThreeButtons (form_widget, work_area3_frame,
			     &ok_button, &cancel_button, &help_button);
#else
	CreateThreeButtons (form_widget, work_area4_frame,
			     &ok_button, &cancel_button, &help_button);
#endif
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "devices,DevModTer");
}


CREATE_TEXT_CALLBACKS (def_value)
CREATE_ON_CALLBACKS (def_toggle_state, False, NUM_DEVICE_ADD_TERMINAL_TOGGLES)
CREATE_CALLBACKS (msg_header[3],  DevModTerClose)

static int
LoadVariables ()
{
	int i;
	int ret;
	int count;

	/* Load the  value defaults */
	ret = XGetDeviceInfo(chosen_device_name, &dv);
	if (ret)
		return (ret);

	/* Get the text field values */
	if (dv.tm.uflg.fg_max_tries) {
		value      [0] = (int) dv.tm.ufld.fd_max_tries;
		value_state[0] = NO_CHAR;
	}
	else
		value_state[0] = YES_CHAR;

	if (dv.tm.uflg.fg_login_timeout) {
		value      [1] = (int) dv.tm.ufld.fd_login_timeout;
		value_state[1] = NO_CHAR;
	}
	else
		value_state[1] = YES_CHAR;

	if (dv.tm.uflg.fg_logdelay) {
		value      [2] = (int) dv.tm.ufld.fd_logdelay;
		value_state[2] = NO_CHAR;
	}
	else
		value_state[2] = YES_CHAR;

	if (dv.tm.uflg.fg_lock)
		if (dv.tm.ufld.fd_lock)
			toggle_state[0] = YES_CHAR;
		else    toggle_state[0] = NO_CHAR;
	else        toggle_state[0] = DEFAULT_CHAR;

	/*
	if (dv.dev.uflg.fg_assign && 
		ISBITSET(dv.dev.ufld.fd_assign, AUTH_DEV_IMPORT))
			toggle_state[1] = YES_CHAR;
	else		toggle_state[1] = NO_CHAR;
			
	if (dv.dev.uflg.fg_assign && 
		ISBITSET(dv.dev.ufld.fd_assign, AUTH_DEV_EXPORT))
			toggle_state[2] = YES_CHAR;
	else		toggle_state[2] = NO_CHAR;
	*/
			
	/* Get the default text values */
	if (dv.tm.sflg.fg_max_tries)
		def_value [0] = (int) dv.tm.sfld.fd_max_tries;
	else	def_value [0] = 0;

	if (dv.tm.sflg.fg_login_timeout)
		def_value [1] = (int) dv.tm.sfld.fd_login_timeout;
	else 	def_value [1] = 0;

	if (dv.tm.sflg.fg_logdelay)
		def_value [2] = (int) dv.tm.sfld.fd_logdelay;
	else 	def_value [2] = 0;

	/* Toggles */
	if (dv.tm.sflg.fg_lock && dv.tm.sfld.fd_lock)
		def_toggle_state[0] = YES_CHAR;
	else	def_toggle_state[0] = NO_CHAR;
	/*
	def_toggle_state[1] = NO_CHAR;
	def_toggle_state[2] = NO_CHAR;
	*/

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

#if SEC_MAC && ! defined (SEC_SHW)
	/* Set the single level, multi level toggles */
	SetModifyMultiLevelToggle(b11, b12, b21, b22, False, &dv.dev);
#endif

	/* If not the first time in then save some memory */
	if (device_list_created) {
		for (i=0; i<device_list_widget_count; i++) {
			XtDestroyWidget(device_list_widget[i]);
		}
	}
	else
		device_list_created = True;

	/* Count the number of device names and allocate the space */
	count = 0;
	if (dv.dev.uflg.fg_devs)
		for (count = 0; dv.dev.ufld.fd_devs[count] != (char *) 0; 
			count++)	 ;
	device_list_widget_count = count + DEVICE_LIST_CHUNK;
	device_list_widget = (Widget *) Malloc (sizeof (Widget) *
		device_list_widget_count);
	if (! device_list_widget)
		MemoryError();
	CreateTexts(device_list_form, 0, device_list_widget_count,
			DEVICE_LIST_MAX_WIDTH, device_list_widget);

	/* Set the device list field */
	if (dv.dev.uflg.fg_devs && count > 0)
		for (i=0; i<count; i++)
			XmTextSetString(device_list_widget[i], 
				dv.dev.ufld.fd_devs[i]);

	/* Set the device name */
#ifdef DEBUG
	printf ("device_name_widget %d\n", device_name_widget);
	printf ("chosen_device_name  %s\n", chosen_device_name);
#endif
	SetDeviceName (device_name_widget, chosen_device_name);
	return (ret);
}

static int
ValidateEntries ()
{
	int ret;
	
	ret = XValidateDeviceEntry (chosen_device_name, device_list_widget,
		device_list_widget_count, b11, b21, &dv, 
		False, AUTH_DEV_TERMINAL);

	/* Validate the terminal part */
	if (ret == SUCCESS) {
		if (ret = XValidateTerminalEntry (chosen_device_name, 
			text_default_widget, text_widget,
			XmToggleButtonGadgetGetState(default_widget[0]),
			XmToggleButtonGadgetGetState(yes_widget[0]),
			&dv, False) ) {
			if (! msg_error) 
				LoadMessage ("msg_devices_default_login_error",
					&msg_error, &msg_error_text);
			ErrorMessageOpen(-1, msg_error, 1, NULL);
		}
	}

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

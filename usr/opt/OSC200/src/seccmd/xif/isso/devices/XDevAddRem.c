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
static char	*sccsid = "@(#)$RCSfile: XDevAddRem.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:09:27 $";
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
		XDevAddRem.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevAddRem function in role programs.
		
	entry points:
		DevAddRemStart()
		DevAddRemOpen()
		DevAddRemClose()
		DevAddRemStop()

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
	XValidateDeviceEntry();

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
#define NUM_DEVICE_ADD_TAPE_TOGGLES		2

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevAddRemStart, DevAddRemOpen, DevAddRemClose,
	DevAddRemStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
		    work_area1_widget,
		    work_area2_frame,
		    work_area2_widget,
		    work_area3_frame,
		    work_area3_widget,
		    ok_button,
		    cancel_button,
		    help_button,
		    w,w1,w2;
	int         i;
	Dimension   max_label_width;


	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevAddRem");
	XtAddCallback(form_widget, XmNhelpCallback,
				 HelpDisplayOpen, "devices,DevAddRem");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_add_tape", &msg_header, 
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
		msg_header[4], &max_label_width, TAPE_NAME_LEN,
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

#if SEC_MAC
	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	toggle_state     = MallocChar(NUM_DEVICE_ADD_TAPE_TOGGLES);
	def_toggle_state = MallocChar(NUM_DEVICE_ADD_TAPE_TOGGLES);
	yes_widget 	 = MallocWidget(NUM_DEVICE_ADD_TAPE_TOGGLES);
	default_widget   = MallocWidget(NUM_DEVICE_ADD_TAPE_TOGGLES);
	
	/**********************************************************************/
	/* Create the single/multi level widgets                              */
	/**********************************************************************/
	work_area2_frame = CreateSingleMultiLevel (form_widget, 
		w1, True, NULL, True,
		&b11, &b12, &b21, &b22);

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], 
		w1, False, work_area2_frame);

	max_label_width = GetWidth(w) + (Dimension) 10;
	/**********************************************************************/
	/*
	/* Create all the Toggle button widgets                               */
	/*
	/**********************************************************************/
	work_area3_frame  = CreateFrame(form_widget, w, False, work_area2_frame);
	work_area3_widget = CreateSecondaryForm(work_area3_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	LoadMessage("msg_devices_add_tape_toggle", 
		&msg_device_login_toggle, &msg_device_login_toggle_text);

	CreateItemsYND(work_area3_widget, 0, 
		NUM_DEVICE_ADD_TAPE_TOGGLES,
		msg_device_login_toggle, &max_label_width, yes_widget,
		default_widget);

	/* Import/Export have no default toggles */
	XtUnmanageChild(default_widget[0]);
	XtUnmanageChild(default_widget[1]);

	/**********************************************************************/
	/* Create callbacks N.B. note that last two have no callbacks         */
	/**********************************************************************/
	/* No callbacks !! 
	for (i=0; i<(NUM_DEVICE_ADD_TAPE_TOGGLES-2); i++) {
		XtAddCallback (yes_widget[i], XmNvalueChangedCallback, 
				OnCallback, i);
		XtAddCallback (default_widget[i], XmNvalueChangedCallback, 
				DefaultCallback, i);
	}
	*/

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader (form_widget, msg_header[2], w1, 
		False, w);

	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons			     */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
			     &ok_button, &cancel_button, &help_button);
#else
	CreateThreeButtons (form_widget, w1,
			     &ok_button, &cancel_button, &help_button);
#endif
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "devices,DevAddRem");
}


CREATE_TEXT_CALLBACKS (def_value)
CREATE_ON_CALLBACKS (def_toggle_state, False, NUM_DEVICE_ADD_TAPE_TOGGLES)
CREATE_CALLBACKS (msg_header[3],  DevAddRemClose)

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

#if SEC_MAC
	toggle_state[0] = YES_CHAR;
	toggle_state[1] = YES_CHAR;

	/* Not used, but just in case set to true */
	def_toggle_state[0] = YES_CHAR;
	def_toggle_state[1] = YES_CHAR;

	/* All values should be default first time in */
	for (i=0; i<NUM_DEVICE_ADD_TAPE_TOGGLES; i++) {
		SetYND(toggle_state[i], def_toggle_state[i],
			yes_widget[i], default_widget[i]);
	}

	/* Set the single level, multi level toggles */
	SetAddMultiLevelToggle(b11, b12, b21, b22, True, df);
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
		True, AUTH_DEV_TAPE);
	XtFree (name);

#if SEC_MAC
	if (XmToggleButtonGadgetGetState (yes_widget[0]))
		ADDBIT (dv.dev.ufld.fd_assign, AUTH_DEV_IMPORT);

	if (XmToggleButtonGadgetGetState (yes_widget[1]))
		ADDBIT (dv.dev.ufld.fd_assign, AUTH_DEV_EXPORT);
#endif
	return (ret);
}

static int
WriteInformation ()
{
	int	i;
	int 	ret;

	ret = XWriteDeviceInfo(&dv);
	/* Should free memory here */
	if (dv.dev.ufld.fd_name) {
		free(dv.dev.ufld.fd_name);
		dv.dev.ufld.fd_name = (char *) NULL;
	}
	if (dv.dev.ufld.fd_devs) {
		for (i=0; dv.dev.ufld.fd_devs[i]; i++) 
			free(dv.dev.ufld.fd_devs[i]);
		free(dv.dev.ufld.fd_devs);
		dv.dev.ufld.fd_devs = (char **) NULL;
	}
	return (ret);
}

#endif /* SEC_BASE **/

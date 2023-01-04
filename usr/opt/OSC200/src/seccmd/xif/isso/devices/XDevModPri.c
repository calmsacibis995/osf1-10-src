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
static char	*sccsid = "@(#)$RCSfile: XDevModPri.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:19 $";
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
		XDevModPri.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevModPri function in role programs.
		
	entry points:
		DevModPriStart()
		DevModPriOpen()
		DevModPriClose()
		DevModPriStop()

*/

/* Common C include files */
#include <sys/types.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>

/* Role program  include files */
#include "XMain.h"
#include "XDevices.h"
#include "XMacros.h"

/* External routines */
extern Widget 
#if SEC_MAC
	CreateSingleMultiLevel(),
#endif /* SEC_MAC */
	CreateScrolledWindow(),
	CreateScrolledWindowForm(),
	CreateTexts(),
	CreateItemText();

extern int
	XValidateDeviceEntry();
	
extern void 
	ErrorMessageOpen();

/* Local variables */
static Widget
	b11, b12, b21, b22,
	device_list_form,
	*device_list_widget;

static int
	device_list_created = False,
	device_list_widget_count,
	device_list_name_count;

static char
	**device_list_names,
	**msg_device_login_value,
	**msg_device_login_toggle,
	**msg_error,
	*msg_device_login_toggle_text,
	*msg_device_login_value_text,
	*msg_error_text;

/* Definitions */
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevModPriStart, DevModPriOpen, DevModPriClose,
	DevModPriStop)

static void 
MakeWidgets() 
{
	Widget      work_area3_frame,
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
	form_widget = CreateForm(mother_form, "DevModPri");
	XtAddCallback(form_widget, XmNhelpCallback,
				 HelpDisplayOpen, "devices,DevModPri");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_modify_printer", &msg_header, 
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
	/* Create the single/multi level widgets                              */
	/**********************************************************************/
#if SEC_MAC
	work_area3_frame = CreateSingleMultiLevel (form_widget, 
		w1, True, NULL, True,
		&b11, &b12, &b21, &b22);
#else
	work_area3_frame = w1;
#endif /* SEC_MAC */

	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons			     */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area3_frame,
			     &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "devices,DevModPri");
}


CREATE_TEXT_CALLBACKS (def_value)
CREATE_CALLBACKS (msg_header[3],  DevModPriClose)

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

#if SEC_MAC
	/* Set the single level, multi level toggles */
	SetModifyMultiLevelToggle(b11, b12, b21, b22, True, &dv.dev);
#endif /* SEC_MAC */

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
	SetDeviceName (device_name_widget, chosen_device_name);
	return (ret);
}

static int
ValidateEntries ()
{
	int ret;
	
	ret = XValidateDeviceEntry (chosen_device_name, device_list_widget,
		device_list_widget_count, b11, b21, &dv, 
		False, AUTH_DEV_PRINTER);
	return (ret);
}

static int
WriteInformation ()
{
	int ret;

	ret = XWriteDeviceInfo(&dv);
	return (ret);
}

#endif /* SEC_BASE **/

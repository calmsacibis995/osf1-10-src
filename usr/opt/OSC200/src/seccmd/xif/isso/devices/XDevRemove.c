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
static char	*sccsid = "@(#)$RCSfile: XDevRemove.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:47 $";
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
		XDevRemove.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for explicitly removing a device
		
	entry points:
		DevRemoveStart()
		DevRemoveOpen()
		DevRemoveClose()
		DevRemoveStop()
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
#include "XMacros.h"
#include "XAccounts.h"
#include "XDevices.h"

/* External routines */
extern Widget
	CreateSelectionBox();

extern void 
	GetAllDevices();

extern char 
	*strdup(),
	*extract_normal_string();

/* Local routines */
static void 
	NoMatchCallback();

/* Local variables */
static char 
	*selected_device_name,
	**msg_error_no_match,
	*msg_error_no_match_text;

static Widget
        selection_widget;
        
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevRemoveStart, DevRemoveOpen, DevRemoveClose, 
	DevRemoveStop)

/* Makes the widgets */
static void 
MakeWidgets() 
{
	Widget      w;

	/**********************************************************************/
	/* Form                                                               */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevRemove");
	XtAddCallback(form_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "devices,DevRemove");
		                         
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_remove_device", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Devices list in scrolled window, titled                            */
	/**********************************************************************/
	selection_widget = CreateSelectionBox (form_widget, "DeviceSelection", 
		w, msg_header[1], msg_header[2], True);

	XtAddCallback(selection_widget, XmNnoMatchCallback, 
			NoMatchCallback, NULL);
	XtAddCallback(selection_widget, XmNokCallback, OKCallback, NULL);
	XtAddCallback(selection_widget, XmNcancelCallback, CancelCallback, 
			NULL);
	XtAddCallback(selection_widget, XmNhelpCallback,  
		            HelpDisplayOpen, "devices,DevRemove");
}		                             

static int 
LoadVariables()
{
	Cardinal	n;
	Arg		args[10];
	int		i;
	XmString	*devices_xmstring;   	/* XmStrings for all devices */
	int		ndevices;		/* total number of devices */
        char		**devices;		/* list of all devices */
                              
	/* Load in all devices on the system except hosts */
	GetAllDevices(&ndevices, &devices, False);

	/* Malloc XmString arrays for the lists of all devices */
	devices_xmstring = (XmString *) Malloc(sizeof(XmString) * ndevices);
	if (! devices_xmstring)
	        MemoryError();
        
	/* Load XmString arrays with values of valid devices */
	for (i = 0; i < ndevices; i++) {
		devices_xmstring[i] = XmStringCreate(devices[i], charset);
		if (! devices_xmstring[i])
			MemoryError();
	}
		
	/* Load in device and values */
	n = 0;
	XtSetArg(args[n], XmNlistItemCount,                    ndevices); n++;
	XtSetArg(args[n], XmNlistItems,                devices_xmstring); n++;
	XtSetValues(selection_widget, args, n);
	
	/* Free various memory */
	if (devices) { 
		free_cw_table(devices);
		devices = NULL;
	}
	if (devices_xmstring) {
		for (i = 0; i < ndevices; i++) {
		    XmStringFree(devices_xmstring[i]);
		}
		free(devices_xmstring);
		devices_xmstring = NULL;
	}
	return (SUCCESS);
}
/* Verifies that the information on the screen is valid;
 * displays a confirmation widget to make sure the device really wants to
 * enter the values.
 */
static void 
OKCallback(w, ptr, cd) 
	Widget                  w; 
	char                    *ptr;
	caddr_t		cd;
{
	XmSelectionBoxCallbackStruct 
		*info = (XmSelectionBoxCallbackStruct *) cd;

	/* Save the device name */
	if (selected_device_name)
		free(selected_device_name);

	selected_device_name = strdup(extract_normal_string (info->value));
	if (! selected_device_name)
		MemoryError();

	/* Create the confirmation box */
	XtSetSensitive (form_widget, False);
	if (! confirmation_open) {
		confirmation_open = TRUE;
		confirmation_widget = CreateConfirmationBox(mother_form,
				msg_header[3]);
		XtAddCallback(confirmation_widget, XmNokCallback,
				ConfirmOKCallback,NULL);
		XtAddCallback(confirmation_widget, XmNcancelCallback,
				ConfirmCancelCallback,NULL);
	}
	else
        	XtManageChild(confirmation_widget);
}

static void
ConfirmOKCallback(w, ptr, info)
	Widget                  w;
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	int ret;

	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
	/* Read the devices protected password database entry */
	ret = XGetDeviceInfo(selected_device_name, &dv);
	if (ret != SUCCESS)
		return;

	/* Remove the device */
	dv.dev.uflg.fg_name   = 0;
	dv.tm.uflg.fg_devname = 0;

	/* If it is a terminal or host then remove also */
	/* Write the information back */
#if SEC_NET_TTY
	if (dv.dev.uflg.fg_type && 
	      ( ISBITSET(dv.dev.ufld.fd_type, AUTH_DEV_TERMINAL) ||
		ISBITSET(dv.dev.ufld.fd_type, AUTH_DEV_REMOTE) ) ) 
#else
	if (dv.dev.uflg.fg_type && 
	      ISBITSET(dv.dev.ufld.fd_type, AUTH_DEV_TERMINAL) )
#endif
		ret = XWriteDeviceAndTerminalInfo(&dv);
	else
		ret = XWriteDeviceInfo(&dv);

	if (ret != SUCCESS)
		return;
	audit_subsystem(dv.dev.ufld.fd_name, 
		"Device has been removed", ET_SYS_ADMIN);
	DevRemoveClose();
}

static void
ConfirmCancelCallback(w, ptr, info)
	Widget                  w;
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
}

static void 
NoMatchCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	if (! msg_error_no_match)
		LoadMessage("msg_devices_remove_device_no_match", 
			&msg_error_no_match, &msg_error_no_match_text);
	ErrorMessageOpen (-1, msg_error_no_match, 0, NULL);
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	DevRemoveClose();  
}
#endif /* SEC_BASE **/

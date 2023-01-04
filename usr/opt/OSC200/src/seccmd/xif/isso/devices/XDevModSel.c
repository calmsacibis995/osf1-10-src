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
static char	*sccsid = "@(#)$RCSfile: XDevModSel.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:27 $";
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
		XDevModSel.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for selecting the device name
		
	entry points:
		DevModSelStart()
		DevModSelOpen()
		DevModSelClose()
		DevModSelStop()
*/

/* Common C include files */
#include <sys/types.h>
#include <sys/security.h>
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
#include "XDevices.h"

/* Definitions */

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
	**msg_error_no_match,
	*msg_error_no_match_text;

static Widget
        selection_widget;
        
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevModSelStart, DevModSelOpen, DevModSelClose,
	DevModSelStop)

static void 
MakeWidgets() 
{
	Widget      w;

	/**********************************************************************/
	/* Form                                                               */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevModSel");
	XtAddCallback(form_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "devices,DevModSel");
		                         
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_modify_select", 
			&msg_header, &msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Device list in scrolled window, titled                             */
	/**********************************************************************/
	selection_widget = CreateSelectionBox (form_widget, "DeviceSelection",
		w, msg_header[1], msg_header[2], True);
	
	XtAddCallback(selection_widget, XmNnoMatchCallback, 
			NoMatchCallback, NULL);
	XtAddCallback(selection_widget, XmNokCallback, OKCallback, NULL);
	XtAddCallback(selection_widget, XmNcancelCallback, CancelCallback, 
			NULL);
	XtAddCallback(selection_widget, XmNhelpCallback,  
		                  HelpDisplayOpen, "devices,DevModSel");
	CenterForm(form_widget);
}		                             

static int 
LoadVariables()
{
	Cardinal	n;
	Arg		args[20];
	int		ndevices;
	char		**devices;
	int		i;
	XmString	*devices_xmstring;
	XmString	xmstring;
                              
	/* Load in all devices includings hosts on the system */
	GetAllDevices(&ndevices, &devices, True);

	/* Malloc XmString arrays for the lists of all devices and groups */
	devices_xmstring = (XmString *) Malloc(sizeof(XmString) * ndevices);
	if (! devices_xmstring)
	        MemoryError();
        
	/* Load XmString arrays with values of valid devices and groups */
	for (i = 0; i < ndevices; i++) {
		devices_xmstring[i] = XmStringCreate(devices[i], charset);
		if (! devices_xmstring[i])
			MemoryError();
	}
		
	/* Load in device values */
	n = 0;
	XtSetArg(args[n], XmNlistItemCount,                     ndevices); n++;
	XtSetArg(args[n], XmNlistItems,                 devices_xmstring); n++;
	if (chosen_device_name) {
		xmstring = XmStringCreate(chosen_device_name, charset);
		if (! xmstring)
			MemoryError();
		XtSetArg(args[n], XmNtextString,      xmstring); n++;
	}
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

	/* If use xmstring we end up with a strange memory bug */
	if (chosen_device_name)
		XmStringFree(xmstring);

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

	if (chosen_device_name)
		free(chosen_device_name);
	chosen_device_name = strdup(extract_normal_string (info->value));
	if (! chosen_device_name)
		MemoryError();
	DevModSelClose();  
}

static void 
NoMatchCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	if (! msg_error_no_match)
		LoadMessage("msg_devices_modify_select_no_match", 
			&msg_error_no_match, &msg_error_no_match_text);
	ErrorMessageOpen (-1, msg_error_no_match, 0, NULL);
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	DevModSelClose();  
}
#endif /* SEC_BASE **/

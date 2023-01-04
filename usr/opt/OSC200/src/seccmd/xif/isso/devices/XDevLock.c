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
static char	*sccsid = "@(#)$RCSfile: XDevLock.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:04 $";
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
		XDevLock.c
		 
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for explicitly locking a terminal/host
		 
	entry points:
		DevLockStart()
		DevLockOpen()
		DevLockClose()
		DevLockStop()
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

/* Definitions */

/* External routines */
extern Widget
	CreateSelectionBox();

extern void 
	GetAllTerminals();

extern char 
	*strdup(),
	*extract_normal_string();

/* Local routines */
static void 
	NoMatchCallback();

/* Local variables */
static char 
	*selected_terminal_name,
	**msg_error_no_match,
	*msg_error_no_match_text;

static Widget
        selection_widget;
        
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevLockStart, DevLockOpen, DevLockClose, DevLockStop)

/* Make widgets for the screen */
static void 
MakeWidgets() 
{
	Widget      w;

	/**********************************************************************/
	/* Form                                                               */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevLock");
	XtAddCallback(form_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "devices,DevLock");
		                         
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_lock_device", &msg_header, 
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
		            HelpDisplayOpen, "devices,DevLock");
	CenterForm(form_widget);
}		                             

static int 
LoadVariables()
{
	Cardinal	n;
	Arg		args[10];
	int		i;
	char		**terminals;
	int		nterminals;
	XmString	*terminals_xmstring;  /* XmStrings for all terminals */
                              
	/* Load in all terminals and hosts on the system */
	GetAllTerminals(&nterminals, &terminals);

	/* Malloc XmString arrays for the lists of all terminals and hosts */
	terminals_xmstring = (XmString *) Malloc(sizeof(XmString) * nterminals);
	if (! terminals_xmstring)
	        MemoryError();
        
	/* Load XmString arrays with values of valid terminals and hosts */
	for (i = 0; i < nterminals; i++) {
		terminals_xmstring[i] = XmStringCreate(terminals[i], charset);
		if (! terminals_xmstring[i])
			MemoryError();
	}
		
	/* Load in terminal and host values */
	n = 0;
	XtSetArg(args[n], XmNlistItemCount,                    nterminals); n++;
	XtSetArg(args[n], XmNlistItems,                terminals_xmstring); n++;
	XtSetValues(selection_widget, args, n);

	/* Free memory */
	if (terminals) { 
		free_cw_table(terminals);
		terminals = NULL;
	}

	if (terminals_xmstring) {
		for (i = 0; i < nterminals; i++) {
		    XmStringFree(terminals_xmstring[i]);
		}
		free(terminals_xmstring);
		terminals_xmstring = NULL;
	}
	
	nterminals = 0;
	if (selected_terminal_name)
		free(selected_terminal_name);

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

	/* Save the terminal name */
	if (selected_terminal_name)
		free(selected_terminal_name);
	selected_terminal_name = strdup(extract_normal_string (info->value));

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
	ret = XGetTerminalInfo(selected_terminal_name, &dv);
	if (ret != SUCCESS)
		return;

	/* Lock the terminal/host */
	dv.tm.uflg.fg_lock = 1;
	dv.tm.ufld.fd_lock = 1;

	/* Write the information back */
	ret = XWriteTerminalInfo(&dv);
	if (ret != SUCCESS)
		return;

	sa_audit_lock(ES_SET_TERM_LOCK, dv.tm.ufld.fd_devname);

	DevLockClose();
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
	LoadMessage("msg_devices_lock_device_no_match", 
		&msg_error_no_match, &msg_error_no_match_text);
	ErrorMessageOpen (-1, msg_error_no_match, 0, NULL);
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	DevLockClose();  
}

#endif /* SEC_BASE **/

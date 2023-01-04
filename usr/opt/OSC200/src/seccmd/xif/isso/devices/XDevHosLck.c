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
static char	*sccsid = "@(#)$RCSfile: XDevHosLck.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:09:54 $";
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
		XDevHosLck.c
		 
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for explicitly locking a host
		 
	entry points:
		DevHosLckStart()
		DevHosLckOpen()
		DevHosLckClose()
		DevHosLckStop()
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

extern char *
	hostname_to_internet();

extern void 
	GetAllHosts();

extern char 
	*strdup(),
	*extract_normal_string();

/* Local routines */
static void 
	NoMatchCallback();

/* Local variables */
static char 
	*selected_host_name,
	**msg_error_no_match,
	*msg_error_no_match_text;

static Widget
        selection_widget;
        
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevHosLckStart, DevHosLckOpen, DevHosLckClose, DevHosLckStop)

/* Make widgets for the screen */
static void 
MakeWidgets() 
{
	Widget      w;

	/**********************************************************************/
	/* Form                                                               */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevHosLck");
	XtAddCallback(form_widget, XmNhelpCallback,
		                  HelpDisplayOpen, "devices,DevHosLck");
		                         
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_lock_host", &msg_header, 
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
		            HelpDisplayOpen, "devices,DevHosLck");
	CenterForm(form_widget);
}		                             

static int 
LoadVariables()
{
	Cardinal	n;
	Arg		args[10];
	int		i;
	char		**hosts;
	int		nhosts;
	XmString	*hosts_xmstrings;  /* XmStrings for all hosts */
                              
	printf ("Load Varaibles\n");
	/* Load in all hosts on the system */
	GetAllHosts(&nhosts, &hosts);

	printf ("There are %d hosts\n", nhosts);
	/* Malloc XmString arrays for the lists of all hosts */
	hosts_xmstrings = (XmString *) Malloc(sizeof(XmString) * nhosts);
	if (! hosts_xmstrings)
	        MemoryError();
        
	/* Load XmString arrays with values of valid hosts */
	for (i = 0; i < nhosts; i++) {
		hosts_xmstrings[i] = XmStringCreate(hosts[i], charset);
		if (! hosts_xmstrings[i])
			MemoryError();
	}
		
	/* Load in host values */
	n = 0;
	XtSetArg(args[n], XmNlistItemCount,                    nhosts); n++;
	XtSetArg(args[n], XmNlistItems,                hosts_xmstrings); n++;
	XtSetValues(selection_widget, args, n);

	/* Free memory */
	if (hosts) { 
		free_cw_table(hosts);
		hosts = NULL;
	}

	if (hosts_xmstrings) {
		for (i = 0; i < nhosts; i++) {
		    XmStringFree(hosts_xmstrings[i]);
		}
		free(hosts_xmstrings);
		hosts_xmstrings = NULL;
	}
	
	nhosts = 0;
	if (selected_host_name)
		free(selected_host_name);

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

	/* Save the host name */
	if (selected_host_name)
		free(selected_host_name);
	selected_host_name = strdup(extract_normal_string (info->value));

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
	char	*internet_add;
	int ret;

	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
	/* Read the devices protected password database entry */
	printf ("Host name is %s\n", selected_host_name);
	/* Convert to internet form */
	internet_add = hostname_to_internet(selected_host_name);
	printf ("Host add is %s\n", internet_add);
	ret = XGetTerminalInfo(internet_add, &dv);
	free (internet_add);
	if (ret != SUCCESS)
		return;

	/* Lock the host */
	dv.tm.uflg.fg_lock = 1;
	dv.tm.ufld.fd_lock = 1;

	/* Write the information back */
	ret = XWriteTerminalInfo(&dv);
	if (ret != SUCCESS)
		return;

	sa_audit_lock(ES_SET_TERM_LOCK, dv.tm.ufld.fd_devname);

	DevHosLckClose();
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
	DevHosLckClose();  
}

#endif /* SEC_BASE **/

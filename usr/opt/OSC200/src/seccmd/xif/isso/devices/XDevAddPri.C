/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifdef SecureWare
#ident "@(#)XDevAddPri.C	3.1 09:09:40 2/26/91 SecureWare"

/*
	filename:
		XDevAddPri.c
		
	copyright:
		Copyright (c) 1989, SecureWare Inc.
		ALL RIGHTS RESERVED
	
	function:
		X windows front end for DevAddPri function in role programs.
		
	entry points:
		DevAddPriStart()
		DevAddPriOpen()
		DevAddPriClose()
		DevAddPriStop()

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
	CreateSingleMultiLevel(),
	CreateScrolledWindow(),
	CreateScrolledWindowForm(),
	CreateTexts(),
	CreateItemText();

extern Dimension
	GetWidth();

extern int
	XIsNameNew(),
	XIsDeviceCharSpecial(),
	XIsDeviceAlreadyListed ();
	
extern void 
	SetToggle(),
	ErrorMessageOpen();

/* Local variables */
static Widget
	b11, b12, b21, b22,
	device_list_form,
	device_text_widget,
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

CREATE_SCREEN_ROUTINES (DevAddPriStart, DevAddPriOpen, DevAddPriClose,
	DevAddPriStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
		    work_area1_widget,
		    work_area3_frame,
		    work_area3_widget,
		    work_area4_frame,
		    work_area4_widget,
		    ok_button,
		    cancel_button,
		    help_button,
		    w,w1;
	int         i;
	Dimension   max_label_width;


	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevAddPri");
	XtAddCallback(form_widget, XmNhelpCallback,
				 HelpDisplayOpen, "devices,DevAddPri");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_add_printer", &msg_header, 
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
		msg_header[4], &max_label_width, PRINTER_NAME_LEN,
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
	/* Create the single/multi level widgets                              */
	/**********************************************************************/
#ifdef B1
	work_area3_frame = CreateSingleMultiLevel (form_widget, 
		w1, True, NULL,
		&b11, &b12, &b21, &b22);
#else
	work_area3_frame = w1;
#endif

	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons			     */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area3_frame,
			     &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "devices,DevAddPri");
}


CREATE_TEXT_CALLBACKS (def_value)
CREATE_CALLBACKS (msg_header[3],  DevAddPriClose)

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

#ifdef B1
	/* Set the single level, multi level toggles */
	SetAddMultiLevelToggle(b11, b12, b21, b22, df);
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
	device_list_widget = (Widget *) malloc(sizeof(Widget) *
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
	char *buf;
	char *name;
	char *list;
	struct dev_asg *dev;
	struct dev_asg *dev_tmp;
	int i;
	long t;
	int nerrs = 0;
	
	name = XmTextGetString (device_text_widget);
	if (XIsNameNew(name) == FAILURE)
		return (FAILURE);

	if (! XIsDeviceCharSpecial(name))
		return (FAILURE);

	device_list_name_count = 0;
	device_list_names = (char **) malloc ( sizeof(char *) * 
			device_list_widget_count);
	if (! device_list_names)
		MemoryError();
	for (i=0; i<device_list_widget_count; i++) {
		buf = XmTextGetString(device_list_widget[i]);
		if (buf[0] != '\0') {
			device_list_names[device_list_name_count] =
				(char *) malloc(strlen(buf) + 1);
			if (! device_list_names[device_list_name_count])
				MemoryError();

			strcpy(device_list_names[device_list_name_count], buf);
			device_list_name_count ++;
		}
		XtFree(buf);
	}
					
	/* If none listed then assume the name */
	if (device_list_name_count == 0) {
		device_list_names[0] = (char *) malloc(strlen(name) + 6);
		if (! device_list_names[0])
			MemoryError();

		sprintf (device_list_names[0], "/dev/%s", name);
		device_list_name_count = 1;
	}

	if (XIsDeviceAlreadyListed (NULL, device_list_names, 
		device_list_name_count))
		return (FAILURE);

	dev = &dv.dev;
	strncpy (dev, "", sizeof(*dev));

	/* Everything looks good - save the name and other parameters */
	dev->uflg.fg_name = 1;
	dev->ufld.fd_name = (char *) malloc (strlen(name) + 1);
	if (! dev->ufld.fd_name)
		MemoryError();

	strcpy (dev->ufld.fd_name, name);
#ifdef B1
	dev->uflg.fg_max_sl = 0;
	dev->uflg.fg_min_sl = 0;
	dev->uflg.fg_cur_sl = 0;
#ifdef ILB
	dev->uflg.fg_cur_il = 0;
#endif
#endif
	/* Copy the device list into the structure */
	dev->uflg.fg_devs = 1;
	dev->ufld.fd_devs = (char **) malloc 
			(sizeof(char *) * (device_list_name_count + 1) );
	if (! dev->ufld.fd_devs)
		MemoryError();
	for (i=0; i<device_list_name_count; i++) {
		dev->ufld.fd_devs[i] = (char *) malloc 
				(strlen(device_list_names[i]) + 1);
		if (! dev->ufld.fd_devs[i])
			MemoryError();
		strcpy(dev->ufld.fd_devs[i], device_list_names[i]);
	}
	dev->ufld.fd_devs[device_list_name_count] = '\0';

	dev->uflg.fg_type = 1;
	/* Clear type field and then set to REMOTE */
	for (i = 0; i<= AUTH_MAX_DEV_TYPE; i++) 
		RMBIT (dev->ufld.fd_type, i);
	ADDBIT (dev->ufld.fd_type, AUTH_DEV_PRINTER);

#ifdef B1
	/* Depending on multi level bits we set these */
	dev->uflg.fg_assign = 1;
	for (i = 0; i<= AUTH_MAX_DEV_ASSIGN; i++) 
		RMBIT (dev->ufld.fd_assign, i);

#ifndef SHW
	if (XmToggleButtonGadgetGetState (b11))
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_SINGLE);
	else
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_MULTI);
#endif
#ifdef ILB
	if (XmToggleButtonGadgetGetState (b21))
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_ILSINGLE);
	else
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_ILMULTI);
#endif

#endif

	return (SUCCESS);
}

static int
WriteInformation ()
{
	int ret;

	/* Write the device database */
	ret = XWriteDeviceInfo(&dv);
	return (ret);
}

#endif /* SecureWare */

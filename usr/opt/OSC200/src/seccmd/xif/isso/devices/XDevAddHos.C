/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifdef SecureWare
#ifdef SEC_NET_TTY
#ident "@(#)XDevAddHos.c	1.1.1.5 17:36:34 1/9/90 SecureWare"

/*
	filename:
		XDevAddHos.c
		
	copyright:
		Copyright (c) 1989, SecureWare Inc.
		ALL RIGHTS RESERVED
	
	function:
		X windows front end for DevAddHos function in role programs.
		
	entry points:
		DevAddHosStart()
		DevAddHosOpen()
		DevAddHosClose()
		DevAddHosStop()

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
	XIsDeviceHost(),
	XIsNameNew(),
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
mod 666*msg_device_login_toggle_text,
	*msg_device_login_value_text,
	*msg_error_text;

/* Definitions */
#define NUM_DEVICE_ADD_HOST_VALUES		3
#define NUM_DEVICE_ADD_HOST_TOGGLES		3

static int device_login_text_col[NUM_DEVICE_ADD_HOST_VALUES] = 
	{3,3,3,};

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevAddHosStart, DevAddHosOpen, DevAddHosClose,
	DevAddHosStop)

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
	Arg         args[20];
	Cardinal    n;
	int         i;
	Dimension   max_label_width;


	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevAddHos");
	XtAddCallback(form_widget, XmNhelpCallback,
					 HelpDisplayOpen, "devices,DevAddHos");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	LoadMessage ("msg_devices_add_host", &msg_header, 
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
		msg_header[4], &max_label_width, HOST_NAME_LEN,
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
	def_value     	 = MallocInt(NUM_DEVICE_ADD_HOST_VALUES);
	value     	 = MallocInt(NUM_DEVICE_ADD_HOST_VALUES);
	value_state      = MallocChar(NUM_DEVICE_ADD_HOST_VALUES);
	toggle_state     = MallocChar(NUM_DEVICE_ADD_HOST_TOGGLES);
	def_toggle_state = MallocChar(NUM_DEVICE_ADD_HOST_TOGGLES);
	text_default_widget = MallocWidget(NUM_DEVICE_ADD_HOST_VALUES);
	text_widget 	 = MallocWidget(NUM_DEVICE_ADD_HOST_VALUES);
	yes_widget 	 = MallocWidget(NUM_DEVICE_ADD_HOST_TOGGLES);
	default_widget   = MallocWidget(NUM_DEVICE_ADD_HOST_TOGGLES);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	LoadMessage("msg_devices_add_host_value", &msg_device_login_value, 
		&msg_device_login_value_text);
	max_label_width = (Dimension) 0;

	CreateItemsTextD(work_area2_widget, 0, NUM_DEVICE_ADD_HOST_VALUES,
		msg_device_login_value, &max_label_width, 
		device_login_text_col, text_widget, text_default_widget);

	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_DEVICE_ADD_HOST_VALUES; i++) {
		XtAddCallback (text_widget[i], XmNvalueChangedCallback, 
				 TextCallback, i);
		XtAddCallback (text_default_widget[i], XmNvalueChangedCallback, 
				 TextDefaultCallback, i);
	}
	/**********************************************************************/
	/* Create the single/multi level widgets                              */
	/**********************************************************************/
	work_area3_frame = CreateSingleMultiLevel (form_widget, 
		work_area2_frame, True, NULL,
		&b11, &b12, &b21, &b22);

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], 
		work_area2_frame, False, work_area3_frame);

	max_label_width = GetWidth(w) + (Dimension) 10;
	/**********************************************************************/
	/*
	/* Create all the Toggle button widgets                               */
	/*
	/**********************************************************************/
	work_area4_frame  = CreateFrame(form_widget, w, False, work_area3_frame);
	work_area4_widget = CreateSecondaryForm(work_area4_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	LoadMessage("msg_devices_add_host_toggle", 
		&msg_device_login_toggle, &msg_device_login_toggle_text);

	CreateItemsYND(work_area4_widget, 0, 
		NUM_DEVICE_ADD_HOST_TOGGLES,
		msg_device_login_toggle, &max_label_width, yes_widget,
		default_widget);

	/* Import/Export have no default toggles */
	XtUnmanageChild(default_widget[1]);
	XtUnmanageChild(default_widget[2]);

	/**********************************************************************/
	/* Create callbacks N.B. note that last two have no callbacks         */
	/**********************************************************************/
	for (i=0; i<(NUM_DEVICE_ADD_HOST_TOGGLES-2); i++) {
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
	CreateThreeButtons (form_widget, work_area4_frame,
			     &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "devices,DevAddHos");
}


CREATE_TEXT_CALLBACKS (def_value)
CREATE_ON_CALLBACKS (def_toggle_state, False, NUM_DEVICE_ADD_HOST_TOGGLES)
CREATE_CALLBACKS (msg_header[3],  DevAddHosClose)

static int
LoadVariables ()
{
	int i;
	int ret;
	struct pr_default *df = &sd.df;
	Arg         args[20];
	Cardinal     n;

	/* Load the  value defaults */
	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);

	/* Set all values/buttons to default */
	for (i=0; i<NUM_DEVICE_ADD_HOST_VALUES; i++) {
		value_state[i] = YES_CHAR;
	}

	for (i=0; i<NUM_DEVICE_ADD_HOST_TOGGLES; i++) {
		toggle_state[i] = DEFAULT_CHAR;
	}
	toggle_state[1] = YES_CHAR;
	toggle_state[2] = YES_CHAR;

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

	/* Not used, but just in case set to true */
	def_toggle_state[1] = YES_CHAR;
	def_toggle_state[2] = YES_CHAR;

	/* All values should be default first time in */
	in_change_mode = False;
	for (i=0; i<NUM_DEVICE_ADD_HOST_VALUES; i++) {
		SetTextD(value[i], value_state[i], def_value[i], 
			text_widget[i], text_default_widget[i]);
	}
	in_change_mode = True;

	for (i=0; i<NUM_DEVICE_ADD_HOST_TOGGLES; i++) {
		SetYND(toggle_state[i], def_toggle_state[i],
			yes_widget[i], default_widget[i]);
	}

	/* Set the single level, multi level toggles */
	SetAddMultiLevelToggle(b11, b12, b21, b22, df);

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
	return (SUCCESS);
}

static int
ValidateEntries ()
{
	char *buf;
	char *name;
	char *list;
	struct dev_asg *dev;
	struct dev_asg *dev_tmp;
	struct pr_term *tm;
	struct pr_term *tm_tmp;
	int i;
	long t;
	int nerrs = 0;
	
	name = XmTextGetString (device_text_widget);
	if (XIsNameNew(name) == FAILURE)
		return (FAILURE);

	if (! XIsDeviceHost(name))
		return (FAILURE);

	device_list_name_count = 0;
	device_list_names = (char *) malloc ( sizeof(char *) * 
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

	dev_tmp = getdvagent();
	tm_tmp  = getprtcent();

	dv.dev  = copydvagent(dev_tmp);
	dv.tm   = *tm_tmp;

	dev = &dv.dev;
	tm = &dv.tm;

	/* Check the text input fields */
	if (XmToggleButtonGadgetGetState(text_default_widget[0])) {
		tm->uflg.fg_max_tries = 0;
		tm->ufld.fd_max_tries = tm->sfld.fd_max_tries;
	}
	else {
		tm->uflg.fg_max_tries = 1;
		buf = XmTextGetString(text_widget[0]);
		i = atoi (buf);
		if (i >= 0 && i <= 999)
			tm->ufld.fd_max_tries = (ushort) (i);
		else
			nerrs ++;

		XtFree(buf);
	}


	if (XmToggleButtonGadgetGetState(text_default_widget[1])) {
		tm->uflg.fg_login_timeout = 0;
		tm->ufld.fd_login_timeout = tm->sfld.fd_login_timeout;
	}
	else {
		tm->uflg.fg_login_timeout = 1;
		buf = XmTextGetString(text_widget[1]);
		i = atoi (buf);
		if (i >= 0 && i <= 999)
			tm->ufld.fd_login_timeout = (ushort) i;
		else
			nerrs ++;

		XtFree(buf);
	}


	if (XmToggleButtonGadgetGetState(text_default_widget[2])) {
		tm->uflg.fg_logdelay = 0;
		tm->ufld.fd_logdelay = tm->sfld.fd_logdelay;
	}
	else {
		tm->uflg.fg_logdelay = 1;
		tm->uflg.fg_login_timeout = 1;
		buf = XmTextGetString(text_widget[2]);
		t = atol (buf);
		if (t >= 0 && t <= 999)
			tm->ufld.fd_logdelay = t;
		else
			nerrs ++;

		XtFree(buf);
	}

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
	dev->ufld.fd_devs = (char *) malloc 
			(sizeof(char *) * (device_list_name_count + 1) );
	if (! dev->ufld.fd_devs)
		MemoryError();
	for (i=0; i<device_list_name_count; i++) {
		dev->ufld.fd_devs[i] = (char *) malloc 
				(strlen(device_list_names[i] + 1));
		if (! dev->ufld.fd_devs[i])
			MemoryError();
		strcpy(dev->ufld.fd_devs[i], device_list_names[i]);
	}
	dev->ufld.fd_devs[device_list_name_count] = '\0';

	dev->uflg.fg_type = 1;
	/* Clear type field and then set to REMOTE */
	for (i = 0; i<= AUTH_MAX_DEV_TYPE; i++) 
		RMBIT (dev->ufld.fd_type, i);
	ADDBIT (dev->ufld.fd_type, AUTH_DEV_REMOTE);

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

	if (XmToggleButtonGadgetGetState (yes_widget[1]))
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_IMPORT);

	if (XmToggleButtonGadgetGetState (yes_widget[2]))
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_EXPORT);
#endif

	tm->uflg.fg_devname = 1;
	strcpy (tm->ufld.fd_devname, name);
	tm->uflg.fg_uid = 0;
	tm->uflg.fg_slogin = 0;
	tm->uflg.fg_uuid = 0;
	tm->uflg.fg_ulogin = 0;
	tm->uflg.fg_loutuid = 0;
	tm->uflg.fg_louttime = 0;
	tm->uflg.fg_nlogins = 0;
	tm->uflg.fg_label = 0;

	/* Lock toggle */
	if (XmToggleButtonGadgetGetState(default_widget[0])) {
		tm->uflg.fg_lock = 0;
		tm->ufld.fd_lock = 1;
	}
	else {
		tm->uflg.fg_lock = 1;
		if (XmToggleButtonGadgetGetState(yes_widget[0]))
			 tm->ufld.fd_lock = 1;
		else     tm->ufld.fd_lock = 0;
	}

	return (SUCCESS);
}

static int
WriteInformation ()
{
	int ret;

	ret = XWriteDeviceAndTerminalInfo(&dv);
	return (ret);
}

#endif /* SEC_NET_TTY */
#endif /* SecureWare */

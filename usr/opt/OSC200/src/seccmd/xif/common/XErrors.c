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
static char	*sccsid = "@(#)$RCSfile: XErrors.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:04:50 $";
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
		XErrors.c
	
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		provide X user interface common error mechanisms

	define's used:
		SUPPORT_ERROR_NUMBER
		TRAVERSAL
		
	notes:
		to provide easy transportability into other systems, not 
		dependent upon message text file, instead text which the
		user may see has been added at the top of the file
		
	entry points:
		CodeError()
		
		ConfirmationStart();
		ConfirmationOpen();
		ConfirmationClose();
		ConfirmationStop();

		ErrorMessageStart();
		ErrorMessageOpen();
		ErrorMessageClose();
		ErrorMessageStop();

		PleaseWaitOpen();
		PleaseWaitClose();
		
		SystemErrorMessageOpen();   Routed through WaitErrorMessageOpen
		WaitConfirmationOpen();     Supports inline confirmation
		WaitErrorMessageOpen();     Supports inline confirmation

	crucial parameters:
		ConfirmationOpen(class, i_index, charptr, ok_callback, cancel_callback)
		ErrorMessageOpen(tag, class, i_index, charptr)
		void PleaseWaitOpen(char **class, int i_index)
		SystemErrorMessageOpen(tag, class, i_index, charptr)

		int WaitConfirmationOpen(class, i_index, charptr) 
		TRUE == OK, FALSE == CANCEL
		void WaitErrorMessageOpen(tag, class, i_index, charptr)
*/

/* Common C include files */
#include <sys/types.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <malloc.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>

/* include files for the main program this is linked with */
#include "XMain.h"

/* Needed functional declarations */
extern void
		MemoryError(),
		StopFunctionalUnits();

void    SystemErrorMessageOpen(),
		WaitErrorMessageOpen();

static void
		ConfirmationClose(),
		ErrorMessageCallback(),
		WaitCallback();

/* Local variables */
static int
		confirm_open_flag,
		error_open_flag,
		please_wait_open_flag,
		user_wait;

static Widget
		confirm_cancel_widget,
		confirm_form_widget,
		confirm_list_widget,
		confirm_ok_widget,
		error_form_widget,
		error_list_widget,
		please_wait_form_widget,
		please_wait_list_widget;
		
static char 
		*msg[] = {
		 /* 0*/ "Insufficient memory.",
		 /* 1*/ "Code error!",
		 /* 2*/ "OK",
		 /* 3*/ "Cancel",
		 /* 4*/ "Help",
		 /* 5*/ "(Message Number %d)",
		 /* 6*/ "(System Error Number %d)",
				""
				} ;
	
void CodeError()
{
	/* AUDIT code failure */
	audit_no_resource("Role program", OT_PROCESS, "Bad code trapped");
	/* Post dialog with error message */
	SystemErrorMessageOpen(1310, msg, 1, NULL);
	/* Dies */
}

void ConfirmationStart() 
{
	confirm_open_flag = False;
}

void 
ConfirmationOpen(class, i_index, charptr, ConfirmOKCallback, ConfirmCancelCallback)
	char      **class;
	int         i_index;
	char       *charptr;
	void       (*ConfirmOKCallback)(),
			   (*ConfirmCancelCallback)();
{
	int         i,
			count;
	Arg         args[20];
	Cardinal    n;
	Widget      action_button_widget,
			separator_widget;
	XmString    xmstring,
			*xmstrings;
	char        buf[PATH_MAX+200];

	XtSetSensitive(mother_form, False);

	/* Build widgets if not laying around */
	if (! confirm_open_flag) {
		/* Form */
		n = 0;
		XtSetArg(args[n], XmNtopAttachment,    XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,        main_menubar); n++;
		XtSetArg(args[n], XmNtopOffset,        10); n++;
		XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftOffset,       10); n++;
		XtSetArg(args[n], XmNautoUnmanage,     FALSE); n++;
		XtSetArg(args[n], XmNborderWidth,      1); n++;
		confirm_form_widget = XtCreateWidget("Confirm", xmFormWidgetClass, 
				main_form, args, n);
			
		/* List */
		n = 0;
		XtSetArg(args[n], XmNsensitive,        FALSE); n++;
		XtSetArg(args[n], XmNborderWidth,      0); n++;
		XtSetArg(args[n], XmNshadowThickness,  0); n++;
		XtSetArg(args[n], XmNlistMarginWidth,  5); n++;
		XtSetArg(args[n], XmNlistMarginHeight, 5); n++;
		XtSetArg(args[n], XmNtopAttachment,    XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
		confirm_list_widget = XtCreateWidget("List", 
				xmListWidgetClass, confirm_form_widget, args, n);
		
		/* Separator bar */
		n = 0;
		XtSetArg(args[n], XmNorientation,     XmHORIZONTAL); n++;
		XtSetArg(args[n], XmNseparatorType,   XmSINGLE_LINE); n++;
		XtSetArg(args[n], XmNtopAttachment,   XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,       confirm_list_widget); n++;
		XtSetArg(args[n], XmNleftAttachment,  XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftOffset,      0); n++;
		XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNrightOffset,     0); n++;
		separator_widget = XmCreateSeparatorGadget(confirm_form_widget, "Separator", 
				args, n);

		/* Row Column for action buttons */
		n = 0;
		XtSetArg(args[n], XmNpacking,         XmPACK_COLUMN); n++;
		XtSetArg(args[n], XmNorientation,     XmHORIZONTAL); n++;
		XtSetArg(args[n], XmNadjustLast,      False); n++;
		XtSetArg(args[n], XmNentryAlignment,  XmALIGNMENT_CENTER); n++;
		XtSetArg(args[n], XmNspacing,         20); n++;
		XtSetArg(args[n], XmNtopAttachment,   XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,       separator_widget); n++;
		XtSetArg(args[n], XmNleftAttachment,  XmATTACH_FORM); n++;
		action_button_widget = XmCreateRowColumn(confirm_form_widget, 
				"Button", args, n);

		/* OK button */    
		xmstring = XmStringCreate(msg[2], charset);
		if (! xmstring)
			MemoryError();
		n = 0;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,     True); n++;
#endif
		XtSetArg(args[n], XmNlabelString,     xmstring); n++;
		XtSetArg(args[n], XmNalignment,       XmALIGNMENT_CENTER); n++;
		confirm_ok_widget = XmCreatePushButtonGadget(
	                action_button_widget, "Button", args, n);
		XmStringFree(xmstring);
	
		/* Cancel button */    
		xmstring = XmStringCreate(msg[3], charset);
		if (! xmstring)
			MemoryError();
		n = 0;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,   True); n++;
#endif
		XtSetArg(args[n], XmNlabelString,   xmstring); n++;
		XtSetArg(args[n], XmNalignment,     XmALIGNMENT_CENTER); n++;
		confirm_cancel_widget = XmCreatePushButtonGadget(
	                action_button_widget, "Button", args, n);
		XmStringFree(xmstring);

		/* Build in good sequence to help size of form */
		XtManageChild(confirm_list_widget);
		XtManageChild(separator_widget);
		XtManageChild(confirm_ok_widget);
		XtManageChild(confirm_cancel_widget);
		XtManageChild(action_button_widget);
		
		confirm_open_flag = TRUE;
	}
	else {
		/* Update the callback lists */
		XtRemoveAllCallbacks(confirm_ok_widget, XmNactivateCallback);
		XtRemoveAllCallbacks(confirm_cancel_widget, XmNactivateCallback);
	}
	
	/* Update the callback lists */
	XtAddCallback(confirm_ok_widget, XmNactivateCallback, 
	                       ConfirmationClose, NULL);
	XtAddCallback(confirm_ok_widget, XmNactivateCallback,       
	                       ConfirmOKCallback, "OK");
	XtAddCallback(confirm_cancel_widget, XmNactivateCallback, 
	                       ConfirmationClose, NULL);
	XtAddCallback(confirm_cancel_widget, XmNactivateCallback, 
	                       ConfirmCancelCallback, "Cancel");
	/* Load message text data into list widget */
	count = 0;
	for (i = i_index; class[i] && *class[i]; i++)
		count++;
	xmstrings = (XmString *) Malloc(sizeof(XmString) * count);
	if (! xmstrings)
		MemoryError();
	for (i = 0; i < count; i++) {
		sprintf(buf, class[i+i_index], charptr);
		xmstrings[i] = XmStringCreate(buf, charset);
		if (! xmstrings[i])
			MemoryError();
	}
	
	n = 0;
	XtSetArg(args[n], XmNitems,                         xmstrings); n++;
	XtSetArg(args[n], XmNitemCount,                         count); n++;
	XtSetArg(args[n], XmNvisibleItemCount,                  count); n++;
	XtSetValues(confirm_list_widget, args, n);
	for (i = 0; i < count; i++)
		XmStringFree(xmstrings[i]);
	free(xmstrings);
	XtRealizeWidget(confirm_form_widget);
	XtManageChild(confirm_form_widget);
}  

static void 
ConfirmationClose() 
{
	XtUnmanageChild(confirm_form_widget);
	XtSetSensitive(mother_form, True);
	if (save_memory) {
		if (confirm_open_flag) {
			confirm_open_flag = FALSE;
			XtDestroyWidget(confirm_form_widget);
		}
	}
}

void 
ConfirmationStop() 
{
	if (confirm_open_flag) {
		confirm_open_flag = FALSE;
		XtDestroyWidget(confirm_form_widget);
	}    
}  

void 
ErrorMessageStart() 
{
	error_open_flag = False;
}

void 
ErrorMessageOpen(tag, class, i_index, charptr)
	int         tag;
	char      **class;
	int         i_index;
	char       *charptr;
{
	int         i,
				count_msg,
				count_total;
	Arg         args[20];
	Cardinal    n;
	Widget      action_button_widget,
				ok_widget,
				separator_widget;
	XmString    xmstring,
				*xmstrings;
	char        buf[PATH_MAX+200];
	
	XtSetSensitive(mother_form, False);

	/* Build widgets if not laying around */
	if (! error_open_flag) {
	
		/* Form */
		n = 0;
		XtSetArg(args[n], XmNbottomAttachment,   XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNbottomOffset,       10); n++;
		XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNrightOffset,        10); n++;
		XtSetArg(args[n], XmNautoUnmanage,       FALSE); n++;
		XtSetArg(args[n], XmNborderWidth,        1); n++;
		error_form_widget = XtCreateWidget("Warning", 
				xmFormWidgetClass, main_form, args, n);
			
		/* List */
		n = 0;
		XtSetArg(args[n], XmNsensitive,          FALSE); n++;
		XtSetArg(args[n], XmNborderWidth,        0); n++;
		XtSetArg(args[n], XmNshadowThickness,    0); n++;
		XtSetArg(args[n], XmNlistMarginWidth,    5); n++;
		XtSetArg(args[n], XmNlistMarginHeight,   5); n++;
		XtSetArg(args[n], XmNtopAttachment,      XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNlistSizePolicy,     XmVARIABLE); n++;
		error_list_widget = XtCreateWidget("List", xmListWidgetClass,
				error_form_widget, args, n);

		/* Separator bar */
		n = 0;
		XtSetArg(args[n], XmNorientation,      XmHORIZONTAL); n++;
		XtSetArg(args[n], XmNseparatorType,    XmSINGLE_LINE); n++;
		XtSetArg(args[n], XmNtopAttachment,    XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,        error_list_widget); n++;
		XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftOffset,       0); n++;
		XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNrightOffset,      0); n++;
		separator_widget = XmCreateSeparatorGadget(error_form_widget, 
				            "Separator", args, n);

		/* Row Column for action buttons */
		n = 0;
		XtSetArg(args[n], XmNpacking,          XmPACK_COLUMN); n++;
		XtSetArg(args[n], XmNorientation,      XmHORIZONTAL); n++;
		XtSetArg(args[n], XmNadjustLast,       False); n++;
		XtSetArg(args[n], XmNentryAlignment,   XmALIGNMENT_CENTER); n++;
		XtSetArg(args[n], XmNspacing,          20); n++;
		XtSetArg(args[n], XmNtopAttachment,    XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,        separator_widget); n++;
		XtSetArg(args[n], XmNrightAttachment,  XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
		action_button_widget = XmCreateRowColumn(error_form_widget, 
				"Button", args, n);

		/* OK button */    
		xmstring = XmStringCreate("OK", charset);
		if (! xmstring)
			MemoryError();
		n = 0;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,    True); n++;
#endif
		XtSetArg(args[n], XmNlabelString,    xmstring); n++;
		XtSetArg(args[n], XmNalignment,      XmALIGNMENT_CENTER); n++;
		ok_widget = XmCreatePushButtonGadget(action_button_widget, "Button", args, n);
		XtAddCallback(ok_widget, XmNactivateCallback, ErrorMessageCallback, NULL);
		XmStringFree(xmstring);

		/* Build in good sequence to help size of form */
		XtManageChild(error_list_widget);
		XtManageChild(separator_widget);
		XtManageChild(ok_widget);
		XtManageChild(action_button_widget);
	
		error_open_flag = TRUE;
	}
	
	/* Load message text data into list widget */
	count_msg = 0;
	for (i = i_index; class[i] && *class[i]; i++)
		count_msg++;
	count_total = count_msg;
#ifdef SUPPORT_ERRROR_NUMBER
	if (tag != -1)    
		count_total++;
#endif

	/* Build XmStrings for list */
	xmstrings = (XmString *) Malloc(sizeof(XmString) * count_total);
	if (! xmstrings)
		MemoryError();
	for (i = 0; i < count_msg; i++) {
		sprintf(buf, class[i+i_index], charptr);
		xmstrings[i] = XmStringCreate(buf, charset);
		if (! xmstrings[i])
			MemoryError();
	}
#ifdef SUPPORT_ERROR_NUMBER
	if (tag != -1) {
		sprintf(buf, msg[5], tag);
		xmstrings[i] = XmStringCreate(buf, charset);
		if (! xmstrings[i])
			MemoryError();
	}
#endif

	/* Roll list text into list widget */
	n = 0;
	XtSetArg(args[n], XmNitems,                         xmstrings); n++;
	XtSetArg(args[n], XmNitemCount,                   count_total); n++;
	XtSetArg(args[n], XmNvisibleItemCount,            count_total); n++;
	XtSetValues(error_list_widget, args, n);
	for (i = 0; i < count_total; i++)
		XmStringFree(xmstrings[i]);
	free(xmstrings);
	XtRealizeWidget(error_form_widget);
	XtManageChild(error_form_widget);
}  

void 
ErrorMessageClose() 
{
	XtUnmanageChild(error_form_widget);
	XtSetSensitive(mother_form, True);
	/* 
	 * If the error message occurs before a form is displayed then we
	 * need to be able to see the menubar after dismissing the error
	 */
	if (no_form_present) {
		XtSetSensitive(main_menubar, True);
		no_form_present = False;
	}
	if (save_memory) {
		if (error_open_flag) {
			error_open_flag = FALSE;
			XtDestroyWidget(error_form_widget);
		}
	}
	user_wait = FALSE;
}

void 
ErrorMessageStop() 
{
	if (error_open_flag) {
		error_open_flag = FALSE;
		XtDestroyWidget(error_form_widget);
	}    
}  

static void 
ErrorMessageCallback(w, ptr, info)
	Widget      w;
	char        *ptr;
	XmAnyCallbackStruct *info;
{
	ErrorMessageClose();        
}

void 
SystemErrorMessageOpen(tag, class, i_index, charptr)
	int         tag;
	char        **class;
	int         i_index;
	char        *charptr;
{
	WaitErrorMessageOpen(tag, class, i_index, charptr);
	/* Crash out */
	StopFunctionalUnits();
	XtDestroyWidget(main_shell);
	exit(1);
}

/**************************************************************************/
/******** Please wait while ... type message ******************************/
/**************************************************************************/

void 
PleaseWaitOpen(class, i_index)
	char        **class;
	int         i_index;
{
	int         i,
				count_msg;
	Arg         args[20];
	Cardinal    n;
	XmString    xmstring,
				*xmstrings;
	
	/* Build widgets if not laying around */
	if (! please_wait_open_flag) {
	
		/* Form */
		n = 0;
		XtSetArg(args[n], XmNtopAttachment,    XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNborderWidth,      2); n++;
		please_wait_form_widget = XtCreateWidget("PleaseWait", 
				xmFormWidgetClass, main_form, args, n);

		/* List */
		n = 0;
		XtSetArg(args[n], XmNsensitive,        FALSE); n++;
		XtSetArg(args[n], XmNborderWidth,      0); n++;
		XtSetArg(args[n], XmNshadowThickness,  0); n++;
		XtSetArg(args[n], XmNtopAttachment,    XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
		please_wait_list_widget = XtCreateWidget("List", 
			xmListWidgetClass, please_wait_form_widget, args, n);
		XtManageChild(please_wait_list_widget);

		please_wait_open_flag = TRUE;
	}
	
	/* Load message text data into list widget */
	count_msg = 0;
	for (i = i_index; class[i] && *class[i]; i++)
		count_msg++;

	/* Build XmStrings for list */
	xmstrings = (XmString *) Malloc(sizeof(XmString) * count_msg);
	if (! xmstrings)
		MemoryError();
	for (i = 0; i < count_msg; i++) {
		xmstrings[i] = XmStringCreate(class[i+i_index], charset);
		if (! xmstrings[i])
			MemoryError();
	}

	/* Roll list text into list widget */
	n = 0;
	XtSetArg(args[n], XmNitems,                         xmstrings); n++;
	XtSetArg(args[n], XmNitemCount,                     count_msg); n++;
	XtSetArg(args[n], XmNvisibleItemCount,              count_msg); n++;
	XtSetValues(please_wait_list_widget, args, n);
	for (i = 0; i < count_msg; i++)
		XmStringFree(xmstrings[i]);
	free(xmstrings);
	
	XtRealizeWidget(please_wait_form_widget);
	XtManageChild(please_wait_form_widget);
	XmUpdateDisplay(main_shell);
}  

void 
PleaseWaitClose() 
{
	XtUnmanageChild(please_wait_form_widget);
	XmUpdateDisplay(main_shell);
	if (save_memory) {
		if (please_wait_open_flag) {
			please_wait_open_flag = FALSE;
			XtDestroyWidget(please_wait_form_widget);
		}
	}
}

/**************************************************************************/
/******** WAIT CONFIRM AND WAIT ERROR FOLLOW ******************************/
/**************************************************************************/

static int  ok_response;

static void 
WaitCallback(w, ptr, info)
	Widget      w;
	char        *ptr;
	XmAnyCallbackStruct *info;
{
	ok_response = ! strcmp(ptr, "OK");
	user_wait = FALSE;
}

static Cursor working_cursor = NULL;

int 
WaitConfirmationOpen(class, i_index, charptr)
	char      **class;
	int         i_index;
	char       *charptr;
{
	int         working_cursor_active;
	XEvent      event;
	XtAppContext context;

#ifdef OLD_CODE
	extern Cursor working_cursor;
#endif
	user_wait = TRUE;
	working_cursor_active = (working_cursor != NULL);
	ConfirmationOpen(class, i_index, charptr, WaitCallback, WaitCallback);
	if (working_cursor_active)
		WorkingClose();
	
	/* trap and dispatch events from our application only */
	context = XtWidgetToApplicationContext(main_shell); 
	while (user_wait) {
		XtAppNextEvent(context, &event);
		XtDispatchEvent(&event);
	}
	if (working_cursor_active)
		WorkingOpen(main_shell);

	/* Insure display updated */
	XmUpdateDisplay(main_shell);

	return ok_response;
}

void 
WaitErrorMessageOpen(tag, class, i_index, charptr)
	int         tag;
	char      **class;
	int         i_index;
	char       *charptr;
{
#ifdef OLD_CODE
	extern Cursor working_cursor;
#endif
	int         working_cursor_active;
	XEvent      event;
	XtAppContext context;

	user_wait = TRUE;
	working_cursor_active = (working_cursor != NULL);
	ErrorMessageOpen(tag, class, i_index, charptr);
	if (working_cursor_active)
		WorkingClose();
	
	/* trap and dispatch events from our application only */
	context = XtWidgetToApplicationContext(main_shell); 
	while (user_wait) {
		XtAppNextEvent(context, &event);
		XtDispatchEvent(&event);
	}
	if (working_cursor_active)
		WorkingOpen(main_shell);

	/* Insure display updated */
	XmUpdateDisplay(main_shell);
}  
#endif /* SEC_BASE */

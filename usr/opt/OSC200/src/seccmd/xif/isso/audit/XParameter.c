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
static char	*sccsid = "@(#)$RCSfile: XParameter.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:46 $";
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

#ifdef SEC_BASE

/*
	filename:
		XParameter.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X window audit interface code to Parameters maintenance
		
	entry points:
		XParametersStart()
		XParametersOpen()
		XParametersClose()
		XParametersStop()
*/
#include "XAudit.h"
#include "Xm/Form.h"
#include "Xm/LabelG.h"
#include "Xm/MessageB.h"
#include "Xm/RowColumn.h"
#include "Xm/Text.h"
#include "Xm/ToggleB.h"

extern int	
		AuditParametersGet();

int           CheckAuditEnabled();

static char   **msg,
			*msg_text;

static Widget form_widget,
			confirmation_widget,
			daemon_bytes_edit_widget,
			collection_buffers_edit_widget,
			audit_bytes_edit_widget,
			compacted_yes_widget,
			compacted_no_widget,
			enable_yes_widget,
			enable_no_widget,
			shutdown_yes_widget,
			shutdown_no_widget,
			confirm_this_session_widget,
			confirm_next_session_widget;
			  
static Display  
			*display;

static int		confirmation_open,
			form_open;
			  
static char		audit_bytes[25],
			collection_buffers[25],
			daemon_bytes[25];

static  audparm_fillin  param_struct;

static void   CancelCallback(),
			  ConfirmCancelCallback(),
			  ConfirmOKCallback(),
			  MakeWidgets(),
			  OKCallback();

void 
ParametersStart() 
{
	confirmation_open = FALSE;
	form_open = FALSE;
}

/**********************************************************************/
/* ParameterOpen is called when Parameters is selected from the main */
/* menu.  It sets up the parameter window, gets the parameter values  */
/* and displays them.                                                 */
/**********************************************************************/

void 
ParametersOpen()
{
	Cardinal    n;
	Arg         args[20];
	char        **new_table;
	int         audit_active,
			fd,
			i;
	
	/* Make menubar insensitive */
	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);    

	/* Load messages (if first time through) */
	if (! msg)
		LoadMessage("msg_isso_audit_param", &msg, &msg_text);
 
	/* Get audit parameters; put in param_struct */
	if (AuditParametersGet(&param_struct)) {
		XtSetSensitive (main_menubar, True);
		WorkingClose();
		return;
	}
	
	/* Copy integer parameter values to strings */
	sprintf(daemon_bytes, "%u", param_struct.au.read_count);
	sprintf(collection_buffers, "%u", param_struct.au.max_memory);
	sprintf(audit_bytes, "%ld", param_struct.au.caf_maxsize); 
	
#ifdef DEBUG
	printf ("Audit XParameters\n");
	printf ("read %d\n", param_struct.au.read_count);
	printf ("collection_buffers %u", param_struct.au.max_memory);
	printf ("audit_bytes %ld", param_struct.au.caf_maxsize); 
#endif

	if (! form_open) {
		/* If first time through, make widgets */
		MakeWidgets();
		form_open = TRUE;
		display = XtDisplay(main_shell);
	}

	/* Load data into widgets */
	XmTextSetString(daemon_bytes_edit_widget,             daemon_bytes);
	XmTextSetString(collection_buffers_edit_widget, collection_buffers);
	XmTextSetString(audit_bytes_edit_widget,               audit_bytes);
	
	/* Set toggle buttons for boolean parameters */
	XmToggleButtonSetState(compacted_yes_widget,
		   param_struct.compacted, False);
	XmToggleButtonSetState(compacted_no_widget, 
		   !param_struct.compacted, False);
	XmToggleButtonSetState(enable_yes_widget, 
		   param_struct.audit_on_startup, False);        
	XmToggleButtonSetState(enable_no_widget, 
		   !param_struct.audit_on_startup, False);        
	XmToggleButtonSetState(shutdown_yes_widget, 
		   param_struct.shut_or_panic, False);        
	XmToggleButtonSetState(shutdown_no_widget, 
		   !param_struct.shut_or_panic, False);        
						   
#ifdef DEBUG
	printf ("audit states %d %d %d\n",
		   param_struct.compacted, 
		   param_struct.audit_on_startup, 
		   param_struct.shut_or_panic );        
#endif

	/* Determine audit state to set sensitivity on buttons */
	audit_active = CheckAuditEnabled();

	/* Set this/next session toggles off */
	XmToggleButtonSetState(confirm_this_session_widget, False, False);
	XtSetSensitive(confirm_this_session_widget, audit_active);
	XmToggleButtonSetState(confirm_next_session_widget, True,  False);
	
	CenterForm(form_widget);
	WorkingClose();
}

/**********************************************************************/
/* XParamterClose closes the parameter display, and returns control to*/
/* the main menu.  It is called whenever the user exits the parameter */
/* window.                                                            */
/**********************************************************************/

void 
ParametersClose() 
{
	
	XSync(display, 0);

	XtSetSensitive(main_menubar, True);
	XtUnmanageChild(form_widget);
	if (save_memory) {
		if (form_open) {
			XtDestroyWidget(form_widget);
			form_open = FALSE;
		}
		if (confirmation_open) {
			XtDestroyWidget(confirmation_widget);
			confirmation_open = FALSE;
		}
	}
	XSync(display, 0);
}

/*************************************************************************/
/* ParameterStop is called when Xaudit is exited to clean up loose ends */
/*************************************************************************/

void 
ParametersStop() 
{
	if (form_open) {
		XtDestroyWidget(form_widget);
		form_open = FALSE;
	}
	if (confirmation_open) {
		XtDestroyWidget(confirmation_widget);
		confirmation_open = FALSE;
	}
}

/**********************************************************************/
/* ParameterMakeWidget actually creates the widgets used by          */
/* Parameters.c.  It does not map them however.                      */
/**********************************************************************/

static void 
MakeWidgets() 
{
	Cardinal    n;
	Arg         args[20];
	XmString    xmstring;
	Widget      label_widget,
		left_button_widget,
		right_button_widget,
		action_button_widget,
		cancel_button,
		help_button,
		ok_button,
		disk_bytes_label_widget,
		disk_seconds_label_widget,
		daemon_bytes_label_widget,
		collection_buffers_label_widget,
		collection_bytes_label_widget,
		audit_bytes_label_widget,
		compacted_label_widget,
		enable_label_widget,
		shutdown_label_widget,
		compacted_radio_widget,
		enable_radio_widget,
		shutdown_radio_widget,
		this_or_next_widget;
				
	/*******************************************************************/
	/* Form widget to hold everything                                  */
	/*******************************************************************/
	n = 0;
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNdefaultPosition,                   TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,              XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning,                 True); n++;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	XtSetArg(args[n], XmNautoUnmanage,                     False); n++;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,           XmATTACH_FORM); n++;
	form_widget = XtCreateWidget("Parameters", xmFormWidgetClass,
			mother_form, args, n);

	XtAddCallback(form_widget, XmNhelpCallback,
			HelpDisplayOpen, "audit,Parameters");

	/*******************************************************************/
	/* Heading label                                                   */
	/*******************************************************************/
	xmstring = XmStringCreate(msg[0], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,              XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNborderWidth,                             0); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	label_widget = XmCreateLabelGadget(form_widget, "Title", args, n);
	XtManageChild(label_widget);
	XmStringFree(xmstring);                          

	
	/*******************************************************************/
	/* # of Bytes between Daemon wake-ups                              */
	/*******************************************************************/
	
	xmstring = XmStringCreate(msg[3], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                    label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,           XmALIGNMENT_BEGINNING); n++;
	XtSetArg(args[n], XmNborderWidth,                             0); n++;
	XtSetArg(args[n], XmNlabelString,                     xmstring); n++;
	daemon_bytes_label_widget = XmCreateLabelGadget(
							       form_widget, "Label", args, n);
	XtManageChild(daemon_bytes_label_widget);
	XmStringFree(xmstring);                          
	
	n = 0;
	XtSetArg(args[n], XmNtraversalOn,                          True); n++;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                    label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,      daemon_bytes_label_widget); n++;
	XtSetArg(args[n], XmNcolumns,                                 6); n++;
	XtSetArg(args[n], XmNmaxLength,                               6); n++;
	XtSetArg(args[n], XmNborderWidth,                             1); n++;
	XtSetArg(args[n], XmNshadowThickness,                         0); n++;
	XtSetArg(args[n], XmNeditMode,               XmSINGLE_LINE_EDIT); n++;
	XtSetArg(args[n], XmNeditable,                             TRUE); n++;
	daemon_bytes_edit_widget = XmCreateText(form_widget, "Edit", args, n);
	XtManageChild(daemon_bytes_edit_widget);
	XmAddTabGroup(daemon_bytes_edit_widget);
	
	/*******************************************************************/
	/* # of collection buffers                                         */
	/* For SW_3000 this is maximum amount of buffer memory (1K bytes)  */
	/*******************************************************************/

	xmstring = XmStringCreate(msg[31], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,        daemon_bytes_edit_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                               3); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,           XmALIGNMENT_BEGINNING); n++;
	XtSetArg(args[n], XmNborderWidth,                             0); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	collection_buffers_label_widget = XmCreateLabelGadget(
			       form_widget,  "Label", args, n);
	XtManageChild(collection_buffers_label_widget);
	XmStringFree(xmstring);                          
	
	n = 0;
	XtSetArg(args[n], XmNtraversalOn,                         True); n++;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,        daemon_bytes_edit_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                               3); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,      daemon_bytes_label_widget); n++;
	XtSetArg(args[n], XmNcolumns,                                 2); n++;
	XtSetArg(args[n], XmNmaxLength,                               2); n++;
	XtSetArg(args[n], XmNborderWidth,                             1); n++;
	XtSetArg(args[n], XmNshadowThickness,                         0); n++;
	XtSetArg(args[n], XmNeditMode,               XmSINGLE_LINE_EDIT); n++;
	XtSetArg(args[n], XmNeditable,                             TRUE); n++;
	collection_buffers_edit_widget = XmCreateText(form_widget, "Edit", args, n);
	XtManageChild(collection_buffers_edit_widget);
	XmAddTabGroup(collection_buffers_edit_widget);
	
	/*******************************************************************/
	/* # of bytes before audit file switched                           */
	/*******************************************************************/

	xmstring = XmStringCreate(msg[6], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,  collection_buffers_edit_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                               3); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,           XmALIGNMENT_BEGINNING); n++;
	XtSetArg(args[n], XmNborderWidth,                             0); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	audit_bytes_label_widget = XmCreateLabelGadget(form_widget, "Label", args, n);
	XtManageChild(audit_bytes_label_widget);
	XmStringFree(xmstring);                          
	
	n = 0;
	XtSetArg(args[n], XmNtraversalOn,                          True); n++;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,  collection_buffers_edit_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                               3); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,      daemon_bytes_label_widget); n++;
	XtSetArg(args[n], XmNcolumns,                                 8); n++;
	XtSetArg(args[n], XmNmaxLength,                               8); n++;
	XtSetArg(args[n], XmNborderWidth,                             1); n++;
	XtSetArg(args[n], XmNshadowThickness,                         0); n++;
	XtSetArg(args[n], XmNeditMode,               XmSINGLE_LINE_EDIT); n++;
	XtSetArg(args[n], XmNeditable,                             TRUE); n++;
	audit_bytes_edit_widget = XmCreateText(form_widget, "Edit", args, n);
	XtManageChild(audit_bytes_edit_widget);
	XmAddTabGroup(audit_bytes_edit_widget);
	
	/*******************************************************************/
	/* Radio box with label for compacting audit output files (y/n)    */
	/*******************************************************************/
	
	n = 0;
	xmstring = XmStringCreate(msg[7], charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNleftAttachment,       XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,  audit_bytes_edit_widget); n++;
	XtSetArg(args[n], XmNalignment,    XmALIGNMENT_BEGINNING); n++;
	XtSetArg(args[n], XmNlabelString,               xmstring); n++;
	XtSetArg(args[n], XmNrecomputeSize,                False); n++;
	compacted_label_widget = XmCreateLabelGadget(form_widget, "Label",
		args, n);
	XtManageChild(compacted_label_widget);
	XmStringFree(xmstring);
		
		/* Radio box which will contain yes and no pushbutton widgets */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,          XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,     audit_bytes_edit_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,         XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,      daemon_bytes_label_widget); n++;
	XtSetArg(args[n], XmNentryClass,   xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNorientation,               XmHORIZONTAL); n++;
	compacted_radio_widget = XmCreateRadioBox(
			form_widget, "RadioBox", args, n);
	XtManageChild(compacted_radio_widget);
		
		/* 'Yes' pushbutton */
	n = 0;
	xmstring = XmStringCreate(msg[8], charset);
	if (! xmstring)
		MemoryError();
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                      True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	compacted_yes_widget = XmCreateToggleButton(
		 compacted_radio_widget, "RadioButton", args, n);
	XtManageChild(compacted_yes_widget);
	XmStringFree(xmstring);
	
		/* 'No' pushbutton */
	n = 0;
	xmstring = XmStringCreate(msg[9], charset);
	if (! xmstring)
		MemoryError();
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	compacted_no_widget = XmCreateToggleButton(
		  compacted_radio_widget, "RadioButton", args, n);
	XtManageChild(compacted_no_widget);
	XmStringFree(xmstring);
#ifdef TRAVERSAL
	XmAddTabGroup(compacted_radio_widget);
#endif

	/*******************************************************************/
	/* Radio box with label for enabling audit on startup (y/n)        */
	/*******************************************************************/
	
	n = 0;
	xmstring =  XmStringCreate(msg[10], charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNleftAttachment,       XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,   compacted_radio_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                        3); n++;
	XtSetArg(args[n], XmNalignment,    XmALIGNMENT_BEGINNING); n++;
	XtSetArg(args[n], XmNlabelString,              xmstring); n++;
	XtSetArg(args[n], XmNrecomputeSize,                False); n++;
	enable_label_widget = XmCreateLabelGadget(
		form_widget, "Label", args, n);
	XtManageChild(enable_label_widget);
	XmStringFree(xmstring);
		
		/* Radio box which will contain yes and no pushbutton widgets */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,          XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,       compacted_radio_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                            3); n++;
	XtSetArg(args[n], XmNleftAttachment,         XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,      daemon_bytes_label_widget); n++;
	XtSetArg(args[n], XmNentryClass,   xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNorientation,               XmHORIZONTAL); n++;
	enable_radio_widget = XmCreateRadioBox(
		form_widget, "RadioBox", args, n);
	XtManageChild(enable_radio_widget);
		
		/* 'Yes' pushbutton */
	n = 0;
	xmstring = XmStringCreate(msg[8], charset);
	if (! xmstring)
		MemoryError();
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	enable_yes_widget = XmCreateToggleButton(
		 enable_radio_widget, "RadioButton", args, n);
	XtManageChild(enable_yes_widget);
	XmStringFree(xmstring);
	
		/* 'No' pushbutton */
	n = 0;
	xmstring = XmStringCreate(msg[9], charset);
	if (! xmstring)
		MemoryError();
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                      True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	enable_no_widget = XmCreateToggleButton(
		  enable_radio_widget, "RadioButton", args, n);
	XtManageChild(enable_no_widget);
	XmStringFree(xmstring);
	
#ifdef TRAVERSAL
	XmAddTabGroup(enable_radio_widget);
#endif
	
	/*****************************************************************/
	/* Radio box with label for shutting down gracefully (y/n)       */
	/*****************************************************************/

	n = 0;
	xmstring =  XmStringCreate(msg[11], charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNleftAttachment,       XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,      enable_radio_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                        3); n++;
	XtSetArg(args[n], XmNalignment,    XmALIGNMENT_BEGINNING); n++;
	XtSetArg(args[n], XmNlabelString,               xmstring); n++;
	XtSetArg(args[n], XmNrecomputeSize,                False); n++;
	shutdown_label_widget = XmCreateLabelGadget(
		form_widget, "Label", args, n);
	XtManageChild(shutdown_label_widget);
	XmStringFree(xmstring);
		
		/* Radio box which will contain yes and no pushbutton widgets */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,          XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,          enable_radio_widget); n++;
	XtSetArg(args[n], XmNtopOffset,                            3); n++;
	XtSetArg(args[n], XmNleftAttachment,         XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,      daemon_bytes_label_widget); n++;
	XtSetArg(args[n], XmNentryClass,   xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNorientation,               XmHORIZONTAL); n++;
	shutdown_radio_widget = XmCreateRadioBox(
		form_widget, "RadioBox", args, n);
	XtManageChild(shutdown_radio_widget);
		
		/* 'Yes' pushbutton */
	n = 0;
	xmstring = XmStringCreate(msg[8], charset);
	if (! xmstring)
		MemoryError();
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                  True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	shutdown_yes_widget = XmCreateToggleButton(
		 shutdown_radio_widget,  "RadioButton", args, n);
	XtManageChild(shutdown_yes_widget);
	XmStringFree(xmstring);
	
		/* 'No' pushbutton */
	n = 0;
	xmstring = XmStringCreate(msg[9], charset);
	if (! xmstring)
		MemoryError();
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                  True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	shutdown_no_widget = XmCreateToggleButton(
		  shutdown_radio_widget,  "RadioButton", args, n);
	XtManageChild(shutdown_no_widget);
	XmStringFree(xmstring);

#ifdef TRAVERSAL
	XmAddTabGroup(shutdown_radio_widget);
#endif

	/*******************************************************************/
	/* Setup a radio box to give the user the choice of whether changes*/
	/* should be made This Session or Next Session                     */
	/*******************************************************************/
	
	n = 0;
	XtSetArg(args[n], XmNpacking,                   XmPACK_COLUMN); n++;
	XtSetArg(args[n], XmNentryClass,    xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNorientation,                XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,         shutdown_radio_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	this_or_next_widget = XmCreateRowColumn(
			form_widget, "CurrentFuture", args, n);
	XtManageChild(this_or_next_widget);

	/* Button to confirm for this session */
	n = 0;
	xmstring = XmStringCreate(msg[12], charset);
	if (! xmstring)
	MemoryError();
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                        True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                    xmstring); n++; 
	XtSetArg(args[n], XmNset,                                True); n++; 
	confirm_this_session_widget = XmCreateToggleButton(this_or_next_widget,
			 "ToggleButton", args, n);
	XtManageChild(confirm_this_session_widget);
	XmStringFree(xmstring);
#ifdef TRAVERSAL
	XmAddTabGroup(confirm_this_session_widget);
#endif
	
	/* Button to confirm for next session */
	n = 0;
	xmstring = XmStringCreate(msg[13], charset);
	if (! xmstring)
		MemoryError();
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	confirm_next_session_widget = XmCreateToggleButton(
		 this_or_next_widget, "ToggleButton", args, n);
	XtManageChild(confirm_next_session_widget);
	XmStringFree(xmstring);
#ifdef TRAVERSAL
	XmAddTabGroup(confirm_next_session_widget);
#endif
	
	/**********************************************************************/
	/* Create the OK, CANCEL, HELP buttons                                */
	/**********************************************************************/
	CreateThreeButtons (form_widget, this_or_next_widget,
				 &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
			HelpDisplayOpen, "audit,Parameters");
}

/*************************************************************************/
/* Gets entered parameter data from the text and toggle widgets.         */
/* Validates (?-see below) data.  If data is good, displays confirmation */
/* widget                                                                */
/*************************************************************************/

static void 
OKCallback(w, ptr, info)
	Widget      w;
	char       *ptr;
	XmAnyCallbackStruct *info;
{
	int n;
	char *buf;
	Arg args[20];
	Widget w1;
	XmString xmstring;
	
	/* Load parameter structure with info from text widgets */
	buf = XmTextGetString(daemon_bytes_edit_widget);
	param_struct.au.read_count = (uint) atoi(buf);
	XtFree(buf);

	buf = XmTextGetString(collection_buffers_edit_widget);
	param_struct.au.max_memory = (uint) atoi(buf);
	XtFree(buf);

	buf = XmTextGetString(audit_bytes_edit_widget);
	param_struct.au.caf_maxsize = atol(buf);
	XtFree(buf);
	
	/* Do some simple checking; display an error if not all numbers
	 * are non-zero integers
	 */
	if (
		! param_struct.au.read_count ||
		! param_struct.au.max_memory ||
		! param_struct.au.caf_maxsize
	) {
		ErrorMessageOpen(1410, msg, 21, NULL);
		return;
	}
	
	/* Load boolean parameters with toggle button states */
	param_struct.compacted = 
		  (char) XmToggleButtonGetState(compacted_yes_widget);
	param_struct.audit_on_startup = 
		  (char) XmToggleButtonGetState(enable_yes_widget);
	param_struct.shut_or_panic = 
		  (char) XmToggleButtonGetState(shutdown_yes_widget);
	param_struct.this =
		  (char) XmToggleButtonGetState(confirm_this_session_widget);
	param_struct.future =
		  (char) XmToggleButtonGetState(confirm_next_session_widget);
			  
	/* Perform SecureWare's checking -- VERY incomplete. BE CAREFUL */
	if (AuditParametersCheck(&param_struct))
		return;
		
	/* The user must choose to implement the parameters some time,
	   or an error will occur */
	if (! param_struct.future && ! param_struct.this) {
		ErrorMessageOpen(1420, msg, 24, NULL);
		return;
	}
	
	XtSetSensitive(form_widget, False);
	if (! confirmation_open) {
		confirmation_open = TRUE;
		n = 0;
		xmstring = XmStringCreate(msg[17], charset);
		if (! xmstring)
			MemoryError();
		XtSetArg(args[n], XmNmessageString,           xmstring); n++;
		XtSetArg(args[n], XmNborderWidth,                    1); n++;
		XtSetArg(args[n], XmNtopAttachment,      XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNdialogType,     XmDIALOG_QUESTION); n++;
		XtSetArg(args[n], XmNdialogStyle,   XmDIALOG_WORK_AREA); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
 		XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
#endif
		confirmation_widget = XmCreateMessageBox(mother_form, "Confirm", args, n);
		XmStringFree(xmstring);
	   
		XtAddCallback(confirmation_widget, XmNokCallback, 
			ConfirmOKCallback, NULL);
		XtAddCallback(confirmation_widget, XmNcancelCallback,
			ConfirmCancelCallback, NULL);

		w1 = XmMessageBoxGetChild(confirmation_widget,
			   XmDIALOG_OK_BUTTON);
		n = 0;
		xmstring =  XmStringCreate(msg[19], charset);
		if (! xmstring)
			MemoryError();
		XtSetArg(args[n], XmNlabelString, xmstring); n++;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn, True); n++;
#endif
		XtSetValues(w1, args, n);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(w1);
#endif
		
		w1 = XmMessageBoxGetChild(confirmation_widget,
			   XmDIALOG_CANCEL_BUTTON);
		n = 0;
		xmstring =  XmStringCreate(msg[20], charset);
		if (! xmstring)
			MemoryError();
		XtSetArg(args[n], XmNlabelString, xmstring); n++;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn, True); n++;
#endif
		XtSetValues(w1, args, n);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(w1);
#endif
		
		/* Destroy Help Button of convenience widget */    
		w1 = XmMessageBoxGetChild(confirmation_widget,
			   XmDIALOG_HELP_BUTTON);
		XtUnmanageChild(w1);
		XtDestroyWidget(w1);
	}
	XtManageChild(confirmation_widget);
}

/* Called if the cancel button is pressed; simply calls XParamtersClose() */
static void 
CancelCallback(w, ptr, info)
	Widget      w;
	char       *ptr;
	XmAnyCallbackStruct *info;
{
	ParametersClose();
}


/*************************************************************************/
/* Called if the user presses OK on the confirmation widget.  Updates    */
/* parameter values.  If bad values, returns to the main parameters      */
/* display.  Otherwise, returns back to the main menu                     */
/*************************************************************************/

static void 
ConfirmOKCallback(w, ptr, info)
	Widget      w;
	char       *ptr;
	XmAnyCallbackStruct *info;
{
	if (AuditParametersPut(&param_struct)) {
		XtSetSensitive(form_widget, True);
		XtUnmanageChild(confirmation_widget);
	}
	
	XtSetSensitive(form_widget, True);
	ParametersClose();
	XtUnmanageChild(confirmation_widget);
}

/**************************************************************************/
/* Called if the user presses Cancel on the confirmation widget.  Returns */
/* the user to the main Parameters display                               */
/**************************************************************************/

static void 
ConfirmCancelCallback(w, ptr, info)
	Widget      w;
	char       *ptr;
	XmAnyCallbackStruct *info;
{
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
}
#endif /* SEC_BASE */

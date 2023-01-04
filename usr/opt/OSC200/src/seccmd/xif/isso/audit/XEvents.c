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
static char	*sccsid = "@(#)$RCSfile: XEvents.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:31 $";
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
		XEvents.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X window System Modify Audit Events
		
	entry points:
		ModifyEventsStart()
		ModifyEventsOpen()
		ModifyEventsClose()
		ModifyEventsStop()
*/

#include "XAudit.h"
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>

int 
	CheckAuditEnabled();

static Widget
	form_widget,
	item_widget[AUDIT_MAX_EVENT],
	toggle_yes_widget[AUDIT_MAX_EVENT],
	toggle_no_widget[AUDIT_MAX_EVENT],
	confirmation_widget,
	confirm_this_session_widget,
	confirm_next_session_widget;

static int 
	confirmation_open,
	form_open;
	
static char
	**msg_audit,
	*msg_audit_text,
	**msg,
	*msg_text;

static void
	CancelCallback(),
	ConfirmOKCallback(),
	ConfirmCancelCallback(),
	MakeWidgets(),
	OKCallback();
	 
static audmask_fillin mask_struct;

/* Initialize variables that need to be initialized;
 * called once at startup time
 */

void 
ModifyEventsStart()
{
	confirmation_open = FALSE;
	form_open = FALSE;
}

/* Main entry point for Xevents.c - causes modify audit events screen
 * to be displayed, event mask values to be loaded, etc.
 */

void 
ModifyEventsOpen() 
{
	Arg         args[20];
	Cardinal    n;
	int         i;
	int         audit_active,
		    fd;

	/* Desensitize and flag busy */
	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);    
	
	/* Load messages if this is the first time through */
	if (! msg)
		LoadMessage("msg_isso_audit_events", &msg, &msg_text);

	/* Load audit mask structure */
	if (AuditGetMaskStructure(&mask_struct)) {
		XtSetSensitive(main_menubar, True);
		WorkingClose();
		return;
	}
	
	/* Load mask */
	if (AuditGetMask(&mask_struct)) {
		XtSetSensitive(main_menubar, True);
		WorkingClose();
		return;
	}
	
	/* Create widgets if first time through */
	if (! form_open) {
		MakeWidgets();
		form_open = TRUE;
	}
	
	/* Load data into widgets */
	/* Set Toggles depending on the values in the mask */
	for (i = 0; i < AUDIT_MAX_EVENT; i++) {
		if (mask_struct.mask[i][0] == YESCHAR) {
		    XmToggleButtonSetState(toggle_yes_widget[i], True, False);
		    XmToggleButtonSetState(toggle_no_widget[i], False, False);
		}
		else {
		    XmToggleButtonSetState(toggle_yes_widget[i], False, False);
		    XmToggleButtonSetState(toggle_no_widget[i], True, False);
		}
	}    
	/* Determine audit state to set sensitivity on buttons */
	audit_active = CheckAuditEnabled();
	
	/* Set this/next session widgets off */
	XmToggleButtonSetState(confirm_this_session_widget, FALSE, FALSE);
	XtSetSensitive(confirm_this_session_widget, audit_active);
	XmToggleButtonSetState(confirm_next_session_widget, TRUE,  FALSE);

	/* Display and go */
	CenterForm(form_widget);
	WorkingClose();
}
	
	
/* Clean up after each invocation of ModifyEventsOpen;
 * called from the callbacks to ModifyEventsOpen;
 * Gets rid of screen and returns control to the menubar.
 */
void 
ModifyEventsClose() 
{
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
}

/* Called when Xaudit ends to clean up remaining memory space */
void 
ModifyEventsStop() 
{
	if (form_open) {
		XtDestroyWidget(form_widget);
		form_open = FALSE;
	}
	if (confirmation_open) {
		XtDestroyWidget(confirmation_widget);
		confirmation_open = FALSE;
	}
	AuditFreeMaskTable(&mask_struct);
}

/* Makes the widgets for the modify audit events screen; only called first
 * time ModifyEventsOpen is called.
 */
static void 
MakeWidgets() 
{
	Arg         args[20];
	Cardinal    n;
	XmString    xmstring;
	Widget      cancel_button,
		    help_button,
		    ok_button,
		    heading_widget,
		    scrolled_widget,
		    work_area1_widget,
		    work_area2_widget,
		    radio_widget[AUDIT_MAX_EVENT],
		    this_or_next_widget,
		    vertical_sb_widget,
		    label_widget;
	Dimension   item_width,
		    total_width,
		    radio_height,
		    temp,
		    vertical_SB_width;
	int         audit_active,
		    fd,
		    i,
		    sb_spacing;
	Dimension   label_width,
		    max_label_width;


	/*******************************************************************/
	/* Bulletin widget to hold everything                              */
	/*******************************************************************/
	n = 0;
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNdefaultPosition,                   TRUE); n++;
	XtSetArg(args[n], XmNrubberPositioning,                 TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,              XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNallowOverlap,                     False); n++;
	XtSetArg(args[n], XmNautoUnmanage,                     False); n++;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,           XmATTACH_FORM); n++;
	form_widget = XtCreateWidget("Events", xmFormWidgetClass,
			mother_form, args, n);
	XtAddCallback(form_widget, XmNhelpCallback,
			        HelpDisplayOpen, "audit,Events");
	
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
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	label_widget = XmCreateLabelGadget(form_widget, "Title", args, n);
	XtManageChild(label_widget);
	XmStringFree(xmstring);                          
#ifdef BLOCK_CODE
	/*******************************************************************/
	/* Scrolled window widget to hold information                      */
	/*******************************************************************/
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                   label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNborderWidth,                            1); n++;
	XtSetArg(args[n], XmNscrollingPolicy,              XmAUTOMATIC); n++;
	XtSetArg(args[n], XmNvisualPolicy,                  XmCONSTANT); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,       XmAS_NEEDED); n++;
	XtSetArg(args[n], XmNshadowThickness,                        0); n++;
	scrolled_widget = XtCreateWidget("ScrolledArea", 
			xmScrolledWindowWidgetClass, form_widget, args, n);
#endif
	/*******************************************************************/
	/* Bulletin board that is placed inside the scrolled window above  */
	/* and that holds the mask descriptions and radio 'on-off' boxes   */
	/* for each mask.                                                  */
	/*******************************************************************/
			            
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                   label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNresizePolicy,                XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning,                   True); n++;
	XtSetArg(args[n], XmNdialogStyle,           XmDIALOG_WORK_AREA); n++;
	work_area1_widget = XmCreateForm(form_widget, "Form", args, n);
	XtManageChild(work_area1_widget);
	XSync(XtDisplay(main_shell), FALSE);
	
	max_label_width = 0;
	
	/*******************************************************************/
	/* Do each item listing                                            */
	/*******************************************************************/
	/* Keep audit events in separate message class */
	if (! msg_audit)
		LoadMessage("msg_audit_events", &msg_audit, &msg_audit_text);
	
	for (i = 0; i < AUDIT_MAX_EVENT/2; i++) {
	
		/* Mask Description in a label widget */
		xmstring = XmStringCreate(msg_audit[i], charset);
		if (! xmstring)
			MemoryError();

		n = 0;
		if (i == 0){
			XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		}
		else {
			XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
			XtSetArg(args[n], XmNtopWidget,radio_widget[i-1]);n++;
		}
		XtSetArg(args[n], XmNleftAttachment,       XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNalignment,    XmALIGNMENT_BEGINNING); n++;
		XtSetArg(args[n], XmNlabelString,               xmstring); n++;
		XtSetArg(args[n], XmNrecomputeSize,                False); n++;
		XtSetArg(args[n], XmNmarginWidth,                      1); n++;
		item_widget[i] = XmCreateLabelGadget(work_area1_widget, "Label", args, n);
		XtManageChild(item_widget[i]);
		XmStringFree(xmstring);
		
		n = 0;
		XtSetArg(args[n], XmNwidth,              &label_width); n++;
		XtGetValues(item_widget[i], args, n);
		max_label_width = label_width > max_label_width 
			? label_width : max_label_width;
		
		/* Radio box which will contain yes and no pushbutton widgets */
		n = 0;
		if (i == 0) {
			 XtSetArg(args[n], XmNtopAttachment,         XmATTACH_FORM); n++;
		}
		else {
			 XtSetArg(args[n], XmNtopAttachment,        XmATTACH_WIDGET); n++;
			 XtSetArg(args[n], XmNtopWidget,          radio_widget[i-1]); n++;
		}
		XtSetArg(args[n], XmNleftAttachment,         XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNleftWidget,              item_widget[i]); n++;
		XtSetArg(args[n], XmNentryClass,   xmToggleButtonWidgetClass); n++;
		XtSetArg(args[n], XmNorientation,               XmHORIZONTAL); n++;
		radio_widget[i] = XmCreateRadioBox(work_area1_widget, "RadioBox", args, n);
		XtManageChild(radio_widget[i]);
		
		/* 'Yes' pushbutton */
		n = 0;
		xmstring = XmStringCreate(msg[1], charset);
		XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
		toggle_yes_widget[i] = XmCreateToggleButton(radio_widget[i],
			    "RadioButton", args, n);
		XtManageChild(toggle_yes_widget[i]);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(toggle_yes_widget[i]);
#endif

		/* 'No' pushbutton */
		n = 0;
		xmstring = XmStringCreate(msg[2], charset);
		XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
		toggle_no_widget[i] = XmCreateToggleButton(
			                  radio_widget[i], "RadioButton", args, n);
		XtManageChild(toggle_no_widget[i]);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(toggle_no_widget[i]);
#endif
	}
	XSync(XtDisplay(main_shell), FALSE);
	
	/* Make all labels as big as the greatest label */
	for (i = 0; i < AUDIT_MAX_EVENT/2; i++) {
		n = 0;
		XtSetArg(args[n], XmNwidth,          max_label_width); n++;
		XtSetValues(item_widget[i], args, n);
	}
	XSync(XtDisplay(main_shell), FALSE);
	
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,         XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,        XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,          work_area1_widget); n++;
	XtSetArg(args[n], XmNresizePolicy,             XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning,                True); n++;
	XtSetArg(args[n], XmNdialogStyle,        XmDIALOG_WORK_AREA); n++;
	work_area2_widget = XmCreateForm(form_widget, "Form", args, n);
	XtManageChild(work_area2_widget);
	XSync(XtDisplay(main_shell), FALSE);
	
	max_label_width = 0;
	
	for (i = AUDIT_MAX_EVENT/2; i < AUDIT_MAX_EVENT; i++) {
	
		/* Mask Description in a label widget */
		xmstring = XmStringCreate(msg_audit[i], charset);
		n = 0;
		if (i == AUDIT_MAX_EVENT / 2) {
			 XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		}
		else {
			 XtSetArg(args[n], XmNtopAttachment,        XmATTACH_WIDGET); n++;
			 XtSetArg(args[n], XmNtopWidget,          radio_widget[i-1]); n++;
		}
		XtSetArg(args[n], XmNleftAttachment,        XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNalignment,     XmALIGNMENT_BEGINNING); n++;
		XtSetArg(args[n], XmNlabelString,                xmstring); n++;
		XtSetArg(args[n], XmNrecomputeSize,                 False); n++;
		XtSetArg(args[n], XmNmarginWidth,                       1); n++;
		item_widget[i] = XmCreateLabelGadget(work_area2_widget, "Label", args, n);
		XtManageChild(item_widget[i]);
		XmStringFree(xmstring);
		
		n = 0;
		XtSetArg(args[n], XmNwidth,              &label_width); n++;
		XtGetValues(item_widget[i], args, n);
		max_label_width = label_width > max_label_width 
			? label_width : max_label_width;
		
		/* Radio box which will contain yes and no pushbutton widgets */
		n = 0;
		if (i == AUDIT_MAX_EVENT / 2) {
			 XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		}
		else {
			 XtSetArg(args[n], XmNtopAttachment,        XmATTACH_WIDGET); n++;
			 XtSetArg(args[n],XmNtopWidget,           radio_widget[i-1]); n++;
		}
		XtSetArg(args[n], XmNleftAttachment,         XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNleftWidget,              item_widget[i]); n++;
		XtSetArg(args[n], XmNentryClass,   xmToggleButtonWidgetClass); n++;
		XtSetArg(args[n], XmNorientation,               XmHORIZONTAL); n++;
		radio_widget[i] = XmCreateRadioBox(
			                work_area2_widget, "RadioBox", args, n);
		XtManageChild(radio_widget[i]);
		
		/* 'Yes' pushbutton */
		xmstring = XmStringCreate(msg[1], charset);
		n = 0;
		XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
		toggle_yes_widget[i] = XmCreateToggleButton(
			                  radio_widget[i], "RadioButton", args, n);
		XtManageChild(toggle_yes_widget[i]);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(toggle_yes_widget[i]);
#endif

		/* 'No' pushbutton */
		n = 0;
		xmstring = XmStringCreate(msg[2], charset);
		XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
		toggle_no_widget[i] = XmCreateToggleButton(
			                  radio_widget[i], "RadioButton", args, n);
		XtManageChild(toggle_no_widget[i]);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(toggle_no_widget[i]);
#endif
	}
	XSync(XtDisplay(main_shell), FALSE);
	
	/* Make all labels as big as the greatest label */
	for (i = AUDIT_MAX_EVENT/2; i < AUDIT_MAX_EVENT; i++) {
		n = 0;
		XtSetArg(args[n], XmNwidth,          max_label_width); n++;
		XtSetValues(item_widget[i], args, n);
	}
	XSync(XtDisplay(main_shell), FALSE);
	/*******************************************************************/
	/* Setup a toggle button box to give the user the choice of        */
	/* whether changes should be made This Session or Next Session     */
	/*******************************************************************/
	n = 0;
	XtSetArg(args[n], XmNpacking,                   XmPACK_COLUMN); n++;
	XtSetArg(args[n], XmNentryClass,    xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNorientation,                XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,             work_area1_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	this_or_next_widget = XmCreateRowColumn(
			        form_widget, "CurrentFuture", args, n);
	XtManageChild(this_or_next_widget);
	
	/* Button to confirm for this session */
	n = 0;
	xmstring = XmStringCreate(msg[3], charset);
	XtSetArg(args[n], XmNlabelString,                    xmstring); n++; 
	XtSetArg(args[n], XmNset,                                True); n++; 
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                        True); n++;
#endif
	confirm_this_session_widget = XmCreateToggleButton(
			         this_or_next_widget, "ToggleButton", args, n);
	XtManageChild(confirm_this_session_widget);
	XmStringFree(xmstring);
#ifdef TRAVERSAL
	XmAddTabGroup(confirm_this_session_widget);
#endif
	
	/* Button to confirm for next session */
	n = 0;
	xmstring = XmStringCreate(msg[4], charset);
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	confirm_next_session_widget = XmCreateToggleButton(
			         this_or_next_widget,
			         "ToggleButton", args, n);
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
				 HelpDisplayOpen, "audit,Events");
}

/* Verifies that the information on the screen is valid;
 * displays a confirmation widget to make sure the user really wants to
 * enter the values.
 */
static void 
OKCallback(w, ptr, info)
	Widget    w;
	char     *ptr;
	XmAnyCallbackStruct *info ;
{
	int      i,
		 n;
	Widget   w1;
	XmString xmstring;
	Arg      args[20];
	
	/* Load into structure whether mask is for this or next session */
	mask_struct.this = (char)
			   XmToggleButtonGetState(confirm_this_session_widget);
	mask_struct.future = (char)
			   XmToggleButtonGetState(confirm_next_session_widget);
			   
	/* The user must choose to implement the parameters some time,
	   or an error will occur */

	if (! mask_struct.future && !mask_struct.this) {
		ErrorMessageOpen(3110, msg, 32, NULL);
		return;
	}
	
	/* Load into structure mask values */
	for (i = 0; i < AUDIT_MAX_EVENT; i++) {
		if(XmToggleButtonGetState(toggle_yes_widget[i]))
			mask_struct.mask[i][0] = YESCHAR;
		else
			mask_struct.mask[i][0] = NOCHAR;
	}
	
	/* Validate mask */
	if (AuditCheckMask(&mask_struct))
		return;

	/* Desensitize main widget; display confirmation widget */
	XtSetSensitive(form_widget, False);
	if (! confirmation_open) {
		confirmation_open = TRUE;
		n = 0;
		xmstring = XmStringCreate(msg[8], charset);
		XtSetArg(args[n], XmNmessageString,           xmstring); n++;
		XtSetArg(args[n], XmNborderWidth,                    1); n++;
		XtSetArg(args[n], XmNtopAttachment,      XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNdialogStyle,   XmDIALOG_WORK_AREA); n++;
		XtSetArg(args[n], XmNdialogType,     XmDIALOG_QUESTION); n++;
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
		xmstring = XmStringCreate(msg[10], charset);
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
		xmstring = XmStringCreate(msg[11], charset);
		XtSetArg(args[n], XmNlabelString, xmstring); n++;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn, True); n++;
#endif
		XtSetValues(w1, args, n);
		XmStringFree(xmstring);
	 
#ifdef TRAVERSAL
		XmAddTabGroup(w1);
#endif
	 
		w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_HELP_BUTTON);
		XtUnmanageChild(w1);
		XtDestroyWidget(w1);
	}  
	XtManageChild(confirmation_widget);
}

static void 
CancelCallback(w, ptr, info)
	Widget              w;
	char                *ptr;
	XmAnyCallbackStruct *info;
{
	ModifyEventsClose();
}


/* Writes info and unmaps the confirmation widget */
static void 
ConfirmOKCallback(w, ptr, info) 
	Widget              w; 
	char                *ptr;
	XmAnyCallbackStruct *info;
{
	int i;

	/* Update mask with values validated in OKCallback */
	/* If bad, go back to the main functional unit window */
	if (AuditUpdateMask(&mask_struct)) {
		XtSetSensitive(form_widget, True);
		XtUnmanageChild(confirmation_widget);
	} 
	
	XtSetSensitive(form_widget, True);
	ModifyEventsClose();
	XtUnmanageChild(confirmation_widget);
}

/* Called if user presses cancel from confirmation widget
 * returns focus to the main functional unit window
 */
static void 
ConfirmCancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct     *info;
{
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
}
#endif /* SEC_BASE */

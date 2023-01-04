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
static char	*sccsid = "@(#)$RCSfile: XUtils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:05:28 $";
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

#include <sys/secdefines.h>
#if SEC_BASE

/*
	filename:
		XUtils.c
	
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	functions:
		
	notes:
	This file contains general utilities. The routines indented an extra
	tab are, or should be, "local" routines.  All others should be 
	"public" routines :

	WorkingOpen()
	WorkingClose()
	CreateForm()
	SetTopAndLeftAttachments()
	CreateAttachedForm()
	CreateHeader()
	SetWidgetWidth()
	SetWidgetHeight()
	SetWidgetHeightAndWidth()
	CreateFrame()
	CreateSelectionBox()
	CreateSecondaryForm()
	CreateScrolledWindow()
	CreateScrolledWindowForm()
	CreateScrolledLabelledList()
	CreateTitle()
	CreateUserName()
	SetUserName()
	CreateDeviceName()
	SetDeviceName()
	GetWidth()
		CreateItem()
	CreateToggle()
	CreatePushButton()
		CreateItemYN()
		CreateItemYND()
	CreateItemsYN()
	CreateItemsYND()
	CreateItemsYNDYND()
		CreateItemLabel()
	CreateText()
	CreateItemText()
	CreateItemMultiText()
		CreateItemTextD()
		CreateTexts()
	CreateItemsText()
	CreateItemsTextD()
	CreateYND()
	CreateYNDYND()
	CreateThreeButtons()
		CreateOldThreeButtons()
	CreateConfirmationBox()
	SetToggle()
	SetTextD()
	SetYND()
	extract_normal_string()
	SetToggleState()
	MallocWidget()
	CreateSingleMultiLevel()
	MakeProgramTrusted()
	CenterForm()
	SetAddMultiLevelToggle()
	SetModifyMultiLevelToggle()
	AssignMultiLevel()


	The following should be made static :

	CreateItem*	(except CreateItems*)

	Should find a better way of handling Top, Left attach.
*/

#include <sys/types.h>
#include <sys/secdefines.h>
#include <sys/security.h>
#include <prot.h>

/* X include files */
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/ToggleBG.h>

#include "XMain.h"

/* External routines */
extern void    MemoryError();

/* Defines */
#define NO_CHAR         'N'
#define YES_CHAR        'Y'
#define DEFAULT_CHAR    'D'

/* Local variables */
static XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
static Cursor   working_cursor = NULL;
static Window   window;
static Display  *display;

void 
WorkingOpen(w)
	 Widget w;
{
	display = XtDisplay(w);
	window = XtWindow(w);
	/* Initialize working cursor to the watch cursor */
	if (! working_cursor) {
		working_cursor = XCreateFontCursor(display, XC_watch);
		if (! working_cursor)
			MemoryError();
	}
	XDefineCursor(display, window, working_cursor);
	XFlush(display);
}

void 
WorkingClose() 
{
	XUndefineCursor(display, window);
}

/***********************************************************************/
/* Create the outer form for each menu page                            */
/***********************************************************************/
Widget 
CreateForm (Parent, WidgetName)
	Widget      Parent;
	char        *WidgetName;
{
	Arg         args[20];
	Cardinal    n;
	Widget      w;

	n = 0;
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNdefaultPosition,                   TRUE); n++;
	XtSetArg(args[n], XmNrubberPositioning,                 TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,              XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNallowOverlap,                     False); n++;
	XtSetArg(args[n], XmNautoUnmanage,                     False); n++;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,           XmATTACH_FORM); n++;
	w = XmCreateForm (Parent, WidgetName, args, n);
	return(w);
}

/***********************************************************************/
/* Set top and left attachments                                        */
/***********************************************************************/
void
SetTopAndLeftAttachments (args, n, attach_top, topw, attach_left, leftw)
	Arg         args[];
	Cardinal    *n;
	Boolean     attach_top;
	Widget      topw;
	Boolean     attach_left;
	Widget      leftw;
{
	if (attach_top) {
	    XtSetArg(args[*n], XmNtopAttachment ,XmATTACH_FORM); *n = *n + 1;
	}
	else {
	    XtSetArg(args[*n], XmNtopAttachment ,XmATTACH_WIDGET); *n = *n + 1;
	    XtSetArg(args[*n], XmNtopWidget     ,topw); *n = *n + 1;
	}

	if (attach_left) {
	    XtSetArg(args[*n], XmNleftAttachment  ,XmATTACH_FORM); *n = *n + 1;
	}
	else {
    	XtSetArg(args[*n], XmNleftAttachment  ,XmATTACH_WIDGET); *n = *n + 1;
    	XtSetArg(args[*n], XmNleftWidget      ,leftw); *n = *n + 1;
	}
}

/***********************************************************************/
/* Set top and right attachments                                       */
/***********************************************************************/
void
SetTopAndRightAttachments (args, n, attach_top, topw, attach_right, rightw)
	Arg         args[];
	Cardinal    *n;
	Boolean     attach_top;
	Widget      topw;
	Boolean     attach_right;
	Widget      rightw;
{
	if (attach_top) {
	     XtSetArg(args[*n], XmNtopAttachment ,XmATTACH_FORM); *n = *n + 1;
	}
	else {
	    XtSetArg(args[*n], XmNtopAttachment ,XmATTACH_WIDGET); *n = *n + 1;
	    XtSetArg(args[*n], XmNtopWidget     ,topw); *n = *n + 1;
	}

	if (attach_right) {
	    XtSetArg(args[*n], XmNrightAttachment  ,XmATTACH_FORM); *n = *n + 1;
	}
	else {
	    XtSetArg(args[*n], XmNrightAttachment,XmATTACH_WIDGET); *n = *n + 1;
	    XtSetArg(args[*n], XmNrightWidget      ,rightw); *n = *n + 1;
	}
}

/***********************************************************************/
/* Create a form which can be attached left and top                    */
/***********************************************************************/
Widget 
CreateAttachedForm (Parent, WidgetName, attach_top, topw, attach_left, leftw)
	Widget      Parent;
	char        *WidgetName;
	Boolean     attach_top;
	Widget      topw;
	Boolean     attach_left;
	Widget      leftw;
{
	Arg         args[20];
	Cardinal    n;
	Widget      w;

	n = 0;
	SetTopAndLeftAttachments (args, &n, attach_top, topw, 
		attach_left, leftw);
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNdefaultPosition,                   TRUE); n++;
	XtSetArg(args[n], XmNrubberPositioning,                 TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,              XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNallowOverlap,                     False); n++;
	XtSetArg(args[n], XmNautoUnmanage,                     False); n++;
	XtSetArg(args[n], XmNverticalSpacing,                      0); n++;
	XtSetArg(args[n], XmNhorizontalSpacing,                    0); n++;
	XtSetArg(args[n], XmNtopOffset,                            0); n++;
	XtSetArg(args[n], XmNbottomOffset,                         0); n++;
	XtSetArg(args[n], XmNrightOffset,                          0); n++;
	XtSetArg(args[n], XmNleftOffset,                           0); n++;
	w = XmCreateForm (Parent, WidgetName, args, n);
	return(w);
}

/***********************************************************************/
/* Create the header for a list of options                             */
/***********************************************************************/
Widget 
CreateHeader (Parent, string, topw, attach_left_form, leftw)
	Widget       Parent;
	char         *string;
	Widget       topw;
	Boolean      attach_left_form;
	Widget       leftw;
{
	Arg          args[10];
	Cardinal     n;
	Widget       w;
	XmString     xmstring;

	xmstring = XmStringLtoRCreate (string, charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	SetTopAndLeftAttachments (args, &n, False, topw, attach_left_form, leftw);
	XtSetArg(args[n], XmNlabelString   ,xmstring); n++;
	XtSetArg(args[n], XmNalignment     ,XmALIGNMENT_BEGINNING); n++;
	w = XmCreateLabel (Parent, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	return (w);
}

/***********************************************************************/
/* Create the button header for a list of options                      */
/***********************************************************************/
Widget 
CreateHeaderRight (Parent, string, topw, attach_right_form, rightw)
	Widget       Parent;
	char         *string;
	Widget       topw;
	Boolean      attach_right_form;
	Widget       rightw;
{
	Arg          args[10];
	Cardinal     n;
	Widget       w;
	XmString     xmstring;

	xmstring = XmStringLtoRCreate (string, charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	SetTopAndRightAttachments (args, &n, False, topw, 
		attach_right_form, rightw);
	XtSetArg(args[n], XmNlabelString   ,xmstring); n++;
	XtSetArg(args[n], XmNalignment     ,XmALIGNMENT_END); n++;
	w = XmCreateLabel (Parent, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	return (w);
}

/***********************************************************************/
/* Set the widget width                                                */
/***********************************************************************/
void
SetWidgetWidth (w, Width)
	Widget		w;
	Dimension 	Width;
{
	Arg		args[5];
	Cardinal	n;

	n = 0;
	XtSetArg(args[n], XmNwidth, Width); n++;
	XtSetValues (w, args, n);
}

/***********************************************************************/
/* Set the widget height                                               */
/***********************************************************************/
void
SetWidgetHeight (w, Height)
	Widget 		w;
	Dimension	Height;
{
	Arg		args[5];
	Cardinal	n;

	n = 0;
	XtSetArg(args[n], XmNheight, Height); n++;
	XtSetValues (w, args, n);
}

/***********************************************************************/
/* Set the widget width                                                */
/***********************************************************************/
void
SetWidgetHeightAndWidth (w, Height, Width)
	Widget		w;
	Dimension	Height;
	Dimension	Width;
{
	Arg		args[5];
	Cardinal	n;

	n = 0;
	XtSetArg(args[n], XmNheight, Height); n++;
	XtSetArg(args[n], XmNwidth,  Width); n++;
	XtSetValues (w, args, n);
}

/***********************************************************************/
/* Create the frame for a form                                         */
/***********************************************************************/
Widget 
CreateFrame (Parent, topw, attach_left_form, leftw)
	Widget    Parent;
	Widget		topw;
	Boolean   attach_left_form;
	Widget    leftw;
{
	Arg       args[5];
	Cardinal  n;
	Widget    w;

	n = 0;
	SetTopAndLeftAttachments (args, &n, False, topw, attach_left_form, leftw);
	w = XmCreateFrame(Parent, "Frame", args, n);
	XtManageChild(w);
	return (w);
}

/***********************************************************************/
/* Create the secondary form                                           */
/***********************************************************************/
Widget 
CreateSecondaryForm (Parent)
	Widget		Parent;
{
	Arg		args[20];
	Cardinal	n;
	Widget		w;

	n = 0;
	XtSetArg(args[n], XmNverticalSpacing,          0); n++;
	XtSetArg(args[n], XmNhorizontalSpacing,        0); n++;
	XtSetArg(args[n], XmNtopOffset,                0); n++;
	XtSetArg(args[n], XmNbottomOffset,             0); n++;
	XtSetArg(args[n], XmNrightOffset,              0); n++;
	XtSetArg(args[n], XmNleftOffset,               0); n++;
	XtSetArg(args[n], XmNresizePolicy,                XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning,                   True); n++;
	XtSetArg(args[n], XmNdialogStyle,           XmDIALOG_WORK_AREA); n++;
	w = XmCreateForm(Parent, "Form", args, n);
	XtManageChild(w);
	return (w);
}

/***********************************************************************/
/* Create a selection box widget                                       */
/***********************************************************************/

Widget 
CreateSelectionBox (Parent, resource_name, topw, list_string, 
		selection_string, must_match)
	Widget 	Parent;
	char	*resource_name;
	Widget 	topw;
	char	*list_string;
	char	*selection_string;
	Boolean	must_match;
{
	Arg		args[10];
	Cardinal	n;
	Widget		selection_widget;
	Widget		w;
	XmString	xmstring;
	XmString	xmstring1;

	/* Label for users */
	xmstring = XmStringCreate(list_string, charset);
	if (! xmstring)
		MemoryError();
	xmstring1 = XmStringCreate(selection_string, charset);
	if (! xmstring1)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlistLabelString,              xmstring); n++;
	XtSetArg(args[n], XmNselectionLabelString,        xmstring1); n++;
	XtSetArg(args[n], XmNmustMatch,                        True); n++;
	XtSetArg(args[n], XmNtopAttachment,         XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftAttachment,          XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopWidget,                        topw); n++;
	XtSetArg(args[n], XmNalignment,          XmALIGNMENT_CENTER); n++;
	selection_widget = XmCreateSelectionBox(Parent, resource_name,
		                args, n);
	XtManageChild(selection_widget);
	XmStringFree(xmstring);
	XmStringFree(xmstring1);
	   
	w = XmSelectionBoxGetChild (selection_widget, XmDIALOG_APPLY_BUTTON);
	XtUnmanageChild (w);

	return (selection_widget);
}

/***********************************************************************/
/* Create the scrolled window for devices                              */
/***********************************************************************/

Widget 
CreateScrolledWindow (Parent, topw, leftw)
	Widget	Parent;
	Widget	topw;
	Widget	leftw;
{
	Arg		args[10];
	Cardinal	n;
	Widget		w;

	n = 0;
	XtSetArg(args[n], XmNscrollingPolicy	,XmAUTOMATIC); n++;
	XtSetArg(args[n], XmNleftAttachment  	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget	  	,leftw); n++;
	XtSetArg(args[n], XmNtopAttachment   	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget       	,topw); n++;
	XtSetArg(args[n], XmNwidth		,175); n++;
	XtSetArg(args[n], XmNheight		,120); n++;
	w = XmCreateScrolledWindow(Parent, "ScrolledWindow", args, n);
	XtManageChild(w);

	return (w);
}
/***********************************************************************/
/* Create the scrolled window form widget                              */
/***********************************************************************/

Widget 
CreateScrolledWindowForm (Parent)
	Widget	Parent;
{
	Arg		args[10];
	Cardinal	n;
	Widget		w;

	n = 0;
	XtSetArg(args[n], XmNdefaultPosition	,True); n++;
	XtSetArg(args[n], XmNresizePolicy	,XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning	,True); n++;
	XtSetArg(args[n], XmNdialogStyle	,XmDIALOG_WORK_AREA); n++;
	w = XmCreateForm(Parent, "Form", args, n);
	XtManageChild(w);

	return (w);
}

/***********************************************************************/
/* Create the scrolled list                                            */
/***********************************************************************/

Widget 
CreateScrolledLabelledList (parent, string, class_name, attach_top, 
		topw, attach_left, leftw, list_type)
	Widget		parent;
	char		*string;
	char		*class_name;
	Boolean		attach_top;
	Widget		topw;
	Boolean		attach_left;
	Widget		leftw;
	unsigned char	list_type;
{
	XmString	xmstring;
	Arg		args[10];
	Cardinal	n;
	Widget		w, window_widget;

	/**********************************************************************/
	/* Users list in scrolled window, titled                              */
	/**********************************************************************/
	
	/* Label for users */
	xmstring = XmStringCreate(string, charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	SetTopAndLeftAttachments (args, &n, attach_top, topw, attach_left, 
		leftw);
	XtSetArg(args[n], XmNlabelString,                  xmstring); n++;
	XtSetArg(args[n], XmNalignment,          XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(parent, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
		
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                 w); n++;
	XtSetArg(args[n], XmNtopOffset,                                 3); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++; 
	window_widget = XtCreateManagedWidget(
		    "ScrolledListWindow", xmScrolledWindowWidgetClass, 
		    parent, args, n);

	n = 0;
	XtSetArg(args[n], XmNselectionPolicy,                list_type); n++; 
	XtSetArg(args[n], XmNlistSizePolicy,      XmRESIZE_IF_POSSIBLE); n++; 
	w = XtCreateManagedWidget(class_name,
		    xmListWidgetClass, window_widget, args, n);

	XtManageChild(w);

	return (w);
}

/***********************************************************************/
/* Create a title for each menu page                                   */
/***********************************************************************/
Widget 
CreateTitle (Parent, title)
	Widget	Parent;
	char	*title;
{
	Arg		args[10];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;

	xmstring = XmStringCreate(title, charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,              XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	w = XmCreateLabel(Parent, "Title", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);                          
	return (w);
}

/***********************************************************************/
/* Create the user name widget                                         */
/***********************************************************************/
Widget 
CreateUserName (Parent, topw)
	Widget		Parent;
	Widget topw;
{ 
	Arg		args[10];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;

	/* This string never gets used anyway so don't need a LoadMessage */
	xmstring = XmStringLtoRCreate("User name :", charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                         topw); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	w = XmCreateLabel(Parent, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);                          
	return (w);
}

void
SetUserName (w, user_name)
	Widget    w;
	char      *user_name;
{
	Arg		args[2];
	Cardinal	n;
	XmString	xmstring;
	char		user_name_buffer[100];	/* Big enough I hope */
	static char **msg_user_name,
		*msg_user_name_text;

	if (! msg_user_name)
		LoadMessage("msg_user_name", &msg_user_name, 
			&msg_user_name_text);
	sprintf (user_name_buffer, "%s : %s", msg_user_name[0], user_name);

	n = 0;
	xmstring = XmStringLtoRCreate(user_name_buffer, charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNlabelString,       xmstring); n++;
	XtSetValues(w, args, n);
	XmStringFree(xmstring);                          
}

/***********************************************************************/
/* Create the device name widget                                       */
/***********************************************************************/
Widget 
CreateDeviceName (Parent, topw)
	Widget		Parent;
	Widget topw;
{ 
	Arg		args[10];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;

	/* This string never gets used anyway so don't need a LoadMessage */
	xmstring = XmStringLtoRCreate("Device name :", charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                         topw); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	w = XmCreateLabel(Parent, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);                          
	return (w);
}

void
SetDeviceName (w, device_name)
	Widget    w;
	char      *device_name;
{
	Arg		args[2];
	Cardinal	n;
	XmString	xmstring;
	char		device_name_buffer[100];	/* Big enough I hope */
	static char **msg_device_name,
		*msg_device_name_text;

	if (! msg_device_name)
		LoadMessage("msg_device_name", &msg_device_name, 
			&msg_device_name_text);
	sprintf (device_name_buffer, "%s : %s", msg_device_name[0], 
		device_name);

	n = 0;
	xmstring = XmStringLtoRCreate(device_name_buffer, charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNlabelString,      xmstring); n++;
	XtSetValues(w, args, n);
	XmStringFree(xmstring);                          
}

Dimension
GetWidth (w)
	Widget w;
{
	Dimension	dim;
	Arg		args[2];
	Cardinal	n;

	n = 0;
	XtSetArg(args[n], XmNwidth,   &dim); n++;
	XtGetValues(w, args, n);
	return (dim);
}

/***********************************************************************/
/* Create Item                                                         */
/***********************************************************************/
Widget 
CreateItem(Parent, AttachForm, topw, string, MaxLabelWidth)
	Widget     Parent;
	Boolean		AttachForm;
	Widget     topw;
	char       *string;
	Dimension  *MaxLabelWidth;
{
	Arg		args[10];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;
	Dimension	label_width;

	/* Mask Description in a label widget */
	xmstring = XmStringLtoRCreate(string, charset);
	if (! xmstring)
		MemoryError();

	n = 0;
	SetTopAndLeftAttachments (args, &n, AttachForm, topw, True, topw);
	XtSetArg(args[n], XmNalignment,    XmALIGNMENT_BEGINNING); n++;
	XtSetArg(args[n], XmNlabelString,               xmstring); n++;
	XtSetArg(args[n], XmNrecomputeSize,                False); n++;
	XtSetArg(args[n], XmNmarginWidth,                      1); n++;
	w = XmCreateLabel(Parent, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
	XSync(XtDisplay(w), True);
	label_width = GetWidth(w);

	*MaxLabelWidth = (Dimension) label_width > (Dimension) *MaxLabelWidth 
			? (Dimension) label_width : (Dimension) *MaxLabelWidth;
	return (w);    
}

/***********************************************************************/
/* Create Toggle                                                       */
/***********************************************************************/
Widget 
CreateToggle(Parent, attach_top, topw, leftw)
	Widget    Parent;
	Boolean   attach_top;
	Widget		topw;
	Widget    leftw;
{
	Arg		args[10];
	Cardinal	n;
	XmString	xmstring;
	Widget		w;

	xmstring = XmStringLtoRCreate (" ", charset);
	if (!xmstring)
		MemoryError();
	n = 0;
	SetTopAndLeftAttachments (args, &n, attach_top, topw, False, leftw);
	XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
	w = XmCreateToggleButtonGadget(Parent, "ToggleButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	return (w);
}
/***********************************************************************/
/* Create Named Toggle                                                 */
/***********************************************************************/
Widget 
CreatePushButton(Parent, name, attach_top, topw, attach_left, leftw)
	Widget    Parent;
	char	  *name;
	Boolean   attach_top;
	Widget		topw;
	Boolean   attach_left;
	Widget    leftw;
{
	Arg		args[10];
	Cardinal	n;
	XmString	xmstring;
	Widget		w;

	xmstring = XmStringLtoRCreate (name, charset);
	if (!xmstring)
		MemoryError();
	n = 0;
	SetTopAndLeftAttachments (args, &n, attach_top, topw, 
		attach_left, leftw);
	XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
	w = XmCreatePushButtonGadget(Parent, "PushButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	return (w);
}

/***********************************************************************/
/* Create Item with Y/N toggle                                         */
/***********************************************************************/
Widget 
CreateItemYN(Parent, AttachForm, topw, string, MaxLabelWidth, YesW)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw;
	char		*string;
	Dimension	*MaxLabelWidth;
	Widget		*YesW;
{
	Widget		w;

	w = CreateItem(Parent, AttachForm, topw, string, MaxLabelWidth);
	*YesW = CreateToggle(Parent, AttachForm, topw, w);
	return (w);
}

/***********************************************************************/
/* Create Item with Y/N and Default toggle                             */
/***********************************************************************/
Widget 
CreateItemYND(Parent, AttachForm, topw, string, MaxLabelWidth, 
				YesW, DefW)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw;
	char		*string;
	Dimension	*MaxLabelWidth;
	Widget		*YesW;
	Widget		*DefW;
{
	Widget		w;

	w = CreateItemYN(Parent, AttachForm, topw, string, MaxLabelWidth,
				        YesW);
	*DefW = CreateToggle(Parent, AttachForm, topw, *YesW);
	return (w);
}

/***********************************************************************/
/* Create Items with Y/N toggle                                        */
/***********************************************************************/
void 
CreateItemsYN(Parent, Start, End, msg, MaxLabelWidth, YesW)
	Widget     Parent;
	int        Start,End;
	char       **msg;
	Dimension  *MaxLabelWidth;
	Widget     *YesW;
{
	Arg		args[5];
	Cardinal	n;
	int		i, j;
	Widget		*LabelW;

	LabelW = (Widget *) Malloc ( sizeof (Widget) * (End - Start) );

	j = 0;
	for (i = Start; i < End; i++) {
		if (i == Start)
			LabelW[j++] = CreateItemYN (Parent, True, Parent, 
				    msg[i], MaxLabelWidth, &YesW[i]);
		else
			LabelW[j++] = CreateItemYN (Parent, False, YesW[i-1],
				    msg[i], MaxLabelWidth, &YesW[i]);
	}

	/* Make all labels as big as the greatest label */
	j = 0;
	for (i = Start; i < End; i++) {
		n = 0;
		XtSetArg(args[n], XmNwidth,          *MaxLabelWidth); n++;
		XtSetValues(LabelW[j++], args, n);
	}
}

/***********************************************************************/
/* Create Items with Y/N and Default toggle                            */
/***********************************************************************/
void 
CreateItemsYND(Parent, Start, End, msg, MaxLabelWidth, YesW, DefW)
	Widget		Parent;
	int		Start,End;
	char		**msg;
	Dimension	*MaxLabelWidth;
	Widget		*YesW, *DefW;
{
	Arg		args[5];
	Cardinal	n;
	int		i, j;
	Widget		w;
	Widget		*LabelW;

	LabelW = (Widget *) Malloc ( sizeof (Widget) * (End - Start) );
	if (! LabelW)
		MemoryError();

	j = 0;
	for (i = Start; i < End; i++) {
		if (i == Start)
			LabelW[j++] = CreateItemYND (Parent, True, Parent, 
				    msg[i], MaxLabelWidth, &YesW[i], &DefW[i]);
		else
			LabelW[j++] = CreateItemYND (Parent, False, YesW[i-1],
				    msg[i], MaxLabelWidth, &YesW[i], &DefW[i]);
	}

	/* Make all labels as big as the greatest label */
	j = 0;
	for (i = Start; i < End; i++) {
		n = 0;
		XtSetArg(args[n], XmNwidth,          *MaxLabelWidth); n++;
		XtSetValues(LabelW[j++], args, n);
	}
}

/***********************************************************************/
/* Create Items with Y/N/D and Y/N/D toggles                           */
/***********************************************************************/
void 
CreateItemsYNDYND(Parent, Start, End, msg, MaxLabelWidth, YesW1, DefW1,
				YesW2, DefW2)
	Widget		Parent;
	int		Start,End;
	char		**msg;
	Dimension	*MaxLabelWidth;
	Widget		*YesW1, *DefW1, *YesW2, *DefW2;
{
	Arg		args[5];
	Cardinal	n;
	int		i, j;
	Widget		*LabelW;

	LabelW = (Widget *) Malloc ( sizeof (Widget) * (End - Start) );
	if (! LabelW)
		MemoryError();

	j = 0;
	for (i = Start; i < End; i++) {
		if (i == Start) {
			LabelW[j++] = CreateItemYND (Parent, True, Parent, 
				 msg[i], MaxLabelWidth, &YesW1[i], &DefW1[i]);
				    /* Now add the YND buttons again */
	YesW2[i] = CreateToggle(Parent, True, Parent, DefW1[i]);
	DefW2[i] = CreateToggle(Parent, True, Parent, YesW2[i]);
		}
		else {
			LabelW[j++] = CreateItemYND (Parent, False, YesW1[i-1],
				    msg[i], MaxLabelWidth, &YesW1[i], &DefW1[i]);
	YesW2[i] = CreateToggle(Parent, False, YesW1[i-1], DefW1[i]);
	DefW2[i] = CreateToggle(Parent, False, YesW1[i-1], YesW2[i]);
		}
	}

	/* Make all labels as big as the greatest label */
	j = 0;
	for (i = Start; i < End; i++) {
		n = 0;
		XtSetArg(args[n], XmNwidth,          *MaxLabelWidth); n++;
		XtSetValues(LabelW[j++], args, n);
	}
}

/***********************************************************************/
/* Create Item label                                                   */
/***********************************************************************/
Widget 
CreateItemLabel(Parent, AttachForm, topw, string, MaxLabelWidth)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw;
	char		*string;
	Dimension	*MaxLabelWidth;
{
	Arg		args[10];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;
	Dimension	LabelWidth;

	/* Mask Description in a label widget */
	xmstring = XmStringCreate(string, charset);
	if (! xmstring)
		MemoryError();

	n = 0;
	SetTopAndLeftAttachments (args, &n, AttachForm, topw, True, topw);
	XtSetArg(args[n], XmNalignment,    XmALIGNMENT_BEGINNING); n++;
	XtSetArg(args[n], XmNlabelString,               xmstring); n++;
	XtSetArg(args[n], XmNrecomputeSize,                False); n++;
	XtSetArg(args[n], XmNmarginWidth,                      1); n++;
	w = XmCreateLabel(Parent, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
	n = 0;
	XtSetArg(args[n], XmNwidth,              &LabelWidth); n++;
	XtGetValues(w, args, n);

	*MaxLabelWidth = (Dimension) LabelWidth > (Dimension) *MaxLabelWidth
			? (Dimension) LabelWidth : (Dimension) *MaxLabelWidth;
	return (w);    
}

/***********************************************************************/
/* Create Text Input value                                             */
/***********************************************************************/
Widget 
CreateText(Parent, AttachForm, topw, MaxLength)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw;
	int		MaxLength;
{
	Arg		args[10];
	Cardinal	n;
	Widget		w;

	n = 0;
	SetTopAndLeftAttachments (args, &n, AttachForm, topw, True, NULL);
	XtSetArg(args[n], XmNcolumns           ,(short) MaxLength); n++;
	XtSetArg(args[n], XmNmaxLength         ,MaxLength); n++;
	w = XmCreateText(Parent, "Text", args, n);
	XtManageChild(w);
	XmAddTabGroup(w);
	return (w);
}

/***********************************************************************/
/* Create Text Input value with a label gadget                         */
/***********************************************************************/
Widget 
CreateItemText(Parent, AttachForm, topw, string, MaxLabelWidth,
				    MaxLength, TextW)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw;
	char		*string;
	Dimension	*MaxLabelWidth;
	int		MaxLength;
	Widget		*TextW;
{
	Arg		args[10];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;

	w = CreateItem(Parent, AttachForm, topw, string, MaxLabelWidth);

	n = 0;
	SetTopAndLeftAttachments (args, &n, AttachForm, topw, False, w);
	XtSetArg(args[n], XmNcolumns           ,(short) MaxLength); n++;
	XtSetArg(args[n], XmNmaxLength         ,MaxLength); n++;
	*TextW = XmCreateText(Parent, "Text", args, n);
	XtManageChild(*TextW);
	XmAddTabGroup(*TextW);
	return (w);
}

/***********************************************************************/
/* Create Text Input value with a label gadget                         */
/***********************************************************************/
Widget 
CreateItemMultiText(Parent, AttachForm, topw, string, MaxLabelWidth,
				    MaxLength, number_rows, TextW)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw;
	char		*string;
	Dimension	*MaxLabelWidth;
	int		MaxLength;
	int		number_rows;
	Widget		*TextW;
{
	Arg		args[20];
	Cardinal	n;
	Widget		work_area_frame, work_area_widget;
	Widget		w;

	n = 0;
	XtSetArg(args[n], XmNeditMode          ,XmMULTI_LINE_EDIT); n++;
	XtSetArg(args[n], XmNrows              ,2); n++;
	XtSetArg(args[n], XmNwordWrap		,True); n++;
	XtSetArg(args[n], XmNscrollHorizontal  ,False); n++;
	*TextW = XmCreateScrolledText(Parent, "Text", args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment   ,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment  ,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment ,XmATTACH_FORM); n++;
	XtSetValues(XtParent(*TextW), args, n);

	XtManageChild(*TextW);
	XmAddTabGroup(*TextW);

	w = *TextW;
	return (w);
}

/***********************************************************************/
/* Create Text Input value with a default toggle                       */
/***********************************************************************/
Widget 
CreateItemTextD(Parent, AttachForm, topw, string, MaxLabelWidth,
				    MaxLength, TextW, DefW)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw;
	char		*string;
	Dimension	*MaxLabelWidth;
	int		MaxLength;
	Widget		*TextW;
	Widget		*DefW;
{
	Arg		args[10];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;

	w = CreateItemText(Parent, AttachForm, topw, string, MaxLabelWidth,
				    MaxLength, TextW);
		
	/* Create the default button */
	xmstring = XmStringLtoRCreate ("Defaults are", charset);
	if (!xmstring)
		MemoryError();

	n = 0;
	SetTopAndLeftAttachments (args, &n, AttachForm, topw, False, *TextW);
	XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
	*DefW = XmCreateToggleButtonGadget(Parent, "ToggleButton", args, n);
	XtManageChild(*DefW);
	XmStringFree(xmstring);
	return (w);
}

/***********************************************************************/
/* Create Several text entry fields                                    */
/***********************************************************************/
void 
CreateTexts(Parent, Start, End, MaxLength, TextW)
	Widget		Parent;
	int		Start,End;
	int		MaxLength;
	Widget		*TextW;
{
	Arg		args[5];
	Cardinal	n;
	int		i, j;
	Widget		w;

	j = 0;
	for (i = Start; i < End; i++) {
		if (i == Start) {
			TextW[j] = CreateText(Parent, True, NULL, MaxLength);
			j++;
		}
		else {
			TextW[j] = CreateText(Parent, False, TextW[j-1],
				MaxLength);
			j++;
		}
	}
}

/***********************************************************************/
/* Create Items with text                                              */
/***********************************************************************/
void 
CreateItemsText(Parent, Start, End, msg, MaxLabelWidth, MaxLength, TextW)
	Widget		Parent;
	int		Start,End;
	char		**msg;
	Dimension	*MaxLabelWidth;
	int		*MaxLength;
	Widget		*TextW;
{
	Arg		args[5];
	Cardinal	n;
	int		i, j;
	Widget		w;
	Widget		*LabelW;

	LabelW = (Widget *) Malloc ( sizeof (Widget) * (End - Start) );
	if (! LabelW)
		MemoryError();

	j = 0;
	for (i = Start; i < End; i++) {
		if (i == Start) {
			LabelW[j++] = CreateItemText(Parent, True, Parent, 
				msg[i], MaxLabelWidth, MaxLength[i], &TextW[i]);
		}
		else {
			LabelW[j++] = CreateItemText(Parent, False, TextW[i-1],
				msg[i], MaxLabelWidth, MaxLength[i], &TextW[i]);
		}
	}

	/* Make all labels as big as the greatest label */
	j = 0;
	for (i = Start; i < End; i++) {
		n = 0;
		XtSetArg(args[n], XmNwidth,          *MaxLabelWidth); n++;
		XtSetValues(LabelW[j++], args, n);
	}
}

/***********************************************************************/
/* Create Items with text and Default toggle                           */
/***********************************************************************/
void 
CreateItemsTextD(Parent, Start, End, msg, MaxLabelWidth, 
		MaxLength, TextW, DefW)
	Widget		Parent;
	int		Start,End;
	char		**msg;
	Dimension	*MaxLabelWidth;
	int		*MaxLength;
	Widget		*TextW, *DefW;
{
	Arg		args[5];
	Cardinal	n;
	int		i, j;
	Widget		w;
	Widget		*LabelW;

	LabelW = (Widget *) Malloc ( sizeof (Widget) * (End - Start) );
	if (! LabelW)
		MemoryError();

	j = 0;
	for (i = Start; i < End; i++) {
		if (i == Start) {
			LabelW[j++] = CreateItemTextD(Parent, True, Parent, 
				msg[i], MaxLabelWidth, MaxLength[i], &TextW[i], &DefW[i]);
		}
		else {
			LabelW[j++] = CreateItemTextD(Parent, False, TextW[i-1],
				msg[i], MaxLabelWidth, MaxLength[i], &TextW[i], &DefW[i]);
		}
	}

	/* Make all labels as big as the greatest label */
	XSync(XtDisplay(main_shell), True);
	j = 0;
	for (i = Start; i < End; i++) {
		n = 0;
		XtSetArg(args[n], XmNwidth,          *MaxLabelWidth); n++;
		XtSetValues(LabelW[j++], args, n);
	}
}

/***********************************************************************/
/* Create Yes/ No/ Default toggles                                     */
/***********************************************************************/
void 
CreateYND(Parent, AttachForm, topw, leftw, RadioW, YesW, NoW, 
			CreateDefault, DefW)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw, leftw;
	Widget		*RadioW, *YesW, *NoW;
	Boolean        CreateDefault;
	Widget         *DefW;
{
	Arg		args[10];
	Cardinal	n;
	XmString	xmstring;

	xmstring = XmStringLtoRCreate (" ", charset);
	if (!xmstring)
		MemoryError();
	n = 0;
	SetTopAndLeftAttachments (args, &n, AttachForm, topw, False, leftw);
	XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
	*YesW = XmCreateToggleButtonGadget(Parent, "ToggleButton", args, n);
	XtManageChild(*YesW);
	XmStringFree(xmstring);
	*NoW = *YesW;
	*DefW = *YesW;
	*RadioW = *YesW;

	/* Create the default button */
	if (CreateDefault) {
		xmstring = XmStringLtoRCreate (" ", charset);
		if (!xmstring)
			MemoryError();
		n = 0;
		SetTopAndLeftAttachments (args, &n, AttachForm, topw, 
			False, *RadioW);
		XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
		*DefW = XmCreateToggleButtonGadget(Parent, "ToggleButton", 
			args, n);
		XtManageChild(*DefW);
		XmStringFree(xmstring);
	}
}

/***********************************************************************/
/* Create Yes/ No/ Default  and Yes/ No/ Default toggles               */
/***********************************************************************/
void 
CreateYNDYND(Parent, AttachForm, topw, leftw, CreateDefault,
				On1W, Def1W, On2W, Def2W)
	Widget		Parent;
	Boolean		AttachForm;
	Widget		topw, leftw;
	Boolean		CreateDefault;
	Widget		*On1W, *Def1W, *On2W, *Def2W;
{
	Arg		args[10];
	Cardinal	n;
	XmString	xmstring;

	/* Create first button */
	xmstring = XmStringLtoRCreate (" ", charset);
	if (!xmstring)
		MemoryError();
	n = 0;
	SetTopAndLeftAttachments (args, &n, AttachForm, topw, False, leftw);
	XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
	*On1W = XmCreateToggleButtonGadget(Parent, "ToggleButton", args, n);
	XtManageChild(*On1W);

	/* Create the default button */
	if (CreateDefault) {
		n = 0;
		SetTopAndLeftAttachments (args, &n, AttachForm, topw, False, 
			*On1W);
		XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
		*Def1W = XmCreateToggleButtonGadget(Parent, "ToggleButton", 
			args, n);
		XtManageChild(*Def1W);
	}

	/* Create second series of buttons */
	n = 0;
	if (AttachForm) {
		 XtSetArg(args[n], XmNtopAttachment,XmATTACH_FORM); n++;
	}
	else {
		 XtSetArg(args[n], XmNtopAttachment,XmATTACH_WIDGET); n++;
		 XtSetArg(args[n], XmNtopWidget    ,topw); n++;
	}
	XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
	XtSetArg(args[n], XmNleftAttachment    ,XmATTACH_WIDGET); n++;
	if (CreateDefault) {
		XtSetArg(args[n], XmNleftWidget        ,*Def1W); n++;
	}
	else {
		XtSetArg(args[n], XmNleftWidget        ,*On1W); n++;
	}
	*On2W = XmCreateToggleButtonGadget(Parent, "ToggleButton", args, n);
	XtManageChild(*On2W);

	/* Create the default button */
	if (CreateDefault) {
		n = 0;
		SetTopAndLeftAttachments (args, &n, AttachForm, topw, False, 
			*On2W);
		XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
		*Def2W = XmCreateToggleButtonGadget(Parent, "ToggleButton", 
			args, n);
		XtManageChild(*Def2W);
	}
	XmStringFree (xmstring);
}

/***********************************************************************/
/* Create three buttons - OK , CANCEL , HELP inside a row column widget*/
/***********************************************************************/
void 
CreateThreeButtons (Parent, topw, OKButton, CancelButton, HelpButton)
	Widget		Parent;
	Widget		topw;
	Widget		*OKButton, *CancelButton, *HelpButton;
{ 
	Arg		args[20];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;
	Widget		RowColumnWidget;

	n = 0;
	XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,          topw); n++;
	XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNminimizeButtons,    False); n++;
	RowColumnWidget = XmCreateMessageBox(Parent, "Button", args, n);
	XtManageChild(RowColumnWidget);

	*OKButton = XmMessageBoxGetChild(RowColumnWidget, XmDIALOG_OK_BUTTON);
	*CancelButton = XmMessageBoxGetChild(RowColumnWidget, 
		XmDIALOG_CANCEL_BUTTON);
	*HelpButton = XmMessageBoxGetChild(RowColumnWidget, 
		XmDIALOG_HELP_BUTTON);
	w = XmMessageBoxGetChild(RowColumnWidget, XmDIALOG_SEPARATOR);
	XtUnmanageChild(w);
	/* If this is left in we get a Null child error when running
	w = XmMessageBoxGetChild(RowColumnWidget, XmDIALOG_SYMBOL_LABEL);
	XtUnmanageChild(w);
	*/
	w = XmMessageBoxGetChild(RowColumnWidget, XmDIALOG_MESSAGE_LABEL);
	XtUnmanageChild(w);
}

#ifdef OLD_CODE
/***********************************************************************/
/* Create three buttons - OK , CANCEL , HELP inside a row column widget*/
/***********************************************************************/
void 
CreateThreeOldButtons (Parent, topw, OKButton, CancelButton, HelpButton)
	Widget		Parent;
	Widget topw;
	Widget    *OKButton, *CancelButton, *HelpButton;
{ 
	Arg         args[20];
	Cardinal     n;
	Widget        w;
	XmString    xmstring;
	Widget        RowColumnWidget;

	n = 0;
	XtSetArg(args[n], XmNentryClass,    xmToggleButtonGadgetClass); n++;
	XtSetArg(args[n], XmNpacking,                   XmPACK_COLUMN); n++;
	XtSetArg(args[n], XmNorientation,                XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNadjustLast,                        False); n++;
	XtSetArg(args[n], XmNentryAlignment,       XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                topw); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	RowColumnWidget = XmCreateRowColumn(Parent, "Button", args, n);
	XtManageChild(RowColumnWidget);

	/*********************************************************************/
	/* OK                                                                */
	/*********************************************************************/
	xmstring = XmStringCreate("OK", charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	XtSetArg(args[n], XmNalignment,            XmALIGNMENT_CENTER); n++;
	*OKButton = XmCreatePushButtonGadget(RowColumnWidget, "Button", args, n);
	XtManageChild(*OKButton);
	XmStringFree(xmstring);
	
	/*********************************************************************/
	/* Cancel button                                                     */
	/*********************************************************************/
	n = 0;
	xmstring = XmStringCreate("Cancel", charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	XtSetArg(args[n], XmNalignment,            XmALIGNMENT_CENTER); n++;
	*CancelButton = XmCreatePushButtonGadget(RowColumnWidget,"Button", args, n);
	XtManageChild(*CancelButton);
	XmStringFree(xmstring);                 
	
	/*********************************************************************/
	/* Help Button                                                       */
	/*********************************************************************/
	n = 0;
	xmstring = XmStringCreate("Help", charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNlabelString,                   xmstring); n++; 
	XtSetArg(args[n], XmNalignment,            XmALIGNMENT_CENTER); n++;
	*HelpButton = XmCreatePushButtonGadget(RowColumnWidget, "Button", args, n);
	XtManageChild(*HelpButton);
	XmStringFree(xmstring);
}
#endif /* OLD_CODE */

/***********************************************************************/
/* Create confirmation box                                             */
/***********************************************************************/
Widget 
CreateConfirmationBox (Parent, string)
	Widget		Parent;
	char		*string;
{
	Arg		args[20];
	Cardinal	n;
	Widget		w, w1;
	XmString	xmstring;

	xmstring = XmStringCreate(string, charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,         XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,        XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNmessageString,         xmstring); n++;
	XtSetArg(args[n], XmNborderWidth,           1); n++;
	XtSetArg(args[n], XmNdialogStyle,           XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNdialogType,            XmDIALOG_QUESTION); n++;
	XtSetArg(args[n], XmNdefaultButtonType,     XmDIALOG_OK_BUTTON); n++;
	w = XmCreateMessageBox(Parent, "Confirm", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);                          
	/* No Help button */
	w1 = XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON);
	XtUnmanageChild(w1);
	return (w);
}

void
SetToggle (w, State)
	Widget w;
	Boolean State;
{
	if (XmToggleButtonGadgetGetState (w) != State) 
		XmToggleButtonGadgetSetState (w, State, False);
}

void
SetTextD (UserValue, UserDef, SystemValue, TextW, DefW)
	int		UserValue;
	char		UserDef;
	int		SystemValue;
	Widget		TextW, DefW;
{
	Arg		args[20];
	Cardinal	n;
	Widget		w;
	XmString	xmstring;
	char buffer[30];

	switch (UserDef) {
		case YES_CHAR:
				SetToggle(DefW, True);
				sprintf (buffer, "%d", SystemValue);
				XmTextSetString (TextW, buffer);
				break;

		case NO_CHAR:
				SetToggle(DefW, False);
				sprintf (buffer, "%d", UserValue);
				XmTextSetString (TextW, buffer);
				break;
	}
	/* Store the default name in the widget */
	sprintf (buffer,"Default (%d)",SystemValue);
	
	xmstring = XmStringLtoRCreate (buffer, charset);
	if (!xmstring)
		MemoryError();

	n = 0;
	XtSetArg(args[n], XmNlabelString       ,xmstring); n++;
	XtSetValues(DefW, args, n);
	XmStringFree(xmstring);
}

void
SetYND (User, System, OnW, DefW)
	char	User, System;
	Widget	OnW, DefW;
{
	switch (User) {
		case YES_CHAR:
				SetToggle(OnW ,  True);
				SetToggle(DefW, False);
				break;

		case NO_CHAR:
				SetToggle(OnW , False);
				SetToggle(DefW, False);
				break;

		case DEFAULT_CHAR:
				SetToggle(OnW , (System == YES_CHAR) );
				SetToggle(DefW, True);
				break;
	}
}

/*
 * extract_normal_string()-extract and return the string from the text
 * widget. Do not free this creature, copy and go on 
 */

char *
extract_normal_string(cs)
	XmString cs;
{
	XmStringContext context;
	XmStringCharSet charset;
	XmStringDirection direction;
	Boolean separator;
	static char *primitive_string;

	XmStringInitContext(&context,cs);
	XmStringGetNextSegment(context,&primitive_string,
			  &charset,&direction,&separator);
	XmStringFreeContext(context);
	return((char *) primitive_string);
}


void
SetToggleState (YesWidget, NoWidget, State, WantCallback)
	Widget    YesWidget;
	Widget    NoWidget;
	Boolean    State;
	Boolean WantCallback;

{
	if (State) {
		XmToggleButtonGadgetSetState (YesWidget,  True, WantCallback);
		XmToggleButtonGadgetSetState (NoWidget , False, WantCallback);
	}
	else {
		XmToggleButtonGadgetSetState (YesWidget, False, WantCallback);
		XmToggleButtonGadgetSetState (NoWidget ,  True, WantCallback);
	}
}

Widget *
MallocWidget(n)
	int n;
{
	Widget *s;

	s = (Widget *) Malloc(sizeof(Widget) * n);
	if (s == (Widget *) NULL)
		MemoryError();
	return(s);
}

#if SEC_MAC
Widget
CreateSingleMultiLevel(parent, topw, attach_left, leftw, info_labels,
		b11, b12, b21, b22)
	Widget 	parent;
	Widget	topw;
	Boolean attach_left;
	Widget	leftw;
	Boolean info_labels;	/* If set then draw IL otherwise don't */
	Widget	*b11, *b12, *b21, *b22;
{
	static char **msg_level;
	static char *msg_level_text;
	Widget		work_area_frame,
			work_area_widget;
	Widget		w, w1, w2;
	Widget		r1,r2;
	XmString	xmstring;
	Arg         args[20];
	Cardinal     n;
	Dimension	max_label_width;

#if SEC_SHW
	if (! msg_level)
		LoadMessage("msg_devices_level_shw", &msg_level, 
			&msg_level_text);
#else
	if (! msg_level)
		LoadMessage("msg_devices_level", &msg_level, &msg_level_text);
#endif /* SEC_SHW */
	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (parent, msg_level[0], topw, attach_left, leftw);

	/**********************************************************************/
	/* Create all the Toggle button widgets                               */
	/**********************************************************************/
	work_area_frame  = CreateFrame(parent, w, attach_left, leftw);
	work_area_widget = CreateSecondaryForm(work_area_frame);
	
	max_label_width = (Dimension) 120;
	/**********************************************************************/
	/* Do each item listing                                               */
	/**********************************************************************/
	w1 = CreateItem(work_area_widget,  True, NULL, msg_level[2], 
		&max_label_width);
	w2 = CreateItem(work_area_widget, False,   w1, msg_level[3], 
		&max_label_width); 
	XSync(XtDisplay(w2), False);
	SetWidgetWidth(w , max_label_width); 
	SetWidgetWidth(w1, max_label_width);
	SetWidgetWidth(w2, max_label_width);

#if ! SEC_SHW
	n = 0;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget		,w1); n++;
	r1 = XmCreateRadioBox (work_area_widget, "RadioBox", args, n);
	XtManageChild(r1);
#endif /* ! SEC_SHW */

	n = 0;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_WIDGET); n++;
#if SEC_SHW
	XtSetArg(args[n], XmNleftWidget		,w1); n++;
#else
	XtSetArg(args[n], XmNleftWidget		,r1); n++;
#endif /* SEC_SHW */
	r2 = XmCreateRadioBox (work_area_widget, "RadioBox", args, n);
	XtManageChild(r2);

	/* Create the radio buttons */
	xmstring = XmStringLtoRCreate (" ", charset);
	if (! xmstring)
		MemoryError();

#if ! SEC_SHW
	n = 0;
	XtSetArg(args[n], XmNlabelString   ,xmstring); n++;
	*b11 = XmCreateToggleButtonGadget (r1, "Toggle", args, n);
	XtManageChild(*b11);

	n = 0;
	XtSetArg(args[n], XmNlabelString   ,xmstring); n++;
	*b12 = XmCreateToggleButtonGadget (r1, "Toggle", args, n);
	XtManageChild(*b12);
#endif /* ! SEC_SHW */

#if SEC_ILB
	/* If we have information labels then draw them. We do not
	 * have information labels for the Host and Terminal connections */
	if (info_labels) {
		n = 0;
		XtSetArg(args[n], XmNlabelString   ,xmstring); n++;
		*b21 = XmCreateToggleButtonGadget (r2, "Toggle", args, n);
		XtManageChild(*b21);

		n = 0;
		XtSetArg(args[n], XmNlabelString   ,xmstring); n++;
		*b22 = XmCreateToggleButtonGadget (r2, "Toggle", args, n);
		XtManageChild(*b22);
	}
#endif /* SEC_ILB */

#if SEC_ILB
	if (info_labels)
		w1 = CreateHeader (parent, msg_level[1], topw, False, w);
	else
#endif /* SEC_ILB */
		w1 = CreateHeader (parent, msg_level[4], topw, False, w);
	return (work_area_frame);
}
#endif /* SEC_MAC */

#ifdef SEC_WIND
/* MakeProgramTrusted puts the TP_COOKIE from mwm into a known
 * window property. mwm then turns on  "Trusted Path is Active" whenever
 * the program has the input focus.
 * The ROLE property is stored so mwm knows which trusted application
 * has terminated. This is so mwm can audit it.
 */

void
MakeProgramTrusted(role)
char	*role;
{
	Atom 	PrivAtom, RoleAtom;
	char 	*cookie;
	char 	*window_cookie;
	int	ret;
	int 	actual_type, actual_format, actual_items, bytes_left;
	Display *display;
	Window	window;

	display = XtDisplay(main_shell);
	window  = XtWindow(main_shell);

	cookie = (char *) getenv("TP_COOKIE");

	/* Change the TP_COOKIE */
	PrivAtom = XInternAtom(display,"TPATH_PRIVILEGE",False);
	
	XChangeProperty(display,window,PrivAtom,PrivAtom,8,PropModeAppend,
			cookie,strlen(cookie));

	/* Change the ROLE */
	RoleAtom = XInternAtom(display,"ROLE",False);
	
	XChangeProperty(display,window,RoleAtom,RoleAtom,8,PropModeAppend,
			role,strlen(role));

	XSync(display,False);
	/* This line not needed. Left it so others can see the check
	 * code 
	 * ret = XGetWindowProperty(display,window,PrivAtom,0L,3L,False,
			AnyPropertyType,&actual_type,&actual_format,
			&actual_items,&bytes_left,&window_cookie);
	*/
}
#endif /* SEC_WIND */

/* Centers a form inside its parent */
void
CenterForm (w)
	Widget w;
{
	Arg         args[20];
	Position	x,y;
	Dimension	form_width,width,height;
	Cardinal     n;

	n = 0;
	XtSetArg(args[n], XmNmappedWhenManaged		,False); n++;
	XtSetValues (w, args, n);

	XtManageChild(w);
	XSync(XtDisplay(main_shell), True);

	/* Get width of form */
	n = 0;
	XtSetArg(args[n], XmNwidth	,&form_width); n++;
	XtGetValues (w, args, n);

	/* Get width of parent */
	n = 0;
	XtSetArg(args[n], XmNwidth	,&width); n++;
	XtGetValues (XtParent(w), args, n);

	/* If fits size it */
	n = 0;
	if (form_width < width) {
		x = (width - form_width) / 2;
		XtSetArg(args[n], XmNleftOffset, (int) x); n++;
	}

	XtSetArg(args[n], XmNmappedWhenManaged		,True); n++;
	XtSetValues (w, args, n);
	XtMapWidget(w);
}

#if SEC_MAC
void
SetAddMultiLevelToggle(b11, b12, b21, b22, info_labels, df)
	Widget	b11, b12, b21, b22;
	Boolean		info_labels;	/* If true then set IL as well */
	struct pr_default	*df;
{
#if ! SEC_SHW
	SetToggle(b11, False);
	SetToggle(b12, True);
#endif /* SEC_SHW */
#if SEC_ILB
	if (info_labels) {
		SetToggle(b21, False);
		SetToggle(b22, True);
	}
#endif /* SEC_ILB */
}


void
SetModifyMultiLevelToggle(b11, b12, b21, b22, info_labels, dev)
	Widget	b11, b12, b21, b22;
	Boolean		info_labels;	/* If true then set IL as well */
	struct dev_asg	*dev;
{
#if ! SEC_SHW
	if (dev->uflg.fg_assign) {
		if (ISBITSET(dev->ufld.fd_assign, AUTH_DEV_SINGLE)) {
			SetToggle(b11, True);
			SetToggle(b12, False);
		}
		else {
			SetToggle(b11, False);
			SetToggle(b12, True);
		}
	}
	else if (dev->sflg.fg_assign) {
		if (ISBITSET(dev->sfld.fd_assign, AUTH_DEV_SINGLE)) {
			SetToggle(b11, False);
			SetToggle(b12, True);
		}
		else {
			SetToggle(b11, True);
			SetToggle(b12, False);
		}
	}
	else {
		SetToggle(b11, False);
		SetToggle(b12, True);
	}
#endif /* SEC_SHW */
#if SEC_ILB
	/* Only display if we want information labels shown. We do not show
	 * them for the terminal or host. If we show then look at uflg first.
	 * If set use ufld, else use sfld (if sflg set). If none set then
	 * choose the default value */
	if (info_labels) {
		if (dev->uflg.fg_assign) {
			if (ISBITSET(dev->ufld.fd_assign, AUTH_DEV_ILSINGLE)) {
				SetToggle(b21, True);
				SetToggle(b22, False);
			}
			else {
				SetToggle(b21, False);
				SetToggle(b22, True);
			}
		}
		else if (dev->sflg.fg_assign) {
			if (ISBITSET(dev->sfld.fd_assign, AUTH_DEV_ILSINGLE)) {
				SetToggle(b21, False);
				SetToggle(b22, True);
			}
			else {
				SetToggle(b21, True);
				SetToggle(b22, False);
			}
		}
		else {
			SetToggle(b21, False);
			SetToggle(b22, True);
		}
	}
#endif /* SEC_ILB */
}

void
AssignMultiLevel(b11, b21, dev)
	Widget	b11, b21;
	struct dev_asg	*dev;
{
	dev->uflg.fg_assign = 1;
#if ! SEC_SHW
	if (XmToggleButtonGadgetGetState(b11)) {
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_SINGLE);
		RMBIT  (dev->ufld.fd_assign, AUTH_DEV_MULTI);
	}
	else {
		RMBIT  (dev->ufld.fd_assign, AUTH_DEV_SINGLE);
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_MULTI);
	}
#endif /* SEC_SHW */
#if SEC_ILB
	if (XmToggleButtonGadgetGetState(b21)) {
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_ILSINGLE);
		RMBIT  (dev->ufld.fd_assign, AUTH_DEV_ILMULTI);
	}
	else {
		RMBIT  (dev->ufld.fd_assign, AUTH_DEV_ILSINGLE);
		ADDBIT (dev->ufld.fd_assign, AUTH_DEV_ILMULTI);
	}
#endif /* SEC_ILB */
}
#endif /* SEC_MAC */
#endif /* SEC_BASE */

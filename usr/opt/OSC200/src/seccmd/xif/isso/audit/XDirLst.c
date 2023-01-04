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
static char	*sccsid = "@(#)$RCSfile: XDirLst.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:27 $";
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
		XDirLst.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X window audit interface code to directory list maintenance
		
	entry points:
		DirectoryListStart()
		DirectoryListOpen()
		DirectoryListClose()
		DirectoryListStop()
*/
#include "XAudit.h"

#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/ScrolledW.h>

static Widget	form_widget,
		confirmation_widget,
		single_user_text_widget,
		*edit_list,
		inner_form_widget;
			  
static int	confirmation_open,
		dirlist_open;

static char	**msg,
		*msg_text;
			  
	 
static void 
	 CancelCallback(),
	 ConfirmCancelCallback(),
	 ConfirmOKCallback(),
	 MakeWidgets(),
	 OKCallback();

static Display  
		*display;

void 
DirectoryListStart() 
{
	confirmation_open = FALSE;
	dirlist_open = FALSE;
}

void 
DirectoryListOpen()
{
	int	i,
		length,
		ndir_temp,
		num_items;
	Cardinal    n;
	Arg         args[20];
	char        **new_table;
	
	/* Set menubar insensitive and note busy */
	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);    
	
	/* Pick up messages if not already loaded */
	if (! msg)
		LoadMessage("msg_isso_audit_dirlst", &msg, &msg_text);
		
	/* Get audit directory list information, return on error  */
	if (AuditDirListGet(&audfil)) {
		XtSetSensitive(main_menubar, True);
		WorkingClose();
		return;
	}

	/* Save number of audit directories */
	ndir_temp = audfil.ndirs;
		
	/* Build widgets if not laying around */
	if (! dirlist_open) {
		MakeWidgets();
		dirlist_open = TRUE;
		display = XtDisplay(main_shell);
	}

	/* Load data into widgets */
	/* Load the single user root directory */
	XmTextSetString (single_user_text_widget, audfil.root_dir);

	/* Expand number of entries to edit */
	new_table = expand_cw_table(audfil.dirs, audfil.ndirs,
		         audfil.ndirs + DIRCHUNK , AUDIRWIDTH + 1);
	audfil.ndirs += DIRCHUNK;
	audfil.dirs = new_table;
	length = AUDIRWIDTH;

	/* Make sure at least default is used */
	if (! ndir_temp)
		strcpy(audfil.dirs[0], AUDIT_LOG_DIRECTORY);

	/* Create a list of editable fields, each with one directory name */
	num_items = audfil.ndirs;
	edit_list = (Widget *) Malloc(sizeof(Widget *) * num_items);
	if (! edit_list)
		MemoryError();
	for (i = 0; i < num_items; i++) {
		n = 0;
		if (i == 0) {
		    XtSetArg(args[n], XmNtopAttachment,   XmATTACH_FORM); n++;
		}
		else {
		    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
		    XtSetArg(args[n], XmNtopWidget,      edit_list[i-1]); n++;
		}
		XtSetArg(args[n], XmNtraversalOn,             True); n++;
		XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNeditable,                TRUE); n++;
		XtSetArg(args[n], XmNvalue,         audfil.dirs[i]); n++;
		XtSetArg(args[n], XmNcolumns,               length); n++;
		XtSetArg(args[n], XmNmaxLength,             length); n++;
		XtSetArg(args[n], XmNshadowThickness,            0); n++;
		XtSetArg(args[n], XmNeditMode,  XmSINGLE_LINE_EDIT); n++;
		edit_list[i] = XmCreateText(inner_form_widget, 
			"TextEdit", args, n);
		XmAddTabGroup(edit_list[i]);
	}
	XtManageChildren(edit_list, num_items);
	
	/* Show dirlst display */
	CenterForm(form_widget);
	
	WorkingClose();
}

void 
DirectoryListClose() 
{
	int i;
	
	for (i = 0; i < audfil.ndirs; i++) {
		XSync(display, 0);
		XtDestroyWidget(edit_list[i]);
	}
	free(edit_list);
	
	XSync(display, 0);

	if (audfil.dirs) {
		free_cw_table(audfil.dirs);
		audfil.dirs = NULL;
		audfil.ndirs = 0;
	}
	
	XtSetSensitive(main_menubar, True);
	XtUnmanageChild(form_widget);
	
	if (save_memory) {
		if (dirlist_open) {
			XtDestroyWidget(form_widget);
			dirlist_open = FALSE;
		}
		if (confirmation_open) {
			XtDestroyWidget(confirmation_widget);
			confirmation_open = FALSE;
		}
	}
	XSync(display, 0);

}

void 
DirectoryListStop() 
{
	if (dirlist_open) {
		XtDestroyWidget(form_widget);
		dirlist_open = FALSE;
	}
	if (confirmation_open) {
		XtDestroyWidget(confirmation_widget);
		confirmation_open = FALSE;
	}
}

static void 
MakeWidgets() 
{
	int		cols,
			i,
			length,
			num_items,
			rows,
			sb_spacing;
	Cardinal	n;
	Arg		args[20];
	Widget		scrolled_widget,
			scrollbar_widget,
			action_button_widget,
			cancel_button,
			help_button,
			ok_button,
			label_widget,
			w,
			vertical_sb_widget;
	Dimension	edit_element_height,
			edit_element_width,
			vertical_sb_width; 
	XmString    	xmstring;

	/*******************************************************************/
	/* Bulletin widget to hold everything                              */
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
	form_widget = XtCreateWidget("DirectoryList", xmFormWidgetClass,
			mother_form, args, n);
	XtAddCallback(form_widget, XmNhelpCallback, 
	HelpDisplayOpen, "audit,DirectoryLst");

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

	xmstring = XmStringCreate(msg[11], charset);

	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,            label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,              XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	w = XmCreateLabelGadget(form_widget, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);                          

	n = 0;
	XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,            label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,    XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,                      w); n++;
	single_user_text_widget = XmCreateText(form_widget, "Text", args, n);
	XtManageChild(single_user_text_widget);

	xmstring = XmStringCreate(msg[12], charset);

	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,         single_user_text_widget); n++;
	XtSetArg(args[n], XmNrightAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,              XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	label_widget = XmCreateLabelGadget(form_widget, "Label", args, n);
	XtManageChild(label_widget);
	XmStringFree(xmstring);                          

	/* Scrolled window */
	length = AUDIRWIDTH;
	n = 0;
	XtSetArg(args[n], XmNscrollingPolicy,       XmAUTOMATIC); n++;
	XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,            label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,      XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNscrolledWindowMarginWidth,       1); n++;
	XtSetArg(args[n], XmNborderWidth,                     1); n++;
	scrolled_widget  = XtCreateManagedWidget("ScrolledArea", 
			xmScrolledWindowWidgetClass, form_widget, args, n);
	XtManageChild(scrolled_widget);
	
#ifdef TRAVERSAL
	n = 0;
	XtSetArg(args[n], XmNverticalScrollBar, &scrollbar_widget); n++;
	XtGetValues(scrolled_widget, args, n);
	n = 0;
	XtSetArg(args[n], XmNtraversalOn, True); n++;
	XtSetValues(scrollbar_widget, args, n);
	XmAddTabGroup(scrollbar_widget);
#endif   

	/* Create an inner form to hold the directory names */
	n = 0;
	XtSetArg(args[n], XmNdefaultPosition,                   TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,              XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning,                 True); n++;
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	inner_form_widget = XmCreateForm(scrolled_widget, "Form", args, n);
	XtManageChild(inner_form_widget); 

	/**********************************************************************/
	/* Create the OK, CANCEL, HELP buttons                                */
	/**********************************************************************/
	CreateThreeButtons (form_widget, scrolled_widget,
				 &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
	HelpDisplayOpen, "audit,DirectoryLst");
}

static void 
OKCallback(w, ptr, info)
	Widget      w;
	char       *ptr;
	XmAnyCallbackStruct *info;
{
	int     empty,
		i,
		n;
	Arg     args[20];
	Widget  w1;
	XmString xmstring;
	char    *temp;
	
	/* Move text strings back over */
	temp = XmTextGetString(single_user_text_widget);
	if (audfil.root_dir)
	XtFree(audfil.root_dir);
	audfil.root_dir = (char *) strdup (temp);
	XtFree(temp);

	empty = TRUE;
	for (i = 0; i < audfil.ndirs; i++) {
		 temp = XmTextGetString(edit_list[i]);
		 if (temp[0] != '\0')
			 empty = FALSE;
		 strcpy(audfil.dirs[i], temp);
		 XtFree(temp);
	}
	if (empty) {
		strcpy(audfil.dirs[0], AUDIT_LOG_DIRECTORY);
		XmTextSetString(edit_list[0], audfil.dirs[0]);
	}
	
	/* Test directory list for validity */
	if (AuditDirListCheck(&audfil))
		return;
		
	XtSetSensitive(form_widget, False);
	if (! confirmation_open) {
		confirmation_open = TRUE;
		xmstring = XmStringCreate(msg[4], charset);
		if (! xmstring)
			MemoryError();
		n = 0;
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
		xmstring = XmStringCreate(msg[5], charset);
		if (! xmstring)
			MemoryError();
		n = 0;
		XtSetArg(args[n], XmNlabelString, xmstring); n++;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,  True);n++;
#endif  
		XtSetValues(w1, args, n);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(w1);
#endif        

		w1 = XmMessageBoxGetChild(confirmation_widget,
			   XmDIALOG_CANCEL_BUTTON);
		xmstring = XmStringCreate(msg[6], charset);
		if (! xmstring)
			MemoryError();
		n = 0;
		XtSetArg(args[n], XmNlabelString, xmstring); n++;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,  True);n++;
#endif
		XtSetValues(w1, args, n);
		XmStringFree(xmstring);
#ifdef TRAVERSAL
		XmAddTabGroup(w1);
#endif
			
		w1 = XmMessageBoxGetChild(confirmation_widget,
			   XmDIALOG_HELP_BUTTON);
		XtUnmanageChild(w1);
		XtDestroyWidget(w1);
	}
	XtManageChild(confirmation_widget);
}

static void 
CancelCallback(w, ptr, info)
	Widget      w;
	char       *ptr;
	XmAnyCallbackStruct *info;
{
	DirectoryListClose();
}

static void 
ConfirmOKCallback(w, ptr, info)
	Widget      w;
	char       *ptr;
	XmAnyCallbackStruct *info;
{
	/* Good names provided -- save */
	AuditDirListPut(&audfil);
	
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
	DirectoryListClose();
	return;
}

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

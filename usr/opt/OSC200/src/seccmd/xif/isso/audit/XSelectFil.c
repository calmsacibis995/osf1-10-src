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
static char	*sccsid = "@(#)$RCSfile: XSelectFil.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:55 $";
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
		XSelectfil.c
	   
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for Report Selection File options
		
	entry points:
		SelectionFileStart()
		SelectionFileOpen()
		SelectionFileClose()
		SelectionFileStop()
*/

#include "XAudit.h"
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>

#define CREATE 1
#define UPDATE 2
#define DELETE 3

void    SelectionFileClose(),   
	SelectionFileReset();

int     selection_action;

char    **msg_selection,
	*msg_selection_text,
	*selection_filename;
		
AUDIT_SELECTION_STRUCT  selection_fillin ;

static int 
	confirm_open,
	file_count,
	files_active,
	files_open,
	selected_file,
	user_count;
		
static char 
		**file_names;

static Widget
	confirm_widget,
	create_toggle_widget,
	delete_toggle_widget,
	files_filename_widget,
	form_widget,
	files_list_widget,
	files_scrolled_widget,
	head_label_widget,
	update_toggle_widget;
		
static XmString 
	*file_xmstrings,
	xmstring;
		
static Display  
	*display;

static void
	CancelCallback(),
	MakeWidgets(),
	OKCallback(),
	SelectionFileConfirm(),
	SelectionFileDelete();
		
static int
	SelectionFileGet();
		
void 
SelectionFileStart()
{
	selection_action = FALSE;
	confirm_open = FALSE;
	files_active = FALSE;
	files_open = FALSE;
	SelectionStart();
}

void 
SelectionFileOpen()                        
{
	int         i;
	Cardinal    n;
	Arg         args[20];
	Widget      w;            
	
	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);    
	
	/* load names of report selection files */
	SelectionFileReset();
		
	if (! files_open) {
		ASelectionOpen();
		LoadMessage("msg_isso_audit_selection", &msg_selection, &msg_selection_text);
		MakeWidgets();
		files_open = TRUE;      /* files widgets exist */
		XtManageChild(form_widget);
		files_active = TRUE;        /* files widgets are managed */
		display = XtDisplay(main_shell);
	}    
	WorkingClose();
}    
	
void 
SelectionFileClose() 
{

	int         i;

	XSync(display, 0);

	if (file_xmstrings) {
		for (i = 0; i < file_count; i++)
			XmStringFree(file_xmstrings[i]);
		free(file_xmstrings);
		file_xmstrings = NULL;
	}
	
	if (file_names) { 
		free_cw_table(file_names);
		file_names = NULL;
	}

	if (files_active) {    
		XtUnmanageChild(form_widget);
		files_active = FALSE;
	}
	
	if (save_memory) {
		if (files_open) {
			XtDestroyWidget(form_widget);
			files_open = FALSE;
		}
		if (confirm_open) {
			XtDestroyWidget(confirm_widget);
			confirm_open = FALSE;
		}
	}
	XSync(display, 0);
	XtSetSensitive(main_menubar, True);
}    


void 
SelectionFileStop() 
{
	if (files_open)
		XtDestroyWidget(form_widget);
	if (confirm_open) 
			XtDestroyWidget(confirm_widget);
	SelectionStop();
}

static void 
OKCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	char        selection_filename_buf[NAME_MAX+1],
		buf[50],
		*dcp,
		*scp;
	int     count,
		i, 
		j,
		k,
		touched;
	Cardinal 	n;
	Arg         args[20];
	XmString    *selected_xmstring,
				xmstring;

	/* selection_action indicates which toggle button is selected */
	
	if (selection_action == CREATE) {
		/* Fetch name of file to create */
		selection_filename = XmTextGetString(files_filename_widget);
		strcpy(selection_filename_buf, selection_filename);
		XtFree(selection_filename);
		selection_filename = selection_filename_buf;
		
		/* Compress blanks */
		touched = FALSE;
		for (dcp = scp = selection_filename; *scp; scp++)
			if (*scp != ' ')
				*dcp++ = *scp;
			else
				touched = TRUE;
		*dcp = '\0';
		if (touched)
			XmTextSetString(files_filename_widget, selection_filename);
		/* Test that something in filename field */
		if (! strlen(selection_filename)) {
			/* msg = "Enter the name of the       " */ 
			/*       "selection file to be created" */
			ErrorMessageOpen(1710, msg_selection, 0, NULL);
			return;
		}   
		if (SelectionCreateValidate(selection_filename, &selection_fillin))
			return;
		if (SelectionCreateFill(&selection_fillin))
			return;
	}
	else {
		/* get pre-existing selection file selected from list */
		n = 0;
		XtSetArg(args[n], XmNselectedItems,        &selected_xmstring); n++;
		XtSetArg(args[n], XmNselectedItemCount,                &count); n++;
		XtGetValues(files_list_widget, args, n);
	
		if (count) {
			/* get name of file to update or delete */
			for (i = 0; i < file_count; i++) {
				if (XmStringCompare(file_xmstrings[i], selected_xmstring[0])) 
					break;
			}
			selected_file = i;
			selection_filename = file_names[i];
			
			/* check file exists and read/write access ok */
			if (SelectionUpdateValidate(selection_filename, &selection_fillin))
				return;
		}
		else {
			/* User pressed Update or Delete but didnt select a file */
			/* msg = "Please select a file to [update | delete]" */
			strcpy(buf, msg_selection[29 + selection_action]);
			ErrorMessageOpen(1720, msg_selection, 3, buf);
			return;
		}
		if (selection_action == DELETE) {
			SelectionFileDelete();
			return;
		} 
		if (SelectionDisplayFill(&selection_fillin))
			return;
	}    

	XtUnmanageChild(form_widget);
	files_active = FALSE;                 /* files widgets are unmanaged */

	SelectionOpen();
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	SelectionFileClose();
}


static void 
SelectionFileToggleCallback(w, ptr, info)
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{              
	short       save_session;
	int         i;
	Cardinal    count,                
				n;                   
	Arg         args[20];
	
	if (strcmp(ptr, "Create")) {
		n = 0;
		XtSetArg(args[n], XmNsensitive,        FALSE); n++;
		XtSetArg(args[n], XmNvalue,            "\0"); n++;
		XtSetValues(files_filename_widget, args, n);
		
		n = 0;
		XtSetArg(args[n], XmNsensitive,        TRUE); n++;
		XtSetValues(files_list_widget, args, n);
		
		if (strcmp(ptr, "Update")) 
			selection_action = DELETE;
		else
			selection_action = UPDATE;
	}
	else {
		selection_action = CREATE;
		
		n = 0;
		XtSetArg(args[n], XmNsensitive,       TRUE); n++;
		XtSetValues(files_filename_widget, args, n);
		XtSetKeyboardFocus(form_widget, files_filename_widget);
		
		n = 0;
		XtSetArg(args[n], XmNsensitive,       FALSE); n++;
		XtSetValues(files_list_widget, args, n);
		XmListDeselectAllItems(files_list_widget);
	}
}    
	
static void 
SelectionFileDelete()
{
	Cardinal    n;
	Arg         args[20];
	XmString    xmstring;
	Widget      w;
	
	XtSetSensitive(form_widget, False);
	xmstring = XmStringCreate(msg_selection[35], charset);
	if (confirm_open) {
		n = 0;
		XtSetArg(args[n], XmNmessageString,         xmstring); n++;
		XtSetValues(confirm_widget, args, n);
	}
	else {
		confirm_open = TRUE;
		n = 0;
		XtSetArg(args[n], XmNmessageString,      xmstring); n++;
		XtSetArg(args[n], XmNborderWidth,        1); n++;
		XtSetArg(args[n], XmNresizePolicy,       XmRESIZE_ANY); n++;
		XtSetArg(args[n], XmNdialogStyle,      XmDIALOG_WORK_AREA); n++;
		XtSetArg(args[n], XmNdialogType,       XmDIALOG_QUESTION); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
	 	XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
#endif
		confirm_widget = XmCreateMessageBox(mother_form,
					  "Confirm", args, n);
					  
		XtAddCallback(confirm_widget, XmNokCallback, 
				SelectionFileConfirm,"Go");
		XtAddCallback(confirm_widget, XmNcancelCallback, 
				SelectionFileConfirm, "No");
		XmStringFree(xmstring);
		
		w = XmMessageBoxGetChild(confirm_widget, XmDIALOG_OK_BUTTON);
		n = 0;
		xmstring =  XmStringCreate(msg_selection[36], charset);
		XtSetArg(args[n], XmNlabelString,             xmstring); n++;
		XtSetValues(w, args, n);
		XmStringFree(xmstring);
		
		w = XmMessageBoxGetChild(confirm_widget,
				XmDIALOG_CANCEL_BUTTON);
		n = 0;
		xmstring =  XmStringCreate(msg_selection[37], charset);
		XtSetArg(args[n], XmNlabelString,          xmstring); n++;
		XtSetValues(w, args, n);
		XmStringFree(xmstring);
		
		w = XmMessageBoxGetChild(confirm_widget, XmDIALOG_HELP_BUTTON);
		XtUnmanageChild(w);
		XtDestroyWidget(w);
	}  
	XtManageChild(confirm_widget);
}

static void 
SelectionFileConfirm(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	
	if (strcmp(ptr, "Go")) 
		XmListDeselectAllItems(files_list_widget);
	else {    
		WorkingOpen(main_shell);
		
		/* Delete filename here */
		SelectionDeleteFile(&selection_fillin);

		XmListDeselectItem(files_list_widget, file_xmstrings[selected_file]); 
		XmListDeleteItem(files_list_widget, file_xmstrings[selected_file]); 
	   
		WorkingClose(); 
	}
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirm_widget);
}

static void 
MakeWidgets() 
{   
	XmString    xmstring;
	Widget      action_rowcol_widget,
				filename_label_widget,
		cancel_button,
		help_button,
		ok_button,
				radiobox_widget,
				w;
	Cardinal    n;
	Arg         args[20];
	
	/* Form to contain everything */
	n = 0;
	XtSetArg(args[n], XmNresizePolicy,                   XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning,                      TRUE); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNautoUnmanage,                          FALSE); n++;
	XtSetArg(args[n], XmNallowOverlap,                          FALSE); n++;
	XtSetArg(args[n], XmNdialogStyle,              XmDIALOG_WORK_AREA); n++;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                            TRUE); n++;
#endif
	form_widget = XtCreateWidget("SelectionFiles", 
			xmFormWidgetClass, mother_form, args, n);
	XtAddCallback(form_widget, XmNhelpCallback, 
			HelpDisplayOpen, "audit,SelectFiles" );
  
	/* Title */
	xmstring = XmStringCreate(msg_selection[39], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                          xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,                   XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,                  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,                 XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "Title", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
	/* Radio button box for create/modify/delete */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                 w); n++;
	XtSetArg(args[n], XmNleftAttachment,               XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNorientation,                    XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNentryClass,        xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNspacing,                                  20); n++;
	XtSetArg(args[n], XmNradioAlwaysOne,                         TRUE); n++;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                            TRUE); n++;
#endif
	radiobox_widget = XmCreateRadioBox(form_widget,
			"RadioBox", args, n);
	XtManageChild(radiobox_widget);
#ifdef TRAVERSAL
	XmAddTabGroup(radiobox_widget);
#endif    

	/* Create radio button */
	xmstring = XmStringCreate(msg_selection[30], charset);
	n = 0;
	XtSetArg(args[n], XmNalignment,                XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,                        xmstring); n++;
	XtSetArg(args[n], XmNset,                                   FALSE); n++;
	XtSetArg(args[n], XmNhighlightOnEnter,                       TRUE); n++;
	create_toggle_widget = XmCreateToggleButton( 
			radiobox_widget, "RadioButton", args, n);
	XtAddCallback(create_toggle_widget, XmNvalueChangedCallback,
			SelectionFileToggleCallback, "Create");
	XtManageChild(create_toggle_widget);
	XmStringFree(xmstring);
	
	/* Modify radio button */
	xmstring = XmStringCreate(msg_selection[31], charset);
	n = 0;
	XtSetArg(args[n], XmNalignment,                XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,                        xmstring); n++;
	XtSetArg(args[n], XmNset,                                    TRUE); n++;
	XtSetArg(args[n], XmNhighlightOnEnter,                       TRUE); n++;
	update_toggle_widget = XmCreateToggleButton( 
			radiobox_widget, "RadioButton", args, n);
	XtAddCallback(update_toggle_widget, XmNvalueChangedCallback,
			SelectionFileToggleCallback, "Update");
	XtManageChild(update_toggle_widget);
	XmStringFree(xmstring);
	
	selection_action = UPDATE;
	
	/* Delete radio button */
	xmstring = XmStringCreate(msg_selection[32], charset);
	n = 0;
	XtSetArg(args[n], XmNalignment,                XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,                        xmstring); n++;
	XtSetArg(args[n], XmNset,                                   FALSE); n++;
	XtSetArg(args[n], XmNhighlightOnEnter,                       TRUE); n++;
	delete_toggle_widget = XmCreateToggleButton( 
			radiobox_widget, "RadioButton", args, n);
	XtAddCallback(delete_toggle_widget, XmNvalueChangedCallback,
			SelectionFileToggleCallback, "Delete");
	XtManageChild(delete_toggle_widget);
	XmStringFree(xmstring);
	
	/* Label for text edit field to write in name of new file */
	xmstring = XmStringCreate(msg_selection[40], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,                          xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                     radiobox_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,                  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNborderWidth,                                 0); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	filename_label_widget = XmCreateLabelGadget(form_widget, 
			"Label", args, n);
	XtManageChild(filename_label_widget);
	XmStringFree(xmstring);
	
	/* Text edit field for name of new selection file to create */
	n = 0;
	XtSetArg(args[n], XmNmaxLength,                            NAME_MAX); n++;
	XtSetArg(args[n], XmNvalue,                                    "\0"); n++;
	XtSetArg(args[n], XmNeditMode,                  XmSINGLE_LINE_EDIT); n++;
	XtSetArg(args[n], XmNeditable,                                TRUE); n++;
	XtSetArg(args[n], XmNtopAttachment,                XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                    radiobox_widget); n++; 
	XtSetArg(args[n], XmNleftAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,             filename_label_widget); n++;
	XtSetArg(args[n], XmNsensitive,                              FALSE); n++;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                            TRUE); n++;
#endif
	XtSetArg(args[n], XmNhighlightOnEnter,                       TRUE); n++;
	files_filename_widget = XmCreateText(form_widget,
			"TextEdit", args, n);
	XtManageChild(files_filename_widget);
#ifdef TRAVERSAL
	XmAddTabGroup(files_filename_widget);
#endif
	
	/* Label for list of existing selection files */
	xmstring = XmStringCreate(msg_selection[41], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,                          xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,               files_filename_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,                  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);                             
	
	/* Scrolled area for existing files list */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,             files_filename_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_WIDGET); n++; 
	XtSetArg(args[n], XmNleftWidget,                                w); n++; 
	XtSetArg(args[n], XmNborderWidth,                               0); n++;
	XtSetArg(args[n], XmNscrollingPolicy,       XmAPPLICATION_DEFINED); n++;
	XtSetArg(args[n], XmNvisualPolicy,                     XmVARIABLE); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,             XmSTATIC); n++;
	XtSetArg(args[n], XmNresizable,                              TRUE); n++;
	files_scrolled_widget = XtCreateManagedWidget("Files", 
			xmScrolledWindowWidgetClass, form_widget, args, n);

	/* Existing files list to roll into scrolled area */
	n = 0;
	XtSetArg(args[n], XmNitems,                        file_xmstrings); n++;
	XtSetArg(args[n], XmNsensitive,                              TRUE); n++;
	XtSetArg(args[n], XmNitemCount,                        file_count); n++;
	XtSetArg(args[n], XmNselectionPolicy,             XmSINGLE_SELECT); n++; 
	XtSetArg(args[n], XmNlistSizePolicy,                   XmVARIABLE); n++;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                            TRUE); n++;
#endif
	XtSetArg(args[n], XmNhighlightOnEnter,                       TRUE); n++;
	files_list_widget = XtCreateManagedWidget("List",
			xmListWidgetClass, files_scrolled_widget, args, n);
#ifdef TRAVERSAL
	XmAddTabGroup(files_list_widget);
#endif
					           
	/**********************************************************************/
	/* Create the OK, CANCEL, HELP buttons                                */
	/**********************************************************************/
	CreateThreeButtons (form_widget, files_scrolled_widget,
				 &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "audit,SelectFiles");
}

void 
SelectionFileReset() {

	int         i;
	Cardinal    n;
	Arg         args[20];

	if (file_xmstrings) {
		for (i = 0; i < file_count; i++)
			XmStringFree(file_xmstrings[i]);
		free(file_xmstrings);
		file_xmstrings = NULL;
	}

	if (SelectionFileGet())
		SelectionFileClose();
	file_xmstrings = (XmString *) Malloc(sizeof(XmString) * file_count);
	if (! file_xmstrings)
		MemoryError();
	for (i = 0; i < file_count; i++) 
		file_xmstrings[i] = XmStringCreate(file_names[i], charset);
		
	if ( files_open) {
		/* rebuild the files list widget */
		n = 0;
		XtSetArg(args[n], XmNselectedItemCount,          0); n++;
		XtSetArg(args[n], XmNitems,         file_xmstrings); n++;
		XtSetArg(args[n], XmNitemCount,         file_count); n++;
		XtSetValues(files_list_widget, args, n);

		/* reset "Create" filename to nulls */ 
		n = 0;
		XtSetArg(args[n], XmNvalue,                   "\0"); n++;
		XtSetValues(files_filename_widget, args, n);
	
		XtManageChild(form_widget);
		files_active = TRUE;  
	} 
}    

static int
SelectionFileGet() 
{

	char        buf[PATH_MAX+1],
		*cp,
				dirname[PATH_MAX+1],
				filename[sizeof(AUDIT_REDUCE_PARM_DIR) + ENTSIZ];
	int         file_len,
				i;
	DIR         *fp;
	struct dirent *d;

	if (eaccess (AUDIT_REDUCE_PARM_DIR, 7) < 0) {
		ErrorMessageOpen(1730, msg_selection, 55, NULL);
		/* AUDIT failure to access AUDIT_REDUCE_PARM_DIR */
		audit_no_resource (AUDIT_REDUCE_PARM_DIR,
		  OT_SUBSYS, "Failure to open for read/write/search", ET_SYS_ADMIN);
		return (1);
	}
	
	/* Build name of directory without trailing slash */
	strcpy(dirname, AUDIT_REDUCE_PARM_DIR);
	dirname[strlen(dirname)-1] = '\0';
	
	/* Open selection files directory */
	fp = opendir(dirname);
	if (! fp) {
		SystemErrorMessageOpen(1731, msg_selection, 58, NULL);
		return (1);                                     
	}
	
	/* Scan selection files directory, count files as go */
	file_count = 0;
	file_len = 0;
	while (d = readdir(fp)) {
	cp = d->d_name;
#ifdef SCO
	/* There is a bug in readdir function on SCO platform */
	cp -= 2;
#endif
		if (! strcmp(cp, ".") 
		||  ! strcmp (cp, "..") 
		||  ! d->d_fileno 
		)
			continue;
		file_count++;
		/* save largest name length */
		if (strlen(cp) > file_len)
			file_len = strlen(cp);
	}
	file_len++;
	
	/* If selection files, scan again and pick up selection names */
	if (file_count) {
#if defined(OSF)
	/* The rewinddir function is broken in OSF */
		closedir (fp);
		fp = opendir(dirname);
		if (! fp) {
			SystemErrorMessageOpen(1731, msg_selection, 58, NULL);
			return (1);                                     
		}
#else
		rewinddir(fp);
#endif /* OSF */
		file_names = alloc_cw_table(file_count, file_len);
		if (! file_names) {
			closedir(fp);
			MemoryError();
		}
		i = 0;
		while (d = readdir(fp)) {
	cp = d->d_name;
#ifdef SCO
	/* There is a bug in readdir function on SCO platform */
	cp -= 2;
#endif
			if (! strcmp(cp, ".") 
			||  ! strcmp(cp, "..")
			||  ! d->d_fileno
			)
				continue;
			strcpy(file_names[i], cp);
			i++;
		}
	}
	closedir(fp);
  
	/* Alpha sort entries */
	sort_cw_table(file_names, file_len, file_count);
 
	return 0;
}        
#endif /* SEC_BASE */

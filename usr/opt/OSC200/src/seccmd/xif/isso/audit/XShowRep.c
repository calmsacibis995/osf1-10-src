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
static char	*sccsid = "@(#)$RCSfile: XShowRep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:09:03 $";
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
		XShowRep.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		XIsso interface to display, print, or delete a previous report
		
	entry points:
		ShowReportsStart() ---- Once, at start
		ShowReportsOpen() ----- Every time before use
		ShowReportsClose() ---- Every time when finished with
		ShowReportsStop()  ---- Once, at end 
*/

#include "XAudit.h"
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/ToggleB.h>

static Widget
		confirmation_widget,
		form_widget,
		list_widget,
		scrolled_widget;

static XmString 
		(*selected_report_names)[],
		*xmreports;

static char
		**report_names,
		**msg,
		*msg_text;
		
static int
		confirmation_open,
		form_open,
		report_count,
		selected_report_count;
		
static int
		GetSelectedReports(),
		ReportNamesGet();
		
static void
		CancelCallback(),
		DeleteCallback(),
		DeleteOKCallback(),
		DeleteCancelCallback(),
		DisplayCallback(),
		MakeWidgets(),
		PrintCallback();

void 
ShowReportsStart() 
{
	form_open = FALSE;        
}

void 
ShowReportsOpen()
{
	int         i;
	Widget      w;
	Cardinal    n;
	Arg         args[20];

	/* Get message text if not yet loaded */
	if (! msg)
		LoadMessage("msg_isso_audit_showreports", &msg, &msg_text);
		
	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);    

	/* Make certain data is available before proceed */
	if (ReportNamesGet()) {
                XtSetSensitive(main_menubar, True);
                WorkingClose();
		return;
	}

	/* Malloc space for pointers to scrolled list and create list items */
	xmreports = (XmString *) Malloc(sizeof(XmString) * report_count);
	for(i = 0; i < report_count; i++)
		xmreports[i] = XmStringCreate(report_names[i], charset);
		
	if (! form_open) {   
		/* Build widgets for function */
		MakeWidgets();
		form_open = TRUE;
	}    
	
	/* Load data into widgets */
	n = 0;
	XtSetArg(args[n], XmNselectedItems,           NULL); n++;
	XtSetArg(args[n], XmNselectedItemCount,          0); n++;
	XtSetArg(args[n], XmNitemCount,       report_count); n++;
	XtSetArg(args[n], XmNitems,              xmreports); n++;
	XtSetValues(list_widget, args, n);

	CenterForm(form_widget);
	WorkingClose();
}    
	
void 
ShowReportsClose()
{
	int         i;
		
	/* Free report names */
	if (report_names) {
		for (i = 0; i < report_count; i++) 
			free(report_names[i]);
		free(report_names);
		report_names = NULL;
	}

	/* Free XmStrings for report names */
	if (xmreports) {
		for (i = 0; i < report_count; i++)
			  XmStringFree(xmreports[i]);
		free(xmreports);
		xmreports = NULL;
	}
	
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

void 
ShowReportsStop()
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

/**************************************************************************/
/* Following code functions supports cancel button                        */
/**************************************************************************/

void 
CancelCallback(w, ptr, info) 
	Widget              w;
	char                *ptr;
	XmAnyCallbackStruct *info;
{
	ShowReportsClose();
}

/**************************************************************************/
/* Following code functions used to display selected report(s)            */
/**************************************************************************/

static void 
DisplayCallback(w, ptr, info) 
	Widget              w;
	char                *ptr;
	XmAnyCallbackStruct *info;
{
	int         i,
				j;
	char        fname[256];  /* USE SYSTEM DEFAULT ????? */
	
	/* Pick up selected reports */
	if (! GetSelectedReports())
		return;
   
	/* Loop through selected reports */
	for(i = 0; i < selected_report_count; i++) {
		for (j = 0; j < report_count; j++)
			if (XmStringCompare(xmreports[j], (*selected_report_names)[i]))
				break;
		strcpy(fname, REDUCE_REPORT_PATH);
		strcat(fname, report_names[j]);
		/* file display err msg = "Audit Report is not available." */
		FileDisplayOpen(fname, report_names[j], msg, 8, NULL, 1950);
	}
}

/* DeleteCallback() ...
*  user has requested that a report file be deleted
*  set up confirmation widget
*  let confirmation widget's OK callback handle the actual delete
*/

void 
DeleteCallback(w, ptr, info) 
	Widget              w;
	char                *ptr;
	XmAnyCallbackStruct *info;
{
	Cardinal            n;
	Arg                 args[20];
	Widget              w1;
	XmString            xmstring;
	
	/* Pick up selected reports -- if none selected quick return */
	if (! GetSelectedReports())
		return;
	
	/* Desensitize form and crank up confirmation widget */
	XtSetSensitive(form_widget, False);
	if (! confirmation_open) {
		
		/* Confirmation's message box widget */
		n = 0;
		xmstring = XmStringCreate(msg[21], charset);
		if (! xmstring)
			MemoryError();
		XtSetArg(args[n], XmNmessageString,   xmstring); n++;
		XtSetArg(args[n], XmNborderWidth,     1); n++;
		XtSetArg(args[n], XmNtopAttachment,   XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,  XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNdialogStyle,     XmDIALOG_WORK_AREA); n++;
		XtSetArg(args[n], XmNdialogType,      XmDIALOG_QUESTION); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
		XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
#endif
		confirmation_widget = XmCreateMessageBox(mother_form,
				      "Confirm", args, n);
		XmStringFree(xmstring);
		XtAddCallback(confirmation_widget, XmNokCallback, 
				DeleteOKCallback, NULL);
		XtAddCallback(confirmation_widget, XmNcancelCallback,
				DeleteCancelCallback, NULL);
				
		w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_OK_BUTTON);
		n = 0;
		xmstring = XmStringCreate(msg[22], charset);
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
		
		w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_CANCEL_BUTTON);
		n = 0;
		xmstring = XmStringCreate(msg[23], charset);
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
		
		w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_HELP_BUTTON);
		XtUnmanageChild(w1);
		XtDestroyWidget(w1);
		
		confirmation_open = TRUE;
	}

	XtManageChild(confirmation_widget);
}
	
/* Cancel delete request */
static void 
DeleteCancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct    *info;
{
	/* resensitize form widget */
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirmation_widget);
}

static void 
DeleteOKCallback(w, ptr, info)
	Widget              w;
	char                *ptr;
	XmAnyCallbackStruct *info;
{
	int     i,
			j;
	char    filename[256];        
	
	/* Loop through selected reports */
	for(i = 0; i < selected_report_count; i++) {
		for (j = 0; j < report_count; j++) {
			if (XmStringCompare(xmreports[j], (*selected_report_names)[i]))
				break;
		}
	
		sprintf (filename, "%s%s", REDUCE_REPORT_PATH, report_names[j]);
		if ( unlink(filename) < 0 ) {
			 ErrorMessageOpen(1905, msg, 16, NULL);
			/* "Unable to remove file from report directory."*/
			/* "Please check permissions on program and directory."*/
			/* AUDIT unsuccessful remove of reduction file */
			sa_audit_audit (ES_AUD_ERROR, msg[19]);
			/* "Unsuccessful remove of report file" */
		} 
		else {
			/* AUDIT successful remove of reduction file */
			sa_audit_audit (ES_AUD_MODIFY, msg[20]);
			/* "Successful remove of report file" */
			XmListDeselectItem(list_widget, xmreports[j]);
			XmListDeleteItem(list_widget, xmreports[j]);
		}
	}
	
	/* resensitize form widget, then close out */
	XtUnmanageChild(confirmation_widget);
	XtSetSensitive(form_widget, True);
}

/**************************************************************************/
/* Following code functions used to get XmString names of selected reports*/
/**************************************************************************/

static int 
GetSelectedReports() {
	Cardinal    n;
	Arg         args[20];

	/* Fetch user selected report(s) */
	n = 0;
	XtSetArg(args[n], XmNselectedItems,          &selected_report_names); n++;
	XtSetArg(args[n], XmNselectedItemCount,      &selected_report_count); n++;
	XtGetValues(list_widget, args, n);
	if (selected_report_count < 1) {
		/* "No reports selected!" */
		ErrorMessageOpen(1910, msg, 0, NULL);
		return FALSE;
	}
	return TRUE;
}

/**************************************************************************/
/*      ReportNamesGet()                                                  */
/* Following code functions used to scan audit reports directory          */
/* data placed in global variables report_count and report_names          */
/**************************************************************************/

static int 
ReportNamesGet()
{
	char        buf[512],
		    *cp,
		    dirname[512];
	int         i;
	DIR         *fp;
	struct dirent *d;
	struct stat sb;


	/* Build name of directory without trailing slash */
	strcpy(dirname, REDUCE_REPORT_PATH);
	dirname[strlen(dirname)-1] = '\0';
	
	/* Test reports file directory exists; create if no */
	if (stat(dirname, &sb) < 0) {
		if (mkdir(dirname, 0) < 0) {
			/* "Cannot create audit reports directory." */
			/* "Please check permission on directory and on this program." */
			SystemErrorMessageOpen(1915, msg, 26, NULL);
			return(1);
		}
	}    
	/* Test report files directory for access */
	if(eaccess(dirname, 5) < 0) {
		/* No read or search permission on audit reports directory. */
		/* Please check permission on \'%s\'. */
		SystemErrorMessageOpen(1920, msg, 2, dirname);
		return(1);
	}
	
	/* Open report files directory */
	fp = opendir(dirname);
	if (! fp) {
		/* "Cannot open audit reports directory for read." */
		/* "Please check permission on directory and on this program." */
		SystemErrorMessageOpen(1930, msg, 5, NULL);
		return(1);
	}
	
	/* Scan report files directory, count report files as go */
	report_count = 0;
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
		report_count++;
	}
	
	/* Malloc array of pointers to report file names */
	if (report_count) {
		report_names = (char **) Malloc(sizeof(char *) * report_count);
		if (! report_names) {
			closedir(fp);
			MemoryError();
			/* Dies */
		}
	}
	else {
		report_names = NULL;
		ErrorMessageOpen(1940, msg, 24, NULL);
		/* "No reports to display."*/
		/* AUDIT unsuccessful remove of reduction file */
		return(1);
	}   

	/* If report files, scan again and pick up report names */
	if (report_count) {
#if defined(OSF)
	/* The rewinddir function is broken in OSF */
		fp = opendir(dirname);
		if (! fp) {
			/* "Cannot open audit reports directory for read." */
			/* "Please check permission on directory " */
			SystemErrorMessageOpen(1930, msg, 5, NULL);
			return(1);
		}
#else
		rewinddir(fp);
#endif /* OSF */
		for (i = 0; i < report_count && (d = readdir(fp)); ) {
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
			report_names[i] = (char *) Malloc(strlen(cp) + 1);
			if (! report_names[i]) {
				closedir(fp);
				MemoryError();
			}
			strcpy(report_names[i], cp);
			i++;
		}
	}
	closedir(fp);
	
	/* If more than one, then alpha sort */
	if (report_count > 1)
		qsort(report_names, report_count, sizeof(char *), strcmp);
		
	return(0);
}  

static void 
MakeWidgets()
{
	int         i;
	Widget      action_button_widget,
				displayprint_button_widget,
				w,
				w1;
	XmString    xm_string;
	Cardinal    n;
	Arg         args[20];

	/**********************************************************************/
	/* Bulletin board -- Form widget                                      */
	/**********************************************************************/
  
	n = 0;
	XtSetArg(args[n], XmNdefaultPosition,                          TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,                     XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNdialogStyle,                XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNtopAttachment,                   XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,                  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNautoUnmanage,                            FALSE); n++;
	XtSetArg(args[n], XmNallowOverlap,                            FALSE); n++;
	form_widget = XtCreateWidget("DisplayReports", xmFormWidgetClass, 
		mother_form, args, n);
	XtAddCallback(form_widget, XmNhelpCallback,
		HelpDisplayOpen, "audit,DispReports");

	/* Title */
	xm_string = XmStringCreate(msg[10], charset);
	if (!xm_string)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                   xm_string); n++;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,           XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,            XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNborderWidth,                           0); n++;
	w = XmCreateLabelGadget(form_widget, "Title", args, n);
	XtManageChild(w);
	XmStringFree(xm_string);
	
	/* Build column titles for scrolled list */
	xm_string = XmStringCreate(msg[11], charset);
	if (!xm_string)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                    xm_string); n++;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                              w); n++;
	XtSetArg(args[n], XmNleftAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset,                            10); n++;
	XtSetArg(args[n], XmNborderWidth,                            0); n++;
	XtSetArg(args[n], XmNalignment,             XmALIGNMENT_CENTER); n++;
	w1 = XmCreateLabelGadget(form_widget, "Label", args, n);
	XtManageChild(w1);
	XmStringFree(xm_string);
	
	
	/* Build Scrolled Window widget */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                 w); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,                               w1); n++;
	XtSetArg(args[n], XmNrightAttachment,               XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNborderWidth,                               0); n++;
	XtSetArg(args[n], XmNscrollingPolicy,       XmAPPLICATION_DEFINED); n++;
	XtSetArg(args[n], XmNvisualPolicy,                     XmVARIABLE); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,             XmSTATIC); n++;
	scrolled_widget = XtCreateManagedWidget("ScrolledArea", 
			xmScrolledWindowWidgetClass, form_widget, args, n);
		
	/* Build List widget, managed by Scrolled window */
	n = 0;
	XtSetArg(args[n], XmNlistSizePolicy,                     XmVARIABLE); n++;
	XtSetArg(args[n], XmNsensitive,                                TRUE); n++;
	XtSetArg(args[n], XmNselectionPolicy,               XmSINGLE_SELECT); n++;
	list_widget = XtCreateManagedWidget("List", xmListWidgetClass, 
			scrolled_widget, args, n);
	
	/**********************************************************************/
	/* RowColumn widget for action buttons                                */
	/***********************************************************************/
	n = 0;
	XtSetArg(args[n], XmNentryClass,    xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNpacking,                   XmPACK_COLUMN); n++;
	XtSetArg(args[n], XmNorientation,                XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNadjustLast,                        False); n++;
	XtSetArg(args[n], XmNentryAlignment,       XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,               scrolled_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	action_button_widget = XmCreateRowColumn(form_widget, "Button", args, n);
	XtManageChild(action_button_widget);
	
	/* Display button */
	xm_string = XmStringCreate(msg[15], charset);
	if (!xm_string)
		MemoryError();
	n = 0;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                              True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                         xm_string); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	displayprint_button_widget = XmCreatePushButtonGadget(
			action_button_widget, "Button", args, n);
	XtManageChild(displayprint_button_widget);
	XtAddCallback(displayprint_button_widget, XmNactivateCallback, 
			DisplayCallback, NULL);
	XmStringFree(xm_string);
#ifdef TRAVERSAL
	XmAddTabGroup(displayprint_button_widget);
#endif
	
	/* Delete button */                                 
	xm_string = XmStringCreate(msg[14], charset);
	if (!xm_string)
		MemoryError();
	n  = 0;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                              True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                         xm_string); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreatePushButtonGadget(action_button_widget, "Button", args, n);
	XtManageChild(w);
	XtAddCallback(w ,XmNactivateCallback, DeleteCallback, NULL);
	XmStringFree(xm_string);
#ifdef TRAVERSAL
	XmAddTabGroup(w);
#endif
	
	/* Cancel Button */
	xm_string = XmStringCreate(msg[13], charset);
	if (! xm_string)
		MemoryError();
	n = 0;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                              True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                         xm_string); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreatePushButtonGadget(action_button_widget, "Button", args, n);
	XtManageChild(w);
	XtAddCallback(w ,XmNactivateCallback, CancelCallback, NULL);
	XmStringFree(xm_string);
#ifdef TRAVERSAL
	XmAddTabGroup(w);
#endif

	/* Help Button */
	xm_string = XmStringCreate(msg[12], charset);
	if (! xm_string)
		MemoryError();
	n = 0;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                              True); n++;
#endif
	XtSetArg(args[n], XmNlabelString,                         xm_string); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreatePushButtonGadget(action_button_widget, "Button", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, HelpDisplayOpen, "audit,DispReports");
	XmStringFree(xm_string);
#ifdef TRAVERSAL
	XmAddTabGroup(w);
#endif

}

#endif /* SEC_BASE */

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
static char	*sccsid = "@(#)$RCSfile: XReduce.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:39 $";
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
		XReduce.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for Reduction menu options
		
	entry points:
		Generate New Report:
			ReduceGenerateStart()
			ReduceGenerateOpen()
			ReduceGenerateClose()
			ReduceGenerateStop()
*/

#include "XAudit.h"
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
	
#define REDUCE_COMMAND      "/tcb/bin/reduce"
#define REDUCE_REPORT_FILE  "AuditReport"

static char 
		**msg,
		*msg_text,
		*report_file_name,
		report_session[6],
		report_pfile[NAME_MAX+1];
		
static int 
		form_open;       /* Flag to avoid double-up of close */
		
static Widget
		form_widget,
		fname_widget,
		ls_list_widget,
		pf_list_widget;

static XmString 
		*audit_sessions,
		*parameter_files;
		
static void
		CancelCallback(),
		OKCallback(),
		MakeWidgets(),
		ReduceQuit();

void 
ReduceGenerateStart()
{
	form_open = FALSE;
}

void 
ReduceGenerateOpen() 
{
	int         i,
		visible_item_count;
	Cardinal    n;
	Arg         args[20];

	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);    

	/* Load in text messages if not already in */
	if (! msg)
		LoadMessage( "msg_isso_audit_reduce", &msg, &msg_text);
		
	/**********************************************************************/
	/* Make certain that data is available before proceed                 */
	/**********************************************************************/

	/* Load audit sessions data */
	if (AuditListSessionsValidate() || AuditListSessionsGet(&lsfil)) {
		ReduceQuit();
		return;
	}
	
	/* Load parameter file data */
	if (ParameterFileListGet(&lpfile_fil)) {
		ReduceQuit();
		return;
	}
	
	/* At least one audit session and one parameter file */
	if (lsfil.nsessions < 1) {
		/* "No audit sessions available!" */
		ErrorMessageOpen(1510, msg, 0, NULL);
		ReduceQuit();
		return;
	}
	
	if (lpfile_fil.nfiles < 1) {
		/* "No parameter files available!" */
		ErrorMessageOpen(1520, msg, 2, NULL);
		ReduceQuit();
		return;
	}

	/* Malloc space for pointers to scrolled list and create list items */
	audit_sessions = (XmString *) Malloc(sizeof(XmString) * lsfil.nsessions);
	if (! audit_sessions)
		MemoryError();
		/* Dies */
	for (i = 0; i < lsfil.nsessions; i++) {
		audit_sessions[i] = XmStringCreate(lsfil.sessions[i], charset);
		if (! audit_sessions[i])
			MemoryError();
			/* Dies */
	}
	
	parameter_files = (XmString *) Malloc(sizeof(XmString) * lpfile_fil.nfiles);
	if (! parameter_files)
		MemoryError();
		/* Dies */
	for (i = 0; i < lpfile_fil.nfiles; i++) {
		parameter_files[i] = XmStringCreate(lpfile_fil.files[i], charset);
		if (! parameter_files[i])
			MemoryError();
			/* Dies */
	}

	/* Build widgets for form if not already created */
	if (! form_open) {   
		MakeWidgets();
		form_open = TRUE;
	}    
	
	/* Load data into the widgets */
	n = 0;
	XtSetArg(args[n], XmNitems,                    audit_sessions); n++;
	XtSetArg(args[n], XmNitemCount,               lsfil.nsessions); n++;
	XtSetValues(ls_list_widget, args, n);
		
	n = 0;
	XtSetArg(args[n], XmNitems,                   parameter_files); n++;
	XtSetArg(args[n], XmNitemCount,             lpfile_fil.nfiles); n++;
	XtSetValues(pf_list_widget, args, n);

	/* Manage the whole ball of wax and go */
	CenterForm(form_widget);
	WorkingClose();
}    

void 
ReduceGenerateClose() 
{

	int         i;

	XtSetSensitive(main_menubar, True);
	XtUnmanageChild(form_widget);
	
	/* Free data structures used */
	AuditListSessionsFree(&lsfil);
	if (audit_sessions) {
		for (i = 0; i < lsfil.nsessions; i++)
			XmStringFree(audit_sessions[i]);
		free(audit_sessions);
		audit_sessions = NULL;
	}
	ParameterFileListFree(&lpfile_fil);
	if (parameter_files) {
		for (i = 0; i < lpfile_fil.nfiles; i++)
			XmStringFree(parameter_files[i]);
		free(parameter_files);
		parameter_files = NULL;
	}
	
	if (save_memory) {
		if (form_open) {
			XtDestroyWidget(form_widget);
			form_open = FALSE;
		}
	}
}

void 
ReduceGenerateStop() 
{
	if (form_open) {
		XtDestroyWidget(form_widget);
		form_open = FALSE;
	}
	if (report_file_name) {
		XtFree(report_file_name);
		report_file_name = NULL;
	}
}

/**************************************************************************/
/* Callbacks:                                                             */
/*      CancelCallback()                                                  */
/*      OKCallback()                                                      */
/**************************************************************************/

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	Cardinal    n;
	Arg         args[20];

	n = 0;
	XtSetArg(args[n], XmNvalue,                              "\0"); n++;
	XtSetValues(fname_widget, args, n);

	ReduceGenerateClose();
}

static void 
OKCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	char        *argv[10],
		buf[100],
		*cp,
		*dcp,
		fname[256],
		*scp;
	int         i,
		pid,
		touched,
		wait_stat;
	Cardinal    count,
				n;
	Arg         args[20];
	XmString    (*array_compound_strings)[];
	struct stat sb;

	/**********************************************************************/
	/* Fetch user selected session                                        */
	/**********************************************************************/
	
	n = 0;
	XtSetArg(args[n], XmNselectedItems,     &array_compound_strings); n++;
	XtSetArg(args[n], XmNselectedItemCount,                  &count); n++;
	XtGetValues(ls_list_widget, args, n);
	if (count != 1) {
		/* "You must select an audit session!" */
		ErrorMessageOpen(1530, msg, 4, NULL);
		return;
	}
	for (i = 0; i < lsfil.nsessions; i++)
		if (XmStringCompare(audit_sessions[i], (*array_compound_strings)[0]))
			break;
	if (i >= lsfil.nsessions)
		CodeError();
	strcpy(buf, lsfil.sessions[i]);
	for (cp = buf; *cp && *cp == ' '; cp++)
		;
	strncpy(report_session, cp, 5);
	report_session[5] = '\0';
	for (cp = report_session; *cp && *cp != ' '; cp++)
		;
	*cp = '\0';

	/**********************************************************************/
	/* Fetch user selected parameter file                                 */
	/**********************************************************************/
	
	n = 0;
	XtSetArg(args[n], XmNselectedItems,     &array_compound_strings); n++;
	XtSetArg(args[n], XmNselectedItemCount,                  &count); n++;
	XtGetValues(pf_list_widget, args, n);
	if (count != 1) {
		/* "You must select a parameter file!" */
		ErrorMessageOpen(1540, msg, 6, NULL);
		return;
	}
	for (i = 0; i < lpfile_fil.nfiles; i++)
		if (XmStringCompare(parameter_files[i], (*array_compound_strings)[0]))
			break;
	if (i >= lpfile_fil.nfiles)
		CodeError();
		/* Dies */
	strncpy(report_pfile, lpfile_fil.files[i], NAME_MAX);
	report_pfile[NAME_MAX] = '\0';
	
	/* Fetch name of report file and compress spaces out of name */
	if (report_file_name)
		XtFree(report_file_name);
	report_file_name = XmTextGetString(fname_widget);
	if (! report_file_name)
		CodeError();
		/* Dies */
	touched = FALSE;
	for (dcp = scp = report_file_name; *scp; scp++)
		if (*scp != ' ')
			*dcp++ = *scp;
		else
			touched = TRUE;
	*dcp = '\0';
	if (touched)
		XmTextSetString(fname_widget, report_file_name);
	
	/**********************************************************************/
	/* Flag takes time                                                    */
	/**********************************************************************/

	WorkingOpen(main_shell);    
	
	/**********************************************************************/
	/* Build report                                                       */
	/**********************************************************************/
	
	/* Build name of directory without trailing slash */
	strcpy(fname, REDUCE_REPORT_PATH);
	fname[strlen(fname)-1] = '\0';
	
	/* Test reports file directory exists; create if no */
	if (stat(fname, &sb) < 0) {
		if (mkdir(fname, 0) < 0) {
			/* "Cannot create audit reports directory." */
			/* "Please check permission on directory and on this program." */
			SystemErrorMessageOpen(1545, msg, 27, NULL);
			return;
		}
	}
	
	/* Build name of reduction report file */
	strcpy(fname, REDUCE_REPORT_PATH);
	strcat(fname, report_file_name);
	
	/* Run reduction program */
	if (eaccess(REDUCE_COMMAND, 1) < 0) {
		WorkingClose();
		ErrorMessageOpen(1550, msg, 10, NULL);
		/* AUDIT failure to access reduce command */
		audit_no_resource(REDUCE_COMMAND, OT_REGULAR,
			"Reduce command is not executable", ET_SYS_ADMIN);
		ReduceGenerateClose();
		return;
	}
	
	pid = fork();
	switch(pid) {
		case    -1:
			WorkingClose();
			ErrorMessageOpen(1555, msg, 12, NULL);
			/* AUDIT fork failure */
			audit_no_resource("Process", OT_PROCESS,
				"Cannot fork to run reduce command", ET_SYS_ADMIN);
			return;
			
		case    0:
			/* Point stdout and stderr at report file */
			if (! freopen(fname, "w", stdout))
				exit(0x7f);
			fclose(stderr);
			dup(1);
			argv[0] = strrchr(REDUCE_COMMAND, '/') + 1;
			argv[1] = "-s";
			argv[2] = report_session;
			argv[3] = "-p";
			argv[4] = report_pfile;
			argv[5] = NULL;
			close(ConnectionNumber(XtDisplay(main_shell)));
			execvp(REDUCE_COMMAND, argv);
			/* Should never get here */
			exit(0x7f);
			
		default:
			wait(&wait_stat);
			WorkingClose();

			if (wait_stat & 0xff00) { 
				/* exit status non-zero */
				int ret;
				char ebuf[80];
				ret = (wait_stat >> 8) & 0xff;
				sprintf(ebuf, "%d", ret);
				ErrorMessageOpen(1560, msg, 14, ebuf); 
				sa_audit_audit (ES_AUD_ERROR,
					"Reduce command terminated with error exit");
			} else if (wait_stat & 0xff) { 
				/* signal received */
				ErrorMessageOpen(1570, msg, 16, NULL);
				sa_audit_audit(ES_AUD_ERROR, msg[16]);
			} else
				/* Success -- go ahead and display for user */
				/* file display err msg = "Audit Report is not available." */
				FileDisplayOpen(fname, report_file_name, msg, 8, NULL, 1580);
		
			break;
	}                  
}

static void 
ReduceQuit() 
{
	XtSetSensitive(main_menubar, True);
	WorkingClose();
}

static void 
MakeWidgets() 
{
	char        *cp;
	int         i,
				visible_item_count;
	Widget      action_button_widget,
		cancel_button,
		help_button,
		ok_button,
				ls_scrolled_widget,
				pf_scrolled_widget,
				w;
	Cardinal    n;
	Arg         args[20];
	XmString    xm_string;
				
	/**********************************************************************/
	/*  Form widget                                                       */
	/**********************************************************************/
	
	n = 0;
	XtSetArg(args[n], XmNdefaultPosition,                        TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,                   XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNautoUnmanage,                          FALSE); n++;
	XtSetArg(args[n], XmNdialogStyle,              XmDIALOG_WORK_AREA); n++;
	form_widget = XtCreateWidget("CreateReports", xmFormWidgetClass,
			mother_form, args, n);
	XtAddCallback(form_widget, XmNunmapCallback, CancelCallback, NULL);
	XtAddCallback(form_widget, XmNhelpCallback, HelpDisplayOpen, "audit,CreateReport");

	/* Primary title */
	xm_string = XmStringCreate(msg[18], charset);
	if (! xm_string)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                   xm_string); n++;
	XtSetArg(args[n], XmNtopAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,           XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,            XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "Title", args, n);
	XtManageChild(w);
	XmStringFree(xm_string);
	
	/**********************************************************************/
	/* Audit sessions selection list in scrolled window, titled           */
	/**********************************************************************/
	
	/* Build audit sessions title */
	xm_string = XmStringCreate(msg[19], charset);
	if (! xm_string)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                        xm_string); n++;
	XtSetArg(args[n], XmNtopAttachment,                XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                  w); n++;
	XtSetArg(args[n], XmNleftAttachment,                 XmATTACH_FORM); n++;
	w = XmCreateLabelGadget(form_widget, "SessionsLabel", args, n);
	XtManageChild(w);
	XmStringFree(xm_string);
	
	/* Build audit sessions header */
	xm_string = XmStringCreate(msg[25], charset);
	if (! xm_string)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                   xm_string); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                             w); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	w = XmCreateLabelGadget(form_widget, "SessionsLabel2", args, n);
	XtManageChild(w);
	XmStringFree(xm_string);
	
	/* Build Scrolled Window widget */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                 w); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++; 
	XtSetArg(args[n], XmNborderWidth,                               0); n++;
	XtSetArg(args[n], XmNscrollingPolicy,       XmAPPLICATION_DEFINED); n++;
	XtSetArg(args[n], XmNvisualPolicy,                     XmVARIABLE); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,             XmSTATIC); n++;
	ls_scrolled_widget = XtCreateManagedWidget(
			"SessionsList", xmScrolledWindowWidgetClass, form_widget, args, n);

	/* Build Audit Sessions List widget, managed by Scrolled window */
	n = 0;
	XtSetArg(args[n], XmNsensitive,                              TRUE); n++;
	XtSetArg(args[n], XmNselectionPolicy,             XmSINGLE_SELECT); n++;
	XtSetArg(args[n], XmNlistSizePolicy,                   XmVARIABLE); n++;
	ls_list_widget = XtCreateManagedWidget("SessionsList",
			xmListWidgetClass, ls_scrolled_widget, args, n);

	/**********************************************************************/
	/* Parameter file selection list in scrolled window, titled           */
	/**********************************************************************/
	
	/* Build parameter file title */
	xm_string = XmStringCreate(msg[20], charset);
	if (! xm_string)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                         xm_string); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                  ls_scrolled_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,                  XmATTACH_FORM); n++;
	w = XmCreateLabelGadget(form_widget, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xm_string);
	
	/* Build Scrolled Window widget */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                   w); n++;
	XtSetArg(args[n], XmNleftAttachment,                  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNborderWidth,                                 0); n++;
	XtSetArg(args[n], XmNscrollingPolicy,         XmAPPLICATION_DEFINED); n++;
	XtSetArg(args[n], XmNvisualPolicy,                       XmVARIABLE); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,               XmSTATIC); n++;
	pf_scrolled_widget = XtCreateManagedWidget("ScrolledArea",
			xmScrolledWindowWidgetClass, form_widget, args, n);

	/* Build Parameter Files List widget, managed by Scrolled window */
	n = 0;
	XtSetArg(args[n], XmNlistSizePolicy,                     XmVARIABLE); n++;
	XtSetArg(args[n], XmNsensitive,                                TRUE); n++;
	XtSetArg(args[n], XmNselectionPolicy,               XmSINGLE_SELECT); n++;
	pf_list_widget = XtCreateManagedWidget("List", 
			xmListWidgetClass, pf_scrolled_widget, args, n);

	/**********************************************************************/
	/* Single line prompt for file name                                   */
	/**********************************************************************/

	/* Build prompt label */
	xm_string = XmStringCreate(msg[21], charset);
	if (! xm_string)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                         xm_string); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                  ls_scrolled_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,                 pf_scrolled_widget); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "FileLabel", args, n);
	XtManageChild(w);
	XmStringFree(xm_string);

	/* Build prompt widget */
	cp = report_file_name ? report_file_name : REDUCE_REPORT_FILE ;
	n = 0;
	XtSetArg(args[n], XmNmaxLength,                           NAME_MAX); n++;
	XtSetArg(args[n], XmNvalue,                                     cp); n++;
	XtSetArg(args[n], XmNeditMode,                  XmSINGLE_LINE_EDIT); n++;
	XtSetArg(args[n], XmNeditable,                                TRUE); n++;
	XtSetArg(args[n], XmNtopAttachment,                XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                  w); n++; 
	XtSetArg(args[n], XmNleftAttachment,      XmATTACH_OPPOSITE_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,                                 w); n++;
	XtSetArg(args[n], XmNleftOffset,                                 0); n++;
	fname_widget = XmCreateText(form_widget, "TextEdit", args, n);
	XtManageChild(fname_widget);

	/**********************************************************************/
	/* Create the OK, CANCEL, HELP buttons                                */
	/**********************************************************************/
	CreateThreeButtons (form_widget, pf_scrolled_widget,
				 &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "audit,CreateReport");
}
#endif /* SEC_BASE */

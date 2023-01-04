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
static char	*sccsid = "@(#)$RCSfile: XBackupDel.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:23 $";
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
		XBackupDel.c
	   
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for Backup and Delete Audit Session files
		
	entry points:
		BackupDeleteStart()
		BackupDeleteOpen()
		BackupDeleteClose()
		BackupDeleteStop()
		
	notes:
		code supported a combined Backup/Delete option prior to 8/30/89
		this code to generate the combined button was pulled
			to allow greater safety with mltape and error passing
		I left remaining code in and also in message file in case decide
			to implement at a latter date
		
*/

#include "XAudit.h"
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#ifdef SCO
#include <unistd.h>
#endif

/* Default backup device should be configurable in future release !!!! */
#ifdef AUX
#define DEFAULT_DEVICE "/dev/rfloppy0"
#else
#define DEFAULT_DEVICE "/dev/rfloppy0"
#endif

#define REDIRECT_PROGRAM   "/tcb/lib/redirect"
#define XTERM_PROGRAM      "/usr/bin/X11/xterm"
#define BACKUP_PROGRAM     "/bin/mltape"
#define BACKUP_ARGUMENTS   "-ovBO"
#define BACKUP_TEMPLATE    "/tmp/auXXXXXX"

#define BACKUP_DELETE 2
#define BACKUPREQ     1
#define DELETEREQ     0

extern Widget
	CreateForm(),
	CreateHeader(),
	CreateTitle();

extern void
	CreateThreeButtons();

static short
	*select_session;   /* array to indicate selected sessions */
	
static int 
	action_request,    /* 0:DELETEREQ  1:BACKUPREQ  2:BACKUP_DELETE */
	backupdel_open,    /* Flag indicating widgets exist */
	confirm_open;      /* indicated confirm widget exists */
		
static char   
	*devicename,
	**msg,
	*msg_text;

static Widget
	form_widget, 
	list_widget, 
	devicename_widget, 
	confirm_widget; 
	
static XmString 
	*list_sessions_xmstrings;    /* the list of session files */

static Display  
	*display;

static int
	BackupFiles(),
	CheckDeviceWrite(),
	DeleteFiles();
		
static void   
	BackupDeleteConfirm(),
	BackupDeleteDeleteCallback(),
	BackupDeleteToggleCallback(),
	OKCallback(),
	CancelCallback(),
	MakeWidgets();
		
void BackupDeleteStart()
{
	backupdel_open = FALSE;
}

void 
BackupDeleteOpen()                        
{
	int         i;
	Cardinal    n;
	Arg         args[20];

	/* Desensitize and get clock on screen */
	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);    

	/* Load message text if not already in */
	if (! msg)
		LoadMessage("msg_isso_audit_backupdel", &msg, &msg_text);
		
	/* Load audit sessions data */
	if (AuditListSessionsValidate() || AuditListSessionsGet(&lsfil)) {
		XtSetSensitive(main_menubar, True);
		WorkingClose();
		return;
	}
	
	/* At least one audit session */
	if (lsfil.nsessions < 1) {
		/* "No audit sessions available!" */
		ErrorMessageOpen(1105, msg, 25, NULL);
		XtSetSensitive(main_menubar, True);
		WorkingClose();
		return;
	}
	
	/* Malloc space for pointers to list xmstrings and create list items */
	list_sessions_xmstrings = (XmString *) 
		Malloc(sizeof(XmString) * lsfil.nsessions);
	if (! list_sessions_xmstrings)
		MemoryError();
		/* Dies */
	for (i = 0; i < lsfil.nsessions; i++)
		list_sessions_xmstrings[i] = 
			XmStringCreate(lsfil.sessions[i], charset);

	/* Malloc space for select sessions array */
	select_session = (short *) Malloc(sizeof(short) * lsfil.nsessions);
	if (! select_session)
		MemoryError();
		/* Dies */

	/* Build widgets if not already created */
	if (! backupdel_open) {
		MakeWidgets();
		backupdel_open = TRUE;
		display = XtDisplay(main_shell);
	}    
	
	/* Load data into widget set */
	n = 0;
	XtSetArg(args[n], XmNselectedItems,           NULL); n++;
	XtSetArg(args[n], XmNselectedItemCount,          0); n++;
	XtSetArg(args[n], XmNitems,           list_sessions_xmstrings); n++;
	XtSetArg(args[n], XmNitemCount,               lsfil.nsessions); n++;
	XtSetValues(list_widget, args, n);

	/* Crank it up */
	CenterForm(form_widget);
	XmListDeselectAllItems(list_widget);
	WorkingClose();
}    
	
void 
BackupDeleteClose() {

	int         i;
		
	XSync(display, 0);
	
	XtUnmanageChild(form_widget);
	XtSetSensitive(main_menubar, True);
	
	/* Free compound strings */
	if (list_sessions_xmstrings) {
		for (i = 0; i < lsfil.nsessions; i++)
			XmStringFree(list_sessions_xmstrings[i]);
		free(list_sessions_xmstrings);
		list_sessions_xmstrings = NULL;
	}
	
	/* Free data structures used to monitor sessions selected */
	if (select_session) {
		free(select_session);
		select_session = NULL;
	}
	
	/* Free data structures by AuditListSessionsGet() */
	AuditListSessionsFree(&lsfil);
	
	if (save_memory) {
		if (backupdel_open) {
			XtDestroyWidget(form_widget);
			backupdel_open = FALSE;
		}
		if (confirm_open) {
			XtDestroyWidget(confirm_widget);
			confirm_open = FALSE;
		}
	}
	XSync(display, 0);
}                                                        

void 
BackupDeleteStop() 
{
	if (backupdel_open) {
		XtDestroyWidget(form_widget);
		backupdel_open = FALSE;
	}
	if (confirm_open) {
		XtDestroyWidget(confirm_widget);
		confirm_open = FALSE;
	}
	if (devicename) {
		XtFree(devicename);
		devicename = NULL;
	}
}

static void 
BackupDeleteToggleCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	/********************************************************************/
	/* The Backup or Delete Toggle button was pressed.                  */
	/********************************************************************/

	if (! strcmp(ptr, "Backup")) 
		action_request = BACKUPREQ;
	else if (! strcmp(ptr, "Delete"))
		action_request = DELETEREQ;
	else 
		action_request = BACKUP_DELETE;    
}

static void 
OKCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	int     i,
		j;
	char        *strings[2];
	Cardinal    count,
			n;
	Arg         args[20];
	XmString    confirm_xmstring,
			*selected_xmstrings;

	/* Fetch items selected */
	n = 0;
	XtSetArg(args[n], XmNselectedItems,       &selected_xmstrings); n++;
	XtSetArg(args[n], XmNselectedItemCount,                &count); n++;
	XtGetValues(list_widget, args, n);
	
	/* At least one item must be selected */
	if (count < 1) {
		ErrorMessageOpen(1110, msg, 0, NULL);
		return;
	}    
	
	/* Set select_session[i] indicator on if list item[i] selected */
	for (i = 0; i < lsfil.nsessions; i++)
		select_session[i] = FALSE;
	for (j = 0; j < count; j++) {
		for (i = 0; i < lsfil.nsessions; i++)
			if (XmStringCompare(list_sessions_xmstrings[i], 
					selected_xmstrings[j])) 
				break;
		select_session[i] = TRUE;
	}    
	
	/* Desensitize widget coming from */
	XtSetSensitive(form_widget, False);
	
	/* Build confirmation widget if not already around */
	if (! confirm_open) {
		n = 0;
		XtSetArg(args[n], XmNtopAttachment,    XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,   XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNborderWidth,      1); n++;
		XtSetArg(args[n], XmNresizePolicy,     XmRESIZE_ANY); n++;
		XtSetArg(args[n], XmNdialogStyle,      XmDIALOG_WORK_AREA); n++;
		XtSetArg(args[n], XmNdialogType,       XmDIALOG_QUESTION); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
 		XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
#endif
		confirm_widget = XmCreateMessageBox(mother_form, "Confirm", 
			args, n);
					  
		XtAddCallback(confirm_widget, XmNokCallback, 
				BackupDeleteConfirm, "Go");
		XtAddCallback(confirm_widget, XmNcancelCallback, 
				BackupDeleteConfirm, "No");
				
		w = XmMessageBoxGetChild(confirm_widget, XmDIALOG_OK_BUTTON);
		confirm_xmstring = XmStringCreate(msg[6], charset);
		n = 0;
		XtSetArg(args[n], XmNlabelString, confirm_xmstring); n++;
		XtSetValues(w, args, n);
		XmStringFree(confirm_xmstring);
		
		w = XmMessageBoxGetChild(confirm_widget, XmDIALOG_CANCEL_BUTTON);
		confirm_xmstring = XmStringCreate(msg[7], charset);
		n = 0;
		XtSetArg(args[n], XmNlabelString, confirm_xmstring); n++;
		XtSetValues(w, args, n);
		XmStringFree(confirm_xmstring);
	 
		w = XmMessageBoxGetChild(confirm_widget, XmDIALOG_HELP_BUTTON);
		XtUnmanageChild(w);
		XtDestroyWidget(w);
		
		confirm_open = TRUE;
	}
	
	/* Load data into the widget */
	confirm_xmstring = XmStringCreate(msg[3+action_request], charset);
	n = 0;
	XtSetArg(args[n], XmNmessageString, confirm_xmstring); n++;
	XtSetValues(confirm_widget, args, n);
	XmStringFree(confirm_xmstring);
	
	XtManageChild(confirm_widget);
}

static void 
BackupDeleteConfirm(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	if (strcmp(ptr, "Go")) {
		/* Do not proceed */
		XtUnmanageChild(confirm_widget);
		XtSetSensitive(form_widget, True);
		return;
	}
	
	/* Perform the action */
	WorkingOpen(main_shell);
	if (BackupFiles() || DeleteFiles()) {
		/* If problems, start over */
		WorkingClose();
		XtUnmanageChild(confirm_widget);
		XtSetSensitive(form_widget, True);
		return;
	}
	
	/*
		Finished with backup and/or delete requests
		
		close out functional unit for now -- do not have time to
		mess with code to truly clean up screen and data structures,
		may want to add this code later -- H.
	*/
	
	WorkingClose();
	XtSetSensitive(form_widget, True);
	XtUnmanageChild(confirm_widget);
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	BackupDeleteClose();
}
				
static void 
MakeWidgets()
{
	int         i;
	char        *cp;
	XmString    backupdel_xmstring;
	Widget      backupdel_sessions_scrolled_widget, 
			backupdel_radiobox_widget,
			backupdel_backup_widget, 
			backupdel_delete_widget,
			backupdel_both_widget,
			help_button, 
			cancel_button, 
			ok_button, 
			w;
	Cardinal    n;
	Arg         args[20];
	

	action_request = BACKUPREQ;         /* default toggle setting */
	
	/**********************************************************************/
	/* Form widget                                                        */
	/**********************************************************************/
	
	form_widget = CreateForm (mother_form, "BackupDelete");
	XtAddCallback(form_widget, XmNhelpCallback, 
			HelpDisplayOpen, "audit,BackupDelete");
  
	/* Title */
	w = CreateTitle (form_widget, msg[8]);

	/**********************************************************************/
	/* Session files list in scrolled window, titled                      */
	/**********************************************************************/

	/* Session list label */
	w = CreateHeader (form_widget, msg[10], w, True, NULL);
	
	/* Build Scrolled Window widget */                           
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                 w); n++;
	XtSetArg(args[n], XmNtopOffset,                                 0); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++; 
	XtSetArg(args[n], XmNborderWidth,                               0); n++;
	XtSetArg(args[n], XmNscrollingPolicy,       XmAPPLICATION_DEFINED); n++;
	XtSetArg(args[n], XmNvisualPolicy,                     XmVARIABLE); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,             XmSTATIC); n++;
	backupdel_sessions_scrolled_widget = XtCreateManagedWidget(
			"SessionsScrolledArea", xmScrolledWindowWidgetClass, 
			form_widget, args, n);

	/* Build Sessions List widget, managed by Scrolled window */
	n = 0;
	XtSetArg(args[n], XmNsensitive,                              TRUE); n++;
	XtSetArg(args[n], XmNselectionPolicy,           XmMULTIPLE_SELECT); n++; 
	XtSetArg(args[n], XmNlistSizePolicy,                   XmVARIABLE); n++;
	list_widget = XtCreateManagedWidget("SessionsList", xmListWidgetClass,
			backupdel_sessions_scrolled_widget, args, n);
							   
	/**********************************************************************/
	/* Backup/Delete toggles in radiobox                                  */
	/**********************************************************************/

	/* Build Radio Box widget */
	n = 0;
	XtSetArg(args[n], XmNpacking,                       XmPACK_COLUMN); n++;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,backupdel_sessions_scrolled_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNentryClass,        xmToggleButtonWidgetClass); n++;
	XtSetArg(args[n], XmNorientation,                    XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNradioAlwaysOne,                         TRUE); n++;
	XtSetArg(args[n], XmNradioBehavior,                          TRUE); n++;
	backupdel_radiobox_widget = XmCreateRowColumn(
					form_widget, "RadioBox", args, n);
	XtManageChild(backupdel_radiobox_widget);
	
	/* Backup radio button */
	backupdel_xmstring = XmStringCreate(msg[11], charset);
	n = 0;
	XtSetArg(args[n], XmNalignment,                XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,              backupdel_xmstring); n++;
	XtSetArg(args[n], XmNset,                                    TRUE); n++;
	backupdel_backup_widget = XmCreateToggleButton( 
			backupdel_radiobox_widget, "RadioButton", args, n);
	XtAddCallback(backupdel_backup_widget, XmNvalueChangedCallback,
			BackupDeleteToggleCallback, "Backup");
	XtManageChild(backupdel_backup_widget);
	XmStringFree(backupdel_xmstring);
	
	/* Delete radio button */
	backupdel_xmstring = XmStringCreate(msg[12], charset);
	n = 0;
	XtSetArg(args[n], XmNalignment,                XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,              backupdel_xmstring); n++;
	XtSetArg(args[n], XmNset,                                   FALSE); n++;
	backupdel_delete_widget = XmCreateToggleButton( 
			backupdel_radiobox_widget, "RadioButton", args, n);
	XtAddCallback(backupdel_delete_widget, XmNvalueChangedCallback,
			BackupDeleteToggleCallback, "Delete");
	XtManageChild(backupdel_delete_widget);
	XmStringFree(backupdel_xmstring);
	
	/**********************************************************************/
	/* Single line prompt for output device                               */
	/**********************************************************************/

	/* Build prompt label */
	backupdel_xmstring = XmStringCreate(msg[14], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,     backupdel_xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,   XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,       backupdel_radiobox_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,       XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "Label", args, n);
	XtManageChild(w);
	XmStringFree(backupdel_xmstring);
	
	/* Build prompt widget */
	cp = devicename ? devicename : DEFAULT_DEVICE;
	n = 0;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,     True);n++;
#endif
	XtSetArg(args[n], XmNmaxLength,       DEVICE_NAME_MAX); n++;
	XtSetArg(args[n], XmNvalue,           cp); n++;
	XtSetArg(args[n], XmNeditMode,        XmSINGLE_LINE_EDIT); n++;
	XtSetArg(args[n], XmNeditable,        TRUE); n++;
	XtSetArg(args[n], XmNtopAttachment,   XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,       backupdel_radiobox_widget); n++; 
	XtSetArg(args[n], XmNleftAttachment,  XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,      w); n++;
	devicename_widget = XmCreateText(form_widget, "TextEdit", args, n);
	XtManageChild(devicename_widget);
#ifdef TRAVERSAL   
	XmAddTabGroup(devicename_widget);
#endif

	/**********************************************************************/
	/* Create the OK, Cancel and Help buttons			      */
	/**********************************************************************/
	CreateThreeButtons (form_widget, devicename_widget,
					&ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback,
			CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
		 HelpDisplayOpen,"audit,BackupDelete");
}

/*
	The following code is adapted from SecureWare's maintenance.c code
*/

static int 
BackupFiles() {
	char    *argv[30],
		buf[256],
		comp_file[AUDIT_PATHSIZE],
		*cp,
		*executable_name,
		filename[AUDIT_PATH_LEN + 1],
		tempfile[sizeof(BACKUP_TEMPLATE)];
	struct  log_header  log;
	int     c,
		i, 
		j,
		n,
		pid,
		ret,
		session,
		wait_stat;
	FILE    *fp,
		*tfp;
	extern char 
		**environ;
	
	/* Quick check for backup request */
	if (action_request != BACKUPREQ && action_request != BACKUP_DELETE)
		return 0;

	/* Fetch name of device */
	if (devicename)
		XtFree(devicename);
	devicename = XmTextGetString(devicename_widget);
	
	/* Check write access to device */
	if (CheckDeviceWrite(devicename)) {
		audit_no_resource(AUDIT_LOGDIR, OT_DEVICE, msg[41], ET_SYS_ADMIN);
		return 1;
	}
	
	/* Check read/search permission on Audit log directory */    
	if (eaccess(AUDIT_LOGDIR, 7) < 0)  {   /* check read, search perm */
		ErrorMessageOpen(1172, msg, 30, NULL);
		/* AUDIT permission failure on audit log directory */
		audit_no_resource(AUDIT_LOGDIR, OT_DIRECTORY, msg[42], ET_SYS_ADMIN);
		return 1;
	}
	
	/* Create temp file with list of backup files */
	strcpy(tempfile, BACKUP_TEMPLATE);
	mktemp(tempfile);
	tfp = fopen(tempfile, "w");
	if (! tfp) {
		ErrorMessageOpen(1140, msg, 22, NULL);
		audit_no_resource(filename, OT_REGULAR, msg[43], ET_SYS_ADMIN);
		return 1;
	}
	
	/* Pick up all files to backup */
	for (i = 0; i < lsfil.nsessions; i++) {
		if (select_session[i]) {
			
			/* Build audit session name */
			sprintf(filename, "%s%.*s%0*ld", AUDIT_LOGDIR,
				strlen(AUDIT_LOG_FILENAME) - AUDIT_DIGITS,
				AUDIT_LOG_FILENAME, AUDIT_DIGITS,
				atol(lsfil.sessions[i]));

			/* Check audit file and get associated filenames */
			fp = fopen (filename, "r");
			if (! fp) {
				ErrorMessageOpen(1120, msg, 18, NULL); 
				audit_no_resource(filename, OT_REGULAR, msg[44], ET_SYS_ADMIN);
				select_session[i] = -1;
				continue;
			}
	
			/* Read in magic collection file number */
			if (fread(&log, sizeof(log), 1, fp) != 1) {
				ErrorMessageOpen(1125, msg, 20, NULL); 
				fclose(fp);
				audit_no_resource(filename, OT_REGULAR, msg[45], ET_SYS_ADMIN);
				select_session[i] = -1;
				continue;
			}
	
			/* Test that this is indeed an audit collection file */
			if (strncmp(log.id, AUDIT_LOGID, sizeof(log.id))) {
				ErrorMessageOpen(1130, msg, 20, NULL);
				fclose(fp);
				/* AUDIT inconsistent audit log file */
				audit_no_resource(filename, OT_REGULAR, msg[45], ET_SYS_ADMIN);
				select_session[i] = -1; /* Flag problem */
				continue;
			}
	
			/* Passed security -- register file for backing up */
			fputs(filename, tfp);
			putc('\n', tfp);
		 
			/* Now fetch associated file for backing up */
			for (j = 0; j < log.comp_files; j++) {
				if (fread(comp_file, sizeof(comp_file), 1, fp) != 1) {
					/* AUDIT inconsistent audit log file */
					ErrorMessageOpen(1145, msg, 20, NULL);
					audit_no_resource(filename, OT_REGULAR, msg[45], ET_SYS_ADMIN);
					fclose(fp);
					goto fetch_next_backup_file;
				}
				if (eaccess(comp_file, R_OK)) {
					/* AUDIT missing collection file */
					audit_no_resource(comp_file, OT_REGULAR, msg[46], ET_SYS_ADMIN);
					continue;
				}
				fputs(comp_file, tfp);
				putc('\n', tfp);
			}
			fclose(fp);
		}
fetch_next_backup_file:
		;
	}
	fclose(tfp);
	
	/* Prepare arguments for the backup program */
	cp = strrchr(XTERM_PROGRAM, '/');
	if (cp)
		cp++;
	else
		cp = XTERM_PROGRAM;
	executable_name = cp;
	
	n = 0;
	argv[n++] = executable_name;
	argv[n++] = "-title";
	argv[n++] = msg[47];
	argv[n++] = "-e";
	argv[n++] = REDIRECT_PROGRAM;
	argv[n++] = BACKUP_PROGRAM;
	argv[n++] = BACKUP_ARGUMENTS;
	argv[n++] = devicename;
	argv[n++] = "stdin";
	argv[n++] = tempfile;
	argv[n++] = NULL;

	/* Fork and run the backup program */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	switch (pid = fork())  {
		case    -1: /* error - can't fork sub-process */
			signal(SIGHUP, SIG_DFL);
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			ret = -1;
			break;
			
		case    0:  /* child */
			/* Reset signals */
			signal(SIGHUP, SIG_DFL);
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			/* Run xterm program */
			close(ConnectionNumber(XtDisplay(main_shell)));
			execv(XTERM_PROGRAM, argv);
			exit(0x7f);
		
		default:
			wait(&wait_stat);
			/* Put signals back */
			signal(SIGHUP, SIG_DFL);
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			if (! (wait_stat & 0xFF)) {
				/* terminated due to exit() */
				ret = (wait_stat >> 8) & 0xFF; /* exit status */
				if (ret == 0x7f)
					ret = -1;
			}
			else 
				/* terminated by signal */
				ret = -1;
			break;
	}

	/* Finished with temporary file, erase it */
	unlink(tempfile);

	/* 'ret' has result code here from exec */
	if (ret) {
		/* Terminal shell program failed */
		ErrorMessageOpen(1182, msg, 38, NULL);
		audit_no_resource(XTERM_PROGRAM, OT_PROCESS, msg[48], ET_SYS_ADMIN);
		return 1;
	}

	/* AUDIT run of backup program */
	sa_audit_audit(ES_AUD_ARCHIVE, msg[49]);
}

static int 
DeleteFiles() {
	char    buf[256],
		comp_file[AUDIT_PATHSIZE],
		filename[AUDIT_PATH_LEN + 1];
	struct  log_header  log;
	struct  audit_init  au;
	int     c,
		fd,
		i, 
		j,
		session;
	FILE    *fp;

	/* Quick exit if no delete requested */
	if (action_request != DELETEREQ && action_request != BACKUP_DELETE)
		return 0;
		
	/* Validate rwx permission on directory */
	if (eaccess (AUDIT_LOGDIR, 7) < 0)  {
		ErrorMessageOpen(1174, msg, 30, NULL);
		/* AUDIT failure to access AUDIT_LOGDIR */
		audit_no_resource(AUDIT_LOGDIR, OT_DIRECTORY, msg[50], ET_SYS_ADMIN);
		return 1;
	}

	/* Fetch current session info -- cannot delete current active session */
	fp = fopen(AUDIT_PARMSFILE, "r");
	if (! fp) {
		ErrorMessageOpen(1176, msg, 32, NULL);
		/* AUDIT failure to open AUDIT_PARMSFILE */
		audit_security_failure(OT_SUBSYS, AUDIT_PARMSFILE, msg[51], ET_SYS_ADMIN);
		return 1;
	} 
	if (fread(&au, sizeof(au), 1, fp) != 1) {
		ErrorMessageOpen(1178, msg, 34, NULL);
		fclose(fp);
		/* AUDIT failure to read AUDIT_PARMSFILE */
		audit_security_failure(OT_SUBSYS, AUDIT_PARMSFILE, msg[52], ET_SYS_ADMIN);
		return 1;
	}
	fclose(fp);

	/* Pick up all files to delete */
	for (i = 0; i < lsfil.nsessions; i++) {
		if (select_session[i] > 0) {
			
			/* Build audit session name */
			sprintf(filename, "%s%.*s%0*ld", AUDIT_LOGDIR,
				strlen(AUDIT_LOG_FILENAME) - AUDIT_DIGITS,
				AUDIT_LOG_FILENAME, AUDIT_DIGITS,
				atol(lsfil.sessions[i]));
					
			/* Must not be current active session */
			if (au.session == atoi(lsfil.sessions[i])) {
				fd = open(AUDIT_WDEVICE, O_WRONLY);
				if (fd >= 0) {
					ErrorMessageOpen(1180, msg, 36, NULL);
					fclose(fp);
					close(fd);
					select_session[i] = -1; /* Flag problems with */
					continue;
				}
			}
 
			/* Check audit file and get associated filenames */
			fp = fopen(filename, "r");
			if (! fp) {
				ErrorMessageOpen(1150, msg, 18, NULL); 
				audit_no_resource(filename, OT_REGULAR, msg[44], ET_SYS_ADMIN);
				select_session[i] = -1;
				continue;
			}
	
			/* Read in magic collection file number */
			if (fread(&log, sizeof(log), 1, fp) != 1) {
				ErrorMessageOpen(1155, msg, 20, NULL); 
				fclose(fp);
				audit_no_resource(filename, OT_REGULAR, msg[45], ET_SYS_ADMIN);
				select_session[i] = -1;
				continue;
			}
	
			/* Test that this is indeed an audit collection file */
			if (strncmp(log.id, AUDIT_LOGID, sizeof(log.id))) {
				ErrorMessageOpen(1160, msg, 20, NULL);
				fclose(fp);
				/* AUDIT inconsistent audit log file */
				audit_no_resource(filename, OT_REGULAR, msg[45], ET_SYS_ADMIN);
				select_session[i] = -1; /* Flag problem */
				continue;
			}
	
			/* Passed security -- delete later */
		 
			/* Now fetch associated files to delete */
			for (j = 0; j < log.comp_files; j++) {
				if (fread(comp_file, sizeof(comp_file), 1, fp) != 1) {
					ErrorMessageOpen(1165, msg, 20, NULL);
					/* AUDIT inconsistent audit log file */
					audit_no_resource(filename, OT_REGULAR, 
						msg[45], ET_SYS_ADMIN);
					fclose(fp);
					goto fetch_next_delete_file;
				}
				if (unlink(comp_file) < 0) {
					/* AUDIT failure to unlink one of session files */
					audit_no_resource(comp_file, OT_REGULAR, 
						   msg[53], ET_SYS_ADMIN);
				}
			}
			fclose(fp);
			/* Unlink primary file */
			if (unlink(filename) < 0) {
				/* AUDIT failure to unlink log file for session */
				audit_no_resource(comp_file, OT_REGULAR,
						msg[53], ET_SYS_ADMIN);
			} 
			else {
				/* AUDIT successful session removal */
				sa_audit_audit(ES_AUD_MODIFY, msg[54]);
				XmListDeselectItem(list_widget, list_sessions_xmstrings[i]);
				XmListDeleteItem(list_widget, list_sessions_xmstrings[i]);
			}
		}
fetch_next_delete_file:
		;
	}
}

static int 
CheckDeviceWrite(device)
	char    *device;
{
	struct stat sb;
	int     ret;

	ret = stat(device, &sb);
	if (ret < 0)  {
		/* Device does not exist. */
		ErrorMessageOpen(1190, msg, 55, NULL);
		return 1;
	}
	if ((sb.st_mode & S_IFMT) != S_IFCHR) {
		/* Device is not a character special device. */
		ErrorMessageOpen(1191, msg, 58, NULL);
		return 1;
	}
	if (eaccess(device, 2) < 0) {
		/* Device is not accessible. */
		ErrorMessageOpen(1192, msg, 61, NULL);
		return 1;
	}
	return 0;
}
#endif /* SEC_BASE */

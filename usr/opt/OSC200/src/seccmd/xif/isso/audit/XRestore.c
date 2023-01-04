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
static char	*sccsid = "@(#)$RCSfile: XRestore.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:50 $";
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
		XRestore.c
	   
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for Restore Session options
		
	entry points:
		RestoreStart()
		RestoreOpen()
		RestoreClose()
		RestoreStop()
*/

#include "XAudit.h"
#include <Xm/Form.h>
#include <Xm/MessageB.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>

/* Default backup device should be configurable in future release !!!! */
#ifdef AUX
#define DEFAULT_DEVICE "/dev/rfloppy0"
#else
#define DEFAULT_DEVICE "/dev/rfloppy0"
#endif
#define REDIRECT_PROGRAM   "/tcb/lib/redirect"
#define XTERM_PROGRAM      "/usr/bin/X11/xterm"
#define RESTORE_PROGRAM    "/bin/mltape"
#define RESTORE_ARGUMENTS  "-imvdBI"

static int 
	confirm_open,       /* Flag indicating confirm widget open */
	form_open;  
		
static char   
	**msg,
	*msg_text;

static char
	*devicename;           /* this holds the input device */
	
static Widget
	confirm_widget,
	devicename_widget, 
	form_widget, 
	list_widget;
	
static XmString 
	*list_xmstrings;
		
static Display  
	*display;

static void
	MakeWidgets(),
	CancelCallback(),
	OKCallback(),
	RestoreWorking();

static int
	RestoreFiles(),
	CheckDeviceRead();
		
void 
RestoreStart()
{
	confirm_open = FALSE;
	form_open = FALSE;
}

void 
RestoreOpen()                        
{
	Cardinal    n;
	Arg         args[20];

	/* Desensitize and flag busy */
	XtSetSensitive(main_menubar, False);
	WorkingOpen(main_shell);
	
	/* Load in message text */
	if (! msg)
		LoadMessage("msg_isso_audit_restore", &msg, &msg_text);
	
	/* Build widgets if not laying around */
	if (! form_open) {
		MakeWidgets();
		form_open = TRUE;      /* Restore widgets exist */
		display = XtDisplay(main_shell);
	}
	
	/* Doit */
	CenterForm(form_widget);
	WorkingClose();
}    
	
void 
RestoreClose() 
{
	XSync(display, 0);
	XtUnmanageChild(form_widget);
	XtSetSensitive(main_menubar, True);
	if (save_memory) {
		if (form_open) {
			XtDestroyWidget(form_widget);
			form_open = FALSE;
		}
		if (confirm_open) {
			XtDestroyWidget(confirm_widget);
			confirm_open = FALSE;
		}
	}
	XSync(display, 0);
}

void 
RestoreStop() 
{
	if (form_open) {
		XtDestroyWidget(form_widget);
		form_open = FALSE;
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
OKCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	int         i,
		j; 
	
	Cardinal    n;
	Arg         args[20];
	XmString    xmstring;
	
	XtSetSensitive(form_widget, False);
	if (! confirm_open) {
		confirm_open = TRUE;
		
		xmstring = XmStringCreate(msg[3], charset);
		if (! xmstring)
			MemoryError();

		n = 0;
		XtSetArg(args[n], XmNtopAttachment,     XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNleftAttachment,    XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNmessageString,     xmstring); n++;
		XtSetArg(args[n], XmNborderWidth,       1); n++;
		XtSetArg(args[n], XmNdialogStyle,      XmDIALOG_WORK_AREA); n++;
		XtSetArg(args[n], XmNdialogType,        XmDIALOG_QUESTION); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
	 	XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
#endif
		confirm_widget = XmCreateMessageBox(mother_form, "Confirm",
			args, n);
		XtAddCallback(confirm_widget, XmNokCallback, 
			RestoreWorking,"Go");
		XtAddCallback(confirm_widget, XmNcancelCallback, 
			RestoreWorking, "No");
		XmStringFree(xmstring);
				
		w = XmMessageBoxGetChild(confirm_widget, XmDIALOG_OK_BUTTON);
		n = 0;
		xmstring =  XmStringCreate(msg[4], charset);
		XtSetArg(args[n], XmNlabelString,        xmstring); n++;
		XtSetValues(w, args, n);
		XmStringFree(xmstring);
		
		w = XmMessageBoxGetChild(confirm_widget, XmDIALOG_CANCEL_BUTTON);
		n = 0;
		xmstring =  XmStringCreate(msg[5], charset);
		XtSetArg(args[n], XmNlabelString,        xmstring); n++;
		XtSetValues(w, args, n);
		XmStringFree(xmstring);
	 
		w = XmMessageBoxGetChild(confirm_widget, XmDIALOG_HELP_BUTTON);
		XtUnmanageChild(w);
		XtDestroyWidget(w);
	}
	XtManageChild(confirm_widget);
}

static void 
RestoreWorking(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	XtUnmanageChild(confirm_widget);
	if (! strcmp("Go", ptr)) {
		WorkingOpen(main_shell);
		RestoreFiles();
		WorkingClose();
		RestoreClose();
		return;
	}
	XtSetSensitive(form_widget, True);
}


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
	XtSetValues(devicename_widget, args, n);

	RestoreClose();
}
				
		
static void 
MakeWidgets() 
{    
	int         i;
	char        *cp;
	XmString    xmstring;
	Widget      head_label_widget,
		cancel_button,
		help_button,
		ok_button,
				action_button_widget,
				w;
	Cardinal    n;
	Arg         args[20];
	
	/* Functional form */
	n = 0;
	XtSetArg(args[n], XmNdefaultPosition,                        TRUE); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNresizePolicy,                   XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNautoUnmanage,                          FALSE); n++;
	XtSetArg(args[n], XmNallowOverlap,                          FALSE); n++;
	XtSetArg(args[n], XmNdialogStyle,              XmDIALOG_WORK_AREA); n++;
	form_widget = XtCreateWidget("Restore", xmFormWidgetClass,
			mother_form, args, n);
	XtAddCallback(form_widget, XmNhelpCallback, HelpDisplayOpen, "audit,Restore");
  
	/* Title */
	xmstring = XmStringCreate(msg[6], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,        xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,      XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNborderWidth,        0); n++;
	XtSetArg(args[n], XmNalignment,          XmALIGNMENT_CENTER); n++;
	head_label_widget = XmCreateLabelGadget(form_widget,
			"Title", args, n);
	XtManageChild(head_label_widget);
	XmStringFree(xmstring);
	
	/* Label for device name prompt */
	xmstring = XmStringCreate(msg[7], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,         xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,       XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,           head_label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,      XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNborderWidth,         0); n++;
	XtSetArg(args[n], XmNalignment,           XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "Label", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
	/* Device to restore from */
	cp = devicename ? devicename : DEFAULT_DEVICE;
	n = 0;
	XtSetArg(args[n], XmNmaxLength,           DEVICE_NAME_MAX); n++;
	XtSetArg(args[n], XmNvalue,               cp); n++;
	XtSetArg(args[n], XmNeditMode,            XmSINGLE_LINE_EDIT); n++;
	XtSetArg(args[n], XmNeditable,            TRUE); n++;
	XtSetArg(args[n], XmNtopAttachment,       XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,           head_label_widget); n++; 
	XtSetArg(args[n], XmNleftAttachment,      XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,          w); n++;
	XtSetArg(args[n], XmNsensitive,           TRUE); n++;
	devicename_widget = XmCreateText(form_widget, "TextEdit", args, n);
	XtManageChild(devicename_widget);
	
	/**********************************************************************/
	/* Create the OK, CANCEL, HELP buttons                                */
	/**********************************************************************/
	CreateThreeButtons (form_widget, devicename_widget,
				 &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				 HelpDisplayOpen, "audit,Restore");
}

static int 
RestoreFiles()
{
	char    *argv[20],
		*cp,
		*executable_name;
	int     n,
		pid,
		ret,
		wait_stat;

	/* Check read, write, search perm to audit log dir */
	if (eaccess (AUDIT_LOGDIR, 7) < 0) {
		ErrorMessageOpen(1610, msg, 16, NULL);
		/* AUDIT permission failure in AUDIT_LOGDIR */
		audit_no_resource(AUDIT_LOGDIR, OT_DIRECTORY,
				"No read/write/execute access", ET_SYS_ADMIN);
		return 1;
	}

	/* Fetch device name */
	if (devicename)
		XtFree(devicename);
	devicename = XmTextGetString(devicename_widget);
	
	/* Check read access to device */
	if (CheckDeviceRead(devicename)) {
		ErrorMessageOpen(1620, msg, 19, NULL);
		audit_no_resource(AUDIT_LOGDIR, OT_DEVICE, "No read access", ET_SYS_ADMIN);
		return 1;
	}
	
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
	argv[n++] = msg[32];
	argv[n++] = "-e";
	argv[n++] = REDIRECT_PROGRAM;
	argv[n++] = RESTORE_PROGRAM;
	argv[n++] = RESTORE_ARGUMENTS;
	argv[n++] = devicename;
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
			setgid(getgid());
			close(ConnectionNumber(XtDisplay(main_shell)));
			/* Run xterm program */
			execv(XTERM_PROGRAM, argv);
			/* Should never get here */
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

	/* 'ret' has result code here from exec */
	if (ret) {
		/* Terminal shell program failed */
		ErrorMessageOpen(1630, msg, 21, NULL);
		audit_no_resource(XTERM_PROGRAM, OT_PROCESS, "Shell program failed", 
		ET_SYS_ADMIN);
		return 1;
	}

	/* AUDIT run of restore program */
	sa_audit_audit(ES_AUD_ARCHIVE, "Audit file restore complete");
	return 1;
}

static int 
CheckDeviceRead(device)
	char    *device;
{
	struct stat sb;
	int     mode,
			ret;

	ret = stat(device, &sb);
	if (ret < 0)  {
		ErrorMessageOpen(1690, msg, 23, NULL);
		return 1;
	}
	if ((sb.st_mode & S_IFMT) != S_IFCHR) {
		ErrorMessageOpen(1691, msg, 26, NULL);
		return 1;
	}
	if (eaccess(device, 4) < 0) {
		ErrorMessageOpen(1692, msg, 29, NULL);
		return 1;
	}
	return 0;
}
#endif /* SEC_BASE */

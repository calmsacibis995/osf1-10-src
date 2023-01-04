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
static char	*sccsid = "@(#)$RCSfile: XMaint.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:35 $";
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
		XMaint.c
		
	copyright:
		Copyright (c) 1989, 1990 SecureWare, Inc.
		ALL RIGHTS RESERVED
	
	function:
		XIsso interface routines for:
			enable/disable audit system
		
	entry points:
		MaintenanceStart()   Loads Message Text
		MaintenanceStop()    No op 
		DisableAudit()
		EnableAudit()
		CheckAuditEnabled()
		DisableAuditDoIt()
		
	notes:
		widgets destroyed on exit .. these windows will probably not be
		accessed multiple times in one run of Xaudit
		
	logic path for Enable, Disable is similar:
	
		Audit pulldown menu button calls EnableAudit() via callback
		
		EnableAudit() builds a please confirm widget tagged 
			enable_disable_widget
			
		enable_disable_widget calls Callback_EnableAudit() via callback
		
		Callback_EnableAudit() calls ...DoIt() code to coerce the change
		
		if DoIt() reports success, then Callback_EnableAudit() builds a 
		confirmation widget to announce success
		
		the confirmtion_widget has single button which goes to OKCallback()
*/

#include "XAudit.h"
#include "sys/secioctl.h"
#include <Xm/MessageB.h>

int 
	CheckAuditEnabled(),
	OpenAuditDevice();

int 
	DisableAuditDoIt();

static Widget
	enable_disable_widget,
	confirmation_widget;

static char 
	**msg,
	*msg_text;
	
static int
	DisableAuditValidate(),
	EnableAuditDoIt(),
	EnableAuditValidate();

static void
	Callback_EnableAudit(),
	Callback_DisableAudit(),
	OKCallback();
	
void 
MaintenanceStart() 
{
	LoadMessage("msg_isso_audit_maint", &msg, &msg_text);
}

void 
MaintenanceStop() 
{
	/* Not used */
}

void 
EnableAudit() 
{
	Cardinal    n;
	Arg         args[20];
	XmString    xmstring;
	
	if (EnableAuditValidate())
		return;
		
	XtSetSensitive(main_menubar, False);
		
	xmstring = XmStringCreate(msg[0], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	/* Default button type and dialog type */
 	XtSetArg(args[n], XmNdialogType, XmDIALOG_QUESTION); n++;
	XtSetArg(args[n], XmNmessageString,       xmstring); n++;
 	XtSetArg(args[n], XmNborderWidth,                1); n++;
 	XtSetArg(args[n], XmNautoUnmanage,           FALSE); n++;
	XtSetArg(args[n], XmNtopAttachment,  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
 	XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
#endif
	enable_disable_widget = XmCreateMessageBox(mother_form,
				                "Enable", args, n);
	XmStringFree(xmstring);
	
	/* Added validate and StringFree  -works */
	/* Added callbacks*/
	XtAddCallback(enable_disable_widget, XmNokCallback, 
		Callback_EnableAudit, 1);
	XtAddCallback(enable_disable_widget, XmNcancelCallback, 
		Callback_EnableAudit, 0);
	XtAddCallback(enable_disable_widget, XmNhelpCallback, 
		HelpDisplayOpen, "audit,Enable");
	XtManageChild(enable_disable_widget);
}

void 
DisableAudit() 
{
	Cardinal    n;
	Arg         args[20];
	XmString    xmstring;
	
	if (DisableAuditValidate())
		return;

	XtSetSensitive(main_menubar, False);
	
	xmstring = XmStringCreate(msg[1], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNdialogType, XmDIALOG_QUESTION); n++;
	XtSetArg(args[n], XmNmessageString, xmstring); n++;
	XtSetArg(args[n], XmNborderWidth, 1); n++;
	XtSetArg(args[n], XmNautoUnmanage, FALSE); n++;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
 	XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_NONE); n++;
#endif
	enable_disable_widget = XmCreateMessageBox(mother_form,
			"Disable", args, n);
	XmStringFree(xmstring);
	
	XtAddCallback(enable_disable_widget, XmNokCallback, 
		Callback_DisableAudit,  1);
	XtAddCallback(enable_disable_widget, XmNcancelCallback, 
		Callback_DisableAudit,  0);
	XtAddCallback(enable_disable_widget, XmNhelpCallback, 
		HelpDisplayOpen, "audit,Disable");
	
	XtManageChild(enable_disable_widget);
} 

static void 
OKCallback(w, user_data, info)
	Widget w;
	char   *user_data;
	XmAnyCallbackStruct *info;
{
	XtSetSensitive(main_menubar, True);
	XtUnmanageChild(confirmation_widget);
	XtDestroyWidget(confirmation_widget);
}

static void 
Callback_EnableAudit(w, flag, info)
	Widget  w;
	long    flag;
	XmAnyCallbackStruct *info;
{
	Widget w1;
	int    n;
	Arg    args[20];
	XmString xmstring;
	
	if (! flag || ! EnableAuditDoIt()) {
		XtUnmanageChild(enable_disable_widget);
		XtDestroyWidget(enable_disable_widget);
		XtSetSensitive(main_menubar, True);
		return;
	}
	
	/* Destroy original query widget */
	XtUnmanageChild(enable_disable_widget);
	XtDestroyWidget(enable_disable_widget);
	
	/* Success!  Tell user that done! */
	n = 0;
	XtSetArg(args[n], XmNborderWidth,                       1); n++;
	XtSetArg(args[n], XmNdialogStyle,      XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNdialogType,        XmDIALOG_QUESTION); n++;
	XtSetArg(args[n], XmNtopAttachment,         XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,        XmATTACH_FORM); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
 	XtSetArg(args[n], XmNdefaultButtonType,       XmDIALOG_NONE); n++;
#endif
	confirmation_widget = XmCreateMessageBox(mother_form,
				"Confirm", args, n);
	XtAddCallback(confirmation_widget, XmNokCallback, 
				OKCallback, NULL);
				
	w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_OK_BUTTON);
	n = 0;
	xmstring = XmStringCreate(msg[4], charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	XtSetValues(w1, args, n);
	XmStringFree(xmstring);
			
	w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_CANCEL_BUTTON);
	XtUnmanageChild(w1);
	XtDestroyWidget(w1);
			
	w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_HELP_BUTTON);
	XtUnmanageChild(w1);
	XtDestroyWidget(w1);
		
	/* Load data into widget */
	xmstring = XmStringCreate(msg[2], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNmessageString, xmstring); n++;
	XtSetValues(confirmation_widget, args, n);
	XmStringFree(xmstring);

	XtManageChild(confirmation_widget);
	XtSetSensitive(maintenance_enable_push_widget, FALSE);
	XtSetSensitive(maintenance_disable_push_widget, TRUE);
	XtSetSensitive(statistics_push_widget, TRUE);
}

static void 
Callback_DisableAudit(w, flag, info)
	Widget  w;
	long    flag;
	XmAnyCallbackStruct *info;
{
	Widget w1;
	int    n;
	Arg    args[20];
	XmString xmstring;
	
	if (! flag || ! DisableAuditDoIt()) {
		XtUnmanageChild(enable_disable_widget);
		XtDestroyWidget(enable_disable_widget);
		XtSetSensitive(main_menubar, True);
		return;
	}
	
	/* Wipe out original query widget */
	XtUnmanageChild(enable_disable_widget);
	XtDestroyWidget(enable_disable_widget);
	
	/* Build widget to announce success */
	xmstring = XmStringCreate(msg[3], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNmessageString,                xmstring); n++;
	XtSetArg(args[n], XmNborderWidth,                         1); n++;
	XtSetArg(args[n], XmNdialogStyle,        XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNdialogType,          XmDIALOG_QUESTION); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,          XmATTACH_FORM); n++;
#ifndef OSF
	/* On OSF this causes a segmentation violation when the
	 * widget is realized */
 	XtSetArg(args[n], XmNdefaultButtonType,       XmDIALOG_NONE); n++;
#endif
	confirmation_widget = XmCreateMessageBox(mother_form, "Confirm", args, n);
	XmStringFree(xmstring);
	XtAddCallback(confirmation_widget, XmNokCallback, 
			OKCallback, NULL);
				
	w1 = XmMessageBoxGetChild(confirmation_widget,
			XmDIALOG_OK_BUTTON);
	n = 0;
	xmstring = XmStringCreate(msg[4], charset);
	if (! xmstring)
		MemoryError();
	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	XtSetValues(w1, args, n);
	XmStringFree(xmstring);
		
	w1 = XmMessageBoxGetChild(confirmation_widget,
			XmDIALOG_CANCEL_BUTTON);
	XtUnmanageChild(w1);
	XtDestroyWidget(w1);
			
	w1 = XmMessageBoxGetChild(confirmation_widget, XmDIALOG_HELP_BUTTON);
	XtUnmanageChild(w1);
	XtDestroyWidget(w1);

	XtManageChild(confirmation_widget);
	XtSetSensitive(maintenance_enable_push_widget, TRUE);
	XtSetSensitive(maintenance_disable_push_widget, FALSE);
	XtSetSensitive(statistics_push_widget, FALSE);
}
		

/* Disable audit by talking to the audit daemon - code taken from auditcmd.c */
int
DisableAuditDoIt()
{
	int audit_fd;
	int ret;

	/* Open the audit device */
	ret = OpenAuditDevice (&audit_fd);
	if (! ret) 
		return FALSE;

	/* Signal to disable auditing. Trap errors here */
	if (ioctl (audit_fd, AUDIOC_DISABLE, 0) == -1) {
		if (errno == EPERM)
				ErrorMessageOpen(0, msg, 143, NULL);
		else if (errno == EINVAL)
				ErrorMessageOpen(0, msg, 145, NULL);
		else
				ErrorMessageOpen(0, msg, 147, NULL);
		close (audit_fd);
		return FALSE;
	}
	close (audit_fd);
	return TRUE;
}

/* Enable audit by talking to the daemon - code lifted from auditcmd.c */
int
EnableAuditDoIt ()
{
	int parms_fd;
	int audit_fd;
	int ret;
	int newpid;
	int pid, wait_stat;
	struct audit_init	*init;
	uint	init_size;

	/* Open the audit device */
	ret = OpenAuditDevice (&audit_fd);
	if (! ret) 
		return FALSE;

	if ( (parms_fd = open (AUDIT_PARMSFILE, O_RDWR) ) == -1) {
			ErrorMessageOpen(0, msg, 149, NULL);
		close (audit_fd);
		return FALSE;
	}

	/* Read the audit initialization parameters from the parameter
	   file. Bump the session count to indicate a new audit session.
	   If the enable ioctl succeeds, re-write the new session value
	   to the parameter file since it is needed by the audit daemon.
	   Otherwise, do not write it since the session was not started
	   and the value may be re-used.				*/

	if(read(parms_fd,&init_size,sizeof(uint)) == -1) {
		ErrorMessageOpen(0, msg, 151, NULL);
		close (parms_fd);
		close (audit_fd);
		return FALSE;
	}

	init = (struct audit_init *) Malloc(init_size);
	if (! init)
		MemoryError();

	lseek(parms_fd,0L,0);	/* re-read from start of file */

	if(read(parms_fd,init,init_size) == -1) {
		ErrorMessageOpen(0, msg, 153, NULL);
		free(init);
		close (parms_fd);
		close (audit_fd);
		return FALSE;
	}

	init->session++;

#ifdef DEBUG
	printf("INIT Parameters\n");
	printf("Session-%d\n",init->session);
#endif

	/* Try enabling audit */
	if((ioctl(audit_fd,AUDIOC_ENABLE,init)) == -1) {
		switch(errno) {

		   case EEXIST:
				ErrorMessageOpen(0, msg, 155, NULL);
			break;

		   case EPERM:
				ErrorMessageOpen(0, msg, 157, NULL);
			break;

		   default:
				ErrorMessageOpen(0, msg, 159, NULL);
			break;

		}
		free(init);
		close (parms_fd);
		close (audit_fd);
		return FALSE;
	}

	/* Rewrite parameter file with the new session value */

	lseek(parms_fd,0L,0);	/* re-read from start of file */

	if(write(parms_fd,init,init_size) == -1) {
			ErrorMessageOpen(0, msg, 161, NULL);
		free(init);
		close (parms_fd);
		close (audit_fd);
		return FALSE;
	}

	free (init);
	close(parms_fd);
	close(audit_fd);

	/* Ought to close all other file descriptors here ... */

	pid = fork();
	switch(pid) {
	case    -1:
		ErrorMessageOpen(2404, msg, 171, NULL);
		/* AUDIT fork failure */
		audit_no_resource("Proccess", OT_PROCESS,
			"Cannot fork to run audit daemon to enable audit",
		ET_SYS_ADMIN);
		break;
			
	case    0:
		close (ConnectionNumber(XtDisplay(main_shell)));
		execl(AUDIT_DMNPATH, AUDIT_DMNPATH, "-m", NULL);
		/* Should never get here -- should not error message here */
		exit(0x7f);
			
	default:
		wait(&wait_stat);
		if (! (wait_stat & 0xFF)) {
			/* terminated due to exit() */
			ret = (wait_stat >> 8) & 0xFF; /* exit status */
			if (ret == 0x7f) {
			    ErrorMessageOpen(2406, msg, 173, NULL);
			    audit_no_resource(AUDIT_COMMAND, OT_REGULAR,
			        "Cannot execute audit daemon", ET_SYS_ADMIN);
			} 
			else if (ret) {
			    /* exit status non-zero */
			    ErrorMessageOpen(2408, msg, 175, NULL);
			    /* AUDIT failure to run AUDIT_COMMAND */
			    sa_audit_audit (ES_AUD_ERROR,
			        "Audit daemon terminated with error exit");
			}
			else { 
			    /* Successful */
			    return TRUE;
			}
		}
		else {
			/* signal received */
			ErrorMessageOpen(2412, msg, 177, NULL);
			/* AUDIT failure to run AUDIT_COMMAND */
			sa_audit_audit(ES_AUD_ERROR,
			    "Audit daemon terminated with signal exit");
		} 
		break;
	}
	return FALSE;
}

static int 
DisableAuditValidate()
{
	int     audit_enabled;

	audit_enabled = CheckAuditEnabled();
	if (audit_enabled) {
		if (audit_enabled > 0) 
			return 0;       /* audit enabled and daemon running */
		else {
			ErrorMessageOpen(2418, msg, 29, NULL);
			/* AUDIT that audit is enabled but there is no daemon */
			sa_audit_audit (ES_AUD_MODIFY,
				"Audit was enabled but there is no daemon running");
			return 0;
		}
	}    
	/* audit is not enabled */
	ErrorMessageOpen(2416, msg, 26, NULL);
	/* can't disable audit if audit is disabled! */
	return 1;
}

static int 
EnableAuditValidate()
{
	int     audit_enabled;

	audit_enabled = CheckAuditEnabled();
	if (audit_enabled) {
		if (audit_enabled > 0) {
			ErrorMessageOpen(2438, msg, 58, NULL);
			/* AUDIT attempt to enable audit that is already enabled */
			sa_audit_audit (ES_AUD_ERROR,
				"Attempt to audit when audit was already enabled");
			return 1;
		}
		else {
			/* audit is enabled but there is no daemon */
			ErrorMessageOpen(2440, msg, 61, NULL);
			/* AUDIT no daemon running but audit is enabled */
			sa_audit_audit (ES_AUD_ERROR,
				"Audit is enabled but there is no daemon running");
			return 1;
		}
	}    
	/* audit is not enabled */
	return 0;
}
		
int 
CheckAuditEnabled()
{
	int     fd;

	/* open the read device to see if a daemon is around */
	fd = open(AUDIT_RDEVICE, O_RDONLY);
	if (fd < 0 && errno == EEXIST)  /* daemon is running */
		return 1;
	if (fd < 0 && errno == EIO)   /* audit is not enabled */
		return 0;
	
	/* audit is enabled but there is no daemon */
	close(fd);
	return -1;
}

/* Open the audit device, returning the file descriptor */
int
OpenAuditDevice (audit_fd)
	int	*audit_fd;
{
	if ((*audit_fd = open (AUDIT_WDEVICE, O_WRONLY)) == -1) {
			ErrorMessageOpen(0, msg, 141, NULL);
		return FALSE;
	}
	return TRUE;
}

#endif /* SEC_BASE */

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
static char	*sccsid = "@(#)$RCSfile: XIsso.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:11:00 $";
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

#if SEC_BASE

/*
	filename:
	    XIsso.c
	
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
	    provide X user interface for role programs
	    
	notes:
	    This file is similar between isso, sysadmin and operator.

*/
	    
/* Common C include files */
#include <sys/types.h>
#include <sys/security.h>

/* Security includes */
#include <prot.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/MenuShell.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>

/* Include files */
#define MAIN
#include "XMain.h"
#include "XAccounts.h"
#include "XAudit.h"
#include "XDevices.h"

/* Definitions */
#define MESSAGE_FILE	"/usr/share/lib/sechelp/XIsso/isso.msg"

/* External routines */
extern void 
	InitializeMessageHandling(), 	/* Initializes message stuff */
#if SEC_WIND
	MakeProgramTrusted(),		/* "Trusted Path Active" routine */
#endif
	LoadMessage();			/* Loads message buffer */

extern void 
	StartAccountsFunctionalUnits(),
	StopAccountsFunctionalUnits(),
	StartAuditFunctionalUnits(),
	StopAuditFunctionalUnits(),
	StartDevicesFunctionalUnits(),
	StopDevicesFunctionalUnits();

extern void 
	FileMenuBarInitialize(),
	AccountsMenuBarInitialize(),
	AuditMenuBarInitialize(),
	DevicesMenuBarInitialize(),
	HelpMenuBarInitialize();

/* Local routines */
static void StartFunctionalUnits();
void StopFunctionalUnits();

/* Local variables */
static char 
	    **msg_error_1,
	    **msg_error_2,
	    **msg_main,
	    *msg_error_1_text,
	    *msg_error_2_text,
	    *msg_main_text;
	
main(argc, argv)
	int     argc;
	char    **argv;
{
	Cardinal    n;
	Arg         args[20];
	Widget      w;
	int         i,
		    ret,
	            shell_height,
	            shell_width;
	Display     *display;

	/* Initialize security */
	set_auth_parameters(argc, argv);
	initprivs();
	forcepriv(SEC_REMOTE); /* This is not really needed */
	forcepriv(SEC_LIMIT);
	forcepriv(SEC_OWNER);
	forcepriv(SEC_CHOWN);
	forcepriv(SEC_EXECSUID);
	forcepriv(SEC_SUSPEND_AUDIT);
	forcepriv(SEC_WRITE_AUDIT);
	forcepriv(SEC_CONFIG_AUDIT);
#if SEC_MAC
	forcepriv(SEC_ALLOWMACACCESS);
#endif /* SEC_MAC */
	forcepriv(SEC_ALLOWDACACCESS);
#if SEC_ILB
	forcepriv(SEC_ALLOWILBACCESS);
	forcepriv(SEC_ILNOFLOAT);
#endif /* SEC_ILB */

	/* For utilities say who we are */
	role_program = ISSO;

	/* Get this application's messages loaded and ready */        
	InitializeMessageHandling(MESSAGE_FILE);

	/* Note that InitializeMessageHandling() will trap many
	*  problems with permissions -- the message file will not be
	*  found!  Code in this function attempts to notify the
	*  user as to problem -- error message goes to stdout
	*/
	LoadMessage("msg_main", &msg_main, &msg_main_text);
	
	/* Crank up X windows early so bulk of startup error messages can be 
	*  routed through X 
	*/
	
	/* Create application shell */
	main_shell = XtInitialize(
	    "Main",                         /* name of toplevel shell widget */
	    "XIsso",                        /* application class */
	    NULL, NULL, &argc, argv);       /* command line parameters */

	display = XtDisplay(main_shell);
	XSync(display, FALSE);

	no_form_present = FALSE;	/* Used if error before form present */

	/* Look for special command line parameter of "-trace" & "-memory" */
	save_memory = FALSE;
	for (i = 0; i < argc; i++) {
	    if (! stricmp(argv[i], msg_main[0]))
	        save_memory = TRUE;
	}

	/* Set title for window manager */
	n = 0;
	XtSetArg(args[n], XmNmappedWhenManaged,               FALSE); n++;
	XtSetArg(args[n], XmNallowShellResize,                 TRUE); n++;
	XtSetArg(args[n], XmNtitle,                     msg_main[1]); n++;
	XtSetValues(main_shell, args, n);

	/**********************************************************************/
	/* Primary application form widget                                    */
	/**********************************************************************/
	n = 0;
	XtSetArg(args[n], XmNdefaultPosition,                   TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,             XmRESIZE_NONE); n++;
	XtSetArg(args[n], XmNautoUnmanage,                     FALSE); n++;
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNtopOffset,                            0); n++;
	XtSetArg(args[n], XmNleftOffset,                           0); n++;
	XtSetArg(args[n], XmNbottomOffset,                         0); n++;
	XtSetArg(args[n], XmNrightOffset,                          0); n++;
	XtSetArg(args[n], XmNverticalSpacing,                      0); n++;
	XtSetArg(args[n], XmNhorizontalSpacing,                    0); n++;
	main_form = XmCreateForm(main_shell, "Main", args, n);
	XtManageChild(main_form);

	/**********************************************************************/
	/* Menu Bar                                                           */
	/**********************************************************************/
	/* Create menu bar for application */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopOffset,                                 0); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset,                                0); n++;
	XtSetArg(args[n], XmNborderWidth,                               0); n++;
	XtSetArg(args[n], XmNshadowThickness,                           0); n++;
	main_menubar = XmCreateMenuBar(main_form, "Menubar", 
	                            args, n);
	XtManageChild(main_menubar);

	/**********************************************************************/
	/* Separator bar                                                      */
	/**********************************************************************/
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                      main_menubar); n++;
	XtSetArg(args[n], XmNtopOffset,                                 0); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset,                                0); n++;
	XtSetArg(args[n], XmNrightAttachment,               XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset,                               0); n++;
	XtSetArg(args[n], XmNmargin,                                    0); n++;
	XtSetArg(args[n], XmNorientation,                    XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNseparatorType,                 XmDOUBLE_LINE); n++;
	w = XmCreateSeparator(main_form, "Separator", args, n);
	XtManageChild(w);
	
	/**********************************************************************/
	/* Primary attachment (i.e. mother) widget for all functions          */
	/**********************************************************************/
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                             w); n++;
	XtSetArg(args[n], XmNtopOffset,                             0); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset,                            0); n++;
	XtSetArg(args[n], XmNrightAttachment,           XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset,                           0); n++;
	XtSetArg(args[n], XmNbottomAttachment,          XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomOffset,                          0); n++;
	XtSetArg(args[n], XmNrubberPositioning,                  True); n++;
	XtSetArg(args[n], XmNresizePolicy,              XmRESIZE_NONE); n++;
	XtSetArg(args[n], XmNautoUnmanage,                      FALSE); n++;
	XtSetArg(args[n], XmNborderWidth,                           0); n++;
	XtSetArg(args[n], XmNdialogStyle,          XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNverticalSpacing,                       0); n++;
	XtSetArg(args[n], XmNhorizontalSpacing,                     0); n++;
	mother_form = XmCreateForm(main_form, "Mother", args, n);
	XtManageChild(mother_form);

	/**********************************************************************/
	/* Initialize functional units                                        */
	/**********************************************************************/
	StartFunctionalUnits();
	XSync(display, FALSE);

	/**********************************************************************/
	/* Realize the widget (it is not mapped). Make it trusted then map it */
	/**********************************************************************/
	XtRealizeWidget(main_shell);
#if SEC_WIND
	XSync(display, FALSE);
	MakeProgramTrusted("isso");
	XSync(display, FALSE);
#endif
	XtMapWidget(main_shell);

	/**********************************************************************/
	/* Error checking                                                     */
	/**********************************************************************/
	if (! authorized_user("isso") ) {
		LoadMessage("msg_not_authorized", &msg_error_1, 
			&msg_error_1_text);
		SystemErrorMessageOpen(-1, msg_error_1, 0, NULL);
		XtMainLoop(); /* Dies */
	}
#ifdef OLD_CODE
    /* if you are running one of the setgid programs and your
     * effective user id is not equal to your real user id, you
     * need to be checked against the protected subsystems
     * database.
     */
    if (getgid() != getegid()) {
        grp = getgrgid(getegid());
        if (! authorized_user(grp->gr_name)) {
		LoadMessage("msg_not_authorized", &msg_error_1, 
			&msg_error_1_text);
            SystemErrorMessageOpen(1010, msg_error_1, 0, NULL);
            /* Dies */
	}
    }

/* 
    Check if the program is running as user "audit", group "audit" 
    and has SelfAudit privilege.  
    
    If all conditions pass, return 1, otherwise return 0.
    
    Note that this code must be run AFTER windows cranked up
*/

    /* check that program is running under proper permissions. */
    pwd = getpwnam("audit");
    grp = getgrnam("audit");
    if ( pwd == (struct passwd *) 0 ||
         geteuid() != pwd->pw_uid   ||
         grp == (struct group *) 0  ||
         getegid() != grp->gr_gid
    ) {
		LoadMessage("msg_not_authorized", &msg_error_1, 
			&msg_error_1_text);
            SystemErrorMessageOpen(1010, msg_error_1, 0, NULL);
        /* can't AUDIT incorrect user running the program */
        /* Dies */
    }
    /* check for audit audit privilege */
    getpriv(SEC_EFFECTIVE_PRIV, privmask);
    if (! ISBITSET(privmask, SEC_CONFIG_AUDIT)) {
		LoadMessage("msg_not_authorized", &msg_error_1, 
			&msg_error_1_text);
            SystemErrorMessageOpen(1010, msg_error_1, 0, NULL);
        /* Dies */
    }

#endif

#if SEC_MAC
	ret = mand_init();
	if (ret != 0) {
		LoadMessage("msg_cant_initialize_mand", &msg_error_2, 
			&msg_error_2_text);
		SystemErrorMessageOpen(-1, msg_error_2, 0, NULL);
		XtMainLoop(); /* Dies */
	}
#endif

	/**********************************************************************/
	/* Initialize the menu bar.                                           */
	/**********************************************************************/
	FileMenuBarInitialize();
	AuditMenuBarInitialize();
	AccountsMenuBarInitialize();
	DevicesMenuBarInitialize();
	HelpMenuBarInitialize("isso,XIsso.hlp");

	/* Normal start */
	XtMainLoop();
}

static void StartFunctionalUnits() 
{
	ConfirmationStart();
	ErrorMessageStart();
	StartAccountsFunctionalUnits();
	StartAuditFunctionalUnits();
	StartDevicesFunctionalUnits();
}

void StopFunctionalUnits() 
{
	ConfirmationStop();
	ErrorMessageStop();
	StopAccountsFunctionalUnits();
	StopAuditFunctionalUnits();
	StopDevicesFunctionalUnits();
}

#endif /* SecureWare */

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
static char	*sccsid = "@(#)$RCSfile: XUserAudit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:59 $";
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
		XUserAudit.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Audit parameters for user account
		
	entry points:
		UserAuditStart()
		UserAuditOpen()
		UserAuditClose()
		UserAuditStop()
*/

/* Common C include files */
#include <sys/types.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/ToggleBG.h>

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XAudit.h"
#include "XMacros.h"

extern int
	AuditGetMask (),
	AuditGetMaskStructure();

static char
	**msg_cant_read_audit, 
	**msg_audit,
	*msg_cant_read_audit_text,
	*msg_audit_text;

/* Definitions */
#define NUM_USER_AUDIT_VALUES		0
#define NUM_USER_AUDIT_TOGGLES		AUDIT_MAX_EVENT

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (UserAuditStart, UserAuditOpen, UserAuditClose,
	UserAuditStop)

static void 
MakeWidgets() 
{
	Widget      	work_area1_frame,
			work_area1_widget,
			work_area2_frame,
			work_area2_widget,
			ok_button,
			cancel_button,
			help_button,
			w,w1;
	int         	i;
	Dimension	max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "UserAudit");

	XtAddCallback(form_widget, XmNhelpCallback,
					HelpDisplayOpen, "accounts,UserAudit");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_user_audit", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* User name                                                          */
	/**********************************************************************/
	user_name_widget = CreateUserName (form_widget, w);

	/**********************************************************************/
	/* Create a label                                                     */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], 
		user_name_widget, True, user_name_widget);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	
	max_label_width = (Dimension) 0;

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	toggle_state     = MallocChar(NUM_USER_AUDIT_TOGGLES);
	def_toggle_state = MallocChar(NUM_USER_AUDIT_TOGGLES);
	yes_widget 	 = MallocWidget(NUM_USER_AUDIT_TOGGLES);
	default_widget   = MallocWidget(NUM_USER_AUDIT_TOGGLES);

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	if (! msg_audit)
		LoadMessage("msg_audit_events", &msg_audit, 
			&msg_audit_text);
	CreateItemsYND(work_area1_widget, 0, ROUND2(NUM_USER_AUDIT_TOGGLES),
		msg_audit, &max_label_width, yes_widget, default_widget);

	/**********************************************************************/
	/* Create the second form                                             */
	/**********************************************************************/
	work_area2_frame = CreateFrame(form_widget, w, False, work_area1_frame);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);
	XSync(XtDisplay(main_shell), FALSE);

	/* Attach the Set Dflt buttons to right of frame */
	w1 = CreateHeaderRight (form_widget, msg_header[2], user_name_widget, 
		False, work_area2_frame);

	max_label_width = (Dimension) 0;

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	CreateItemsYND(work_area2_widget, ROUND2(NUM_USER_AUDIT_TOGGLES), 
		NUM_USER_AUDIT_TOGGLES, msg_audit, &max_label_width, 
		yes_widget, default_widget);
	XSync(XtDisplay(main_shell), FALSE);

	/* Are the forms uneven */
	if (ROUND2(NUM_USER_AUDIT_TOGGLES) != (NUM_USER_AUDIT_TOGGLES / 2) )
		CreateHeader(work_area2_widget," ", 
			yes_widget[NUM_USER_AUDIT_TOGGLES-1], True,
			yes_widget[NUM_USER_AUDIT_TOGGLES-1]);

	/* Make the heading as big as largest label */
	w = CreateHeader (form_widget, msg_header[1], user_name_widget, 
		False, work_area1_frame);
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader (form_widget, msg_header[2], user_name_widget, 
		False, w);
	
	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_USER_AUDIT_TOGGLES; i++) {
		XtAddCallback (yes_widget[i], XmNvalueChangedCallback, 
				OnCallback, i);
		XtAddCallback (default_widget[i], XmNvalueChangedCallback, 
				DefaultCallback, i);
	}

	/**********************************************************************/
	/* Create the OK, Cancel and Help buttons			      */
	/**********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
				    &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
					HelpDisplayOpen, "accounts,UserAudit");
}

CREATE_ON_CALLBACKS (def_toggle_state, False, NUM_USER_AUDIT_TOGGLES)
CREATE_CALLBACKS (msg_header[3], UserAuditClose) 

static int
LoadVariables ()
{
	struct pr_passwd *prpwd = &pr.prpw;
	audmask_fillin  audit_fillin;
	int i;
	int ret;

	ret = XGetUserInfo(chosen_user_name, &pr);
	if (ret)
		return (ret);

	for (i=0;i<NUM_USER_AUDIT_TOGGLES;i++) {
		if (!prpwd->uflg.fg_auditcntl || 
			!ISBITSET(prpwd->ufld.fd_auditcntl, i + 1))
				toggle_state[i] = DEFAULT_CHAR;
		else
			if (ISBITSET (prpwd->ufld.fd_auditdisp, i + 1))
				toggle_state[i] = YES_CHAR;
			else	toggle_state[i] = NO_CHAR;
	}
	
	/* System audit toggles */
        /* Load audit mask structure */
        if (AuditGetMaskStructure(&audit_fillin)) {
		WorkingClose();
		no_form_present = True;
		if (! msg_cant_read_audit)
			LoadMessage ("msg_accounts_cant_read_audit",
			&msg_cant_read_audit, &msg_cant_read_audit_text);
		ErrorMessageOpen (-1, msg_cant_read_audit, 0, NULL);
            	return(FAILURE);
	}
    
        /* Load mask */
        if (AuditGetMask(&audit_fillin)) {
		WorkingClose();
		no_form_present = True;
		if (! msg_cant_read_audit)
			LoadMessage ("msg_accounts_cant_read_audit",
			&msg_cant_read_audit, &msg_cant_read_audit_text);
		ErrorMessageOpen (-1, msg_cant_read_audit, 0, NULL);
            	return(FAILURE);
	}

    	for (i = 0; i < NUM_USER_AUDIT_TOGGLES; i++) {
		def_toggle_state[i] = audit_fillin.mask[i][0];
	}    
    
	/* Load the auditword value defaults */
	for (i=0; i<NUM_USER_AUDIT_TOGGLES; i++) {
		SetYND(toggle_state[i], def_toggle_state[i],
			yes_widget[i], default_widget[i]);
	}

	/* Set the user name */
	SetUserName(user_name_widget, pr.prpw.ufld.fd_name);
	return (SUCCESS);
}


/*************************************************************************/
/* Check the input - no checking needed in this case                     */
/*************************************************************************/
static int
ValidateEntries ()
{
	return (SUCCESS);
}

static int
WriteInformation ()
{
	int i,j;
	struct pr_passwd *prpwd = &pr.prpw;

	/* For each toggle field we need to set the audit mask */
	/* Check to see if they are all default */
	j = 0;
	for (i =0; i<NUM_USER_AUDIT_TOGGLES; i++) {
		if (XmToggleButtonGadgetGetState(default_widget[i])) 
			j ++;
	}

	/* All defaults */
	if (j == NUM_USER_AUDIT_TOGGLES) {
		prpwd->uflg.fg_auditcntl = 0;
		prpwd->uflg.fg_auditdisp = 0;
	}
	else {
		prpwd->uflg.fg_auditcntl = 1;
		prpwd->uflg.fg_auditdisp = 1;
		for (i =0; i<NUM_USER_AUDIT_TOGGLES; i++) {
			if (XmToggleButtonGadgetGetState(default_widget[i])) {
				RMBIT (prpwd->ufld.fd_auditcntl, i+1);
				RMBIT (prpwd->ufld.fd_auditdisp, i+1);
			}
			else {
				ADDBIT  (prpwd->ufld.fd_auditcntl, i+1);
				if (XmToggleButtonGadgetGetState(yes_widget[i]))
					ADDBIT (prpwd->ufld.fd_auditdisp, i+1);
				else
					RMBIT (prpwd->ufld.fd_auditdisp, i+1);
			}
		}
	}
	/* Write information */
	return (XWriteUserInfo (&pr));
}

#endif /* SEC_BASE **/

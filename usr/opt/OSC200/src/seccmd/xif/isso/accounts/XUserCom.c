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
static char	*sccsid = "@(#)$RCSfile: XUserCom.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:09 $";
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
		XUserCom.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Command authorizations for user account
		(on pre SMP+3.0 systems these were called authorizations)
		
	entry points:
		UserComStart()
		UserComOpen()
		UserComClose()
		UserComStop()
*/

/* Common C include files */
#include <sys/types.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XMacros.h"

static char
	**msg_error,
	*msg_error_text,
	**msg_subsystems;

static int
	NUM_USER_SUB_TOGGLES;	/* Make it static */
extern char **alloc_table() ;
extern void SystemErrorMessageOpen();

/* Definitions */
#define NUM_USER_SUB_VALUES		0
#ifdef OLD_CODE
#define NUM_USER_SUB_TOGGLES		( total_auths() )
#endif

/* For the CMW we have to do this on the fly, as the number of
 * subsytems is very dynamic. Therefore we use dynamic structures.
 * This means that this screen is different from other screens
 */

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (UserComStart, UserComOpen, UserComClose,
	UserComStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
		    work_area1_widget,
		    work_area2_frame,
		    work_area2_widget,
		    work_area3_frame,
		    work_area3_widget,
		    ok_button,
		    cancel_button,
		    help_button,
		    w,w1;
	int         i;
	Dimension   max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "UserCom");
	/* Before SMP+ Version 3.0 these were called subsystems not
	 * command authorizations */
	XtAddCallback(form_widget, XmNhelpCallback,
				    HelpDisplayOpen, "accounts,UserCom");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage("msg_accounts_user_command", &msg_header, 
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
	
	max_label_width = GetWidth(w) + (Dimension) 25;
	/* Read the subsytem names in */
	build_cmd_priv ();
	NUM_USER_SUB_TOGGLES = total_auths();
	/* Can't read tables */
	if (NUM_USER_SUB_TOGGLES == -1) {
		if (! msg_error)
			LoadMessage("msg_accounts_subsystems_error", 
				&msg_error, &msg_error_text);
		SystemErrorMessageOpen(-1, msg_error, 0, NULL);
	}
	if (!(msg_subsystems = alloc_table (cmd_priv, NUM_USER_SUB_TOGGLES)))
		MemoryError ();

	for (i = 0; i< NUM_USER_SUB_TOGGLES; i++) {
		strcpy (msg_subsystems[i], cmd_priv[i].name);
	}

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	toggle_state     = MallocChar(NUM_USER_SUB_TOGGLES);
	def_toggle_state = MallocChar(NUM_USER_SUB_TOGGLES);
	yes_widget 	 = MallocWidget(NUM_USER_SUB_TOGGLES);
	default_widget   = MallocWidget(NUM_USER_SUB_TOGGLES);

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	CreateItemsYND(work_area1_widget, 0, ROUND13(NUM_USER_SUB_TOGGLES),
		msg_subsystems, &max_label_width, yes_widget, default_widget);

	XSync(XtDisplay(main_shell), FALSE);

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader (form_widget, msg_header[2], user_name_widget, 
			False, w);

	/**********************************************************************/
	/* Create the second form                                             */
	/**********************************************************************/
	work_area2_frame  =CreateFrame(form_widget, w, False, work_area1_frame);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);
	XSync(XtDisplay(main_shell), FALSE);

	/* Make the heading as big as largest label */
	w = CreateHeader(form_widget, msg_header[1], user_name_widget, 
		False, w1);

	max_label_width = GetWidth(w) + (Dimension) 25;

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	CreateItemsYND(work_area2_widget, ROUND13(NUM_USER_SUB_TOGGLES), 
		ROUND23(NUM_USER_SUB_TOGGLES),msg_subsystems, &max_label_width, 
		yes_widget, default_widget);
	XSync(XtDisplay(main_shell), FALSE);

	/* If necessary, add another item to line up */
	if ( (NUM_USER_SUB_TOGGLES % 3) == 1)
		CreateHeader(work_area2_widget," ",
			yes_widget[ROUND23(NUM_USER_SUB_TOGGLES)-1],
			True, yes_widget[ROUND23(NUM_USER_SUB_TOGGLES)-1]);
	
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader(form_widget, msg_header[2], user_name_widget, 
		False, w);

	/**********************************************************************/
	/* Create the third form                                              */
	/**********************************************************************/
	work_area3_frame  =CreateFrame(form_widget, w, False, work_area2_frame);
	work_area3_widget = CreateSecondaryForm(work_area3_frame);
	XSync(XtDisplay(main_shell), FALSE);

	w = CreateHeader(form_widget, msg_header[1], user_name_widget, 
		False, w1);

	max_label_width = GetWidth(w) + (Dimension) 25;

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	CreateItemsYND(work_area3_widget, ROUND23(NUM_USER_SUB_TOGGLES), 
		NUM_USER_SUB_TOGGLES,msg_subsystems, &max_label_width, 
		yes_widget, default_widget);
	XSync(XtDisplay(main_shell), FALSE);

	/* If necessary, add another item to line up */
	if ( (NUM_USER_SUB_TOGGLES % 3) != 0)
		CreateHeader(work_area3_widget," ",
			yes_widget[NUM_USER_SUB_TOGGLES-1],
			True, yes_widget[NUM_USER_SUB_TOGGLES-1]);
	
	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader(form_widget, msg_header[2], user_name_widget, 
		False, w);
	
	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_USER_SUB_TOGGLES; i++) {
		 XtAddCallback (yes_widget[i], XmNvalueChangedCallback, 
			OnCallback, i);
		 XtAddCallback (default_widget[i], XmNvalueChangedCallback, 
			DefaultCallback, i);
	}

	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons		 	     */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
			    &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				    HelpDisplayOpen, "accounts,UserCom");
}

CREATE_ON_CALLBACKS (def_toggle_state, True, NUM_USER_SUB_TOGGLES)
CREATE_CALLBACKS (msg_header[3], UserComClose)

static int
LoadVariables ()
{
	int i;
	int ret;
	struct pr_passwd *prpwd = &pr.prpw;

	ret = XGetUserInfo(chosen_user_name, &pr);
	if (ret)
		return (ret);

	/* Load the subsystems */
	if (!prpwd->uflg.fg_cprivs) {
		for (i=0;i<NUM_USER_SUB_TOGGLES;i++)
		    toggle_state[i] = DEFAULT_CHAR;
	}
	else {
		for (i=0;i<NUM_USER_SUB_TOGGLES;i++) {
		    if (ISBITSET (prpwd->ufld.fd_cprivs, cmd_priv[i].value))
		            toggle_state[i] = YES_CHAR;
		    else    toggle_state[i] = NO_CHAR;
		}
	}

	/* Load the default subsystems */
	for (i=0;i<NUM_USER_SUB_TOGGLES;i++) {
		if (ISBITSET (prpwd->sfld.fd_cprivs, cmd_priv[i].value))
		        def_toggle_state[i] = YES_CHAR;
		else    def_toggle_state[i] = NO_CHAR;
	}

	/* Load the subword value defaults */
	for (i=0; i<NUM_USER_SUB_TOGGLES; i++) {
		SetYND(toggle_state[i], def_toggle_state[i],
			yes_widget[i], default_widget[i]);
	}

	/* Set the user name */
	SetUserName(user_name_widget, pr.prpw.ufld.fd_name);
	return (ret);
}

static int
ValidateEntries ()
{
	return (SUCCESS);
}

static int
WriteInformation ()
{
	struct pr_passwd *prpwd = &pr.prpw;
	int i,j,k;

	/* Allocate space to store subsystem names */
	/* Loop through all cmd privs. If all default then set user = default */
	j = 0;
	for (i=0; i<NUM_USER_SUB_TOGGLES; i++) {
		if (XmToggleButtonGadgetGetState(default_widget[i])) 
		j ++;
	}

	/* All defaults */
	if (j == NUM_USER_SUB_TOGGLES) {
		prpwd->uflg.fg_cprivs = 0;
		strcpy (msg_subsystems[0], AUTH_DEFAULT);
		k = 1;
	}
	else {
		k = 0;
		prpwd->uflg.fg_cprivs = 1;

		for (i =0; i<NUM_USER_SUB_TOGGLES; i++) {
			if (XmToggleButtonGadgetGetState(yes_widget[i])) {
				ADDBIT (prpwd->ufld.fd_cprivs, cmd_priv[i].value);
			strcpy (msg_subsystems[k++], cmd_priv[i].name);
		}
			else
				RMBIT (prpwd->ufld.fd_cprivs, cmd_priv[i].value);
		}
	}

	/* Write information */
	j = XWriteUserInfo (&pr);
	/* For authorizations we must also write to authorizations file */
	if (j == SUCCESS)
		write_authorizations (prpwd->ufld.fd_name, msg_subsystems, k);
	return (j);
}

#endif /* SEC_BASE **/

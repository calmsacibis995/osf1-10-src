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
static char	*sccsid = "@(#)$RCSfile: XUserAuth.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:02 $";
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
		XUserAuth.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Kernel authorizations for user account
		
	entry points:
		UserAuthStart()
		UserAuthOpen()
		UserAuthClose()
		UserAuthStop()
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
#include "XMacros.h"

/* External routines */
char ** alloc_table();

/* Local variables */
static char
	**msg_auths;

#define	NUM_USER_AUTH_VALUES		0
#define NUM_USER_AUTH_TOGGLES		(SEC_MAX_SPRIV + 1)

static char
	*base_toggle_state,
	*def_base_toggle_state;

/* Kernel values are stored in variables declared in the 
 * CREATE_SCREEN_ROUTINES macro. Base privileges are stored 
 * separately in static storage */

static void
	BaseOnCallback(),
	BaseDefaultCallback ();

static Widget
	*base_yes_widget,
	*base_default_widget;

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (UserAuthStart, UserAuthOpen, UserAuthClose, 
			UserAuthStop)

static void 
MakeWidgets() 
{
	Widget		work_area1_frame,
			work_area1_widget,
			work_area2_frame,
			work_area2_widget,
			work_area3_frame,
			work_area3_widget,
			ok_button,
			cancel_button,
			help_button,
			k1,k2,k3,
			w,w1;
	int         	i;
	Dimension   	max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "UserAuth");
	XtAddCallback(form_widget, XmNhelpCallback,
			HelpDisplayOpen, "accounts,UserAuth");
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_user_authorizations_b1", 
			&msg_header, &msg_header_text);
#ifdef OLD_CODE
		LoadMessage ("msg_accounts_user_authorizations", 
			&msg_header, &msg_header_text);
#endif
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* User name                                                          */
	/**********************************************************************/
	user_name_widget = CreateUserName (form_widget, w);

	/**********************************************************************/
	/* Create two labels                                                  */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	k1=CreateHeader(form_widget, msg_header[1], user_name_widget,True,NULL);
	w =CreateHeader(form_widget, msg_header[2], user_name_widget,False, k1);
	k2=CreateHeader(form_widget, msg_header[1], user_name_widget,False, w);
	w =CreateHeader(form_widget, msg_header[2], user_name_widget,False, k2);
	k3=CreateHeader(form_widget, msg_header[1], user_name_widget,False, w);
	w =CreateHeader(form_widget, msg_header[2], user_name_widget,False, k3);
	XSync(XtDisplay(main_shell), FALSE);

	w = CreateHeader (form_widget, msg_header[3], k1, True, NULL);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	XSync(XtDisplay(main_shell), FALSE);
	
	max_label_width = (Dimension) 0;

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	toggle_state     	= MallocChar(NUM_USER_AUTH_TOGGLES);
	def_toggle_state 	= MallocChar(NUM_USER_AUTH_TOGGLES);
	yes_widget 	 	= MallocWidget(NUM_USER_AUTH_TOGGLES);
	default_widget   	= MallocWidget(NUM_USER_AUTH_TOGGLES);
	base_toggle_state     	= MallocChar(NUM_USER_AUTH_TOGGLES);
	def_base_toggle_state 	= MallocChar(NUM_USER_AUTH_TOGGLES);
	base_yes_widget 	= MallocWidget(NUM_USER_AUTH_TOGGLES);
	base_default_widget   	= MallocWidget(NUM_USER_AUTH_TOGGLES);

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	/* Read the authorization names in */
	if (!(msg_auths = alloc_table (sys_priv, NUM_USER_AUTH_TOGGLES)))
		MemoryError ();

	for (i = 0; i< NUM_USER_AUTH_TOGGLES; i++) {
		strcpy (msg_auths[i], sys_priv[i].name);
	}

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	CreateItemsYNDYND(work_area1_widget, 0, ROUND13(NUM_USER_AUTH_TOGGLES),
		msg_auths, &max_label_width, yes_widget, default_widget,
		base_yes_widget, base_default_widget);
#ifdef OLD_CODE
	CreateItemsYND(work_area1_widget, 0, ROUND13(NUM_USER_AUTH_TOGGLES),
		msg_auths, &max_label_width, yes_widget, default_widget);
#endif

	XSync(XtDisplay(main_shell), FALSE);
	/* Make the heading as big as largest label */
	SetWidgetWidth (k1, max_label_width);
	SetWidgetWidth (k2, max_label_width);
	SetWidgetWidth (k3, max_label_width);
	SetWidgetWidth (w, max_label_width);
	XSync(XtDisplay(main_shell), FALSE);
	w1 = CreateHeader (form_widget, msg_header[4], k1, False, w);

	/**********************************************************************/
	/* Create the second form                                             */
	/**********************************************************************/
	work_area2_frame  = CreateFrame(form_widget, w, False, work_area1_frame);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);
	XSync(XtDisplay(main_shell), FALSE);

	max_label_width = (Dimension) 0;
	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	CreateItemsYNDYND(work_area2_widget, ROUND13(NUM_USER_AUTH_TOGGLES), 
		ROUND23(NUM_USER_AUTH_TOGGLES),
		msg_auths, &max_label_width, yes_widget, default_widget,
		base_yes_widget, base_default_widget);
#ifdef OLD_CODE
	CreateItemsYND(work_area2_widget, ROUND13(NUM_USER_AUTH_TOGGLES), 
		ROUND23(NUM_USER_AUTH_TOGGLES),
		msg_auths, &max_label_width, yes_widget, default_widget);
#endif
	XSync(XtDisplay(main_shell), FALSE);

	/* Do we have to add a value ? */
	if ( (NUM_USER_AUTH_TOGGLES % 3) == 1)
		CreateHeader(work_area2_widget, " ", 
			yes_widget[ROUND23(NUM_USER_AUTH_TOGGLES) - 1],
			True, yes_widget[ROUND23(NUM_USER_AUTH_TOGGLES) - 1]);

	XSync(XtDisplay(main_shell), FALSE);

	/* Make the heading as big as largest label */
	XSync(XtDisplay(main_shell), FALSE);
	w = CreateHeader (form_widget, msg_header[3], k1, False, w1);
	/* This causes a segmentation violation. It is a bug in Motif */
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader (form_widget, msg_header[4], k1, False, w);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Create the third form                                              */
	/**********************************************************************/
	work_area3_frame  = CreateFrame(form_widget, w, False, work_area2_frame);
	work_area3_widget = CreateSecondaryForm(work_area3_frame);
	XSync(XtDisplay(main_shell), FALSE);

	max_label_width = (Dimension) 0;

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	CreateItemsYNDYND(work_area3_widget, ROUND23(NUM_USER_AUTH_TOGGLES), 
		NUM_USER_AUTH_TOGGLES,
		msg_auths, &max_label_width, yes_widget, default_widget,
		base_yes_widget, base_default_widget);
#ifdef OLD_CODE
	CreateItemsYND(work_area3_widget, ROUND23(NUM_USER_AUTH_TOGGLES), 
		NUM_USER_AUTH_TOGGLES,
		msg_auths, &max_label_width, yes_widget, default_widget);
#endif
	XSync(XtDisplay(main_shell), FALSE);
	free(msg_auths);

	/* Do we have to add a value ? */
	if ( (NUM_USER_AUTH_TOGGLES % 3) != 0)
	CreateHeader(work_area3_widget, " ", 
		yes_widget[NUM_USER_AUTH_TOGGLES - 1],
		True, yes_widget[NUM_USER_AUTH_TOGGLES - 1]);

	/* Make the heading as big as largest label */
	XSync(XtDisplay(main_shell), FALSE);
	w = CreateHeader (form_widget, msg_header[3], k1, False, w1);
	XSync(XtDisplay(main_shell), FALSE);
	SetWidgetWidth (w, max_label_width);
	w1 = CreateHeader (form_widget, msg_header[4], k1, False, w);
	XSync(XtDisplay(main_shell), FALSE);
	
	/**********************************************************************/
	/* Create callbacks                                                   */
	/**********************************************************************/
	for (i=0; i<NUM_USER_AUTH_TOGGLES; i++) {
		XtAddCallback (yes_widget[i], XmNvalueChangedCallback, 
				OnCallback, i);
		XtAddCallback (default_widget[i], XmNvalueChangedCallback, 
				DefaultCallback, i);
		XtAddCallback (base_yes_widget[i], XmNvalueChangedCallback, 
				BaseOnCallback, i);
		XtAddCallback (base_default_widget[i], XmNvalueChangedCallback, 
				BaseDefaultCallback, i);
	}

	/*********************************************************************/
	/* RowColumn widget for action buttons                               */
	/*********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	CreateThreeButtons (form_widget, work_area2_frame,
				 &ok_button, &cancel_button, &help_button);
	XSync(XtDisplay(main_shell), FALSE);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
			HelpDisplayOpen, "accounts,UserAuth");
	XSync(XtDisplay(main_shell), FALSE);
}

static void 
BaseOnCallback(w, i, info) 
	Widget                  w; 
	int                     i;
	XmToggleButtonCallbackStruct    *info;
{
	int j;

	/* Clear all default toggles */
	for (j=0; j<NUM_USER_AUTH_TOGGLES; j++)
 	SetToggle(base_default_widget[j], False);

	/* If the same kernel auth is off then turn it on */
	if (info->set) {
		if (! XmToggleButtonGadgetGetState(yes_widget[i]) ) {
			SetToggle (yes_widget[i], True);
			/* Must clear all the default buttons also */
			for (j=0; j<NUM_USER_AUTH_TOGGLES; j++)
				SetToggle(default_widget[j], False);
		}
	}
}

static void 
BaseDefaultCallback(w, i, info) 
	Widget                  w; 
	int                     i;
	XmToggleButtonCallbackStruct    *info;
{
	int	j;

	/* Set the current user value to system default */
	SetToggle(base_yes_widget[i], (def_base_toggle_state[i] == YES_CHAR) );

	/* If the same kernel auth is off then turn it on */
	if (info->set && (def_base_toggle_state[i] == YES_CHAR) ) {
		if (! XmToggleButtonGadgetGetState(yes_widget[i]) ) {
			SetToggle (yes_widget[i], True);
			/* Must clear all the default buttons also */
			for (j=0; j<NUM_USER_AUTH_TOGGLES; j++)
				SetToggle(default_widget[j], False);
		}
	}
}

CREATE_CALLBACKS(msg_header[5], UserAuthClose)

static void
OnCallback(w, i, info)
	Widget                  w;
	int                     i;
	XmToggleButtonCallbackStruct    *info;
{
	int j;

	/* Clear the default toggle */
	for (j=0; j<NUM_USER_AUTH_TOGGLES; j++)
		SetToggle(default_widget[j], False);
	
	/* If turning the button off then we must turn the base button off */
	if (! info->set) {
		if (XmToggleButtonGadgetGetState(base_yes_widget[i]) ) {
			SetToggle (base_yes_widget[i], False);
			/* Must clear all the default buttons also */
			for (j=0; j<NUM_USER_AUTH_TOGGLES; j++)
				SetToggle(base_default_widget[j], False);
		}
	}
}

static void
DefaultCallback(w, i, info)
	Widget                  w;
	int                     i;
	XmToggleButtonCallbackStruct    *info;
{
	int j;

	/* Set the current user value to system default */
	SetToggle(yes_widget[i], (def_toggle_state [i] == YES_CHAR) );

	/* If removing the default, and the default is no */
	if (info->set && (def_toggle_state[i] == NO_CHAR) ) {
		if (XmToggleButtonGadgetGetState(base_yes_widget[i]) ) {
			SetToggle (base_yes_widget[i], False);
			/* Must clear all the default buttons also */
			for (j=0; j<NUM_USER_AUTH_TOGGLES; j++)
				SetToggle(base_default_widget[j], False);
		}
	}
}


static int
LoadVariables ()
{
	int i;
	int ret;
    	struct pr_passwd *prpwd = &pr.prpw;

	/* Load the kernelword value defaults */
	ret = XGetUserInfo(chosen_user_name, &pr);
	if (ret)
		return (ret);

	/* Load the kernel authorizations */
	if (!prpwd->uflg.fg_sprivs) {
		for (i=0;i<NUM_USER_AUTH_TOGGLES;i++)
			toggle_state[i] = DEFAULT_CHAR;
	}
	else {
		for (i=0;i<NUM_USER_AUTH_TOGGLES;i++) {
			if (ISBITSET (prpwd->ufld.fd_sprivs, i))
			    	toggle_state[i] = YES_CHAR;
			else    toggle_state[i] = NO_CHAR;
		}
	}

	/* Load the default kernel authorizations */
	for (i=0;i<NUM_USER_AUTH_TOGGLES;i++) {
		if (ISBITSET (prpwd->sfld.fd_sprivs, i))
			def_toggle_state[i] = YES_CHAR;
		else    def_toggle_state[i] = NO_CHAR;
	}

	/* Load the base authorizations */
	if (!prpwd->uflg.fg_bprivs) {
		for (i=0;i<NUM_USER_AUTH_TOGGLES;i++)
			base_toggle_state[i] = DEFAULT_CHAR;
	}
	else {
		for (i=0;i<NUM_USER_AUTH_TOGGLES;i++) {
			if (ISBITSET (prpwd->ufld.fd_bprivs, i))
			    	base_toggle_state[i] = YES_CHAR;
			else    base_toggle_state[i] = NO_CHAR;
		}
	}

	/* Load the default base authorizations */
	for (i=0;i<NUM_USER_AUTH_TOGGLES;i++) {
		if (ISBITSET (prpwd->sfld.fd_bprivs, i))
			def_base_toggle_state[i] = YES_CHAR;
		else    def_base_toggle_state[i] = NO_CHAR;
	}
	
	for (i=0; i<NUM_USER_AUTH_TOGGLES; i++) {
		SetYND(toggle_state[i], def_toggle_state[i],
			yes_widget[i], default_widget[i]);
		SetYND(base_toggle_state[i], def_base_toggle_state[i],
			base_yes_widget[i], base_default_widget[i]);
	}
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
	int i,j;

	/* Loop through all sys privs. If all default then set default */
	j = 0;
	for (i=0; i<NUM_USER_AUTH_TOGGLES; i++) {
		if (XmToggleButtonGadgetGetState(default_widget[i])) 
			j ++;
	}

	/* All defaults */
	if (j == NUM_USER_AUTH_TOGGLES) 
		prpwd->uflg.fg_sprivs = 0;
	else {
		prpwd->uflg.fg_sprivs = 1;
		for (i =0; i<NUM_USER_AUTH_TOGGLES; i++) {
			if (XmToggleButtonGadgetGetState(yes_widget[i]))
				ADDBIT (prpwd->ufld.fd_sprivs, sys_priv[i].value);
			else
				RMBIT (prpwd->ufld.fd_sprivs, sys_priv[i].value);
		}
	}

	/* All defaults */
	/* Loop through all base privs. If all default then set default */
	j = 0;
	for (i=0; i<NUM_USER_AUTH_TOGGLES; i++) {
		if (XmToggleButtonGadgetGetState(base_default_widget[i])) 
		   j ++;
	}

	if (j == NUM_USER_AUTH_TOGGLES) 
		prpwd->uflg.fg_bprivs = 0;
	else {
		prpwd->uflg.fg_bprivs = 1;
		for (i =0; i<NUM_USER_AUTH_TOGGLES; i++) {
			if (XmToggleButtonGadgetGetState(base_yes_widget[i]))
				ADDBIT (prpwd->ufld.fd_bprivs, sys_priv[i].value);
			else
				RMBIT (prpwd->ufld.fd_bprivs, sys_priv[i].value);
		}
	}
	/* Write information */
	return (XWriteUserInfo (&pr));
}

#endif /* SEC_BASE **/

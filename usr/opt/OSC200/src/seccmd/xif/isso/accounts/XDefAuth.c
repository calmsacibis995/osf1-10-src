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
static char	*sccsid = "@(#)$RCSfile: XDefAuth.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:04 $";
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
		XDefAuth.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Default Kernel Authorizations
		
	entry points:
		DefAuthStart()
		DefAuthOpen()
		DefAuthClose()
		DefAuthStop()

	This file uses the standard code.
	yes_widget --> kernel authorizations
	default_widget --> base authorizations
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
char **
	alloc_table();

/* Local variables */
static char
	**msg_auths;

#define NUM_SYSTEM_AUTH_VALUES		0
#define NUM_SYSTEM_AUTH_TOGGLES		(SEC_MAX_SPRIV + 1)

static void
	OnCallback(),
	BaseCallback();

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DefAuthStart, DefAuthOpen, DefAuthClose, 
			DefAuthStop)

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
		    title,
		    w,w1;
	int         i;
	Dimension   max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DefAuth");
	XtAddCallback(form_widget, XmNhelpCallback,
				    HelpDisplayOpen, "accounts,DefAuth");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_default_authorizations", 
			&msg_header, &msg_header_text);
	title = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Create two labels                                                  */
	/**********************************************************************/
	w  = CreateHeader (form_widget, msg_header[1], title, True, NULL);
	w1 = CreateHeader (form_widget, msg_header[2], title, False, w);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);

	/**********************************************************************/
	/* Make the authorizations wide enough                                */
	/**********************************************************************/
	max_label_width = GetWidth(w) + (Dimension) 20;
	
	if (max_label_width < (Dimension) 120) 
		max_label_width = (Dimension) 120;
	
	/**********************************************************************/
	/* Read the authorization names in 			              */
	/**********************************************************************/
	if (!(msg_auths = alloc_table (sys_priv, NUM_SYSTEM_AUTH_TOGGLES)))
		MemoryError ();

	for (i = 0; i< NUM_SYSTEM_AUTH_TOGGLES; i++) {
		strcpy (msg_auths[i], sys_priv[i].name);
	}

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	toggle_state     = MallocChar(NUM_SYSTEM_AUTH_TOGGLES);
	def_toggle_state = MallocChar(NUM_SYSTEM_AUTH_TOGGLES);
	yes_widget 	 = MallocWidget(NUM_SYSTEM_AUTH_TOGGLES);
	default_widget   = MallocWidget(NUM_SYSTEM_AUTH_TOGGLES);

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	CreateItemsYND(work_area1_widget, 0, 
			ROUND13(NUM_SYSTEM_AUTH_TOGGLES), msg_auths, 
			&max_label_width, yes_widget, default_widget);
#ifdef OLD_CODE
	CreateItemsYN(work_area1_widget, 0, 
			ROUND13(NUM_SYSTEM_AUTH_TOGGLES), msg_auths, 
			&max_label_width, yes_widget);
#endif

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);

	/**********************************************************************/
	/* Create the second form                                             */
	/**********************************************************************/
	w  = CreateHeader (form_widget, msg_header[1], title, False, 
		work_area1_frame);
	w1 = CreateHeader (form_widget, msg_header[2], title, False, w);
	work_area2_frame = CreateFrame(form_widget, w, False, work_area1_frame);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);
	XSync(XtDisplay(main_shell), FALSE);

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	CreateItemsYND(work_area2_widget, ROUND13(NUM_SYSTEM_AUTH_TOGGLES), 
		ROUND23(NUM_SYSTEM_AUTH_TOGGLES), msg_auths, 
		&max_label_width, yes_widget, default_widget);
#ifdef OLD_CODE
	CreateItemsYN(work_area2_widget, ROUND13(NUM_SYSTEM_AUTH_TOGGLES), 
		ROUND23(NUM_SYSTEM_AUTH_TOGGLES), msg_auths, 
		&max_label_width, yes_widget);
#endif

	/* Do we have to add a value ? */
	if ( (NUM_SYSTEM_AUTH_TOGGLES % 3) == 1)
	CreateHeader(work_area2_widget, " ", 
		yes_widget[ROUND23(NUM_SYSTEM_AUTH_TOGGLES) - 1],
		True, yes_widget[ROUND23(NUM_SYSTEM_AUTH_TOGGLES) - 1]);

	XSync(XtDisplay(main_shell), FALSE);

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	
	/**********************************************************************/
	/* Create the third form                                              */
	/**********************************************************************/
	w  = CreateHeader (form_widget, msg_header[1], title, False, 
		work_area2_frame);
	w1 = CreateHeader (form_widget, msg_header[2], title, False, w);
	work_area3_frame = CreateFrame(form_widget, w, False, work_area2_frame);
	work_area3_widget = CreateSecondaryForm(work_area3_frame);
	XSync(XtDisplay(main_shell), FALSE);

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	CreateItemsYND(work_area3_widget, ROUND23(NUM_SYSTEM_AUTH_TOGGLES), 
		NUM_SYSTEM_AUTH_TOGGLES, msg_auths, 
		&max_label_width, yes_widget, default_widget);
#ifdef OLD_CODE
	CreateItemsYN(work_area3_widget, ROUND23(NUM_SYSTEM_AUTH_TOGGLES), 
		NUM_SYSTEM_AUTH_TOGGLES, msg_auths, 
		&max_label_width, yes_widget);
#endif
	XSync(XtDisplay(main_shell), FALSE);
	free(msg_auths);

	/* Do we have to add a value ? */
	if ( (NUM_SYSTEM_AUTH_TOGGLES % 3) != 0)
		CreateHeader(work_area3_widget, " ", 
			yes_widget[NUM_SYSTEM_AUTH_TOGGLES - 1],
			True, yes_widget[NUM_SYSTEM_AUTH_TOGGLES - 1]);

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	
#if SEC_PRIV
	/* Create callbacks for the kernel and base privileges buttons */
	for (i=0; i<NUM_SYSTEM_AUTH_TOGGLES; i++) {
		XtAddCallback (yes_widget[i], XmNvalueChangedCallback,
			OnCallback, i);
		XtAddCallback (default_widget[i], XmNvalueChangedCallback,
			BaseCallback, i);
	}
#endif
	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons			     */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
			        &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback, 
				    HelpDisplayOpen, "accounts,DefAuth");
}

CREATE_CALLBACKS(msg_header[3], DefAuthClose)

static int
LoadVariables ()
{
	int i;
	int ret;
	struct pr_default *df = &sd.df;

	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);

	/* Load the default kernel authorizations */
	if (df->prg.fg_sprivs) {
		for (i=0;i<NUM_SYSTEM_AUTH_TOGGLES;i++) {
		    if (ISBITSET (df->prd.fd_sprivs, i))
		        	toggle_state[i] = YES_CHAR;
		    else    toggle_state[i] = NO_CHAR;
		}
	}
	else {
		for (i=0;i<NUM_SYSTEM_AUTH_TOGGLES;i++)
		    toggle_state[i] = NO_CHAR;
	}

	/* Load the default base authorizations */
	if (df->prg.fg_bprivs) {
		for (i=0;i<NUM_SYSTEM_AUTH_TOGGLES;i++) {
		    if (ISBITSET (df->prd.fd_bprivs, i))
		        	def_toggle_state[i] = YES_CHAR;
		    else    def_toggle_state[i] = NO_CHAR;
		}
	}
	else {
		for (i=0;i<NUM_SYSTEM_AUTH_TOGGLES;i++)
		    def_toggle_state[i] = NO_CHAR;
	}

	/* Load the subword value defaults */
	for (i=0; i<NUM_SYSTEM_AUTH_TOGGLES; i++) {
		SetToggle(yes_widget[i],   (toggle_state[i] == YES_CHAR));
		SetToggle(default_widget[i], (def_toggle_state[i] == YES_CHAR));
	}

	return (ret);
}

static int
ValidateEntries ()
{
	return (SUCCESS);
}

/* If kernel privilege turned off then make sure the base is turned off also */
static void
OnCallback (w, i, ct)
	Widget	w;
	int	i;
	XmToggleButtonCallbackStruct	*ct;
{
	if (! ct->set)
		SetToggle (default_widget[i], False);
}

/* If base privilege selected make sure turn the kernel priv on also */
static void
BaseCallback (w, i, ct)
	Widget	w;
	int	i;
	XmToggleButtonCallbackStruct	*ct;
{
	if (ct->set)
		SetToggle (yes_widget[i], True);
}

static int
WriteInformation ()
{
	struct pr_default *sdef = &sd.df;
	int i;

	/* System privileges first */
	sdef->prg.fg_sprivs = 1;
	for (i =0; i<NUM_SYSTEM_AUTH_TOGGLES; i++) {
		if (XmToggleButtonGadgetGetState(yes_widget[i]))
			ADDBIT (sdef->prd.fd_sprivs, sys_priv[i].value);
		else
			RMBIT (sdef->prd.fd_sprivs, sys_priv[i].value);
	}

	/* Now the base privileges */
	sdef->prg.fg_bprivs = 1;
	for (i =0; i<NUM_SYSTEM_AUTH_TOGGLES; i++) {
		if (XmToggleButtonGadgetGetState(default_widget[i]))
			ADDBIT (sdef->prd.fd_bprivs, sys_priv[i].value);
		else
			RMBIT (sdef->prd.fd_bprivs, sys_priv[i].value);
	}

	/* Write information */
	return (XWriteSystemInfo (&sd));
}
#endif /* SEC_BASE */

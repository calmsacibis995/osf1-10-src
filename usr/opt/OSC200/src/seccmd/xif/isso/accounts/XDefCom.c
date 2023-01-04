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
static char	*sccsid = "@(#)$RCSfile: XDefCom.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:12 $";
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
		XDefCom.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Default Command Authorizations
		
	entry points:
		DefComStart()
		DefComOpen()
		DefComClose()
		DefComStop()
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

/* External routines */
extern void SystemErrorMessageOpen();
extern char **alloc_table() ;

/* Local variables */
static char 
	**msg_error,
	*msg_error_text,
	**msg_subsystems;

	/* The next variable is in caps for consistency with the other
	 * screen files */
static int
	NUM_SYSTEM_SUB_TOGGLES;

/* Definitions */
#define NUM_SYSTEM_SUB_VALUES		0

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DefComStart, DefComOpen, DefComClose, DefComStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
		    work_area1_widget,
		    work_area2_frame,
		    work_area2_widget,
		    work_area3_frame,
		    work_area3_widget,
		    title,
		    ok_button,
		    cancel_button,
		    help_button,
		    w,w1;
	int         i;
	Dimension   max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DefCom");
	XtAddCallback(form_widget, XmNhelpCallback,
				    HelpDisplayOpen, "accounts,DefCom");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_default_command", &msg_header, 
			&msg_header_text);
	title = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], title, True, title);
	w1 = CreateHeader (form_widget, msg_header[2], title, False, w);
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);
	work_area1_widget = CreateSecondaryForm(work_area1_frame);
	
	/* Read the subsytem names in */
	build_cmd_priv ();
	NUM_SYSTEM_SUB_TOGGLES = total_auths();
	/* Can't read tables */
	if (NUM_SYSTEM_SUB_TOGGLES == -1) {
		if (! msg_error)
			LoadMessage("msg_accounts_subsystems_error", 
				&msg_error, &msg_error_text);
		SystemErrorMessageOpen(-1, msg_error, 0, NULL);
	}
	if (!(msg_subsystems = alloc_table (cmd_priv, NUM_SYSTEM_SUB_TOGGLES)))
		MemoryError ();

	/**********************************************************************/
	/* Create space for the widgets and toggles                           */
	/**********************************************************************/
	toggle_state     = MallocChar(NUM_SYSTEM_SUB_TOGGLES);
	yes_widget 	 = MallocWidget(NUM_SYSTEM_SUB_TOGGLES);

	for (i = 0; i< NUM_SYSTEM_SUB_TOGGLES; i++) {
		strcpy (msg_subsystems[i], cmd_priv[i].name);
	}

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	max_label_width = GetWidth(w) + (Dimension) 25;
	CreateItemsYN(work_area1_widget, 0, ROUND13(NUM_SYSTEM_SUB_TOGGLES),
		msg_subsystems, &max_label_width, yes_widget);

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);

	/**********************************************************************/
	/* Create the second form                                             */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], title, False, 
		work_area1_frame);
	w1 = CreateHeader (form_widget, msg_header[2], title, False, w);
	XSync(XtDisplay(main_shell), FALSE);
	work_area2_frame = CreateFrame(form_widget, w, False, work_area1_frame);
	work_area2_widget = CreateSecondaryForm(work_area2_frame);

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	max_label_width = GetWidth(w) + (Dimension) 25;
	CreateItemsYN(work_area2_widget, ROUND13(NUM_SYSTEM_SUB_TOGGLES), 
		ROUND23(NUM_SYSTEM_SUB_TOGGLES), msg_subsystems, 
		&max_label_width, yes_widget);

	/* If necessary, add another item to line up */
	if ( (NUM_SYSTEM_SUB_TOGGLES % 3) == 1)
		CreateHeader(work_area2_widget," ",
			yes_widget[ROUND23(NUM_SYSTEM_SUB_TOGGLES)-1],
			True, yes_widget[ROUND23(NUM_SYSTEM_SUB_TOGGLES)-1]);
	

	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);

	/**********************************************************************/
	/* Create the third  form                                             */
	/**********************************************************************/
	w = CreateHeader (form_widget, msg_header[1], title, False, 
		work_area2_frame);
	w1 = CreateHeader (form_widget, msg_header[2], title, False, w);
	XSync(XtDisplay(main_shell), FALSE);
	work_area3_frame = CreateFrame(form_widget, w, False, work_area2_frame);
	work_area3_widget = CreateSecondaryForm(work_area3_frame);

	/**********************************************************************/
	/* For each item create the label then the Y/N/D toggles.             */
	/**********************************************************************/
	XSync(XtDisplay(main_shell), FALSE);
	max_label_width = GetWidth(w) + (Dimension) 25;
	CreateItemsYN(work_area3_widget, ROUND23(NUM_SYSTEM_SUB_TOGGLES), 
		NUM_SYSTEM_SUB_TOGGLES, msg_subsystems, &max_label_width, 
		yes_widget);

	/* If necessary, add another item to line up */
	if ( (NUM_SYSTEM_SUB_TOGGLES % 3) != 0)
		CreateHeader(work_area3_widget," ",
			yes_widget[NUM_SYSTEM_SUB_TOGGLES-1],
			True, yes_widget[NUM_SYSTEM_SUB_TOGGLES-1]);
	
	/* Make the heading as big as largest label */
	SetWidgetWidth (w, max_label_width);
	
	/*********************************************************************/
	/* Create the OK, Cancel and Help buttons			     */
	/*********************************************************************/
	CreateThreeButtons (form_widget, work_area2_frame,
				        &ok_button, &cancel_button, &help_button);
	XSync(XtDisplay(main_shell), FALSE);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
				    HelpDisplayOpen, "accounts,DefCom");
}

CREATE_CALLBACKS (msg_header[3], DefComClose)

static int
LoadVariables ()
{
	int i;
	int ret;
	struct pr_default *df = &sd.df;

	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);

	/* Load the default subsystems */
	if (df->prg.fg_cprivs) {
		for (i=0;i<NUM_SYSTEM_SUB_TOGGLES;i++) {
			if (ISBITSET (df->prd.fd_cprivs, cmd_priv[i].value)) {
			        toggle_state[i] = YES_CHAR;
			}
			else    {
				toggle_state[i] = NO_CHAR;
			}
		}
	}
	else {
		for (i=0;i<NUM_SYSTEM_SUB_TOGGLES;i++)
			toggle_state[i] = NO_CHAR;
	}

	/* Load the subword value defaults */
	for (i=0; i<NUM_SYSTEM_SUB_TOGGLES; i++) {
		SetToggle(yes_widget[i], (toggle_state[i] == YES_CHAR));
	}

	return (SUCCESS);
}

static int
ValidateEntries ()
{
	return (SUCCESS);
}

static int
WriteInformation ()
{
	struct pr_default *sdef = &sd.df;
	char **table;
	int i,k,ret;

	k = NUM_SYSTEM_SUB_TOGGLES;
	if (!(table = alloc_table (cmd_priv, k)))
		MemoryError();

	sdef->prg.fg_cprivs = 1;
	k = 0;

	for (i =0; i<NUM_SYSTEM_SUB_TOGGLES; i++) {
		if (XmToggleButtonGadgetGetState(yes_widget[i])) {
			ADDBIT (sdef->prd.fd_cprivs, cmd_priv[i].value);
			strcpy (table[k++], cmd_priv[i].name);
		}
		else
			RMBIT (sdef->prd.fd_cprivs, cmd_priv[i].value);
	}

	/* Write information */
	ret = XWriteSystemInfo (&sd);
	/* For authorizations we must also write to authorizations file */
	if (ret == SUCCESS)
		ret = write_authorizations ((char *) 0, table, k);
	free (table);
	return (ret);
}

#endif /* SEC_BASE **/

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
static char	*sccsid = "@(#)$RCSfile: XOptions.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:42 $";
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
		Xoptions.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X window AUDIT menu bar code
		
	entry points:
		OptionsMenuBarInitialize()
*/

#include "XAudit.h"
#include "Xm/CascadeB.h"
#include "Xm/MenuShell.h"
#include "Xm/PushBG.h"
#include "Xm/RowColumn.h"

void 
	AuditStatsOpen(),
#if SEC_MAC && ! SEC_SHW
	AudColnslOpen(),
	AudColxslOpen(),
#endif
	BackupDeleteOpen(),
	EnableAudit(),
	DisableAudit(),
	DirectoryListOpen(),
	ModifyEventsOpen(),
	ParametersOpen(),
	ReduceGenerateOpen(),
	RestoreOpen(),
	SelectionFileOpen(),
	SensitivityOpen(),
	ShowReportsOpen(),
	UsersGroupsOpen();

int 
	CheckAuditEnabled();

static void 
	MapPulldown();

static char 
	**msg,
	*msg_text;

void 
AuditMenuBarInitialize()
{
	char        m;
	Cardinal    n;
	Arg         args[20];
	int         audit_active,
			fd,
			i;
	Widget      Audit_pulldown,
			Collection_pulldown,
			Maintenance_pulldown,
			Reports_pulldown,
			w;
	XmString    xmstring;
				
	LoadMessage("msg_isso_audit_options", &msg, &msg_text);
				
	/********************************************************************/
	/* Audit Pulldown                                                   */
	/********************************************************************/
	
	n = 0;
	Audit_pulldown = XmCreatePulldownMenu(main_menubar, 
			"PulldownMenu", args, n);
	XtAddCallback(Audit_pulldown, XmNmapCallback, MapPulldown, NULL);
	
	/********************************************************************/
	/* Audit Pushbuttons                                                */
	/********************************************************************/
	
	/* Determine audit state to set sensitivity on buttons */
	audit_active = CheckAuditEnabled();
	
	/* Enable */
	i = 19;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,     xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,               m); n++;
	XtSetArg(args[n], XmNsensitive, ! audit_active); n++;
	maintenance_enable_push_widget = XmCreatePushButtonGadget(Audit_pulldown,
			"PulldownButton", args, n);
	XtManageChild(maintenance_enable_push_widget);
	XtAddCallback(maintenance_enable_push_widget, 
			XmNactivateCallback, EnableAudit, NULL);
	XmStringFree(xmstring);

	/* Disable */
	i = 21;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,   xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,             m); n++;
	XtSetArg(args[n], XmNsensitive, audit_active); n++;
	maintenance_disable_push_widget = XmCreatePushButtonGadget(Audit_pulldown,
			"PulldownButton", args, n);
	XtManageChild(maintenance_disable_push_widget);
	XtAddCallback(maintenance_disable_push_widget,
			XmNactivateCallback, DisableAudit, NULL);
	XmStringFree(xmstring);

		/************************************************************/
		/* System collection parameters                             */
		/************************************************************/
	
	n = 0;
	Collection_pulldown = XmCreatePulldownMenu(Audit_pulldown, 
		"PulldownMenu", args, n);

	/* Modify Audit Events */
	i = 0;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,           m); n++;
	w = XmCreatePushButtonGadget(Collection_pulldown, "PulldownButton", 
			args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, ModifyEventsOpen, NULL);
	XmStringFree(xmstring);

	/* Modify Audit Users/Groups */
	i = 33;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,           m); n++;
	w = XmCreatePushButtonGadget(Collection_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, UsersGroupsOpen, NULL);
	XmStringFree(xmstring);

#if SEC_MAC && ! SEC_SHW
	/* Required CMW, B1 */
	/* Set min and max Sensitivity Levels */
	i = 39;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,  xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,            m); n++;
	w = XmCreatePushButtonGadget(Collection_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, AudColnslOpen, NULL);
	XmStringFree(xmstring);

	/* Maximum SL */
	i = 41;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,  xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,            m); n++;
	w = XmCreatePushButtonGadget(Collection_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, AudColxslOpen, NULL);
	XmStringFree(xmstring);
#endif /* SEC_MAC && ! SEC_SHW */

	/* Collection Parameters (cascade button) */
	i = 3;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,          xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,                    m); n++;
	XtSetArg(args[n], XmNsubMenuId, Collection_pulldown); n++;
	w = XmCreateCascadeButton(Audit_pulldown, "CascadeButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

		/************************************************************/
		/* Maintenance                                              */
		/************************************************************/
	
	n = 0;
	Maintenance_pulldown = XmCreatePulldownMenu(Audit_pulldown, 
		"PulldownMenu", args, n);

	/* Backup/Delete Collection Files */
	i = 9;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,           m); n++;
	XtSetArg(args[n], XmNsensitive,       TRUE); n++;
	w = XmCreatePushButtonGadget(Maintenance_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, BackupDeleteOpen, NULL);
	XmStringFree(xmstring);
	
	/* Restore Collection Files */
	i = 11;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,           m); n++;
	XtSetArg(args[n], XmNsensitive,       TRUE); n++;
	w = XmCreatePushButtonGadget(Maintenance_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, RestoreOpen, NULL);
	XmStringFree(xmstring);
	
	i = 23;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,           m); n++;
	w = XmCreatePushButtonGadget(Maintenance_pulldown, 
			"PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, ParametersOpen, NULL);
	XmStringFree(xmstring);
				
	/* Modify Directory List */
	/* For SW_3000 this screen looks different */
	i = 15;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,           m); n++;
	w = XmCreatePushButtonGadget(Maintenance_pulldown, 
			"PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, DirectoryListOpen, NULL);
	XmStringFree(xmstring);

	i = 13;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,           xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,                     m); n++;
	XtSetArg(args[n], XmNsubMenuId, Maintenance_pulldown); n++;
	w = XmCreateCascadeButton(Audit_pulldown, "CascadeButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
		/************************************************************/
		/* Reports                                                  */
		/************************************************************/
	
	n = 0;
	Reports_pulldown = XmCreatePulldownMenu(Audit_pulldown, "PulldownMenu", args, n);
	
	/* Create/Modify Selection */
	i = 29;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,   xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,             m); n++;
	w = XmCreatePushButtonGadget(Reports_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, SelectionFileOpen, NULL);
	XmStringFree(xmstring);

	/* Display Existing Reports */
	i = 5;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,  xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,            m); n++;
	w = XmCreatePushButtonGadget(Reports_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, ShowReportsOpen, NULL);
	XmStringFree(xmstring);
		
	/* Create New Reports */
	i = 27;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;                                              
	XtSetArg(args[n], XmNlabelString,   xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,             m); n++;
	w = XmCreatePushButtonGadget(Reports_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, ReduceGenerateOpen, NULL);
	XmStringFree(xmstring);

	/* Reports (Cascade button) */
	i = 7;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,                xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,                          m); n++;
	XtSetArg(args[n], XmNsubMenuId,          Reports_pulldown); n++;
	w = XmCreateCascadeButton(Audit_pulldown, "CascadeButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
	/* Statistics (off of primary pulldown menu) */
	i = 25;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;                                              
	XtSetArg(args[n], XmNlabelString,    xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,              m); n++;
	XtSetArg(args[n], XmNsensitive,  audit_active); n++;
	statistics_push_widget = XmCreatePushButtonGadget(Audit_pulldown, "PulldownButton", args, n);
	XtManageChild(statistics_push_widget);
	XtAddCallback(statistics_push_widget, XmNactivateCallback, AuditStatsOpen, NULL);
	XmStringFree(xmstring);

	/****************************************************************/
	/* Audit Cascade from primary menu bar                          */
	/****************************************************************/
	
	i = 17;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
		/* Dies */
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,     xmstring); n++;
	XtSetArg(args[n], XmNmnemonic,               m); n++;
	XtSetArg(args[n], XmNsubMenuId, Audit_pulldown); n++;
	w = XmCreateCascadeButton(main_menubar, "MenubarButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
}

/* This callback invoked when the pulldown is mapped; it checks whether */
/* audit is enabled and sets pushbutton sensitivities appropriately.   */
static void 
MapPulldown(w, ptr, info)
	Widget    w;
	char     *ptr;
	XmAnyCallbackStruct *info ;
{
	int     audit_active;
	Cardinal    n;
	Arg         args[20];

	/* Determine audit state to set sensitivity on buttons */
	audit_active = CheckAuditEnabled();

	/* Enable button */
	n = 0;
	XtSetArg(args[n], XmNsensitive, ! audit_active); n++;
	XtSetValues(maintenance_enable_push_widget, args, n);
	
	/* Disable button */
	n = 0;
	XtSetArg(args[n], XmNsensitive, audit_active); n++;
	XtSetValues(maintenance_disable_push_widget, args, n);

	/* Audit statistics button */
	n = 0;
	XtSetArg(args[n], XmNsensitive, audit_active); n++;
	XtSetValues(statistics_push_widget, args, n);

}    
#endif /* SEC_BASE */

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
static char	*sccsid = "@(#)$RCSfile: XSysMenu.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:52 $";
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
		XMenu.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		Menu bar definition for Accounts
		System administrator version.
		
	entry points:
		AccountsMenuBarInitialize()
*/


/* Common C include files */
#include <sys/types.h>
#include <sys/secdefines.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>

/* ISSO include files */
#include "XMain.h"
#include "XAccounts.h"

/* Definitions */
#define NUM_USER_OPTIONS    9

/* External routines */
extern void
	LoadMessage(),
	MakeUserStart() ,   MakeUserOpen() ,   MakeUserClose() ,
	MakeGroupStart(),   MakeGroupOpen(),   MakeGroupClose(),
	GetUserStart()  ,   GetUserOpen()  ,   GetUserClose()  ,
	UserAuditStart(),   UserAuditOpen(),   UserAuditClose(),
	UserAuthStart() ,   UserAuthOpen() ,   UserAuthClose() ,
	UserGroupStart(),   UserGroupOpen(),   UserGroupClose(),
	UserComStart()  ,   UserComOpen()  ,   UserComClose()  ,
#if SEC_MAC && ! SEC_SHW
	UserClearStart(),   UserClearOpen(),   UserClearClose(),
#endif
	UserPassStart() ,   UserPassOpen() ,   UserPassClose() ,
#ifdef V2
	UserHoursStart(),   UserHoursOpen(),   UserHoursClose(),
#endif
	UserLoginStart(),   UserLoginOpen(),   UserLoginClose(),
	RetireStart()   ,   RetireOpen()   ,   RetireClose()   ;

/* Local routines */
static void
	MapPulldown();

/* Local variables */
	/* We insert 'dummy' routines so message file is correct */

static XtCallbackProc ModifyUser[] = {
	GetUserOpen ,
	UserAuditOpen ,
#if SEC_MAC && ! SEC_SHW
	UserClearOpen ,            /* not for SHW */
#else
	UserAuditOpen,		/* Dummy */
#endif
	UserGroupOpen ,
	UserComOpen   ,
	UserAuthOpen  ,
#ifdef V2
	/* Not supporting Hours for 1.0 */
	UserHoursOpen ,
#else
	UserPassOpen ,
#endif
	UserLoginOpen ,
	UserPassOpen ,
	};

static char
	**msg,
	*msg_text;

static Widget
	ModifyButtons[NUM_USER_OPTIONS];

void 
SysAdminAccountsMenuBarInitialize()
{
	Cardinal    n;
	Arg         args[20];
	int         i, j;
	char        m;
	Widget      AccountsPulldown,
		    pulldown,
		    w;
	XmString    xmstring;
				
	/* Read the menu options from the external file */
	LoadMessage ("msg_accounts_options_sysadmin", &msg, &msg_text);

	/******************************************************************/
	/* Accounts Pulldown                                              */
	/******************************************************************/
	
	n = 0;
	AccountsPulldown = XmCreatePulldownMenu(main_menubar, 
			"PulldownMenu", args, n);
	
	i = 0;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString  ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic     ,m); n++;
	XtSetArg(args[n], XmNsubMenuId    ,AccountsPulldown); n++;
	w = XmCreateCascadeButton(main_menubar, "MenubarButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

	/******************************************************************/
	/* Create Account                                                 */
	/******************************************************************/
	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(AccountsPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, MakeUserOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);

	/******************************************************************/
	/* Create Group                                                   */
	/******************************************************************/
	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(AccountsPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, MakeGroupOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);

	/******************************************************************/
	/* Modify User Accounts                                           */
	/******************************************************************/
	
	n = 0;
	pulldown = XmCreatePulldownMenu(AccountsPulldown, "PulldownMenu", 
		args, n);
	XtAddCallback(pulldown, XmNmapCallback, MapPulldown, NULL);

	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString  ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic     ,m); n++;
	XtSetArg(args[n], XmNsubMenuId    ,pulldown); n++;
	w = XmCreateCascadeButton(AccountsPulldown, "CascadeButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

	/* Cascading Menu Options */
	for (j=0; j<NUM_USER_OPTIONS; j++) {
		i++;
		xmstring = XmStringCreate(msg[i], charset);
		if (! xmstring)
			MemoryError();
		m = msg[++i][0];
		n = 0;
		XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
		XtSetArg(args[n], XmNmnemonic       ,m); n++;
#ifdef V2
	/* Not supporting Hours for V2 */
	if (True &&
#else
	if (j != 6 &&
#endif
#if ! SEC_MAC || SEC_SHW
	/* No clearance for SHW */
	j != 2) {
#else
	True) {
#endif
		w = XmCreatePushButtonGadget(pulldown, "PulldownButton", args, n);
		XtManageChild(w);
		XtAddCallback (w, XmNactivateCallback, ModifyUser[j], NULL);
		ModifyButtons[j] = w;
		XmStringFree(xmstring);
		}
	/* Add a separator */
	if (j == 0) {
		n = 0;
		XtSetArg(args[n], XmNseparatorType    ,XmDOUBLE_LINE); n++;
		w = XmCreateSeparatorGadget(pulldown, "Separator", args, n);
		XtManageChild(w);
		}
	}

	/* Add a separator so that Retire Accounts stands out */
	n = 0;
	XtSetArg(args[n], XmNseparatorType    ,XmDOUBLE_LINE); n++;
	w = XmCreateSeparatorGadget(AccountsPulldown, "Separator", args, n);
	XtManageChild(w);

	/******************************************************************/
	/* Retire Accounts                                                */
	/******************************************************************/

	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(AccountsPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, RetireOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);


}

void 
SysAdminStartAccountsFunctionalUnits()
{
	chosen_user_name = NULL;
	/* The order does not really matter so all non SHW stuff put at end */
	MakeUserStart();
	GetUserStart();
	UserAuditStart();
	UserAuthStart();
	UserComStart();
	UserPassStart();
	UserGroupStart();
	UserLoginStart();
#ifdef V2
	UserHoursStart();
#endif
#if SEC_MAC && ! SEC_SHW
	UserClearStart();
#endif
}

void 
SysAdminStopAccountsFunctionalUnits()
{
	MakeUserStop();
	GetUserStop();
	UserAuditStop();
	UserAuthStop();
	UserComStop();
	UserPassStop();
	UserGroupStop();
	UserLoginStop();
#ifdef V2
	UserHoursStop();
#endif
#if SEC_MAC && ! SEC_SHW
	UserClearStop();
#endif
}

/* This callback invoked when the pulldown is mapped; it checks whether */
/* all the modify options are available for this user                  */
static void 
MapPulldown(w, ptr, info)
	Widget    w;
	char     *ptr;
	XmAnyCallbackStruct *info ;
{
	int	j;

	/* If no name selected then only enable Select */
	if (! chosen_user_name) {
		XtSetSensitive (ModifyButtons[1], False);
#if SEC_MAC && ! SEC_SHW
		XtSetSensitive (ModifyButtons[2], False);
#endif
		XtSetSensitive (ModifyButtons[3], False);
		XtSetSensitive (ModifyButtons[4], False);
		XtSetSensitive (ModifyButtons[5], False);
#ifdef V2
		XtSetSensitive (ModifyButtons[6], False);
#endif
		XtSetSensitive (ModifyButtons[7], False);
		XtSetSensitive (ModifyButtons[8], False);
	}
	else {
		XtSetSensitive (ModifyButtons[1], True);
#if SEC_MAC && ! SEC_SHW
		XtSetSensitive (ModifyButtons[2], True);
#endif
		XtSetSensitive (ModifyButtons[3], True);
		XtSetSensitive (ModifyButtons[4], True);
		XtSetSensitive (ModifyButtons[5], True);
#ifdef V2
		XtSetSensitive (ModifyButtons[6], True);
#endif
		XtSetSensitive (ModifyButtons[7], True);
		XtSetSensitive (ModifyButtons[8], True);
	}
}    
#endif /* SEC_BASE **/


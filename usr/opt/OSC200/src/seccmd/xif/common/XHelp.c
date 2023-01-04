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
static char	*sccsid = "@(#)$RCSfile: XHelp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:04:59 $";
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
		XHelp.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		Role program help menu bar code
		
	entry points:
		HelpMenuBarInitialize() -- requires single parameter for control
			string which controls where the version help file is pulled from
*/
		
/* Common C include files */
#include <sys/types.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>

/* role program include file */
#include "XMain.h"

/* External routines */
extern void 
		HelpDisplayOpen();

/* Local variables */
static char
		**msg_help,
		*msg_help_text;

void HelpMenuBarInitialize(version)
	char        *version;
{
	char        mnemonic;
	int         i;
	Cardinal    n;
	Arg         args[20];
	Widget      Help_pulldown,
				w;
	XmString    xm_string;

	if (! msg_help)
		LoadMessage("msg_help", &msg_help, &msg_help_text);
	
	/********************************************************************/
	/* Help Pulldown                                                    */
	/********************************************************************/
	
	n = 0;
	Help_pulldown = XmCreatePulldownMenu(main_menubar, 
			"PulldownMenu", args, n);

	/********************************************************************/
	/* Help Pushbuttons                                                 */
	/********************************************************************/
	
	i = 0;
	xm_string = XmStringCreate(msg_help[i], charset);
	if (! xm_string)
		MemoryError();
	mnemonic = msg_help[i+1][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,                xm_string); n++;
	XtSetArg(args[n], XmNmnemonic,                    mnemonic); n++;
	w = XmCreatePushButtonGadget(Help_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, HelpDisplayOpen, "motif,HelpKeys");
	XmStringFree(xm_string);

	i = 2;
	xm_string = XmStringCreate(msg_help[i], charset);
	if (! xm_string)
		MemoryError();
	mnemonic = msg_help[i+1][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,                xm_string); n++;
	XtSetArg(args[n], XmNmnemonic,                    mnemonic); n++;
	w = XmCreatePushButtonGadget(Help_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, HelpDisplayOpen, "motif,HelpHelp");
	XmStringFree(xm_string);

	i = 4;
	xm_string = XmStringCreate(msg_help[i], charset);
	if (! xm_string)
		MemoryError();
	mnemonic = msg_help[i+1][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,                xm_string); n++;
	XtSetArg(args[n], XmNmnemonic,                    mnemonic); n++;
	w = XmCreatePushButtonGadget(Help_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, HelpDisplayOpen, "motif,HelpFund");
	XmStringFree(xm_string);
	
	i = 6;
	xm_string = XmStringCreate(msg_help[i], charset);
	if (! xm_string)
		MemoryError();
	mnemonic = msg_help[i+1][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,                xm_string); n++;
	XtSetArg(args[n], XmNmnemonic,                    mnemonic); n++;
	w = XmCreatePushButtonGadget(Help_pulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, HelpDisplayOpen, version);
	XmStringFree(xm_string);
	
	/********************************************************************/
	/* Help Cascade from primary menu bar                               */
	/********************************************************************/
	
	i = 8;
	xm_string = XmStringCreate(msg_help[i], charset);
	if (! xm_string)
		MemoryError();
	mnemonic = msg_help[i+1][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString,                xm_string); n++;
	XtSetArg(args[n], XmNmnemonic,                    mnemonic); n++;
	XtSetArg(args[n], XmNsubMenuId,              Help_pulldown); n++;
	w = XmCreateCascadeButton(main_menubar, "MenubarButton", args, n);
	XtManageChild(w);
	XmStringFree(xm_string);
}
#endif /* SEC_BASE */

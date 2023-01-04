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
static char	*sccsid = "@(#)$RCSfile: XQuit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:05:15 $";
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
		XQuit.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	
	function:
		X window file menu bar code
		
	entry points:
		FileMenuBarInitialize()    
*/

/* Common C include files */
#include <sys/types.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>

/* ISSO include files */
#include "XMain.h"

/* Definitions */

/* External routines */

/* Local routines */
static void MainExit(); 

/* Local variables */
static char
	**msg,
	*msg_text;

void 
FileMenuBarInitialize()
{
	Cardinal    n;
	Arg         args[10];
	int         i;
	char        m;
	Widget      FilePulldown,
		    w;
	XmString    accelerator,
		xmstring;
			    
	/* Read the menu options from the external file */
	LoadMessage ("msg_file_options", &msg, &msg_text);

	/******************************************************************/
	/* File Pulldown                                                  */
	/******************************************************************/
	
	n = 0;
	FilePulldown = XmCreatePulldownMenu(main_menubar, 
			"PulldownMenu", args, n);
	
	i = 0;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic		,m); n++;
	XtSetArg(args[n], XmNsubMenuId		,FilePulldown); n++;
	w = XmCreateCascadeButton(main_menubar, "MenubarButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

		/**********************************************************/
		/* Quit                                                   */
		/**********************************************************/

	xmstring = XmStringCreate(msg[++i], charset);
	if (! xmstring)
		MemoryError();
		accelerator = XmStringCreate(msg[++i], charset);
	if (! accelerator)
		MemoryError();
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic		,m); n++;
	XtSetArg(args[n], XmNsensitive		,TRUE); n++;
	XtSetArg(args[n], XmNaccelerator	,"<KeyPress>F4"); n++;
	XtSetArg(args[n], XmNacceleratorText	,accelerator); n++;
	w = XmCreatePushButtonGadget(FilePulldown, "PulldownButton", args, n);
	XtManageChild(w);
	XtAddCallback(w, XmNactivateCallback, MainExit, NULL);
	XmStringFree(xmstring);
	XmStringFree(accelerator);
}

static void 
MainExit(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmAnyCallbackStruct     *info;
{
	Display     *display;
	
	XtSetSensitive(main_menubar, True);
	
	/* Clean up */
	StopFunctionalUnits();
	
	display = XtDisplay(main_shell);
	XSync(display, FALSE);
	
	exit(0);
}
#endif /* SEC_BASE */

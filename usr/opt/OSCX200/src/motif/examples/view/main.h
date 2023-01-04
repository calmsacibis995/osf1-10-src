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
/*
 * @(#)$RCSfile: main.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 20:20:38 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: main.h,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 20:20:38 $ */

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MainW.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <Xm/Form.h>
#include <Xm/CascadeB.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
/*
#include <Xm/TextF.h>
*/

#define main_h

#define DECLAREGLOBAL
#include "fileview.h"
#undef DECLAREGLOBAL

#include "mainE.h"
#include "textE.h"

/*
 * Local variables
 */
#define UIL_FILE_COUNT 1

static char *uid_files[UIL_FILE_COUNT] = {"fileview.uid"};

static MrmHierarchy theUIDdatabase;  /* MRM database hierarchy id */
/*
static MRMRegisterArg regvec[] = {
        {"exit_proc", (caddr_t) ExitCallback},
};

static MrmCount regnum = sizeof(regvec) / sizeof(MRMRegisterArg);
*/

#ifdef _NO_PROTO
static String MyLanguageProc();
static ViewPtr NewFileShell();
static Widget CreateMenuBarEntry();
static void ExitCallback();
static void CloseCallback();
static void HelpCallback();
static void OpenNewShellCallback();
static void OpenFileCallback();
static Widget CreateFileSelectionBox();
static void FileCancelCallback();
#else
static String MyLanguageProc(Display * dpy, String xnl,
			     XtAppContext theContext);

static ViewPtr NewFileShell(Widget parent, Bool primary,
			   int argc, char *argv[]);

static Widget CreateMenuBarEntry(Widget menubar, String entry, String names[],
		XtCallbackProc procs[], XtPointer private[], int count);

static void ExitCallback(Widget button, Widget root,
			 XmPushButtonCallbackStruct *call_data);

static void CloseCallback(Widget button, ViewPtr this,
			 XmPushButtonCallbackStruct *call_data);

static void HelpCallback(Widget	widget, ViewPtr this,
			 XmPushButtonCallbackStruct *call_data);

static void OpenNewShellCallback(Widget widget, ViewPtr this,
			 XmPushButtonCallbackStruct *call_data);

static void OpenFileCallback(Widget widget, ViewPtr this,
			     XmPushButtonCallbackStruct *call_data);

static Widget CreateFileSelectionBox(ViewPtr this);

static void FileCancelCallback(Widget fsb, ViewPtr this,
			XmFileSelectionBoxCallbackStruct *call_data);

#endif

#undef main_h

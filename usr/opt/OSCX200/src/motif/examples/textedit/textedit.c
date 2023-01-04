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
static char *rcsid = "@(#)$RCSfile: textedit.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 18:47:32 $";
#endif
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: textedit.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 18:47:32 $"
#endif
#endif

/************************************************************
 *
 *      textedit.c -- simple text editor
 *              Ellis S. Cohen
 *         Open Software Foundation
 *
 *   This simple text editor allows a single file at a time to
 *   be edited in one or more panes, and also allows the file to
 *   be saved, saved under a new name, copied, moved, or removed
 *
 *   The code in this file is used to initialize the application,
 *   to define localizable strings, and to create widgets.
 *   It does so by using the Mrm library to read UID files (compiled
 *   UIL files).  The code in this file hides Mrm from the rest
 *   of the application.
 *
 ************************************************************/

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>

#include <Mrm/MrmPublic.h>

extern char *appString;
extern XmString openFileString, openString,
                saveFileString, saveString,
                copyFileString, copyString,
                moveFileString, moveString,
                warnOpenString, warnWriteString, warnRemoveString;

extern Display *dpy;
extern Widget toplevel, panedWindow, textFirst, textStore;
extern Widget fileDialog, saveDialog, warnDialog, questionDialog;
extern Widget openToggle, saveToggle, copyToggle, moveToggle;

static  char *mrmfiles[] = { "textedit.uid" };

extern XtCallbackProc ToggleOpCB();
extern XtCallbackProc ToggleKeepFileDialogueCB();
extern XtCallbackProc ToggleKeepFileDialogueCB();
extern XtCallbackProc ToggleRevertToOpenCB();
extern XtCallbackProc TextChangedCB();
extern XtCallbackProc TextGainFocusCB();
extern XtCallbackProc TextLoseFocusCB();
extern XtCallbackProc TextGainPrimaryCB();
extern XtCallbackProc TextLosePrimaryCB();
extern XtCallbackProc NewCB();
extern XtCallbackProc OpenCB();
extern XtCallbackProc SaveCB();
extern XtCallbackProc SaveAsCB();
extern XtCallbackProc CopyFileCB();
extern XtCallbackProc MoveFileCB();
extern XtCallbackProc RemoveCB();
extern XtCallbackProc ExitCB();
extern XtCallbackProc CutCB();
extern XtCallbackProc CopyCB();
extern XtCallbackProc PasteCB();
extern XtCallbackProc ClearCB();
extern XtCallbackProc DeleteCB();
extern XtCallbackProc SplitCB();
extern XtCallbackProc RemovePaneCB();
extern XtCallbackProc OnePaneCB();
extern XtCallbackProc SaveYesCB();
extern XtCallbackProc SaveNoCB();
extern XtCallbackProc SaveCancelCB();
extern XtCallbackProc WarnCancelCB();
extern XtCallbackProc QuestionYesCB();

extern XtCallbackProc OkFileCB();
extern XtCallbackProc CancelFileCB();

  /* To associate UIL callback names with actual callback routines */

static MRMRegisterArg argNames[] = {
  { "ToggleOpCB", (XtPointer)ToggleOpCB },
  { "ToggleKeepFileDialogueCB", (XtPointer)ToggleKeepFileDialogueCB },
  { "ToggleRevertToOpenCB", (XtPointer)ToggleRevertToOpenCB },
  { "TextChangedCB", (XtPointer)TextChangedCB },
  { "TextGainFocusCB", (XtPointer)TextGainFocusCB },
  { "TextLoseFocusCB", (XtPointer)TextLoseFocusCB },
  { "TextGainPrimaryCB", (XtPointer)TextGainPrimaryCB },
  { "TextLosePrimaryCB", (XtPointer)TextLosePrimaryCB },
  { "NewCB", (XtPointer)NewCB },
  { "OpenCB", (XtPointer)OpenCB },
  { "SaveCB", (XtPointer)SaveCB },
  { "SaveAsCB", (XtPointer)SaveAsCB },
  { "CopyFileCB", (XtPointer)CopyFileCB },
  { "MoveFileCB", (XtPointer)MoveFileCB },
  { "RemoveCB", (XtPointer)RemoveCB },
  { "ExitCB", (XtPointer)ExitCB },
  { "CutCB", (XtPointer)CutCB },
  { "CopyCB", (XtPointer)CopyCB },
  { "PasteCB", (XtPointer)PasteCB },
  { "ClearCB", (XtPointer)ClearCB },
  { "DeleteCB", (XtPointer)DeleteCB },
  { "SplitCB", (XtPointer)SplitCB },
  { "RemovePaneCB", (XtPointer)RemovePaneCB },
  { "OnePaneCB", (XtPointer)OnePaneCB },
  { "OkFileCB", (XtPointer)OkFileCB },
  { "CancelFileCB", (XtPointer)CancelFileCB },
  { "SaveYesCB", (XtPointer)SaveYesCB },
  { "SaveNoCB", (XtPointer)SaveNoCB },
  { "SaveCancelCB", (XtPointer)SaveCancelCB },
  { "WarnCancelCB", (XtPointer)WarnCancelCB },
  { "QuestionYesCB", (XtPointer)QuestionYesCB }
};

#define MAX_ARGS 10
static Arg args[MAX_ARGS];
static int n;

static MrmHierarchy muid;
static MrmType type;

 /************************************************************
 *     Create Text Pane
 ************************************************************/

      /* Called to create another text pane in the PanedWindow.
         "paneht" is its desired height. */

Widget DemoTextPaneCreate( paneht )
     int paneht;
{
  Widget w = NULL;

  /* Each time "textPane" is fetched, a new text widget is created.
     Its source is obtained from textStore.
     We set paneMinimum (as well as height) to paneHt to ensure
     it will actually get that size */

  n = 0;
  XtSetArg( args[n], XmNsource, XmTextGetSource( textStore ) ); n++;
  XtSetArg( args[n], XmNheight, paneht ); n++;
  XtSetArg( args[n], XmNpaneMinimum, paneht ); n++;
  MrmFetchWidgetOverride( muid, "textPane", panedWindow,
			 NULL, args, n,  &w, &type);
  XtManageChild( w );

  /* Now that "textPane" is managed, we can reset paneMinimum */

  n = 0;
  XtSetArg( args[n], XmNpaneMinimum, 20 ); n++;
  XtSetValues( XtParent(w), args, n );
  return w;
}

/************************************************************
 *     Initialize the Application
 ************************************************************/

main(argc, argv)
    int argc;
    char **argv;
{
    Widget mainWindow;

      /* Initialize Mrm and Xt */

    MrmInitialize();

    toplevel = XtInitialize( "main", "TextEdit", NULL, NULL, &argc, argv );
    dpy = XtDisplay(toplevel);

      /* Associate callback names to be obtained from UIL files
         with callback routines in the application, and open
         the UIL database */

    MrmRegisterNames( argNames, XtNumber(argNames) );
    MrmOpenHierarchy( 1, mrmfiles, NULL, &muid );

      /* Get string names (which may be locale-specific) */

    MrmFetchLiteral( muid, "appString", dpy, (XtPointer)&appString, &type );
    MrmFetchLiteral( muid, "openFileString", dpy, (XtPointer)&openFileString, &type );
    MrmFetchLiteral( muid, "openString", dpy, (XtPointer)&openString, &type );
    MrmFetchLiteral( muid, "saveFileString", dpy, (XtPointer)&saveFileString, &type );
    MrmFetchLiteral( muid, "saveString", dpy, (XtPointer)&saveString, &type );
    MrmFetchLiteral( muid, "copyFileString", dpy, (XtPointer)&copyFileString, &type );
    MrmFetchLiteral( muid, "copyString", dpy, (XtPointer)&copyString, &type );
    MrmFetchLiteral( muid, "moveFileString", dpy, (XtPointer)&moveFileString, &type );
    MrmFetchLiteral( muid, "moveString", dpy, (XtPointer)&moveString, &type );
    MrmFetchLiteral( muid, "warnOpenString", dpy, (XtPointer)&warnOpenString, &type );
    MrmFetchLiteral( muid, "warnWriteString", dpy, (XtPointer)&warnWriteString, &type );
    MrmFetchLiteral( muid, "warnRemoveString", dpy, (XtPointer)&warnRemoveString, &type );

      /* Instantiate and Manage MainWindow widget hierarchy */

    mainWindow = NULL;
    MrmFetchWidget( muid, "mainWindow", toplevel, &mainWindow, &type);
    XtManageChild( mainWindow );
    XtRealizeWidget( toplevel );

      /* Locate the PanedWindow and its initial text pane */

    panedWindow = XtNameToWidget( mainWindow, "*panedWindow" );
    textFirst = XtNameToWidget( panedWindow, "*textFirst" );

      /* The unmanaged text widget textStore will hold onto the
         "source" shared by all the text panes */

/* $$$ comment the following code out */

    n = 0;
    XtSetArg( args[n], XmNsource, XmTextGetSource( textFirst ) ); n++;
    textStore = NULL;
    MrmFetchWidgetOverride( muid, "textStore", toplevel,
                                  NULL, args, n,  &textStore, &type);
    XtRealizeWidget( textStore );

/* $$$ and replace it by

    textStore = textFirst;

    $$$ to get errors without the complication of shared sources */

      /* Instantiate FileSelectionBox widget hierarchy, but leave unmanaged */

    fileDialog = NULL;
    MrmFetchWidget( muid, "fileDialog", toplevel, &fileDialog, &type );

      /* Locate various toggle buttons placed in the FileSelectionBox,
         so that their toggle state can be set when necessary */

    openToggle = XtNameToWidget( fileDialog, "*openToggle" );
    saveToggle = XtNameToWidget( fileDialog, "*saveToggle" );
    copyToggle = XtNameToWidget( fileDialog, "*copyToggle" );
    moveToggle = XtNameToWidget( fileDialog, "*moveToggle" );

      /* Instantiate other dialogs, but leave unmanaged */

    saveDialog = NULL;
    warnDialog = NULL;
    questionDialog = NULL;
    MrmFetchWidget( muid, "saveDialog", toplevel, &saveDialog, &type );
    MrmFetchWidget( muid, "warnDialog", toplevel, &warnDialog, &type );
    MrmFetchWidget( muid, "questionDialog", toplevel, &questionDialog, &type );

      /* Initialize the toolkit-specific layer */

    TkInit();

      /* Wait for events to trigger callbacks */

    XtMainLoop();
}




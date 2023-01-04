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
static char *rcsid = "@(#)$RCSfile: tk.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 18:52:08 $";
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
static char rcsid[] = "$RCSfile: tk.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 18:52:08 $"
#endif
#endif
/************************************************************
 *     tk.c -- toolkit-specific dialogue layer
 *
 *  The code in this file specifically interacts with the
 *  OSF/Motif toolkit.  
 *
 *  The first half of the file provides utility routines and a
 *  toolkit-independent (but application-specific) interface that can
 *  be used by the toolkit-independent dialogue layer of the application
 *
 *  The second half of the file defines all the callback routines.
 *  Many of these simply update state internal to this layer;
 *  others perform additional actions.  Where those actions
 *  unambiguously correspond to toolkit-specific action, routines
 *  defined in the first half of the file are called.
 *  Otherwise, an upcall is made to the toolkit-independent dialogue
 *  layer to decide what to do.
 *
 ************************************************************/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/PanedW.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>

#include "basic.h"
#include "tkdef.h"
#include "dlg.h"

#define MAX_ARGS 10

extern Widget DemoTextPaneCreate();

/* imported strings */

char *appString;
XmString openFileString, openString, saveFileString, saveString,
         copyFileString, copyString, moveFileString, moveString,
         warnOpenString, warnWriteString, warnRemoveString;

/* globals for callbacks */

Display *dpy;
Widget toplevel, panedWindow, textStore, textFirst;
Widget fileDialog, saveDialog, warnDialog, questionDialog;

   /* The File Selection Box contains a radio box which allows the user
        to select whether the file chosen is to be opened, or to be
        the destination of a save, move, or copy operation.
      openToggle, saveToggle, copyToggle, moveToggle are the toggle
        buttons; curToggle indicates which is currently selected.
      The currently selected toggle is also reflected in the title
        of the FSB, and in the label of its "ok" button. */

Widget openToggle, saveToggle, copyToggle, moveToggle;
Widget curToggle = NULL;

   /* statics for callbacks */

static XmString xms;
static Arg args[MAX_ARGS];
static int n;

   /* keep track of why a warning was raised */

static enum warn_reasons warn_reason;

   /* "textchanged" and "textchangedsince" indicate whether the text
        has been modified.
      "textchanged" is reset whenever the text is read in or written out.
      "textchangedsince" can also be explicitly reset, typically because
        a decision has been made not to save the modified text. */

static int textchanged = 0;
static int textchangedsince = 0;

   /* These four variables keep track of aspects of the various text widgets.
        textCurPrimary -- which one has the primary selection (if any)
        textCurFocus -- which currently has the focus (if any)
        textLastFocus -- which last had the focus (if any)
        textLastDestination -- which one the user selected or edited
      When an action is chosen from a menu which could potentially
        apply to any of the widgets, these determine which widget
        to use.
      textCurPrimary is used when Cut or Copy is chosen from the Edit
        Menu; the selection is cut or copied from the text widget which
        has the primary selection.
      The other variable are used when Paste is chosen from the Edit Menu,
        or when operations on panes are performed (e.g. Remove Pane).
        See TkPaneTarget for the logic as mandated by the Motif Style Guide. */

static Widget textCurFocus = NULL;
static Widget textLastFocus = NULL;
static Widget textLastDestination = NULL;
static Widget textCurPrimary = NULL;

  /* Remembers if an explicit or implicit focus policy is in use */

static unsigned char policy;

/*===========================================================
                 Support Functions
============================================================*/

/************************************************************
 * Beep
 ************************************************************/

void TkBeep()
{
    XBell( dpy, 0 );
}

/************************************************************
 * Exit
 ************************************************************/

void TkExit()
{
    exit(0);
}

/*===========================================================
                 Window Title
============================================================*/

/************************************************************
 * Display the application name plus "str" as the window title.
 ************************************************************/

  /* Note: this code should be fixed to be locale-independent */

void TkUpdateStatus( str )
    char *str;
{
    char title[128];
    
    strcpy( title, appString );
    if ( strlen(str) > 0 ) {
        strcat( title, ": " );
        strcat( title, str );
    }

    n = 0;
    XtSetArg( args[n], XmNtitle, title ); n++;
    XtSetValues( toplevel, args, n );
}

/*===========================================================
                 Text Functions
============================================================*/

/************************************************************
 * Clear XmText's XmNvalue
 ************************************************************/

void TkTextClear()
{
     n = 0;
     XtSetArg( args[n], XmNvalue, "" ); n++;
     XtSetValues( textStore, args, n );
     textchanged = 0;
     textchangedsince = 0;
}

/************************************************************
 * Store "txt" as XmText's XmNvalue
 ************************************************************/

void TkTextStore( txt )
     char *txt;
{
     n = 0;
     XtSetArg( args[n], XmNvalue, txt ); n++;
     XtSetValues( textStore, args, n );
     textchanged = 0;
     textchangedsince = 0;
}

/************************************************************
 * Get XmText's XmNvalue
 ************************************************************/

char *TkTextRetrieve()
{
     char *txt;

     n = 0;
     XtSetArg( args[n], XmNvalue, &txt ); n++;
     XtGetValues( textStore, args, n );
     textchanged = 0;
     textchangedsince = 0;
     return txt;
}

/************************************************************
 * Has Text Changed? 
 ************************************************************/

TkTextChanged()
{
    return ( textchanged > 0 );
}

/************************************************************
 * Set "Since" point for text change
 ************************************************************/

void TkTextActUnchangedSince()
{
    textchangedsince = 0;
}

/************************************************************
 * Has Text Changed Since?
 ************************************************************/

TkTextChangedSince()
{
    return ( textchangedsince > 0 );
}

/*===========================================================
                     Panes
============================================================*/

#define MAX_PANES 20

static Widget panes[MAX_PANES];
static int nextpane = 1;

/************************************************************
 * Initialize Paned Window support
 ************************************************************/

static void TkPaneInit()
{
    panes[0] = textFirst;

    n = 0;
    XtSetArg( args[n], XmNkeyboardFocusPolicy, &policy ); n++;
    XtGetValues( toplevel, args, n );
}

/************************************************************
 * Get Pane Target
 ************************************************************/

   /* Determines which pane to use when an action is chosen from a menu
        which could potentially apply to any of them, as mandated by the
        Motif Style Guide.
      When an explicit focus policy is being used, the applicable pane
        is the one which has, or last had focus.
      When an implicit focus policy is being used, and the pointer is
        in one of the panes (which means that the menu action is invoked by
        typing an accelerator), the applicable pane is the one under the
        pointer.
      When an implicit focus policy is being used, but the pointer not
        in one of the panes, the applicable pane is the one which the user
        last selected or edited. */

static Widget TkPaneTarget()
{
    Widget w;

    if (nextpane == 1)  return panes[0];

    if ( policy == XmEXPLICIT )
        w = textLastFocus;
    else if ( textCurFocus != NULL ) /* when an accelerator is used */
        w = textCurFocus;
    else
        w = textLastDestination;

    return w;
}

/************************************************************
 * Return index of the pane which holds Text Widget "w"
 ************************************************************/

static int TkPaneForText( w )
    Widget w;
{
    int findpane = 0;

    while ( panes[findpane] != w )
        findpane++;

    return findpane;
}

/************************************************************
 * Delete Text Pane
 ************************************************************/

static void TkPaneDelete( w )
    Widget w;
{
    XtDestroyWidget( w );
    if ( w == textCurFocus ) textCurFocus = NULL;
    if ( w == textLastFocus ) textLastFocus = NULL;
    if ( w == textCurPrimary ) textCurPrimary = NULL;
    if ( w == textLastDestination ) textLastDestination = NULL;
}

/************************************************************
 * Append a new Text Pane
 ************************************************************/

static void TkPaneAppend()
{
    short ht;

    if ( nextpane == MAX_PANES ) {
        TkBeep();
        return;
    }    

      /* Set the new pane's height so it will be 90%
         of the average pane height */

    n = 0;
    XtSetArg( args[n], XmNheight, &ht ); n++;
    XtGetValues( panedWindow, args, n );

    ht = ( 9 * ht ) / ( 10 * ( nextpane + 1 ) );

    panes[nextpane] = DemoTextPaneCreate( ht );
    nextpane++;
}

/************************************************************
 * Remove the target Text Pane
 ************************************************************/

static void TkPaneRemove()
{
    Widget w;
    int findpane;

    if ( nextpane == 1 ) {
        TkBeep();
        return;
    }    

    nextpane--;
    w = TkPaneTarget();
    if ( w == NULL )
        findpane = nextpane;
    else
        findpane = TkPaneForText( w );

    TkPaneDelete( panes[findpane] );

    while ( findpane < nextpane ) {
        panes[findpane] = panes[findpane+1];
        findpane++;
    }
}

/************************************************************
 * Remove all panes but the target Text Pane
 ************************************************************/

static void TkPaneOne()
{
    Widget w;
    int findpane;

    if ( nextpane == 1 ) {
        TkBeep();
        return;
    }    

    w = TkPaneTarget();
    if ( w == NULL )
        findpane = 0;
    else
        findpane = TkPaneForText( w );

    if ( findpane > 0 ) {
        w = panes[0];
        panes[0] = panes[findpane];
        panes[findpane] = w;
    }

    while ( nextpane > 1 ) {
        nextpane--;
        TkPaneDelete( panes[nextpane] );
    }
}

/*===========================================================
                 FSB Functions
============================================================*/

/************************************************************
 * Arrange FSB to Open
 ************************************************************/

void TkArrangeToOpen()
{
    if ( curToggle == openToggle ) return;

    n = 0;
    XtSetArg( args[n], XmNdialogTitle, openFileString ); n++;
    XtSetValues( fileDialog, args, n );

    n = 0;
    XtSetArg( args[n], XmNokLabelString, openString ); n++;
    XtSetValues( fileDialog, args, n );

    curToggle = openToggle;
    XmToggleButtonSetState( curToggle, 1, true );
}

/************************************************************
 * Arrange FSB to Save
 ************************************************************/

static void TkArrangeToSave()
{
    if ( curToggle == saveToggle ) return;

    n = 0;
    XtSetArg( args[n], XmNdialogTitle, saveFileString ); n++;
    XtSetValues( fileDialog, args, n );

    n = 0;
    XtSetArg( args[n], XmNokLabelString, saveString ); n++;
    XtSetValues( fileDialog, args, n );

    curToggle = saveToggle;
    XmToggleButtonSetState( curToggle, 1, true );
}

/************************************************************
 * Arrange FSB to Copy
 ************************************************************/

static void TkArrangeToCopy()
{
    if ( curToggle == copyToggle ) return;

    n = 0;
    XtSetArg( args[n], XmNdialogTitle, copyFileString ); n++;
    XtSetValues( fileDialog, args, n );

    n = 0;
    XtSetArg( args[n], XmNokLabelString, copyString ); n++;
    XtSetValues( fileDialog, args, n );

    curToggle = copyToggle;
    XmToggleButtonSetState( curToggle, 1, true );
}

/************************************************************
 * Arrange FSB to Move
 ************************************************************/

static void TkArrangeToMove()
{
    if ( curToggle == moveToggle ) return;

    n = 0;
    XtSetArg( args[n], XmNdialogTitle, moveFileString ); n++;
    XtSetValues( fileDialog, args, n );

    n = 0;
    XtSetArg( args[n], XmNokLabelString, moveString ); n++;
    XtSetValues( fileDialog, args, n );

    curToggle = moveToggle;
    XmToggleButtonSetState( curToggle, 1, true );
}

/************************************************************
 * Post FSB for Opening
 ************************************************************/

void TkAskFileToOpen()
{
    TkArrangeToOpen();
    XtManageChild( fileDialog );
}

/************************************************************
 * Post FSB for Saving
 ************************************************************/

void TkAskFileToSave()
{
    TkArrangeToSave();
    XtManageChild( fileDialog );
}

/************************************************************
 * Post FSB for Copying
 ************************************************************/

void TkAskFileToCopy()
{
    TkArrangeToCopy();
    XtManageChild( fileDialog );
}

/************************************************************
 * Post FSB for Moving
 ************************************************************/

void TkAskFileToMove()
{
    TkArrangeToMove();
    XtManageChild( fileDialog );
}

/************************************************************
 * Unpost file selection box
 ************************************************************/

void TkDoneAskingFile()
{
    XtUnmanageChild( fileDialog );
}

/************************************************************
 * Get Filename selected in FSB
 ************************************************************/

static char *TkGetFilename()
{
    char *str;

    n = 0;
    XtSetArg( args[n], XmNtextString, &xms ); n++;
    XtGetValues( fileDialog, args, n );
    XmStringGetLtoR( xms, XmSTRING_DEFAULT_CHARSET, &str );
    return str;
}

/*===========================================================
                 Warn & Question Dialog
============================================================*/

/************************************************************
 * Warn couldn't open
 ************************************************************/

void TkWarn( reason )
    enum warn_reasons reason;
{
    TkBeep();
    warn_reason = reason;
    n = 0;
    switch ( reason )
    {
    case warn_open:
        XtSetArg( args[n], XmNmessageString, warnOpenString ); n++;
	break;
    case warn_write:
        XtSetArg( args[n], XmNmessageString, warnWriteString ); n++;
	break;
    case warn_save:
        XtSetArg( args[n], XmNmessageString, warnWriteString ); n++;
	break;
    case warn_remove:
        XtSetArg( args[n], XmNmessageString, warnRemoveString ); n++;
	break;
    }
    XtSetValues( warnDialog, args, n );
    XtManageChild( warnDialog );
}

/************************************************************
 * Post FSB for Saving, and warn
 ************************************************************/

void TkWarnAndAskFileToSave( reason )
    enum warn_reasons reason;
{
    TkAskFileToSave();
    TkWarn( reason );
}

/************************************************************
 * Question about Removing
 ************************************************************/

void TkQuestionRemove()
{
    XtManageChild( questionDialog );
}

/*===========================================================
                 Save Dialog
============================================================*/

/************************************************************
 * Post Save Dialog
 ************************************************************/

void TkAskSave()
{
    XtManageChild( saveDialog );
}

/************************************************************
 * Unpost Save Dialog
 ************************************************************/

void TkDoneAskingSave()
{
    XtUnmanageChild( saveDialog );
}

/*===========================================================
               Initialization & Callbacks
============================================================*/

TkInit()
{
    TkPaneInit();   /* Text Pane support initialization */
    DlgWantClearCB(); /* Arrange for a clear text area */
}

/************************************************************
 * User toggled among Open/Save/Copy/Move
 ************************************************************/

XtCallbackProc ToggleOpCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    w = ((XmRowColumnCallbackStruct *)wd)->widget;
    wd = (XtPointer)( ((XmRowColumnCallbackStruct *)wd)->callbackstruct );
    if ( ((XmToggleButtonCallbackStruct *)wd)->set )
        if      ( w == openToggle ) TkArrangeToOpen();
        else if ( w == saveToggle ) TkArrangeToSave();
        else if ( w == copyToggle ) TkArrangeToCopy();
        else if ( w == moveToggle ) TkArrangeToMove();
}

/************************************************************
 * User toggled whether or not FSB should stay mapped
 ************************************************************/

XtCallbackProc ToggleKeepFileDialogueCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    unsigned char val;

    n = 0;
    XtSetArg( args[n], XmNset, &val ); n++;
    XtGetValues( w, args, n );
    DlgKeepFileDialogueCB( val );
}

/************************************************************
 * User toggled whether operation should revert to Open after selecting a file
 ************************************************************/

XtCallbackProc ToggleRevertToOpenCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    unsigned char val;

    n = 0;
    XtSetArg( args[n], XmNset, &val ); n++;
    XtGetValues( w, args, n );
    DlgRevertToOpenCB( val );
}

/************************************************************
 * A text pane gained focus
 ************************************************************/

XtCallbackProc TextGainFocusCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    textCurFocus = w;
    textLastFocus = w;
}

/************************************************************
 * A text pane lost focus
 ************************************************************/

XtCallbackProc TextLoseFocusCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    textCurFocus = NULL;
    if ( w == XmGetDestination( dpy ) )
        textLastDestination = w;
}

/************************************************************
 * A text pane became owner of the primary selection
 ************************************************************/

XtCallbackProc TextGainPrimaryCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    textCurPrimary = w;
}

/************************************************************
 * A text pane lost ownership of the primary selection
 ************************************************************/

XtCallbackProc TextLosePrimaryCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    textCurPrimary = NULL;
}

/************************************************************
 * The text was modified
 ************************************************************/

XtCallbackProc TextChangedCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    if ( textchanged == 0 ) DlgNoteJustChangedCB();
    textchanged++;
    if ( textchangedsince == 0 ) DlgNoteJustChangedSinceCB();
    textchangedsince++;
}

/************************************************************
 * The user pressed the FSB's Ok button
 ************************************************************/

XtCallbackProc OkFileCB(w, cd, wd)
    Widget w;
    XtPointer cd, wd;
{
    char *filnam;

    filnam = TkGetFilename();

    if      ( curToggle == openToggle ) DlgSelectOpenCB( filnam );
    else if ( curToggle == saveToggle ) DlgSelectSaveCB( filnam );
    else if ( curToggle == copyToggle ) DlgSelectCopyCB( filnam );
    else if ( curToggle == moveToggle ) DlgSelectMoveCB( filnam );

    XtFree( filnam );
}

/************************************************************
 * The user pressed the FSB's Cancel button
 ************************************************************/

XtCallbackProc CancelFileCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgSelectCancelCB();
}

/************************************************************
 * The user pressed Yes when asked about saving the File
 ************************************************************/

XtCallbackProc SaveYesCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgSaveYesCB();
}

/************************************************************
 * The user pressed No when asked about saving the File
 ************************************************************/

XtCallbackProc SaveNoCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgSaveNoCB();
}

/************************************************************
 * The user pressed Cancel when asked about saving the File
 ************************************************************/

XtCallbackProc SaveCancelCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgSaveCancelCB();
}

/************************************************************
 * The user pressed Cancel when warned
 ************************************************************/

XtCallbackProc WarnCancelCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgWarnCancelCB( warn_reason );
}

/************************************************************
 * The user pressed Yes when questioned about remving the file
 ************************************************************/

XtCallbackProc QuestionYesCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgQuestionYesCB();
}

/************************************************************
 * The user pressed New in the File Menu
 ************************************************************/

XtCallbackProc NewCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgWantClearCB();
}

/************************************************************
 * The user pressed Open in the File Menu
 ************************************************************/

XtCallbackProc OpenCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgWantOpenCB();
}

/************************************************************
 * The user pressed Save in the File Menu
 ************************************************************/

XtCallbackProc SaveCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgWantSaveCB();
}

/************************************************************
 * The user pressed Save As in the File Menu
 ************************************************************/

XtCallbackProc SaveAsCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgWantSaveAsCB();
}

/************************************************************
 * The user pressed Copy in the File Menu
 ************************************************************/

XtCallbackProc CopyFileCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgWantCopyCB();
}

/************************************************************
 * The user pressed Move in the File Menu
 ************************************************************/

XtCallbackProc MoveFileCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgWantMoveCB();
}

/************************************************************
 * The user pressed Remove in the File Menu
 ************************************************************/

XtCallbackProc RemoveCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgWantRemoveCB();
}

/************************************************************
 * The user pressed Exit in the File Menu
 ************************************************************/

XtCallbackProc ExitCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    DlgExitCB();
}

/************************************************************
 * The user pressed Cut in the Edit Menu
 ************************************************************/

XtCallbackProc CutCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    if ( textCurPrimary != NULL )
        XmTextCut( textCurPrimary, XtLastTimestampProcessed( dpy ) );
    else
        TkBeep();
}

/************************************************************
 * The user pressed Copy in the Edit Menu
 ************************************************************/

XtCallbackProc CopyCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    if ( textCurPrimary != NULL )
        XmTextCopy( textCurPrimary, XtLastTimestampProcessed( dpy ) );
    else
        TkBeep();
}

/************************************************************
 * The user pressed Paste in the Edit Menu
 ************************************************************/

XtCallbackProc PasteCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    w = TkPaneTarget();
    if ( w != NULL )
        XmTextPaste( w );
    else
        TkBeep();
}

/************************************************************
 * The user pressed Clear in the Edit Menu
 ************************************************************/

XtCallbackProc ClearCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    if ( textCurPrimary != NULL )
        XtCallActionProc( textCurPrimary, "clear-selection",
            ((XmAnyCallbackStruct *)(wd))->event, NULL, 0 );
    else
        TkBeep();
}

/************************************************************
 * The user pressed Delete in the Edit Menu
 ************************************************************/

XtCallbackProc DeleteCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    if ( textCurPrimary != NULL )
        XmTextRemove( textCurPrimary );
    else
        TkBeep();
}

/************************************************************
 * The user pressed Split in the View Menu
 ************************************************************/

XtCallbackProc SplitCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
      /* It would be nice if the target pane could be split, and the
         new pane inserted underneath the target pane, but this is
         not easy to do in Motif 1.1 */

    TkPaneAppend();
}

/************************************************************
 * The user pressed Remove Pane in the View Menu
 ************************************************************/

XtCallbackProc RemovePaneCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    TkPaneRemove();
}

/************************************************************
 * The user pressed One Pane in the View Menu
 ************************************************************/

XtCallbackProc OnePaneCB(w,cd,wd)
    Widget w;
    XtPointer cd, wd;
{
    TkPaneOne();
}

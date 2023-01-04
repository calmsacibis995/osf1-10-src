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
*****************************************************************************

	      Copyright (c) Digital Equipment Corporation, 1990  
	      All Rights Reserved.  Unpublished rights reserved
	      under the copyright laws of the United States.
	      
	      The software contained on this media is proprietary
	      to and embodies the confidential technology of 
	      Digital Equipment Corporation.  Possession, use,
	      duplication or dissemination of the software and
	      media is authorized only pursuant to a valid written
	      license from Digital Equipment Corporation.

	      RESTRICTED RIGHTS LEGEND   Use, duplication, or 
	      disclosure by the U.S. Government is subject to
	      restrictions as set forth in Subparagraph (c)(1)(ii)
	      of DFARS 252.227-7013, or in FAR 52.227-19, as
	      applicable.

*****************************************************************************
**++
**  FACILITY:
**
**	DECwindows Toolkit
**
**  ABSTRACT:
**
**	International Text Widget Input Module
**
**
**  MODIFICATION HISTORY:
**
**	05 June 1990  Begin rework of old CSText widget code.
**
**	21 Jan 1993   Replaced -999 constant with IN_LOSE_SELECTION
**
**--
**/

/* File: CSTextIn.c

/* Private definitions. */

/* default keysyms - define these before includes */
#define XK_MISCELLANY
#define XK_LATIN1
#define XK_LATIN2
#define XK_LATIN3
#define XK_LATIN4
#define XK_GREEK
#define DXK_PRIVATE


#include <Xm/XmP.h>
#include <X11/IntrinsicP.h>
#include "DXmPrivate.h"
#include "CSTextI.h"
#include "CSTextSrc.h"
#include "CSTextOutP.h"
#include "CSTextStr.h"
#include "CSTextInP.h"
#include "DECspecific.h"
#include <Xm/AtomMgr.h>
#include <Xm/DragC.h>
#include <Xm/DragIcon.h>
#include <Xm/DropSMgr.h>
#include <Xm/DropTrans.h>
#include <Xm/DragIconP.h>
#include <stdio.h>

#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#ifndef DXmSClassCSTextWidget

#define DXmSClassCSTextWidget "DXmCSText"
#endif 

#ifndef CSTEXT_MAX_INSERT_SIZE 
#define CSTEXT_MAX_INSERT_SIZE 1000
#endif

extern Boolean _DXmCSTextSrcConvert();

#define CastOutput(w)          ( (DXmCSTextOutput)  Output(w) )

#define OutputXYToPos(w, x, y)    ( (DXmCSTextPosition)           \
                                    (*(CastOutput(w)->XYToPos)) (w, x, y) )

#define OutputPosToXY(w, pos, x, y)  ( (DXmCSTextPosition)               \
                                ( *(CastOutput(w)->PosToXY)) (w, pos, x, y) )

#define OutputRedisplayHBar(w) ((void)( *(CastOutput(w)->RedisplayHBar)) (w) )

#define OutputHandleData(w, a, b, op)  ( \
                             ( *(CastOutput(w)->HandleData)) (w, a, b, op) )

/*DON'T conditionalize this by I18N_IO_SHR.  It will be conditionalized
 *in CSTextOut.c (which is a localizable module).
 */
#define OutputChangeInputMethod(w, e) (				\
			( *(CastOutput(w)->ChangeInputMethod)) (w, e) )


/* forward declarations */

  /* make NeedsPendingDelete() and DeleteCurrentSelection() non-static
   * to be used in localized ioshr image to support pending delete for
   * Asian character input
   */
Boolean NeedsPendingDelete();
	/* DXmCSTextWidget   widget;	*/
	/* DXmCSTextPosition insertPos;	*/

void DeleteCurrentSelection();
	/* Widget   widget;	*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RingBell();
	/* Widget   w;			*/
	/* XEvent   *event;		*/
	/* char     **params;		*/
	/* Cardinal *num_params;	*/

static int I_STR_Insert();
	/* DXmCSTextWidget	widget;		*/
	/* XmString	 	in_string;	*/
	/* DXmCSTextPosition	position;	*/

static XmString Get_Text();
	/* DXmCSTextWidget   widget;	*/
	/* DXmCSTextPosition from,	*/
	/*		     to;	*/

static Boolean DeleteOrKill();
	/*     DXmCSTextWidget    widget;	*/
	/*     DXmCSTextPosition  from, 	*/
	/*			  to;		*/
	/*     Boolean		  kill;		*/

static void StuffFromBuffer();
	/*   DXmCSTextWidget widget;	*/
	/*   int 	     buffer;	*/

static void UnKill();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RemoveCurrentSelection();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/
	/* Boolean  kill;	*/

static void KillCurrentSelection();
	/* Widget   widget;	*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void SelfInsert();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void InsertString();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char	    **params;	*/
	/* Cardinal *num_params;*/

static void SetCursorPosition();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveForwardChar();
	/* Widget w;		*/
	/* XEvent *event;	*/
	/* char **params;	*/
	/* Cardinal *num_params;*/

static void MoveBackwardChar();
	/* Widget w;		*/
	/* XEvent *event;	*/
	/* char **params;	*/
	/* Cardinal *num_params;*/

static void MoveRightChar();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveLeftChar();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveForwardWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveBackwardWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveLeftWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveRightWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveToLineStart();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveToLineEnd();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void _MoveNextLine();
	/* Widget   w;			*/
	/* XEvent   *event;		*/
	/* char     **params;		*/
	/* Cardinal *num_params;	*/
	/* Boolean  clear_selection;	*/

static void MoveNextLine();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void _MovePreviousLine();
	/* Widget   w;			*/
	/* XEvent   *event;		*/
	/* char     **params;		*/
	/* Cardinal *num_params;	*/
	/* Boolean  clear_selection;	*/

static void MovePreviousLine();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveNextPage();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MovePreviousPage();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveBeginningOfFile();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveEndOfFile();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MoveForwardParagraph();
	/* Widget w;		*/
	/* XEvent *event;	*/
	/* char **params;	*/
	/* Cardinal *num_params;*/

static void MoveBackwardParagraph();
	/* Widget w;		*/
	/* XEvent *event;	*/
	/* char **params;	*/
	/* Cardinal *num_params;*/

static void MoveDestination();
	/* Widget w;		*/
	/* XEvent *event;	*/
	/* char **params;	*/
	/* Cardinal *num_params;*/

static void ScrollOneLineUp();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ScrollOneLineDown();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void InsertNewLine();
	/* DXmCSTextWidget w;		*/
	/* XEvent         *event;	*/
	/* char           **params;	*/
	/* Cardinal       *num_params;	*/

static void InsertNewLineAndBackup();
	/* Widget   widget;	*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void InsertNewLineAndIndent();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RedrawDisplay();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void Help();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void Activate();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessVerticalParams();
	/* Widget           w;		*/
	/* XEvent           *event;	*/
	/* char             **params;	*/
	/* Cardinal         *num_params;*/
	/* DXmCSTextPosition *left;	*/
	/* DXmCSTextPosition *right;	*/
	/* DXmCSTextPosition *position;	*/

static void ProcessHorizontalParams();
	/* Widget           w;		*/
	/* XEvent           *event;	*/
	/* char             **params;	*/
	/* Cardinal         *num_params;*/
	/* DXmCSTextPosition *left;	*/
	/* DXmCSTextPosition *right;	*/
	/* DXmCSTextPosition *position;	*/
	/* DXmCSTextPosition *cursorPos;*/

static void ProcessWordParams();
	/* Widget           w;		*/
	/* XEvent           *event;	*/
	/* char             **params;	*/
	/* Cardinal         *num_params;*/
	/* DXmCSTextPosition *left;	*/
	/* DXmCSTextPosition *right;	*/
	/* DXmCSTextPosition *position;	*/
	/* DXmCSTextPosition *cursorPos;*/

static void ProcessParaParams();
	/* Widget           w;		*/
	/* XEvent           *event;	*/
	/* char             **params;	*/
	/* Cardinal         *num_params;*/
	/* DXmCSTextPosition *left;	*/
	/* DXmCSTextPosition *right;	*/
	/* DXmCSTextPosition *position;	*/
	/* DXmCSTextPosition *cursorPos;*/

static void ProcessSelectParams();
	/* Widget           w;		*/
	/* XEvent           *event;	*/
	/* char             **params;	*/
	/* Cardinal         *num_params;*/
	/* DXmCSTextPosition *left;	*/
	/* DXmCSTextPosition *right;	*/
	/* DXmCSTextPosition *position;	*/

static void KeySelection();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static Boolean VerifyLeave();
	/* Widget  w;		*/
	/* XEvent  *event;	*/

static void TextLeave();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void TextFocusOut();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessReturn();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessCancel();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void TraverseNextTabGroup();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void TraversePrevTabGroup();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void TraverseNext();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void TraversePrev();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessTab();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void TraverseUp();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void TraverseDown();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessUp();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessDown();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessShiftUp();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessShiftDown();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void TraverseHome();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessHome();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MovePageLeft();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void MovePageRight();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RemoveBackwardChar();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/
	/* Boolean  kill;	*/

static void DeleteBackwardChar();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void KillBackwardChar();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RemoveForwardChar();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/
	/* Boolean  kill;	*/

static void KillForwardChar();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void DeleteForwardChar();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RemoveForwardWord();
	/* Widget w;		*/
	/* XEvent *event;	*/
	/* char **params;	*/
	/* Cardinal *num_params;*/
	/* Boolean kill;	*/

static void DeleteForwardWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void KillForwardWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RemoveBackwardWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/
	/* Boolean  kill;	*/

static void DeleteBackwardWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void KillBackwardWord();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RemoveToEndOfLine();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/
	/* Boolean  kill;	*/

static void RemoveToStartOfLine();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/
	/* Boolean  kill;	*/

static void DeleteToStartOfLine();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void KillToStartOfLine();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void DeleteToEndOfLine();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void KillToEndOfLine();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static Boolean Convert();
	/* Widget  w;		*/
	/* Atom    *seltype;	*/
	/* Atom    *desiredtype;*/
	/* Atom    *type;	*/
	/* XtPointer *value;	*/
	/* int     *length;	*/
	/* int     *format;	*/

static Boolean SetSel2();
	/* DXmCSTextInputData data;	*/
	/* DXmCSTextPosition  left;	*/
	/* DXmCSTextPosition  right;	*/
	/* Time time;			*/

static void LoseSel2();
	/* Widget w;		*/
	/* Atom   *selection;	*/

static Boolean GetSel2();
	/* DXmCSTextInputData data;	*/
	/* DXmCSTextPosition  *left,	*/
	/*		      *right;	*/

static void SetSelectionHint();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void a_Selection();
	/* DXmCSTextWidget widget;	*/
	/* int             x, y;	*/
	/* Time            time;	*/

static void SelectAll();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ClearSelection();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void SetAnchor();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void DeselectAll();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void DoSelection();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void SetScanType();
	/* DXmCSTextInputData data;	*/
	/* XEvent            *event;	*/

static void SetQuickCut();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void SetQuickCopy();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void SecondaryNotify();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void StartPrimary();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void DoQuickAction();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void StartSecondary();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void DoGrabFocus();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static Boolean dragged();
	/* DXmCSTextSelectionHint selectionHint;	*/
	/* XEvent        *event;	*/
	/* int           threshold;	*/

static void ExtendSelection();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ExtendSecondary();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ExtendEnd();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void DoStuff();
	/* Widget   w;		*/
	/* Opaque   closure;	*/
	/* Atom     *seltype;	*/
	/* Atom     *type;	*/
	/* char     *value;	*/
	/* int      *length;	*/
	/* int      *format;	*/

static void Stuff();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ExtendSecondaryEnd();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ExtendSecondaryEndAndDelete();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessCopy();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ProcessMove();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void CopyPrimary();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void CutPrimary();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void CutClipboard();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void CopyClipboard();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void RemoteKillSelection();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void StuffSecondary();
	/* Widget   w;		*/
	/* XEvent   *event;	*/
	/* char     **params;	*/
	/* Cardinal *num_params;*/

static void ToggleEditingPath();
	/* DXmCSTextWidget widget;	*/
	/* XEvent         *event;	*/
	/* char           **params;	*/
	/* Cardinal       *num_params;	*/

static void InverseWidget();
	/* DXmCSTextWidget widget;	*/
	/* XEvent         *event;	*/
	/* char           **params;	*/
	/* Cardinal       *num_params;	*/

#ifdef _NO_PROTO
static void DropDestroyCB() ;
static void DropTransferCallback() ;
static void HandleDrop() ;
static void DragProcCallback() ;
static void DropProcCallback() ;
static void RegisterDropSite() ;
static void StartDrag() ;
static void ProcessBDrag() ;
static void ToggleOverstrike() ;
static void VoidAction() ;

#else
static void DropDestroyCB( 
                        Widget w,
                        XtPointer clientData,
                        XtPointer callData) ;
static void DropTransferCallback( 
                        Widget w,
                        XtPointer closure,
                        Atom *seltype,
                        Atom *type,
                        XtPointer value,
                        unsigned long *length,
                        int *format) ;
static void HandleDrop( 
                        Widget w,
                        XmDropProcCallbackStruct *cb) ;
static void DragProcCallback( 
                        Widget w,
                        XtPointer client,
                        XtPointer call) ;
static void DropProcCallback( 
                        Widget w,
                        XtPointer client,
                        XtPointer call) ;
static void RegisterDropSite( 
                        Widget w) ;
static void StartDrag( 
                        Widget w,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
static void ProcessBDrag( 
                        Widget w,
                        XEvent *event,
                        char **params,
                        Cardinal *num_params) ;
static void ToggleOverstrike(
                        Widget w,
                        XEvent *event,
                        char **params,
                        Cardinal *num_params) ;
static void VoidAction( 
                        Widget w,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;
#endif

/* end forward declarations */

static XContext _DXmCSTextDestContext = 0;
static XContext _DXmCSTextDNDContext = 0;

static const DXmCSTextScanType sarray[] = {
    DXmstPositions, DXmstWordBreak, DXmstEOL, DXmstAll
};

static const int sarraysize = XtNumber(sarray);

static const Boolean def_true = TRUE;
static const int def_5 = 5;

externaldef(nonvisible) XtResource i_str_input_resources[] =
{
    { XmNselectionArray,
      XmCSelectionArray,
      XmRPointer,
      sizeof(DXmCSTextScanType *),
      XtOffset(DXmCSTextInputData, sarray), 
      XmRImmediate, (XtPointer)sarray },

    { XmNselectionArrayCount,
      XmCSelectionArrayCount,
      XmRInt,
      sizeof(int),
      XtOffset(DXmCSTextInputData, sarraycount),
      XmRInt,
      (XtPointer)&sarraysize },

    { XmNpendingDelete,
      XmCPendingDelete,
      XmRBoolean,
      sizeof(Boolean),
      XtOffset(DXmCSTextInputData, pendingdelete),
      XmRBoolean,
      (XtPointer)&def_true },

    { XmNselectThreshold,
      XmCSelectThreshold,
      XmRInt,
      sizeof(int),
      XtOffset(DXmCSTextInputData, threshold),
      XmRInt,
      (XtPointer)&def_5 },

};
externaldef(nonvisible) int i_str_input_resources_count = 
			    XtNumber(i_str_input_resources);


/*=======================================================================
/* this routine will check to see if there is a selection.  If so, then  
/* if the cursor is within the selection text, and if pending delete is
/* not suppressed, then return TRUE.  Otherwise, return FALSE .          
*/
Boolean NeedsPendingDelete( widget, insertPos )
DXmCSTextWidget   widget;
DXmCSTextPosition insertPos;
{
    DXmCSTextInputData data;

    data = InputData (widget);

    /* check if pending delete is necessary
    */

    if ( data->pendingdelete	      &&
         !data->pendingoff	      &&
         CSTextHasSelection(widget)   &&
         CSTextSelLeft(widget)   <= insertPos &&
	 CSTextSelRight(widget)  >= insertPos)     
    {
       return(True);
    }else{
       return(False);
    }
}



/*================================================
/*ring the bell
*/
static void RingBell(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    XBell( XtDisplay( widget ), 0 );
}



/*=========================================================================
*   This routine will insert the international string into the source.
*   The cursor position will be set following the inserted IString.
*/
								   
static int I_STR_Insert( widget, in_string, position )
DXmCSTextWidget	 widget;
XmString	 in_string;
DXmCSTextPosition position;
{
    DXmCSTextPosition cursor_position, upos, old_length, new_length;
    DXmCSTextStatus   status;
								   
    if ( in_string == NULL ) 
	return TRUE;

    DXmCSTextDisableRedisplay(widget, FALSE);

    /*
    ** Replace call to DXmCSTextInsert() with call to
    ** _DXmCSTextSourceReplaceString() since calling DXmCSTextInsert()
    ** always sets editable to true and then calls 
    ** _DXmCSTextSourceReplaceString().	    
    ** 12/08/93, JH
    */
    status = _DXmCSTextSourceReplaceString( (DXmCSTextWidget)widget, position, 
					     position, in_string );

    if ( status != I_STR_EditDone)
    {
        RingBell( widget, (XEvent *) NULL, (char **) NULL, (Cardinal) 0 );
    }

    DXmCSTextEnableRedisplay ( widget );

    return ( status == I_STR_EditDone );
}


static XmString Get_Text( widget, from, to )
DXmCSTextWidget   widget;
DXmCSTextPosition from, to;
{
    XmString out_string;

    DXmCSTextRead( (Widget) widget, from, to, &out_string );

    return out_string;
}


/*=========================================================================
/* routine to remove a range of text enclosed by "from" and "to" from the
/* text widget.  If "kill" equals TRUE, save it to the Buffer #1 for
/* possibility of "unkill" feature.
*/
static Boolean DeleteOrKill(widget, from, to, kill)
    DXmCSTextWidget    widget;
    DXmCSTextPosition  from, to;
    Boolean	       kill;
{
    XmString           ptr;
    DXmCSTextPosition  upos;
    DXmCSTextStatus    status;


    if ( kill && from < to ) 
    {
	ptr = Get_Text( widget, from, to );
	XStoreBuffer( XtDisplay((Widget) widget ), (char *)ptr, XmStringLength( ptr ), 1 );
	XtFree((char *) ptr );
    }

    DXmCSTextDisableRedisplay( widget, FALSE );

    status = _DXmCSTextSourceReplaceString( widget, from, to, NULL);

    if ( status!= I_STR_EditDone )
    {
       RingBell( widget, (XEvent *) NULL, (char **) NULL, (Cardinal) 0 );
    }

    DXmCSTextEnableRedisplay( widget );

    return ( status == I_STR_EditDone );
}




/*====================================================================
/* retrieve the text stored in the buffer and insert it to the current
/* cursor position
*/
static void StuffFromBuffer(widget, buffer)
  DXmCSTextWidget widget;
  int 		 buffer;
{
    int len;
    DXmCSTextPosition cursor_position;
    XmString buffered_string;
    Boolean  insert_success;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    buffered_string = (XmString)XFetchBuffer( XtDisplay( widget ), 
					      &len, buffer );

    if ( (CSTextLength(widget) + XmStringLength(buffered_string)) > 
                                             CSTextMaxLength(widget) )
    {
	RingBell( widget, (XEvent *) NULL, (char **) NULL, (Cardinal) 0 );
	if (buffered_string) XmStringFree(buffered_string);
        return;
    }

    /*
    ** Fix memory leaks and stop bell from ringing twice...
    ** 12/08/93, JH
    */
    I_STR_Insert( widget, buffered_string, cursor_position );

    if (buffered_string) XmStringFree(buffered_string);
}



/*================================================================
/* restore buffered string
*/
static void UnKill(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    StuffFromBuffer( widget, 1 );
}


/*======================================================================
/* routine to remove the current selected text
*/
static void RemoveCurrentSelection(w, event, params, num_params, kill)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
Boolean  kill;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* if nothing selected, ring bell
    */
    if ( !CSTextHasSelection (widget) )
    {
	RingBell( widget, event, params, num_params );

    } else {

        /* remove the selected text
        */
	(void) DeleteOrKill( widget,
			     CSTextSelLeft(widget),
			     CSTextSelRight(widget),
			     kill );
    }
}

void DeleteCurrentSelection(widget, event, params, num_params)
Widget   widget;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* remove current selection forever (i.e. can't be undeleted)
    */

    RemoveCurrentSelection( widget, event, params, num_params, FALSE );
}


static void KillCurrentSelection(widget, event, params, num_params)
Widget   widget;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* remove current selection and store in buffer (i.e. can be unkilled)
    */

    RemoveCurrentSelection( widget, event, params, num_params, TRUE );
}



/*===================================================================
/* routine to handle normal text input 
*/
static void SelfInsert(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{				/* %%% Do look-ahead in event queue. */
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    char               input_string[CSTEXT_MAX_INSERT_SIZE];
    int                string_length, i;
    DXmCSTextPosition  cursor_position, new_position;
    KeySym             keysym;
    Boolean	       pending_delete;
    DXmCSTextStatus    status;
    XmString           cs_str;
    Status	       status_return;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);    

    /* convert keypress event to the ascii string
    */
    string_length = XmImMbLookupString((Widget) widget, 
		    (XKeyEvent *) event, 
		    input_string, 
		    CSTEXT_MAX_INSERT_SIZE,    
		    (KeySym *) &keysym, 
		    &status_return);

    /* If the user has input more data than we can handle, bail out */
    if (status_return == XBufferOverflow || string_length > CSTEXT_MAX_INSERT_SIZE)
	return;

/* The following is suitable key processing for locales which do not use
** any special keyboard input method but relies upon the character set 
** returned (directly) in each KeySym (byte 3 - see X11 keysym protocol)
** The 'case' below can be expanded at will
*/
#define	   KeysymLatin1CharSet	       0
#define	   KeysymLatin2CharSet	       1
#define	   KeysymLatin3CharSet	       2
#define	   KeysymLatin4CharSet	       3
#define	   KeysymKanaCharSet	       4
#define	   KeysymArabicCharSet	       5
#define	   KeysymCyrillicCharSet       6
#define	   KeysymGreekCharSet	       7
#define	   KeysymTechnicalCharSet      8
#define	   KeysymSpecialCharSet        9
#define	   KeysymPublishCharSet        10
#define	   KeysymAplCharSet	       11
#define    KeysymHebrewCharSet         12

    switch (keysym >> 8) {

	   case KeysymHebrewCharSet :
    		if (string_length == 0 ) {
       		     input_string[0] = (keysym & 0xFF) ;
		     string_length++ ;
		}
                     break;
	   default : break;
    }


    if ( string_length > 0 )
    {

        if ( (CSTextLength(widget) + string_length) > 
                                             CSTextMaxLength(widget) )
        {
            RingBell( widget, event, params, num_params );
            return;
        }

        DXmCSTextDisableRedisplay(widget, FALSE);

	cursor_position = DXmCSTextGetCursorPosition( widget );

        pending_delete  = NeedsPendingDelete( widget, cursor_position );

        /* handle pending delete is needed
        */
	if ( pending_delete )
        {
	    DeleteCurrentSelection( widget, event, params, num_params );
	    cursor_position = DXmCSTextGetCursorPosition( widget );
	}

        /* Now insert the new data at cursor_position
	*/

        input_string[string_length] = '\0';

        cs_str = XmStringSegmentCreate (input_string,
                             (XmStringCharSet)CSTextDefaultCharSet(widget),
                             CSTextEditingPath(widget), FALSE) ;

        if ( DXmCSTextInsertChar( (Widget) widget, cursor_position, cs_str )
							!= I_STR_EditDone )
        {
            RingBell( widget, event, params, num_params );
        }

        /* Set push-mode */
        if (CSTextEditingPath(widget) != widget->cstext.text_path)
            DXmCSTextSetCursorPosition( widget, cursor_position );

        DXmCSTextEnableRedisplay(widget);

        XmStringFree ( cs_str );

    }
}



/*========================================================================
 * Insert the string specified in the params
 */

static void InsertString(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;
    DXmCSTextPosition cursor_position;
    XmAnyCallbackStruct cb;
    char *str;
    int string_length;
    XmString cs_str;
    Boolean  pending_delete;

    if ( *num_params <= 0 )
    {
	return;
    }

    string_length = *num_params;

    if ( (CSTextLength(widget) + string_length) > CSTextMaxLength(widget) )
    {
         RingBell( widget, event, params, num_params );
         return;
    }

    str = XtMalloc ( sizeof (char) * *num_params + 1 );

    strncpy ( str, *params, *num_params );

    str[*num_params] = '\0';

    DXmCSTextDisableRedisplay(widget, FALSE);

    cursor_position = DXmCSTextGetCursorPosition (widget);

    pending_delete  = NeedsPendingDelete( widget, cursor_position );

    /* handle pending delete is needed
     */
    if ( pending_delete )
    {
	 DeleteCurrentSelection( widget, event, params, num_params );
    }

    cursor_position = DXmCSTextGetCursorPosition( widget );

    cs_str = XmStringCreateSimple ( str );

    /*
    ** Stop bell from ringing twice...
    ** 12/08/93, JH
    */
    I_STR_Insert( widget, cs_str, cursor_position );

    DXmCSTextEnableRedisplay(widget);

    XmStringFree ( cs_str );
    XtFree ( str );
}



static void SetDefaultEditingPath( widget, position)
DXmCSTextWidget    widget;
DXmCSTextPosition  position;
{
    TextLocationRec    location ;    
    /* Set the editing path in accordance to the segment at position
    */
    _DXmCSTextSourceLocate( widget, position, &location) ;

    CSTextEditingPath(widget) = 
              (location.segment == (TextSegment)NULL) 
                               ? CSTextPath(widget)
                               : location.segment->direction ;
}

/*=========================================================================
/* this routine will set the cursor position to the mouse pointer position
*/
static void SetCursorPosition(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  xy_position;
    

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    DXmCSTextDisableRedisplay(widget, FALSE);

    /* find out the position of the pointer
    */
    xy_position = OutputXYToPos (widget, event->xbutton.x, event->xbutton.y );

    /* Set the editing path in accordance to the new segment
    */
    SetDefaultEditingPath ( widget, xy_position) ; 

    /* set cursor to pointer position
    */
    DXmCSTextSetCursorPosition( widget, xy_position );

    DXmCSTextEnableRedisplay(widget);

}



/*=========================================================================
*/
/*ARGSUSED*/
static void MoveForwardChar(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextPosition  cursor_position, out_position;

    widget = (DXmCSTextWidget) w;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* unhighlight the current selection, if any
    */
    if( CSTextHasSelection (widget) )
    {
        DXmCSTextSetSelection( (Widget) widget, 
			      cursor_position, 
			      cursor_position,
			      event->xkey.time );
    }

    /* find the next position on the right
    */
    _DXmCSTextScanPositions ( widget, 
		              cursor_position, 
		              DXmsdRight, 
		              1, 
		              &out_position );

    SetDefaultEditingPath( widget, out_position) ;

    /* update the cursor position to one position right
    */
    DXmCSTextSetCursorPosition( widget, out_position );

    /* disable pending delete
    */
    InputData (widget)->pendingoff = TRUE;

}


/*=========================================================================
*/
/*ARGSUSED*/
static void MoveBackwardChar(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextPosition  cursor_position, out_position;

    widget = (DXmCSTextWidget) w;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* unhighlight the current selection, if any
    */
    if( CSTextHasSelection( widget ) )
    {
        DXmCSTextSetSelection( (Widget) widget, 
			      cursor_position, 
			      cursor_position,
			      event->xkey.time );
    }

    /* find the next position on the left
    */
    _DXmCSTextScanPositions ( widget, 
		              cursor_position, 
		              DXmsdLeft, 
		              1, 
		              &out_position );

    SetDefaultEditingPath( widget, out_position) ;

    /* update the cursor position to one position right
    */
    DXmCSTextSetCursorPosition( widget, out_position );

    /* disable pending delete
    */
    InputData (widget)->pendingoff = TRUE;

}



/*=========================================================================
*/
/*ARGSUSED*/
static void MoveRightChar(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget        widget;

    widget = (DXmCSTextWidget) w;

    /* determine the moving direction
    */
    if (CSTextPath (widget) != 0 )
    {
	MoveBackwardChar( (Widget) widget, event, params, num_params );

    } else {
	MoveForwardChar ( (Widget) widget, event, params, num_params );
    }
}


/*=========================================================================
*/
/*ARGSUSED*/
static void MoveLeftChar(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget  widget;

    widget = (DXmCSTextWidget) w;

    /* determine the moveing direction
    */
    if ( CSTextPath (widget) != 0 )
    {
	MoveForwardChar ( (Widget) widget, event, params, num_params );

    } else {
	MoveBackwardChar( (Widget) widget, event, params, num_params );
    }
}


/*=========================================================================
*/
/*ARGSUSED*/
static void MoveForwardWord(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition new_position, cursor_position;

    widget = (DXmCSTextWidget) w;

    /* find current cursor position
    */
    cursor_position = DXmCSTextGetCursorPosition( widget );

    if ( *num_params > 0 && strcmp( *params, "extend" ) == 0 )
    {
       /* perform forward word selection
        */
       char *dir = "forwardextend";
       Cardinal num = 1;

       KeySelection(w, event, &dir, &num);

       return;
    }

    if ( CSTextHasSelection( widget ) )
    {
        /* unhighlight the current selection, if any
        */
        DXmCSTextSetSelection( (Widget) widget, 
			       cursor_position, 
			       cursor_position,
			       event->xkey.time );
    }

    /* find position for next word to the right
     */
    _DXmCSTextScanWord( widget, cursor_position, DXmsdRight, &new_position );

    /* disable pending delete
    */
    InputData (widget)->pendingoff = TRUE;

    SetDefaultEditingPath( widget, new_position) ;

    DXmCSTextSetCursorPosition( widget, new_position );
}


/*=========================================================================
*/
/*ARGSUSED*/
static void MoveBackwardWord(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition new_position, cursor_position;

    widget = (DXmCSTextWidget) w;

    /* find current cursor position
    */
    cursor_position = DXmCSTextGetCursorPosition( widget );

    if ( *num_params > 0 && strcmp( *params, "extend" ) == 0 )
    {
       /* perform forward word selection
        */
       char *dir = "backwardextend";
       Cardinal num = 1;

       KeySelection(w, event, &dir, &num);

       return;
    }

    if ( CSTextHasSelection( widget ) )
    {
        /* unhighlight the current selection, if any
        */
        DXmCSTextSetSelection( (Widget) widget, 
			       cursor_position, 
			       cursor_position,
			       event->xkey.time );
    }

    /* find position for next word to the left
     */
    _DXmCSTextScanWord( widget, cursor_position, DXmsdLeft, &new_position );

    /* disable pending delete
     */
    InputData (widget)->pendingoff = TRUE;

    SetDefaultEditingPath( widget, new_position) ;

    DXmCSTextSetCursorPosition( widget, new_position );
}


/*=========================================================================
*/
/*ARGSUSED*/
static void MoveLeftWord(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;

    widget = (DXmCSTextWidget) w;

    /* move to next word according to the textpath
    */
    if (CSTextPath (widget) != 0 )
    {
	MoveForwardWord ( (DXmCSTextWidget) widget, event, params, num_params );

    } else {
	MoveBackwardWord( (DXmCSTextWidget) widget, event, params, num_params );
    }
}


/*=========================================================================
*/
/*ARGSUSED*/
static void MoveRightWord(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;

    widget = (DXmCSTextWidget) w;

    /* move to next word according to the textpath
    */
    if (CSTextPath (widget) != 0 )
    {
	MoveBackwardWord( (DXmCSTextWidget) widget, event, params, num_params );

    } else {
	MoveForwardWord ( (DXmCSTextWidget) widget, event, params, num_params );
    }
}



static void MoveToLineLeft(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget = (DXmCSTextWidget) w;

    if (CSTextPath (widget) == 0 )    /* LtoR */
    {  
	MoveToLineStart(w, event, params, num_params) ;
    }
    else
    {  
	MoveToLineEnd(w, event, params, num_params) ;
    }
}

static void MoveToLineRight(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget = (DXmCSTextWidget) w;

    if (CSTextPath (widget) == 0 )    /* LtoR */
    {  
	MoveToLineEnd(w, event, params, num_params) ;
    }
    else
    {  
	MoveToLineStart(w, event, params, num_params) ;
    }
}


/*=========================================================================
*/
static void MoveToLineStart(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition cursor_position, start_position, end_position;

    widget = (DXmCSTextWidget) w;

    cursor_position = DXmCSTextGetCursorPosition ( widget );

    /* scan the left end of the line
    */
    _DXmCSTextScanStartOfLine( widget, 
		               cursor_position, 
		               &start_position );

    /* need select?
     */
    if ( *num_params > 0 && strcmp( *params, "extend" ) == 0 )
    {
	_DXmCSTextScanEndOfLine( widget, cursor_position, &end_position );

	DXmCSTextSetSelection ( (Widget) widget,
				start_position,
				end_position,
				event->xkey.time );

	InputData (widget)->pendingoff = False;

    } else if( CSTextHasSelection( widget ) ) {

        DXmCSTextSetSelection( (Widget) widget, 
			       cursor_position, 
			       cursor_position,
			       event->xkey.time );

	InputData (widget)->pendingoff = True;
    }

    SetDefaultEditingPath( widget, start_position) ;

    /* update new cursor position
    */
    DXmCSTextSetCursorPosition( widget, start_position );
}


/*=========================================================================
*/
static void MoveToLineEnd(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition cursor_position, start_position, end_position;

    widget = (DXmCSTextWidget) w;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* scan the right end of the line
    */
    _DXmCSTextScanEndOfLine( widget, 
		             cursor_position, 
		             &end_position );

    /* need select?
     */
    if ( *num_params > 0 && strcmp( *params, "extend" ) == 0 )
    {
	_DXmCSTextScanStartOfLine( widget, cursor_position, &start_position );

	DXmCSTextSetSelection ( (Widget) widget,
				start_position,
				end_position,
				event->xkey.time );

	InputData (widget)->pendingoff = False;

    } else if( CSTextHasSelection( widget ) ) {

        	DXmCSTextSetSelection( (Widget) widget, 
			      	       cursor_position, 
			      	       cursor_position,
			      	       event->xkey.time );

		InputData (widget)->pendingoff = True;
    }

    SetDefaultEditingPath( widget, end_position) ;

    /* update new cursor position
    */
    DXmCSTextSetCursorPosition( widget, end_position );
}


/*=========================================================================
*/
static void _MoveNextLine(w, event, params, num_params, clear_selection)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
Boolean  clear_selection;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition cursor_position, new_position;
    Position         new_x, new_y, old_x, old_y;

    widget = (DXmCSTextWidget) w;

    /* find current cursor position
    */
    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* unhighlight the current selection, if required and if any
    */
    if( clear_selection && CSTextHasSelection( widget ) )
    {
        DXmCSTextSetSelection( (Widget) widget, 
			      cursor_position, 
			      cursor_position,
			      event->xkey.time );
    }

    /* figure out the old x, y.
    */
    OutputPosToXY ( widget, cursor_position, &old_x, &old_y );

    /* with the current cursor position, find the start position of the
       next line.
    */
    _DXmCSTextScanNextLine ( widget, cursor_position, &new_position );

    /* figure out the new x y.  the new y will be used for the old x and new
       y will be used for new position
    */

    OutputPosToXY ( widget, new_position, &new_x, &new_y );

    /* if new_position == max and new_y == old_y, just set position to the
     * max position
     */
    if ( new_position == CSTextLength(widget) && new_y == old_y )
    {
	/* just use CSTextLength(widget) as position */
    } else {

        /* now by using the old x and new y, find the updated position
         */
        new_position = OutputXYToPos ( widget, old_x, new_y );    
    }

    SetDefaultEditingPath( widget, new_position) ;

    /* update cursor position
    */
    DXmCSTextSetCursorPosition( widget, new_position );

    /* suppress the pending delete
    */
    InputData (widget)->pendingoff = TRUE;

}


/*=========================================================================
*/
static void MoveNextLine(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{

    _MoveNextLine (w, event, params, num_params, True);
}


static void _MovePreviousLine(w, event, params, num_params, clear_selection)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
Boolean  clear_selection;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition cursor_position, new_position;
    Position	     old_x, old_y, new_x, new_y;

    widget = (DXmCSTextWidget) w;

    /* find current cursor position
    */
    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* unhighlight the current selection, if required and if any
    */
    if( clear_selection && CSTextHasSelection( widget ) )
    {
        DXmCSTextSetSelection( (Widget) widget, 
			      cursor_position, 
			      cursor_position,
			      event->xkey.time );
    }

    /* figure out the old x, y.
    */
    OutputPosToXY ( widget, cursor_position, &old_x, &old_y );

    /* with the current cursor position, find the start position of the
       previous line.
    */

    _DXmCSTextScanPrevLine ( widget, cursor_position, &new_position );

    /* figure out the new x y.  the new y will be used for the old x and new
       y will be used for new position
    */

    OutputPosToXY ( widget, new_position, &new_x, &new_y );

    /* if new_position return == 0 and no change with y, just set new
     * position to 0
     */
    if ( new_position == 0 && new_y == old_y )
    {
	/* just use new_position == 0 */
    } else {
        /* now by using the old x and new y, find the updated position
         */
        new_position = OutputXYToPos ( widget, old_x, new_y );
    }

    SetDefaultEditingPath( widget, new_position) ;

    /* update cursor position
    */
    DXmCSTextSetCursorPosition( widget, new_position );

    /* suppress the pending delete
    */
    InputData (widget)->pendingoff = TRUE;
    
}


/*=========================================================================
*/
static void MovePreviousLine(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{

    _MovePreviousLine(w, event, params, num_params, True);
}


static void MoveNextPage(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition left, right, cursor_position, new_pos;
    int num_lines, num_text_lines;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
        return;
    }

    cursor_position = DXmCSTextGetCursorPosition( widget );

    num_lines = DXmCSTextNumLines (widget);

    num_text_lines = DXmCSTextPosToLine ( widget,
					  DXmCSTextGetLastPosition( widget ) );

    /* just return if can't scroll
     */
    if (num_lines >= num_text_lines)
    {
	return;
    }

    DXmCSTextDisableRedisplay( widget, TRUE );

    DXmCSTextVerticalScroll (widget, -num_lines);

    /* position the cursor
     */
    if ( BotPos(widget) <= CSTextLength(widget) )
	new_pos = (TopPos(widget) + BotPos(widget) ) / 2;
    else
	new_pos = BotPos(widget);

    SetDefaultEditingPath( widget, new_pos) ;

    DXmCSTextSetCursorPosition( widget, new_pos );

    if (CSTextAddMode (widget) &&
	DXmCSTextGetSelectionInfo ((Widget) widget, &left, &right) &&
	cursor_position >= left && cursor_position <= right)
    {
	InputData (widget)->pendingoff = False;
    } else {
	InputData (widget)->pendingoff = True;
    }

    DXmCSTextEnableRedisplay ( widget );
}




/*=========================================================================
*/
static void MovePreviousPage(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition left, right, cursor_position, new_pos;
    int num_lines, num_text_lines;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
        return;
    }

    cursor_position = DXmCSTextGetCursorPosition( widget );

    num_lines = DXmCSTextNumLines (widget);

    num_text_lines = DXmCSTextPosToLine ( widget,
					  DXmCSTextGetLastPosition( widget ) );

    /* just return if can't scroll
     */
    if (num_lines >= num_text_lines)
    {
	return;
    }

    DXmCSTextDisableRedisplay( widget, TRUE );

    DXmCSTextVerticalScroll (widget, num_lines);

    /* position the cursor
     */
    if ( TopPos(widget) == (DXmCSTextPosition) 0 )
	new_pos = TopPos(widget);
    else
	new_pos = (TopPos(widget) + BotPos(widget) ) / 2;

    SetDefaultEditingPath( widget, new_pos) ;

    DXmCSTextSetCursorPosition( widget, new_pos );

    if (CSTextAddMode (widget) &&
	DXmCSTextGetSelectionInfo ((Widget) widget, &left, &right) &&
	cursor_position >= left && cursor_position <= right)
    {
	InputData (widget)->pendingoff = False;
    } else {
	InputData (widget)->pendingoff = True;
    }

    DXmCSTextEnableRedisplay ( widget );
}


static void MoveFileExtremeLeft(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget = (DXmCSTextWidget) w;

    if (CSTextPath (widget) == 0 )    /* LtoR */
    {  
        MoveBeginningOfFile(w, event, params, num_params) ;
    }
    else
    {  
        MoveEndOfFile(w, event, params, num_params) ;
    }
}

static void MoveFileExtremeRight(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget   widget = (DXmCSTextWidget) w;

    if (CSTextPath (widget) == 0 )    /* LtoR */
    {  
        MoveEndOfFile(w, event, params, num_params) ;
    }
    else
    {  
        MoveBeginningOfFile(w, event, params, num_params) ;
    }
}


/*=========================================================================
*/
static void MoveBeginningOfFile(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextPosition  cursor_position, new_position;

    widget = (DXmCSTextWidget) w;

    cursor_position = DXmCSTextGetCursorPosition ( widget );

    /* scan for the beginning of the file
    */
    _DXmCSTextScanAll( widget, cursor_position, DXmsdLeft, &new_position );

    SetDefaultEditingPath( widget, new_position) ;

    /* update the cursor to new position
    */
    DXmCSTextSetCursorPosition( widget, new_position );

    if ( *num_params > 0 && strcmp( *params, "extend" ) == 0 )
    {
	DXmCSTextSetSelection ( (Widget) widget,
				new_position,
				cursor_position,
				event->xkey.time );

	InputData (widget)->pendingoff = False;
    } else {
	/* unhighlight the current selection, if any
	 */
	if( CSTextHasSelection( widget ) )
	{
		DXmCSTextSetSelection( (Widget) widget, 
			      	       cursor_position, 
			               cursor_position,
			               event->xkey.time );
	}

    	/* suppress pending delete
    	 */
    	InputData (widget)->pendingoff = TRUE;
    }
}




/*=========================================================================
*/
static void MoveEndOfFile(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextPosition  cursor_position, new_position;

    widget = (DXmCSTextWidget) w;

    cursor_position = DXmCSTextGetCursorPosition ( widget );

    /* scan for the end of the file
    */
    _DXmCSTextScanAll( widget, cursor_position, DXmsdRight, &new_position );

    SetDefaultEditingPath( widget, new_position) ;

    /* update the cursor to new position
    */
    DXmCSTextSetCursorPosition( widget, new_position );

    if ( *num_params > 0 && strcmp( *params, "extend" ) == 0 )
    {
	DXmCSTextSetSelection ( (Widget) widget,
				cursor_position,
				new_position,
				event->xkey.time );

	InputData (widget)->pendingoff = False;
    } else {

	/* unhighlight the current selection, if any
	 */
	if( CSTextHasSelection( widget ) )
	{
		DXmCSTextSetSelection( (Widget) widget, 
				       cursor_position, 
				       cursor_position,
				       event->xkey.time );
	}

	/* suppress pending delete
	 */
	InputData (widget)->pendingoff = TRUE;
    }
}


static void MoveForwardParagraph(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;
    DXmCSTextPosition new_position, cursor_position;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
        return;
    }

    /* find current cursor position
    */
    cursor_position = DXmCSTextGetCursorPosition( widget );

    if ( *num_params > 0 && strcmp( *params, "extend" ) == 0 )
    {
       /* perform forward paragraph selection
        */
       char *dir = "for_para_extend";
       Cardinal num = 1;

       KeySelection(w, event, &dir, &num);

       return;
    }

    if ( CSTextHasSelection( widget ) )
    {
        /* unhighlight the current selection, if any
        */
        DXmCSTextSetSelection( (Widget) widget, 
			       cursor_position, 
			       cursor_position,
			       event->xkey.time );
    }

    /* find position for next paragraph
     */
    _DXmCSTextScanNextParagraph( widget, cursor_position, &new_position );

    /* disable pending delete
    */
    InputData (widget)->pendingoff = TRUE;

    SetDefaultEditingPath( widget, new_position) ;

    DXmCSTextSetCursorPosition( widget, new_position );
}


static void MoveBackwardParagraph(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;
    DXmCSTextPosition new_position, cursor_position;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
        return;
    }

    /* find current cursor position
    */
    cursor_position = DXmCSTextGetCursorPosition( widget );

    if ( *num_params > 0 && strcmp( *params, "extend" ) == 0 )
    {
       /* perform forward word selection
        */
       char *dir = "back_para_extend";
       Cardinal num = 1;

       KeySelection(w, event, &dir, &num);

       return;
    }

    if ( CSTextHasSelection( widget ) )
    {
        /* unhighlight the current selection, if any
        */
        DXmCSTextSetSelection( (Widget) widget, 
			       cursor_position, 
			       cursor_position,
			       event->xkey.time );
    }

    /* find position for previous paragraph
     */
    _DXmCSTextScanPreviousParagraph( widget, cursor_position, &new_position );

    /* disable pending delete
    */
    InputData (widget)->pendingoff = TRUE;

    SetDefaultEditingPath( widget, new_position) ;

    DXmCSTextSetCursorPosition( widget, new_position );
}


static void MoveDestination(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
   DXmCSTextWidget widget = (DXmCSTextWidget) w;
   DXmCSTextPosition new_pos, left, right;
   DXmCSTextInputData data;

   if ( event->type != ButtonPress )
   {
	return;
   }

   data = InputData (widget);

   new_pos = OutputXYToPos (widget, event->xbutton.x, event->xbutton.y );

   DXmCSTextGetSelectionInfo( (Widget) widget, &left, &right );

   data->pendingoff = False;

   if ( _XmGetFocusPolicy( (Widget) widget ) == XmPOINTER )
   {
       DXmCSTextSetCursorPosition( (DXmCSTextWidget) widget, new_pos );

   } else if ( widget == (DXmCSTextWidget) _XmGetTabGroup( (Widget) widget ) ||
	       XmProcessTraversal( (Widget) widget, XmTRAVERSE_CURRENT ) ) {

       DXmCSTextSetCursorPosition( (DXmCSTextWidget) widget, new_pos );
   }

   if (  data->pendingdelete && new_pos < left && new_pos > right )
   {
      data->pendingoff = True;
   }
}



/*=========================================================================
*/
/*ARGSUSED*/
static void ScrollOneLineUp(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    /* scroll up 1 line 
    */
    DXmCSTextVerticalScroll( widget, -1 );
}


/*=========================================================================
*/
/*ARGSUSED*/
static void ScrollOneLineDown(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    /* scroll down 1 line
    */
    DXmCSTextVerticalScroll( widget, 1 );
}


/*=========================================================================
*/
static void InsertNewLine(w, event, params, num_params)
DXmCSTextWidget w;
XEvent         *event;
char           **params;
Cardinal       *num_params;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition cursor_position, new_position;
    XmString	     new_line_string;
    Boolean	     pending_delete;
    DXmCSTextStatus   status;

    widget = (DXmCSTextWidget) w;

    if ( CSTextLength(widget) + 1 > CSTextMaxLength(widget) )
    {
        RingBell( widget, event, params, num_params );
        return;
    }

    /* create the compound string equivalent of the newline character
    */

    /* must give separator the editing direction so that _parse_stream
       will not initialize level 0 to the default LtoR. Make sure it
       comes before the newline.
    */
    {
       XmString path =
                    XmStringDirectionCreate(CSTextPath ( widget ) ) ;
       XmString separator =
                    XmStringSeparatorCreate ();

       new_line_string = XmStringConcat(path, separator) ;

       XmStringFree( path ) ;
       XmStringFree( separator ) ;
    }

    /* get current cursor position.
    */
    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* check if pending delete is necessary
    */
    pending_delete  = NeedsPendingDelete( widget, cursor_position );

    if ( pending_delete )
    {
	DeleteCurrentSelection( (Widget) widget, event, params, num_params );
	cursor_position = DXmCSTextGetCursorPosition( widget );
    } 

    /* 
    ** Insert a new line character to the current position.  No need to
    ** ring the bell since I_STR_Insert will do that for you if there is
    ** an error.
    ** 12/08/93, JH
    */
    I_STR_Insert( widget, new_line_string, cursor_position );

    /* free up the memory
    */    
    XmStringFree( new_line_string );
}    


/*=========================================================================
*/
static void InsertNewLineAndBackup(widget, event, params, num_params)
Widget   widget;
XEvent   *event;
char     **params;
Cardinal *num_params;
{

    InsertNewLine    ( widget, event, params, num_params );
    MovePreviousLine ( widget, event, params, num_params );
    MoveToLineEnd    ( widget, event, params, num_params );
}


/*=========================================================================
*/
static void InsertNewLineAndIndent(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
#ifdef NOT_YET
    DXmCSTextWidget    widget;
    DXmCSTextPosition  cursor_position, 
		      left_position, 
		      right_position,
		      out_position;

    widget = (DXmCSTextWidget) w;

    DXmCSTextDisableRedisplay( widget, TRUE );

    /* get current cursor position
    */
    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* find start of line
    */
    _DXmCSTextScanStartOfLine( widget, 
		               cursor_position, 
		               &left_position );

    /* if current line is not indented, treat it as normal new line
    */

	  /*BOGUS  IsWhiteSpace() is obsoleted, find another way to do it!
           */
    if ( !IsWhiteSpace( widget, left_position ) )
    {
       InsertNewLine ( (Widget) widget, event, params, num_params );
    } else {
       /* figure out how many spaces
       */
       _DXmCSTextScanWord( widget,
		           left_position,
		           DXmsdRight,
		           &right_position );

       /* get the spaces bounded by left_position and right_position
       */
       _DXmCSTextScanPositions( widget, 
		                left_position, 
		                DXmsdRight,
		                right_position - left_position,
		                &out_position );

       /* insert new line first
       */
       InsertNewLine( (Widget) widget, event, params, num_params );

       /* indent by adding the out_char (some blanks) to the line start
       */

/* BOGUS
 * the I18n layer will have to deliver the appropriate character....
 */
       cursor_position = DXmCSTextGetCursorPosition( widget );

       I_STR_Insert( widget, out_char, cursor_position );
   }

       DXmCSTextEnableRedisplay( widget );
#endif /* NOT YET */
}



/*=========================================================================
*/
/*ARGSUSED*/
static void RedrawDisplay(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;

    DXmCSTextInvalidate (widget, 
			DXmCSTextGetTopPosition (widget), 
			MAXINT);
}



/*=========================================================================
 * Help dispatch function.  Start at the width help was invoked on, find
 * the first non-null help callback list, and call it.  Similar to _XmSocorro
 * funtion in Manager.c of Xm.
 */
static void Help(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    Widget   parent;
    XmAnyCallbackStruct cb;    

    if (w == NULL) return;

    cb.reason = XmCR_HELP;
    cb.event = (XEvent *) event;

    do
    {
        if ((XtHasCallbacks(w, XmNhelpCallback) == XtCallbackHasSome))
        {
            XtCallCallbacks (w, XmNhelpCallback, &cb);
            return;
        }
        else
            w = XtParent(w);
    }    
    while (w != NULL);
}


static void ToggleAddMode(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{

/* nyi */

}

static void ToggleOverstrike(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{

/* nyi */

}

/* ARGSUSED */
static void 
#ifdef _NO_PROTO
VoidAction( w, event, params, num_params )
        Widget w ;
        XEvent *event ;
        String *params ;
        Cardinal *num_params ;
#else
VoidAction(
        Widget w,
        XEvent *event,
        String *params,
        Cardinal *num_params )
#endif /* _NO_PROTO */
{
   /* Do Nothing */
}


/*=========================================================================
*/
/*ARGSUSED*/
static void Activate(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextCallbackStruct cb;
    XmParentInputActionRec  p_event ;

    cb.reason      = XmCR_ACTIVATE;
    cb.event       = event;
    cb.charset     = NULL;
    cb.charset_len = 0;

    p_event.process_type = XmINPUT_ACTION ;
    p_event.action = XmRETURN ;
    p_event.event = event ;/* Pointer to XEvent. */
    p_event.params = NULL ; /* Or use what you have if   */
    p_event.num_params = 0 ;/* input is from translation.*/

    XtCallCallbacks( w, XmNactivateCallback, (Opaque) &cb );

    (void) _XmParentProcess(XtParent(w), (XmParentProcessData) &p_event);
}


/*=========================================================================
*/
static void ProcessVerticalParams(w, event, params, num_params,
			          left, right, position)
Widget           w;
XEvent           *event;
char             **params;
Cardinal         *num_params;
DXmCSTextPosition *left;
DXmCSTextPosition *right;
DXmCSTextPosition *position;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  old_position, cursor_position, out_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* store the initial position
    */
    old_position = DXmCSTextGetCursorPosition( widget );

    /* move one line up or down
    */
    if ( strcmp( *params, "up" ) == 0 )
    {
        _MovePreviousLine( (Widget) widget, event, params, num_params, False );

    } else {

        _MoveNextLine( (Widget) widget, event, params, num_params, False );
    }

    data->pendingoff      = FALSE;
    data->selectionHint.x = 
    data->selectionHint.y = 0;
    data->extending       = TRUE;

    cursor_position = DXmCSTextGetCursorPosition ( widget );

    if ( CSTextHasSelection (widget) )
    {
         *left  = CSTextSelLeft (widget);
         *right = CSTextSelRight (widget);
    }

    /* if not selected, set the anchor, left and right correctly
    */
    if ( !CSTextHasSelection (widget) )
    {
       if ( strcmp( *params, "up" ) == 0 )
       {
          data->anchor    = 
          *right          = old_position;

          data->origLeft  =
          *left           = cursor_position;

       } else {

          data->anchor    = 
          *left           = old_position;

          data->origRight = 
          *right          = cursor_position;
       }
    }
    
    /* cursor_position should be the updated position
    */
    *position = cursor_position;
}



/*=========================================================================
*/
static void ProcessHorizontalParams(w, event, params, num_params,
			            left, right, position, cursorPos)
Widget           w;
XEvent           *event;
char             **params;
Cardinal         *num_params;
DXmCSTextPosition *left;
DXmCSTextPosition *right;
DXmCSTextPosition *position;
DXmCSTextPosition *cursorPos;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  out_position, cursor_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    data->selectionHint.x = 0;
    data->selectionHint.y = 0;
    data->extending       = TRUE;

    /* if no selection, set anchor to the current cursor position
    */
    if ( !CSTextHasSelection (widget) )
    {
       data->anchor    = 
       data->origLeft  = 
       data->origRight =
       *left           = 
       *right          =
       cursor_position = DXmCSTextGetCursorPosition( widget );

       if ( data->stype != DXmstPositions ) 
	    data->stype  = DXmstPositions;

    } else {

       cursor_position = DXmCSTextGetCursorPosition( widget );

       *left  = CSTextSelLeft (widget);
       *right = CSTextSelRight(widget);
    }

    /* move text cursor in direction of cursor key
    */
    if ( strcmp( *params, "right" ) == 0 )
    {
       _DXmCSTextScanPositions ( widget, 
     		                 cursor_position, 
                                 /* DXmsdLeft really means 'backward'
                                 */
                                   (CSTextPath ( widget ) != 0)
                                        ? DXmsdLeft 
                                        :
                                 /* DXmsdRight really means 'foreward'
                                 */
		                 DXmsdRight, 
		                 1, 
		                 &out_position );


    } else if ( strcmp( *params, "left" ) == 0 ) {

       _DXmCSTextScanPositions ( widget, 
     		                 cursor_position, 
                                 /* DXmsdRight really means 'foreward'
                                 */
                                   (CSTextPath ( widget ) != 0)
                                        ? DXmsdRight
                                        :
                                 /* DXmsdLeft really means 'backward'
                                 */
		                 DXmsdLeft, 
		                 1, 
		                 &out_position );

    }

    *cursorPos = out_position;
    *position  = out_position;
}


/*=========================================================================
*/
static void ProcessWordParams(w, event, params, num_params,
			      left, right, position, cursorPos)
Widget           w;
XEvent           *event;
char             **params;
Cardinal         *num_params;
DXmCSTextPosition *left;
DXmCSTextPosition *right;
DXmCSTextPosition *position;
DXmCSTextPosition *cursorPos;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  out_position, cursor_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    data->selectionHint.x = 0;
    data->selectionHint.y = 0;
    data->extending       = TRUE;

    /* if no selection, set anchor to the current cursor position
    */
    if ( !CSTextHasSelection (widget) )
    {
       data->anchor    = 
       data->origLeft  = 
       data->origRight =
       *left           = 
       *right          =
       cursor_position = DXmCSTextGetCursorPosition( widget );

       if ( data->stype != DXmstWordBreak ) 
	    data->stype  = DXmstWordBreak;

    } else {

       cursor_position = DXmCSTextGetCursorPosition( widget );

       *left  = CSTextSelLeft (widget);
       *right = CSTextSelRight(widget);
    }

    /* move text cursor in direction of cursor key
    */
    if ( strcmp( *params, "forwardextend" ) == 0 )
    {
       /* DXmsdRight is not always right and DXmsdLeft is not always left.
          Better terminology here would be DXmsdForward and DXmsdBackward.
          See ProcessHorizontalParams() as well
       */
       _DXmCSTextScanWord( widget, cursor_position, DXmsdRight, &out_position );

    } else if ( strcmp( *params, "backwardextend" ) == 0 ) {
       _DXmCSTextScanWord( widget, cursor_position, DXmsdLeft, &out_position );
    }

    *cursorPos = out_position;
    *position  = out_position;
}

/*=========================================================================
*/
static void ProcessParaParams(w, event, params, num_params,
			      left, right, position, cursorPos)
Widget           w;
XEvent           *event;
char             **params;
Cardinal         *num_params;
DXmCSTextPosition *left;
DXmCSTextPosition *right;
DXmCSTextPosition *position;
DXmCSTextPosition *cursorPos;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  out_position, cursor_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    data->selectionHint.x = 0;
    data->selectionHint.y = 0;
    data->extending       = TRUE;

    /* if no selection, set anchor to the current cursor position
    */
    if ( !CSTextHasSelection (widget) )
    {
       data->anchor    = 
       data->origLeft  = 
       data->origRight =
       *left           = 
       *right          =
       cursor_position = DXmCSTextGetCursorPosition( widget );

    } else {

       cursor_position = DXmCSTextGetCursorPosition( widget );

       *left  = CSTextSelLeft (widget);
       *right = CSTextSelRight(widget);
    }

    /* move text cursor in direction of cursor key
    */
    if ( strcmp( *params, "for_para_extend" ) == 0 )
    {
       _DXmCSTextScanNextParagraph( widget, cursor_position, &out_position );

    } else if ( strcmp( *params, "back_para_extend" ) == 0 ) {

       _DXmCSTextScanPreviousParagraph( widget, cursor_position, &out_position);
    }

    *cursorPos = out_position;
    *position  = out_position;
}


/*=========================================================================
*/
static void ProcessSelectParams(w, event, params, num_params,
			            left, right, position )
Widget           w;
XEvent           *event;
char             **params;
Cardinal         *num_params;
DXmCSTextPosition *left;
DXmCSTextPosition *right;
DXmCSTextPosition *position;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  cursor_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    data->selectionHint.x = 
    data->selectionHint.y = 0;
    data->extending       = TRUE;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* if not selected, then set current cursor position as start of selection.
       else, set left as the start
    */
    if ( !CSTextHasSelection(widget) )
    {
       data->anchor = 
       *left        = 
       *right       = cursor_position;

    } else {

       data->anchor = *left;
       *left  = CSTextSelLeft (widget);
       *right = CSTextSelRight(widget);
    }

    *position = cursor_position;
}


/*=========================================================================
*/
static void KeySelection(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextPosition  position, left, right, cursor_position;
    DXmCSTextPosition  old_position;
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    if ( *num_params == 0 )
    {
       ProcessSelectParams( w, event, params, num_params,
                               &left, &right, &position );

       cursor_position = position;
    }

    if ( *num_params > 0 &&
	 (strcmp( *params, "up"   ) == 0 || 
          strcmp( *params, "down" ) == 0) )
    {
       /* process up/down arrow
       */
       ProcessVerticalParams( w, event, params, num_params,
                                 &left, &right, &position );
       cursor_position = position;

    } else if ( *num_params > 0 &&
		(strcmp( *params, "right" ) == 0 ||
                 strcmp( *params, "left"  ) == 0 )) {
    
       /* process left/right arrow (cursor_position is set inside the procedure)
       */
       ProcessHorizontalParams( w, event, params, num_params,
                                   &left, &right, &position, &cursor_position );

    } else if ( *num_params > 0 &&
		(strcmp( *params, "forwardextend" )  == 0 ||
                 strcmp( *params, "backwardextend" ) == 0 )) {

       /* process forward/backward word selection
        */
       ProcessWordParams ( w, event, params, num_params,
			   &left, &right, &position, &cursor_position );

    } else if ( *num_params > 0 &&
		(strcmp( *params, "for_para_extend")  == 0 ||
                 strcmp( *params, "back_para_extend") == 0 )) {

       /* process forward/backward paragraph selection
        */
       ProcessParaParams ( w, event, params, num_params,
			   &left, &right, &position, &cursor_position );

    } else if ( *num_params > 0 && strcmp( *params, "select" ) &&
		CSTextAddMode (widget) ) {

       /* process select function.  select from last selected to current
          cursor position
       */
       ProcessSelectParams( w, event, params, num_params,
                               &left, &right, &position );
       cursor_position = position;

    } else {
       /* wrong parameter passed, ignored
       */
       return;
    }
     
    if ( position < 0 )
    {
       position = 0;

    } else if ( position > CSTextLength ( widget ) ) {

       position = CSTextLength ( widget );
    }

    DXmCSTextDisableRedisplay( widget, FALSE );

    /* decide the selection direction
    */
    if ( cursor_position < data->anchor )
    {
       data->extendDir = DXmsdLeft;

    } else if ( cursor_position > data->anchor ) {

       data->extendDir = DXmsdRight;
    }

    /* check for change in extend direction */
    if ( ( data->extendDir == DXmsdRight && position < data->anchor ) ||
         ( data->extendDir == DXmsdLeft  && position > data->anchor ) )
    {
        /* change extend direction
        */
        data->extendDir =
            ( data->extendDir == DXmsdRight ) ? DXmsdLeft : DXmsdRight;
    }

    /* find the left/right for setting selection
    */
    if ( data->extendDir == DXmsdRight )
    {
          right = cursor_position;
          left  = data->anchor;

    } else {

          left  = cursor_position;
          right = data->anchor;
    }

    /* select from left to right (or from anchor to the cursor position )
    */
    DXmCSTextSetSelection( (Widget)widget, left, right, event->xkey.time );

    data->pendingoff = FALSE;

    DXmCSTextSetCursorPosition( widget, cursor_position );

    DXmCSTextShowPosition( widget, (DXmCSTextPosition) -1 );

    DXmCSTextEnableRedisplay(widget);

    /* reset origLeft and origRight 
    */
    data->origLeft  = left;
    data->origRight = right;

    data->extending = FALSE;
}
 

/***************************************************************************
 * Functions to process text widget in multi-line edit mode versus single  *
 * line edit mode.                                                         *
 ***************************************************************************/

/*--------------------------------------------------------------------------+*/
static Boolean VerifyLeave(w, event)
/*--------------------------------------------------------------------------+*/
Widget  w;
XEvent  *event;
{
    DXmCSTextWidget widget;

    DXmCSTextVerifyCallbackStruct  cbdata;

    widget = (DXmCSTextWidget) w;

    cbdata.reason     = XmCR_LOSING_FOCUS;
    cbdata.event      = event;
    cbdata.doit       = True;
    cbdata.currInsert = CursorPos (widget);
    cbdata.newInsert  = CursorPos (widget);
    cbdata.startPos   = CursorPos (widget);
    cbdata.endPos     = CursorPos (widget);
    cbdata.text       = (XmString) NULL;

    XtCallCallbackList( (Widget) widget,
                        (XtCallbackList)LoseFocusCB (widget), 
                        (Opaque) &cbdata );

    return( cbdata.doit );
}
 

/*--------------------------------------------------------------------------+*/
static void TextLeave(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    if ( _XmGetFocusPolicy( (Widget) widget ) == XmPOINTER )
       VerifyLeave( widget, event );

    _XmPrimitiveLeave( (Widget)widget, event, (String *) NULL, (Cardinal *) NULL );
}
 

/*--------------------------------------------------------------------------+*/
static void TextFocusOut(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    /* If traversal is on, then the leave verification callback is called in
       the traversal event handler 
    */
    if ( event->xfocus.send_event && 
         _XmGetFocusPolicy( (Widget) widget ) == XmEXPLICIT &&
         ! CSTextTraversed (widget) )
    {
       if ( !VerifyLeave( widget, event ) )
           return;

    } else {

      if ( CSTextTraversed (widget) ) 

          CSTextTraversed (widget) = False;
    }

    _XmPrimitiveFocusOut( (Widget)widget, event, (String *)NULL, (Cardinal *) NULL );
}


/*--------------------------------------------------------------------------+*/
static void ProcessReturn(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
       Activate( (Widget) widget, event, params, num_params );

    } else {

       InsertNewLine( (Widget) widget, event, params, num_params );
    }
}


/*--------------------------------------------------------------------------+*/
static void ProcessCancel(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget        widget;
    DXmCSTextInputData     data;
    XmParentInputActionRec p_event ;
    DXmCSTextPosition      cursor_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    p_event.process_type = XmINPUT_ACTION ;
    p_event.action = XmCANCEL ;
    p_event.event = event ;/* Pointer to XEvent. */
    p_event.params = NULL ; /* Or use what you have if   */
    p_event.num_params = 0 ;/* input is from translation.*/

    if ( data->hasSel2 && data->Sel2Extending )
    {
       data->cancel = True;

       SetSel2(data, 1, 0, event->xkey.time);

       data->hasSel2 =
       data->Sel2Extending = False;
    }

    if ( CSTextHasSelection (widget) )
    {
       data->cancel = True;

       DXmCSTextSetSelection( (Widget) widget, 
			      cursor_position, 
			      cursor_position,
			      event->xkey.time );
    }

    if ( !data->cancel )
       (void) _XmParentProcess(XtParent(widget), (XmParentProcessData) &p_event);

}


/*--------------------------------------------------------------------------+*/
static void TraverseNextTabGroup(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

   /* Allow the verification routine to control the traversal 
   */
    if ( VerifyLeave( widget, event ) )
    {
       CSTextTraversed (widget) = True;
       _XmProcessTraversal( (Widget)widget, XmTRAVERSE_NEXT_TAB_GROUP, True );
    }
}


/*--------------------------------------------------------------------------+*/
static void TraversePrevTabGroup(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

   /* Allow the verification routine to control the traversal 
   */
    if ( VerifyLeave( widget, event ) )
    {
       CSTextTraversed (widget) = True;
       _XmProcessTraversal( (Widget)widget, XmTRAVERSE_PREV_TAB_GROUP, True );
    }
}
 


/*--------------------------------------------------------------------------+*/
static void TraverseNext(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
   DXmCSTextWidget widget;

   widget = (DXmCSTextWidget) w;

   /* Find out if there is anything else to traverse to */
   /* Allow the verification routine to control the traversal 
   */
   if ( VerifyLeave( widget, event ) )
   {
      CSTextTraversed (widget) = True;

      _XmProcessTraversal( (Widget) widget, XmTRAVERSE_NEXT, True );
    }
}


/*--------------------------------------------------------------------------+*/
static void TraversePrev(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
   DXmCSTextWidget widget;

   widget = (DXmCSTextWidget) w;

   /* Allow the verification routine to control the traversal 
   */
   if ( VerifyLeave( widget, event ) )
   {
       CSTextTraversed (widget) = True;

       _XmProcessTraversal( (Widget) widget, XmTRAVERSE_PREV, True );
    }
}


/*--------------------------------------------------------------------------+*/
static void ProcessTab(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
       TraverseNextTabGroup( (Widget) widget, event, params, num_params );

    } else {
       SelfInsert( (Widget) widget, event, params, num_params );
    }
}

/* ARGSUSED */
/*--------------------------------------------------------------------------+*/
static void TraverseUp(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget tw = (DXmCSTextWidget) w;

    /* Allow the verification routine to control the traversal 
    */
    if (VerifyLeave(tw, event)) 
    {
       CSTextTraversed( tw ) = True;
       _XmProcessTraversal((Widget)tw, XmTRAVERSE_UP, True);
    }

    return;
}


/* ARGSUSED */
/*--------------------------------------------------------------------------+*/
static void TraverseDown(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget tw = (DXmCSTextWidget) w;

    /* Allow the verification routine to control the traversal 
    */
    if (VerifyLeave(tw, event)) 
    {
       CSTextTraversed( tw ) = True;
       _XmProcessTraversal((Widget)tw, XmTRAVERSE_DOWN, True);
    }

    return;
}


/*--------------------------------------------------------------------------+*/
static void ProcessUp(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmMULTI_LINE_EDIT )
    {
       MovePreviousLine( (Widget) widget, event, params, num_params );
    }else{
	if ( w != XmGetTabGroup( w ))
	{
	    TraverseUp( widget, event, params, num_params );
	}
    }
}


/*--------------------------------------------------------------------------+*/
static void ProcessDown(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmMULTI_LINE_EDIT )
    {
	MoveNextLine( widget, event, params, num_params );
    }else{
	if( w != XmGetTabGroup(w) )
	{
	    TraverseDown(w, event, params, num_params);
	}
    }
}

/*--------------------------------------------------------------------------+*/
static void ProcessShiftUp(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
       TraverseUp( (Widget) widget, event, params, num_params );
    } else {
       char *dir = "up";
       Cardinal num = 1;

       KeySelection( (Widget) widget, event, &dir, &num );
    }
}


/*--------------------------------------------------------------------------+*/
static void ProcessShiftDown(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
       TraverseDown( widget, event, params, num_params );
    } else {
       char *dir = "down";
       Cardinal num = 1;

       KeySelection(w, event, &dir, &num);
    }
}


/*--------------------------------------------------------------------------+*/
static void TraverseHome(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

   /* Allow the verification routine to control the traversal 
   */
    if ( VerifyLeave( widget, event ) )
    {
       CSTextTraversed (widget) = True;
       _XmProcessTraversal( (Widget) widget, XmTRAVERSE_HOME, True );
    }
}
 

/*--------------------------------------------------------------------------+*/
static void ProcessHome(w, event, params, num_params)
/*--------------------------------------------------------------------------+*/
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;

    widget = (DXmCSTextWidget) w;

    if ( CSTextEditMode (widget) == XmSINGLE_LINE_EDIT )
    {
       TraverseHome( (Widget) widget, event, params, num_params );
    } else {
       MoveToLineStart( (Widget) widget, event, params, num_params );
    }
}


static void MovePageHorizontalBackward(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;

    if (CSTextPath (widget) == 0 )    /* LtoR */
    {  
	MovePageLeft(w, event, params, num_params) ;
    }
    else
    {  
	MovePageRight(w, event, params, num_params) ;
    }
}


static void MovePageHorizontalForward(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;

    if (CSTextPath (widget) == 0 )    /* LtoR */
    {  
	MovePageRight(w, event, params, num_params) ;
    }
    else
    {  
	MovePageLeft(w, event, params, num_params) ;
    }
}


static void MovePageLeft(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;
    DXmCSTextInputData data;
    int scroll_pixel;

    widget  = (DXmCSTextWidget) w;
    data    = InputData (widget);
    scroll_pixel = InnerWidth (widget);

    DXmCSTextDisableRedisplay( widget, FALSE );

    DXmCSTextHorizontalScroll (widget, scroll_pixel);

    DXmCSTextEnableRedisplay( widget );
}


static void MovePageRight(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget widget;
    DXmCSTextInputData data;
    int scroll_pixel;

    widget  = (DXmCSTextWidget) w;
    data    = InputData (widget);
    scroll_pixel = InnerWidth (widget);

    DXmCSTextDisableRedisplay( widget, FALSE );

    DXmCSTextHorizontalScroll (widget, -scroll_pixel);

    DXmCSTextEnableRedisplay( widget );
}
 


static void RemoveBackwardChar(w, event, params, num_params, kill)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
Boolean  kill;
{
    DXmCSTextWidget    widget;
    DXmCSTextPosition  cursor_position, left_position, right_position;
    DXmCSTextInputData data;
    Boolean	      deleted;

    widget  = (DXmCSTextWidget) w;
    data    = InputData (widget);
    deleted = FALSE;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    DXmCSTextDisableRedisplay( widget, FALSE );

    if ( NeedsPendingDelete( widget, cursor_position ) )
    {
        /* if pendingdelete is needed, just do it and return
        */
	DeleteCurrentSelection( (Widget) widget, event, params, num_params );

    } else {

	if (CSTextPath (widget) == CSTextEditingPath (widget) )
        {
            /* find the next position on the left
            */
            _DXmCSTextScanPositions ( widget, 
		                      cursor_position, 
		                      DXmsdLeft, 
		                      1, 
		                      &left_position );

            /* delete the character enclosed by left_position, and
               cursor_position
            */
            DeleteOrKill( widget, left_position, cursor_position, kill );

	} else {

            /* find the next position on the right
            */
            _DXmCSTextScanPositions ( widget, 
		                      cursor_position, 
		                      DXmsdRight, 
		                      1, 
		                      &right_position );

            /* delete the character enclosed by cursor_position, and 
               right_positon 
            */
            DeleteOrKill( widget, cursor_position, right_position, kill );

	}		
    }

    /* ask output to redraw
    */
    DXmCSTextEnableRedisplay ( widget );
}


static void DeleteBackwardChar(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* delete backward character
    */
    RemoveBackwardChar( w, event, params, num_params, FALSE );
}

static void KillBackwardChar(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* kill backward character
    */
    RemoveBackwardChar( w, event, params, num_params, TRUE );
}



static void RemoveForwardChar(w, event, params, num_params, kill)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
Boolean  kill;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition cursor_position, left_position, right_position;

    widget = (DXmCSTextWidget) w;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    DXmCSTextDisableRedisplay( widget, FALSE );

    if ( NeedsPendingDelete( widget, cursor_position ) )
    {
        /* if pendingdelete is needed, just do it and return
        */
	DeleteCurrentSelection( (Widget) widget, event, params, num_params );

    } else {

	if (CSTextPath (widget) == CSTextEditingPath (widget) )
        {
            /* find the next position on the right
            */
            _DXmCSTextScanPositions ( widget, 
		                      cursor_position, 
		                      DXmsdRight, 
		                      1, 
		                      &right_position );

            /* delete the character enclosed by cursor_position, and
               right_position
            */
            DeleteOrKill( widget, cursor_position, right_position, kill );

	} else {

            /* find the next position on the left
            */
            _DXmCSTextScanPositions ( widget, 
		                      cursor_position, 
/* Delete backward visually (forward logically)
*/
		                      DXmsdLeft, 
		                      1, 
		                      &left_position );

            /* delete the character enclosed by left_position, and 
               cursor_position 
            */
            DeleteOrKill( widget, left_position, cursor_position, kill );

	}		
    }

    /* ask output to redraw
    */
    DXmCSTextEnableRedisplay ( widget );
}


static void KillForwardChar(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* kill forward character
    */
    RemoveForwardChar( w, event, params, num_params, TRUE );
}

static void DeleteForwardChar(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* delete forward character
    */
    RemoveForwardChar( w, event, params, num_params, FALSE );
}


static void RemoveForwardWord(w, event, params, num_params, kill)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
Boolean kill;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition cursor_position, left_position, right_position;

    widget = (DXmCSTextWidget) w;

    cursor_position   = DXmCSTextGetCursorPosition( widget );

    DXmCSTextDisableRedisplay( widget, FALSE );

    if ( NeedsPendingDelete( widget, cursor_position ) )
    {
        /* if pendingdelete is needed, just do it and return
        */
	DeleteCurrentSelection( (Widget) widget, event, params, num_params );
    } else {

        /* Working with visual mode, there is no longer dependence on the
           Editing Path for determining the delete direction.
        */
            left_position = cursor_position;

            /* find position for next word to the right
            */
            _DXmCSTextScanWord( widget, 
				left_position,
				DXmsdRight,
				&right_position );

            /* delete the character enclosed by left_position, and
               right_position
            */
            DeleteOrKill( widget, left_position, right_position, kill );

        /* Working with visual mode, there is no longer dependence on the
           Editing Path for determining the delete direction.
        */
    }

    /* ask output to redraw
    */
    DXmCSTextEnableRedisplay ( widget );
}



static void DeleteForwardWord(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* delete forward word
    */
    RemoveForwardWord( w, event, params, num_params, FALSE );
}

static void KillForwardWord(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* kill forward word
    */
    RemoveForwardWord( w, event, params, num_params, TRUE );
}



static void RemoveBackwardWord(w, event, params, num_params, kill)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
Boolean  kill;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition cursor_position, left_position, right_position;

    widget = (DXmCSTextWidget) w;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    DXmCSTextDisableRedisplay( widget, FALSE );

    if ( NeedsPendingDelete( widget, cursor_position ) )
    {
        /* if pendingdelete is needed, just do it and return
        */
	DeleteCurrentSelection( (Widget) widget, event, params, num_params );

    } else {

        /* Working with visual mode, there is no longer dependence on the
           Editing Path for determining the delete direction.
        */

            right_position = cursor_position;

            /* find position for next word to the left
            */
            _DXmCSTextScanWord( widget,
				right_position,
				DXmsdLeft,
				&left_position );

            /* delete the character enclosed by left_position, and
               right_position
            */
            DeleteOrKill( widget, left_position, right_position, kill );

        /* Working with visual mode, there is no longer dependence on the
           Editing Path for determining the delete direction.
        */
    }

    /* ask output to redraw
    */
    DXmCSTextEnableRedisplay ( widget );
}


static void DeleteBackwardWord(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* delete backward word
    */
    RemoveBackwardWord( w, event, params, num_params, FALSE );
}

static void KillBackwardWord(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* kill backward word
    */
    RemoveBackwardWord( w, event, params, num_params, TRUE );
}


static void RemoveToEndOfLine(w, event, params, num_params, kill)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
Boolean  kill;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition left_position, right_position;

    widget = (DXmCSTextWidget) w;

    DXmCSTextDisableRedisplay( widget, TRUE );

    left_position = DXmCSTextGetCursorPosition( widget );

    /* scan to end of line
    */
    _DXmCSTextScanEndOfLine( widget, 
		             left_position, 
		             &right_position );

    /* if left_position < right_position, go ahead to delete
    */
    if ( left_position < right_position ) 
    {
	DeleteOrKill( widget, left_position, right_position, kill );
    }

    DXmCSTextEnableRedisplay(widget);
}


static void RemoveToStartOfLine(w, event, params, num_params, kill)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
Boolean  kill;
{
    DXmCSTextWidget   widget;
    DXmCSTextPosition left_position, right_position;

    widget = (DXmCSTextWidget) w;

    DXmCSTextDisableRedisplay( widget, TRUE );

    right_position = DXmCSTextGetCursorPosition( widget );

    /* scan to start of line
    */
    _DXmCSTextScanStartOfLine( widget, 
		               right_position, 
		               &left_position );

    /* if left_position < right_position, go ahead to delete
    */
    if ( left_position < right_position )
    {
	DeleteOrKill( widget, left_position, right_position, kill );
    }

    DXmCSTextEnableRedisplay( widget );
}


static void DeleteToStartOfLine(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* delete to start of line
    */
    RemoveToStartOfLine( w, event, params, num_params, FALSE );
}

static void KillToStartOfLine(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* kill to start of line
    */
    RemoveToStartOfLine( w, event, params, num_params, TRUE );
}


static void DeleteToEndOfLine(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* delete to end of line
    */
    RemoveToEndOfLine( w, event, params, num_params, FALSE );
}

static void KillToEndOfLine(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* kill to end of line
    */
    RemoveToEndOfLine( w, event, params, num_params, TRUE );
}


static Boolean Convert(w, seltype, desiredtype, type, value, length, format)
Widget  w;
Atom    *seltype;
Atom    *desiredtype;
Atom    *type;
XtPointer *value;
unsigned long *length;
int     *format;
{
    Atom XA_DELETE = XInternAtom(XtDisplay(w), "DELETE", False);
    Boolean           SetSel2();
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  left, right;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* convert Secondary selection only
    */
    if ( *seltype != XA_SECONDARY || !data->hasSel2 )
    {
       return FALSE;
    }

    /* call the source convert routine
    */
    if( !_DXmCSTextSrcConvert( w, seltype, desiredtype, type, 
			       value, length, format ) )
    {
	return False;
    }

    /* get left & right of the secondary selection
    */
    left  = data->sel2Left;
    right = data->sel2Right;

    /* force no secondary selection. set selected text to normal */
    /*    (forced by setting left = 1 and right = 0)
    */
    (void) SetSel2( data, 1, 0, CurrentTime );

    /* remove the secondary selected text if selectionMove is true and
    /* we haven't already removed it via a delete selection
    */
    if ( data->selectionMove && *desiredtype != XA_DELETE )
    {
        DXmCSTextDisableRedisplay ( widget, True );

	_DXmCSTextSourceReplaceString( (DXmCSTextWidget) widget, 
				       left, right, NULL);

        DXmCSTextEnableRedisplay ( widget );
    }

    return True;
}	


static Boolean SetSel2(data, left, right, time)
DXmCSTextInputData data;
DXmCSTextPosition  left;
DXmCSTextPosition  right;     /* Special hack: if right == IN_LOSE_SELECTION, */
			      /* then we're in LoseSelection, so don't call */
			      /* XtDisownSelection. */
Time time;
{
    Boolean          result;

    DXmCSTextPosition left_position, right_position; 
    DXmCSTextWidget   widget;
    void             LoseSel2();

    result = TRUE;
    widget = (DXmCSTextWidget) data->widget;

    DXmCSTextDisableRedisplay( widget, FALSE );

    /* if there is secondary selection, deselect it first
    */

    if ( data->hasSel2 )
    {
        left_position  = data->sel2Left;

	right_position = data->sel2Right;

        DXmCSTextSetHighlight( widget, 
			      left_position, 
			      right_position, 
			      XmHIGHLIGHT_NORMAL );
    }

    /* now try to secondary select from left to right
    */

    if ( left < right )
    {
        result = XtOwnSelection((Widget) widget,
				 XA_SECONDARY,
				 time,
				 (XtConvertSelectionProc)Convert,
				 (XtLoseSelectionProc) LoseSel2,
				 (XtSelectionDoneProc) NULL );

	data->hasSel2 = result;

        /* if sucessfully owned by widget, set to secondary selected
        */

  	if (result) {

	    DXmCSTextSetHighlight( widget,
				  left,
				  right,
				  XmHIGHLIGHT_SECONDARY_SELECTED );

	    data->sel2Left  = left;
	    data->sel2Right = right;
	}
    } else {

	data->hasSel2 = FALSE;

	if ( right != IN_LOSE_SELECTION )

	    XtDisownSelection( (Widget)widget, XA_SECONDARY, time );
    }

    DXmCSTextEnableRedisplay( widget );

    return result;
}


static void LoseSel2(w, selection)
Widget w;
Atom   *selection;
{

    DXmCSTextWidget    widget;
    DXmCSTextInputData data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    if ( data->hasSel2 )
    {
       (void) SetSel2( data, 1, IN_LOSE_SELECTION, CurrentTime );
    }
}
 

static Boolean GetSel2(data, left, right)
DXmCSTextInputData data;
DXmCSTextPosition  *left, *right; 
{
    if ( data->hasSel2 && data->sel2Left <= data->sel2Right )
    {
	*left  = data->sel2Left;
	*right = data->sel2Right;

	return TRUE;

    } else {

        data->hasSel2 = FALSE;
        return FALSE;
    }
}


static void SetSelectionHint(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* update the selection hint
    */
    data->selectionHint.x = event->xbutton.x;
    data->selectionHint.y = event->xbutton.y;
}


/*
 * This routine implements multi-click selection in a hardwired manner.
 * It supports multi-click entity cycling (char, word, line, file) and mouse
 * motion adjustment of the selected entities (i.e. select a word then, with
 * button still down, adjust which word you really meant by moving the mouse).
 * [Note: This routine is to be replaced by a set of procedures that
 * will allows clients to implements a wide class of draw through and
 * multi-click selection user interfaces.]
 */
static void a_Selection(widget, x, y, time)
DXmCSTextWidget widget;
int            x, y;
Time           time;
{
    DXmCSTextInputData data;
    DXmCSTextPosition  xy_position, 
		      left_position, 
		      right_position,
	              updleft, 
		      updright;
    Boolean           can_select = True;

    data = InputData (widget);

    DXmCSTextDisableRedisplay( widget, FALSE );

    /* convert x, y to position on screen
    */
    xy_position = OutputXYToPos ( widget, x, y );

    /* now find out the range to be selected (enclosed by left_ and right_
       position
    */

    switch ( data->stype )
    {
       case DXmstPositions :

            left_position  = xy_position;
            right_position = xy_position;

            break;

       case DXmstWordBreak :

	    /* find start and end position for the word the pointer is on
	    */
	    can_select = _DXmCSTextScanWordLimits( widget,
					 	   xy_position,
					 	   &left_position,
					           &right_position );
            break;

       case DXmstEOL :

	    /* scan to start of line
	    */
	    _DXmCSTextScanStartOfLine( widget, 
			               xy_position, 
			               &left_position );

	    /* scan to end of line
	    */
	    _DXmCSTextScanEndOfLine( widget, 
			             xy_position, 
			             &right_position );

            break;

       case DXmstAll :

    	    /* scan for the beginning of the file
            */
            _DXmCSTextScanAll( widget, xy_position, DXmsdLeft, &left_position );

	    /* scan for the beginning of the file
    	    */
    	    _DXmCSTextScanAll( widget, xy_position, DXmsdRight, &right_position );

            break;
       default :

	    left_position  =
	    right_position = xy_position;

            break;
    }
      
    /* can we select?  when double clicked on a white space, don't select
    */

    if ( can_select )
    {
       /* at this point, the left_position and right_positin will contain
          the text to be selected
       */
   
       /* select from left_position to right_position
       */
   
       DXmCSTextSetSelection( (Widget) widget, left_position, right_position, time );
   
       data->pendingoff = FALSE;
       data->anchor     = left_position;
   
       /* update cursor position and extendDir
       */
       if ( xy_position - left_position < right_position - xy_position )
       {
   	DXmCSTextSetCursorPosition( widget, left_position ); 
   	data->extendDir = DXmsdLeft;
   
       } else {
   
   	DXmCSTextSetCursorPosition( widget, right_position ); 
   	data->extendDir = DXmsdRight;
       }

       DXmCSTextShowPosition( widget, (DXmCSTextPosition) -1 );
   
       DXmCSTextEnableRedisplay( widget );
   
       data->origLeft  = left_position;
       data->origRight = right_position;

    } else {

       /* to reset the disable_redisplay flag
        */
       DXmCSTextEnableRedisplay( widget );
    }
}

/*
 * This Action Proc selects all of the text.
 */
static void SelectAll(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    Position x, y;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    OutputPosToXY ( widget, CursorPos (widget), &x, &y );

    /* select all
    */
    data->stype = DXmstAll;

    a_Selection( widget, x, y, event->xkey.time );
}


static void ClearSelection(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextPosition  cursor_position, left_position, right_position;
    DXmCSTextStatus    status;
    char              *selected_str;
    int		      length, i;
    char 	      *spaces;

    widget = (DXmCSTextWidget) w;


    if ( !CSTextHasSelection (widget) )
    {
        RingBell( widget, event, params, num_params );
        return;

    } else {

        left_position  = CSTextSelLeft (widget);
        right_position = CSTextSelRight(widget);

        /* replace selected text with spaces
        */
        if ( left_position < right_position)
        {
           /* get current cursor position
           */
           cursor_position = DXmCSTextGetCursorPosition( widget );

           if ( DeleteOrKill( widget, left_position, right_position, False ) )
           {
	      XmString cs;

              /* delete successful, replace selected text with spaces
              */
              length = right_position - left_position;

              spaces = (char *)XtMalloc( length + 1 );

              for (i = 0; i < length; i++) 
                  spaces[i] = ' ';

              spaces[length] = '\0';

	      cs = XmStringCreateSimple(spaces);

	      /*
	      ** Stop bell from ringing twice...
	      ** 12/08/93, JH
	      */
              I_STR_Insert( widget, cs, left_position );

              XtFree( spaces );
	      XmStringFree( cs );
           }
        }
    }
}


static void SetAnchor(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  cursor_position;
  
    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    data->anchor    =
    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* select the current cursor position
    */
    DXmCSTextSetSelection( (Widget) widget, 
			  cursor_position, 
			  cursor_position,
			  event->xkey.time );
}


/*
 * This Action Proc deselects all of the text.
 */
static void DeselectAll(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  cursor_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);
    cursor_position = DXmCSTextGetCursorPosition( widget );

    data->stype = DXmstPositions;

    /* select the current cursor position
    */
    DXmCSTextSetSelection( (Widget) widget, 
			  cursor_position, 
			  cursor_position,
			  event->xkey.time );

    /* anchor the current cursor position
    */
    data->anchor = cursor_position;
}

static void DoSelection(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    /* do the selection according to the data->stype
    */
    a_Selection( w, event->xbutton.x, event->xbutton.y, event->xbutton.time );
}


static void SetScanType(data, event)
DXmCSTextInputData data;
XEvent            *event;
{						/* %%% HARDCODED */
    int i;

    /* check to see if time difference between 2 consecutive clicks is 
       less than the current multi-click time
    */
    if ( event->xbutton.time > data->lasttime &&
	 event->xbutton.time - data->lasttime <=
	                          XtGetMultiClickTime(event->xbutton.display) )
    {
        /* find the current scan type
        */
	for ( i = 0;
	      i < data->sarraycount && data->sarray[i] != data->stype ;
	      i++ ) ;

        /* increment scantype pointer.  if out of bound, force it to
           DXmstPositions.
        */
	if ( ++i >= data->sarraycount ) 

           i = 0;

        /* update the scan type for the current click
        */
	data->stype = data->sarray[i];

    } else {

        /* single-click event, i.e. set it to DXmstPositions
         */
	data->stype = data->sarray[0];
    }

    /* store current time
    */
    data->lasttime = event->xbutton.time;
}


/*
 * This action proc gains ownership of the secondary selection
 * and initializes the secondary selection data for performing
 * a quick cut.
 */
static void SetQuickCut(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    Boolean           result;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* set ownership %%hack : don't know the impact if using widget instead
       of data->widget here.
    */
    result = XtOwnSelection( (Widget)data->widget, 
			     XA_SECONDARY, 
			     event->xkey.time,
			     (XtConvertSelectionProc)Convert, 
			     (XtLoseSelectionProc) LoseSel2, 
			     (XtSelectionDoneProc) NULL );

    data->hasSel2 = result;

    /* if succesfully owned, update pointers
    */
    if (result) {
        data->sel2Left     = 
	data->sel2Right    =
        data->Sel2OrigLeft =  DXmCSTextGetCursorPosition( (DXmCSTextWidget) widget );
    }

    data->quick_key     = TRUE;
    data->selectionMove = TRUE;
}


/*
 * This action proc gains ownership of the secondary selection
 * and initializes the secondary selection data for performing
 * a quick copy.
 */
static void SetQuickCopy(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    Boolean           result;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* set ownership %%hack : don't know the impact if using widget instead
       of data->widget here.
    */
    result = XtOwnSelection( (Widget)data->widget,
			     XA_SECONDARY,
			     event->xkey.time,
			     (XtConvertSelectionProc)Convert,
			     (XtLoseSelectionProc) LoseSel2,
			     (XtSelectionDoneProc) NULL );

    data->hasSel2 = result;

    /* if successfully owned, update pointers
    */
    if (result)
    {
        data->sel2Left     = 
	data->sel2Right    =
        data->Sel2OrigLeft =  DXmCSTextGetCursorPosition( (DXmCSTextWidget) widget );
    }

    data->quick_key     = TRUE;
    data->selectionMove = FALSE;
}




/* send a client message to perform the quick cut/copy and paste
*/
static void SecondaryNotify(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    XClientMessageEvent cm;
    int                 revert;
    DXmCSTextWidget     widget;
    Window		focus_window;

    widget = (DXmCSTextWidget) w;

    XGetInputFocus( XtDisplay( widget ), &focus_window, &revert );

    /* set up the client message
    */
    cm.type         = ClientMessage;
    cm.display      = XtDisplay( widget );
    cm.message_type = XInternAtom( XtDisplay( widget ),
				   "STUFF_SELECTION",
				   FALSE );
    cm.window       = focus_window;
    cm.format       = 32;
    cm.data.l[0]    = XA_SECONDARY;
    cm.data.l[1]    = event->xbutton.time;

    /* send the client message event
    */
    XSendEvent( XtDisplay( widget ), cm.window, TRUE, NoEventMask, (XEvent*)&cm );
}


static void StartPrimary(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{                                              
    DXmCSTextPosition  xy_position;
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    char              *selected_str;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* update the selection hint
    */
    SetSelectionHint( w, event, params, num_params );

    /* find out the scan type for the current click
    */
    SetScanType( data, event );

    if ( data->stype != DXmstPositions || 
         data->stype != data->sarray[0] )
    {
        /* scan type must be wordbreak, EOL or ALL.  Do the selection.
        */
        DoSelection( (Widget) widget, event, params, num_params );
    } else {

        /* it is a first single click
        */

        /* if text is selected, deselect it
        */
        if ( CSTextHasSelection(widget) )
        {
            xy_position = OutputXYToPos ( widget, event->xbutton.x,
					          event->xbutton.y );

            /* deselect it first
            */
            DXmCSTextSetSelection( (Widget) widget, 0, 0, event->xkey.time );
	    data->anchor = xy_position;
        }
    }
}



/*
 * This action proc. performs a quick copy/cut from the quick anchor
 * point (denoted by data->Sel2OrigLeft) to the current insert cursor
 * position.
 */
static void DoQuickAction(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  cursor_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);
    cursor_position = DXmCSTextGetCursorPosition( widget );

    /* if there is secondary selection, update and send secondary notify
       client message
    */
    if ( data->hasSel2 )
    {
       /* set the secondary left right
       */
       if ( data->Sel2OrigLeft < cursor_position )
       {
          data->sel2Left  = data->Sel2OrigLeft;
          data->sel2Right = cursor_position;

       } else if ( data->Sel2OrigLeft > cursor_position ) {

          data->sel2Left  = cursor_position;
          data->sel2Right = data->Sel2OrigLeft;
       }

       DXmCSTextSetHighlight( data->widget,
			     data->sel2Left,
			     data->sel2Right,
			     XmHIGHLIGHT_SECONDARY_SELECTED );

       data->stuffpos = cursor_position;

       if ( !CSTextHasFocus (widget) ||
	    !XtSensitive (widget) )
       {
       	    SecondaryNotify( (Widget) widget, event, params, num_params );
       } else {
	    Stuff ( (Widget) widget, event, params, num_params );
       }
    }
}

 

static void StartSecondary( w, event, params, num_params )
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{                                              
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    data->Sel2Hint.x    = event->xbutton.x;
    data->Sel2Hint.y    = event->xbutton.y;
    data->selectionMove = FALSE;
}


/*ARGSUSED*/
static void DoGrabFocus(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    if ( _XmGetFocusPolicy( w ) == XmPOINTER ||
         XmProcessTraversal( w, XmTRAVERSE_CURRENT ) )
    { 
	SetCursorPosition( w, event, params, num_params );
    }

    StartPrimary( w, event, params, num_params );

}


static void 
#ifdef _NO_PROTO
StartDrag( w, event, params, num_params )
        Widget w ;
        XEvent *event ;
        String *params ;
        Cardinal *num_params ;
#else
StartDrag(
        Widget w,
        XEvent *event,
        String *params,
        Cardinal *num_params )
#endif /* _NO_PROTO */
{                                              
    DXmCSTextWidget widget = (DXmCSTextWidget) w;
    Cardinal num_targets = 0;
    Atom targets[5];
    Widget drag_icon;
    int status = 0;
    Arg args[10];
    int n = 0;

    targets[num_targets++] = XmInternAtom(XtDisplay(w), "DDIF", False);
    targets[num_targets++] = XmInternAtom(XtDisplay(w), "COMPOUND_TEXT", False);
    targets[num_targets++] = XA_STRING;

    drag_icon = _XmGetTextualDragIcon(w);

    n = 0;
    XtSetArg(args[n], XmNcursorBackground, widget->core.background_pixel);  n++;
    XtSetArg(args[n], XmNcursorForeground, widget->primitive.foreground);  n++;
    XtSetArg(args[n], XmNsourceCursorIcon, drag_icon);  n++;
    XtSetArg(args[n], XmNexportTargets, targets);  n++;
    XtSetArg(args[n], XmNnumExportTargets, num_targets);  n++;
    XtSetArg(args[n], XmNconvertProc, _DXmCSTextSrcConvert);  n++;
    XtSetArg(args[n], XmNclientData, w);  n++;

    if (CSTextEditable(widget))
	XtSetArg(args[n], XmNdragOperations, (XmDROP_MOVE | XmDROP_COPY));
    else
	XtSetArg(args[n], XmNdragOperations, XmDROP_COPY);

    n++;

    XmDragStart(w, event, args, n);
}


/* ARGSUSED */
static void
#ifdef _NO_PROTO
ProcessBDrag( w, event, params, num_params )
        Widget w ;
        XEvent *event ;
        char **params ;
        Cardinal *num_params ;
#else
ProcessBDrag(
        Widget w,
        XEvent *event,
        char **params,
        Cardinal *num_params )
#endif /* _NO_PROTO */
{
    DXmCSTextWidget widget = (DXmCSTextWidget) w;
    DXmCSTextPosition position, left, right;
    Position left_x, left_y, right_x, right_y;
    DXmCSTextInputData data = InputData(widget);

    position = OutputXYToPos (widget, event->xbutton.x, event->xbutton.y);

    if (_DXmCSTextGetSelection(widget, &left, &right) && (right != left)) 
    {
	if (position > left && position < right ||
	   (position == left && OutputPosToXY(widget, left, &left_x, &left_y) &&
	      event->xbutton.x > left_x) ||
           (position == right && OutputPosToXY(widget, right, &right_x, &right_y) &&
	      event->xbutton.x < right_x)) 
	{
	    StartDrag(w, event, params, num_params);
	} 
	else 
	{
	    XAllowEvents(XtDisplay(w), AsyncBoth, event->xbutton.time);
	    StartSecondary(w, event, params, num_params);
	}
    } 
    else 
    {
	XAllowEvents(XtDisplay(w), AsyncBoth, event->xbutton.time);
	StartSecondary(w, event, params, num_params);
    }
}


/*
 * This routine implements extension of the currently selected text in
 * the "current" mode (i.e. char word, line, etc.). It worries about
 * extending from either end of the selection and handles the case when you
 * cross through the "center" of the current selection (e.g. switch which
 * end you are extending!).
 * [NOTE: This routine will be replaced by a set of procedures that
 * will allows clients to implements a wide class of draw through and
 * multi-click selection user interfaces.]
*/
static Boolean dragged(selectionHint, event, threshold)
DXmCSTextSelectionHint selectionHint;
XEvent        *event;
int           threshold;
{
    int xdiff, ydiff;

    xdiff = abs( selectionHint.x - event->xbutton.x );
    ydiff = abs( selectionHint.y - event->xbutton.y );

    if ( ( xdiff > threshold ) || ( ydiff > threshold ) )
    {
        return TRUE;

    } else {

	return FALSE;
    }
}



static void ExtendSelection(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  xy_position, left, right, cursor_position;
    char              *selected_str;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    if (data->cancel)
	return;

    /* if non-zero selectionhints, do selection if dragged 
    */
    if ( data->selectionHint.x || data->selectionHint.y )
    {
        if ( !dragged( data->selectionHint, event, data->threshold ) )
        {
	    return;
        }

        /* do selection
        */
	a_Selection( widget,
		     data->selectionHint.x,
		     data->selectionHint.y,
		     event->xbutton.time );

	data->selectionHint.x = 
        data->selectionHint.y = 0;
    } 

    data->extending = TRUE;

    /* not selected, update 
    */
    if ( !CSTextHasSelection (widget) )
    {
	data->origLeft  = 
        data->origRight = 
        data->anchor    =
        left            = 
        right           = DXmCSTextGetCursorPosition( widget );

	if ( data->stype != DXmstPositions )

	     data->stype  = data->sarray[0];
    } else {

       left  = CSTextSelLeft (widget);
       right = CSTextSelRight(widget);
    }

    /* find the last button position
    */

    xy_position = OutputXYToPos ( widget,
				  event->xbutton.x,
				  event->xbutton.y );

    /* check for change in extend direction 
    */
    if ( ( data->extendDir == DXmsdRight && xy_position < data->origLeft  ) ||
	 ( data->extendDir == DXmsdLeft  && xy_position > data->origRight ) )
    {
	data->extendDir =
	    ( data->extendDir == DXmsdRight ) ? DXmsdLeft : DXmsdRight;

	left  = data->origLeft;
	right = data->origRight;
    }
	
    if ( data->extendDir == DXmsdRight )
    {
        cursor_position = right = xy_position;

    } else {

        cursor_position = left = xy_position;
    }

    /* avoid flickering effect
    */

    if ( left  != CSTextSelLeft (widget) || right != CSTextSelRight(widget) )
    {
       DXmCSTextDisableRedisplay( widget, FALSE );

       /* set selection from left to right (extended selection)
       */
       DXmCSTextSetSelection( (Widget) widget, left, right, event->xkey.time );

       data->pendingoff = FALSE;

       DXmCSTextSetCursorPosition( widget, cursor_position );

       DXmCSTextShowPosition( widget, (DXmCSTextPosition) -1 );

       DXmCSTextEnableRedisplay( widget );
    }

}


static void ExtendSecondary(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  xy_position, hint_position,  left, right;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    if (data->cancel)
	return;

    xy_position = OutputXYToPos ( widget, 
				  event->xbutton.x,
				  event->xbutton.y );

    /* if secondary selection started
    */
    if ( data->Sel2Hint.x || data->Sel2Hint.y )
    {
        /* if not dragged, return
        */
        if ( !dragged( data->Sel2Hint, event, data->threshold ) )

	     return;

        /* get the hint position for the secondary select
        */
	hint_position = OutputXYToPos ( widget,
				        data->Sel2Hint.x,
				        data->Sel2Hint.y );

        /* check the direction of the drag
        */
	if ( xy_position < hint_position )
        {
            data->Sel2Extending = SetSel2( data,
					   xy_position,
					   hint_position,
					   event->xbutton.time );

	    data->Sel2OrigLeft  = xy_position;
	    data->Sel2OrigRight = hint_position;
	    data->Sel2ExtendDir = DXmsdLeft;

	} else {

	    data->Sel2Extending = SetSel2( data,
					   hint_position,
					   xy_position,
					   event->xbutton.time );

            data->Sel2OrigLeft  = hint_position;
	    data->Sel2OrigRight = xy_position;
	    data->Sel2ExtendDir = DXmsdRight;
	}

	data->Sel2Hint.x = 
        data->Sel2Hint.y = 0;
    }

    /* if not secondary selection extending, return
    */
    if ( !data->Sel2Extending )

	 return;

    DXmCSTextDisableRedisplay( widget, FALSE );

    GetSel2( data, &left, &right );

    /* check for change in extend direction
    */
    if ( ( data->Sel2ExtendDir == DXmsdRight &&
	   xy_position < data->Sel2OrigLeft  )      ||
	 ( data->Sel2ExtendDir == DXmsdLeft  &&
	   xy_position > data->Sel2OrigRight ) )
    {
        if (data->Sel2ExtendDir == DXmsdRight)
        {
            data->Sel2ExtendDir = DXmsdLeft;
            left  = xy_position;
            right = data->Sel2OrigLeft;  /* the hint */

            /* reset the hint position here, take the "right" above as the
               temp position. Here the assumption is when direction ==
               left, the Sel2OrigRight is always the hint
            */
            data->Sel2OrigLeft  = data->Sel2OrigRight;
            data->Sel2OrigRight = right;
        
        } else {
            data->Sel2ExtendDir = DXmsdRight;
            left  = data->Sel2OrigRight; /* the hint */
            right = xy_position;

            /* reset the hint position here, take the "left" above as the
               temp position. Here the assumption is when direction ==
               right, the Sel2OrigLeft is always the hint
            */
            data->Sel2OrigRight = data->Sel2OrigLeft;
            data->Sel2OrigLeft  = left;
        }
    }

    if ( data->Sel2ExtendDir == DXmsdRight )
    {
         right = xy_position;
    } else {
         left  = xy_position;
    }	


    /* avoid flickering
    */
    
    if ( left  != data->sel2Left || right != data->sel2Right )
    {
       /* do the secondary selection
       */
       (void) SetSel2( data, left, right, event->xbutton.time );
    }

    DXmCSTextEnableRedisplay( widget );

}


static void ExtendEnd(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);


    if ( data->extending || 
         dragged( data->selectionHint, event, data->threshold ) )
    {
        ExtendSelection( (Widget) widget, event, params, num_params );

        /* update pointers
        */
        data->origLeft  = CSTextSelLeft (widget);
        data->origRight = CSTextSelRight(widget);
    }

    data->extending       = FALSE;
    data->selectionHint.x =
    data->selectionHint.y = 0;

    data->cancel = False;
}


/*ARGSUSED*/
static void DoStuff(w, closure, seltype, type, value, length, format)
Widget   w;
Opaque   closure;
Atom     *seltype;
Atom     *type;
char     *value;
int      *length;
int      *format;
{
    DXmCSTextWidget    widget = (DXmCSTextWidget) w;
    DXmCSTextInputData data;
    DXmCSTextPosition  old_length, new_length;
    long return_status;
    XmString cstring;
    int len;
    Time time;

    Atom requested_type = (Atom)closure;

    Atom XA_DDIF     = XInternAtom(XtDisplay(widget), "DDIF", FALSE);
    Atom XA_C_TEXT   = XInternAtom(XtDisplay(widget), "COMPOUND_TEXT", FALSE);
    Atom XA_C_STRING = XInternAtom(XtDisplay(widget), "DEC_COMPOUND_STRING",
                       FALSE);

    if (!CSTextHasFocus (widget) && _XmGetFocusPolicy(widget) == XmEXPLICIT)
     (void) XmProcessTraversal(widget, XmTRAVERSE_CURRENT);

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    old_length = CSTextLength (widget);

    /* if the type returned is not the type requested, then try the next
    *  type of interest
    */ 
    if ( *type != requested_type )
    {
	Atom next_type;

	if( requested_type == XA_C_STRING )
	{
	    /* failed to get compound string, so ask for DDIF
	    */
	    next_type = XA_DDIF;

	}else{
	if( requested_type == XA_DDIF )
	{
	    /* failed to get DDIFF, so ask for compound text
	    */
	    next_type = XA_C_TEXT;

	}else{
	if( requested_type == XA_C_TEXT )
	{
	    /* failed to get compound text, so ask for string (last chance)
	    */
	    next_type = XA_STRING;

	}else{
	    /* couldn't get data in any useful form so give up
	    */
            /*BOGUS don't beep
	    RingBell( w, (XEvent *) NULL, (char **) NULL, (Cardinal) 0 );
            */
	    return;
	}}}

	if( *seltype == XA_PRIMARY )
	{
	    time = data->ppastetime;
	}else{
	    time = data->spastetime;
	}

	/* try getting the value in the next useful type
	*/
	XtGetSelectionValue( (Widget) widget,
			     *seltype,
			     next_type,
			     (XtSelectionCallbackProc)DoStuff,
			     (Opaque) next_type,
			     time
			   );
	return;
    }

    if ( *length == 0 )
    {
       return;
    }

    /* if we get here, then we obtained the data in some useful type
    *  now convert whatever it is into compound string for insertion in source
    */
    return_status = DXmCvtStatusFail;

    if( *type == XA_C_STRING )
    {
	cstring = (XmString)value;
	len = *length;
	return_status = DXmCvtStatusOK;

    }else{
    if( *type == XA_DDIF )
    {
	cstring = DXmCvtDDIFtoCS( value, (long *) &len, &return_status);
	XtFree(value);

    }else{
    if( *type == XA_C_TEXT )
    {
	cstring = XmCvtCTToXmString( value );
	len = XmStringLength( cstring );
	return_status = DXmCvtStatusOK;
	XtFree(value);

    }else{
    if( *type == XA_STRING )
    {
	cstring = DXmCvtFCtoCS( value, (long *) length, &return_status);
	XtFree(value);

    }else{
	/* shouldn't ever get here
	*/
	return;
    }}}}

    if( return_status != DXmCvtStatusOK )
    {
        RingBell( w, (XEvent *) NULL, (char **) NULL, (Cardinal) 0 );
	return;
    } 

#ifdef DEC_MOTIF_BUG_FIX
    /* don't exceed max length limit. 5-13-93 sp */
    if( old_length + len > CSTextMaxLength(widget) )
    {
        RingBell( w, (XEvent *) NULL, (char **) NULL, (Cardinal) 0 );
	return;
    } 
#endif

    /*
    ** Stop bell from ringing twice...
    ** 12/08/93, JH
    */
    if( I_STR_Insert( widget, cstring, data->stuffpos ) )
    {
	/* BOGUS Any better way??  update the selection position
	*/
	new_length = CSTextLength (widget);

	/* the adjustment for primary highlight pointers will be handled
	*  by CSTEXTSTR.  So do the adjustment for 2ndary selection only
	*/
	if ( data->stuffpos <= data->sel2Left )
	{
	     if ( data->sel2Left != data->sel2Right )
	     {
		data->sel2Left  += (new_length - old_length);
		data->sel2Right += (new_length - old_length);
	     }
	}            
    }

    XmStringFree( cstring );
    return;
}


/*ARGSUSED*/
static void Stuff(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    Atom               XA_I_STRING;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    XA_I_STRING = XInternAtom( XtDisplay( widget ), "DDIF", FALSE );    

    /* secondary selection has the priority over Stuff.
    *  if the CSText has focus, select it as secondary else
    *  treat it as primary so that the focused window can quick copy it
    */
    if ( data->hasSel2 )
    {
      data->spastetime = event->xbutton.time;
      data->sselformat = FMT_I_STRING;

      XtGetSelectionValue( (Widget) widget,
			   XA_SECONDARY,
			   XA_I_STRING,
			   (XtSelectionCallbackProc)DoStuff,
			   (Opaque) XA_I_STRING,
			   event->xbutton.time );
    } else {

      if ( CSTextHasSelection (widget) )
      {
           data->ppastetime = event->xbutton.time;
           data->pselformat = FMT_I_STRING;
      }

      XtGetSelectionValue( (Widget) widget,
			   XA_PRIMARY,
			   XA_I_STRING,
			   (XtSelectionCallbackProc)DoStuff,
			   (Opaque) XA_I_STRING,
			   event->xbutton.time );
   }
}


/*
 * This function set the final position of the secondary selection and
 * calls SecondaryNotify().
 */
static void ExtendSecondaryEnd(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    Boolean           primary_paste;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    primary_paste = True;

    /* if it is a secondary selection, reset highlight
    */
    if ( (data->Sel2Extending ||
	  dragged(data->Sel2Hint, event, data->threshold)) && !data->cancel )
    {

      /* set the final position of the secondary selection
      */
      ExtendSecondary( (Widget) widget, event, params, num_params );
      GetSel2( data, &(data->Sel2OrigLeft), &(data->Sel2OrigRight) );

      if ( data->sel2Left < data->sel2Right )
      {
          DXmCSTextSetHighlight( widget, 
			         data->sel2Left, 
			         data->sel2Right, 
			         XmHIGHLIGHT_NORMAL );
          }


       /* doing quick paste instead of primary paste */
        primary_paste = False;
    }


    /* Re-initialize the secondary selection data
    */
    data->Sel2Extending = FALSE;
    data->Sel2Hint.x    =
    data->Sel2Hint.y    = 0;
    data->stuffpos      = DXmCSTextGetCursorPosition(widget);

    /* If not doing primary paste, do secondary selection notification.
    */
    if ( !primary_paste )
    {
       if ( !CSTextHasFocus (widget) ||
	    !XtSensitive (widget) )
       {
       	    SecondaryNotify( (Widget) widget, event, params, num_params );
       } else {
	    Stuff ( (Widget) widget, event, params, num_params );
       }
    }

    data->cancel = False;
}


static void ExtendSecondaryEndAndDelete(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    Boolean           is_dragged;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    is_dragged = dragged( data->Sel2Hint, event,data->threshold );

    if ( data->Sel2Extending || is_dragged )
    {
	data->selectionMove = TRUE;
	ExtendSecondaryEnd( (Widget) widget, event, params, num_params );
    }
}


static void ProcessCopy(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;
    DXmCSTextPosition  xy_position;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    if (dragged(data->Sel2Hint, event, data->threshold)) {
       if (data->Sel2Extending) {
         /*
          * Causes the converter to perform a delete action of the
          * secondary selection when the Convert routine is called.
          */
	  ExtendSecondaryEnd(w, event, params, num_params);
       }
    } else {
      /*
       * Copy contents of primary selection to the stuff position found above.
       */

       data->stuffpos = OutputXYToPos ( widget, 
				        event->xbutton.x,
				        event->xbutton.y );

       Stuff(w, event, params, num_params);
    }
}	



static void ProcessMove(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    XClientMessageEvent cm;
    DXmCSTextWidget      widget;
    DXmCSTextInputData   data;


    widget = (DXmCSTextWidget) w;
    data = InputData (widget);

    ProcessCopy( w, event, params, num_params );

    cm.type         = ClientMessage;
    cm.display      = XtDisplay(widget);
    cm.message_type = XInternAtom( XtDisplay(widget), "KILL_SELECTION", FALSE );
    cm.window       = XGetSelectionOwner( XtDisplay( widget ), XA_PRIMARY );
    cm.format       = 32;
    cm.data.l[0]    = XA_PRIMARY;

    XSendEvent( XtDisplay( widget ), cm.window, TRUE, NoEventMask, (XEvent*)&cm );
}	



/* This function does a primary copy and paste on keyboard actions. 
*/
static void CopyPrimary(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    DXmCSTextWidget    widget;
    DXmCSTextInputData data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* Set the stuff position to the current cursor position
    */
    data->stuffpos = DXmCSTextGetCursorPosition( widget );

    /* perform the primary paste action
    */
    Stuff( w, event, params, num_params );
}



/* This function does a primary cut and paste on keyboard actions. 
*/
static void CutPrimary(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    XClientMessageEvent cm;
    DXmCSTextWidget      widget;
    DXmCSTextInputData   data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    /* Set the stuff position to the current cursor position
    */
    data->stuffpos = DXmCSTextGetCursorPosition( widget );

    /* perform the primary paste action
    */
    Stuff( (Widget)widget, event, params, num_params );

   /*
    * Send a client messsage to the owner of the primary selection to
    * delete the primary selection to complete the cut action.
    */
    cm.type         = ClientMessage;
    cm.display      = XtDisplay( w );
    cm.message_type = XmInternAtom( XtDisplay(w), "KILL_SELECTION", FALSE );
    cm.window       = XGetSelectionOwner(XtDisplay(w), XA_PRIMARY);
    cm.format       = 32;
    cm.data.l[0]    = XA_PRIMARY;
    cm.data.l[1]    = event->xbutton.time;

    XSendEvent( XtDisplay( w ), cm.window, TRUE, NoEventMask, (XEvent*)&cm );
}


static void CutClipboard(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
   DXmCSTextCut( w, event->xkey.time );
}

static void CopyClipboard(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
   DXmCSTextCopy( w, event->xkey.time );
}

static void PasteClipboard(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
   DXmCSTextPaste( (DXmCSTextWidget) w );
}
    

static void RemoteKillSelection(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
/* just knows about primary selection for now */

    DXmCSTextPosition    dumpos;
    XClientMessageEvent  *cm;
    DXmCSTextWidget      widget;

    widget = (DXmCSTextWidget) w;
    cm     = (XClientMessageEvent *) event;

    if( cm->message_type !=
              XInternAtom( XtDisplay( widget ), "KILL_SELECTION", FALSE ) )
    {
        return;
    }

    if ( !CSTextHasSelection (widget) )
    {
        RingBell( (Widget)widget, event, params, num_params );

    } 
    else 
      {
        DXmCSTextDisableRedisplay ( widget, True );

	_DXmCSTextSourceReplaceString( (DXmCSTextWidget)widget,
			               CSTextSelLeft(widget),
			               CSTextSelRight(widget),
			               XmStringCreateSimple("") );
        DXmCSTextEnableRedisplay ( widget );

      }
}


static void StuffSecondary(w, event, params, num_params)
Widget   w;
XEvent   *event;
char     **params;
Cardinal *num_params;
{
    Atom                XA_I_STRING;
    XClientMessageEvent *cm;
    DXmCSTextWidget      widget;
    DXmCSTextInputData   data;

    widget = (DXmCSTextWidget) w;
    data   = InputData (widget);

    XA_I_STRING = XInternAtom( XtDisplay( widget ), "DDIF", FALSE );

    cm = (XClientMessageEvent *) event;

    if( cm->message_type != 
		XInternAtom( XtDisplay( widget ), "STUFF_SELECTION", FALSE ) )
    {
	return;
    }

    data->stuffpos   = DXmCSTextGetCursorPosition( widget );
    data->spastetime = cm->data.l[1];
    data->sselformat = FMT_I_STRING;

    XtGetSelectionValue((Widget) widget,
				 XA_SECONDARY,
				 XA_I_STRING,
				 (XtSelectionCallbackProc)DoStuff,
				 (Opaque) XA_I_STRING,
				 cm->data.l[1] );
}


static
Boolean _NavNextSegment( location )
TextLocation location;
{
  TextSegment seg;
  Boolean ok = TRUE;

  for (seg = location->segment;
       (seg == location->segment) && ok;
       ok = _DXmCSTextNavNextChar (location))
    ;

  return (ok);
}

/* F17 toggles editing path and sets cursor to next segment with same direction.
 * Shift-F17 stays put.
 */
static void  GetNextDirectionPos( widget, direction, position, new_position) 
DXmCSTextWidget widget;
XmStringDirection direction;
DXmCSTextPosition position;
DXmCSTextPosition *new_position;
{
    TextLocationRec location ;
    TextLine        line ;

    _DXmCSTextSourceLocate( widget, position, &location) ;
    line = location.line ;
    
    do{
         if (location.segment == NULL ||    /* end of line */
             line != location.line   ||  
             location.segment->direction == direction ) 
                 break ;
      } while ( _NavNextSegment( &location ) ) ;
    
    *new_position = location.position ;
}
       
static void ToggleEditingPathMove(widget, event, params, num_params)
DXmCSTextWidget widget;
XEvent         *event;
char           **params;
Cardinal       *num_params;
{
/* KREMER: temppos not used !?*/
    DXmCSTextPosition temppos;
    DXmCSTextPosition cursor_position;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    if ( CSTextEditable (widget) != TRUE )
    {
        RingBell( widget, event, params, num_params );
	return;
    }

    CSTextEditingPath (widget) = ( CSTextEditingPath (widget) == 0 ) ? 1 : 0;

    GetNextDirectionPos(  widget, 
                          CSTextEditingPath( widget ), 
                          cursor_position,
                          &cursor_position 
                        ) ;
    DXmCSTextSetCursorPosition( widget, cursor_position );
}

static void ToggleEditingPath(widget, event, params, num_params)
DXmCSTextWidget widget;
XEvent         *event;
char           **params;
Cardinal       *num_params;
{
/* KREMER: temppos not used !?*/
    DXmCSTextPosition temppos;
    DXmCSTextPosition cursor_position;

    cursor_position = DXmCSTextGetCursorPosition( widget );

    if ( CSTextEditable (widget) != TRUE )
    {
        RingBell( widget, event, params, num_params );
	return;
    }

    CSTextEditingPath (widget) = ( CSTextEditingPath (widget) == 0 ) ? 1 : 0;

    DXmCSTextMarkRedraw( widget, cursor_position, cursor_position );
}




static void InverseWidget(widget, event, params, num_params)
DXmCSTextWidget widget;
XEvent         *event;
char           **params;
Cardinal       *num_params;
{

   /* Toggling the TextPath will inverse the widget (mirror image)  
   */
   TextLine    line     = _DXmCSTextGetFirstLine (widget);
   TextSegment segment ;

   DXmCSTextDisableRedisplay( widget, TRUE );

   CSTextPath (widget) = (CSTextPath (widget) == 0 ) ? 1 : 0 ;

   /* This code appears in module CSTEXT.C as well
   */
   /* Toggle EditingPath as well */
   CSTextEditingPath (widget) = (CSTextEditingPath (widget) == 0) ? 1 : 0;

   /*
   We will toggle the directions of all segments. No need to reverse 
   text for visual order since we will be creating a mirror image. 
   */
   while (line != (TextLine)NULL)
   {
          segment = _DXmCSTextGetFirstSegment(line);
         while (segment != (TextSegment)NULL)
         {
                  segment->direction =
                      ( segment->direction == 0 ) ? 1 : 0;
                  segment = _DXmCSTextGetNextSegment( segment ) ;
         }
         line = _DXmCSTextGetNextLine( line );
   }
       
   /* Recalculate all the output segments from scratch
   */
   OutputHandleData (widget, 
	      NULL, NULL,      /* not used */
                     DXmCSTextOutputDelete) ;

   OutputRedisplayHBar (widget) ;

   DXmCSTextSetCursorPosition( widget, CursorPos ( widget ) );
   DXmCSTextEnableRedisplay( widget );
}


/* DON'T conditionalize this routine by I18N_IO_SHR
 */
static void ChangeInputMethod(widget, event)
Widget widget;
XEvent         *event;
{
    DXmCSTextWidget w = (DXmCSTextWidget) widget;

    OutputChangeInputMethod (w, event);
}




static XtActionsRec ZdefaultCSTextActionsTable[] = {

/* Insert bindings */
  {"self-insert",		(XtActionProc)SelfInsert},
  {"insert-string",		(XtActionProc)InsertString},
/* Motion bindings */
  {"grab-focus",		(XtActionProc)DoGrabFocus},
  {"set-insertion-point",	(XtActionProc)SetCursorPosition},
  {"forward-character", 	(XtActionProc)MoveForwardChar},
  {"backward-character", 	(XtActionProc)MoveBackwardChar},
  {"forward-word", 		(XtActionProc)MoveForwardWord},
  {"backward-word", 		(XtActionProc)MoveBackwardWord},
  {"forward-paragraph", 	(XtActionProc)MoveForwardParagraph},
  {"backward-paragraph", 	(XtActionProc)MoveBackwardParagraph},
  {"beginning-of-line", 	(XtActionProc)MoveToLineStart},
  {"end-of-line", 		(XtActionProc)MoveToLineEnd},
  {"next-line", 		(XtActionProc)MoveNextLine},
  {"previous-line", 		(XtActionProc)MovePreviousLine},
  {"next-page", 		(XtActionProc)MoveNextPage},
  {"previous-page", 		(XtActionProc)MovePreviousPage},
  {"page-left", 		(XtActionProc)MovePageLeft},
  {"page-right", 		(XtActionProc)MovePageRight},
  {"beginning-of-file", 	(XtActionProc)MoveBeginningOfFile},
  {"end-of-file", 		(XtActionProc)MoveEndOfFile},
  {"move-destination",		(XtActionProc)MoveDestination},
  {"scroll-one-line-up", 	(XtActionProc)ScrollOneLineUp},
  {"scroll-one-line-down", 	(XtActionProc)ScrollOneLineDown},
  {"right-character",           (XtActionProc)MoveRightChar},
  {"left-character",            (XtActionProc)MoveLeftChar},
  {"right-word",                (XtActionProc)MoveRightWord},
  {"left-word",                 (XtActionProc)MoveLeftWord},
  {"extreme-left-of-file",      (XtActionProc)MoveFileExtremeLeft},
  {"extreme-right-of-file",     (XtActionProc)MoveFileExtremeRight},
  {"right-side-of-line",        (XtActionProc)MoveToLineRight},
  {"left-side-of-line",         (XtActionProc)MoveToLineLeft},
  {"page-horizontal-forward",   (XtActionProc)MovePageHorizontalForward},
  {"page-horizontal-backward",  (XtActionProc)MovePageHorizontalBackward},
  {"toggle-editing-path-move",  (XtActionProc)ToggleEditingPathMove},
  {"toggle-editing-path-stay",  (XtActionProc)ToggleEditingPath},
  {"toggle-text-path",          (XtActionProc)InverseWidget},
/* Delete bindings */
  {"delete-selection", 		(XtActionProc)DeleteCurrentSelection},
  {"delete-next-character", 	(XtActionProc)DeleteForwardChar},
  {"delete-previous-character",	(XtActionProc)DeleteBackwardChar},
  {"delete-next-word", 		(XtActionProc)DeleteForwardWord},
  {"delete-previous-word", 	(XtActionProc)DeleteBackwardWord},
  {"delete-to-end-of-line", 	(XtActionProc)DeleteToEndOfLine},
  {"delete-to-start-of-line",	(XtActionProc)DeleteToStartOfLine},
/* Kill bindings */
  {"kill-selection", 		(XtActionProc)KillCurrentSelection},
  {"kill-next-character", 	(XtActionProc)KillForwardChar},
  {"kill-previous-character",	(XtActionProc)KillBackwardChar},
  {"kill-next-word", 		(XtActionProc)KillForwardWord},
  {"kill-previous-word", 	(XtActionProc)KillBackwardWord},
  {"kill-to-end-of-line", 	(XtActionProc)KillToEndOfLine},
  {"kill-to-start-of-line",	(XtActionProc)KillToStartOfLine},
/* Unkill bindings */
  {"unkill", 			(XtActionProc)UnKill},
  {"stuff", 			(XtActionProc)Stuff},
/* New line bindings */
  {"newline-and-indent", 	(XtActionProc)InsertNewLineAndIndent},
  {"newline-and-backup", 	(XtActionProc)InsertNewLineAndBackup},
  {"newline",			(XtActionProc)InsertNewLine},
/* Selection bindings */
  {"select-all", 		(XtActionProc)SelectAll},
  {"deselect-all", 		(XtActionProc)DeselectAll},
  {"select-start", 		(XtActionProc)StartPrimary},
  {"quick-cut-set", 		(XtActionProc)VoidAction},
  {"quick-copy-set", 		(XtActionProc)VoidAction},
  {"do-quick-action", 		(XtActionProc)VoidAction},
  {"key-select", 		(XtActionProc)KeySelection},
  {"set-anchor", 		(XtActionProc)SetAnchor},
  {"select-adjust", 		(XtActionProc)DoSelection},
  {"select-end", 		(XtActionProc)DoSelection},
  {"extend-start", 		(XtActionProc)ExtendSelection},
  {"extend-adjust", 		(XtActionProc)ExtendSelection},
  {"extend-end", 		(XtActionProc)ExtendEnd},
  {"set-selection-hint",	(XtActionProc)SetSelectionHint},
  {"process-bdrag",		(XtActionProc)ProcessBDrag},
  {"secondary-start",		(XtActionProc)StartSecondary},
  {"secondary-adjust",		(XtActionProc)ExtendSecondary},
  {"secondary-notify",		(XtActionProc)SecondaryNotify},
  {"clear-selection",		(XtActionProc)ClearSelection},
  {"copy-to",			(XtActionProc)ProcessCopy},
  {"move-to",			(XtActionProc)ProcessMove},
  {"copy-primary",		(XtActionProc)CopyPrimary},
  {"cut-primary",		(XtActionProc)CutPrimary},
/* Clipboard bindings */
  {"copy-clipboard",		(XtActionProc)CopyClipboard},
  {"cut-clipboard",		(XtActionProc)CutClipboard},
  {"paste-clipboard",		(XtActionProc)PasteClipboard},
/* Miscellaneous bindings */
  {"beep", 			(XtActionProc)RingBell},
  {"redraw-display", 		(XtActionProc)RedrawDisplay},
  {"activate",			(XtActionProc)Activate},
  {"toggle-overstrike",		(XtActionProc)ToggleOverstrike},
  {"toggle-add-mode",		(XtActionProc)ToggleAddMode},
  {"Help",			(XtActionProc)Help}, /* vs _XmPrimitiveHelp */
  {"enter",                     (XtActionProc)_XmPrimitiveEnter},
  {"leave",			(XtActionProc)TextLeave},
  {"focusIn",			(XtActionProc)_XmPrimitiveFocusIn},
  {"focusOut",			(XtActionProc)TextFocusOut},
  {"unmap",			(XtActionProc)_XmPrimitiveUnmap},
/* Process multi-line and single line bindings */
  {"process-cancel",		(XtActionProc)ProcessCancel},
  {"process-return",		(XtActionProc)ProcessReturn},
  {"process-tab",		(XtActionProc)ProcessTab},
  {"process-up",		(XtActionProc)ProcessUp},
  {"process-down",		(XtActionProc)ProcessDown},
  {"process-shift-up",		(XtActionProc)ProcessShiftUp},
  {"process-shift-down",	(XtActionProc)ProcessShiftDown},
  {"process-home",		(XtActionProc)ProcessHome},
/* Traversal bindings*/
  {"traverse-next",		(XtActionProc)TraverseDown},
  {"traverse-prev",		(XtActionProc)TraverseUp},
  {"traverse-home",		(XtActionProc)TraverseHome},
  {"next-tab-group",		(XtActionProc)TraverseNextTabGroup},
  {"prev-tab-group",		(XtActionProc)TraversePrevTabGroup},
/* DON'T conditionalize this by I18N_IO_SHR.  This is the only extra action
 * item as compared to XmText
 */
  {"change-input-method",	(XtActionProc)ChangeInputMethod},
};


#ifdef I18N_IO_SHR

#ifdef VMS
externaldef(nonvisible)
XtPointer _defaultCSTextActionsTable = (XtPointer) ZdefaultCSTextActionsTable;

externaldef(nonvisible)
Cardinal _defaultCSTextActionsTableSize = XtNumber( ZdefaultCSTextActionsTable );
	  			/* %%%HACK */
#else
XtPointer defaultCSTextActionsTable = (XtPointer) ZdefaultCSTextActionsTable;

Cardinal defaultCSTextActionsTableSize = XtNumber( ZdefaultCSTextActionsTable );
	  			/* %%%HACK */
#endif /* VMS */

#else
externaldef(nonvisible)
XtPointer defaultCSTextActionsTable = (XtPointer) ZdefaultCSTextActionsTable;

Cardinal defaultCSTextActionsTableSize = XtNumber( ZdefaultCSTextActionsTable );
	  			/* %%%HACK */
#endif /* I18N_IO_SHR */


#ifdef DXmUNIX			/* Just a temp hack so I can test things.%%% */
#define EMACSISH
#endif

#ifndef WIN32
#ifdef I18N_IO_SHR
#ifdef ultrix
char defaultCSTextEventBindings[] =
#else /* ultrix */
externaldef(dxmtms) const char _defaultCSTextEventBindings[] =
#endif
#else   /* I18N_IO_SHR */
#ifdef ultrix
char defaultCSTextEventBindings[] =
#else /* ultrix */
externaldef(dxmtms) const char defaultCSTextEventBindings[] =
#endif
#endif /* I18N_IO_SHR */

"m <Key>osfPrimaryPaste:cut-primary()\n\
 a <Key>osfPrimaryPaste:cut-primary()\n\
 <Key>osfPrimaryPaste:copy-primary()\n\
 m <Key>osfCut:cut-primary()\n\
 a <Key>osfCut:cut-primary()\n\
 <Key>osfCut:cut-clipboard()\n\
 <Key>osfPaste:paste-clipboard()\n\
 m <Key>osfCopy:copy-primary()\n\
 a <Key>osfCopy:copy-primary()\n\
 <Key>osfCopy:copy-clipboard()\n\
 s c <Key>osfBeginLine:extreme-left-of-file(extend)\n\
 c <Key>osfBeginLine:extreme-left-of-file()\n\
 s <Key>osfBeginLine:left-side-of-line(extend)\n\
 <Key>osfBeginLine:left-side-of-line()\n\
 s c <Key>osfEndLine:extreme-right-of-file(extend)\n\
 c <Key>osfEndLine:extreme-right-of-file()\n\
 s <Key>osfEndLine:right-side-of-line(extend)\n\
 <Key>osfEndLine:right-side-of-line()\n\
 s <Key>osfPageLeft:page-left(extend)\n\
 <Key>osfPageLeft:page-left()\n\
 s c <Key>osfPageUp:page-left(extend)\n\
 c <Key>osfPageUp:page-horizontal-backward()\n\
 s <Key>osfPageUp:previous-page(extend)\n\
 <Key>osfPageUp:previous-page()\n\
 s <Key>osfPageRight:page-right(extend)\n\
 <Key>osfPageRight:page-right()\n\
 s c <Key>osfPageDown:page-right(extend)\n\
 c <Key>osfPageDown:page-horizontal-forward()\n\
 s <Key>osfPageDown:next-page(extend)\n\
 <Key>osfPageDown:next-page()\n\
 <Key>osfClear:clear-selection()\n\
 ~m ~a <Key>osfBackSpace:delete-previous-character()\n\
 s Meta<Key>osfDelete:cut-primary()\n\
 s Alt<Key>osfDelete:cut-primary()\n\
 c ~m ~a <Key>osfDelete:delete-to-end-of-line()\n\
 ~c ~m ~a <Key>osfDelete:delete-next-character()\n\
 c Meta<Key>osfInsert:copy-primary()\n\
 c Alt<Key>osfInsert:copy-primary()\n\
 s ~c ~m ~a <Key>osfInsert:paste-clipboard()\n\
 ~s c ~m ~a <Key>osfInsert:copy-clipboard()\n\
 ~s ~c ~m ~a <Key>osfInsert:toggle-overstrike()\n\
 s ~c <Key>osfSelect:key-select()\n\
 ~s <Key>osfSelect:set-anchor()\n\
 <Key>osfActivate:activate()\n\
 <Key>osfAddMode:toggle-add-mode()\n\
 <Key>osfHelp:Help()\n\
 <Key>osfCancel:process-cancel()\n\
 s c <Key>osfLeft:left-word(extend)\n\
 c <Key>osfLeft:left-word()\n\
 s <Key>osfLeft:key-select(left)\n\
 <Key>osfLeft:left-character()\n\
 s c <Key>osfRight:right-word(extend)\n\
 c <Key>osfRight:right-word()\n\
 s <Key>osfRight:key-select(right)\n\
 <Key>osfRight:right-character()\n\
 s c <Key>osfUp:backward-paragraph(extend)\n\
 c <Key>osfUp:backward-paragraph()\n\
 s <Key>osfUp:process-shift-up()\n\
 <Key>osfUp:process-up()\n\
 s c <Key>osfDown:forward-paragraph(extend)\n\
 c <Key>osfDown:forward-paragraph()\n\
 s <Key>osfDown:process-shift-down()\n\
 <Key>osfDown:process-down()\n\
 c ~m ~a <Key>slash:select-all()\n\
 c ~m ~a <Key>backslash:deselect-all()\n\
 s ~m ~a <Key>Tab:prev-tab-group()\n\
 c ~m ~a <Key>Tab:next-tab-group()\n\
 ~m ~a <Key>Tab:process-tab()\n\
 c ~s ~m ~a <Key>Return:activate()\n\
 ~s ~m ~a <Key>Return:process-return()\n\
 c ~s ~m ~a <Key>space:set-anchor()\n\
 c s ~m ~a <Key>space:key-select()\n\
 a ~c ~s ~Meta<Key>F17:toggle-text-path()\n\
 s ~a ~c ~Meta<Key>F17:toggle-editing-path-stay()\n\
 ~a ~c ~s ~Meta<Key>F17:toggle-editing-path-move()\n\
 s ~c ~m ~a <Key>space:self-insert()\n\
 <Key>:self-insert()\n\
 ~c  s ~m ~Alt<Btn1Down>:extend-start()\n\
  c ~s ~m ~Alt<Btn1Down>:move-destination()\n\
 ~c ~s ~m ~Alt<Btn1Down>:grab-focus()\n\
 ~c ~m ~Alt<Btn1Motion>:extend-adjust()\n\
 ~c ~m ~Alt<Btn1Up>:extend-end()\n\
 <Btn2Down>:process-bdrag()\n\
 ~c ~s  m ~Alt<Btn2Motion>:secondary-adjust()\n\
 ~c ~s ~m  Alt<Btn2Motion>:secondary-adjust()\n\
 ~s ~m ~Alt<Btn2Motion>:secondary-adjust()\n\
 ~c ~s  m ~Alt<Btn2Up>:move-to()\n\
 ~c ~s ~m  Alt<Btn2Up>:move-to()\n\
 ~s ~m ~Alt<Btn2Up>:copy-to()\n\
 <EnterWindow>:enter()\n\
 <LeaveWindow>:leave()\n\
 <FocusIn>:focusIn()\n\
 <FocusOut>:focusOut()\n\
 <Unmap>:unmap()";
#else

externaldef(dxmtms) const char defaultCSTextEventBindings1[] ="\
 m <Key>osfPrimaryPaste:cut-primary()\n\
 a <Key>osfPrimaryPaste:cut-primary()\n\
 <Key>osfPrimaryPaste:copy-primary()\n\
 m <Key>osfCut:cut-primary()\n\
 a <Key>osfCut:cut-primary()\n\
 <Key>osfCut:cut-clipboard()\n\
 <Key>osfPaste:paste-clipboard()\n\
 m <Key>osfCopy:copy-primary()\n\
 a <Key>osfCopy:copy-primary()\n\
 <Key>osfCopy:copy-clipboard()\n\
 s c <Key>osfBeginLine:extreme-left-of-file(extend)\n\
 c <Key>osfBeginLine:extreme-left-of-file()\n\
 s <Key>osfBeginLine:left-side-of-line(extend)\n\
 <Key>osfBeginLine:left-side-of-line()\n\
 s c <Key>osfEndLine:extreme-right-of-file(extend)\n\
 c <Key>osfEndLine:extreme-right-of-file()\n\
 s <Key>osfEndLine:right-side-of-line(extend)\n\
 <Key>osfEndLine:right-side-of-line()\n\
 s <Key>osfPageLeft:page-left(extend)\n\
 <Key>osfPageLeft:page-left()\n\
 s c <Key>osfPageUp:page-left(extend)\n\
 c <Key>osfPageUp:page-horizontal-backward()\n\
 s <Key>osfPageUp:previous-page(extend)\n\
 <Key>osfPageUp:previous-page()\n\
 s <Key>osfPageRight:page-right(extend)\n\
 <Key>osfPageRight:page-right()\n\
 s c <Key>osfPageDown:page-right(extend)\n\
 c <Key>osfPageDown:page-horizontal-forward()\n\
 s <Key>osfPageDown:next-page(extend)\n\
 <Key>osfPageDown:next-page()\n\
 <Key>osfClear:clear-selection()\n\
 ~m ~a <Key>osfBackSpace:delete-previous-character()\n\
 s Meta<Key>osfDelete:cut-primary()\n\
 s Alt<Key>osfDelete:cut-primary()\n\
 c ~m ~a <Key>osfDelete:delete-to-end-of-line()\n\
 ~c ~m ~a <Key>osfDelete:delete-next-character()\n\
 c Meta<Key>osfInsert:copy-primary()\n\
 c Alt<Key>osfInsert:copy-primary()\n\
 s ~c ~m ~a <Key>osfInsert:paste-clipboard()\n\
 ~s c ~m ~a <Key>osfInsert:copy-clipboard()\n\
 ~s ~c ~m ~a <Key>osfInsert:toggle-overstrike()\n\
 s ~c <Key>osfSelect:key-select()\n\
 ~s <Key>osfSelect:set-anchor()\n\
 <Key>osfActivate:activate()\n\
 <Key>osfAddMode:toggle-add-mode()\n\
 <Key>osfHelp:Help()\n\
 <Key>osfCancel:process-cancel()\n\
 s c <Key>osfLeft:left-word(extend)\n\
 c <Key>osfLeft:left-word()\n\
 s <Key>osfLeft:key-select(left)\n\
 <Key>osfLeft:left-character()\n";

 externaldef(dxmtms2) const char defaultCSTextEventBindings2[] ="\
 s c <Key>osfRight:right-word(extend)\n\
 c <Key>osfRight:right-word()\n\
 s <Key>osfRight:key-select(right)\n\
 <Key>osfRight:right-character()\n\
 s c <Key>osfUp:backward-paragraph(extend)\n\
 c <Key>osfUp:backward-paragraph()\n\
 s <Key>osfUp:process-shift-up()\n\
 <Key>osfUp:process-up()\n\
 s c <Key>osfDown:forward-paragraph(extend)\n\
 c <Key>osfDown:forward-paragraph()\n\
 s <Key>osfDown:process-shift-down()\n\
 <Key>osfDown:process-down()\n\
 c ~m ~a <Key>slash:select-all()\n\
 c ~m ~a <Key>backslash:deselect-all()\n\
 s ~m ~a <Key>Tab:prev-tab-group()\n\
 c ~m ~a <Key>Tab:next-tab-group()\n\
 ~m ~a <Key>Tab:process-tab()\n\
 c ~s ~m ~a <Key>Return:activate()\n\
 ~s ~m ~a <Key>Return:process-return()\n\
 c ~s ~m ~a <Key>space:set-anchor()\n\
 c s ~m ~a <Key>space:key-select()\n\
 a ~c ~s ~Meta<Key>F17:toggle-text-path()\n\
 s ~a ~c ~Meta<Key>F17:toggle-editing-path-stay()\n\
 ~a ~c ~s ~Meta<Key>F17:toggle-editing-path-move()\n\
 s ~c ~m ~a <Key>space:self-insert()\n\
 <Key>:self-insert()\n\
 ~c  s ~m ~Alt<Btn1Down>:extend-start()\n\
  c ~s ~m ~Alt<Btn1Down>:move-destination()\n\
 ~c ~s ~m ~Alt<Btn1Down>:grab-focus()\n\
 ~c ~m ~Alt<Btn1Motion>:extend-adjust()\n\
 ~c ~m ~Alt<Btn1Up>:extend-end()\n\
 <Btn2Down>:process-bdrag()\n\
 ~c ~s  m ~Alt<Btn2Motion>:secondary-adjust()\n\
 ~c ~s ~m  Alt<Btn2Motion>:secondary-adjust()\n\
 ~s ~m ~Alt<Btn2Motion>:secondary-adjust()\n\
 ~c ~s  m ~Alt<Btn2Up>:move-to()\n\
 ~c ~s ~m  Alt<Btn2Up>:move-to()\n\
 ~s ~m ~Alt<Btn2Up>:copy-to()\n\
 <EnterWindow>:enter()\n\
 <LeaveWindow>:leave()\n\
 <FocusIn>:focusIn()\n\
 <FocusOut>:focusOut()\n\
 <Unmap>:unmap()";
#endif


static void Invalidate(widget, position, topos, delta)
DXmCSTextWidget   widget;
DXmCSTextPosition position, topos;
int              delta;
{
    DXmCSTextInputData data;

    data = InputData (widget);

    if ( delta == NODELTA ) 
        return; /* Just use what we have as best guess. */

    if ( data->stuffpos > position )
         data->stuffpos += delta;

    if ( data->origLeft > position )
         data->origLeft += delta;

    if ( data->origRight >= position )
         data->origRight += delta;
}



static void InputGetValues(widget, args, num_args)
DXmCSTextWidget widget;
ArgList        args;
Cardinal       num_args;
{
    XtGetSubvalues( (XtPointer) InputData (widget),
		    i_str_input_resources,
                    XtNumber( i_str_input_resources ), 
		    args,
                    num_args );
}


static void InputSetValues(widget, args, num_args)
DXmCSTextWidget widget;
ArgList        args;
Cardinal       num_args;
{
    XtSetSubvalues( (XtPointer) InputData (widget),
		    i_str_input_resources,
                    XtNumber( i_str_input_resources ), 
		    args,
                    num_args );
}


static void Destroy(widget)
DXmCSTextWidget widget;
{
    XtFree( (char *) InputData (widget) );
    XtFree( (char *) Input (widget) );
    XmImUnregister((Widget) widget);
}

static void
#ifdef _NO_PROTO
DropDestroyCB(w, clientData, callData)
    Widget      w;
    XtPointer   clientData;
    XtPointer   callData;
#else
DropDestroyCB(
    Widget      w,
    XtPointer   clientData,
    XtPointer   callData )
#endif /* NO_PROTO */
{
    XtFree((char *)clientData);
}

/* 
 * Called when a drop transaction has been negotiated between the cstext
 * widget and the initiator.  Responsible for transferring the dragged
 * data, converting it into compound string format, and inserting it
 * into the cstext widget in the proper place.
 */

static void
#ifdef _NO_PROTO
DropTransferCallback( w, closure, seltype, type, value, length, format )
        Widget w ;
        XtPointer closure ;
        Atom *seltype ;
        Atom *type ;
        XtPointer value ;
        unsigned long *length ;
        int *format ;
#else
DropTransferCallback(
        Widget w,
        XtPointer closure,
        Atom *seltype,
        Atom *type,
        XtPointer value,
        unsigned long *length,
        int *format )
#endif /* _NO_PROTO */
{
    _DXmCSTextDropTransferRec *transfer_rec = (_DXmCSTextDropTransferRec *) closure;
    DXmCSTextWidget widget = (DXmCSTextWidget) transfer_rec->widget;
    Atom C_TEXT   = XInternAtom(XtDisplay(widget), "COMPOUND_TEXT", FALSE);
    Atom DDIF = XInternAtom(XtDisplay(widget), "DDIF", FALSE);
    DXmCSTextInputData data = InputData (widget);
    DXmCSTextPosition  old_length, left, right, insertPosLeft, insertPosRight;
    Boolean local = CSTextHasSelection(widget), sep;
    XmStringContext context;
    XmStringCharSet tag;
    XmStringDirection dir;
    char *text;
    long return_status;
    XmString cstring;
    Arg args[20];
    int n, len, num_chars, max_length;

    /* When type = NULL, we assume a DELETE request has been satisfied */
    /* and the move operation is complete, so we can now select the    */
    /* dropped text.						       */

    if (*type == XmInternAtom(XtDisplay(widget), "NULL", False)) 
    {
	if ((transfer_rec->move) && (transfer_rec->num_chars > 0))
	{
	    DXmCSTextSetSelection( (Widget) widget, 
			transfer_rec->insert_pos,
			transfer_rec->insert_pos + transfer_rec->num_chars,
			transfer_rec->timestamp);
	    if (value) 
	    {
		XtFree((char *)value);
		value = NULL;
	    }

	    return;
	 }
    }

    /* 
     * Give up if the initiator doesn't offer it's data in any of the
     * formats that we're capable of converting to compound string.
     */

    if (!value || (*type != C_TEXT && *type != DDIF && *type != XA_STRING)) 
    {
        n = 0;
        XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
        XtSetArg(args[n], XmNnumDropTransfers, 0); n++;
        XtSetValues(w, args, n);

        if (value) {
	   XtFree((char *)value);
	   value = NULL;
        }

        return;
    }

    insertPosLeft = insertPosRight = transfer_rec->insert_pos;

    /* if we get here, then we obtained the data in some useful type
     * now convert whatever it is into compound string for insertion in source
     */

    return_status = DXmCvtStatusFail;

    if (*type == DDIF)
    {
	cstring = DXmCvtDDIFtoCS( value, (long *) &len, &return_status);
    }
    else {
    if (*type == C_TEXT)
    {
	cstring = XmCvtCTToXmString( value );
	return_status = DXmCvtStatusOK;
    }
    else {
    if (*type == XA_STRING)
    {
	cstring = DXmCvtFCtoCS( value, (long *) length, &return_status);
    }}}

    if (return_status != DXmCvtStatusOK)
    {
        RingBell( w, (XEvent *) NULL, (char **) NULL, (Cardinal) 0 );
	return;
    } 

    old_length = CSTextLength (widget);

    /* If pending delete is on and we're dropping onto the selected region,
     * set things up so that the selected region gets replaced by the
     * dropped text.
     */

    if (data->pendingdelete && 
	(_DXmCSTextGetSelection(widget, &left, &right) && left != right) && 
	insertPosLeft > left && insertPosRight < right) 
    {
       insertPosLeft = left;
       insertPosRight = right;
    }

    /* Don't let max_length inhibit moving of text when drag site */
    /* and drop site are the same widget			  */

    if (transfer_rec->move && local) {
       max_length = CSTextMaxLength(widget);
       CSTextMaxLength(widget) = MAXINT;
    }

    /* Now insert the dropped text into the widget */

    num_chars = 0;
    if (XmStringInitContext(&context, cstring))
    {
	while (XmStringGetNextSegment(context, &text, &tag, &dir, &sep))
	{
	    num_chars += strlen(text);

	    if (text) XtFree(text);
	    if (tag) XtFree(tag);
	}
	
	XmStringFreeContext (context);
    }
		
    if(( old_length + num_chars <= CSTextMaxLength(widget)) &&
       (_DXmCSTextSourceReplaceString(widget, insertPosLeft, insertPosRight, 
				     cstring) != DXmCSTextStatusEditError))
    {
	if (_DXmCSTextGetSelection(widget, &left, &right))
	{
	    /* Drag initiator and receiver are the same widget */

	    transfer_rec->num_chars = right - left;

	    /* Since a move operation removes the dragged text from it's
	     * original point of origin, we need to adjust the insertion
	     * point if the original dragged text is to the left of the 
	     * drop point.
	     */

	    if (transfer_rec->move && (transfer_rec->insert_pos > right))
		transfer_rec->insert_pos -= transfer_rec->num_chars;
	}
	else
	    transfer_rec->num_chars = CSTextLength (widget) - old_length;

	/* 
	 * If the operation is a move, add a DELETE transfer request.  This
	 * will cause the initiator widget's XmNconvertProc to get called
	 * with the DELETE atom as the desired type.  It can then interpret
	 * this as a signal that the move operation was successful and that
	 * it is OK to delete the selected text.
	 */
     
	if (transfer_rec->move) {
	    XmDropTransferEntryRec transferEntries[1];
	    XmDropTransferEntryRec *transferList = NULL;

	    transferEntries[0].client_data = (XtPointer) transfer_rec;
	    transferEntries[0].target = XmInternAtom(XtDisplay(w),"DELETE",
						     False);
	    transferList = transferEntries;
	    XmDropTransferAdd(w, transferEntries, 1);
	}
    }
    else
    {
        RingBell( w, (XEvent *) NULL, (char **) NULL, (Cardinal) 0 );
    }

    if (transfer_rec->move && local)
	CSTextMaxLength(widget) = max_length;

    XmStringFree( cstring );
    XtFree((char *)value);
    value = NULL;
}

static XtCallbackRec dropDestroyCB[] = { {DropDestroyCB, NULL},
                 			 {(XtCallbackProc)NULL, NULL} };

/*
 * Called from the DropProcCallback whenever something is dropped on a
 * cstext widget.  Determines if the initiator widget offers a 
 * compatible data format and initializes the record needed by the
 * DropTransferCallback if so.
 */

static void
#ifdef _NO_PROTO
HandleDrop(w, cb)
        Widget w;
        XmDropProcCallbackStruct *cb;
#else
HandleDrop(
        Widget w,
        XmDropProcCallbackStruct *cb )
#endif /* _NO_PROTO */
{
    Widget drag_ctx, initiator;
    DXmCSTextWidget widget = (DXmCSTextWidget) w;
    DXmCSTextPosition insert_pos, left, right;
    XmDropTransferEntryRec transferEntries[2], *transferList = NULL;
    Atom DDIF, C_TEXT, *exportTargets;
    _DXmCSTextDropTransferRec *transfer_rec;
    Cardinal numExportTargets, n, numTransfers = 0;
    Boolean ddif_found, c_text_found, string_found;
    Arg args[10];
    int status;

    drag_ctx = cb->dragContext;

    n = 0;
    XtSetArg(args[n], XmNsourceWidget, &initiator); n++;
    XtSetArg(args[n], XmNexportTargets, &exportTargets); n++;
    XtSetArg(args[n], XmNnumExportTargets, &numExportTargets); n++;
    XtGetValues((Widget) drag_ctx, args, n);

    /* Determine the attempted drop position */
    insert_pos = OutputXYToPos(widget, cb->x, cb->y);

    /* Moving a selected range onto itself just fails */ 
    if (cb->operation & XmDROP_MOVE && w == initiator &&
        (_DXmCSTextGetSelection(widget, &left, &right) &&
	 left != right && insert_pos >= left && insert_pos <= right)) 
    {
	n = 0;
	XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
	XtSetArg(args[n], XmNnumDropTransfers, 0); n++;
    } 
    else 
    {
	/* intialize data to send to drop transfer callback */

	transfer_rec = (_DXmCSTextDropTransferRec *) 
		      XtMalloc(sizeof(_DXmCSTextDropTransferRec));

	transfer_rec->widget = w;
	transfer_rec->insert_pos = insert_pos;
	transfer_rec->num_chars = 0;
	transfer_rec->timestamp = cb->timeStamp;

	if (cb->operation & XmDROP_MOVE)
	    transfer_rec->move = True;
	else
	    transfer_rec->move = False;

	transferEntries[0].client_data = (XtPointer) transfer_rec;
	transferList = transferEntries;
	numTransfers = 1;

	DDIF     = XInternAtom(XtDisplay(widget), "DDIF", FALSE);
	C_TEXT   = XInternAtom(XtDisplay(widget), "COMPOUND_TEXT", FALSE);

	ddif_found = c_text_found = string_found = False;

	/* Determine if the initiator widget offers a compatible format */
	for (n = 0; n < numExportTargets; n++) 
	{
	    if (exportTargets[n] == DDIF) ddif_found = True;
	    if (exportTargets[n] == C_TEXT) c_text_found = True;
	    if (exportTargets[n] == XA_STRING) string_found = True;
	}

	n = 0;
	if (c_text_found || ddif_found || string_found) 
	{
	    if (ddif_found)
		transferEntries[0].target = DDIF;
	    else if (c_text_found)
		transferEntries[0].target = C_TEXT;
	    else if (string_found)
		transferEntries[0].target = XA_STRING;

	    if (cb->operation & XmDROP_MOVE || cb->operation & XmDROP_COPY) 
	    {
		XtSetArg(args[n], XmNdropTransfers, transferList); n++;
		XtSetArg(args[n], XmNnumDropTransfers, numTransfers); n++;
	    } 
	    else 
	    {
		XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
		XtSetArg(args[n], XmNnumDropTransfers, 0); n++;
	    }
	} 
	else 
	{
	    XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE); n++;
	    XtSetArg(args[n], XmNnumDropTransfers, 0); n++;
	}

	dropDestroyCB[0].closure = (XtPointer) transfer_rec;
	XtSetArg(args[n], XmNdestroyCallback, dropDestroyCB); n++;
	XtSetArg(args[n], XmNtransferProc, DropTransferCallback); n++;
    }

    XmDropTransferStart(drag_ctx, args, n);
}

/* 
 * This proc is only called when dynamic drag protocol is used.  It takes
 * care of the necessary drag under effects (border highlighting in this
 * case).
 */

static void
#ifdef _NO_PROTO
DragProcCallback(w, client, call)
        Widget w;
        XtPointer client;
        XtPointer call;
#else
DragProcCallback(
        Widget w,
        XtPointer client,
        XtPointer call )
#endif /* _NO_PROTO */
{
    DXmCSTextWidgetClass cstext_class = (DXmCSTextWidgetClass) XtClass(w);
    XmDragProcCallbackStruct *cb = (XmDragProcCallbackStruct *)call;
    Cardinal num_exp_targets, n;
    Atom targets[4], *exp_targets;
    Widget drag_ctx;
    int status = 0;
    Arg args[10];

    targets[0] = XmInternAtom(XtDisplay(w), "DDIF", False);
    targets[1] = XmInternAtom(XtDisplay(w), "COMPOUND_TEXT", False);
    targets[2] = XA_STRING;

    drag_ctx = cb->dragContext;

    n = 0;
    XtSetArg(args[n], XmNexportTargets, &exp_targets); n++;
    XtSetArg(args[n], XmNnumExportTargets, &num_exp_targets); n++;
    XtGetValues(drag_ctx, args, n);

    switch(cb->reason) 
    {
	case XmCR_DROP_SITE_ENTER_MESSAGE:
	    if (XmTargetsAreCompatible (XtDisplay(drag_ctx), exp_targets,
					num_exp_targets, targets, 3)) 
	    {

		cb->animate = FALSE;
		cb->dropSiteStatus = XmVALID_DROP_SITE;
		_XmHighlightBorder (w);
	    }
	    else
		cb->dropSiteStatus = XmINVALID_DROP_SITE;

	    break;

	case XmCR_DROP_SITE_LEAVE_MESSAGE:
	    if (XmTargetsAreCompatible(XtDisplay(drag_ctx), exp_targets,
				       num_exp_targets, targets, 3)) 
	    {
		cb->animate = FALSE;
		_XmUnhighlightBorder (w);
	    }

	    break;

	case XmCR_DROP_SITE_MOTION_MESSAGE:
	case XmCR_OPERATION_CHANGED:
	    /* we currently don't care about these message */
	    break;

	default:
	    /* other messages we consider invalid */
	    cb->dropSiteStatus = XmINVALID_DROP_SITE;
	    break;
    }
}

static void
#ifdef _NO_PROTO
DropProcCallback(w, client, call)
        Widget w;
        XtPointer client;
        XtPointer call;
#else
DropProcCallback(
        Widget w,
        XtPointer client,
        XtPointer call )
#endif /* _NO_PROTO */
{
    XmDropProcCallbackStruct *cb = (XmDropProcCallbackStruct *) call;
    Arg args[2];

    if (cb->dropAction != XmDROP_HELP) 
       HandleDrop(w, cb);
    else 
    {
       XtSetArg(args[0], XmNtransferStatus, XmTRANSFER_FAILURE);
       XtSetArg(args[1], XmNnumDropTransfers, 0);
       XmDropTransferStart(cb->dragContext, args, 2);
    }
}

/*
 * Registers the cstext widget a drop site capable of receiving data in 
 * DDIF, compound text, or STRING formats.
 */

static void
#ifdef _NO_PROTO
RegisterDropSite(w)
        Widget w ;
#else
RegisterDropSite(
        Widget w )
#endif /* _NO_PROTO */
{
    Atom targets[4];
    int status = 0;
    Arg args[10];
    int n;

    targets[0] = XmInternAtom(XtDisplay(w), "DDIF", False);
    targets[1] = XmInternAtom(XtDisplay(w), "COMPOUND_TEXT", False);
    targets[2] = XA_STRING;

    n = 0;
    XtSetArg(args[n], XmNimportTargets, targets); n++;
    XtSetArg(args[n], XmNnumImportTargets, 3); n++;
    XtSetArg(args[n], XmNdragProc, DragProcCallback); n++;
    XtSetArg(args[n], XmNdropProc, DropProcCallback); n++;
    XmDropSiteRegister(w, args, n);
}


#ifdef _NO_PROTO

#ifdef I18N_IO_SHR
#ifdef VMS
void _DXmCSTextInputCreate(widget, args, num_args)
#else
void DXmCSTextInputCreate(widget, args, num_args)
#endif /* VMS */
#else
void DXmCSTextInputCreate(widget, args, num_args)
#endif /* I18N_IO_SHR */

DXmCSTextWidget widget;
ArgList        args;
Cardinal       num_args;

#else

#ifdef I18N_IO_SHR
#ifdef VMS
void _DXmCSTextInputCreate(
#else
void DXmCSTextInputCreate(
#endif /* VMS */
#else
void DXmCSTextInputCreate(
#endif /* I18N_IO_SHR */
			   DXmCSTextWidget widget,
			   ArgList	   args,
			   Cardinal	   num_args)
#endif /* _NO_PROTO */
{
    DXmCSTextInput     input;
    DXmCSTextInputData data;
    Arg im_args[6];  /* To set initial values to input method */
    Cardinal n = 0;
    XPoint xmim_point;

    /* create the input and inputdata structure for the widget
    */
    input = (DXmCSTextInput) XtMalloc( (unsigned) sizeof(DXmCSTextInputRec) );

    data  = (DXmCSTextInputData)
                            XtMalloc( (unsigned) sizeof(DXmCSTextInputDataRec) );

    Input (widget) = input;
    input->data    = data;

    XtGetSubresources( (Widget) widget->core.parent, 
		       (XtPointer)data,
		       widget->core.name, 
		       DXmSClassCSTextWidget, 
		       i_str_input_resources,
		       XtNumber( i_str_input_resources ), 
		       args, 
		       num_args );

    data->widget = widget;

#ifndef WIN32
    bzero((char *) &(data->compstatus), sizeof(XComposeStatus) );
#else
    memset((void *) &(data->compstatus), 0, sizeof(XComposeStatus) );
#endif

    /* set up the scan type array.  Must be in the order or sarray[]
    */
    if ( data->sarray == NULL || data->sarraycount <= 0 )
    {
	 data->sarray = (DXmCSTextScanType *) sarray;
	 data->sarraycount = XtNumber( sarray );
    }

    data->lasttime  = 0;
    data->cancel    = False;
    data->stype     = data->sarray[0];
    data->extendDir = DXmsdRight;
    data->hasSel2   = FALSE;
    data->stuffpos  = 0;
    data->origLeft  = CursorPos (widget);
    data->origRight = CursorPos (widget);
    data->anchor    = CursorPos (widget);
    data->has_destination = False;
    data->changed_dest_visible = False;
    data->dest_time = 0;
    data->sec_time  = 0;

    /* register the procedures defined here
    */
    input->Invalidate = Invalidate;
    input->GetValues  = InputGetValues;
    input->SetValues  = InputSetValues;
    input->destroy    = (XtWidgetProc)Destroy;

    if (CSTextEditable(widget)) {

	XmImRegister((Widget) widget, (unsigned int) NULL);

        DXmCSTextPosToXY ((Widget) widget, DXmCSTextGetCursorPosition(widget),
                                      &xmim_point.x, &xmim_point.y);
	n = 0;
        XtSetArg(im_args[n], XmNfontList, FontList(widget)); n++;
	XtSetArg(im_args[n], XmNbackground, 
		 widget->core.background_pixel); n++;
	XtSetArg(im_args[n], XmNforeground, widget->primitive.foreground); n++;
	XtSetArg(im_args[n], XmNbackgroundPixmap,
                   widget->core.background_pixmap);n++;
	XtSetArg(im_args[n], XmNspotLocation, &xmim_point); n++;
	XtSetArg(im_args[n], XmNlineSpace, TypicalHeight(widget)); n++;
	XmImSetValues((Widget)widget, im_args, n);
    }

    RegisterDropSite((Widget) widget);
}


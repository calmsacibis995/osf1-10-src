/* #module searchform.c "v1.0"
 *
 *  Copyright (c) Digital Equipment Corporation, 1993
 *  All Rights Reserved.  Unpublished rights reserved
 *  under the copyright laws of the United States.
 *  
 *  The software contained on this media is proprietary
 *  to and embodies the confidential technology of 
 *  Digital Equipment Corporation.  Possession, use,
 *  duplication or dissemination of the software and
 *  media is authorized only pursuant to a valid written
 *  license from Digital Equipment Corporation.
 *
 *  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 *  disclosure by the U.S. Government is subject to
 *  restrictions as set forth in Subparagraph (c)(1)(ii)
 *  of DFARS 252.227-7013, or in FAR 52.227-19, as
 *  applicable.
 *
 *
 * FACILITY:
 *	Notepad - DECwindows simple out-of-the-box editor.
 *
 * ABSTRACT:
 * 
 *
 * NOTES:
 *	
 *
 * REVISION HISTORY:
 */

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: [searchForm.c,v 1.3 91/05/23 18:48:41 rmurphy Exp ]$";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

#include "notepad.h"

void grabSelection(w)
  Widget w;
{
  XmTextWidget textW;
  XmTextPosition left, right;
  XmTextSource _Source;

    if (XtClass(w) != xmTextWidgetClass) {
	textW = (XmTextWidget) XmSelectionBoxGetChild(w, XmDIALOG_TEXT);
    } else {
	textW = (XmTextWidget) w;
    }

#if 0
    _Source = XmTextGetSource(textW);
    left =  (*_Source->Scan)(_Source, 0, XmSELECT_ALL, XmsdLeft,  1, FALSE);
    right = (*_Source->Scan)(_Source, 0, XmSELECT_ALL, XmsdRight, 1, FALSE);
    (*_Source->SetSelection)
	(_Source, left, right, XtLastTimestampProcessed(XtDisplay(w)));
#else
    XmTextSetSelection
    (
	textW,
	0,
	XmTextGetLastPosition(textW),
	XtLastTimestampProcessed(XtDisplay(w))
    );
#endif
}

void adjustButtons(wl, n)
  Widget *wl;
  int n;
{
    int i, maxi = 0;
    for(i=0; i<n; i++)
    {
	if (XtWidth(wl[i]) > maxi) maxi = XtWidth(wl[i]);
    }
    for(i=0; i<n; i++)
    {
	setaValue(wl[i], XmNwidth, maxi);
    }
}

void  mkSaveDialog(w, sd)
  Widget w;
  simpleDialog *sd;
{

  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "saveDialog", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
}

void mkLineDialog(w, sd)
  Widget w;
  simpleDialog *sd;
{
  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "lineDialog", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
}

void mkSimpleFindDialog(w, sd)
  Widget w;
  simpleDialog *sd;
{
  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "findDialog", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
}

void mkIncrDialog(w, sd)
  Widget w;
  simpleDialog *sd; 
{
  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "incrDialog", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
}

void mkReplaceDialog(w, sd)
  simpleDialog *sd;
  Widget w;
{
  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "replaceDialog", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
}

void makeSearchOptionsDialog(w, sd)
  Widget w;
  simpleDialog *sd;
{
  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "searchOptionsDialogBox", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
/*     state[st_CaseSensitive] = TRUE;
 */
}

/*
 * Modeless Dialog box for Undo and Redo
 */
void makeUndoDialog(w, sd)
  Widget w;
  simpleDialog *sd;
{
  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "undoDialog", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
}

#ifdef unix

/* dialog for filter input */ 

void mkFilterDialog(w, sd)
  Widget w;
  simpleDialog *sd;
{
  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "filterDialog", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
}

#endif /* unix */

void makeFontDialog(w, sd)
  Widget w;
  simpleDialog *sd;
{
  int n = 0;
  Arg a[10];
  Widget popup;
    MrmFetchWidgetOverride( DRM_hierarchy, "fontDialog", mainWin,
	NULL, a,  n, &popup, &dummy_class);	
}

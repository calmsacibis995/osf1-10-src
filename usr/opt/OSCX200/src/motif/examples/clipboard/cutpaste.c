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
static char *rcsid = "@(#)$RCSfile: cutpaste.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 15:29:56 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * Motif Release 1.2
 */

/******************************************************************************
 * cutpaste.c
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/CutPaste.h>
#include <Mrm/MrmPublic.h>

#ifdef _NO_PROTO

static Boolean CopyToClipboard();
static Boolean PasteClipboard();
static void PopupHandler();
static void ManageCb();
static void InitPopupCb();
static void CutCb();
static void CopyCb();
static void PasteCb();
static void ExitCb();

#else

static Boolean CopyToClipboard(
	Time time);
static Boolean PasteClipboard(
	Time time );
static void PopupHandler(
	Widget w,
	Widget pw,
	XEvent *event,
	Boolean *ctd );
static void ManageCb(
	Widget w,
	String id,
	XtPointer cb );
static void InitPopupCb(
	Widget w,
	String id,
	XtPointer cb );
static void CutCb(
	Widget w,
	XtPointer cd,
	XmPushButtonCallbackStruct *cb );
static void CopyCb(
	Widget w,
	XtPointer cd,
	XmPushButtonCallbackStruct *cb );
static void PasteCb(
	Widget w,
	XtPointer cd,
	XmPushButtonCallbackStruct *cb );
static void ExitCb(
	Widget w,
	XtPointer cd,
	XtPointer cb );

#endif

static MrmHierarchy mrm_id;
static char *mrm_vec[]={"cutpaste.uid"};
static MrmCode mrm_class;
static MRMRegisterArg mrm_names[] = {
        {"InitPopupCb", (caddr_t)InitPopupCb },
        {"ManageCb", (caddr_t)ManageCb },
        {"CutCb", (caddr_t)CutCb },
        {"CopyCb", (caddr_t)CopyCb },
        {"PasteCb", (caddr_t)PasteCb },
        {"ExitCb", (caddr_t)ExitCb }
};

#define GraphFormat "MY_GRAPH_DATA"

#define MIN(x,y)  ((x) > (y) ? (y) : (x))

static XtAppContext  appContext;
static Widget shell;
static Widget *bars;
static Cardinal nbars;

int
#ifdef _NO_PROTO
main(argc, argv)
    int argc;
    char *argv[];
#else
main(
    int argc,
    char *argv[] )
#endif
{
    Widget app_main;
    Display *display;
    Arg args[2];
    Widget *children;

    XtToolkitInitialize();
    MrmInitialize ();
    appContext = XtCreateApplicationContext();
    display = XtOpenDisplay(appContext, NULL, argv[0], "Cutpaste",
			NULL, 0, &argc, argv);
    if (display == NULL) {
	fprintf(stderr, "%s:  Can't open display\n", argv[0]);
	exit(1);
    }
    shell = XtAppCreateShell(argv[0], NULL, applicationShellWidgetClass,
			  display, NULL, 0);
    if (MrmOpenHierarchy (1, mrm_vec, NULL, &mrm_id) != MrmSUCCESS) exit(0);
    MrmRegisterNames(mrm_names, XtNumber(mrm_names));
    MrmFetchWidget (mrm_id, "appMain", shell, &app_main, &mrm_class);
    XtManageChild(app_main);

    /* define graph data format */

    while (XmClipboardRegisterFormat (display, GraphFormat, 32) ==
					XmClipboardLocked)  { }

    XtSetArg (args[0], XmNchildren, &children);
    XtSetArg (args[1], XmNnumChildren, &nbars);
    XtGetValues (XtNameToWidget (shell, "*graphForm"), args, 2);
    bars = (Widget *) XtMalloc (nbars * sizeof(Widget));
    memcpy (bars, children, nbars * sizeof(Widget));

    XtRealizeWidget(shell);

    XtAppMainLoop(appContext);
}

static void
#ifdef _NO_PROTO
ManageCb(w, id, cb)
    Widget w;
    String id;
    XtPointer cb;
#else
ManageCb(
    Widget w,
    String id,
    XtPointer cb )
#endif

{
    XtManageChild (XtNameToWidget (shell, id));
}

static void
#ifdef _NO_PROTO
ExitCb(w, cd, cb)
    Widget w;
    XtPointer cd;
    XtPointer cb;
#else
ExitCb(
    Widget w,
    XtPointer cd,
    XtPointer cb )
#endif
{
    exit(0);
}

/*****************************************************************
 *
 * PopupMenu support
 *
 *****************************************************************/

static void
#ifdef _NO_PROTO
InitPopupCb(w, id, cb)
    Widget w;
    String id;
    XtPointer cb;
#else
InitPopupCb(
    Widget w,
    String id,
    XtPointer cb )
#endif
{
    Widget popupWindow = XtNameToWidget (shell, id);

    XtAddEventHandler (popupWindow, ButtonPressMask, False,
		(XtEventHandler)PopupHandler, (XtPointer) w);
}

static void
#ifdef _NO_PROTO
PopupHandler (w, pw, event, ctd)
    Widget w;
    Widget pw;
    XEvent *event;
    Boolean *ctd;
#else
PopupHandler (
    Widget w,
    Widget pw,
    XEvent *event,
    Boolean *ctd )
#endif
{
    if (((XButtonEvent *)event)->button != Button3) return;

    XmMenuPosition((Widget) pw, (XButtonEvent *)event);
    XtManageChild ((Widget) pw);
}

/*****************************************************************
 *
 * Clipboard support
 *
 *****************************************************************/

static void
#ifdef _NO_PROTO
CutCb(w, cd, cb)
    Widget w;
    XtPointer cd;
    XmPushButtonCallbackStruct *cb;
#else
CutCb(
    Widget w,
    XtPointer cd,
    XmPushButtonCallbackStruct *cb )
#endif
{
    XButtonEvent *be = (XButtonEvent *)cb->event;
    Arg args[1];
    int i;

    if (CopyToClipboard(be->time) == True) {

	/* clear graph data */

	XtSetArg (args[0], XmNvalue, 0);
	for (i=0; i < nbars; i++)
	    XtSetValues (bars[i], args, 1);
    }
    else {
	XBell (XtDisplay(w), 0);
    }
}

static void
#ifdef _NO_PROTO
CopyCb(w, cd, cb)
    Widget w;
    XtPointer cd;
    XmPushButtonCallbackStruct *cb;
#else
CopyCb(
    Widget w,
    XtPointer cd,
    XmPushButtonCallbackStruct *cb )
#endif
{
    XButtonEvent *be = (XButtonEvent *)cb->event;

    if (CopyToClipboard(be->time) != True) {
	XBell (XtDisplay(w), 0);
    }
}

static void
#ifdef _NO_PROTO
PasteCb(w, cd, cb)
    Widget w;
    XtPointer cd;
    XmPushButtonCallbackStruct *cb;
#else
PasteCb(
    Widget w,
    XtPointer cd,
    XmPushButtonCallbackStruct *cb )
#endif
{
    XButtonEvent *be = (XButtonEvent *)cb->event;

    if (PasteClipboard(be->time) != True) {
	XBell (XtDisplay(w), 0);
    }
}

static Boolean
#ifdef _NO_PROTO
CopyToClipboard(time)
    Time time;
#else
CopyToClipboard(
    Time time )
#endif
{
    Display *dpy = XtDisplay (shell);
    Window window = XtWindow (shell);
    long itemId = 0;
    XmString clipLabel;
    long *graphData;
    int value;
    int i;
    Arg args[1];

    if (XmClipboardLock (dpy, window) == XmClipboardLocked)
	return (False);

    clipLabel = XmStringCreateLtoR ("cutpaste", XmFONTLIST_DEFAULT_TAG);

    XmClipboardStartCopy ( dpy, window, clipLabel, time, NULL, NULL, &itemId);

    /* copy graph data */

    graphData = (long *)XtMalloc (nbars * sizeof (long));
    XtSetArg (args[0], XmNvalue, &value);
    for (i=0; i < nbars; i++) {
	XtGetValues (bars[i], args, 1);
	graphData[i] = (long)value;
    }

    XmClipboardCopy (dpy, window, itemId, GraphFormat, (XtPointer)graphData,
			(nbars * sizeof(long)), 0, NULL);

    XmClipboardEndCopy (dpy, window, itemId);

    XmClipboardUnlock (dpy, window, False);

    XtFree ((char *)clipLabel);
    XtFree ((char *)graphData);

    return (True);
}

static Boolean
#ifdef _NO_PROTO
PasteClipboard(time)
    Time time;
#else
PasteClipboard(
    Time time )
#endif
{
    Display *dpy = XtDisplay (shell);
    Window window = XtWindow (shell);
    unsigned long length = 0;
    long *graphData;
    int i;
    int setBars;
    Arg args[1];

    if (XmClipboardStartRetrieve (dpy, window, time) == XmClipboardLocked)
	return (False);

    XmClipboardInquireLength(dpy, window, GraphFormat, &length);

    if (length == 0) {
	XmClipboardEndRetrieve (dpy, window);
	return (False);
    }

    graphData = (long *)XtMalloc (length * sizeof (long));

    XmClipboardRetrieve(dpy, window, GraphFormat, graphData, length,
			NULL, NULL);

    XmClipboardEndRetrieve(dpy, window);

    setBars = MIN (nbars, length);
    for (i = 0; i < setBars; i++) {
	XtSetArg (args[0], XmNvalue, graphData[i]);
	XtSetValues (bars[i], args, 1);
    }

    XtFree ((char *)graphData);

    return (True);
}


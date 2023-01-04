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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: Display.c,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/15 16:16:27 $"
#endif
#endif
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <stdio.h>
#include <Xm/DisplayP.h>
#include <Xm/ScreenP.h>
#include <Xm/WorldP.h>
#include <Xm/DropTransP.h>
#include "DisplayI.h"
#include "DragCI.h"
#include "DragICCI.h"
#include <Xm/AtomMgr.h>
#include <X11/Xatom.h>


#define TheDisplay(dd) (XtDisplayOfObject((Widget)dd))
#define TheScreen(dd) (XtScreen((Widget)dd))

#define Offset(x) (XtOffsetOf( struct _XmDisplayRec, x))

#define CHECK_TIME(dc, time) \
  ((dc->drag.dragStartTime <= time) && \
   ((dc->drag.dragFinishTime == 0) || (time <= dc->drag.dragFinishTime)))

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void DisplayClassPartInitialize() ;
static void DisplayClassInitialize() ;
static void SetDragReceiverInfo() ;
static void TreeUpdateHandler() ;
static void DisplayInitialize() ;
static void DisplayInsertChild() ;
static void DisplayDeleteChild() ;
static void DisplayDestroy() ;
static XmDragContext FindDC() ;
static int isMine() ;
static void ReceiverShellExternalSourceHandler() ;
static Widget GetDisplay() ;

#else

static void DisplayClassPartInitialize( 
                        WidgetClass wc) ;
static void DisplayClassInitialize( void ) ;
static void SetDragReceiverInfo( 
                        Widget w,
                        XtPointer client_data,
                        XEvent *event,
                        Boolean *dontSwallow) ;
static void TreeUpdateHandler( 
                        Widget w,
                        XtPointer client,
                        XtPointer call) ;
static void DisplayInitialize( 
                        Widget requested_widget,
                        Widget new_widget,
                        ArgList args,
                        Cardinal *num_args) ;
static void DisplayInsertChild( 
                        Widget w) ;
static void DisplayDeleteChild( 
                        Widget w) ;
static void DisplayDestroy( 
                        Widget w) ;
static XmDragContext FindDC( 
                        XmDisplay xmDisplay,
                        Time time,
#if NeedWidePrototypes
                        int sourceIsExternal) ;
#else
                        Boolean sourceIsExternal) ;
#endif /* NeedWidePrototypes */
static int isMine( 
                        Display *dpy,
                        register XEvent *event,
                        char *arg) ;
static void ReceiverShellExternalSourceHandler( 
                        Widget w,
                        XtPointer client_data,
                        XEvent *event,
                        Boolean *dontSwallow) ;
static Widget GetDisplay( 
                        Display *display) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

#ifdef DEC_MOTIF_BUG_FIX
externaldef(_xm_motif_drag_and_drop_message) String _Xm_MOTIF_DRAG_AND_DROP_MESSAGE;
#else
String	_Xm_MOTIF_DRAG_AND_DROP_MESSAGE;
#endif

static XContext	displayContext = 0;
static WidgetClass curDisplayClass = NULL;

static XtResource resources[] = {
    {
	XmNdropSiteManagerClass, XmCDropSiteManagerClass, XmRWidgetClass,
	sizeof(WidgetClass), Offset(display.dropSiteManagerClass), 
	XmRImmediate, (XtPointer)&xmDropSiteManagerClassRec,
    },
    {
	XmNdropTransferClass, XmCDropTransferClass, XmRWidgetClass,
	sizeof(WidgetClass), Offset(display.dropTransferClass), 
	XmRImmediate, (XtPointer)&xmDropTransferClassRec,
    },
    {
	XmNdragContextClass, XmCDragContextClass, XmRWidgetClass,	
	sizeof(WidgetClass), Offset(display.dragContextClass), 
	XmRImmediate, (XtPointer)&xmDragContextClassRec,
    },
    {
	XmNdragInitiatorProtocolStyle, XmCDragInitiatorProtocolStyle,
	XmRDragInitiatorProtocolStyle, sizeof(unsigned char), 
	Offset(display.dragInitiatorProtocolStyle), 
	XmRImmediate, (XtPointer)XmDRAG_PREFER_RECEIVER,
    },
    {
	XmNdragReceiverProtocolStyle, XmCDragReceiverProtocolStyle,
	XmRDragReceiverProtocolStyle, sizeof(unsigned char), 
	Offset(display.dragReceiverProtocolStyle), 
	XmRImmediate, (XtPointer)XmDRAG_PREFER_PREREGISTER,
    },
    {
	"defaultVirtualBindings", "DefaultVirtualBindings",
	XmRString, sizeof(String),
	Offset(display.bindingsString),
	XmRImmediate, (XtPointer)NULL,
    },
};

#undef Offset

static XmBaseClassExtRec baseClassExtRec = {
    NULL,
    NULLQUARK,
    XmBaseClassExtVersion,
    sizeof(XmBaseClassExtRec),
    (XtInitProc)NULL,			/* InitializePrehook	*/
    (XtSetValuesFunc)NULL,		/* SetValuesPrehook	*/
    (XtInitProc)NULL,			/* InitializePosthook	*/
    (XtSetValuesFunc)NULL,		/* SetValuesPosthook	*/
    NULL,				/* secondaryObjectClass	*/
    (XtInitProc)NULL,			/* secondaryCreate	*/
    (XmGetSecResDataFunc)NULL,        	/* getSecRes data	*/
    { 0 },     				/* fastSubclass flags	*/
    (XtArgsProc)NULL,			/* getValuesPrehook	*/
    (XtArgsProc)NULL,			/* getValuesPosthook	*/
    (XtWidgetClassProc)NULL,               /* classPartInitPrehook */
    (XtWidgetClassProc)NULL,               /* classPartInitPosthook*/
    NULL,               /* ext_resources        */
    NULL,               /* compiled_ext_resources*/
    0,                  /* num_ext_resources    */
    FALSE,              /* use_sub_resources    */
    (XmWidgetNavigableProc)NULL,               /* widgetNavigable      */
    (XmFocusChangeProc)NULL,               /* focusChange          */
    (XmWrapperData)NULL	/* wrapperData		*/
};


#ifdef DEC_MOTIF_BUG_FIX
externaldef(xmdisplayclassrec) XmDisplayClassRec xmDisplayClassRec = {
#else
XmDisplayClassRec xmDisplayClassRec = {
#endif
    {	
	(WidgetClass) &applicationShellClassRec,	/* superclass		*/   
	"XmDisplay",			/* class_name 		*/   
	sizeof(XmDisplayRec),	 	/* size 		*/   
	DisplayClassInitialize,		/* Class Initializer 	*/   
	DisplayClassPartInitialize,	/* class_part_init 	*/ 
	FALSE, 				/* Class init'ed ? 	*/   
	DisplayInitialize,		/* initialize         	*/   
	(XtArgsProc)NULL,		/* initialize_notify    */ 
	XtInheritRealize,		/* realize            	*/   
	NULL,	 			/* actions            	*/   
	0,				/* num_actions        	*/   
	resources,			/* resources          	*/   
	XtNumber(resources),		/* resource_count     	*/   
	NULLQUARK, 			/* xrm_class          	*/   
	FALSE, 				/* compress_motion    	*/   
	FALSE, 				/* compress_exposure  	*/   
	FALSE, 				/* compress_enterleave	*/   
	FALSE, 				/* visible_interest   	*/   
	DisplayDestroy,			/* destroy            	*/   
	(XtWidgetProc)NULL, 		/* resize             	*/   
	(XtExposeProc)NULL, 		/* expose             	*/   
	(XtSetValuesFunc)NULL, 		/* set_values         	*/   
	(XtArgsFunc)NULL, 		/* set_values_hook      */ 
	(XtAlmostProc)NULL,	 	/* set_values_almost    */ 
	(XtArgsProc)NULL,		/* get_values_hook      */ 
	(XtAcceptFocusProc)NULL, 	/* accept_focus       	*/   
	XtVersion, 			/* intrinsics version 	*/   
	NULL, 				/* callback offsets   	*/   
	NULL,				/* tm_table           	*/   
	(XtGeometryHandler)NULL, 	/* query_geometry       */ 
	(XtStringProc)NULL, 		/* display_accelerator  */ 
	(XtPointer)&baseClassExtRec, 	/* extension            */ 
    },	
    { 					/* composite class record */
	(XtGeometryHandler)NULL,	/* geometry_manager 	*/
	(XtWidgetProc)NULL,		/* change_managed	*/
	DisplayInsertChild,		/* insert_child		*/
	DisplayDeleteChild, 		/* from the shell 	*/
	NULL, 				/* extension record     */
    },
    { 					/* shell class record 	*/
	NULL, 				/* extension record     */
    },
    { 					/* wm shell class record */
	NULL, 				/* extension record     */
    },
    { 					/* vendor shell class record */
	NULL,				/* extension record     */
    },
    { 					/* toplevelclass record */
	NULL, 				/* extension record     */
    },
    { 					/* appShell record 	*/
	NULL, 				/* extension record     */
    },
    {					/* Display class	*/
	GetDisplay,			/* GetDisplay		*/
	NULL,				/* extension		*/
    },
};

externaldef(xmdisplayclass) WidgetClass 
      xmDisplayClass = (WidgetClass) (&xmDisplayClassRec);



static void 
#ifdef _NO_PROTO
DisplayClassPartInitialize(wc)
	WidgetClass wc;
#else
DisplayClassPartInitialize(
	WidgetClass wc )
#endif /* _NO_PROTO */
{
	_XmFastSubclassInit(wc, XmDISPLAY_BIT);
}

static void 
#ifdef _NO_PROTO
DisplayClassInitialize()
#else
DisplayClassInitialize( void )
#endif /* _NO_PROTO */
{
	baseClassExtRec.record_type = XmQmotif;
    _Xm_MOTIF_DRAG_AND_DROP_MESSAGE =
		XmMakeCanonicalString("_MOTIF_DRAG_AND_DROP_MESSAGE");
}    

/*ARGSUSED*/
static void 
#ifdef _NO_PROTO
SetDragReceiverInfo( w, client_data, event, dontSwallow )
        Widget w ;
        XtPointer client_data ;
        XEvent *event ;
        Boolean *dontSwallow ;
#else
SetDragReceiverInfo(
        Widget w,
        XtPointer client_data,
        XEvent *event,
        Boolean *dontSwallow )
#endif /* _NO_PROTO */
{
    XmDisplay	dd = (XmDisplay) XmGetXmDisplay(XtDisplay(w));

    if (XtIsRealized(w)) {
	_XmSetDragReceiverInfo(dd, (Widget)client_data);
	XtRemoveEventHandler(w, StructureNotifyMask, False,
			     SetDragReceiverInfo,
			     client_data);
    }
}

/*
 * this routine is registered on the XmNtreeUpdateProc resource of the
 * dropSiteManager.  It is called whenever the tree is changed.
 */
/*ARGSUSED*/
static void 
#ifdef _NO_PROTO
TreeUpdateHandler( w, client, call )
        Widget w ;
        XtPointer client ;
        XtPointer call ;
#else
TreeUpdateHandler(
        Widget w,
        XtPointer client,
        XtPointer call )
#endif /* _NO_PROTO */
{
    XmAnyCallbackStruct	    	*anyCB = (XmAnyCallbackStruct *)call;
    XmDisplay	  	dd = (XmDisplay) XmGetXmDisplay(XtDisplay(w));

    if (dd->display.dragReceiverProtocolStyle == XmDRAG_NONE)
		return;

    switch(anyCB->reason) {
      case XmCR_DROP_SITE_TREE_ADD:
	{
	    XmDropSiteTreeAddCallback cb =
	      (XmDropSiteTreeAddCallback)anyCB;

	    if (XtIsRealized(cb->rootShell)) {
		_XmSetDragReceiverInfo(dd, cb->rootShell);
	    }
	    else {
		XtAddEventHandler(cb->rootShell, 
				  StructureNotifyMask, False,
				  SetDragReceiverInfo,
				  (XtPointer)cb->rootShell);
	    }
	    /*
	     * ClientMessages are not maskable so all we have to
	     * do is indicate interest in non-maskable events.
	     */
	    XtAddEventHandler(cb->rootShell, NoEventMask, True,
			      ReceiverShellExternalSourceHandler,
			      (XtPointer)dd);
	}
	break;
      case XmCR_DROP_SITE_TREE_REMOVE:
	{
	    XmDropSiteTreeRemoveCallback cb =
	      (XmDropSiteTreeRemoveCallback)anyCB;
	    XtRemoveEventHandler(cb->rootShell, NoEventMask, True,
				 ReceiverShellExternalSourceHandler,
				 (XtPointer)dd);
	    if (XtIsRealized(cb->rootShell))
	      _XmClearDragReceiverInfo(cb->rootShell);
	}
	break;
      default:
	break;
    }
}

/************************************************************************
 *
 *  DisplayInitialize
 *
 ************************************************************************/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
DisplayInitialize( requested_widget, new_widget, args, num_args )
        Widget requested_widget ;
        Widget new_widget ;
        ArgList args ;
        Cardinal *num_args ;
#else
DisplayInitialize(
        Widget requested_widget,
        Widget new_widget,
        ArgList args,
        Cardinal *num_args )
#endif /* _NO_PROTO */
{
    XmDisplay	xmDisplay = (XmDisplay)new_widget;
    Arg		lclArgs[1];

    xmDisplay->display.shellCount = 0;

    xmDisplay->display.numModals = 0;
    xmDisplay->display.modals = NULL;
    xmDisplay->display.maxModals = 0;
    xmDisplay->display.userGrabbed = False;
    xmDisplay->display.activeDC = NULL;

	xmDisplay->display.proxyWindow =
		_XmGetDragProxyWindow(XtDisplay(xmDisplay));

    XtSetArg(lclArgs[0], XmNtreeUpdateProc, TreeUpdateHandler);
    xmDisplay->display.dsm = (XmDropSiteManagerObject)
      XtCreateWidget("dsm", 
		     xmDisplay->display.dropSiteManagerClass,
		     (Widget) xmDisplay, lclArgs, 1);

    _XmInitByteOrderChar();
    xmDisplay->display.xmim_info = NULL;

    xmDisplay->display.displayInfo = (XtPointer) XtNew(XmDisplayInfo);
    ((XmDisplayInfo *)(xmDisplay->display.displayInfo))->SashCursor = 0L;
    ((XmDisplayInfo *)(xmDisplay->display.displayInfo))->TearOffCursor = 0L;
    ((XmDisplayInfo *)(xmDisplay->display.displayInfo))->destinationWidget= 
	(Widget)NULL;

    _XmVirtKeysInitialize (new_widget);

    if (displayContext == 0)
      displayContext = XUniqueContext();
	
	if (! XFindContext(XtDisplay(xmDisplay), None, displayContext,
		(char **) &xmDisplay))
	{
		/*
		 * There's one already created for this display.
		 * What should we do?  If we destroy the previous one, we may
		 * wreak havoc with shell modality and screen objects.  BUT,
		 * Xt doesn't really give us a way to abort a create.  We'll
		 * just let the new one dangle.
		 */

		_XmWarning((Widget) xmDisplay,
"Creating multiple XmDisplays for the same X display.  Only the\n\
first XmDisplay created for a particular X display can be referenced\n\
by calls to XmGetXmDisplay");
	}
	else
	{
		XSaveContext(XtDisplayOfObject((Widget)xmDisplay),
			 None,
			 displayContext,
			 (char *)xmDisplay);
	}
}


/************************************************************************
 *
 *  DisplayInsertChild
 *
 ************************************************************************/
static void 
#ifdef _NO_PROTO
DisplayInsertChild( w )
        Widget w ;
#else
DisplayInsertChild(
        Widget w )
#endif /* _NO_PROTO */
{
	if (XtIsRectObj(w))
		(* ((CompositeWidgetClass)compositeWidgetClass)
			->composite_class.insert_child) (w);
}


/************************************************************************
 *
 *  DisplayDeleteChild
 *
 ************************************************************************/
static void 
#ifdef _NO_PROTO
DisplayDeleteChild( w )
        Widget w ;
#else
DisplayDeleteChild(
        Widget w )
#endif /* _NO_PROTO */
{
	if (XtIsRectObj(w))
		(* ((CompositeWidgetClass)compositeWidgetClass)
			->composite_class.delete_child) (w);
}

/************************************************************************
 *
 *  DisplayDestroy
 *
 ************************************************************************/
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
DisplayDestroy( w )
        Widget w ;
#else
DisplayDestroy(
        Widget w )
#endif /* _NO_PROTO */
{
    XmDisplay dd = (XmDisplay) w ;

    XtFree((char *) dd->display.modals);
    XtFree((char *) dd->display.displayInfo);

    _XmVirtKeysDestroy (w);

    XDeleteContext( XtDisplay( w), None, displayContext) ;
}

/*ARGSUSED*/
XmDropSiteManagerObject 
#ifdef _NO_PROTO
_XmGetDropSiteManagerObject( xmDisplay )
        XmDisplay xmDisplay ;
#else
_XmGetDropSiteManagerObject(
        XmDisplay xmDisplay )
#endif /* _NO_PROTO */
{
 
    return(xmDisplay->display.dsm);
}


unsigned char 
#ifdef _NO_PROTO
_XmGetDragProtocolStyle( w )
        Widget w ;
#else
_XmGetDragProtocolStyle(
        Widget w )
#endif /* _NO_PROTO */
{
    XmDisplay		xmDisplay;
    unsigned char	style;

    xmDisplay = (XmDisplay) XmGetXmDisplay(XtDisplay(w));

    switch(xmDisplay->display.dragReceiverProtocolStyle) {
	  case XmDRAG_NONE:
	  case XmDRAG_DROP_ONLY:
	    style = XmDRAG_NONE;
	    break;
	  case XmDRAG_DYNAMIC:
	    style = XmDRAG_DYNAMIC;
	    break;
	  case XmDRAG_PREFER_DYNAMIC:
	  case XmDRAG_PREFER_PREREGISTER:
	  case XmDRAG_PREREGISTER:
	    style = XmDRAG_PREREGISTER;
	    break;
	}
    return style;
}

Widget 
#ifdef _NO_PROTO
XmGetDragContext( w, time )
        Widget w ;
        Time time ;
#else
XmGetDragContext(
        Widget w,
        Time time )
#endif /* _NO_PROTO */
{
	XmDisplay		xmDisplay;
	XmDragContext	matchedDC = NULL, dc = NULL;
	Cardinal		i;

	xmDisplay = (XmDisplay)XmGetXmDisplay(XtDisplay(w));
	for(i = 0; i < xmDisplay->composite.num_children; i++)
	{
		dc = (XmDragContext)(xmDisplay->composite.children[i]);
		if ((XmIsDragContext((Widget) dc)) && (CHECK_TIME(dc, time)) &&
			((!matchedDC) ||
				(matchedDC->drag.dragStartTime
					< dc->drag.dragStartTime)) &&
			!dc->core.being_destroyed)
			matchedDC = dc;
	}
	return((Widget)matchedDC);
}

Widget 
#ifdef _NO_PROTO
_XmGetDragContextFromHandle( w, iccHandle )
        Widget w ;
        Atom iccHandle ;
#else
_XmGetDragContextFromHandle(
        Widget w,
        Atom iccHandle )
#endif /* _NO_PROTO */
{
	XmDisplay		xmDisplay;
	XmDragContext	dc;
	Cardinal		i;

	xmDisplay = (XmDisplay)XmGetXmDisplay(XtDisplay(w));

	for(i = 0; i < xmDisplay->composite.num_children; i++)
	{
		dc = (XmDragContext)(xmDisplay->composite.children[i]);
		if ((XmIsDragContext((Widget) dc)) && 
			(dc->drag.iccHandle == iccHandle) &&
			!dc->core.being_destroyed)
			return((Widget)dc);
	}
	return(NULL);
}




static XmDragContext 
#ifdef _NO_PROTO
FindDC( xmDisplay, time, sourceIsExternal )
        XmDisplay xmDisplay ;
        Time time ;
        Boolean sourceIsExternal ;
#else
FindDC(
        XmDisplay xmDisplay,
        Time time,
#if NeedWidePrototypes
        int sourceIsExternal )
#else
        Boolean sourceIsExternal )
#endif /* NeedWidePrototypes */
#endif /* _NO_PROTO */
{
	XmDragContext	dc = NULL;
	Cardinal			i;

	for(i = 0; i < xmDisplay->composite.num_children; i++)
	{
		dc = (XmDragContext)(xmDisplay->composite.children[i]);
		if ((XmIsDragContext((Widget) dc)) &&
			(CHECK_TIME(dc, time)) &&
			(dc->drag.sourceIsExternal == sourceIsExternal) &&
			!dc->core.being_destroyed)
			return(dc);
	}
	return(NULL);
}

/*ARGSUSED*/
static int 
#ifdef _NO_PROTO
isMine( dpy, event, arg )
        Display *dpy ;
        register XEvent *event ;
        char *arg ;
#else
isMine(
        Display *dpy,
        register XEvent *event,
        char *arg )
#endif /* _NO_PROTO */
{
	XmDisplayEventQueryStruct 	*q = (XmDisplayEventQueryStruct *) arg;
	XmICCCallbackStruct		callback, *cb = &callback;

	/* Once we have a drop start we must stop looking at the queue */
	if (q->hasDropStart)
		return(False);

	if (!_XmICCEventToICCCallback((XClientMessageEvent *)event, 
			cb, XmICC_INITIATOR_EVENT))
		return(False);

	if ((cb->any.reason == XmCR_DROP_SITE_ENTER) ||
		(cb->any.reason == XmCR_DROP_SITE_LEAVE))
	{
		/*
		 * We must assume this to be a dangling set of messages, so
		 * we will quietly consume it in the interest of hygene.
		 */
		return(True);
	}


	if (!q->dc)
		q->dc = FindDC(q->dd, cb->any.timeStamp, True);
	/*
	 * if we can find a dc then we have already processed
	 * an enter for this shell.
	 */

	switch(cb->any.reason)
	{
		case XmCR_TOP_LEVEL_ENTER:
			q->hasLeave = False;
			if (q->dc == NULL)
			{
				*(q->enterCB) = *(XmTopLevelEnterCallbackStruct *)cb;
				q->hasEnter = True;
			}
		break;
		case XmCR_TOP_LEVEL_LEAVE:
			q->hasEnter = False;
			if (q->dc != NULL)
			{
				*(q->leaveCB) = *(XmTopLevelLeaveCallbackStruct *)cb;
				q->hasLeave = True;
				q->hasMotion = False;
			}
			else
			{
			_XmWarning((Widget) q->dd,
				"Received TOP_LEVEL_LEAVE with no active DragContext");
			}
		break;
		case XmCR_DRAG_MOTION:
			*(q->motionCB) = *(XmDragMotionCallbackStruct *)cb;
			q->hasMotion = True;
		break;
		case XmCR_DROP_START:
			*(q->dropStartCB) = *(XmDropStartCallbackStruct *)cb;
			q->hasDropStart = True;
		break;
		default:
		break;
	}
	return True;
}

/*
 * this handler is used to catch messages from external drag
 * contexts that want to map motion or drop events
 */
/*ARGSUSED*/
static void 
#ifdef _NO_PROTO
ReceiverShellExternalSourceHandler( w, client_data, event, dontSwallow )
        Widget w ;
        XtPointer client_data ;
        XEvent *event ;
        Boolean *dontSwallow ;
#else
ReceiverShellExternalSourceHandler(
        Widget w,
        XtPointer client_data,
        XEvent *event,
        Boolean *dontSwallow )
#endif /* _NO_PROTO */
{
    XmDragTopLevelClientDataStruct 	topClientData;
    XmTopLevelEnterCallbackStruct	enterCB;
    XmTopLevelLeaveCallbackStruct	leaveCB;
    XmDropStartCallbackStruct		dropStartCB;
    XmDragMotionCallbackStruct		motionCB;
    XmDisplayEventQueryStruct		 		q;	
    Widget	  			shell = w;
    XmDisplay			dd = (XmDisplay) XmGetXmDisplay(XtDisplay(w));
    XmDragContext			dc;
    XmDropSiteManagerObject		dsm = dd->display.dsm;

    /*
     * If dd has an active dc then we are the initiator.  We shouldn't
     * do receiver processing as the initiator, so bail out.
     */
    if (dd->display.activeDC != NULL)
	return;

    q.dd = dd;
    q.dc = (XmDragContext)NULL;
    q.enterCB = &enterCB;
    q.leaveCB = &leaveCB;
    q.motionCB = &motionCB;
    q.dropStartCB = &dropStartCB;
    q.hasEnter =
      q.hasDropStart = 
	q.hasLeave = 
	  q.hasMotion = False;
    /*
     * Since this event handler gets called for all non-maskable events,
     * we want to bail out now with don't swallow if we don't want this
     * event.  Otherwise we'll swallow it.
     */
    
     /*
	  * process the event that fired this event handler.
	  * If it's not a Receiver DND event, bail out.
	  */
    if (!isMine(XtDisplayOfObject(w), event, (char*)&q))
		return;

    /*
     * swallow all the pending messages inline except the last motion
     */
    while (XCheckIfEvent( XtDisplayOfObject(w), event, isMine, (char*)&q)) {
    }

    dc = q.dc;

    if (q.hasEnter || q.hasMotion || q.hasDropStart || q.hasLeave) {
	/*
	 * handle a dangling leave first
	 */
	if (q.hasLeave) 
	  {
	      XmTopLevelLeaveCallback	callback = &leaveCB;
	      
	      topClientData.destShell = shell;
	      topClientData.xOrigin = shell->core.x;
	      topClientData.yOrigin = shell->core.y;
	      topClientData.sourceIsExternal = True;
	      topClientData.iccInfo = NULL;
	      topClientData.window = XtWindow(w);
	      topClientData.dragOver = NULL;
	      
	      _XmDSMUpdate(dsm, 
			   (XtPointer)&topClientData, 
			   (XtPointer)callback);
	      /* destroy it if no drop. otherwise done in dropTransfer */
	      if (!q.hasDropStart)
	      {
		XtDestroyWidget((Widget)dc);
		q.dc = dc = NULL;
	      }
	  }
	/*
	 * check for a dropStart from a preregister client or an enter
	 * either of which require a dc to be alloced
	 */
	if (q.hasEnter || q.hasDropStart) {
	    if (!q.dc) {
		Arg		args[4];
		Cardinal	i = 0;
		Time		timeStamp;
		Window		window;
		Atom		iccHandle;

		if (q.hasDropStart) {
		    XmDropStartCallback	dsCallback = &dropStartCB;
		    
		    timeStamp = dsCallback->timeStamp;
		    window = dsCallback->window;
		    iccHandle = dsCallback->iccHandle;
		}
		else {
		    XmTopLevelEnterCallback teCallback = &enterCB;	    
		    
		    timeStamp = teCallback->timeStamp;
		    window = teCallback->window;
		    iccHandle = teCallback->iccHandle;
		}
		XtSetArg(args[i], XmNsourceWindow, window);i++;
		XtSetArg(args[i], XmNsourceIsExternal,True);i++;
		XtSetArg(args[i], XmNstartTime, timeStamp);i++;
		XtSetArg(args[i], XmNiccHandle, iccHandle);i++;
		dc = (XmDragContext) XtCreateWidget("dragContext", 
			dd->display.dragContextClass, (Widget)dd, args, i);
		_XmReadInitiatorInfo((Widget)dc);
		/*
		 * force in value for dropTransfer to use in selection
		 * calls.
		 */
		dc->drag.currReceiverInfo =
		  _XmAllocReceiverInfo(dc);
		dc->drag.currReceiverInfo->shell = shell;
	    }
	    topClientData.destShell = shell;
	    topClientData.xOrigin = XtX(shell);
	    topClientData.yOrigin = XtY(shell);
	    topClientData.width = XtWidth(shell);
	    topClientData.height = XtHeight(shell);
	    topClientData.sourceIsExternal = True;
	    topClientData.iccInfo = NULL;
	}

	if (!dc) return;

	if (q.hasDropStart) {
	    dc->drag.dragFinishTime = dropStartCB.timeStamp;
	    _XmDSMUpdate(dsm,
			 (XtPointer)&topClientData, 
			 (XtPointer)&dropStartCB);
	}
	/* 
	 * we only see enters if they're not matched with a leave
	 */
	if (q.hasEnter) {
	    _XmDSMUpdate(dsm,
			 (XtPointer)&topClientData, 
			 (XtPointer)&enterCB);
	}
	if (q.hasMotion) {
	    XmDragMotionCallback	callback = &motionCB;
	    XmDragMotionClientDataStruct	motionData;
	    
	    motionData.window = XtWindow(w);
	    motionData.dragOver = NULL;
	    _XmDSMUpdate(dsm, (XtPointer)&motionData, (XtPointer)callback);
	}
    }
}

static Widget 
#ifdef _NO_PROTO
GetDisplay( display )
        Display *display ;
#else
GetDisplay(
        Display *display )
#endif /* _NO_PROTO */
{
	XmDisplay	xmDisplay = NULL;
	Arg args[3];
	int n;

	if ((displayContext == 0) ||
		(XFindContext(display, None, displayContext,
			(char **) &xmDisplay)))
	{
		String	name, w_class;

		XtGetApplicationNameAndClass(display, &name, &w_class);

		n = 0;
		XtSetArg(args[n], XmNmappedWhenManaged, False); n++;
		XtSetArg(args[n], XmNwidth, 1); n++;
		XtSetArg(args[n], XmNheight, 1); n++;
		xmDisplay = (XmDisplay) XtAppCreateShell(name, w_class,
			xmDisplayClass, display, args, n);
	}

	/* We need a window to be useful */
	if (!XtIsRealized((Widget)xmDisplay))
		XtRealizeWidget((Widget)xmDisplay);

	return ((Widget)xmDisplay);
}

Widget
#ifdef _NO_PROTO
XmGetXmDisplay( display )
        Display *display ;
#else
XmGetXmDisplay(
        Display *display )
#endif /* _NO_PROTO */
{
	XmDisplayClass dC;

	/*
	 * We have a chicken and egg problem here; we'd like to get
	 * the display via a class function, but we don't know which
	 * class to use.  Hence the magic functions _XmGetXmDisplayClass
	 * and _XmSetXmDisplayClass.
	 */
	
	dC = (XmDisplayClass) _XmGetXmDisplayClass();

	return((*(dC->display_class.GetDisplay))(display));
}

/*
 * It would be nice if the next two functions were methods, but
 * for obvious reasons they're not.
 */

WidgetClass
#ifdef _NO_PROTO
_XmGetXmDisplayClass()
#else
_XmGetXmDisplayClass( void )
#endif /* _NO_PROTO */
{
	if (curDisplayClass == NULL)
		curDisplayClass = xmDisplayClass;
	return(curDisplayClass);
}

WidgetClass
#ifdef _NO_PROTO
_XmSetXmDisplayClass(wc)
	WidgetClass wc;
#else
_XmSetXmDisplayClass(
	WidgetClass wc )
#endif /* _NO_PROTO */
{
	WidgetClass oldDisplayClass = curDisplayClass;
	WidgetClass sc = wc;

	/*
	 * We aren't going to let folks just set any old class in as the
	 * display class.  They will have to use subclasses of xmDisplay.
	 */
	while ((sc != NULL) && (sc != xmDisplayClass))
		sc = sc->core_class.superclass;

	if (sc != NULL)
		curDisplayClass = wc;
	else
		XtWarning("Cannot set XmDisplay class to a non-subclass of XmDisplay");
	return(oldDisplayClass);
}


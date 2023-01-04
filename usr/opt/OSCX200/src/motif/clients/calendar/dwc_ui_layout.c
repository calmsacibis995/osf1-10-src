/* dwc_ui_layout.c */
#ifndef lint
static char rcsid[] = "$Id$";
#endif /* lint */
/*    
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows general purpose.
**
**  AUTHOR:
**
**	Marios Cleovoulou, January-1988
**
**  ABSTRACT:
**
**	This module implements the "widgetness" of the layout widget.
**
**--
*/

#include "dwc_compat.h"

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/StringDefs.h>	/* for XtCCallbacks, etc. */
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_layout.h"

/* Declare routines static to this module */
static Boolean AcceptFocus PROTOTYPE ((LayoutWidget lw, Time *time));

static void ClassInitialize PROTOTYPE ((void));

static void Destroy PROTOTYPE ((Widget	w));

static XtGeometryResult geometry_manager PROTOTYPE ((
	Widget			w,
	XtWidgetGeometry	*request,
	XtWidgetGeometry	*reply));

static void Initialize PROTOTYPE ((LayoutWidget request, LayoutWidget new));

#if !defined(AUD)
static
#endif
void LayoutHelp PROTOTYPE ((Widget lw, XEvent *event));

static void managed_set_changed PROTOTYPE ((Widget w));

static void Realize PROTOTYPE ((
	Widget			w,
	XtValueMask		*window_mask,
	XSetWindowAttributes	*window_attributes));

static void Redisplay PROTOTYPE ((
	Widget		w,
	XExposeEvent	*ee));

static void Resize PROTOTYPE ((Widget w));

static Boolean SetValues PROTOTYPE ((Widget cur, Widget req, Widget new));

static char translations [] =
    "@Help<Btn1Down>:	    LayoutHelp()";

static XtActionsRec action_table [] =
{
    {"LayoutHelp",	    (XtActionProc)LayoutHelp},
    {NULL,		    NULL}
};

static Boolean default_default_positioning = TRUE;

static XtResource resources [] =
{
   {LwNresizeCallback,	     XtCCallback,		XtRCallback,
    sizeof (XtCallbackList), XtOffset (LayoutWidget, layout.resize_callback),
    XtRCallback,	     NULL},

   {LwNgeometryCallback,     XtCCallback,		XtRCallback,
    sizeof (XtCallbackList), XtOffset (LayoutWidget, layout.geometry_callback),
    XtRCallback,	     NULL},

   {LwNfocusCallback,	     XtCCallback,		XtRCallback,
    sizeof (XtCallbackList), XtOffset (LayoutWidget, layout.focus_callback),
    XtRCallback,	     NULL},

   {LwNtag,		     LwCTag,		        XtRPointer,
    sizeof (caddr_t),        XtOffset (LayoutWidget, layout.tag),
    XtRPointer,		     NULL},

   {LwNdefaultPositioning,   XtCBoolean,                XtRBoolean,
    sizeof (Boolean),        XtOffset (LayoutWidget, layout.default_positioning),
    XtRBoolean,		     &default_default_positioning},

   {XmNmarginWidth,	    XmCMarginWidth,		XtRDimension,
    sizeof (Dimension),	    XtOffset (LayoutWidget, layout.margin_width),
    XtRImmediate,	    (caddr_t)10},

   {XmNmarginHeight,	    XmCMarginHeight,		XtRDimension,
    sizeof (Dimension),	    XtOffset (LayoutWidget, layout.margin_height),
    XtRImmediate,	    (caddr_t)10},

   {XmNshadowType,	    XmCShadowType,		XmRShadowType,
    sizeof(unsigned char),  XtOffset (LayoutWidget, layout.shadow_type),
    XmRImmediate,	    (caddr_t)XmSHADOW_OUT}
};

/*
** Static initialization of the menu widget class record, must do each field
*/         

LayoutClassRec layoutwidgetclassrec =
{
    /*
    ** Core Class record
    */
    {
	(WidgetClass) &xmManagerClassRec, /* superclass ptr		    */
	"Layout",			/* class_name			    */
	sizeof (LayoutWidgetRec),	/* size of layout widget instance   */
	ClassInitialize,		/* class init proc		    */
	NULL,				/* Class Part Initialize	    */
	FALSE,				/* Class is not initialised	    */
	(XtInitProc)Initialize,		/* Widget init proc		    */
	NULL,				/* Initialise hook		    */
	Realize,			/* Widget realise proc		    */
	action_table,			/* Class Action Table		    */
	XtNumber (action_table),
	resources,			/* this class's resource list	    */
	XtNumber (resources),		/*  "	  " resource     	    */
	NULLQUARK,			/* xrm_class			    */
	TRUE,				/* class: compressed motion	    */
	TRUE,				/* class: compressed exposure	    */
	TRUE,				/* class: compressed enterleave	    */
	FALSE,				/* class: no VisibilityNotify	    */
	Destroy,			/* class destroy proc		    */
	Resize,				/* class resize proc		    */
	(XtExposeProc)Redisplay,	/* class expose proc		    */
	(XtSetValuesFunc)SetValues,	/* class set_value proc		    */
	NULL,				/* set values hook		    */
	XtInheritSetValuesAlmost,	/* set values almost		    */
	NULL,				/* get values hook		    */
	(XtAcceptFocusProc)AcceptFocus,	/* class accept focus proc	    */
	XtVersion,			/* version			    */
	NULL,				/* Callback Offsets		    */
	translations,			/* Tm_table			    */
	NULL,
	NULL,				/* disp accelerators		    */	
	NULL				/* extension			    */ 
    },

    /*
    **  Composite class record
    */
    {
	geometry_manager,		/* geometry_manager */
	managed_set_changed,		/* change_managed */
	XtInheritInsertChild,		/* insert_child */
	XtInheritDeleteChild,		/* delete_child */
	NULL				/* extension */
    },
    
    /*
    **  Constraint class record
    */
    {
	NULL,				/* subresources */
	0,				/* subresource_count */	
	0,				/* constraint_size */	
	NULL,				/* initialize */
	NULL,				/* destroy */
	NULL,				/* set values */
	NULL				/* extension */
    },

    /*	  
    **  XmManager Class fields
    */	  
    {
	XtInheritTranslations,		/* default translations */
	NULL,				/* syn_resources */
	0,				/* num_syn_resources */
	NULL,				/* syn_cont_resources */
	0,				/* num_syn_cont_resources */
	XmInheritParentProcess,		/* parent_process */
	NULL				/* extension */
    },

    /*
    ** Layout Class record
    */
    {
	NULL,
	0				/* just a dummy field */
    }
};

LayoutClass layoutwidgetclass = &layoutwidgetclassrec;

LayoutWidget LwLayoutCreate
#ifdef _DWC_PROTO_
	(
	Widget	parent,
	char	*name,
	Arg	*arglist,
	int	argcount)
#else	/* no prototypes */
	(parent, name, arglist, argcount)
	Widget	parent;
	char	*name;
	Arg	*arglist;
	int	argcount;
#endif	/* prototype */
{

#ifdef DEBUG
    printf ("LayoutWidget -- LwLayoutCreate Called\n");
#endif

    return
    (
	(LayoutWidget)XtCreateWidget
	    (name, (WidgetClass)layoutwidgetclass, parent, arglist, argcount)
    );

}

LayoutWidget LwLayout
#ifdef _DWC_PROTO_
	(
	Widget		parent,
	char		*name,
	Boolean		default_pos,
	Position	x,
	Position	y,
	Dimension	width,
	Dimension	height,
	XtCallbackList	resize_callback,
	XtCallbackList	geometry_callback,
	XtCallbackList	focus_callback,
	XtCallbackList	help_callback,
	dwcaddr_t		tag)
#else	/* no prototypes */
	(parent, name, default_pos, x, y, width, height, resize_callback,
	geometry_callback, focus_callback, help_callback, tag)
	Widget		parent;
	char		*name;
	Boolean		default_pos;
	Position	x;
	Position	y;
	Dimension	width;
	Dimension	height;
	XtCallbackList	resize_callback;
	XtCallbackList	geometry_callback;
	XtCallbackList	focus_callback;
	XtCallbackList	help_callback;
	dwcaddr_t		tag;
#endif	/* prototype */
{
    Arg		    arglist[15];
    Cardinal	    ac;

#ifdef DEBUG
    printf ("LayoutWidget -- LwLayout Called\n");
#endif

    ac = 0;
    XtSetArg (arglist [ac], LwNdefaultPositioning,  default_pos);	ac++;
    XtSetArg (arglist [ac], XtNx,		    x);			ac++;
    XtSetArg (arglist [ac], XtNy,		    y);			ac++;
    XtSetArg (arglist [ac], XtNwidth,		    width);		ac++;
    XtSetArg (arglist [ac], XtNheight,		    height);		ac++;
    XtSetArg (arglist [ac], LwNresizeCallback,	    resize_callback);	ac++;
    XtSetArg (arglist [ac], LwNgeometryCallback,    geometry_callback);	ac++;
    XtSetArg (arglist [ac], LwNfocusCallback,	    focus_callback);	ac++;
    XtSetArg (arglist [ac], XmNhelpCallback,	    help_callback);	ac++;
    XtSetArg (arglist [ac], LwNtag,		    tag);		ac++;

    return
    (
	(LayoutWidget)XtCreateWidget
	    (name, (WidgetClass)layoutwidgetclass, parent, arglist, ac)
    );
}

static void ClassInitialize
#ifdef _DWC_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{

#ifdef DEBUG
    printf ("LayoutWidget -- ClassInitialize Called\n");
#endif
    XmResolvePartOffsets
    (
	(WidgetClass)layoutwidgetclass,
	&layoutwidgetclassrec.layout_class.layoutoffsets
    );

}

static void Initialize
#ifdef _DWC_PROTO_
	(
	LayoutWidget	request,
	LayoutWidget	new)
#else	/* no prototypes */
	(request, new)
	LayoutWidget	request;
	LayoutWidget	new;
#endif	/* prototype */
{

#ifdef DEBUG
    printf ("LayoutWidget -- Initialize Called\n");
#endif

    /*
    **  Setup default widget size if width or height not specified.  
    */
    if (XtWidth (new) <= 0)
    {
	XtWidth (new) = MAX (0, XtWidth (XtParent (new)));
    }
    
    if (XtHeight (new) <= 0)
    {
	XtHeight (new) = MAX (0, XtHeight (XtParent (new)));
    }
    
    /*
    **  Setup default widget position if required.  Centre in parent window.
    */
    if (new->layout.default_positioning)
    {
	XtX (new) = (XtWidth  (XtParent (new)) - XtWidth  (new)) / 2;
	XtY (new) = (XtHeight (XtParent (new)) - XtHeight (new)) / 2;
    }

}

static void Realize
#ifdef _DWC_PROTO_
	(
	Widget			w,
	XtValueMask		*window_mask,
	XSetWindowAttributes	*window_attributes)
#else	/* no prototypes */
	(w, window_mask, window_attributes)
	Widget			w;
	XtValueMask		*window_mask;
	XSetWindowAttributes	*window_attributes;
#endif	/* prototype */
{

#ifdef DEBUG
    printf ("LayoutWidget -- Realize Called\n");
#endif

    XtCreateWindow
	(w, InputOutput, CopyFromParent, *window_mask, window_attributes);

}

static void Destroy
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{

#ifdef DEBUG
    printf ("LayoutWidget -- Destroy Called\n");
#endif

    return;

}

static XtGeometryResult geometry_manager
#ifdef _DWC_PROTO_
	(
	Widget			w,
	XtWidgetGeometry	*request,
	XtWidgetGeometry	*reply)
#else	/* no prototypes */
	(w, request, reply)
	Widget			w;
	XtWidgetGeometry	*request;
	XtWidgetGeometry	*reply;
#endif	/* prototype */
{
    Position		x;
    Position		y;
    LwCallbackStruct	cb;
    LayoutWidget	lw = (LayoutWidget)XtParent(w);
    XmOffsetPtr		o = LwOffsetPtr(lw);
    
#ifdef DEBUG
    printf ("LayoutWidget -- geometry_manager Called\n");
#endif

    cb.reason        = LwCRGeometry;
    cb.result        = XtGeometryYes;
    cb.request       = request;
    cb.reply         = reply;

    /* execute the geometry callbacks in the parent */
    XtCallCallbackList ((Widget) lw, LwGeometryCallback(lw,o), &cb);


    /* we had a problem so return it */
    if (cb.result != XtGeometryYes)
    {
	return (cb.result);
    }


    if ((request->request_mode & (CWX | CWY)) != 0)
    {
	/* We've got a request to change the window origin relative to the  */
	/* parent */
	x = XtX (w);
	y = XtY (w);

	if ((request->request_mode & CWX) != 0)
	{
	    x = request->x;
	}

	if ((request->request_mode & CWY) != 0)
	{
	    y = request->y;
	}

	/*
	** move the window to match the request
	*/
	XtMoveWidget (w, x, y);
    }


    if ((request->request_mode & CWWidth) != 0)
    {
	w->core.width = request->width;
    }

    if ((request->request_mode & CWHeight) != 0)
    {
	w->core.height = request->height;
    }
    
    if ((request->request_mode & CWBorderWidth) != 0)
    {
	w->core.border_width = request->border_width;
    }

    /*
    **  Be agreeable...!
    */
    return (XtGeometryYes);
    
}

static void managed_set_changed
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{

#ifdef DEBUG
    printf ("LayoutWidget -- managed_set_changed Called\n");
#endif

    /*
    **  Don't care....
    */

}

static Boolean SetValues
#ifdef _DWC_PROTO_
	(
	Widget	cur,
	Widget	req,
	Widget	new)
#else	/* no prototypes */
	(cur, req, new)
	Widget	cur;
	Widget	req;
	Widget	new;
#endif	/* prototype */
{

#ifdef DEBUG
    printf ("LayoutWidget -- SetValues Called\n");
#endif

    return (FALSE);

}

void LwGotFocus
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    LayoutWidget    lw = (LayoutWidget) w;

    lw->layout.got_focus = TRUE;    

}

static Boolean AcceptFocus
#ifdef _DWC_PROTO_
	(
	LayoutWidget	lw,
	Time		*time)
#else	/* no prototypes */
	(lw, time)
	LayoutWidget	lw;
	Time		*time;
#endif	/* prototype */
{
    LwCallbackStruct	cb_args;
    XmOffsetPtr		o = LwOffsetPtr(lw);

    lw->layout.got_focus = FALSE;    
    cb_args.reason     = LwCRFocus;
    cb_args.time       = *time;
    XtCallCallbackList ((Widget) lw, LwFocusCallback(lw,o), &cb_args);

    return (lw->layout.got_focus);
}

static void Resize
#ifdef _DWC_PROTO_
	(Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    LwCallbackStruct	cb;
    LayoutWidget	lw = (LayoutWidget)w;
    XmOffsetPtr		o = LwOffsetPtr(lw);

#ifdef DEBUG
    printf ("LayoutWidget -- Resize Called\n");
#endif
    
    cb.reason        = LwCRResize;
    cb.width         = XtWidth (w);
    cb.height        = XtHeight (w);

/* debugging info to catch layout resize notification problem
printf("Layout Resize being called width: %d height: %d\n",
    cb.width,cb.height);
*/
    cb.border_width  = w->core.border_width;
    XtCallCallbackList ((Widget) w, LwResizeCallback(lw,o), &cb);

}

static void Redisplay
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XExposeEvent	*ee
	)
#else
	(w, ee)
	Widget		w;
	XExposeEvent	*ee;
#endif
{
    XmOffsetPtr	    o = LwOffsetPtr(w);
    LayoutWidget    lw = (LayoutWidget)w;    

    if (LwShadowThickness(w,o) > 0)
    {
	switch (LwShadowType(w,o))
	{
	case XmSHADOW_OUT:
	    _XmDrawShadow
	    (
		XtDisplay(w),
		XtWindow(w),
		LwTopShadowGC(w,o),
		LwBottomShadowGC(w,o),
		LwShadowThickness(w,o),
		0, 0, XtWidth(w), XtHeight(w)
	    );
	    break;
	case XmSHADOW_IN:
	    _XmDrawShadow
	    (
		XtDisplay(w),
		XtWindow(w),
		LwBottomShadowGC(w,o),
		LwTopShadowGC(w,o),
		LwShadowThickness(w,o),
		0, 0, XtWidth(w), XtHeight(w)
	    );
	    break;
	default:
#ifdef DEBUG
#if DEBUG
	    _XmDrawShadowType
	    (
		w,
		LwShadowType(w,o),
		LwShadowThickness(w,o),
		0,
		LwTopShadowGC(w,o),
		LwBottomShadowGC(w,o)
	    );
#endif
#endif
	    break;
	}
    }
}

#if !defined (AUD)
static
#endif
void LayoutHelp
#ifdef _DWC_PROTO_
	(
	Widget	w,
	XEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget	w;
	XEvent	*event;
#endif	/* prototype */
{
    LwCallbackStruct	cb_args;
    LayoutWidget	lw = (LayoutWidget)w;
    XmOffsetPtr		o = LwOffsetPtr(lw);

    cb_args.reason     = (int)XmCR_HELP;
    cb_args.event      = event;
    XtCallCallbackList ((Widget) lw, LwHelpCallback(lw,o), &cb_args);
    
}

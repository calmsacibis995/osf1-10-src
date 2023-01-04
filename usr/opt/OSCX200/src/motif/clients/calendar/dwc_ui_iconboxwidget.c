/* dwc_ui_iconboxwidget.c */
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
**	DECwindows Calendar Iconbox Display
**
**  AUTHOR:
**
**	Marios Cleovoulou, March-1989
**
**  ABSTRACT:
**
**	This module implements the iconbox display
**
**--
*/

#include "dwc_compat.h"

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/StringDefs.h>		/* for XtCForeground... */
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_iconboxwidgetp.h"
#include "dwc_ui_misc.h"		/* for MISCSetGCClipMask */

static void ClassInitialize PROTOTYPE ((void));

#if !defined (AUD)
static
#endif
void ACTION_IBW_CANCEL PROTOTYPE ((
	Widget	w,
	XEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_IBW_MB1UP PROTOTYPE ((
	Widget	w,
	XEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_IBW_HELP PROTOTYPE ((
	Widget	w,
	XEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_IBW_MB1MOTION PROTOTYPE ((
	Widget	w,
	XEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_IBW_MB1DOWN PROTOTYPE ((
	Widget	w,
	XEvent	*event));

static Boolean get_position_for_icon PROTOTYPE ((
	IconboxWidget	ibw,
	Cardinal	icon,
	XRectangle	*position));

static void redisplay_icon PROTOTYPE ((
	IconboxWidget	ibw,
	unsigned char	icon,
	XRectangle	*position));

static void redisplay_iconbox PROTOTYPE ((
	IconboxWidget	ibw,
	XRectangle	*expose_area));

static void Initialize PROTOTYPE ((
	Widget	request,
	Widget	new));

static void Realize PROTOTYPE ((
	Widget			w,
	XtValueMask		*window_mask,
	XSetWindowAttributes	*window_attributes));

static void Redisplay PROTOTYPE ((
	Widget		w,
	XExposeEvent	*ee,
	Region		region));

static void Resize PROTOTYPE ((
	Widget	w));

static Boolean SetValues PROTOTYPE ((
	Widget	o,
	Widget	r,
	Widget	n));

static void Destroy PROTOTYPE ((
	Widget	w));


/*
**  Translation table for Iconbox
*/

static char translations [] = 
   "@Help<BtnDown>:	 ACTION_IBW_HELP()\n\
    <Btn1Down>:	         ACTION_IBW_MB1DOWN()\n\
    Button1<Motion>:	 ACTION_IBW_MB1MOTION()\n\
    <Btn1Up>:		 ACTION_IBW_MB1UP()\n\
    Button1<BtnDown>:	 ACTION_IBW_CANCEL()\n\
    Button2<BtnDown>:	 ACTION_IBW_CANCEL()\n\
    Button3<BtnDown>:	 ACTION_IBW_CANCEL()\n\
    Button4<BtnDown>:	 ACTION_IBW_CANCEL()\n\
    Button5<BtnDown>:	 ACTION_IBW_CANCEL()";

/*
**  Action table - 'action' routines that may be called via translation table
*/

static XtActionsRec actions [] =
{
    {"ACTION_IBW_HELP",		(XtActionProc)ACTION_IBW_HELP},
    {"ACTION_IBW_MB1DOWN",	(XtActionProc)ACTION_IBW_MB1DOWN},
    {"ACTION_IBW_MB1MOTION",	(XtActionProc)ACTION_IBW_MB1MOTION},
    {"ACTION_IBW_MB1UP",	(XtActionProc)ACTION_IBW_MB1UP},
    {"ACTION_IBW_CANCEL",	(XtActionProc)ACTION_IBW_CANCEL},
    {NULL,			NULL}
};

static Dimension default_icon_width	    = 16;
static Dimension default_icon_height	    = 16;
static Dimension default_margin_width	    = 5;
static Dimension default_margin_height	    = 5;
static Dimension default_spacing	    = 5;
static Cardinal  default_zero		    = 0;
static DwcIbwIconboxStyle default_style     = DwcIbwSingleSelection;
static Boolean	 default_editable	    = TRUE;

/*
**  Resources for the Iconbox widget.  These are the 'public' widget
**  attributes that the caller can set.
*/
static XmPartResource resources [] =
{ 
    {DwcIbwNmarginWidth,	DwcIbwCMarginWidth,	    XtRDimension,
    sizeof (Dimension),		XmPartOffset (Iconbox, margin_width),
    XtRDimension,		(caddr_t) &default_margin_width},

   {DwcIbwNmarginHeight,	DwcIbwCMarginHeight,	    XtRDimension,
    sizeof (Dimension),		XmPartOffset (Iconbox, margin_height),
    XtRDimension,		(caddr_t) &default_margin_height},

    {DwcIbwNiconWidth,		DwcIbwCIconWidth,	    XtRDimension,
    sizeof (Dimension),		XmPartOffset (Iconbox, pixmap_width),
    XtRDimension,		(caddr_t) &default_icon_width},

    {DwcIbwNiconHeight,		DwcIbwCIconHeight,	    XtRDimension,
    sizeof (Dimension),		XmPartOffset (Iconbox, pixmap_height),
    XtRDimension,		(caddr_t) &default_icon_height},

   {DwcIbwNspacing,		DwcIbwCSpacing,		    XtRShort,
    sizeof (Dimension),		XmPartOffset (Iconbox, spacing), 
    XtRShort,			(caddr_t) &default_spacing},

   {DwcIbwNnumberOfIcons,	DwcIbwCNumberOfIcons,	    XtRInt,
    sizeof (Cardinal),		XmPartOffset (Iconbox, number_of_icons), 
    XtRInt,			(caddr_t) &default_zero},

   {DwcIbwNoffIcons,		DwcIbwCOffIcons,	    XtRPointer,
    sizeof (Pixmap *),		XmPartOffset (Iconbox, off_icons), 
    XtRPointer,			(caddr_t) NULL},

   {DwcIbwNselectedIcon,	DwcIbwCSelectedIcon,	    XtRShort,
    sizeof (unsigned char),	XmPartOffset (Iconbox, selected_icon), 
    XtRShort,			(caddr_t) NULL},

   {DwcIbwNselectedIcons,	DwcIbwCSelectedIcons,	    XtRPointer,
    sizeof (unsigned char *),	XmPartOffset (Iconbox, selected_icons), 
    XtRPointer,			(caddr_t) NULL},

   {DwcIbwNnumberOfSelected,	DwcIbwCNumberOfSelected,    XtRInt,
    sizeof (Cardinal),		XmPartOffset (Iconbox, number_of_selected), 
    XtRInt,			(caddr_t) &default_zero},

   {DwcIbwNcolumns,		DwcIbwCColumns,		    XtRInt,
    sizeof (Cardinal),		XmPartOffset (Iconbox, columns), 
    XtRInt,			(caddr_t) &default_zero},

   {DwcIbwNrows,		DwcIbwCRows,		    XtRInt,
    sizeof (Cardinal),		XmPartOffset (Iconbox, rows), 
    XtRInt,			(caddr_t) &default_zero},

   {DwcIbwNiconboxStyle,	DwcIbwCIconboxStyle,	    XtRInt,
    sizeof (DwcIbwIconboxStyle),XmPartOffset (Iconbox, iconbox_style), 
    XtRInt,			(caddr_t) &default_style},

   {DwcIbwNeditable,		DwcIbwCEditable,	    XtRBoolean,
    sizeof (Boolean),		XmPartOffset (Iconbox, editable), 
    XtRBoolean,			(caddr_t) &default_editable},

   {DwcIbwNvalueChangedCallback,DwcIbwCValueChangedCallback, XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Iconbox, value_changed_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcIbwNhelpCallback,	DwcIbwCHelpCallback,	    XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Iconbox, help_callback), 
    XtRCallback,		(caddr_t) NULL}

};

/*
** Static initialization of the widget class record, must do each field
*/         
IconboxClassRec iconboxClassRec =
{
    /*
    ** Core Class record
    */
    {
	(WidgetClass) &xmManagerClassRec,	/* superclass ptr */
	"Iconbox",				/* class_name */
	sizeof (IconboxWidgetRec),		/* size of widget instance */
	ClassInitialize,			/* class init proc */
	NULL,					/* Class Part Initialize */
	FALSE,					/* Class is not initialised */
	(XtInitProc) Initialize,		/* Widget init proc */
	NULL,					/* Initialise hook */
	Realize,				/* Widget realise proc */
	actions,				/* Class Action Table */
	XtNumber (actions),
	(XtResource *) resources,		/* this class's resource list */
	XtNumber (resources),			/*  "	  " resource */
	NULLQUARK,				/* xrm_class */
	TRUE,					/* class: compress motion */
	TRUE,					/* class: compress exposure */
	TRUE,					/* class: compress enterleave */
	FALSE,					/* class: no VisibilityNotify */
	Destroy,				/* class destroy proc */
	Resize,					/* class resize proc */
	(XtExposeProc) Redisplay,		/* class expose proc */
	(XtSetValuesFunc) SetValues,		/* class set_value proc */
	NULL,					/* set values hook */
	XtInheritSetValuesAlmost,		/* set values almost */
	NULL,					/* get values hook */
	NULL,					/* class accept focus proc */
	XtVersion,				/* version */
	NULL,					/* Callback Offsets */
	translations,				/* Tm_table */
	NULL,					/* disp accelerators */	
	NULL					/* extension */
    },
    /*
    **  Composite class record
    */
    {
	MISCAgreeableGeometryManager,
	XtInheritChangeManaged,
	XtInheritInsertChild,
	XtInheritDeleteChild,
	NULL
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
    ** Iconbox Class record
    */
    {
	NULL,
	0				/* just a dummy field */
    }

};

/*
**  static widget class pointer
*/

WidgetClass iconboxWidgetClass = (WidgetClass) &iconboxClassRec;


static void ClassInitialize
#ifdef _DWC_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{

#ifdef DEBUG
    printf ("IconboxWidget -- ClassInitialize Called\n");
#endif

    XmResolvePartOffsets
	(iconboxWidgetClass, &iconboxClassRec.iconbox_class.iconboxoffsets);

}

static void Initialize
#ifdef _DWC_PROTO_
	(
	Widget	request,
	Widget	new)
#else	/* no prototypes */
	(request, new)
	Widget	request;
	Widget	new;
#endif	/* prototype */
{
    XGCValues	    gcv;
    Dimension	    width;
    Dimension	    height;
    Window	    pm_root;
    int		    pm_x, pm_y;
    unsigned int    pm_border, pm_depth;
    Dimension	    pm_width  = 17;
    Dimension	    pm_height = 17;
    IconboxWidget   ibw = (IconboxWidget) new;
    XmOffsetPtr     o = IbwOffsetPtr (ibw);

    pm_width = IbwPixmapWidth (ibw, o);
    pm_height = IbwPixmapHeight (ibw, o);

    if (XtWidth (ibw) == 0)
    {
	width =
	    ((pm_width + IbwSpacing(ibw,o)) * (Dimension)IbwColumns(ibw,o)) +
	    (IbwMarginWidth(ibw,o) * 2) - IbwSpacing(ibw,o);
	if (width == 0)
	{
	    width = pm_width;
	}
	XtWidth (ibw) = width;
    }

    if (XtHeight (ibw) == 0)
    {
	height =
	    ((pm_height + IbwSpacing(ibw,o)) * (Dimension)IbwRows(ibw,o)) +
	    (IbwMarginHeight(ibw,o) * 2) - IbwSpacing(ibw,o);

	if (height == 0)
	{
	    height = (Dimension)pm_height;
	}
	XtHeight (ibw) = height;
    }

    if ((IbwIconboxStyle (ibw, o) == DwcIbwSingleSelection) &&
        (IbwNumberOfSelected (ibw, o) == 0))
    {
	IbwNumberOfSelected (ibw, o) = 1;
	IbwSelectedIcon     (ibw, o) = 0;
    }

    gcv.function = GXcopy;
    gcv.foreground = IbwForegroundPixel (ibw, o);
    gcv.background = IbwBackgroundPixel (ibw, o);
    IbwIconOffGC(ibw, o) = XtGetGC
	((Widget) ibw, GCFunction | GCForeground | GCBackground, &gcv); 

    gcv.function = GXcopy;
    gcv.foreground = IbwBackgroundPixel (ibw, o);
    gcv.background = IbwForegroundPixel (ibw, o);
    IbwIconOnGC(ibw, o) = XtGetGC
	((Widget) ibw, GCFunction | GCForeground | GCBackground, &gcv); 

    gcv.function = GXxor;
    gcv.foreground = IbwForegroundPixel (ibw, o) ^ IbwBackgroundPixel (ibw, o);
    IbwIconInvertGC (ibw, o) = XCreateGC
    (
	XtDisplay(ibw),
	RootWindowOfScreen(XtScreen(ibw)),
	GCFunction | GCForeground,
	&gcv
    );

    IbwInvertUp (ibw, o) = FALSE;

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
    printf ("IconboxWidget -- Realize Called\n");
#endif

    XtCreateWindow
	(w, InputOutput, CopyFromParent, *window_mask, window_attributes);
}

static void Redisplay
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XExposeEvent	*ee,
	Region		region)
#else	/* no prototypes */
	(w, ee, region)
	Widget		w;
	XExposeEvent	*ee;
	Region		region;
#endif	/* prototype */
{
    XRectangle		expose_area;
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);
    
    expose_area.x      = ee->x;
    expose_area.y      = ee->y;
    expose_area.width  = ee->width;
    expose_area.height = ee->height;

    redisplay_iconbox (ibw, &expose_area);

    if (IbwInvertUp (ibw, o))
    {
	MISCSetGCClipMask
	    (XtDisplay (ibw), IbwIconInvertGC (ibw, o), &expose_area, region);
	XDrawRectangle
	(
	    XtDisplay (ibw),
	    XtWindow (ibw),
	    IbwIconInvertGC (ibw, o),
	    IbwInvertX (ibw, o),
	    IbwInvertY (ibw, o),
	    IbwInvertWidth (ibw, o),
	    IbwInvertHeight (ibw, o)
	);
	MISCSetGCClipMask
	    (XtDisplay (ibw), IbwIconInvertGC (ibw, o), NULL, NULL);
    }
}

static void Resize
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    int			cols;
    int			rows;
    int			width;
    int			height;
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);

    if (IbwIconboxStyle (ibw, o) != DwcIbwOrderList)
    {
	return;
    }

    width  = (int)XtWidth(ibw) - (int)(IbwMarginWidth(ibw, o) * 2)
		+ (int)IbwSpacing (ibw, o);

    width  = MAX (0, width);
    cols   = width / (int)(IbwPixmapWidth(ibw, o) + IbwSpacing(ibw, o));

    IbwColumns (ibw, o) = MAX (1, cols);

    height = (int)XtHeight(ibw) - (int)(IbwMarginHeight (ibw, o) * 2)
		+ (int)IbwSpacing (ibw, o);

    height = MAX (0, height);
    rows   = height / (int)(IbwPixmapHeight (ibw, o) + IbwSpacing (ibw, o));

    IbwRows (ibw, o) = MAX (1, rows);

}

static Boolean SetValues
#ifdef _DWC_PROTO_
	(
	Widget	o,
	Widget	r,
	Widget	n)
#else	/* no prototypes */
	(o, r, n)
	Widget	o;
	Widget	r;
	Widget	n;
#endif	/* prototype */
{
    IconboxWidget	old = (IconboxWidget) o;
    IconboxWidget	new = (IconboxWidget) n;
    XmOffsetPtr		oo = IbwOffsetPtr (old);
    XmOffsetPtr		no = IbwOffsetPtr (new);
    Boolean		redisplay = FALSE;


    MISCUpdateCallback
    (
	(Widget) old,
	&(IbwHelpCallback (old, oo)),
	(Widget) new,
	&(IbwHelpCallback (new, no)),
	DwcIbwNhelpCallback
    );

    MISCUpdateCallback
    (
	(Widget) old,
	&(IbwValueChangedCallback (old, oo)),
	(Widget) new,
	&(IbwValueChangedCallback (new, no)),
	DwcIbwNvalueChangedCallback
    );

    return (redisplay && (XtIsRealized ((Widget)new)));

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
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);

    XtRemoveAllCallbacks (w, DwcIbwNhelpCallback);
    XtRemoveAllCallbacks (w, DwcIbwNvalueChangedCallback);

    if (IbwIconOnGC (ibw, o) != NULL)
    {
	XtReleaseGC ((Widget)ibw, IbwIconOnGC (ibw, o));
    }
    if (IbwIconOffGC (ibw, o) != NULL)
    {
	XtReleaseGC ((Widget)ibw, IbwIconOffGC (ibw, o));
    }
    if (IbwIconInvertGC (ibw, o) != NULL)
    {
	XFreeGC (XtDisplay(ibw), IbwIconInvertGC (ibw, o));
    }

}

Widget DwcIbwIconboxCreate
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
    printf ("IconboxWidget -- DwcIconboxCreate Called\n");
#endif

    return
	(XtCreateWidget (name, iconboxWidgetClass, parent, arglist, argcount));

}

#if !defined (AUD)
static
#endif
void ACTION_IBW_HELP
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
    XmAnyCallbackStruct		cbs;
    IconboxWidget		ibw = (IconboxWidget)w;
    XmOffsetPtr			o = IbwOffsetPtr (ibw);


    cbs.reason = (int)XmCR_HELP;
    cbs.event  = event;
    XtCallCallbackList ((Widget)ibw, IbwHelpCallback(ibw,o), &cbs);

}

#if !defined (AUD)
static
#endif
void ACTION_IBW_MB1DOWN
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
    Cardinal		icon;
    XRectangle		position;
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);
    XButtonEvent	*pointer = (XButtonEvent *) event;
    
    if (! IbwEditable (ibw, o))
    {
	return;
    }
    
    for (icon = 0;  icon < IbwNumberOfIcons (ibw, o);  icon++)
    {
	if (get_position_for_icon (ibw, icon, &position))
	{
	    if ((pointer->x >= position.x) && (pointer->y >= position.y) &&
		(pointer->x <  position.x + position.width) &&
		(pointer->y <  position.y + position.height))
	    {

		IbwInvertX (ibw, o) = position.x - 1;
		IbwInvertY (ibw, o) = position.y - 1;

		IbwInvertWidth  (ibw, o) = (Dimension)position.width  + 1;
		IbwInvertHeight (ibw, o) = (Dimension)position.height + 1;

		IbwInvertUp (ibw, o) = TRUE;

		XDrawRectangle
		(
		    XtDisplay (ibw),
		    XtWindow (ibw),
		    IbwIconInvertGC (ibw, o),
		    IbwInvertX (ibw, o), IbwInvertY (ibw, o),
		    IbwInvertWidth (ibw, o),
		    IbwInvertHeight (ibw, o)
		);
	    }
	}
    }
}

#if !defined (AUD)
static
#endif
void ACTION_IBW_MB1MOTION
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
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);
    XButtonEvent	*pointer = (XButtonEvent *) event;

    if ((! IbwEditable (ibw, o)) || (! IbwInvertUp (ibw, o)))
    {
	return;
    }

    if ((pointer->x <  IbwInvertX (ibw, o)) ||
        (pointer->y <  IbwInvertY (ibw, o)) ||
	(pointer->x >= IbwInvertX (ibw, o) + IbwInvertWidth  (ibw, o)) ||
	(pointer->y >= IbwInvertY (ibw, o) + IbwInvertHeight (ibw, o)))
    {
	XDrawRectangle
	(
	    XtDisplay (ibw),
	    XtWindow (ibw),
	    IbwIconInvertGC (ibw, o),
	    IbwInvertX (ibw, o),
	    IbwInvertY (ibw, o),
	    IbwInvertWidth (ibw, o),
	    IbwInvertHeight (ibw, o)
	);

	IbwInvertUp (ibw, o) = FALSE;
    }
}

static Boolean do_single_selection
#ifdef _DWC_PROTO_
	(
	IconboxWidget	ibw,
	Cardinal	icon,
	XRectangle	*position)
#else	/* no prototypes */
	(ibw, icon, position)
	IconboxWidget	ibw;
	Cardinal	icon;
	XRectangle	*position;
#endif	/* prototype */
{
    Cardinal		old;
    XRectangle		old_position;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);

    old = IbwSelectedIcon(ibw, o);

    if (old == icon)
    {
	return (FALSE);
    }

    IbwSelectedIcon(ibw, o) = icon;

    redisplay_icon(ibw, icon, position);

    get_position_for_icon (ibw, old, &old_position);
    redisplay_icon (ibw, old, &old_position);

    return (TRUE);

}

static Boolean IbwDoMultipleSelection
#ifdef _DWC_PROTO_
	(
	IconboxWidget	ibw,
	Cardinal	icon,
	XRectangle	*position,
	Boolean		*ret_selected)
#else	/* no prototypes */
	(ibw, icon, position, ret_selected)
	IconboxWidget	ibw;
	Cardinal	icon;
	XRectangle	*position;
	Boolean		*ret_selected;
#endif	/* prototype */
{
    Cardinal		i;
    Cardinal		j;
    Boolean		selected;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);


    selected = FALSE;
    for (i = 0;  i < IbwNumberOfSelected (ibw, o);  i++)
    {
	if (IbwSelectedIcons (ibw, o) [i] == icon)
	{
	    for (j = i + 1; j < IbwNumberOfSelected (ibw, o); j++)
	    {
		IbwSelectedIcons (ibw, o)[j-1] = IbwSelectedIcons (ibw, o) [j];
	    }

	    selected = TRUE;

	    IbwNumberOfSelected (ibw, o)--;
	    if (IbwNumberOfSelected (ibw, o) == 0)
	    {
		XtFree ((char *) IbwSelectedIcons (ibw, o));
		IbwSelectedIcons (ibw, o) = NULL;
	    }
	    else
	    {
		IbwSelectedIcons (ibw, o) = (unsigned char *) XtRealloc
		(
		    (char *) IbwSelectedIcons (ibw, o),
		    sizeof (unsigned char) * IbwNumberOfSelected (ibw, o)
		);
	    }

	    break;
	}
    }

    if (! selected)
    {
	IbwSelectedIcons (ibw, o) = (unsigned char *) XtRealloc
	(
	    (char *) IbwSelectedIcons (ibw, o),
	    sizeof (unsigned char) * (IbwNumberOfSelected (ibw, o) + 1)
	);
	IbwSelectedIcons (ibw, o) [IbwNumberOfSelected (ibw, o)] = icon;
	IbwNumberOfSelected (ibw, o)++;
    }

    redisplay_icon (ibw, icon, position);

    *ret_selected = ! selected;
    
    return (TRUE);
    
}

static Boolean IbwDoOrderList
#ifdef _DWC_PROTO_
	(
	IconboxWidget	ibw,
	Cardinal	icon,
	XRectangle	*position)
#else	/* no prototypes */
	(ibw, icon, position)
	IconboxWidget	ibw;
	Cardinal	icon;
	XRectangle	*position;
#endif	/* prototype */
{
    Cardinal		i;
    Cardinal		old;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);


    old = IbwSelectedIcons (ibw, o) [0];
    if (old == icon)
    {
	return (FALSE);
    }

    for (i = 0;  i < IbwNumberOfSelected (ibw, o);  i++)
    {
	if (IbwSelectedIcons (ibw, o) [i] == icon)
	{
	    IbwSelectedIcons (ibw, o) [i] = old;
	    IbwSelectedIcons (ibw, o) [0] = icon;
	    break;
	}
    }

    return (TRUE);
    
}

static void
IbwInvokeValueChangedCB
#ifdef _DWC_PROTO_
	(
	IconboxWidget	ibw,
	XEvent		*event,
	unsigned char	icon,
	Boolean		selected)
#else	/* no prototypes */
	(ibw, event, icon, selected)
	IconboxWidget	ibw;
	XEvent		*event;
	unsigned char	icon;
	Boolean		selected;
#endif	/* prototype */
{
    DwcIbwCallbackStruct	cbs;
    XmOffsetPtr			o = IbwOffsetPtr (ibw);


    cbs.reason = (int)XmCR_VALUE_CHANGED;
    cbs.event  = event;
    cbs.style  = IbwIconboxStyle (ibw, o);

    cbs.selected_icon    = icon;
    cbs.selected_icon_on = selected;

    if (IbwIconboxStyle (ibw, o) == DwcIbwSingleSelection)
    {
	cbs.number_selected  = 1;
	cbs.selected_icons   = NULL;
    }
    else
    {
	cbs.number_selected  = IbwNumberOfSelected (ibw, o);
	cbs.selected_icons   = IbwSelectedIcons (ibw, o);
    }
    XtCallCallbackList ((Widget)ibw, IbwValueChangedCallback(ibw,o), &cbs);
}

#if !defined (AUD)
static
#endif
void ACTION_IBW_MB1UP
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
    Boolean		selected;
    Boolean		changed;
    XRectangle		position;
    Cardinal		icon;
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);
    XButtonEvent	*pointer = (XButtonEvent *) event;

    if ((! IbwEditable (ibw, o)) || (! IbwInvertUp (ibw, o)))
    {
	return;
    }

    XDrawRectangle
    (
	XtDisplay (ibw),
	XtWindow (ibw),
	IbwIconInvertGC (ibw, o),
	IbwInvertX (ibw, o),
	IbwInvertY (ibw, o),
	IbwInvertWidth (ibw, o),
	IbwInvertHeight (ibw, o)
    );

    IbwInvertUp (ibw, o) = FALSE;
    
    for (icon = 0;  icon < IbwNumberOfIcons (ibw, o);  icon++)
    {
	if (get_position_for_icon (ibw, icon, &position))
	{
	    if ((pointer->x >= position.x) && (pointer->y >= position.y) &&
		(pointer->x <  position.x + position.width) &&
		(pointer->y <  position.y + position.height))
	    {

		if (IbwIconboxStyle (ibw, o) == DwcIbwSingleSelection)
		{
		    changed  = do_single_selection(ibw, icon, &position);
		    selected = TRUE;
		}
		else
		{
		    if (IbwIconboxStyle (ibw, o) == DwcIbwMultipleSelection)
		    {
			changed = IbwDoMultipleSelection
			    (ibw, icon, &position, &selected);
		    }
		    else
		    {
			changed = IbwDoOrderList (ibw, icon, &position);
			if (changed)
			{
			    if (XtIsRealized ((Widget)ibw))
			    {
				XClearArea
				(
				    XtDisplay(ibw),
				    XtWindow(ibw),
				    0,
				    0,
				    0,
				    0,
				    TRUE
				);
			    }
			    selected = TRUE;
			}
		    }
		}

		if (changed)
		{
		    IbwInvokeValueChangedCB (ibw, event, icon, selected);
		}

		return;
	    }
	}
    }
}

#if !defined (AUD)
static
#endif
void ACTION_IBW_CANCEL
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
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);
    XButtonEvent	*pointer = (XButtonEvent *) event;

    if ((! IbwEditable (ibw, o)) || (! IbwInvertUp (ibw, o)))
    {
	return;
    }

    XDrawRectangle
    (
	XtDisplay (ibw),
	XtWindow (ibw),
	IbwIconInvertGC (ibw, o),
	IbwInvertX (ibw, o),
	IbwInvertY (ibw, o),
	IbwInvertWidth (ibw, o),
	IbwInvertHeight (ibw, o)
    );

    IbwInvertUp (ibw, o) = FALSE;

}

static Boolean get_position_for_icon
#ifdef _DWC_PROTO_
	(
	IconboxWidget	ibw,
	Cardinal	icon,
	XRectangle	*position)
#else	/* no prototypes */
	(ibw, icon, position)
	IconboxWidget	ibw;
	Cardinal	icon;
	XRectangle	*position;
#endif	/* prototype */
{
    Boolean		selected;
    Cardinal		i;
    Cardinal		row;
    Cardinal		col;
    Position		x;
    Position		y;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);


    if (IbwIconboxStyle (ibw, o) == DwcIbwOrderList)
    {
	selected = FALSE;
	for (i = 0;  i < IbwNumberOfSelected (ibw, o);  i++)
	{
	    if (IbwSelectedIcons (ibw, o) [i] == icon)
	    {
		selected = TRUE;
		row = i / IbwColumns (ibw, o);
		col = i % IbwColumns (ibw, o);
		break;
	    }
	}

	if (! selected)
	{
	    return (FALSE);
	}

    }
    else
    {
	row = icon / IbwColumns (ibw, o);
	col = icon % IbwColumns (ibw, o);
    }

    x = (int)XtWidth(ibw) - (int)IbwMarginWidth (ibw, o);

    x = (int)IbwMarginWidth (ibw, o) +  ((x * col) / IbwColumns (ibw, o));

    y = (int)XtHeight(ibw) - (int)IbwMarginHeight (ibw, o);
    y = (int)IbwMarginHeight(ibw, o) + ((y * row) / IbwRows (ibw, o));

    position->x = x;
    position->y = y;
    position->width  = (int)IbwPixmapWidth(ibw, o);
    position->height = (int)IbwPixmapHeight(ibw, o);

    return (TRUE);

}

static void redisplay_icon
#ifdef _DWC_PROTO_
	(
	IconboxWidget	ibw,
	unsigned char	icon,
	XRectangle	*position)
#else	/* no prototypes */
	(ibw, icon, position)
	IconboxWidget	ibw;
	unsigned char	icon;
	XRectangle	*position;
#endif	/* prototype */
{
    Cardinal		i;
    Boolean		selected;
    Pixmap		pixmap;
    GC			gc;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);

    if (IbwIconboxStyle (ibw, o) == DwcIbwSingleSelection)
    {
	selected = (icon == IbwSelectedIcon (ibw, o));
    }
    else
    {
	selected = FALSE;
	for (i = 0;  i < IbwNumberOfSelected (ibw, o);  i++)
	{
	    if (IbwSelectedIcons (ibw, o) [i] == icon)
	    {
		selected = TRUE;
		break;
	    }
	}
    }

    if (IbwIconboxStyle (ibw, o) == DwcIbwOrderList)
    {
	if (selected)
	{
	    gc = IbwIconOffGC (ibw, o);
	}
	else
	{
	    return;
	}
    }
    else
    {
	if (selected)
	{
	    gc = IbwIconOnGC (ibw, o);
	}
	else
	{
	    gc = IbwIconOffGC (ibw, o);
	}
    }

    pixmap = IbwOffIcons (ibw, o) [icon];

    XCopyPlane
    (
	XtDisplay (ibw),
	pixmap,
	XtWindow (ibw),
	gc,
	0,
	0,
	position->width,
	position->height,
	position->x,
	position->y,
	1
    );

}

static void redisplay_iconbox
#ifdef _DWC_PROTO_
	(
	IconboxWidget	ibw,
	XRectangle	*expose_area)
#else	/* no prototypes */
	(ibw, expose_area)
	IconboxWidget	ibw;
	XRectangle	*expose_area;
#endif	/* prototype */
{
    XRectangle		position;
    Cardinal		icon;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);


    for (icon = 0;  icon < IbwNumberOfIcons (ibw, o);  icon++)
    {

	if (get_position_for_icon (ibw, icon, &position))
	{

	    if (! ((expose_area->x + expose_area->width  <= position.x) ||
		   (expose_area->y + expose_area->height <= position.y) ||
		   (expose_area->x >= position.x + position.width)  ||
		   (expose_area->y >= position.y + position.height)))
	    {

		redisplay_icon (ibw, icon, &position);
	    }
	}
    }
	    
}

void DwcIbwSetSelectedIcon
#ifdef _DWC_PROTO_
	(
	Widget		w,
	unsigned char	icon)
#else	/* no prototypes */
	(w, icon)
	Widget		w;
	unsigned char	icon;
#endif	/* prototype */
{
    XRectangle		position;
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);


    if (IbwIconboxStyle (ibw, o) != DwcIbwSingleSelection)
    {
	return;
    }

    if (get_position_for_icon (ibw, icon, &position))
    {
	(void) do_single_selection(ibw, icon, &position);
    }
}

unsigned char DwcIbwGetSelectedIcon
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);

    return (IbwSelectedIcon (ibw, o));
}
    
void DwcIbwSetSelectedIcons
#ifdef _DWC_PROTO_
	(
	Widget		w,
	unsigned char	*icons,
	Cardinal	num_icons)
#else	/* no prototypes */
	(w, icons, num_icons)
	Widget		w;
	unsigned char	*icons;
	Cardinal	num_icons;
#endif	/* prototype */
{
    Cardinal		size;
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);

    XtFree ((char *) IbwSelectedIcons (ibw, o));

    IbwNumberOfSelected (ibw, o) = num_icons;

    if (num_icons != 0)
    {
	size = sizeof (unsigned char) * IbwNumberOfSelected (ibw, o);
	IbwSelectedIcons (ibw, o) = (unsigned char *) XtMalloc (size);
	memcpy (IbwSelectedIcons (ibw, o), icons, size);
    }
    else
    {
	IbwSelectedIcons (ibw, o) = NULL;
    }
    
    if (XtIsRealized ((Widget)ibw))
    {
	XClearArea (XtDisplay (ibw), XtWindow (ibw), 0, 0, 0, 0, TRUE);
    }

}
    
void DwcIbwGetSelectedIcons
#ifdef _DWC_PROTO_
	(
	Widget		w,
	unsigned char	**ret_icons,
	Cardinal	*ret_num_icons)
#else	/* no prototypes */
	(w, ret_icons, ret_num_icons)
	Widget		w;
	unsigned char	**ret_icons;
	Cardinal	*ret_num_icons;
#endif	/* prototype */
{
    Cardinal		size;
    unsigned char	*icons;
    IconboxWidget	ibw = (IconboxWidget) w;
    XmOffsetPtr		o = IbwOffsetPtr (ibw);

    if (IbwNumberOfSelected (ibw, o) == 0)
    {
	*ret_icons     = NULL;
	*ret_num_icons = 0;
	return;
    }
    
    size  = sizeof (unsigned char) * IbwNumberOfSelected (ibw, o);
    icons = (unsigned char *) XtMalloc (size);
    memcpy (icons, IbwSelectedIcons (ibw, o), size);

    *ret_icons = icons;
    *ret_num_icons = IbwNumberOfSelected (ibw, o);

}

int DwcIbwInitializeForMrm
#ifdef _DWC_PROTO_
(void)
#else
()
#endif
{
    int status;
    status  = MrmRegisterClass
    (
	MrmwcUnknown,
	"Iconbox",
	"IconBoxLowLevelCreate",
	DwcIbwIconboxCreate,
	iconboxWidgetClass
    );
    return (status);
}

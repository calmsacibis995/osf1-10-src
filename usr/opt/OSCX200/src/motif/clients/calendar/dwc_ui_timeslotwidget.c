/* dwc_ui_timeslotwidget.c */
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
**	DECwindows Calendar Dayslots Widget
**
**  AUTHOR:
**
**	Marios Cleovoulou, December-1988
**
**  ABSTRACT:
**
**	This module implements the timeslot widget.
**
**--
*/

#include "dwc_compat.h"

#include <ctype.h>
#include <stdio.h>
#include <assert.h>

#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <X11/cursorfont.h>		/* for various cursors */
#include <DXm/DECspecific.h>
#ifdef vaxc
#pragma standard
#endif

#include "dwc_ui_timeslotwidgetp.h"
#include "dwc_ui_dayslotswidgetp.h"	/* for DSWRequestOperationOk */
#include "dwc_ui_mba.h"
#include "dwc_ui_misc.h"		/* for MISCGetFontFromFontlist */


static void ClassInitialize PROTOTYPE ((void));

static void Destroy PROTOTYPE ((Widget	w));

static void Initialize PROTOTYPE ((Widget request, Widget new));

static void Realize PROTOTYPE ((
	Widget			w,
	XtValueMask		*window_mask,
	XSetWindowAttributes	*window_attributes));

static void Redisplay PROTOTYPE ((
	Widget		w,
	XExposeEvent	*ee,
	Region		region));

static void Resize PROTOTYPE ((Widget w));

static void setup_timeslot_areas PROTOTYPE ((TimeslotWidget tsw));

static Boolean SetValues PROTOTYPE ((Widget o, Widget r, Widget n));

static void get_pixmap_x_and_y PROTOTYPE ((
	TimeslotWidget	tsw,
	Cardinal	pn,
	Position	*x,
	Position	*y));

static Widget find_timeslot PROTOTYPE ((Widget w));

static Boolean AcceptFocus PROTOTYPE ((Widget w, Time *time));

static void redisplay_timeslot PROTOTYPE ((
	TimeslotWidget	tsw,
	XRectangle	*expose_area,
	Region		region));

static void ResizeTimeslot PROTOTYPE ((TimeslotWidget tsw, Boolean setfont));

static Boolean on_a_resize_knob PROTOTYPE ((
	TimeslotWidget	tsw,
	XButtonEvent	*event,
	Boolean		*top_knob));

#if !defined (AUD)
static
#endif
void ACTION_TSW_MB1UP PROTOTYPE ((Widget w, XButtonEvent *event));

#if !defined (AUD)
static
#endif
void ACTION_TSW_SELECT PROTOTYPE ((Widget w, XEvent *event));

#if !defined (AUD)
static
#endif
void ACTION_TSW_CANCEL PROTOTYPE ((Widget w, XButtonEvent *event));

#if !defined (AUD)
static
#endif
void ACTION_TSW_CLOSE PROTOTYPE ((Widget	w, XEvent *event));

#if !defined (AUD)
static
#endif
void ACTION_TSW_MB1DOWN PROTOTYPE ((Widget w, XButtonEvent *event));

#if !defined (AUD)
static
#endif
void ACTION_TSW_EXTEND PROTOTYPE ((Widget w, XButtonEvent *event));

#if !defined (AUD)
static
#endif
void ACTION_TSW_OPEN PROTOTYPE ((Widget w, XEvent *keyevent));

static void start_motion PROTOTYPE ((Widget w, XEvent *event));

#if !defined (AUD)
static
#endif
void ACTION_TSW_MB1MOTION PROTOTYPE ((Widget w, XButtonEvent *event));

static void set_timeslot_opened PROTOTYPE ((
	TimeslotWidget	tsw,
	Boolean		opened,
	Time		time));

static void TEXT_GOT_FOCUS PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*text_cbs));

static void TEXT_LOSING_FOCUS PROTOTYPE ((
	Widget				w,
	caddr_t				tag,
	DXmCSTextVerifyCallbackStruct	*text_cbs));

static void TEXT_HELP PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*text_cbs));

#if !defined (AUD)
static
#endif
void ACTION_TSW_HELP PROTOTYPE ((Widget w, XEvent *event));

static void TEXT_CHANGED PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct *cbs));

static void HandleSash PROTOTYPE ((
	Widget	w,
	caddr_t	closure,
	caddr_t	callData));

static XtCallbackRec TextChangedCB [2] =
{
    {(XtCallbackProc) TEXT_CHANGED,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec TextGotFocusCB [2] =
{
    {(XtCallbackProc) TEXT_GOT_FOCUS,	NULL},
    {NULL,				NULL}
};

static XtCallbackRec TextLosingFocusCB [2] =
{
    {(XtCallbackProc) TEXT_LOSING_FOCUS,	NULL},
    {NULL,					NULL}
};

static XtCallbackRec TextHelpCB [2] =
{
    {(XtCallbackProc) TEXT_HELP,	NULL},
    {NULL,				NULL}
};

#define forGCmask       GCForeground | GCBackground | GCLineWidth | GCDashList
#define formapGCmask    GCForeground | GCBackground | GCTile | GCFillStyle | GCLineWidth | GCDashList

#define backGCmask      GCForeground | GCBackground | GCLineWidth
#define backmapGCmask   GCForeground | GCBackground | GCTile | GCFillStyle | GCLineWidth

static int  default_pixmap_width    = 12;
static int  default_pixmap_height   = 12;
static int  default_pixmap_margin   = 1;
static int  default_knob_size       = 9;

static int  default_zero	    = 0;

static Boolean default_editable	    = TRUE;
static Boolean default_dragable	    = TRUE;

static Cursor			    move_cursor = 0;
static Cursor			    resize_top_cursor = 0;
static Cursor			    resize_bottom_cursor = 0;

static XmString			    zero_length_string;

/*
**  Resources for the Timeslot widget.  These are the 'public' widget
**  attributes that the caller can set.
*/
static XmPartResource resources [] =
{
   {DwcTswNpixmapWidth,		DwcTswCPixmapWidth,	     XtRDimension,
    sizeof (Dimension),		XmPartOffset (Timeslot, pixmap_width), 
    XtRInt,			(caddr_t) &default_pixmap_width},

   {DwcTswNpixmapHeight,	DwcTswCPixmapHeight,	     XtRDimension,
    sizeof (Dimension),		XmPartOffset (Timeslot, pixmap_height), 
    XtRInt,			(caddr_t) &default_pixmap_height},

   {DwcTswNpixmapCount,		DwcTswCPixmapCount,	     XtRInt,
    sizeof (Cardinal),		XmPartOffset (Timeslot, pixmap_count), 
    XtRInt,			(caddr_t) &default_zero},

   {DwcTswNpixmaps,		DwcTswCPixmaps,		     XtRPointer,
    sizeof (Pixmap *),		XmPartOffset (Timeslot, pixmaps),
    XtRPointer,			(caddr_t) NULL},

   {DwcTswNpixmapMargin,	DwcTswCPixmapMargin,	     XtRDimension,
    sizeof (Dimension),		XmPartOffset (Timeslot, pixmap_margin), 
    XtRInt,			(caddr_t) &default_pixmap_margin},

   {DwcTswNknobSize,		DwcTswCKnobSize,	     XtRDimension,
    sizeof (Dimension),		XmPartOffset (Timeslot, knob_size), 
    XtRInt,			(caddr_t) &default_knob_size},

   {DwcTswNeditable,		DwcTswCEditable,	     XtRBoolean,
    sizeof (Boolean),		XmPartOffset (Timeslot, editable), 
    XtRBoolean,			(caddr_t) &default_editable},

   {DwcTswNdragable,		DwcTswCDragable,	     XtRBoolean,
    sizeof (Boolean),		XmPartOffset (Timeslot, dragable), 
    XtRBoolean,			(caddr_t) &default_dragable},

   {DwcTswNsingleCallback,	DwcTswCSingleCallback,	     XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, single_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNdoubleCallback,	DwcTswCDoubleCallback,	     XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, double_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNdragStartCallback,	DwcTswCDragStartCallback,    XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, drag_start_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNdragCallback,	DwcTswCDragCallback,	     XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, drag_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNdragEndCallback,	DwcTswCDragEndCallback,	     XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, drag_end_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNdragCancelCallback,	DwcTswCDragCancelCallback,   XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, drag_cancel_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNresizeStartCallback,	DwcTswCResizeStartCallback,  XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, resize_start_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNresizeCallback,	DwcTswCResizeCallback,       XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, resize_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNresizeEndCallback,	DwcTswCResizeEndCallback,    XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, resize_end_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNresizeCancelCallback,DwcTswCResizeCancelCallback, XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, resize_cancel_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNfocusCallback,	DwcTswCFocusCallback,        XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, focus_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNlosingFocusCallback,	DwcTswCFocusCallback,        XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, losingfocus_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNcloseCallback,	DwcTswCCloseCallback,        XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, close_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNextendSelectCallback,DwcTswCExtendSelectCallback, XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, extend_select_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNincrementSizeCallback,DwcTswCIncrementSizeCallback, XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, incr_size_callback), 
    XtRCallback,		(caddr_t) NULL},

   {DwcTswNincrementPositionCallback,DwcTswCIncrementPositionCallback, XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset (Timeslot, incr_posi_callback), 
    XtRCallback,		(caddr_t) NULL},

    {DwcTswNfontList,		XmCFontList,		    XmRFontList,
    sizeof(XmFontList),		XmPartOffset(Timeslot, fontlist),
    XmRString,			DXmDefaultFont}

};  

/*
**  Translation table for Timeslot
*/

static char translations [] =
   "\043override\n\
     @Help <BtnDown>:			ACTION_TSW_HELP()\n\
     Ctrl ~@Help ~Button2<Btn1Down>:	ACTION_TSW_EXTEND()\n\
    ~Ctrl ~@Help ~Button2<Btn1Down>:	ACTION_TSW_MB1DOWN()\n\
    ~@Help Button1<Motion>:		ACTION_TSW_MB1MOTION()\n\
    ~@Help <Btn1Up>:			ACTION_TSW_MB1UP()\n\
    ~Shift ~@Help ~Button1<Btn2Down>:	ACTION_TSW_MB1DOWN()\n\
    ~@Help Button2<Motion>:		ACTION_TSW_MB1MOTION()\n\
    ~@Help <Btn2Up>:			ACTION_TSW_MB1UP()\n\
    Button1<BtnDown>:			ACTION_TSW_CANCEL()\n\
    Button2<BtnDown>:			ACTION_TSW_CANCEL()\n\
    Button3<BtnDown>:			ACTION_TSW_CANCEL()\n\
    Button4<BtnDown>:			ACTION_TSW_CANCEL()\n\
    Button5<BtnDown>:			ACTION_TSW_CANCEL()\n\
    Button1<Key>osfCancel:		ACTION_TSW_CANCEL()\n\
    Button2<Key>osfCancel:		ACTION_TSW_CANCEL()";

static XtTranslations text_translations_parsed;
static char text_translations [] =
    "\043override\n\
    <Key>osfSelect:			ACTION_TSW_SELECT()\n\
    <Key>0xff8d:			ACTION_TSW_CLOSE()\n\
    <Key>osfActivate:			ACTION_TSW_CLOSE()";

static XtTranslations sash_translations_parsed;
static char sash_translations [] =
    "\043override\n\
    <Key>osfSelect:			ACTION_TSW_SELECT()\n\
    <Key>0xff8d:			ACTION_TSW_CLOSE()\n\
    <Key>osfActivate:			ACTION_TSW_CLOSE()";

/*
**  Action table - 'action' routines that may be called via translation table
*/

static XtActionsRec actions [] =
{
   {"ACTION_TSW_HELP",	    (XtActionProc)ACTION_TSW_HELP},
   {"ACTION_TSW_MB1DOWN",   (XtActionProc)ACTION_TSW_MB1DOWN},
   {"ACTION_TSW_MB1MOTION", (XtActionProc)ACTION_TSW_MB1MOTION},
   {"ACTION_TSW_MB1UP",	    (XtActionProc)ACTION_TSW_MB1UP},
   {"ACTION_TSW_EXTEND",    (XtActionProc)ACTION_TSW_EXTEND},
   {"ACTION_TSW_CANCEL",    (XtActionProc)ACTION_TSW_CANCEL},
   {"ACTION_TSW_CLOSE",	    (XtActionProc)ACTION_TSW_CLOSE},
   {"ACTION_TSW_OPEN",	    (XtActionProc)ACTION_TSW_OPEN},
   {"ACTION_TSW_SELECT",    (XtActionProc)ACTION_TSW_SELECT},
   {NULL,		    NULL}
};

/*
** Static initialization of the widget class record, must do each field
*/         

TimeslotClassRec timeslotClassRec =
{

    /*
    ** Core Class record
    */
    {
	(WidgetClass) &xmManagerClassRec, /* superclass ptr		    */
	"Timeslot",			/* class_name			    */
	sizeof (TimeslotRec),	/* size of timeslot widget instance */
	ClassInitialize,		/* class init proc		    */
	NULL,				/* Class Part Initialize	    */
	FALSE,				/* Class is not initialised	    */
	(XtInitProc)Initialize,		/* Widget init proc		    */
	NULL,				/* Initialise hook		    */
	Realize,			/* Widget realise proc		    */
	actions,			/* Class Action Table		    */
	XtNumber (actions),
	(XtResource *) resources,	/* this class's resource list	    */
	XtNumber (resources),		/*  "	  " resource     	    */
	NULLQUARK,			/* xrm_class			    */
	TRUE,				/* class: compressed motion	    */
	TRUE,				/* class: compressed exposure	    */
	TRUE,				/* class: compressed enterleave	    */
	FALSE,				/* class: no VisibilityNotify	    */
	Destroy,			/* class destroy proc		    */
	Resize,				/* class resize proc		    */
	(XtExposeProc)Redisplay,	/* class expose proc		    */
	(XtSetValuesFunc) SetValues,	/* class set_value proc		    */
	NULL,				/* set values hook		    */
	XtInheritSetValuesAlmost,	/* set values almost		    */
	NULL,				/* get values hook		    */
	AcceptFocus,			/* class accept focus proc	    */
	XtVersion,			/* version			    */
	NULL,				/* Callback Offsets		    */
	translations,			/* Tm_table			    */
	NULL,				/* disp accelerators  */	
	NULL				/* extension          */ 
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
    ** Timeslot Class record
    */
    {
	NULL,
	0				/* just a dummy field */
    }
};

/*
**  widget class pointer
*/

WidgetClass timeslotWidgetClass = (WidgetClass) &timeslotClassRec;

static void ClassInitialize
#ifdef _DWC_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{
    long	byte_count, cvt_status;

#ifdef DEBUG
    printf ("TimeslotWidget -- ClassInitialize Called\n");
#endif

    XmResolvePartOffsets (timeslotWidgetClass,
		       &timeslotClassRec.timeslot_class.timeslotoffsets);
    text_translations_parsed = XtParseTranslationTable (text_translations);
    sash_translations_parsed = XtParseTranslationTable (sash_translations);

    zero_length_string = DXmCvtFCtoCS ("", &byte_count, &cvt_status);
}

static void setup_timeslot_areas
#ifdef _DWC_PROTO_
	(
	TimeslotWidget	tsw)
#else	/* no prototypes */
	(tsw)
	TimeslotWidget	tsw;
#endif	/* prototype */
{
    int		    height;
    int		    rows;
    int		    cols;
    int		    row;
    int		    col;
    int		    tw_xr;
    Dimension	    hwidth;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);
    int		    min_width;


    height = XtHeight(tsw) - TswPixmapMargin(tsw, o);
    rows   = height / (TswPixmapHeight(tsw, o) + TswPixmapMargin(tsw, o));
    rows   = MAX (rows, 1);

    cols   = (TswPixmapCount(tsw, o) + rows - 1) / rows;
    cols   = MAX (cols, 1);
        
    TswPixmapRows(tsw, o) = rows;

    TswPixmapAreaWidth(tsw, o) = TswPixmapMargin(tsw, o) +
      ((TswPixmapWidth(tsw, o) + TswPixmapMargin(tsw, o)) * cols);
    
    /*	  
    **  Get the offset for the top of the lower knob.
    */	  
    TswKnobsTopOfLowerY(tsw, o) = XtHeight (tsw) - TswKnobSize(tsw, o);
    
    /*
    ** Determine the minimum width for a timeslot text widget.  The auto
    ** wrap code doesn't like widgets less than a character width.
    */
    min_width = MISCFontListWidth (TswFontList(tsw,o));
    min_width += (((XmTextWidget)(TswTextWidget(tsw, o)))->text.margin_width * 2);
    min_width += (((XmPrimitiveWidget)(TswTextWidget(tsw, o)))->primitive.shadow_thickness * 2);
    min_width += (((XmPrimitiveWidget)(TswTextWidget(tsw, o)))->primitive.highlight_thickness * 2);
    /*	  
    **  Let's see which direction we're reading this.
    */	  
    if (TswDirectionRToL(tsw, o))
    {
        /*	  
	**  Right to left
	*/	  
        /*	  
	**  Get the leftmost edge of the pixmap area.
	*/	  
        TswPixmapAreaX(tsw, o) =
	    (int) XtWidth (tsw) - (int) TswPixmapAreaWidth(tsw, o);

        TswKnobsLeftMarginX(tsw,o) =
	    TswPixmapAreaX(tsw, o) -
	    (int)TswPixmapMargin(tsw, o) -
	    (int) TswKnobSize(tsw, o);

	tw_xr = TswKnobsLeftMarginX(tsw, o) - (int) TswPixmapMargin(tsw, o);
	if (tw_xr >= min_width)
	{
	    TswTextAreaX   (tsw,o) = 0;
	    TswTextAreaWidth(tsw, o) = tw_xr;
	}
	else
	{
	    TswTextAreaWidth(tsw, o) = min_width;
	    TswTextAreaX   (tsw,o) = TswKnobsLeftMarginX(tsw, o) - min_width;
	}

    }
    else
    {
        /*	  
	**  Left to Right.
	*/	  
        /*	  
	**  Start off at the beginning and get the rightmost x.
	*/	  
        TswPixmapAreaX(tsw, o) = 0;

	TswKnobsLeftMarginX(tsw,o) =
	    TswPixmapAreaWidth(tsw, o) + (int) TswPixmapMargin(tsw, o);

	TswTextAreaX (tsw,o) =
	    TswKnobsLeftMarginX(tsw, o) +
	    (int)TswKnobSize(tsw, o) +
	    (int)TswPixmapMargin(tsw, o);

	if (TswTextAreaX(tsw, o) <= (XtWidth (tsw) - min_width))
	{
	    TswTextAreaWidth(tsw, o) = XtWidth (tsw) - TswTextAreaX(tsw, o);
	}
	else
	{
	    TswTextAreaWidth(tsw, o) = min_width;
	}

    }
}

static void get_pixmap_x_and_y
#ifdef _DWC_PROTO_
	(
	TimeslotWidget	tsw,
	Cardinal	pn,
	Position	*x,
	Position	*y)
#else	/* no prototypes */
	(tsw, pn, x, y)
	TimeslotWidget	tsw;
	Cardinal	pn;
	Position	*x;
	Position	*y;
#endif	/* prototype */
    {
    int		    row;
    int		    col;
    XmOffsetPtr     o = TswOffsetPtr(tsw);


    col = pn / TswPixmapRows(tsw, o);
    row = pn % TswPixmapRows(tsw, o);

    if (TswDirectionRToL(tsw, o)) {

	*x = TswPixmapAreaX(tsw, o) + TswPixmapAreaWidth(tsw, o) -
	   ((TswPixmapWidth(tsw, o) + TswPixmapMargin(tsw, o)) * (col + 1));

    } else {

	*x = TswPixmapAreaX(tsw, o) + TswPixmapMargin(tsw, o) + 
	   ((TswPixmapWidth(tsw, o) + TswPixmapMargin(tsw, o)) * col);

    }

    *y = TswPixmapMargin(tsw, o) + 
       ((TswPixmapHeight(tsw, o) + TswPixmapMargin(tsw, o)) * row);
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
    TimeslotWidget  tsw = (TimeslotWidget) new;
    Position	    tw_x;
    Dimension	    tw_width;
    Arg		    arglist[31];
    Cardinal	    ac;
    Cardinal	    i;
    XmOffsetPtr     o = TswOffsetPtr(tsw);
    XGCValues	    values;
    int		    f_bit;
    XFontStruct	    *font;
    static XtCallbackRec SashCallback[] =
    {
	{(XtCallbackProc)HandleSash, NULL},
	{NULL, NULL}
    };
    XmString	    xm_text;
    long	    byte_count, cvt_status;

    if (move_cursor == (Cursor)NULL)
    {
	move_cursor = (Cursor)XCreateFontCursor
	    (XtDisplay(new), XC_double_arrow);
    }

    if (resize_top_cursor == (Cursor)NULL)
    {
	resize_top_cursor = (Cursor)XCreateFontCursor
	    (XtDisplay(new), XC_top_side);
    }

    if (resize_bottom_cursor == (Cursor)NULL)
    {
	resize_bottom_cursor = (Cursor)XCreateFontCursor
	    (XtDisplay(new), XC_bottom_side);
    }

    f_bit = 0;


    TswGrayPixmap(tsw, o) = MISCXtGrayPixmap(XtScreen(new));

    if (TswFontList(tsw,o) != NULL)
    {
	MISCGetFontFromFontlist(TswFontList(new,o), &font);    
	values.font       = font->fid;
	f_bit = GCFont;
    }

    /* set line width to 0 for server speed */

    values.line_width = 0;
    values.foreground = TswForegroundPixel(tsw,o);
    values.background = TswBackgroundPixel(tsw,o);
    values.tile       = TswGrayPixmap(tsw,o);
    values.fill_style = FillTiled;
    values.dashes     = 1;

    if (XtSensitive(new))
       TswForegroundGC(tsw,o) = XtGetGC (new, (forGCmask | f_bit), &values);
    else
       TswForegroundGC(tsw,o) = XtGetGC (new, (formapGCmask | f_bit), &values);    

    values.background = TswForegroundPixel(tsw,o);
    values.foreground = TswBackgroundPixel(tsw,o);
    values.tile       = TswBackgroundPixmap(tsw,o);

    /* this is to catch the parent relative and other x constants */

    if (TswBackgroundPixmap(tsw,o) > 10)
        TswBackgroundGC(tsw,o) = XtGetGC(new, (backmapGCmask | f_bit), &values);
    else
        TswBackgroundGC(tsw,o) = XtGetGC(new, (backGCmask | f_bit), &values);


    TswMbaContext(tsw, o) = MBAInitContext(new, TRUE, TRUE, start_motion);

    TswSelected (tsw,o) = FALSE;
    TswOpened   (tsw,o) = FALSE;
    TswResizing (tsw,o) = FALSE;
    TswDragging (tsw,o) = FALSE;

    TextChangedCB[0].closure = (caddr_t)tsw;
    TextGotFocusCB[0].closure = (caddr_t)tsw;
    TextLosingFocusCB[0].closure = (caddr_t)tsw;
    TextHelpCB[0].closure = (caddr_t)tsw;

    /*
    **  Copy pixmap pointers (but not pixmaps themselves) and *important* set
    **	the pixmap resource to NULL.
    */
    
    if (TswPixmapCount(tsw, o) == 0) {
	TswRealPixmaps(tsw, o) = NULL;
    } else {
	TswRealPixmaps(tsw, o) = (Pixmap *)
	  XtMalloc (sizeof (Pixmap) * TswPixmapCount(tsw, o));
        for (i = 0;  i < TswPixmapCount(tsw, o);  i++) {
	    TswRealPixmaps(tsw, o) [i] = TswPixmaps(tsw, o) [i];
	}
    }
    TswPixmaps(tsw, o) = NULL;
    


    SashCallback[0].closure = (caddr_t)tsw;

    if (TswDragable(tsw,o))
    {
	TswTopSash(tsw, o) = XtVaCreateWidget
	(
	    "topSash", (WidgetClass) xmSashWidgetClass, (Widget) tsw,
	    XmNwidth, TswKnobSize(tsw, o),
	    XmNheight, TswKnobSize(tsw, o),
	    XmNx, TswKnobsLeftMarginX(tsw, o),
	    XmNy, 0,
	    XmNshadowThickness, 2,
	    XmNcallback, SashCallback,
	    XmNunitType, XmPIXELS,
	    XmNtraversalOn, True,
	    XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
	    NULL
	);

	TswMoveSash(tsw, o) = XtVaCreateWidget
	(
	    "moveSash", (WidgetClass) xmSashWidgetClass, (Widget) tsw,
	    XmNwidth, TswKnobSize(tsw,o),
	    XmNheight, XtHeight(tsw) - (2 * TswKnobSize(tsw,o)),
	    XmNx, TswKnobsLeftMarginX(tsw, o),
	    XmNy, TswKnobSize(tsw,o),
	    XmNshadowThickness, 2,
	    XmNcallback, SashCallback,
	    XmNunitType, XmPIXELS,
	    XmNtraversalOn, True,
	    XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
	    NULL
	);

	TswBottomSash(tsw, o) = XtVaCreateWidget
	(
	    "bottomSash", (WidgetClass) xmSashWidgetClass, (Widget) tsw,
	    XmNwidth, TswKnobSize(tsw, o),
	    XmNheight, TswKnobSize(tsw, o),
	    XmNx, TswKnobsLeftMarginX(tsw, o),
	    XmNy, TswKnobsTopOfLowerY(tsw, o),
	    XmNshadowThickness, 2,
	    XmNcallback, SashCallback,
	    XmNunitType, XmPIXELS,
	    XmNtraversalOn, True,
	    XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
	    NULL
	);
    }
    else
    {
	TswMoveSash(tsw, o) = NULL;
	TswTopSash(tsw, o) = NULL;
	TswBottomSash(tsw, o) = NULL;
    }

    ac = 0;

    if (XtWidth (tsw) == 0) {
	XtSetArg(arglist[ac], XmNcolumns, (short)1); ac++;             
    }

    if (XtHeight (tsw) == 0) {
	XtSetArg(arglist[ac], XmNrows, (short)1); ac++;             
    }


    XtSetArg(arglist[ac], XmNfontList,	    TswFontList(tsw,o)); ac++;
    XtSetArg(arglist[ac], XmNy,	       0); ac++;
    XtSetArg(arglist[ac], XmNmarginHeight,  0); ac++;            
    XtSetArg(arglist[ac], XmNmarginWidth,   0); ac++;
    XtSetArg(arglist[ac], XmNvalue,         zero_length_string); ac++;
#if 0
    XtSetArg(arglist[ac], XmNwordWrap,      TRUE); ac++;
#endif
    XtSetArg(arglist[ac], XmNresizeHeight,  FALSE); ac++;
    XtSetArg(arglist[ac], XmNresizeWidth,   FALSE); ac++;         
    XtSetArg(arglist[ac], XmNeditable,      FALSE); ac++;
    XtSetArg(arglist[ac], XmNeditMode,	    XmMULTI_LINE_EDIT); ac++;
    XtSetArg(arglist[ac], XmNautoShowCursorPosition,  TRUE); ac++;
    XtSetArg(arglist[ac], XmNhelpCallback,          TextHelpCB); ac++;
    XtSetArg(arglist[ac], XmNvalueChangedCallback,  TextChangedCB); ac++;
    XtSetArg(arglist[ac], XmNfocusCallback,	    TextGotFocusCB); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;
    assert (ac <= XtNumber(arglist));

    TswTextWidget(tsw,o) =
	DXmCreateCSText ((Widget)tsw, "timeslotText", arglist, ac);

    XtOverrideTranslations
	(TswTextWidget(tsw,o), text_translations_parsed);

    if (TswTopSash(tsw,o))
    {
	XtOverrideTranslations
	    (TswTopSash(tsw,o), sash_translations_parsed);

	XtOverrideTranslations
	    (TswBottomSash(tsw,o), sash_translations_parsed);

	XtOverrideTranslations
	    (TswMoveSash(tsw,o), sash_translations_parsed);
    }

    if (XtHeight (tsw) == 0)
    {
	XtHeight (tsw) =
	 MAX (XtHeight (TswTextWidget(tsw, o)),
	      TswPixmapMargin(tsw, o) + TswPixmapHeight(tsw, o) +
	      TswPixmapMargin(tsw, o));
    }

    if (XtWidth (tsw) == 0)
    {
	XtWidth (tsw) = XtWidth (TswTextWidget(tsw, o)) +
			TswPixmapMargin(tsw, o) + TswPixmapWidth(tsw, o) +
			TswPixmapMargin(tsw, o) + TswKnobSize(tsw, o) +
			TswPixmapMargin(tsw, o);
    }

    ResizeTimeslot(tsw, FALSE);
    XtManageChild (TswTextWidget(tsw, o));
    TswTextChanged(tsw, o) = FALSE;
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
    TimeslotWidget	    tsw = (TimeslotWidget) w;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);

#ifdef DEBUG
    printf ("TimeslotWidget -- Realize Called\n");
#endif

    XtCreateWindow
	(w, InputOutput, CopyFromParent, *window_mask, window_attributes);

    XSetTile
	(XtDisplay (tsw), TswForegroundGC(tsw, o), TswGrayPixmap(tsw, o));

}

static Boolean AcceptFocus
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Time	*time)
#else	/* no prototypes */
	(w, time)
	Widget	w;
	Time	*time;
#endif	/* prototype */
{
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);
    XEvent	    *event;

    return (True);
}

static void redisplay_timeslot
#ifdef _DWC_PROTO_
	(
	TimeslotWidget	tsw,
	XRectangle	*expose_area,
	Region		region)
#else	/* no prototypes */
	(tsw, expose_area, region)
	TimeslotWidget	tsw;
	XRectangle	*expose_area;
	Region		region;
#endif	/* prototype */
    {
    Position	    x;
    Position	    y;
    Dimension	    w;
    Dimension	    h;
    Cardinal	    p;
    Position	    ex1, ex2;
    Position	    ey1, ey2;
    Dimension	    knob_size;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);
    Boolean	    editable = TswEditable(tsw, o);
    Boolean	    dragable = TswDragable(tsw, o);
    Boolean	    selected = TswSelected(tsw, o);
    Boolean	    opened   = TswOpened (tsw,o);
    GC		    fgc	     = TswForegroundGC(tsw, o);
    GC		    bgc      = TswBackgroundGC(tsw, o);
    GC		    sgc      = bgc;
    GC		    ogc	     = fgc;
    Display	    *wd = XtDisplay (tsw);
    Window	    ww  = XtWindow  (tsw);    
    Boolean	    refresh = FALSE;


    if (! XtIsRealized ((Widget) tsw))
    {
	return;
    }

    if (! editable)
    {
	XSetFillStyle (wd, fgc, FillTiled);
    }
  
    /*	  
    **  show that it is selected by highlighting the icon area
    */	  
    if (selected)
    {
	sgc = fgc;
	ogc = bgc;
    }

    refresh = ((expose_area != NULL) || (region != NULL));

    if (refresh) {
	MISCSetGCClipMask (wd, sgc, expose_area, region);
	XFillRectangle
	  (wd, ww, sgc, TswPixmapAreaX   (tsw,o), 0,
			TswPixmapAreaWidth(tsw, o), XtHeight (tsw));
	MISCSetGCClipMask (wd, sgc, NULL, NULL);
    } else {
	XFillRectangle
	  (wd, ww, sgc, TswPixmapAreaX   (tsw,o), 0,
			TswPixmapAreaWidth(tsw, o), XtHeight (tsw));
    }

    if (expose_area == NULL) {
	ex1 = 0;  ex2 = XtWidth  (tsw) - 1;
	ey1 = 0;  ey2 = XtHeight (tsw) - 1;
    }
    else
    {
	ex1 = expose_area->x;  ex2 = ex1 + expose_area->width  - 1;
	ey1 = expose_area->y;  ey2 = ey1 + expose_area->height - 1;
    }

    for (p = 0;  p < TswPixmapCount(tsw, o);  p++)
    {
	get_pixmap_x_and_y(tsw, p, &x, &y);

	if ((x <= ex2) && (x + TswPixmapWidth(tsw,o) > ex1) &&
	    (y <= ey2) && (y + TswPixmapHeight(tsw, o) > ey1))
	{
	    XCopyPlane
	    (
		wd, TswRealPixmaps(tsw,o)[p], ww,
		ogc,	    /* ????? */
		0, 0,
		TswPixmapWidth(tsw, o), TswPixmapHeight(tsw, o),
		x, y,
		1
	    );
	}
    }

    knob_size = TswKnobSize(tsw, o);
    x = TswKnobsLeftMarginX(tsw, o);
    y = TswKnobsTopOfLowerY(tsw, o);

    /*	  
    **  Draw the vertical line between the two resize knobs.
    */	  
    x = x + (knob_size / 2);
    XDrawLine (wd, ww, fgc, x, 0, x, XtHeight (tsw));
    
    if (! editable)
    {
	XSetFillStyle (wd, fgc, FillSolid);
    }

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
    XRectangle	    expose_area;
    TimeslotWidget  tsw = (TimeslotWidget) w;
    
    expose_area.x      = ee->x;
    expose_area.y      = ee->y;
    expose_area.width  = ee->width;
    expose_area.height = ee->height;

    redisplay_timeslot(tsw, &expose_area, region);

}

static void ResizeTimeslot
#ifdef _DWC_PROTO_
	(
	TimeslotWidget	tsw,
	Boolean		setfont)
#else	/* no prototypes */
	(tsw, setfont)
	TimeslotWidget	tsw;
	Boolean		setfont;
#endif	/* prototype */
{
    Arg		    arglist [20];
    Cardinal	    ac;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);
    Boolean	    changed = TswTextChanged(tsw, o);
    int		    x, y;

    setup_timeslot_areas(tsw);

    ac = 0;
    XtSetArg(arglist[ac], XmNtopPosition, 0); ac++;
    XtSetArg(arglist[ac], XmNcursorPosition, 0); ac++;
    XtSetArg(arglist[ac], XmNx,	TswTextAreaX(tsw, o)); ac++;
    XtSetArg(arglist[ac], XmNheight, XtHeight(tsw)); ac++;
    XtSetArg(arglist[ac], XmNwidth,  TswTextAreaWidth(tsw, o)); ac++;
    if (setfont) {
	XtSetArg(arglist[ac], XmNfontList, TswFontList(tsw,o)); ac++;
    }
    assert (ac <= XtNumber(arglist));
    XtSetValues (TswTextWidget(tsw, o), arglist, ac);

    /*
    **  Bahhh....the above does NOT change the text no matter what the
    **	stupid text widget says!
    */
    TswTextChanged(tsw, o) = changed;

    /*
    ** If we have created the sash widgets, then set their geomtries so that
    ** there is one at the top of the icon region near the text widget, one
    ** at the bottom and one that fills the icon region.
    */
    if (TswTopSash(tsw, o))
    {
	x = TswKnobsLeftMarginX(tsw, o);
	y = TswKnobsTopOfLowerY(tsw, o);

	XtVaSetValues
	    ((Widget)TswTopSash(tsw, o), XmNx, x, XmNy, 0, NULL);
	XtVaSetValues
	    ((Widget)TswBottomSash(tsw, o), XmNx, x, XmNy, y, NULL);
	XtVaSetValues
	(
	    (Widget)TswMoveSash(tsw, o),
	    XmNwidth, TswKnobSize(tsw,o),
	    XmNheight, XtHeight(tsw) - (2 * TswKnobSize(tsw,o)),
	    XmNx, TswKnobsLeftMarginX(tsw, o),
	    XmNy, TswKnobSize(tsw,o),
	    NULL
	);
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
    TimeslotWidget  tsw = (TimeslotWidget) w;

    ResizeTimeslot(tsw, FALSE);

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
    Cardinal	    p;
    TimeslotWidget  old = (TimeslotWidget) o;
    TimeslotWidget  new = (TimeslotWidget) n;
    XmOffsetPtr	    oo = TswOffsetPtr(old);
    XmOffsetPtr	    no = TswOffsetPtr(new);
    Boolean	    redisplay = FALSE;
    Boolean	    resize    = FALSE;


    MISCUpdateCallback
    (
	o,
	&(TswHelpCallback (old, oo)),
	n,
	&(TswHelpCallback (new, no)),
	XmNhelpCallback
    );

    MISCUpdateCallback
    (
	o,
	&(TswSingleCallback (old, oo)),
	n,
	&(TswSingleCallback (new, no)),
	DwcTswNsingleCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswDoubleCallback (old, oo)),
	n,
	&(TswDoubleCallback (new, no)),
	DwcTswNdoubleCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswDragStartCallback (old, oo)),
	n,
	&(TswDragStartCallback (new, no)),
	DwcTswNdragStartCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswDragCallback (old, oo)),
	n,
	&(TswDragCallback (new, no)),
	DwcTswNdragCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswDragEndCallback (old, oo)),
	n,
	&(TswDragEndCallback (new, no)),
	DwcTswNdragEndCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswResizeStartCallback (old, oo)),
	n,
	&(TswResizeStartCallback (new, no)),
	DwcTswNresizeStartCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswResizeCallback (old, oo)),
	n,
	&(TswResizeCallback (new, no)),
	DwcTswNresizeCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswResizeEndCallback (old, oo)),
	n,
	&(TswResizeEndCallback (new, no)),
	DwcTswNresizeEndCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswFocusCallback (old, oo)),
	n,
	&(TswFocusCallback (new, no)),
	DwcTswNfocusCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswCloseCallback (old, oo)),
	n,
	&(TswCloseCallback (new, no)),
	DwcTswNcloseCallback
    );
		       
    MISCUpdateCallback
    (
	o,
	&(TswExtendSelectCallback (old, oo)),
	n,
	&(TswExtendSelectCallback (new, no)),
	DwcTswNextendSelectCallback
    );
		       
    /*
    **  Update pixmap pointers if needed.  If the new resource is NULL then
    **	either the resource wasn't set or it was set specifically to NULL.  If
    **	the new resource is non-null then it was definitely set.
    */
        
    if (TswPixmaps (new, no) == NULL) {

	/*
	**  If the new resource is NULL and the count is the same, then retain
	**  previous pixmaps.  If the count is different then it better be zero!
	*/
	
        if (TswPixmapCount (old, oo) != TswPixmapCount (new, no)) {
	    if (TswPixmapCount (new, no) == 0) {
		XtFree ((char *) TswRealPixmaps (new, no));
		TswRealPixmaps (new, no) = NULL;
		redisplay = TRUE;
	    } else {
		XtAppError
		(
		    XtWidgetToApplicationContext ((Widget) new),
		    "Timeslot Widget: Non-zero Pixmap Count with NULL Pixmaps"
		);
	    }
	}

    } else {

	/*
	**  New pixmaps resource set.  If the count is the same then use
	**  existing real pixmaps array and copy new pointers into it.  If any
	**  are different then we must redisplay.
	*/
	
        if (TswPixmapCount (old, oo) == TswPixmapCount (new, no)) {
	    for (p = 0;  p < TswPixmapCount (new, no);  p++) {
		if (TswRealPixmaps (new, no) [p] != TswPixmaps (new, no) [p]) {
		    TswRealPixmaps (new, no) [p] =  TswPixmaps (new, no) [p];
		    redisplay = TRUE;
		}
	    }
	} else {

	    /*
	    **  New pixmaps resource and a new count.  We will definitely
	    **	redisplay.  Free up previous real-pixmaps storage and, if the
	    **	count is non-zero copy the new ones.
	    */

	    redisplay = TRUE;
	    XtFree ((char *) TswRealPixmaps (new, no));

	    if (TswPixmapCount (new, no) == 0) {
		TswRealPixmaps (new, no) = NULL;
	    } else {
		TswRealPixmaps (new, no) = (Pixmap *)
		  XtMalloc (sizeof (Pixmap) * TswPixmapCount (new, no));
		for (p = 0;  p < TswPixmapCount (new, no);  p++) {
		    TswRealPixmaps (new, no) [p] = TswPixmaps (new, no) [p];
		}
	    }
	}

	/*
	**  *Important*.  Set the resource to NULL so we can detect the next
	**  change.
	*/

	TswPixmaps (new, no) = NULL;

    }


    if ((TswPixmapWidth  (old, oo) != TswPixmapWidth  (new, no)) ||
        (TswPixmapHeight (old, oo) != TswPixmapHeight (new, no)) ||
        (TswPixmapMargin (old, oo) != TswPixmapMargin (new, no)) ||
        (TswPixmapCount  (old, oo) != TswPixmapCount  (new, no)) ||
	(TswDirectionRToL(old, oo) != TswDirectionRToL(new,no)) ||
        (TswFontList	 (old, oo) != TswFontList     (new, no)) ||
        (TswKnobSize     (old, oo) != TswKnobSize     (new, no))) {

	redisplay = TRUE;
	resize    = TRUE;

    }

    if (TswEditable (old, oo) != TswEditable (new, no))
    {
	DXmCSTextSetEditable
	    ((DXmCSTextWidget) TswTextWidget (new, no), TswEditable (new, no));
	redisplay = TRUE;
    }

    redisplay = redisplay || (TswDragable (old, oo) != TswDragable (new, no));

    if (resize) {
	ResizeTimeslot (new, (TswFontList(old,oo) != TswFontList(new,no)));
    }

    return (redisplay && XtIsRealized ((Widget) new));

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
    TimeslotWidget	tsw = (TimeslotWidget) w;
    XmOffsetPtr		o = TswOffsetPtr(tsw);

    MBAFreeContext(TswMbaContext(tsw, o));

    XtRemoveAllCallbacks (w, XmNhelpCallback);
    XtRemoveAllCallbacks (w, DwcTswNsingleCallback);
    XtRemoveAllCallbacks (w, DwcTswNdoubleCallback);
    XtRemoveAllCallbacks (w, DwcTswNdragStartCallback);
    XtRemoveAllCallbacks (w, DwcTswNdragCallback);
    XtRemoveAllCallbacks (w, DwcTswNdragEndCallback);
    XtRemoveAllCallbacks (w, DwcTswNresizeStartCallback);
    XtRemoveAllCallbacks (w, DwcTswNresizeCallback);
    XtRemoveAllCallbacks (w, DwcTswNresizeEndCallback);
    XtRemoveAllCallbacks (w, DwcTswNfocusCallback);
    XtRemoveAllCallbacks (w, DwcTswNlosingFocusCallback);
    XtRemoveAllCallbacks (w, DwcTswNcloseCallback);
    XtRemoveAllCallbacks (w, DwcTswNextendSelectCallback);

}

Widget
TSWTimeslotCreate
#ifdef _DWC_PROTO_
	(
	Widget	parent,
	char	*name,
	ArgList	args,
	int	argCount)
#else	/* no prototypes */
	(parent, name, args, argCount)
	Widget	parent;
	char	*name;
	ArgList	args;
	int	argCount;
#endif	/* prototype */
    {

    return (XtCreateWidget (name, timeslotWidgetClass, parent,
			    args, argCount));

}

/* See whether the event was on a resize knob or not. Returns TRUE if it    */
/* was, FALSE  if it wasn't. top_knob is set to TRUE or FALSE depending	    */
/* upon whether the event was on the top_knob or not. */
static Boolean
on_a_resize_knob
#ifdef _DWC_PROTO_
	(
	TimeslotWidget	tsw,
	XButtonEvent	*event,
	Boolean		*top_knob)
#else	/* no prototypes */
	(tsw, event, top_knob)
	TimeslotWidget	tsw;
	XButtonEvent	*event;
	Boolean		*top_knob;
#endif	/* prototype */
    {
    XmOffsetPtr		o = TswOffsetPtr(tsw);
    Position		knob_x    = TswKnobsLeftMarginX(tsw, o);
    Position		knob_y    = TswKnobsTopOfLowerY(tsw, o);
    Dimension		knob_size = TswKnobSize(tsw, o);

    /*	  
    **  Are we within the x range of the knobs?
    */	  
    if ((event->x < knob_x) || (event->x >= knob_x + knob_size)) {
	return (FALSE);
    }

    /*	  
    **  Is the event between the top of the timeslot and the bottom of the
    **	top_knob?
    */	  
    if (event->y < knob_size) {
	*top_knob = TRUE;
	return (TRUE);
    }
    
    /*	  
    **  We already know that we've selected one of the two knobs at this point
    **	since we're within the x range. If the place selected is greater than or
    **	equal to the top of the bottom knob then we've selected the bottom knob.
    **	[@tbs@]...
    */	  
    if (event->y >= knob_y) {
	*top_knob = FALSE;
	return (TRUE);
    }

    return (FALSE);

}

#if !defined (AUD)
static
#endif
void ACTION_TSW_MB1UP
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
{
    MbaAction		    action;
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) w;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);
    CalendarDisplay	    cd;

    action = MBAMouseButton1Up(TswMbaContext(tsw, o), (XEvent *) event);
    if (action == MbaIgnoreEvent)
    {
	return;
    }


    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened (tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event	 = (XEvent *) event;

    if (TswResizing(tsw, o))
    {
	cbs.reason   = DwcTswCRResizeEnd;
	cbs.top_knob = TswTopKnob(tsw, o);
	TswResizing(tsw, o) = FALSE;

	XtCallCallbackList ((Widget) tsw, TswResizeEndCallback(tsw,o), &cbs);

	if (TswTopKnob(tsw,o))
	{
	    XUndefineCursor
	    (
		XtDisplay (TswTopSash(tsw,o)),
		XtWindow (TswTopSash(tsw,o))
	    );
	}
	else
	{
	    XUndefineCursor
	    (
		XtDisplay (TswBottomSash(tsw,o)),
		XtWindow (TswBottomSash(tsw,o))
	    );
	}

	return;
    }

    if (TswDragging(tsw, o))
    {
	cbs.reason = DwcTswCRDragEnd;
	TswDragging(tsw, o) = FALSE;

	XtCallCallbackList ((Widget) tsw, TswDragEndCallback(tsw,o), &cbs);


	XUndefineCursor (XtDisplay (tsw), XtWindow (tsw));
	XUndefineCursor
	(
	    XtDisplay(TswMoveSash(tsw,o)),
	    XtWindow(TswMoveSash(tsw,o))
	);
	return;
    }

    cbs.reason = DwcTswCRSingle;

    XtCallCallbackList ((Widget) tsw, TswSingleCallback(tsw,o), &cbs);

}

#if !defined (AUD)
static
#endif
void ACTION_TSW_EXTEND
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
{
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) w;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);


    cbs.reason   = DwcTswCRExtendSelect;
    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened (tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event	 = (XEvent *) event;

    XtCallCallbackList ((Widget) tsw, TswExtendSelectCallback(tsw,o), &cbs);
    
}

#if !defined (AUD)
static
#endif
void ACTION_TSW_SELECT
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XEvent		*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XEvent		*event;
#endif	/* prototype */
{
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) XtParent(w);
    XmOffsetPtr		    o = TswOffsetPtr(tsw);

    cbs.reason   = DwcTswCRExtendSelect;
    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened (tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event	 = (XEvent *) event;

    XtCallCallbackList ((Widget) tsw, TswExtendSelectCallback(tsw,o), &cbs);
}

#if !defined (AUD)
static
#endif
void ACTION_TSW_CANCEL
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
{
    MbaAction		    action;
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw;
    XmOffsetPtr		    o;

    tsw = (TimeslotWidget) find_timeslot(w);
    if (tsw == NULL) return;

    o = TswOffsetPtr(tsw);

    action = MBAMouseButtonOther(TswMbaContext(tsw, o), (XEvent *) event);

    if (action == MbaIgnoreEvent)
    {
	return;
    }


    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened (tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event	 = (XEvent *) event;

    if (TswResizing(tsw, o))
    {
	cbs.reason   = DwcTswCRResizeCancel;
	cbs.top_knob = TswTopKnob(tsw, o);
	TswResizing(tsw, o) = FALSE;

	XtCallCallbackList ((Widget) tsw, TswResizeCancelCallback(tsw,o), &cbs);

	if (TswTopKnob(tsw,o))
	{
	    XUndefineCursor
	    (
		XtDisplay (TswTopSash(tsw,o)),
		XtWindow (TswTopSash(tsw,o))
	    );
	}
	else
	{
	    XUndefineCursor
	    (
		XtDisplay (TswBottomSash(tsw,o)),
		XtWindow (TswBottomSash(tsw,o))
	    );
	}

	return;
    }

    if (TswDragging(tsw, o))
    {
	cbs.reason = DwcTswCRDragCancel;
	TswDragging(tsw, o) = FALSE;

	XtCallCallbackList ((Widget) tsw, TswDragCancelCallback(tsw,o), &cbs);

	XUndefineCursor (XtDisplay (tsw), XtWindow (tsw));
	XUndefineCursor
	(
	    XtDisplay(TswMoveSash(tsw,o)),
	    XtWindow(TswMoveSash(tsw,o))
	);
	return;
    }

}

/* We got a MB1 Button Down event */
#if !defined (AUD)
static
#endif
void ACTION_TSW_MB1DOWN
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
    {
    MbaAction		    action;
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) w;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);
    CalendarDisplay	    cd;
    
    action = MBAMouseButton1Down(TswMbaContext(tsw, o), (XEvent *) event);
    if (action == MbaIgnoreEvent)
    {
	return;
    }

    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened (tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event    = (XEvent *) event;

    if (action == MbaDoubleClick)
    {
	cbs.reason = DwcTswCRDouble;

	XtCallCallbackList ((Widget) tsw, TswDoubleCallback(tsw,o), &cbs);

	return;
    }

    
    /*	  
    **  Did the user select one of the resize knobs?
    */	  
    if (on_a_resize_knob (tsw, event, &cbs.top_knob))
    {

	if ((! TswOpened(tsw, o)) ||
	    (! TswEditable(tsw, o)) ||
	    (! TswDragable(tsw, o)))
	{
	    return;
	}

	if (cbs.top_knob)
	{
	    XDefineCursor
		(XtDisplay (tsw), XtWindow (tsw), resize_top_cursor);
	}
	else
	{
	    XDefineCursor
		(XtDisplay (tsw), XtWindow (tsw), resize_bottom_cursor);
	}

	cbs.reason = DwcTswCRResizeStart;
	TswResizing(tsw, o) = TRUE;
	TswTopKnob(tsw, o)  = cbs.top_knob;

	XtCallCallbackList ((Widget) tsw, TswResizeStartCallback(tsw,o), &cbs);

    }

}

static void start_motion
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
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) w;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);
    CalendarDisplay	    cd;


    if ((! TswEditable(tsw, o)) || (TswResizing(tsw, o)) ||
        (! TswDragable(tsw, o)))
    {
	return;
    }

    TswDragging(tsw, o) = TRUE;

    /*
    **  Turn on the move cursor
    */	  
    XDefineCursor (XtDisplay (tsw), XtWindow (tsw), move_cursor);
    XDefineCursor
    (
	XtDisplay(TswMoveSash(tsw,o)),
	XtWindow(TswMoveSash(tsw,o)),
	move_cursor
    );

    cbs.reason   = DwcTswCRDragStart;
    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened (tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event    = event;

    XtCallCallbackList ((Widget) tsw, TswDragStartCallback(tsw,o), &cbs);

}

#if !defined (AUD)
static
#endif
void ACTION_TSW_MB1MOTION
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
{
    MbaAction		    action;
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) w;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);


    action = MBAMouseButton1Motion
	(TswMbaContext(tsw, o), (XEvent *) event);

    if ((! TswEditable(tsw, o)) ||
	(! TswDragable(tsw, o)) ||
	(action == MbaIgnoreEvent))
    {
	return;
    }

    if ((action == MbaMotionStart) && (! TswResizing(tsw, o)))
    {
	start_motion (w, (XEvent *) event);
	return;
    }

    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened (tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event    = (XEvent *) event;

    if (TswResizing(tsw, o))
    {
	cbs.reason   = DwcTswCRResize;
	cbs.top_knob = TswTopKnob(tsw,o);

	XtCallCallbackList ((Widget) tsw, TswResizeCallback(tsw,o), &cbs);

	return;
    }

    if (TswDragging(tsw, o))
    {
	cbs.reason = DwcTswCRDrag;

	XtCallCallbackList ((Widget) tsw, TswDragCallback(tsw,o), &cbs);

	return;
    }    

}

void TSWSetSelected
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Boolean	selected)
#else	/* no prototypes */
	(w, selected)
	Widget	w;
	Boolean	selected;
#endif	/* prototype */
    {
    TimeslotWidget	tsw = (TimeslotWidget) w;
    XmOffsetPtr		o = TswOffsetPtr(tsw);

    if (TswSelected(tsw, o) != selected) {
	TswSelected(tsw, o) = selected;
	redisplay_timeslot(tsw, NULL, NULL);
    }
}

static void set_timeslot_opened
#ifdef _DWC_PROTO_
	(
	TimeslotWidget	tsw,
	Boolean		opened,
	Time		time)
#else	/* no prototypes */
	(tsw, opened, time)
	TimeslotWidget	tsw;
	Boolean		opened;
	Time		time;
#endif	/* prototype */
    {
    Arg			arglist [10];
    Cardinal		ac;
    Window		focwin;
    Widget		focwid;
    int			revert;
    XmOffsetPtr		o = TswOffsetPtr(tsw);
    Boolean		editable = TswEditable(tsw, o);

    ac = 0;
    if (! opened) {
	XtSetArg(arglist[ac], XmNtopPosition, 0); ac++;
	XtSetArg(arglist[ac], XmNcursorPosition, 0); ac++;
    }
    XtSetArg(arglist[ac], XmNcursorPositionVisible, opened & editable); ac++;
    XtSetArg(arglist[ac], XmNeditable,	       opened & editable); ac++;
    assert (ac <= XtNumber(arglist));
    XtSetValues (TswTextWidget(tsw, o), arglist, ac);

    TswOpened(tsw, o) = opened;
    if (opened && TswDragable(tsw,o) && editable)
    {
	XtManageChild (TswTopSash(tsw, o));
	XtManageChild (TswBottomSash(tsw, o));
	XtManageChild (TswMoveSash(tsw, o));
	XUndefineCursor
	    (XtDisplay (TswTopSash(tsw,o)), XtWindow (TswTopSash(tsw,o)));
	XUndefineCursor
	    (XtDisplay (TswMoveSash(tsw,o)), XtWindow (TswMoveSash(tsw,o)));
	XUndefineCursor
	    (XtDisplay (TswBottomSash(tsw,o)), XtWindow (TswBottomSash(tsw,o)));
    }
    else if (TswDragable(tsw,o))
    {
	XtUnmanageChild (TswTopSash(tsw, o));
	XtUnmanageChild (TswBottomSash(tsw, o));
	XtUnmanageChild (TswMoveSash(tsw, o));
    }


}

void TSWSetOpened
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Boolean	opened,
	Time	time)
#else	/* no prototypes */
	(w, opened, time)
	Widget	w;
	Boolean	opened;
	Time	time;
#endif	/* prototype */
{
    Arg			arglist [20];
    Cardinal		ac;
    TimeslotWidget	tsw = (TimeslotWidget) w;
    XmOffsetPtr		o = TswOffsetPtr(tsw);

    if (TswOpened(tsw, o) != opened)
    {
	set_timeslot_opened(tsw, opened, time);
	redisplay_timeslot(tsw, NULL, NULL);
    }

}

void TSWSetEditable
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Boolean	editable,
	Time	time)
#else	/* no prototypes */
	(w, editable, time)
	Widget	w;
	Boolean	editable;
	Time	time;
#endif	/* prototype */
{
    Arg			arglist [20];
    Cardinal		ac;
    TimeslotWidget	tsw = (TimeslotWidget) w;
    XmOffsetPtr		o = TswOffsetPtr(tsw);

    if (TswEditable(tsw, o) != editable)
    {
	TswEditable(tsw, o) = editable;
	set_timeslot_opened(tsw, TswOpened(tsw, o), time);
	redisplay_timeslot(tsw, NULL, NULL);
    }
}

void TSWSetState
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Boolean	selected,
	Boolean	opened,
	Boolean	editable,
	Time	time)
#else	/* no prototypes */
	(w, selected, opened, editable, time)
	Widget	w;
	Boolean	selected;
	Boolean	opened;
	Boolean	editable;
	Time	time;
#endif	/* prototype */
{
    Arg			arglist [20];
    Cardinal		ac;
    TimeslotWidget	tsw = (TimeslotWidget) w;
    XmOffsetPtr		o = TswOffsetPtr(tsw);

    if ((TswSelected(tsw, o) == selected) &&
        (TswOpened (tsw,o) == opened)   &&
        (TswEditable(tsw, o) == editable)) {
	return;
    }
      
    TswSelected(tsw, o) = selected;

    if ((TswOpened(tsw, o) != opened) || (TswEditable(tsw, o) != editable)) {
        TswEditable(tsw, o) = editable;
	set_timeslot_opened(tsw, opened, time);
    }

    redisplay_timeslot(tsw, NULL, NULL);

}

static void TEXT_GOT_FOCUS
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    TimeslotWidget	    tsw = (TimeslotWidget) tag;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);
    DwcTswCallbackStruct    tsw_cbs;
    Time		    time;
    CalendarDisplay	    cd;
    DwcDswEntry		    entry;

    /*	  
    **  Get the open one if any and close it
    */	  
    (void)MISCFindCalendarDisplay(&cd,w);
    time = MISCGetTimeFromAnyCBS(cbs);
    entry = DSWGetOpenEntry((DayslotsWidget)XtParent(tsw));
    if (tsw != (TimeslotWidget) DSWGetEntryTimeslotWidget (entry))
	DSWRequestCloseOpenEntry((DayslotsWidget)XtParent(tsw), time);

    tsw_cbs.reason   = DwcTswCRFocus;
    tsw_cbs.selected = TswSelected(tsw, o);
    tsw_cbs.opened   = TswOpened (tsw,o);
    tsw_cbs.editable = TswEditable(tsw, o);
    tsw_cbs.event    = cbs->event;

    XtCallCallbackList ((Widget) tsw, TswFocusCallback(tsw,o), &tsw_cbs);

}

static void TEXT_LOSING_FOCUS
#ifdef _DWC_PROTO_
	(
	Widget				w,
	caddr_t				tag,
	DXmCSTextVerifyCallbackStruct	*text_cbs)
#else	/* no prototypes */
	(w, tag, text_cbs)
	Widget				w;
	caddr_t				tag;
	DXmCSTextVerifyCallbackStruct	*text_cbs;
#endif	/* prototype */
{
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) tag;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);

    cbs.reason   = DwcTswCRLosingFocus;
    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened (tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event    = text_cbs->event;

    XtCallCallbackList ((Widget) tsw, TswLosingFocusCallback(tsw,o), &cbs);
    
}

#if !defined (AUD)
static
#endif
void ACTION_TSW_HELP
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
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) w;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);

#ifdef DEBUG
    printf ("Timeslot Widget -- Help callback\n");
#endif

    cbs.reason = (int)XmCR_HELP;
    cbs.event  = event;

    XtCallCallbackList ((Widget) tsw, TswHelpCallback(tsw,o), &cbs);

} 

static void TEXT_HELP
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			client_data,
	XmAnyCallbackStruct	*text_cbs)
#else	/* no prototypes */
	(w, client_data, text_cbs)
	Widget			w;
	caddr_t			client_data;
	XmAnyCallbackStruct	*text_cbs;
#endif	/* prototype */
{
    TimeslotWidget	    tsw = (TimeslotWidget) client_data;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);

    XtCallCallbackList ((Widget) tsw, TswHelpCallback(tsw,o), NULL);

}

#if !defined (AUD)
static
#endif
void ACTION_TSW_CLOSE
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
    DwcTswCallbackStruct    cbs;
    TimeslotWidget	    tsw = (TimeslotWidget) XtParent (w);
    XmOffsetPtr		    o = TswOffsetPtr(tsw);

    cbs.reason   = DwcTswCRClose;
    cbs.selected = TswSelected(tsw, o);
    cbs.opened   = TswOpened(tsw,o);
    cbs.editable = TswEditable(tsw, o);
    cbs.event    = event;

    XtCallCallbackList ((Widget) tsw, TswCloseCallback(tsw,o), &cbs);

} 

static void
TEXT_CHANGED
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct *cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget	w;
	caddr_t	tag;
	XmAnyCallbackStruct *cbs;
#endif	/* prototype */
    {
    Arg		    arglist[10];
    Cardinal	    ac;
    TimeslotWidget  tsw = (TimeslotWidget) tag;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

#ifdef DEBUG
    if (! TswTextChanged(tsw, o)) {
	printf ("TEXT_CHANGED\n");
    }
#endif

    /*	  
    **  Make sure that the beginning of the text shows.
    */	  
    TswTextChanged(tsw, o) = TRUE;

}

void TSWSetText
#ifdef _DWC_PROTO_
	(
	Widget	w,
	char	*text)
#else	/* no prototypes */
	(w, text)
	Widget	w;
	char	*text;
#endif	/* prototype */
{
    TimeslotWidget	tsw = (TimeslotWidget) w;
    XmOffsetPtr		o = TswOffsetPtr(tsw);
    XmString		xm_text;
    long		byte_count, cvt_status;

    xm_text = DXmCvtFCtoCS(text, &byte_count, &cvt_status);

    /*	  
    **  Make sure that the beginning of the text shows
    */
    XtVaSetValues
    (
	TswTextWidget(tsw, o),
	XmNvalue, xm_text,
	XmNtopPosition, 0,
	XmNcursorPosition, 0,
	NULL
    );

    XmStringFree(xm_text);

    TswTextChanged(tsw, o) = FALSE;

}

void TSWSetCSText
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XmString	text)
#else	/* no prototypes */
	(w, text)
	Widget		w;
	XmString	text;
#endif	/* prototype */
{
    TimeslotWidget	    tsw = (TimeslotWidget) w;
    XmOffsetPtr		    o = TswOffsetPtr(tsw);

    /*	  
    **  Make sure that the beginning of the text shows
    */
    XtVaSetValues
    (
	TswTextWidget(tsw, o),
	XmNvalue, text,
	XmNtopPosition, 0,
	XmNcursorPosition, 0,
	NULL
    );
    XtVaSetValues
    (
	TswTextWidget(tsw, o),
	XmNtopPosition, 0,
	XmNcursorPosition, 0,
	NULL
    );

    TswTextChanged(tsw, o) = FALSE;

}

char *TSWGetText
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);
    char *text;
    XmString	    xm_text;
    long	    byte_count, cvt_status;

#ifdef DEBUG
    printf ("Timeslot Widget -- Text Get\n");
#endif

    /*	  
    **  XmTextGetString can return a NULL, DwtSTextGetString returned a "", so
    **	mimic the previous behaviour
    */	  
    xm_text = DXmCSTextGetString (TswTextWidget(tsw,o));
    if (xm_text)
    {
	text = DXmCvtCStoFC (xm_text, &byte_count, &cvt_status);
	XmStringFree(xm_text);
    }
    else
    {
	text = NULL;
    }

    if (text == NULL)
    {
	text = XtMalloc (1);
	text[0] = '\0';
    }

    return (text);

}

XmString TSWGetCSText
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);
    XmString	    xm_text;
    long	    byte_count, cvt_status;

#ifdef DEBUG
    printf ("Timeslot Widget -- Text Get\n");
#endif

    /*
    **  XmTextGetString can return a NULL, DwtSTextGetString returned a "", so
    **	mimic the previous behaviour
    */	  
    xm_text = DXmCSTextGetString (TswTextWidget(tsw,o));

    return (xm_text);

}

Boolean TSWTextChanged
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Boolean	reset)
#else	/* no prototypes */
	(w, reset)
	Widget	w;
	Boolean	reset;
#endif	/* prototype */
    {
    Boolean	    changed;
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    changed = TswTextChanged(tsw, o);
    if (reset) {
	TswTextChanged(tsw, o) = FALSE;
    }

    return (changed);
    
}

Boolean TSWGetEditable
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    return (TswEditable(tsw, o));
    
}

Boolean
TSWGetSelected
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    return (TswSelected(tsw, o));
    
}

Dimension
TSWGetTrimmingsWidth
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    return (TswPixmapMargin(tsw, o) + TswPixmapWidth(tsw, o) +
	    TswPixmapMargin(tsw, o) + (TswKnobSize(tsw, o) / 2));

}

Widget TSWGetTextWidget
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    return (TswTextWidget(tsw, o));

}

Widget TSWGetTopSash
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    return (TswTopSash(tsw, o));

}

Widget TSWGetBottomSash
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    return (TswBottomSash(tsw, o));

}

Widget TSWGetMoveSash
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    return (TswMoveSash(tsw, o));

}

void
TSWReset
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    TimeslotWidget  tsw = (TimeslotWidget) w;
    XmOffsetPtr	    o = TswOffsetPtr(tsw);

    /*	  
    **  Reset the state engine to its initial state and remove and pending
    **	timers.
    */	  
    MBAResetContext(TswMbaContext(tsw, o));

}

/* We're constructing a fake callback so that we get the same effect as if  */
/* the timeslot got a focus callback */
#if !defined (AUD)
static
#endif
void ACTION_TSW_OPEN
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XEvent		*keyevent)
#else	/* no prototypes */
	(w, keyevent)
	Widget		w;
	XEvent		*keyevent;
#endif	/* prototype */
{
    int			status;
    Widget		tsw;
    XmAnyCallbackStruct constructed_cbs;
    CalendarDisplay	cd;
    DwcDswEntry		entry;

    (void)MISCFindCalendarDisplay(&cd, w);

    /*	  
    **  Run up the tree looking for a timeslot widget and just return if we
    **	can't seem to find one (ie ignore whatever the user did)
    */	  
    tsw = find_timeslot(w);
    if (tsw == NULL)
	return;

    /*	  
    **  Is there an open entry? If there is, return since for right now this is
    **	being used as a way to open a timeslot that has keyboard focus but is
    **	closed.
    */	  
    entry = DSWGetOpenEntry((DayslotsWidget)XtParent(tsw));
    if (entry != NULL)
	return;
	
    constructed_cbs.reason = 1;             /* dummy reason */
    constructed_cbs.event = keyevent;
    TEXT_GOT_FOCUS(w, (caddr_t)tsw, &constructed_cbs);

    return;     
}     

static Widget
find_timeslot
#ifdef _DWC_PROTO_
	(
	Widget		w)
#else	/* no prototypes */
	(w)
	Widget		w;
#endif	/* prototype */
{
    Widget tsw = NULL;
    Widget temp;

    temp = w;
    do 
	{
	if (XtIsSubclass(temp, timeslotWidgetClass))
	    tsw = temp;
	temp = XtParent( temp );
	} while ( temp != 0 );

    return ( tsw );
    
}

static void HandleSash
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	closure,	/* (paned) Widget (unused) */
	caddr_t	callData	/* SashCallData */
	)
#else
	(w, closure, callData)
	Widget	w;
	caddr_t	closure;	/* (paned) Widget (unused) */
	caddr_t	callData;	/* SashCallData */
#endif
{
    SashCallData		call_data = (SashCallData)callData;
    register TimeslotWidget	tsw = (TimeslotWidget) closure;
    register XmSashWidget	sw = (XmSashWidget) w;
    DayslotsWidget		dsw = (DayslotsWidget) XtParent(tsw);
    XmOffsetPtr			o = TswOffsetPtr(tsw);
    DwcTswCallbackStruct	cbs;
    int				diff, y;
    short			increment;
    short			c_index;
    char			ActionType;
    Boolean			resize;
    Boolean			top;
    XEvent			*event = call_data->event;

    if (call_data->num_params == 0)
    {
#if 0
	_XmWarning (tsw, MESSAGE8);
#endif
	return;
    }

    /*
    ** Determine which adjustments to make here.
    */
    top = ((Widget)sw == TswTopSash(tsw, o));
    resize = ((Widget)sw != TswMoveSash(tsw, o));

    /*
    **
    */
    switch (call_data->event->xany.type)
    {
    case KeyRelease:
	return;
    case KeyPress:
	if (call_data->num_params < 3)
	{
#if 0
	    _XmWarning (tsw, MESSAGE8);
#endif
	    return;
	}
	if (*call_data->params[2] == 'U')
	{
	    if (*call_data->params[1] == 'L')
		increment = -2;
	    else
		increment = -1;
	}
	else if (*call_data->params[2] == 'D')
	{
	    if (*call_data->params[1] == 'L')
		increment = 2;
	    else
		increment = 1;
	}

	cbs.selected = TswSelected(tsw, o);
	cbs.opened   = TswOpened (tsw,o);
	cbs.editable = TswEditable(tsw, o);
	cbs.event = (XEvent *) event;

	if (resize)
	{
	    cbs.top_knob = top;
	    cbs.reason   = increment;

	    XtCallCallbackList
		((Widget) tsw, TswIncrementSizeCallback(tsw,o), &cbs);

	}
	else
	{
	    cbs.reason   = increment;

	    XtCallCallbackList
		((Widget) tsw, TswIncrementPositionCallback(tsw,o), &cbs);
	}
	return;

    }

    ActionType = *call_data->params[0];
    if (islower(ActionType)) ActionType = toupper(ActionType);

    switch (ActionType)
    {
    case 'S':						/* Start adjustment */
	cbs.selected = TswSelected(tsw, o);
	cbs.opened   = TswOpened (tsw,o);
	cbs.editable = TswEditable(tsw, o);
	cbs.event = (XEvent *) event;

	if (resize)
	{
	    TswResizing(tsw, o) = True;
	    TswTopKnob(tsw, o)  = top;
	    cbs.top_knob = top;
	    cbs.reason   = DwcTswCRResizeStart;

	    XtCallCallbackList
		((Widget) tsw, TswResizeStartCallback(tsw,o), &cbs);

	    if (top)
	    {
		XDefineCursor
		    (XtDisplay (w), XtWindow (w), resize_top_cursor);
	    }
	    else
	    {
		XDefineCursor
		    (XtDisplay (w), XtWindow (w), resize_bottom_cursor);
	    }

	}
	else
	{
	    TswDragging(tsw, o) = True;
	    cbs.reason   = DwcTswCRDragStart;

	    XtCallCallbackList
		((Widget) tsw, TswDragStartCallback(tsw,o), &cbs);

	    XDefineCursor (XtDisplay(w), XtWindow(w), move_cursor);
	    XDefineCursor (XtDisplay(tsw), XtWindow(tsw), move_cursor);
	}
	break;
    case 'M':						/* Move adjustment */
	cbs.selected = TswSelected(tsw, o);
	cbs.opened   = TswOpened (tsw,o);
	cbs.editable = TswEditable(tsw, o);
	cbs.event    = (XEvent *) event;

	if (TswResizing(tsw, o))
	{
	    cbs.reason   = DwcTswCRResize;
	    cbs.top_knob = TswTopKnob(tsw,o);

	    XtCallCallbackList ((Widget) tsw, TswResizeCallback(tsw,o), &cbs);

	    return;
	}

	if (TswDragging(tsw, o))
	{
	    cbs.reason = DwcTswCRDrag;

	    XtCallCallbackList ((Widget) tsw, TswDragCallback(tsw,o), &cbs);

	    return;
	}    
	break;
    case 'C':						/* Commit */
	cbs.selected = TswSelected(tsw, o);
	cbs.opened   = TswOpened (tsw,o);
	cbs.editable = TswEditable(tsw, o);
	cbs.event = (XEvent *) event;

	if (TswResizing(tsw, o))
	{
	    cbs.reason   = DwcTswCRResizeEnd;
	    cbs.top_knob = TswTopKnob(tsw, o);
	    TswResizing(tsw, o) = False;

	    XtCallCallbackList
		((Widget) tsw, TswResizeEndCallback(tsw,o), &cbs);

	    XUndefineCursor (XtDisplay (w), XtWindow (w));

	    return;
	}

	if (TswDragging(tsw, o))
	{
	    cbs.reason = DwcTswCRDragEnd;
	    TswDragging(tsw, o) = False;

	    XtCallCallbackList
		((Widget) tsw, TswDragEndCallback(tsw,o), &cbs);

	    XUndefineCursor (XtDisplay (w), XtWindow (w));
	    XUndefineCursor (XtDisplay (tsw), XtWindow (tsw));

	    return;
	}
	break;
    default:
#if 0
	_XmWarning (tsw, MESSAGE9);
#endif
	return;
    }
}

Boolean TSWTraverseToText
#ifdef _DWC_PROTO_
	(
	Widget		w)
#else	/* no prototypes */
	(w)
	Widget		w;
#endif	/* prototype */
{
    register TimeslotWidget	tsw = (TimeslotWidget) w;
    XmOffsetPtr			o = TswOffsetPtr(tsw);

    return (XmProcessTraversal (TswTextWidget(tsw, o), XmTRAVERSE_CURRENT));
}

Boolean TSWTraverseToSash
#ifdef _DWC_PROTO_
	(
	Widget		w)
#else	/* no prototypes */
	(w)
	Widget		w;
#endif	/* prototype */
{
    register TimeslotWidget	tsw = (TimeslotWidget) w;
    XmOffsetPtr			o = TswOffsetPtr(tsw);

    return (XmProcessTraversal (TswTopSash(tsw, o), XmTRAVERSE_CURRENT));
}

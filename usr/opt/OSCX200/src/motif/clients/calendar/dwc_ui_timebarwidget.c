/* dwc_ui_timebarwidget.c */
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
**	DECwindows Calendar Timebar Display
**
**  AUTHOR:
**
**	Marios Cleovoulou, March-1989
**
**  ABSTRACT:
**
**	This module implements the timebar display
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3.0-003 Paul Ferwerda					18-Oct-1990
**		Set up own get_font routine so not dependent on MISC routine.
**  V3-002  Paul Ferwerda					05-Sep-1990
**		Port to Motif.  Took out DwtCommon. Sub-classed it off of
**		primitive (no good reason). Added initialization of
**		TbwForegroundGC and TbwGrayPixmap since we can't get it from
**		DwtCommon anymore. Also added code to SetValues to update
**		TbwForegroundGC as appropriate and code to Destroy to get rid
**		of it. Did the same for the CreateGidget and DestroyGidget
**		stuff. Fixed setup_timebar code so it wouldn't loop if a time
**		of midnight was passed in (UWS QAR 2755).
**
**	V1-001  Marios Cleovoulou	  			 Mar-1989
**		Initial version.
**--
**/

#include "dwc_compat.h"

#include <stdio.h>

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Intrinsic.h>			/* for MIN */
#include <X11/StringDefs.h>			/* XtRint... */
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_timebarwidgetp.h"
#include "dwc_ui_catchall.h"			/* for DWC$UI_NOFONTINIT */
#include "dwc_ui_misc.h"

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

static Boolean SetValues PROTOTYPE ((Widget o, Widget r, Widget n));

static void get_font PROTOTYPE ((XmFontList fontlist, XFontStruct **font));

static void redisplay_timebar PROTOTYPE ((
	TimebarWidget	tbw,
	TbwTimeRange	ranges,
	XRectangle	*exposed,
	XRectangle	*timebar,
	GC		foreground_gc));

static void resize_timebar PROTOTYPE ((
	TimebarWidget	tbw,
	DwcTbwTimeKind	*times,
	TbwTimeRange	*ranges,
	XRectangle	*timebar,
	GC		foreground_gc));

static TbwTimeRange setup_timebar_ranges PROTOTYPE ((
	DwcTbwTimeKind	*kinds,
	Dimension	height));

static void setup_timebar PROTOTYPE ((
	DwcTbwTimeKind	*kinds,
	Cardinal	start,
	Cardinal	finish,
	DwcTbwTimeKind	kind));


#define forGCmask       GCForeground | GCBackground | GCLineWidth | GCDashList
#define formapGCmask    GCForeground | GCBackground | GCTile | GCFillStyle | GCLineWidth | GCDashList

static int  default_v_margin = 1;
static int  default_h_margin = 1;

extern void _XmBackgroundColorDefault();
extern void _XmForegroundColorDefault();

static XmPartResource resources [] =
{
   {DwcTbwNverticalMargin,	    DwcTbwCVerticalMargin,   XtRDimension,
    sizeof (Dimension),		    XmPartOffset (Timebar, v_margin),
    XtRDimension,		    (caddr_t) &default_v_margin},

   {DwcTbwNhorizontalMargin,	    DwcTbwCHorizontalMargin, XtRDimension,
    sizeof (Dimension),		    XmPartOffset (Timebar, h_margin),
    XtRDimension,		    (caddr_t) &default_h_margin},

   {DwcTbwNfontList,		    XmCFontList,	    XmRFontList,
    sizeof(XmFontList),		    XmPartOffset(Timebar, fontlist),
    XtRString,			    DXmDefaultFont}
};


/*
**  Translation table for Timebar
*/

static char translations [] = "";

/*
**  Action table - 'action' routines that may be called via translation table
*/

static XtActionsRec actions [] = {
   {NULL,		    NULL}
};

/*
** Static initialization of the widget class record, must do each field
*/         

TimebarClassRec timebarWidgetClassRec =
{
    /*
    ** Core Class record
    */
    {
	(WidgetClass) &xmPrimitiveClassRec, /* superclass ptr		    */
	"Timebar",			/* class_name			    */
	sizeof (TimebarWidgetRec),	/* size of timebar widget instance */
	ClassInitialize,		/* class init proc		    */
	NULL,				/* Class Part Initialize	    */
	FALSE,				/* Class is not initialised	    */
	(XtInitProc) Initialize,	/* Widget init proc		    */
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
	(XtExposeProc) Redisplay,	/* class expose proc		    */
	(XtSetValuesFunc) SetValues,	/* class set_value proc		    */
	NULL,				/* set values hook		    */
	XtInheritSetValuesAlmost,	/* set values almost		    */
	NULL,				/* get values hook		    */
	NULL,				/* class accept focus proc	    */
	XtVersion,			/* version			    */
	NULL,				/* Callback Offsets		    */
	translations,			/* Tm_table			    */
	NULL,				/* disp accelerators  */	
	NULL				/* extension          */ 
    },

    /*
    **  Primitive class record
    */
    {
	(XtWidgetProc) _XtInherit,	/* XtWidgetProc border_highlight */
	(XtWidgetProc) _XtInherit,	/* XtWidgetProc border_unhighlight */
	NULL,				/* XtTranslations translations */
	(XtActionProc) _XtInherit,	/* XtActionProc arm_and_activate */
	NULL,				/* XmGetValueResource * get_resources */
	(int)0,				/* int num_get_resources */
	NULL				/* caddr_t extension */
    },

    /*
    ** Timebar Class record
    */
    {
	NULL,
	0				/* just a dummy field		    */
    }
};

/*
**  static widget class pointer
*/

WidgetClass timebarWidgetClass = (WidgetClass) &timebarWidgetClassRec;

static void
ClassInitialize
#ifdef _DWC_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{

#ifdef DEBUG
    printf ("TimebarWidget -- ClassInitialize Called\n");
#endif

    XmResolvePartOffsets (timebarWidgetClass,
		       &timebarWidgetClassRec.timebar_class.timebaroffsets);

}

static void
Initialize
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
    int		    font_width;
    TimebarWidget   tbw = (TimebarWidget) new;
    XmOffsetPtr	    o = TbwOffsetPtr (tbw);
    XGCValues	    values;
    int		    f_bit;
    XFontStruct	    *font;

    f_bit = 0;

    get_font(TbwFontList(tbw,o), &font);
    values.font = font->fid;
    f_bit = GCFont;

    font_width = (font->max_bounds.rbearing -
		  font->min_bounds.lbearing) /4;

    XtWidth (tbw) = MAX (font_width * XtWidth (tbw),
			    (int)TbwHMargin(tbw, o) * 3);

    setup_timebar
	((DwcTbwTimeKind *)&(TbwTimes (tbw, o)), 0, 24 * 60, DwcTbwNonWork);
    TbwRanges (tbw, o) = NULL;

    /* copied from dwtcommon.c */
    TbwGrayPixmap(tbw, o) = MISCXtGrayPixmap(XtScreen(tbw));

    /* copied from dwtcommon.c */
    /* set line width to 0 for server speed */

    values.line_width = 0;
    values.foreground = TbwForegroundPixel(tbw,o);
    values.background = TbwBackgroundPixel(tbw,o);
    values.tile       = TbwGrayPixmap(tbw,o);
    values.fill_style = FillTiled;
    values.dashes     = 1;

    if (XtSensitive(tbw))
       TbwForegroundGC(tbw,o) = XtGetGC
	    ((Widget) tbw, (forGCmask | f_bit), &values);
    else
       TbwForegroundGC(tbw,o) = XtGetGC
	    ((Widget) tbw, (formapGCmask | f_bit), &values);    


}

static void
Realize
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
    printf ("TimebarWidget -- Realize Called\n");
#endif

    XtCreateWindow
	(w, InputOutput, CopyFromParent, *window_mask, window_attributes);

    TBWSetTilesAndStipples(w);
 
}

static void
Redisplay
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
    int		    width;
    int		    height;
    XRectangle	    expose_area;
    XRectangle	    timebar;
    TimebarWidget   tbw = (TimebarWidget) w;
    XmOffsetPtr    o = TbwOffsetPtr (tbw);
    
    width  = (int) XtWidth  (w) - (int) (TbwHMargin (tbw, o) * 2);
    height = (int) XtHeight (w) - (int) (TbwVMargin (tbw, o) * 2);

    if ((width <= 0) || (height <= 0)) {
	return;
    }
    
    expose_area.x      = ee->x;
    expose_area.y      = ee->y;
    expose_area.width  = ee->width;
    expose_area.height = ee->height;

    timebar.x	       = (int)TbwHMargin (tbw, o);
    timebar.y	       = (int)TbwVMargin (tbw, o);
    timebar.width      = width;
    timebar.height     = height;

    redisplay_timebar(tbw, TbwRanges(tbw, o),
	&expose_area, &timebar,TbwForegroundGC(tbw,o));

}

void TBWRedisplayGidget
#ifdef _DWC_PROTO_
	(
	Widget			w,
	DwcTbgTimebarGidget	tbg,
	XRectangle		*expose_area)
#else	/* no prototypes */
	(w, tbg, expose_area)
	Widget			w;
	DwcTbgTimebarGidget	tbg;
	XRectangle		*expose_area;
#endif	/* prototype */
{
    TimebarWidget   tbw = (TimebarWidget) w;

    redisplay_timebar
	(tbw, tbg->ranges, expose_area, &(tbg->timebar), tbg->foreground_gc);

}

static void
Resize
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    int		    width;
    int		    height;
    XRectangle	    timebar;
    TimebarWidget   tbw = (TimebarWidget) w;
    XmOffsetPtr    o = TbwOffsetPtr (tbw);

    width  = (int) XtWidth  (w) - (int) (TbwHMargin (tbw, o) * 2);
    height = (int) XtHeight (w) - (int) (TbwVMargin (tbw, o) * 2);

    if ((width <= 0) || (height <= 0)) {
	return;
    }

    timebar.x	   = (int)TbwHMargin (tbw, o);
    timebar.y	   = (int)TbwVMargin (tbw, o);
    timebar.width  = width;
    timebar.height = height;

    resize_timebar
    (
	tbw,
	(DwcTbwTimeKind *)&(TbwTimes (tbw, o)),
	&(TbwRanges (tbw, o)),
	&timebar,
	TbwForegroundGC(tbw, o)
    );

}

void TBWResizeGidget
#ifdef _DWC_PROTO_
	(
	Widget			w,
	DwcTbgTimebarGidget	tbg,
	Position		x,
	Position		y,
	Dimension		width,
	Dimension		height)
#else	/* no prototypes */
	(w, tbg, x, y, width, height)
	Widget			w;
	DwcTbgTimebarGidget	tbg;
	Position		x;
	Position		y;
	Dimension		width;
	Dimension		height;
#endif	/* prototype */
{
    TimebarWidget   tbw = (TimebarWidget) w;

    tbg->timebar.x = x;
    tbg->timebar.y = y;
    tbg->timebar.width  = width;
    tbg->timebar.height = height;

    resize_timebar
	(tbw, tbg->times, &(tbg->ranges), &(tbg->timebar), tbg->foreground_gc);

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
    Boolean	    redisplay;
    int		    font_width;
    TimebarWidget   old = (TimebarWidget) o;
    TimebarWidget   new = (TimebarWidget) n;
    XmOffsetPtr	    oo = TbwOffsetPtr (old);
    XmOffsetPtr	    no = TbwOffsetPtr (new);
    XGCValues	    values;
    XFontStruct	    *font;
    int		    f_bit;

    f_bit = 0;

    redisplay = (TbwHMargin (old, oo) != TbwHMargin (new, no));

    MISCGetFontFromFontlist(TbwFontList(new,no),&font);
    values.font = font->fid;
    f_bit = GCFont;

    if ((TbwFontList(old, oo) != TbwFontList(new,no)) ||
        (XtWidth(old) != XtWidth(new)))
    {
	font_width = (font->max_bounds.rbearing -
		      font->min_bounds.lbearing) /4;

	XtWidth(new) = MAX(font_width * XtWidth(new),
			    (int)TbwHMargin(new,no) * 3);
	redisplay = TRUE;
    }
    
    if (TbwVMargin (old, oo) != TbwVMargin (new, no))
    {
	if (TbwRanges (new, no) != NULL) {
	    XtFree ((char *)TbwRanges (new, no));
	}

	TbwRanges (new, no) = setup_timebar_ranges
	(
	    (DwcTbwTimeKind *)&(TbwTimes (new, no)), 
	    XtHeight (new) - (int)(TbwVMargin (new, no) * 2)
	);
	redisplay = TRUE;
    }
    
    if ((TbwForegroundPixel(old,oo) != TbwForegroundPixel(new,no)) ||
	(TbwBackgroundPixel(old,oo) != TbwBackgroundPixel(new,no)) ||
	(TbwGrayPixmap(old,oo) != TbwGrayPixmap(new,no)))
    {
	/* copied from dwtcommon.c */
	/* set line width to 0 for server speed */
	values.line_width = 0;
	values.foreground = TbwForegroundPixel(new,no);
	values.background = TbwBackgroundPixel(new,no);
	values.tile       = TbwGrayPixmap(new,no);
	values.fill_style = FillTiled;
	values.dashes     = 1;

	/* Get rid of the old one */
	XtReleaseGC ((Widget) new, TbwForegroundGC(new,no));
	if (XtSensitive(new))
	    TbwForegroundGC(new,no) = XtGetGC
		((Widget) new, (forGCmask | f_bit), &values);
	else
	    TbwForegroundGC(new,no) = XtGetGC
		((Widget) new, (formapGCmask | f_bit), &values);    

	redisplay = TRUE;	    	
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
    TimebarWidget	tbw = (TimebarWidget) w;
    XmOffsetPtr        o   = TbwOffsetPtr (tbw);

    if (TbwRanges (tbw, o) != NULL)
    {
	XtFree ((char *) TbwRanges (tbw, o));
    }
    if (TbwForegroundGC(w, o) != NULL)
    {
	XtReleaseGC(w, TbwForegroundGC(w,o));
    }

}

void TBWDestroyGidget
#ifdef _DWC_PROTO_
	(
	Widget			w,
	DwcTbgTimebarGidget	tbg)
#else	/* no prototypes */
	(w, tbg)
	Widget			w;
	DwcTbgTimebarGidget	tbg;
#endif	/* prototype */
{

    if (tbg->ranges != NULL)
    {
	XtFree ((char *) tbg->ranges);
    }

    if (tbg->foreground_gc != NULL)
    {
	XFreeGC(XtDisplay(w), tbg->foreground_gc);
    }

    XtFree ((char *) tbg);

}

Widget TBWTimebarCreate
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
    printf ("TimebarWidget -- DwcTimebarCreate Called\n");
#endif

    return (XtCreateWidget (name, timebarWidgetClass, parent,
			    arglist, argcount));

}

void
TBWSetTilesAndStipples
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    TimebarWidget	tbw = (TimebarWidget) w;
    XmOffsetPtr		o   = TbwOffsetPtr (tbw);
    static char		bitmap_data [4] = {001, 002, 004, 010};
    int			bm_w = 4;
    int			bm_h = 4;
    Pixmap		pixmap;


    XSetTile (XtDisplay(w), TbwForegroundGC(tbw,o), TbwGrayPixmap(tbw,o));

    pixmap = XCreateBitmapFromData
    (
	XtDisplay(w),
	RootWindowOfScreen(XtScreen(w)),
	bitmap_data,
	bm_w,
	bm_h
    );

    XSetStipple (XtDisplay(w), TbwForegroundGC(tbw,o), pixmap);

    XFreePixmap (XtDisplay(w), pixmap);

}

void TbwSetTilesAndStipplesGidget
#ifdef _DWC_PROTO_
	(
	Widget			w,
	DwcTbgTimebarGidget	tbg)
#else	/* no prototypes */
	(w, tbg)
	Widget			w;
	DwcTbgTimebarGidget	tbg;
#endif	/* prototype */
    {
    static char	    bitmap_data [4] = {001, 002, 004, 010};
    int		    bm_w = 4;
    int		    bm_h = 4;
    Pixmap	    pixmap;


    XSetTile (XtDisplay(w), tbg->foreground_gc, tbg->gray_pixmap);

    pixmap = XCreateBitmapFromData
    (
	XtDisplay(w),
	RootWindowOfScreen(XtScreen(w)),
	bitmap_data,
	bm_w,
	bm_h
    );

    XSetStipple (XtDisplay(w), tbg->foreground_gc, pixmap);

    XFreePixmap (XtDisplay(w), pixmap);

}

DwcTbgTimebarGidget TBWCreateGidget
#ifdef _DWC_PROTO_
	(
	Widget		w,
	Position	x,
	Position	y,
	Dimension	width,
	Dimension	height,
	Pixel		foreground)
#else	/* no prototypes */
	(w, x, y, width, height, foreground)
	Widget		w;
	Position	x;
	Position	y;
	Dimension	width;
	Dimension	height;
	Pixel		foreground;
#endif	/* prototype */
    {
    DwcTbgTimebarGidget	tbg;
    Cardinal	    i;
    XGCValues	    values;
    Window	    root;


    /*	  
    **  NOTE: The widget being passed into us is a dayslotswidget
    */	  

    tbg = (DwcTbgTimebarGidget) XtMalloc (sizeof (DwcTbgTimebarGidgetRec));
    tbg->timebar.x = x;
    tbg->timebar.y = y;
    tbg->timebar.width  = width;
    tbg->timebar.height = height;
    tbg->ranges = NULL;
    tbg->gray_pixmap = MISCXtGrayPixmap(XtScreen(w));

    values.foreground = foreground;
    values.background = XtBackground(w);
    values.tile = tbg->gray_pixmap;
    values.line_width = 0;
    values.fill_style = FillTiled;
    values.dashes     = 1;

    root = RootWindowOfScreen (XtScreen(w));
    if (XtSensitive(w))
    {
	tbg->foreground_gc = XCreateGC
	    (XtDisplay(w), root, forGCmask, &values);
    }
    else
    {
	tbg->foreground_gc = XCreateGC
	    (XtDisplay(w), root, formapGCmask, &values);
    }

    for (i = 0;  i < 24 * 60;  i++)
    {
	tbg->times [i] = DwcTbwNonWork;
    }

    TbwSetTilesAndStipplesGidget (w, tbg);

    return (tbg);

}

static void redisplay_timebar
#ifdef _DWC_PROTO_
	(
	TimebarWidget	tbw,
	TbwTimeRange	ranges,
	XRectangle	*exposed,
	XRectangle	*timebar,
	GC		foreground_gc)
#else	/* no prototypes */
	(tbw, ranges, exposed, timebar, foreground_gc)
	TimebarWidget	tbw;
	TbwTimeRange	ranges;
	XRectangle	*exposed;
	XRectangle	*timebar;
	GC		foreground_gc;
#endif	/* prototype */
    {
    Position	    y1;
    Position	    y2;
    Position	    area_x;
    Position	    area_y;
    Dimension	    area_w;
    Dimension	    area_h;
    Cardinal	    range;
    GC		    gc;
    Boolean	    gc_changed = FALSE;
    XmOffsetPtr    o = TbwOffsetPtr (tbw);


    if ((ranges == NULL) ||
	(timebar->width == 0) ||
	(timebar->height == 0) ||
        (! XtIsRealized ((Widget)tbw)))
    {
	return;
    }

    if (exposed == NULL)
    {
	y1 = 0;
	y2 = (int) timebar->height - 1;
    }
    else
    {
	if ((exposed->x >= timebar->x + (int) timebar->width)  ||
	    (timebar->x >  exposed->x + (int) exposed->width)  ||
	    (exposed->y >= timebar->y + (int) timebar->height) ||
	    (timebar->y >  exposed->y + (int) exposed->height))
	{
	    return;
	}

	y1 = MAX (0, exposed->y - timebar->y);
	y2 = exposed->y + (int) exposed->height - timebar->y - 1;
	y2 = MIN (y2, timebar->height - 1);
    }

    area_x = timebar->x;
    area_w = timebar->width;

    range = 0;
    while (TRUE)
    {
	if ((y2 >= ranges [range].y1) && 
	    (y1 <= ranges [range].y2))
	{

	    area_y = timebar->y + ranges [range].y1;
	    area_h = ranges [range].y2 - ranges [range].y1 + 1;

	    if (ranges [range].kind == DwcTbwWork)
	    {
		if (exposed == NULL)
		{
		    XClearArea
		    (
			XtDisplay (tbw),
			XtWindow (tbw), 
			area_x,
			area_y,
			area_w,
			area_h,
			FALSE
		    );
		}
	    }
	    else
	    {
		if (ranges [range].kind == DwcTbwNonWork)
		{
		    XSetFillStyle
			(XtDisplay (tbw), foreground_gc, FillOpaqueStippled);
		}
		else
		{
		    XSetFillStyle
			(XtDisplay (tbw), foreground_gc, FillTiled);
		}
		gc_changed = TRUE;
		XFillRectangle
		(
		    XtDisplay (tbw),
		    XtWindow (tbw),
		    foreground_gc,
		    area_x,
		    area_y,
		    area_w,
		    area_h
		);
	    }
	}

        if (ranges [range].y2 == timebar->height - 1)
	{
	    break;
	}
	range++;
    }

    if (gc_changed)
    {
	XSetFillStyle (XtDisplay (tbw), foreground_gc, FillSolid);
    }

}

static void resize_timebar
#ifdef _DWC_PROTO_
	(
	TimebarWidget	tbw,
	DwcTbwTimeKind	*times,
	TbwTimeRange	*ranges,
	XRectangle	*timebar,
	GC		foreground_gc)
#else	/* no prototypes */
	(tbw, times, ranges, timebar, foreground_gc)
	TimebarWidget	tbw;
	DwcTbwTimeKind	*times;
	TbwTimeRange	*ranges;
	XRectangle	*timebar;
	GC		foreground_gc;
#endif	/* prototype */
    {

    if (*ranges != NULL)
    {
	XtFree ((char *) *ranges);
    }

    *ranges = setup_timebar_ranges (times, timebar->height);
    redisplay_timebar (tbw, *ranges, NULL, timebar, foreground_gc);

}

static TbwTimeRange setup_timebar_ranges
#ifdef _DWC_PROTO_
	(
	DwcTbwTimeKind	*kinds,
	Dimension	height)
#else	/* no prototypes */
	(kinds, height)
	DwcTbwTimeKind	*kinds;
	Dimension	height;
#endif	/* prototype */
    {
    TbwTimeRange    ranges;
    DwcTbwTimeKind  kind;
    Cardinal	    time;
    Cardinal	    range;
    Position	    y1;
    Position	    y2;
    Position	    new_y1;


    ranges = NULL;
    range  = 0;
    kind   = kinds [0];
    time   = 1;

    y1 = 0;
    while (time < (24 * 60)) {
	if (kinds [time] != kind) {
	    y2 = (time * height) / (24 * 60);
	    new_y1 = y2;

	    if (kinds [time] > kind) {
		y2--;
	    } else {
		new_y1++;
	    }

	    if (y1 <= y2) {
		ranges = (TbwTimeRange) XtRealloc
		    ((char *) ranges, (sizeof (TbwTimeRangeRec) * (range + 1)));
		ranges [range].y1 = y1;
		ranges [range].y2 = y2;
		ranges [range].kind = kind;
		kind = kinds [time];
		range++;
		y1 = new_y1;
	    }

	}

	time++;
    }

    if (y1 <= (height - 1)) {
	ranges = (TbwTimeRange) XtRealloc
	    ((char *) ranges, (sizeof (TbwTimeRangeRec) * (range + 1)));
	ranges [range].y1 = y1;
	ranges [range].y2 = height - 1;
	ranges [range].kind = kind;
    }
    
    return (ranges);
    
}

static void setup_timebar
#ifdef _DWC_PROTO_
	(
	DwcTbwTimeKind	*kinds,
	Cardinal	start,
	Cardinal	finish,
	DwcTbwTimeKind	kind)
#else	/* no prototypes */
	(kinds, start, finish, kind)
	DwcTbwTimeKind	*kinds;
	Cardinal	start;
	Cardinal	finish;
	DwcTbwTimeKind	kind;
#endif	/* prototype */
    {
    Cardinal		i;

    i = start;
    while (i != finish)
	{
        if (i == (24 * 60))
            /*	  
	    **  We've gotten to the end of the day
	    */	  
            {
            if ((start == 0) || (finish == 0))
                /*	  
		**  If we started at midnight then we've done the whole day. If
		**  we're to end at midnight then we're there so end
		*/	  
                {
		break;
		}
            /*	  
	    **  We're at the end of the day, but finish must be in the am since
	    **	we haven't reached it yet, so start again at the beginning of
	    **	the day
	    */	  
            i = 0;
	    }
        kinds [i] = kind;
	i++;
	}

}

void
TBWSetupTimebar
#ifdef _DWC_PROTO_
	(
	Widget		w,
	Cardinal	start,
	Cardinal	finish,
	DwcTbwTimeKind	kind)
#else	/* no prototypes */
	(w, start, finish, kind)
	Widget		w;
	Cardinal	start;
	Cardinal	finish;
	DwcTbwTimeKind	kind;
#endif	/* prototype */
{
    TimebarWidget	tbw = (TimebarWidget) w;
    XmOffsetPtr	o = TbwOffsetPtr (tbw);

    setup_timebar ((DwcTbwTimeKind *)&(TbwTimes (tbw, o)), start, finish, kind);

}

void
TBWSetupGidget
#ifdef _DWC_PROTO_
	(
	DwcTbgTimebarGidget	tbg,
	Cardinal		start,
	Cardinal		finish,
	DwcTbwTimeKind		kind)
#else	/* no prototypes */
	(tbg, start, finish, kind)
	DwcTbgTimebarGidget	tbg;
	Cardinal		start;
	Cardinal		finish;
	DwcTbwTimeKind		kind;
#endif	/* prototype */
{

    setup_timebar(tbg->times, start, finish, kind);

}

void
TBWUpdateTimebar
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    int			width;
    int			height;
    XRectangle		timebar;
    TimebarWidget	tbw = (TimebarWidget) w;
    XmOffsetPtr	o = TbwOffsetPtr (tbw);

    width  = (int) XtWidth  (w) - (int) (TbwHMargin (tbw, o) * 2);
    height = (int) XtHeight (w) - (int) (TbwVMargin (tbw, o) * 2);

    if ((width <= 0) || (height <= 0)) {
	return;
    }
    
    timebar.x	   = (int)TbwHMargin (tbw, o);
    timebar.y	   = (int)TbwVMargin (tbw, o);
    timebar.width  = width;
    timebar.height = height;

    resize_timebar
    (
	tbw,
	(DwcTbwTimeKind *)&(TbwTimes (tbw, o)),
	&(TbwRanges (tbw, o)),
	&timebar,
	TbwForegroundGC(tbw,o)
    );

}
void TBWUpdateGidget
#ifdef _DWC_PROTO_
	(
	Widget			w,
	DwcTbgTimebarGidget	tbg)
#else	/* no prototypes */
	(w, tbg)
	Widget			w;
	DwcTbgTimebarGidget	tbg;
#endif	/* prototype */
    {
    TimebarWidget	tbw = (TimebarWidget) w;
    XmOffsetPtr	o = TbwOffsetPtr (tbw);

    resize_timebar
	(tbw, tbg->times, &(tbg->ranges), &(tbg->timebar), tbg->foreground_gc);

}

static void get_font
#ifdef _DWC_PROTO_
	(
	XmFontList	fontlist,
	XFontStruct	**font)
#else	/* no prototypes */
	(fontlist, font)
	XmFontList	fontlist;
	XFontStruct	**font;
#endif	/* prototype */
    {
    XmFontContext	fontlist_context;
    XmStringCharSet	charset;

    if (!XmFontListInitFontContext(&fontlist_context, fontlist))
	{
	printf("%Unable to initialize font context\n");
	DWC$UI_Catchall(DWC$UI_NOFONTINIT, 0,0);
	}

    if (!XmFontListGetNextFont(fontlist_context,
			    &charset,
			    font))
	{
	printf("%Unable to get font\n");
	DWC$UI_Catchall(DWC$UI_NOFONT, 0,0);
	}
    XtFree(charset);

    XmFontListFreeFontContext(fontlist_context);

}

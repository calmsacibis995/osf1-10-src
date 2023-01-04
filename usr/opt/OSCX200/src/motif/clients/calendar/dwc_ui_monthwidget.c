/* dwc_ui_monthwidget.c */
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
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Marios Cleovoulou, November-1987
**
**  ABSTRACT:
**
**	This module implements the "widgetness" of the month widget.
**
**--
*/

#include "dwc_compat.h"

#include <time.h>

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/XmP.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_monthwidgetp.h"
#include "dwc_ui_dateformat.h"
#include "dwc_ui_misc.h"		/* for MISCGetFontFromFontlist */
#include "dwc_ui_datefunctions.h"


/*
**  Structure for yer basic box.  Hopefully fairly obvious.
*/

typedef struct
{
    int		x_pos;
    int		y_pos;
    int	        width;
    int		height;
} box;

/*
** Macros for implementing bounding box checks.
*/
#define BOX_CHECK(box, l, r, t, b) \
    (!( \
	(box.x_pos > r) || \
	(box.y_pos < t) || \
	((box.y_pos - box.height + 1) > b) || \
	((box.x_pos + box.width  - 1) < l) \
    ))

#define BOX_INIT(box, wid, rtol) \
    if (rtol) box.x_pos = wid - box.width; \
    else box.x_pos = 0;

#define BOX_INCR(box, rtol) \
    if (rtol) box.x_pos -= box.width; \
    else box.x_pos += box.width;

/*
** How to check for right to left.
*/
#ifdef DEC_MOTIF_EXTENSION

#define LAYOUT_DIRECTION(w) \
(((XmPrimitiveWidget)w)->primitive.dxm_layout_direction == \
(int)DXmLAYOUT_LEFT_DOWN)

#else

#define LAYOUT_DIRECTION(w) \
(((XmManagerWidget)XtParent(w))->manager.string_direction == \
(int)XmSTRING_DIRECTION_R_TO_L)

#endif

static void ClassInitialize PROTOTYPE ((void));

static void Destroy PROTOTYPE ((Widget w));

static void Initialize PROTOTYPE ((Widget request, Widget new));

static void Realize PROTOTYPE ((
	Widget			w,
	XtValueMask		*window_mask,
	XSetWindowAttributes	*window_attributes));

static void Redisplay PROTOTYPE ((Widget w, XExposeEvent *ee));

static void Resize PROTOTYPE ((Widget w));

static Boolean SetValues PROTOTYPE ((Widget o, Widget r, Widget n));

void static center_xmstring_with_style PROTOTYPE ((
	MonthWidget	w,
	XmString	string,
	box		*text_box,
	XmFontList	fontlist,
	GC		gc_tns,
	unsigned char	style,
	GC		gc_invert,
	Boolean		invert));

static void create_entry_gcs PROTOTYPE ((MonthWidget mw));

static void create_plain_gcs PROTOTYPE ((MonthWidget mw));

static void create_special_gcs PROTOTYPE ((MonthWidget mw));

static void destroy_entry_gcs PROTOTYPE ((MonthWidget mw));

static void destroy_plain_gcs PROTOTYPE ((MonthWidget mw));

static void destroy_special_gcs PROTOTYPE ((MonthWidget mw));

static void display_day PROTOTYPE ((
	Cardinal		today,
	MiMonthInfoBlock	*month,
	box			*day_box,
	MonthWidget		mw));

static void get_xmstring_extents PROTOTYPE ((
	XmString	string,
	XmFontList	fontlist,
	int		*width,
	int		*height));

static void toggle_select PROTOTYPE ((CalendarBlock *cb));

static void toggle_month_select PROTOTYPE ((
	MonthWidget	mw,
	MwTypeSelected	st,
	Cardinal	day,
	Cardinal	week,
	Cardinal	month,
	Cardinal	year));

#if !defined(AUD)
static
#endif
void ACTION_MW_HELP PROTOTYPE ((
	MonthWidget	mw,
	XEvent		*event,
	String		*params,
	Cardinal	num_params));

static void determine_selection PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	MwTypeSelected	*s_type,
	Cardinal	*day,
	Cardinal	*week,
	Cardinal	*month,
	Cardinal	*year));

#if !defined(AUD)
static
#endif
void ACTION_MW_CLICK PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_DBLCLICK PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_PSEUDODBL PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_HOME PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_UP PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_DOWN PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_LEFT PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_RIGHT PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

static void mw_do_left PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

static void mw_do_right PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_FOCUSIN PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

#if !defined(AUD)
static
#endif
void ACTION_MW_FOCUSOUT PROTOTYPE ((
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params));

static void setup_month PROTOTYPE ((MonthWidget mw));

static void update_month PROTOTYPE ((
	MonthWidget	mw,
	int		top,
	int		bottom,
	int		left,
	int		right));

static void mw_create_header_strings PROTOTYPE ((MonthWidget mw));

static void mw_free_header_strings PROTOTYPE ((MonthWidget mw));

static void mw_get_header_extents PROTOTYPE ((MonthWidget mw));

static void mw_create_dayname_strings PROTOTYPE ((MonthWidget mw));

static void mw_free_dayname_strings PROTOTYPE ((MonthWidget mw));

static XtTranslations traversal_translations_parsed;
static char traversal_translations [] =
    "Shift ~Meta ~Alt <Key>Tab:	PrimitivePrevTabGroup()\n\
     ~Meta ~Alt <Key>Tab:	PrimitiveNextTabGroup()\n\
     <Key>osfBeginLine:	    traverse_home()\n\
     <Key>osfUp:	    traverse_up()\n\
     <Key>osfDown:	    traverse_down()\n\
     <Key>osfLeft:	    traverse_left()\n\
     <Key>osfRight:	    traverse_right()\n\
     <Key>osfActivate:	    key_selection()\n\
     <Key>osfSelect:	    key_selection()\n\
     <Key>Return:	    key_selection()\n\
     <Key>space:	    key_selection()\n\
     <FocusIn>:		    PrimitiveFocusIn() MWFocusIn()\n\
     <FocusOut>:	    PrimitiveFocusOut() MWFocusOut()";
static char translations [] =
    "None<Btn1Down>:		selection()\n\
     !Lock<Btn1Down>:		selection()\n\
     !Mod1<Btn1Down>:		selection()\n\
     !Mod2<Btn1Down>:		selection()\n\
     !Mod3<Btn1Down>:		selection()\n\
     !Mod4<Btn1Down>:		selection()\n\
     !Mod5<Btn1Down>:		selection()\n\
     !Lock Mod1<Btn1Down>:	selection()\n\
     !Lock Mod2<Btn1Down>:	selection()\n\
     !Lock Mod3<Btn1Down>:	selection()\n\
     !Lock Mod4<Btn1Down>:	selection()\n\
     !Lock Mod5<Btn1Down>:	selection()\n\
     None<Btn1Down>(2):		dc_selection()\n\
     !Lock<Btn1Down>(2):	dc_selection()\n\
     !Mod1<Btn1Down>(2):	dc_selection()\n\
     !Mod2<Btn1Down>(2):	dc_selection()\n\
     !Mod3<Btn1Down>(2):	dc_selection()\n\
     !Mod4<Btn1Down>(2):	dc_selection()\n\
     !Mod5<Btn1Down>(2):	dc_selection()\n\
     !Lock Mod1<Btn1Down>(2):	dc_selection()\n\
     !Lock Mod2<Btn1Down>(2):	dc_selection()\n\
     !Lock Mod3<Btn1Down>(2):	dc_selection()\n\
     !Lock Mod4<Btn1Down>(2):	dc_selection()\n\
     !Lock Mod5<Btn1Down>(2):	dc_selection()\n\
     @Help<Btn1Down>:		help()";

static XtActionsRec action_table [] =
{
    {"dc_selection",	    (XtActionProc) ACTION_MW_DBLCLICK},
    {"selection",	    (XtActionProc) ACTION_MW_CLICK},
    {"key_selection",	    (XtActionProc) ACTION_MW_PSEUDODBL},
    {"help",		    (XtActionProc) ACTION_MW_HELP},
    {"traverse_home",	    (XtActionProc) ACTION_MW_HOME},
    {"traverse_up",	    (XtActionProc) ACTION_MW_UP},
    {"traverse_down",	    (XtActionProc) ACTION_MW_DOWN},
    {"traverse_left",	    (XtActionProc) ACTION_MW_LEFT},
    {"traverse_right",	    (XtActionProc) ACTION_MW_RIGHT},
    {"MWFocusIn",	    (XtActionProc) ACTION_MW_FOCUSIN},
    {"MWFocusOut",	    (XtActionProc) ACTION_MW_FOCUSOUT},
    {"PrimitiveFocusIn",    (XtActionProc) _XmPrimitiveFocusIn},
    {"PrimitiveFocusOut",   (XtActionProc) _XmPrimitiveFocusOut},
    {NULL,		    NULL}
};

static int default_border	= 1;
static int default_day		= 0;
static int default_month	= 0;
static int default_year		= 0;
static int default_mode		= DwcMwModeDefault;
static int default_work_days	= 076;
static int default_style_mask	= 0377;
static int default_week_start	= 1;
static int default_week_depth	= 6;
static int default_show_weeks   = 1;
static int default_week_no_day  = 1;
static int default_week_no_month= 1;

extern void _XmForegroundColorDefault();
extern void _XmSelectColorDefault();

static XtResource resources [] =
{
   {DwcMwNentryForeground,	XmCHighlightColor,	XmRPixel,
    sizeof(Pixel),		XmPartOffset(Month, entry_foreground),
    XmRCallProc,		(caddr_t) _XmForegroundColorDefault},

   {DwcMwNspecialForeground,	XmCSelectColor,	        XmRPixel,
    sizeof (Pixel),	        XmPartOffset(Month, special_foreground),
    XmRCallProc,		(caddr_t) _XmSelectColorDefault},

   {DwcMwNhelpCallback,		XtCCallback,		XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset(Month, help_callback),
    XtRCallback,	        NULL},

   {DwcMwNdcCallback,		XtCCallback,		XtRCallback,
    sizeof (XtCallbackList),	XmPartOffset(Month,dc_callback),
    XtRCallback,	        NULL},

   {DwcMwNday,			DwcMwCDay,	        XtRInt,
    sizeof (int),		XmPartOffset(Month,day),
    XtRInt,		        (caddr_t) &default_day},

   {DwcMwNmonth,			DwcMwCMonth,	        XtRInt,
    sizeof (int),		XmPartOffset(Month, month),
    XtRInt,		        (caddr_t) &default_month},

   {DwcMwNyear,			DwcMwCYear,	        XtRInt,
    sizeof (int),		XmPartOffset(Month, year),
    XtRInt,		        (caddr_t) &default_year},

   {DwcMwNcalendarBlock,	DwcMwCCalendarBlock,      XtRPointer,
    sizeof (XtPointer),		XmPartOffset(Month, cb),
    XtRPointer,			NULL},

   {DwcMwNmode,			DwcMwCMode,	        XtRInt,
    sizeof (int),		XmPartOffset(Month, mode),
    XtRInt,		        (caddr_t) &default_mode},

   {DwcMwNstyleMask,		DwcMwCStyleMask,        XtRInt,
    sizeof (char),		XmPartOffset(Month, style_mask),
    XtRInt,		        (caddr_t) &default_style_mask},

   {DwcMwNworkDays,		DwcMwCWorkDays,	        XtRInt,
    sizeof (char),		XmPartOffset(Month, work_days),
    XtRInt,		        (caddr_t) &default_work_days},

   {DwcMwNweekStart,		DwcMwCWeekStart,	        XtRInt,
    sizeof (int),		XmPartOffset(Month, first_day_of_week),
    XtRInt,		        (caddr_t) &default_week_start},

   {DwcMwNweekDepth,		DwcMwCWeekDepth,	        XtRInt,
    sizeof (int),		XmPartOffset(Month, week_depth),
    XtRInt,		        (caddr_t) &default_week_depth},

   {DwcMwNshowWeekNumbers,	DwcMwCShowWeekNumbers,    XtRInt,
    sizeof (int),		XmPartOffset(Month, show_week_numbers),
    XtRInt,		        (caddr_t) &default_show_weeks},

   {DwcMwNweekNumbersStartingDay,	DwcMwCWeekNumbersStartingDay, XtRInt,
    sizeof (int),		XmPartOffset(Month, week_numbers_starting_day),
    XtRInt,		        (caddr_t) &default_week_no_day},

   {DwcMwNweekNumbersStartingMonth, DwcMwCWeekNumbersStartingMonth, XtRInt,
    sizeof (int),		XmPartOffset(Month, week_numbers_starting_month),
    XtRInt,		        (caddr_t) &default_week_no_month},

   {DwcMwNworkDaysFontList,		XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month, font_work_days),
    XtRString,		        DXmDefaultFont},

   {DwcMwNnonWorkDaysFontList,	XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month, font_non_work_days),
    XtRString,		        DXmDefaultFont},

   {DwcMwNspecialDaysFontList,	XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month, font_special_days),
    XtRString,		        DXmDefaultFont},

   {DwcMwNworkDaysEntryFontList,	XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month, font_work_days_entry),
    XtRString,		        DXmDefaultFont},

   {DwcMwNnonWorkDaysEntryFontList,	XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month,font_non_work_days_entry),
    XtRString,		        DXmDefaultFont},

   {DwcMwNspecialDaysEntryFontList,	XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month, font_special_days_entry),
    XtRString,		        DXmDefaultFont},

   {DwcMwNdayNamesFontList,       XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month, font_day_names),
    XtRString,		        DXmDefaultFont},

   {DwcMwNmonthNameFontList,      XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month, font_month_name),
    XtRString,		        DXmDefaultFont},

   {DwcMwNweekNumbersFontList,    XmCFontList,	    XmRFontList,
    sizeof (XmFontList),	XmPartOffset(Month, font_week_numbers),
    XtRString,		        DXmDefaultFont},

   {DwcMwNmonthBeforeYear,	DwcMwCMonthBeforeYear,	    XmRBoolean,
    sizeof (Boolean),		XmPartOffset(Month, month_before_year),
    XtRImmediate,	        (caddr_t)True}

};

/*
** Static initialization of the widget class record, must do each field
*/

MonthClassRec monthWidgetClassRec =
{

    /*
    ** Core Class record
    */
    {
	(WidgetClass) &xmPrimitiveClassRec,	/* superclass ptr */
	"MonthWidget",				/* class_name */
	sizeof (MonthWidgetRec),		/* size of month widget instance    */
	ClassInitialize,			/* class init proc */
	NULL,					/* Class Part Initialize */
	FALSE,					/* Class is not initialised */
	(XtInitProc)Initialize,			/* Widget init proc */
	NULL,					/* Initialise hook */
	Realize,				/* Widget realise proc */
	action_table,				/* Class Action Table */
	XtNumber (action_table),
	resources,				/* this class's resource list */
	XtNumber (resources),			/*  "	  " resource_month */
	NULLQUARK,				/* xrm_class */
	TRUE,					/* class: compressed motion */
	FALSE,					/* class: no compress exposure*/
	TRUE,					/* class: compress enterleave */
	FALSE,					/* class: no VisibilityNotify */
	Destroy,				/* class destroy proc */
	Resize,					/* class resize proc */
	(XtExposeProc)Redisplay,		/* class expose proc */
	(XtSetValuesFunc)SetValues,		/* class set_value proc */
	NULL,					/* set values hook */
	NULL,					/* set values almost */
	NULL,					/* get values hook */
	XtInheritAcceptFocus,			/* class accept focus proc */
	XtVersion,				/* version */
	NULL,					/* Callback offsets */
	translations,				/* tm_table */
	NULL,					/* Preferred Geometry */
	NULL,					/* disp accelerators */
	NULL					/* extension */
    },

    /*	  
    **  XmPrimitive Class fields
    */	  
    {
	(XtWidgetProc)_XtInherit,	/* border_highlight   */
	(XtWidgetProc)_XtInherit,	/* border_unhighlight */
	traversal_translations,		/* translations       */
	NULL,				/* arm_and_activate   */
	NULL,				/* syn resources      */
	0,				/* num syn_resources  */
	NULL,				/* extension          */
    },

    /*
    ** Month Class record
    */
    {
	NULL,
	0				/* just a dummy field */
    }

};

/*
**  static widget class pointer
*/

WidgetClass monthWidgetClass = (WidgetClass) &monthWidgetClassRec;

Widget MWMonthCreate
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
    Widget w;

    w = XtCreateWidget (name, monthWidgetClass, parent, arglist, argcount);

    return (w);

}

static void ClassInitialize
#ifdef _DWC_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{

#ifdef DEBUG
    printf ("MonthWidget -- ClassInitialize called\n");
#endif

    XmResolvePartOffsets
	(monthWidgetClass, &monthWidgetClassRec.month_class.monthoffsets);
}

static void get_area_sizes (mw)
MonthWidget mw;
{
    int			height, theight;
    XFontStruct		*font;
    XmOffsetPtr		o = MwOffsetPtr(mw);
    XCharStruct		min_bounds;
    XCharStruct		max_bounds;

    /*
    ** Find the maximum ascent for a day.
    */
    height = MISCFontListAscent (MwFontSpecialDays(mw,o));
    height = MAX (height, MISCFontListAscent (MwFontNonWorkDays(mw,o)));
    height = MAX (height, MISCFontListAscent (MwFontWorkDays(mw,o)));
    height = MAX (height, MISCFontListAscent (MwFontSpecialDaysEntry(mw,o)));
    height = MAX (height, MISCFontListAscent (MwFontNonWorkDaysEntry(mw,o)));
    height = MAX (height, MISCFontListAscent (MwFontWorkDaysEntry(mw,o)));
    MwDayNumAscent(mw,o) = height;

    /*
    ** Find the character height and width for a month widget font units.
    */
    MwDayNameWidth(mw, o) = MISCFontListWidth (MwFontDayNames(mw, o));
    MwDayNameHeight(mw, o) = MISCFontListHeight (MwFontDayNames(mw, o));

    /*
    ** Find the height of the month name for later use in determining how to
    ** clear the month area.
    */
    MwMonthHeight(mw, o) = MISCFontListHeight (MwFontMonthName(mw, o));
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
    int		    our_width,  our_height;
    int		    par_width,  par_height;
    MonthWidget	    mw = (MonthWidget)new;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    XGCValues	    gcv;
    dtb		    date_time;
    int		    dummy;

#ifdef DEBUG
    MonthWidget	    rmw = (MonthWidget)request;
    XmOffsetPtr	    ro = MwOffsetPtr(rmw);

    printf ("MonthWidget -- Initialize called\n");

    printf
    (
        "Initialize : NEW day=%d, month=%d, year=%d\n",
        MwDay(mw,o), MwMonth(mw,o), MwYear(mw,o)
    );
    printf
    (
        "Initialize : REQUEST day=%d, month=%d, year=%d\n",
        MwDay(rmw,ro), MwMonth(rmw,ro), MwYear(rmw,ro)
    );
#endif

    /*
    ** Get some useful information from the fonts and put it in the widget
    ** instance structure.
    */
    get_area_sizes(mw);


    /*
    **  Setup default widget size if width or height not specified -- make
    **	it the same size as the parent.  
    **	If width and height is specified, assume caller means font units....
    */
    our_width  = XtWidth  (mw);
    our_height = XtHeight (mw);
    par_width  = XtWidth  (XtParent (mw));
    par_height = XtHeight (XtParent (mw));

    if (our_width <= 0)
    {
	our_width = par_width;
    }
    else
    {
	/*
	** 8 units of width is defined to be the same as the width of the
	** largest character in the font set added to the width of the
	** narrowest.
	*/
	our_width = (our_width * MwDayNameWidth(mw, o)) / 8;
    }
    XtWidth (mw) = our_width;

    if (our_height <= 0)
    {
	our_height = par_height;
    }
    else
    {
	/*
	** 8 units of height  is defined to be the same as the ascent of the
	** largest character in the fontset.
	*/
	our_height = (our_height * MwDayNameHeight(mw, o)) / 8;
    }
    XtHeight (mw) = our_height;

    /*
    **  Setup default widget position if X or Y not specified.  Centre in
    **	parent window.
    */
    if (XtX (mw) <= 0)
    {
	XtX (mw) = (par_width - our_width) / 2;
	if (XtX (mw) < 0)  XtX (mw) = 0;
    }

    if (XtY (mw) <= 0)
    {
	XtY (mw) = (par_height - our_height) / 2;
	if (XtY (mw) < 0)  XtY (mw) = 0;
    }

    /*
    **  Create graphic context....
    */
    if (((DefaultVisualOfScreen (XtScreen (mw)))->class == StaticGray) ||
        ((DefaultVisualOfScreen (XtScreen (mw)))->class == GrayScale)) {
	MwSpecialForeground(mw, o) = MwForeground(mw, o);
	MwEntryForeground(mw,o) = MwForeground(mw,o);
    }
    
    /*	  
    **  Go create GCs for everything except "entry" and "special"
    */	  
    create_plain_gcs(mw);
    
    /*	  
    **  Go create the GCs to mark "special" days
    */	  
    create_special_gcs(mw);
    
    /*
    **  Go create the GCs to mark days that have entries on them.
    */
    create_entry_gcs(mw);

/*    MwWeekNumbers(mw, o) =*/
    mw->month.week_numbers =
     (unsigned char *) XtMalloc(sizeof (unsigned char) * MwWeekDepth(mw, o));
/*    MwFiscalYears(mw, o) = */
    mw->month.fiscal_years =
     (unsigned short int *) XtMalloc (sizeof (unsigned short int) *
				      MwWeekDepth(mw, o));

    /*
    **  Set up the calendar block information.
    */

/*    if (MwCB(mw, o) == NULL) {*/
    if (mw->month.cb == NULL)
    {
	MwCB(mw, o) = MWCreateCalendarBlock (12, NULL, NULL);
    }

    MwCB(mw, o)->monthwidgets_number++;

    if (MwCB(mw, o)->monthwidgets_number == 1)
    {
	MwCB(mw, o)->monthwidgets = (Widget *) XtMalloc (sizeof (MonthWidget));
    }
    else
    {
	MwCB(mw, o)->monthwidgets = (Widget *) XtRealloc
	(
	    (char *) MwCB(mw, o)->monthwidgets,
	    (sizeof (MonthWidget) * MwCB(mw, o)->monthwidgets_number)
	);
    }

    MwCB(mw, o)->monthwidgets [MwCB(mw, o)->monthwidgets_number - 1] =
      (Widget) mw;

    MwFirstMonth(mw, o) = NULL;
    MwLastMonth(mw, o) = NULL;

    /*
    ** Fetch a bunch of strings for this month widget.
    */
    MwDayNames(mw,o) = (XmString *) XtMalloc (sizeof(XmString)*7);
    mw_create_header_strings (mw);
    mw_create_dayname_strings (mw);
    mw_get_header_extents (mw);

    date_time.month = MwMonth(mw,o);
    date_time.year = MwMonth(mw,o);
    MwWeekHead(mw,o) = DATEFORMATTimeToCS
	(dwc_k_mw_week_heading_format, &date_time);
    MwDitto(mw,o) = DATEFORMATTimeToCS (dwc_k_mw_ditto_format, &date_time);

    setup_month(mw);

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
    printf ("Realize called\n");
#endif

    XtCreateWindow (w, InputOutput, CopyFromParent, *window_mask,
		    window_attributes);

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
    MonthWidget	    mw;
    XmOffsetPtr	    o;

    mw  = (MonthWidget)w;
    o  = MwOffsetPtr(mw);
#ifdef DEBUG
    printf ("Destroy called\n");
#endif

    destroy_plain_gcs(mw);
    destroy_entry_gcs(mw);
    destroy_special_gcs(mw);

    /*
    ** Free up the various strings.
    */
    mw_free_header_strings (mw);
    mw_free_dayname_strings (mw);
    XtFree ((char *) MwDayNames(mw,o));
    XmStringFree (MwWeekHead(mw,o));
    XmStringFree (MwDitto(mw,o));

    XtRemoveAllCallbacks ((Widget) mw, DwcMwNhelpCallback);
    XtRemoveAllCallbacks ((Widget) mw, DwcMwNdcCallback);

    XtFree ((char *) MwWeekNumbers(mw, o));
    XtFree ((char *) MwFiscalYears(mw, o));

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
    XGCValues	    gcv;
    MonthWidget	    old = (MonthWidget) o;
    MonthWidget	    new = (MonthWidget) n;
    XmOffsetPtr	    oo = MwOffsetPtr(old);
    XmOffsetPtr	    no = MwOffsetPtr(new);
    Boolean	    redisplay;
    dtb		    date_time;

#ifdef DEBUG
    printf ("SetValues Called\n");
#endif

    redisplay = FALSE;

    if ((MwDay(old, oo)			!= MwDay(new, no)) ||
	(MwMonth(old, oo)		!= MwMonth(new, no)) ||
	(MwYear(old, oo)		!= MwYear(new, no)) ||
	(MwFontMonthName(old,oo)	!= MwFontMonthName(new,no)) ||
	(MwMonthName(old,no)		== NULL))
    {
	mw_free_header_strings (old);
	mw_create_header_strings (new);
	mw_get_header_extents (new);

	redisplay = TRUE;
    }

    if ((MwCB(old, oo)		          != MwCB(new, no))		     ||
	(MwMode(old, oo)	          != MwMode(new, no))	             ||
	(MwWorkDays(old, oo)	          != MwWorkDays(new, no))           ||
	(MwFirstDayOfWeek(old, oo)	  != MwFirstDayOfWeek(new, no))     ||
	(MwShowWeekNumbers(old, oo)	  != MwShowWeekNumbers(new, no))    ||
	(MwWeekDepth(old, oo)	          != MwWeekDepth(new, no))          ||
	(MwStyleMask(old, oo)	          != MwStyleMask(new, no))          ||
	(MwWeekNumStartDay(old, oo)	  != MwWeekNumStartDay(new, no))    ||
	(MwWeekNumStartMonth(old, oo)	  != MwWeekNumStartMonth(new, no)))
    {
	redisplay = TRUE;
    };
    
    if (MwWeekDepth(old, oo) != MwWeekDepth(new, no))
    {
	MwWeekNumbers(new, no) = (unsigned char *) XtRealloc
	(
	    (char *) MwWeekNumbers(new, no),
	    sizeof (unsigned char) * MwWeekDepth(new, no)
	);
	MwFiscalYears(new, no) = (unsigned short int *) XtRealloc
	(
	    (char *) MwFiscalYears(new, no),
	    sizeof (unsigned short int) * MwWeekDepth(new, no)
	);
	redisplay = TRUE;
    }

    if (((DefaultVisualOfScreen (XtScreen (new)))->class == StaticGray) ||
        ((DefaultVisualOfScreen (XtScreen (new)))->class == GrayScale))
    {
	MwSpecialForeground(new, no) = MwForeground(new, no);
	MwEntryForeground(new,no) = MwForeground(new,no);
    }

    if ((MwForeground(old, oo) != MwForeground(new, no)) ||
        (MwBackground(old, oo) != MwBackground(new, no)) ||
	(MwFontWorkDays(old, oo) != MwFontWorkDays(new,no)) ||
	(MwFontNonWorkDays(old, oo) != MwFontNonWorkDays(new, no)) ||
	(MwFontDayNames(old, oo) != MwFontDayNames(new, no)) ||
	(MwFontMonthName(old, oo) != MwFontMonthName(new, no)) ||
	(MwFontWeekNumbers(old, oo) != MwFontWeekNumbers(new, no))
	)
    {
	destroy_plain_gcs(old);
	create_plain_gcs(new);
	redisplay = TRUE;
    }

    if ((MwSpecialForeground(old, oo) != MwSpecialForeground(new, no)) ||
        (MwBackground(old, oo) != MwBackground(new, no)) ||
	(MwFontSpecialDays(old, oo) != MwFontSpecialDays(new,no)) ||
	(MwFontSpecialDaysEntry(old,oo) != MwFontSpecialDaysEntry(new,no)))
    {
	destroy_special_gcs(old);
	create_special_gcs(new);	
	redisplay = TRUE;
    }
	
    if ((MwEntryForeground(old, oo) != MwEntryForeground(new, no)) ||
        (MwBackground(old, oo) != MwBackground(new, no)) ||
	(MwFontWorkDaysEntry(old, oo) != MwFontWorkDaysEntry(new,no)) ||
	(MwFontNonWorkDaysEntry(old, oo) != MwFontNonWorkDaysEntry(new,no)))
    {
	destroy_entry_gcs(old);
	create_entry_gcs(new);
	redisplay = TRUE;
    }
    

    if (redisplay)
    {
	get_area_sizes(new);
	setup_month(new);
    }
    
    return (redisplay);

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

#ifdef DEBUG
    printf ("Resize Called\n");
#endif

}

static void Redisplay
#ifdef _DWC_PROTO_
	(
	Widget		w,
	XExposeEvent	*ee)
#else	/* no prototypes */
	(w, ee)
	Widget		w;
	XExposeEvent	*ee;
#endif	/* prototype */
    {
    MonthWidget	    mw = (MonthWidget)w;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    
#ifdef DEBUG
    printf ("Redisplay Called\n");
#endif

    update_month
	(mw, ee->y, ee->y + ee->height - 1, ee->x, ee->x + ee->width  - 1);

    /*
    **
    **  THIS SHOULD BE REPLACED BY A MACRO WHICH USES THE XmPartOffset
    **	STUFF.
    **
    */
    if (mw->primitive.have_traversal)
	(*(((XmPrimitiveWidgetClass)(mw->core.widget_class))->
	    primitive_class.border_highlight)) ((Widget) mw);

    if (MwShadowThickness(mw,o) > 0)
    {
	_XmDrawShadow
	(
	    XtDisplay(mw),
	    XtWindow(mw),
	    MwBottomShadowGC(mw,o),
	    MwTopShadowGC(mw,o),
	    MwShadowThickness(mw,o),
	    MwHighlightThickness(mw,o),
	    MwHighlightThickness(mw,o),
	    XtWidth(mw) - (MwHighlightThickness(mw,o) * 2),
	    XtHeight(mw) - (MwHighlightThickness(mw,o) * 2)
	);
    }
}

static void setup_month
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw)
#else	/* no prototypes */
	(mw)
	MonthWidget	mw;
#endif	/* prototype */
    {
    MiMonthInfoList   *mi_list;
    MiMonthInfoList   *mi_next;
    MiMonthInfoList   *old_first_month;
    MiMonthInfoList   *old_last_month;
    Cardinal	    weekday;
    Cardinal	    shift;
    int		    day;
    int		    month;
    int		    year;
    Cardinal	    week;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    
    /* this needs to be moved ***********************/
    struct tm		    *local_time;
    time_t		    the_time;

    if ((MwDay(mw, o) == 0) ||
	(MwMonth(mw, o) == 0) ||
	(MwYear(mw, o) == 0)) {
	time (&the_time);
	local_time = localtime (&the_time);

	MwDay(mw, o) = local_time->tm_mday;
	MwMonth(mw, o) = local_time->tm_mon + 1;
	MwYear(mw, o) = local_time->tm_year + 1900;
    }
	    
    /*
    **  Save pointers to old list (in any)
    */
    
    old_first_month = MwFirstMonth(mw, o);
    old_last_month  = MwLastMonth(mw, o);

    /*
    **  If we are showing the whole month then override day setting.
    **  Get info for month requested.
    */
        
    if (MwMode(mw, o) == DwcMwModeWholeMonth) {
	MwDay(mw, o) = 1;
    }
    
    mi_list = MIGetMonthInfoListItem (MwMonth(mw, o), MwYear(mw, o),
				        MwCB(mw, o)->allmonths);
    MwFirstMonth(mw, o) = mi_list;

    /*
    **  Calculate day of week for day requested and work out how many days
    **  there are to the left of that day in the month display the user wants.
    */
        
    weekday = ((mi_list->month->first_weekday + MwDay(mw, o) - 2) % 7) + 1;

    if (MwFirstDayOfWeek(mw, o) <= weekday) {
	shift = weekday - MwFirstDayOfWeek(mw, o);
    } else {
	shift = 7 - MwFirstDayOfWeek(mw, o) + weekday;
    }

    /*
    **  Now get the real first day of that week.  If it is in the month before
    **  AND we are to fill blank days, then need to point to the month before
    **  for the first day.  End up with "day" being the LAST day of the week.
    */
    
    day = MwDay(mw, o) - shift;
    if (day >= 1) {

	/*
	**  First day of week is still in current month.  Save it and save an
	**  offset saying there are no days in the week before it.
	*/
	
	MwFirstDay(mw, o) = day;
	MwLeadingBlanks(mw, o) = 0;

    } else {

	/*
	**  First day of the week is in the previous month....
	*/
	
	if (MwMode(mw, o) != DwcMwModeFillBlanks) {

	    /*
	    **  If we are not filling blank days then save the 1st of the month
	    **  as the first day to display and save number of days before it
	    **  in the week.
	    */
	    
	    MwFirstDay(mw, o) = 1;
	    MwLeadingBlanks(mw, o) = 1 - day;

	} else {

	    /*
	    **	If we are filling blank days then we need the info for the
	    **	previous month.  Get it and save as "first month" pointer, but
	    **	keep the current month pointer where it is.  Calculate first
	    **	day of the week and save and save an offset saying there are no
	    **	days in the week before it.
	    **  Note:  we keep the pointer to the original month 'cos we
	    **	don't want to "add" the month to the list twice, which is
	    **	what would happen if we moved the pointer back and fell into
	    **	the code that follows.
	    */
	    
	    MwFirstMonth(mw, o) =
	      MIAddPreviousMonthToList(mi_list, MwCB(mw, o)->allmonths);
	    MwFirstDay(mw, o) = MwFirstMonth(mw, o)->month->month_length +
				  day;
	    MwLeadingBlanks(mw, o) = 0;

	}
    }
    day = day + 6;
    
    /*
    **  Move from end-of-week to end-of-week until it is determined that the
    **  month display will be filled as per requirements.  Add new months onto
    **  the list as need be.
    */
    
    MwNamedMonth(mw, o) = NULL;
    week = 1;
    while (TRUE) {

/*
**  *************** Week number stuff needs some work for performance.
*/

	if (day <= mi_list->month->month_length) {

	    DATEFUNCWeekNumberForDate (day, 
				  mi_list->month->month_number,
				  mi_list->month->year_number,
				  MwFirstDayOfWeek(mw, o),
				  MwWeekNumStartDay(mw, o),
				  MwWeekNumStartMonth(mw, o),
				  &MwWeekNumbers(mw, o)[week - 1],
				  &MwFiscalYears(mw, o)[week - 1]);
	    /*
	    **  End of this week is still within current month.  If there is
	    **  still space then move to the end of the next week.  Otherwise
	    **  all done.
	    */
	    
	    if (week == MwWeekDepth(mw, o)) {
		break;
	    }
	    week = week + 1;
	    day  = day  + 7;

	} else {

	    /*
	    **	End of the week is in the next month.  Figure what the day at
	    **	the end of the week really is and deal with week number.
	    */

	    day = day - mi_list->month->month_length;
	    month = mi_list->month->month_number + 1;
	    if (month <= 12) {
		year  = mi_list->month->year_number;
	    } else {
		month = 1;
		year  = mi_list->month->year_number + 1;
	    }

	    DATEFUNCWeekNumberForDate (day, month, year,
				  MwFirstDayOfWeek(mw, o),
				  MwWeekNumStartDay(mw, o),
				  MwWeekNumStartMonth(mw, o),
				  &MwWeekNumbers(mw, o)[week - 1],
				  &MwFiscalYears(mw, o)[week - 1]);

	    /*
	    **	If we are only displaying a single (whole) month, then exit
	    **	now.  If the last day of the week is the 7th then fry the week
	    **	number -- didn't need it after all.
	    */

	    if (MwMode(mw, o) == DwcMwModeWholeMonth) {
		if (day == 7) {
		    MwWeekNumbers(mw, o)[week - 1] = 0;
		    break;
		}
		    
		if (week != MwWeekDepth(mw, o) ) {
		    week = week + 1;
		    MwWeekNumbers(mw, o)[week - 1] = 0;
		}
		break;
	    }

	    /*
	    **	If we are not to fill blank days then count an extra row,
	    **	unless the last day of the week is the 7th, in which case we
	    **	have filled the whole of this week with days from this month
	    **	anyway.  If there isn't space then exit.
	    */
	    
	    if ((MwMode(mw, o) != DwcMwModeFillBlanks) && (day != 7)) {
		if (week == MwWeekDepth(mw, o) ) {
		    break;
		}
		week = week + 1;
	    }

	    /*
	    **  Add the next month onto the list.
	    */
	    
	    mi_list = MIAddNextMonthToList(mi_list, MwCB(mw, o)->allmonths);

	}

	/*
	**  If we haven't filled it in already then fill in pointer to month
	**  to name for the heading.  Doing this this way at this point gets
	**  us pointing to the month that corresponds to the first last-day-
	**  of-week shown.
	*/

	if (MwNamedMonth(mw, o) == NULL) {
	    MwNamedMonth(mw, o) = mi_list;
	}

    }

/* Don't need this.....
    while (week != MwWeekDepth(mw, o) ) {
	MwWeekNumbers(mw, o)[week - 1] = 0;
	week = week + 1;
    }
*/

    /*
    **	Save last month pointer and remove old months (if any) from list.  Do
    **	the "remove"s here, after the "add"s so that the list routines don't
    **	decide any months that we had to start and still have now have become
    **	unreferenced, and hence shuffle them to the cache and back again.
    */

    MwLastMonth(mw, o) = mi_list;

    if (old_first_month != NULL) {
	mi_list = old_first_month;
	while (TRUE) {
	    mi_next = mi_list->next;
	    MIRemoveMonthInfoListItem (mi_list, MwCB(mw, o)->allmonths);
	    if (mi_list == old_last_month) {
		break;
	    }
	    mi_list = mi_next;
	}
    }

}

static void update_month
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	int		top,
	int		bottom,
	int		left,
	int		right)
#else	/* no prototypes */
	(mw, top, bottom, left, right)
	MonthWidget	mw;
	int		top;
	int		bottom;
	int		left;
	int		right;
#endif	/* prototype */
{
    box		    day_box;
    box		    save_box;
    MiMonthInfoList   *mlist;
    Cardinal	    today;
    Cardinal	    week;
    Cardinal	    last_week_number;
    char	    *week_number_text;
    Cardinal	    offset;
    dtb		    date_time;
    char	    *text;
    int		    height, width, month_width, year_width;
    Boolean	    invert;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    XFontStruct	    *font;
    XmString	    xmstring, month_xmstring, year_xmstring, separator_xmstring;
    Boolean	    mby, rtol;

    /*
    ** Get some important layout controls
    */
    mby = (MwMonthBeforeYear(mw,o) == True);
    rtol = LAYOUT_DIRECTION(mw);

    /*
    **  Calculate height of box for each day (do width later).
    */
    height = (int)XtHeight(mw) / (MwWeekDepth(mw, o) + 2);
    day_box.height = MAX(height, MwMonthHeight(mw,o));

    day_box.x_pos = 0;
    day_box.y_pos = day_box.height - 1;

    /*
    **  Display month name.
    */
    day_box.width = (int)XtWidth (mw);
    date_time.month = MwNamedMonth(mw, o)->month->month_number;
    date_time.year  = MwNamedMonth(mw, o)->month->year_number;
    if (BOX_CHECK(day_box, left, right, top, bottom))
    {
	month_xmstring = MwMonthName(mw,o);
	year_xmstring  = MwYearName(mw,o);
	separator_xmstring = MwSepString(mw,o);
	month_width = MwMonthNameWidth(mw,o);
	year_width = MwYearNameWidth(mw,o);
	width = MwSepWidth(mw,o);
	height = MwMonthNameHeight(mw,o);

	invert = (MwCB(mw, o)->select_type  == MwMonthSelected) &&
		 (MwCB(mw, o)->select_month == date_time.month) &&
		 (MwCB(mw, o)->select_year  == date_time.year);

	if (!(mby ^ rtol))
	    day_box.x_pos =
		(XtWidth (mw) - width - month_width - year_width) / 2 +
		year_width + width;
	else
	    day_box.x_pos =
		(XtWidth (mw) - width - month_width - year_width) / 2;

	day_box.width = month_width;

	if (!((day_box.x_pos > right) ||
	      (day_box.x_pos + day_box.width  - 1 < left)))
	{
	    center_xmstring_with_style
	    (
		mw,
		month_xmstring,
		&day_box,
		MwFontMonthName(mw,o),
		MwGCNormal(mw,o),
		0,
		MwGCInvert(mw,o),
		invert
	    );
	}
	
	if (!(mby ^ rtol))
	    day_box.x_pos -= width;
	else
	    day_box.x_pos = day_box.x_pos + month_width;

	day_box.width = width;

	if (! ((day_box.x_pos > right) ||
	       (day_box.x_pos + day_box.width  - 1 < left)))
	{
	    center_xmstring_with_style
	    (
		mw,
		separator_xmstring,
		&day_box,
		MwFontMonthName(mw,o),
		MwGCNormal(mw,o),
		0,
		MwGCInvert(mw,o),
		invert
	    );
	}

	invert = invert | (MwCB(mw, o)->select_type  == MwYearSelected) &&
			  (MwCB(mw, o)->select_month == date_time.month) &&
			  (MwCB(mw, o)->select_year  == date_time.year);

	if (!(mby ^ rtol))
	    day_box.x_pos -= year_width;
	else
	    day_box.x_pos = day_box.x_pos + width;

	day_box.width = year_width;
	if (! ((day_box.x_pos > right) ||
	       (day_box.x_pos + day_box.width  - 1 < left)))
	{
	    center_xmstring_with_style
	    (
		mw,
		year_xmstring,
		&day_box,
		MwFontMonthName(mw,o),
		MwGCNormal(mw,o),
		0,
		MwGCInvert(mw,o),
		invert
	    );
	}
	
    }

    /*
    **	Display day-of-week headings.  Only draw characters that need
    **	repainting.
    */
    height = (int)XtHeight (mw) / (MwWeekDepth(mw, o) + 2);
    day_box.height = height;
    day_box.y_pos = (day_box.height * 2) - 1;

    /*
    ** Only do the heading above week numbers, if we are showing week numbers.
    */
    if (MwShowWeekNumbers(mw, o) )
    {
	day_box.width = XtWidth(mw) / 8;
	BOX_INIT(day_box, XtWidth(mw), rtol);

	if (BOX_CHECK(day_box, left, right, top, bottom))
	{
	    xmstring = MwWeekHead(mw,o);
	    center_xmstring_with_style
	    (
		mw,
		xmstring,
		&day_box,
		MwFontWeekNumbers(mw,o),
		MwGCNormal(mw,o),
		0,
		NULL,
		False
	    );
	}
	BOX_INCR (day_box, rtol);
    }
    else
    {
	day_box.width = XtWidth(mw) / 7;
	BOX_INIT(day_box, XtWidth(mw), rtol);
    }

    today = MwFirstDayOfWeek(mw, o);
    do
    {
	if (BOX_CHECK(day_box, left, right, top, bottom))
	{
	    xmstring = MwDayNames(mw,o)[today-1];
	    center_xmstring_with_style
	    (
		mw,
		xmstring,
		&day_box,
		MwFontDayNames(mw,o),
		MwGCNormal(mw,o),
		0,
		NULL,
		False
	    );
	}
	BOX_INCR (day_box, rtol);

	today = (today % 7) + 1;
    }
    while (today != MwFirstDayOfWeek(mw, o));

    /*
    ** Show week numbers.
    */
    if (MwShowWeekNumbers(mw, o))
    {
	save_box = day_box;

	BOX_INIT(day_box, XtWidth(mw), rtol);
	day_box.y_pos = day_box.y_pos + day_box.height;

	week = 0;
	last_week_number = 0;
	while (week < MwWeekDepth(mw, o))
	{
	    register int    week_num;
	    week_num = MwWeekNumbers(mw,o)[week];

	    if (week_num == 0) break;

	    if (BOX_CHECK(day_box, left, right, top, bottom))
	    {
		if (week_num == last_week_number)
		{
		    xmstring = MwDitto(mw,o);
		    invert = FALSE;
		}
		else
		{
		    xmstring = DATEFORMATWeekToSharedCS (week_num, True);

		    invert =
			(MwCB(mw,o)->select_type == MwWeekSelected) &&
			(MwCB(mw,o)->select_week == week_num) &&
			(MwCB(mw,o)->select_year == MwFiscalYears(mw,o)[week]);
	        }

		center_xmstring_with_style
		(
		    mw,
		    xmstring,
		    &day_box,
		    MwFontWeekNumbers(mw,o),
		    MwGCNormal(mw,o),
		    0,
		    MwGCInvert(mw,o),
		    invert
		);

	    }
	    last_week_number = week_num;
	    week = week + 1;
	    day_box.y_pos = day_box.y_pos + day_box.height;
	}
	day_box = save_box;    
    }
    
    /*
    **  Get data for first day to be displayed and where it should be displayed
    */
    
    mlist  = MwFirstMonth(mw, o);
    today  = MwFirstDay(mw, o);
    offset = MwLeadingBlanks(mw, o);
    
    /*
    **  Calculate position of first day to be displayed
    */
    if (rtol)
    {
	day_box.x_pos = XtWidth(mw) - (day_box.width * (offset + 1));
    }
    else
    {
	day_box.x_pos = day_box.width * offset;
    }
    day_box.y_pos = day_box.y_pos + day_box.height;

    if (MwShowWeekNumbers(mw, o)) BOX_INCR (day_box, rtol);

    /*
    **  Now loop round filling the month display
    */
    week = 1;
    while (TRUE)
    {

	/*
	**  Display a day if it is within the area to update.
	*/
	if (BOX_CHECK(day_box, left, right, top, bottom))
	{
	    display_day (today, mlist->month, &day_box, mw);
	}

	/*
	**  If more days on this week then just move to next box horizontally.
	**  Otherwise advance to the start of the next row.  All done if out
	**  of rows.
	*/
	if (offset < 6)
	{
	    BOX_INCR (day_box, rtol);

	    offset = offset + 1;
	}
	else
	{
	    if (week == MwWeekDepth(mw, o)) break;

	    BOX_INIT(day_box, XtWidth(mw), rtol);

	    day_box.y_pos = day_box.y_pos + day_box.height;
	    week   = week + 1;

	    if (MwShowWeekNumbers(mw, o)) BOX_INCR (day_box, rtol);

	    offset = 0;
	}

	/*
	**  If we haven't reached the end of the month, then move to next day
	*/
	if (today < mlist->month->month_length)
	{
	    today = today + 1;
	}
	else
	{

	    /*
	    **	If we are at the end of the month and we are in "whole month"
	    **	mode then all done.
	    */
	    
	    if (MwMode(mw, o) == DwcMwModeWholeMonth)
	    {
		break;
	    }

	    /*
	    **	Otherwise if we are not in filling blank days mode then move to
	    **	the next box across, but one row down, unless we are already at
	    **	the beginning of a row, in which case just stay put.  If out
	    **	of rows, then exit.
	    */
	    
	    if ((MwMode(mw, o) != DwcMwModeFillBlanks) && (offset != 0))
	    {
		if (week == MwWeekDepth(mw, o))
		{
		    break;
		}
		day_box.y_pos = day_box.y_pos + day_box.height;
		week = week + 1;
	    }

	    /*
	    **  Setup pointers and data for next month.
	    */
	    
	    mlist = mlist->next;
	    today = 1;
	}
    }
}

static void display_day
#ifdef _DWC_PROTO_
	(
	Cardinal		today,
	MiMonthInfoBlock	*month,
	box			*day_box,
	MonthWidget		mw)
#else	/* no prototypes */
	(today, month, day_box, mw)
	Cardinal		today;
	MiMonthInfoBlock	*month;
	box			*day_box;
	MonthWidget		mw;
#endif	/* prototype */
    {
    XmFontList	    fontlist;
    GC		    gc;
    GC		    gc_invert;
    Boolean	    invert;
    Cardinal	    weekday;
    Boolean	    got_entry;
    unsigned char   markup;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    XFontStruct	    *font;
    XmString	    xmstring;
    
    got_entry = ((month->markup [today - 1] & MWMARKUP_ENTRY) != 0);
    markup    = month->markup [today - 1] & ~MWMARKUP_ENTRY;

    if (markup == MWMARKUP_DEFAULT) {
	weekday = ((month->first_weekday + today - 2) % 7) + 1;
	if (((0200 >> weekday) & MwWorkDays(mw, o)) != 0) {
	    markup = MWMARKUP_WORK_DAY;
	} else {
	    markup = MWMARKUP_NON_WORK_DAY;
	}
    }

    if (got_entry) {
	switch (markup) {
	  case MWMARKUP_WORK_DAY :
	    fontlist = MwFontWorkDaysEntry(mw, o);
	    gc   = MwGCEntry(mw, o);
	    gc_invert = MwGCInvertEntry(mw, o);
	    break;
	  case MWMARKUP_NON_WORK_DAY :
	    fontlist = MwFontNonWorkDaysEntry(mw, o);
	    gc   = MwGCEntry(mw, o);
	    gc_invert = MwGCInvertEntry(mw, o);
	    break;
	  case MWMARKUP_SPECIAL :
	    fontlist = MwFontSpecialDaysEntry(mw, o);
	    gc   = MwGCSpecialDays(mw, o);
	    gc_invert = MwGCInvertSpecial(mw, o);
	    break;
	}
    } else {
	switch (markup) {
	  case MWMARKUP_WORK_DAY :
	    fontlist = MwFontWorkDays(mw, o);
	    gc   = MwGCNormal(mw, o);
	    gc_invert = MwGCInvert(mw, o);
	    break;
	  case MWMARKUP_NON_WORK_DAY :
	    fontlist = MwFontNonWorkDays(mw, o);
	    gc   = MwGCNormal(mw, o);
	    gc_invert = MwGCInvert(mw, o);
	    break;
	  case MWMARKUP_SPECIAL :
	    fontlist = MwFontSpecialDays(mw, o);
	    gc   = MwGCSpecialDays(mw, o);
	    gc_invert = MwGCInvertSpecial(mw, o);
	    break;
	}
    }

    invert = (MwCB(mw, o)->select_type  == MwDaySelected)              &&
	     (MwCB(mw, o)->select_day   == today)		 &&
	     (MwCB(mw, o)->select_month == month->month_number) &&
	     (MwCB(mw, o)->select_year  == month->year_number);

    xmstring = DATEFORMATDayToSharedCS (today, False);
    center_xmstring_with_style
    (
	mw,
	xmstring,
	day_box,
	fontlist,
	gc,
	(month->style [today - 1] & MwStyleMask(mw, o)),
	gc_invert,
	invert
    );

}

static void date_for_position
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	Cardinal	row,
	Cardinal	column,
	Cardinal	*posday,
	MiMonthInfoList	**posmil)
#else	/* no prototypes */
	(mw, row, column, posday, posmil)
	MonthWidget	mw;
	Cardinal	row;
	Cardinal	column;
	Cardinal	*posday;
	MiMonthInfoList	**posmil;
#endif	/* prototype */
    {
    MiMonthInfoList   *mlist;
    int		    day;
    int		    week;
    int		    offset;
    XmOffsetPtr	    o = MwOffsetPtr(mw);    

    mlist  = MwFirstMonth(mw, o);
    day    = MwFirstDay(mw, o);
    offset = MwLeadingBlanks(mw, o);
    
    day = day - offset;
    week = 1;
    while (TRUE) {
	if (row == week) {
	    if (column <= offset) {
		break;
	    }

	    day = day + column - 1;
	    if (day <= mlist->month->month_length) {
		*posday = day;
		*posmil = mlist;
	        return ;
	    }

	    if (MwMode(mw, o) != DwcMwModeFillBlanks) {
	        break;
	    }

	    *posday = day - mlist->month->month_length;
	    *posmil = mlist->next;
	    return ;

	}

	if (week == MwWeekDepth(mw, o)) {
	    break;
	}

	day  = day  + 7;
	week = week + 1;
	if (day <= mlist->month->month_length) {
	    offset = 0;
	} else {
	    if (MwMode(mw, o) == DwcMwModeWholeMonth) {
		break;
	    }

	    if (MwMode(mw, o) == DwcMwModeFillBlanks) {
		day    = day - mlist->month->month_length;
		offset = 0;
	    } else {
		day    = day - mlist->month->month_length - 7;
		if (day == - 6) {
		    day = 1;
		}
		offset = 1 - day;
	    }	
	    mlist = mlist->next;

	}
    }

    *posday = 0;
    *posmil = NULL;

}

static void get_rowcol_for_day
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year,
	Cardinal	*row,
	Cardinal	*column)
#else	/* no prototypes */
	(mw, day, month, year, row, column)
	MonthWidget	mw;
	Cardinal	day;
	Cardinal	month;
	Cardinal	year;
	Cardinal	*row;
	Cardinal	*column;
#endif	/* prototype */
    {
    MiMonthInfoList   *mlist;
    int		    week;
    int		    today;
    int		    offset;
    XmOffsetPtr	    o = MwOffsetPtr(mw);

    if (((((year * 12) + month) * 31) + day) < 
        ((((MwFirstMonth(mw, o)->month->year_number * 12) +
            MwFirstMonth(mw, o)->month->year_number * 31) +
	    MwFirstDay(mw, o)))) {
	*row = 0;
	*column = 0;
	return;
    }
	
    mlist  = MwFirstMonth(mw, o);
    today  = MwFirstDay(mw, o);
    offset = MwLeadingBlanks(mw, o);
    
    week = 1;
    while (TRUE) {

	if ((year  == mlist->month->year_number)  &&
	    (month == mlist->month->month_number) &&
	    (day   == today)) {
	    *row = week;
	    *column = offset + 1;
	    return;
	}
    
	/*
	**  If more days on this week then just move to next box horizontally.
	**  Otherwise advance to the start of the next row.  All done if out
	**  of rows.
	*/
	
	if (offset < 6) {
	    offset = offset + 1;
	} else {
	    if (week == MwWeekDepth(mw, o)) {
		break;
	    }
	    week   = week + 1;
	    offset = 0;
	}

	/*
	**  If we haven't reached the end of the month, then move to next day
	*/
	
	if (today < mlist->month->month_length) {
	    today = today + 1;
	} else {

	    /*
	    **	If we are at the end of the month and we are in "whole month"
	    **	mode then all done.
	    */
	    
	    if (MwMode(mw, o) == DwcMwModeWholeMonth) {
		break;
	    }

	    /*
	    **	Otherwise if we are not in filling blank days mode then move to
	    **	the next box across, but one row down, unless we are already at
	    **	the beginning of a row, in which case just stay put.  If out
	    **	of rows, then exit.
	    */
	    
	    if ((MwMode(mw, o) != DwcMwModeFillBlanks) && (offset != 0)) {
		if (week == MwWeekDepth(mw, o)) {
		    break;
		}
		week = week + 1;
	    }

	    /*
	    **  Setup pointers and data for next month.
	    */
	    
	    mlist = mlist->next;
	    today = 1;
	}
    }

    *row = 0;
    *column = 0;

}

static void get_selection_for_xy
#ifdef _DWC_PROTO_
(
    MonthWidget		mw,
    int			x,
    int			y,
    MwTypeSelected	*s_type,
    Cardinal		*day,
    Cardinal		*week,
    Cardinal		*month,
    Cardinal		*year)
#else
(mw, x, y, s_type, day, week, month, year)
    MonthWidget		mw;
    int			x;
    int			y;
    MwTypeSelected	*s_type;
    Cardinal		*day;
    Cardinal		*week;
    Cardinal		*month;
    Cardinal		*year;
#endif
{
    box			day_box;
    MiMonthInfoList	*mlist;
    Cardinal		row;
    Cardinal		column;
    dtb			date_time;
    int			month_width, year_width, height;
    XmOffsetPtr		o = MwOffsetPtr(mw);
    XFontStruct		*font;
    XmString		month_xmstring, year_xmstring;
    Boolean		mby, rtol;

    /*
    ** Get some important layout controls
    */
    mby = (MwMonthBeforeYear(mw,o) == True);
    rtol = LAYOUT_DIRECTION(mw);

    /*
    **  Calculate width & height of box for each day.
    */
    day_box.height = XtHeight (mw) / (MwWeekDepth(mw, o) + 2);
    if (MwShowWeekNumbers(mw, o))
    {
	day_box.width = XtWidth (mw) / 8;
    }
    else
    {
	day_box.width = XtWidth (mw) / 7;
    }

    row    = y / day_box.height;

    if (row == 1)
    {
	*s_type = MwNothingSelected;
	return;    /* Select on a day name?!  */
    }

    if (row == 0)
    {
	date_time.month = MwNamedMonth(mw, o)->month->month_number;
	date_time.year  = MwNamedMonth(mw, o)->month->year_number;

	month_width = MwMonthNameWidth(mw,o);
	year_width = MwYearNameWidth(mw,o);
	height = MwMonthHeight(mw,o);

	column = (XtWidth (mw) - month_width - year_width) / 2;
	if ((x < column) ||
	    (x > column + month_width + year_width))
	{
	    *s_type = MwNothingSelected;
	    return;
	}

	if (!(mby ^ rtol))
	{
	    if (x > column + year_width)
	    {
		*s_type = MwMonthSelected;
	    }
	    else
	    {
		*s_type = MwYearSelected;
	    }
	}
	else if (x > column + month_width)
	{
	    *s_type = MwYearSelected;
	}
	else
	{
	    *s_type = MwMonthSelected;
	}

	*month  = MwNamedMonth(mw, o)->month->month_number;
	*year   = MwNamedMonth(mw, o)->month->year_number;
    }
    else
    {

	if (rtol)
	{
	    column = (XtWidth(mw) - x) / day_box.width;
	}
	else
	{
	    column = x / day_box.width;
	}

	if (MwShowWeekNumbers(mw, o) && (column == 0))
	{
	    row = row - 2;
	    if (MwWeekNumbers(mw, o)[row] == 0)
	    {
		*s_type = MwNothingSelected;
		return;
	    }
	    if (row != 0)
	    {
		if (MwWeekNumbers(mw, o)[row] == MwWeekNumbers(mw, o)[row - 1])
		{
		    *s_type = MwNothingSelected;
		    return;
		}
	    }
	    *s_type = MwWeekSelected;
	    *week   = MwWeekNumbers(mw, o)[row];
	    *year   = MwFiscalYears(mw, o)[row];
	}
	else
	{
	    if (! MwShowWeekNumbers(mw, o))
	    {
		column = column + 1;
	    }
	    date_for_position(mw, row - 1, column, day, &mlist);
	    if (*day == 0)
	    {
		*s_type = MwNothingSelected;
		return;
	    }
	    *s_type = MwDaySelected;
	    *month  = mlist->month->month_number;
	    *year   = mlist->month->year_number;
	}
    }
}

static void determine_selection
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	MwTypeSelected	*s_type,
	Cardinal	*day,
	Cardinal	*week,
	Cardinal	*month,
	Cardinal	*year)
#else	/* no prototypes */
	(mw, event, s_type, day, week, month, year)
	MonthWidget	mw;
	XButtonEvent	*event;
	MwTypeSelected	*s_type;
	Cardinal	*day;
	Cardinal	*week;
	Cardinal	*month;
	Cardinal	*year;
#endif	/* prototype */
{

    get_selection_for_xy
	(mw, event->x, event->y, s_type, day, week, month, year);

}

#if !defined(AUD)
static
#endif
void ACTION_MW_HELP
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XEvent		*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XEvent		*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
    {
    DwcMonthCallbackStruct	cbs;
    MwTypeSelected		s_type;
    Cardinal			day;
    Cardinal			week;
    Cardinal			month;
    Cardinal			year;
    XmOffsetPtr			o = MwOffsetPtr(mw);

#ifdef DEBUG
    printf ("ACTION_MW_HELP invoked\n");
#endif

    determine_selection
    (
	mw,
	(XButtonEvent *)event,
	&s_type,
	&day,
	&week,
	&month,
	&year
    );

    cbs.select_type = s_type;
    cbs.select_day = (unsigned char) day;
    cbs.select_week = (unsigned char) week;
    cbs.select_month = (unsigned char) month;
    cbs.select_year = (unsigned char) year;

    cbs.reason = (int)XmCR_HELP;

    XtCallCallbackList ((Widget) mw, MwHelpCallback(mw,o), &cbs);

}

#if !defined(AUD)
static
#endif
void ACTION_MW_CLICK
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
    {
    MwTypeSelected  s_type;
    Cardinal	    day;
    Cardinal	    week;
    Cardinal	    month;
    Cardinal	    year;
    XmOffsetPtr	    o = MwOffsetPtr(mw);

    if (mw->primitive.traversal_on)
	XmProcessTraversal ((Widget)mw, XmTRAVERSE_CURRENT);

    s_type = MwCB(mw, o)->select_type;
    day    = MwCB(mw, o)->select_day;
    week   = MwCB(mw, o)->select_week;
    month  = MwCB(mw, o)->select_month;
    year   = MwCB(mw, o)->select_year;

    determine_selection
	(mw, (XButtonEvent *)event, &s_type, &day, &week, &month, &year);

    if (s_type == MwNothingSelected) return;

    if ((MwCB(mw, o)->select_type  == s_type) &&
	(MwCB(mw, o)->select_day   == day)    &&
	(MwCB(mw, o)->select_week  == week)   &&
	(MwCB(mw, o)->select_month == month)  &&
	(MwCB(mw, o)->select_year  == year)) return;

    toggle_select(MwCB(mw, o));

    MwCB(mw, o)->select_type  = s_type;
    MwCB(mw, o)->select_day   = day;
    MwCB(mw, o)->select_week  = week;
    MwCB(mw, o)->select_month = month;
    MwCB(mw, o)->select_year  = year;

    toggle_select(MwCB(mw, o));

}

static void toggle_select
#ifdef _DWC_PROTO_
	(
	CalendarBlock	*cb)
#else	/* no prototypes */
	(cb)
	CalendarBlock	*cb;
#endif	/* prototype */
    {
    MonthWidget		mw;
    int			i;

    if (cb->select_type == MwNothingSelected) {
	return;
    }

    for (i = 0;  i < cb->monthwidgets_number;  i++) {
	if (XtIsRealized (cb->monthwidgets [i]))
	{
	    toggle_month_select
	    (
		(MonthWidget) cb->monthwidgets [i],
		cb->select_type,
		cb->select_day,
		cb->select_week,
		cb->select_month,
		cb->select_year
	    );
	}
    }

}

static void toggle_month_select
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	MwTypeSelected	st,
	Cardinal	day,
	Cardinal	week,
	Cardinal	month,
	Cardinal	year)
#else	/* no prototypes */
	(mw, st, day, week, month, year)
	MonthWidget	mw;
	MwTypeSelected	st;
	Cardinal	day;
	Cardinal	week;
	Cardinal	month;
	Cardinal	year;
#endif	/* prototype */
{
    Cardinal	    row;
    Cardinal	    column;
    box		    day_box;
    dtb		    date_time;
    int		    month_width, year_width, height, separator;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    XFontStruct	    *font;
    XmString	    month_xmstring, year_xmstring, separator_xmstring;
    Boolean	    mby, rtol;

    /*
    ** Get some important layout controls
    */
    mby = (MwMonthBeforeYear(mw,o) == True);
    rtol = LAYOUT_DIRECTION(mw);

    /*
    ** Determine the size of bounding boxes.
    */
    day_box.height = XtHeight (mw) / (MwWeekDepth(mw, o) + 2);
    if (MwShowWeekNumbers(mw, o))
    {
	day_box.width = XtWidth (mw) / 8;
    }
    else
    {
	day_box.width = XtWidth (mw) / 7;
    }

    switch (st)
    {
    case MwNothingSelected:
	return;

    case MwDaySelected:
	get_rowcol_for_day ((MonthWidget) mw, day, month, year, &row, &column);
	if (row == 0)
	{
	    return;
	}
	if (!MwShowWeekNumbers(mw, o))
	{
	    day_box.x_pos = day_box.width * (column - 1);
	}
	else
	{
	    day_box.x_pos = day_box.width * column;
        }
	day_box.y_pos = day_box.height * (row + 1);
	break;
      
    case MwWeekSelected :
	if (!MwShowWeekNumbers(mw, o))
	{
	    return;
	}
	else
	{
	    row = 0;
	    while (TRUE)
	    {
		if (MwWeekNumbers(mw, o)[row] == 0)
		{
		    return;
		}
		else
		{
		    if ((MwWeekNumbers(mw, o)[row] == week) &&
		        (MwFiscalYears(mw, o)[row] == year))
		    {
			break;
		    }
		}
		row = row + 1;
	        if (row == MwWeekDepth(mw, o))
		{
		    return;
		}
	    }
	    day_box.x_pos = 0;
	    day_box.y_pos = day_box.height * (row + 2);
        }
	break;
      
    case MwMonthSelected :
    case MwYearSelected  :
	if ((MwNamedMonth(mw, o)->month->year_number  != year) ||
	    (MwNamedMonth(mw, o)->month->month_number != month))
	{
	    return;
	}

	month_width = MwMonthNameWidth(mw,o);
	year_width = MwYearNameWidth(mw,o);
	separator = MwSepWidth(mw,o);

	if (st == MwMonthSelected)
	{
	    day_box.x_pos =
		(XtWidth(mw) - month_width - year_width - separator) / 2 - 1;
	    day_box.width = month_width + year_width + separator + 2;
	}
	else
	{
	    if (mby)
		day_box.x_pos =
		    ((XtWidth(mw) - month_width - year_width - separator) /2) +
		    month_width + separator - 1;
	    else
		day_box.x_pos =
		    (XtWidth(mw) - month_width - year_width - separator) / 2;

	    day_box.width = year_width + 2;
	}
	day_box.y_pos = 0;
	day_box.height = MAX(day_box.height,MwMonthHeight(mw,o)+1);
	break;

    }

    /*
    ** Turn around if rtol.
    */
    if (rtol)
    {
	day_box.x_pos = XtWidth(mw) - day_box.x_pos - day_box.width;
    }

    XClearArea
    (
	XtDisplay(mw),
	XtWindow(mw),
	day_box.x_pos,
	day_box.y_pos,
	day_box.width,
	day_box.height,
	True
    );

}

void MWGetSelection
#ifdef _DWC_PROTO_
	(
	Widget		w,
	MwTypeSelected	*s_type,
	Cardinal	*day,
	Cardinal	*week,
	Cardinal	*month,
	Cardinal	*year)
#else	/* no prototypes */
	(w, s_type, day, week, month, year)
	Widget		w;
	MwTypeSelected	*s_type;
	Cardinal	*day;
	Cardinal	*week;
	Cardinal	*month;
	Cardinal	*year;
#endif	/* prototype */
{
    MonthWidget	    mw = (MonthWidget) w;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    
    *s_type = MwCB(mw, o)->select_type;
    *day    = MwCB(mw, o)->select_day;
    *week   = MwCB(mw, o)->select_week;
    *month  = MwCB(mw, o)->select_month;
    *year   = MwCB(mw, o)->select_year;

}

void MWSetSelection
#ifdef _DWC_PROTO_
	(
	Widget		w,
	MwTypeSelected	s_type,
	Cardinal	day,
	Cardinal	week,
	Cardinal	month,
	Cardinal	year)
#else	/* no prototypes */
	(w, s_type, day, week, month, year)
	Widget		w;
	MwTypeSelected	s_type;
	Cardinal	day;
	Cardinal	week;
	Cardinal	month;
	Cardinal	year;
#endif	/* prototype */
{
    MonthWidget	    mw = (MonthWidget) w;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    
    toggle_select(MwCB(mw, o));

    MwCB(mw, o)->select_type  = s_type;
    MwCB(mw, o)->select_day   = day;
    MwCB(mw, o)->select_week  = week;
    MwCB(mw, o)->select_month = month;
    MwCB(mw, o)->select_year  = year;
    toggle_select(MwCB(mw, o));

}

#if !defined(AUD)
static
#endif
void ACTION_MW_DBLCLICK
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected	    s_type;
    Cardinal		    day;
    Cardinal		    week;
    Cardinal		    month;
    Cardinal		    year;
    DwcMonthCallbackStruct  cbs;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    
    s_type = MwCB(mw, o)->select_type;
    day    = MwCB(mw, o)->select_day;
    week   = MwCB(mw, o)->select_week;
    month  = MwCB(mw, o)->select_month;
    year   = MwCB(mw, o)->select_year;

    determine_selection
    (
	mw,
	(XButtonEvent *)event,
	&s_type,
	&day,
	&week,
	&month,
	&year
    );

    if (s_type == MwNothingSelected) {
	return;
    }
    
    if ((MwCB(mw, o)->select_type  != s_type) ||
	(MwCB(mw, o)->select_day   != day)    ||
	(MwCB(mw, o)->select_week  != week)   ||
	(MwCB(mw, o)->select_month != month)  ||
	(MwCB(mw, o)->select_year  != year))
    {

	toggle_select(MwCB(mw, o));

	MwCB(mw, o)->select_type  = s_type;
	MwCB(mw, o)->select_day   = day;
	MwCB(mw, o)->select_week  = week;
	MwCB(mw, o)->select_month = month;
	MwCB(mw, o)->select_year  = year;
	toggle_select(MwCB(mw, o));
    }
    
    cbs.reason	     = (int)XmCR_SINGLE_SELECT;
    cbs.select_type  = s_type;
    cbs.select_day   = day;
    cbs.select_week  = week;
    cbs.select_month = month;
    cbs.select_year  = year;

    XtCallCallbackList ((Widget) mw, MwDcCallback(mw,o), &cbs);

}

#if !defined(AUD)
static
#endif
void ACTION_MW_PSEUDODBL
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected	    s_type;
    Cardinal		    day;
    Cardinal		    week;
    Cardinal		    month;
    Cardinal		    year;
    DwcMonthCallbackStruct  cbs;
    XmOffsetPtr		    o = MwOffsetPtr(mw);
    
    MWGetSelection ((Widget)mw, &s_type, &day, &week, &month, &year);

    if (s_type == MwNothingSelected) {
	return;
    }
    
    cbs.reason	     = (int)XmCR_SINGLE_SELECT;
    cbs.select_type  = s_type;
    cbs.select_day   = day;
    cbs.select_week  = week;
    cbs.select_month = month;
    cbs.select_year  = year;
    XtCallCallbackList ((Widget) mw, MwDcCallback(mw,o), &cbs);

}

#if !defined(AUD)
static
#endif
void ACTION_MW_HOME
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected  s_type;
    Cardinal	    day;
    Cardinal	    week;
    Cardinal	    month;
    Cardinal	    year;
    Cardinal	    row, column;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    int		    i;
    MiMonthInfoList	*mlist;

    MWGetSelection ((Widget)mw, &s_type, &day, &week, &month, &year);

    switch (s_type)
    {
    case MwNothingSelected:
	s_type = MwMonthSelected;
	break;
    case MwYearSelected:
	s_type = MwMonthSelected;
	break;
    case MwMonthSelected:
	return;
    case MwWeekSelected:
	return;
    case MwDaySelected:
	s_type = MwWeekSelected;
	get_rowcol_for_day ((MonthWidget) mw, day, month, year, &row, &column);
	week = MwWeekNumbers(mw,o)[row-1];
	break;	
    }

    MWSetSelection ((Widget)mw, s_type, day, week, month, year);

}

#if !defined(AUD)
static
#endif
void ACTION_MW_UP
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected  s_type;
    Cardinal	    day;
    Cardinal	    week;
    Cardinal	    month;
    Cardinal	    year;
    Cardinal	    row, column;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    int		    i;
    MiMonthInfoList	*mlist;

    MWGetSelection ((Widget)mw, &s_type, &day, &week, &month, &year);

    switch (s_type)
    {
    case MwNothingSelected:
	s_type = MwMonthSelected;
	break;
    case MwYearSelected:
    case MwMonthSelected:
	if (!MwShowWeekNumbers(mw,o))
	{
	    date_for_position (mw, MwWeekDepth(mw,o), 1, &day, &mlist);
	    if (day == 0)
		date_for_position (mw, MwWeekDepth(mw,o)-1, 1, &day, &mlist);
	    if (day == 0) return;
	    month  = mlist->month->month_number;
	    year   = mlist->month->year_number;
	    s_type = MwDaySelected;
	}
	else
	{
	    week = MwWeekNumbers(mw,o)[MwWeekDepth(mw,o)-1];
	    if (week == 0)
		week = MwWeekNumbers(mw,o)[MwWeekDepth(mw,o)-2];
	    s_type = MwWeekSelected;
	}
	break;
    case MwWeekSelected:
	if (week == MwWeekNumbers(mw,o)[0])
	{
	    /*
	    ** Get the "right" month.
	    */
	    /*
	    ** Row 1 will either have stuff in column 1 or 7.  Sometimes
	    ** even both.  It should never have both empty.
	    */
	    date_for_position (mw, 1, 1, &day, &mlist);
	    if (day == 0) date_for_position (mw, 1, 7, &day, &mlist);
	    if (day == 0) return;
	    month  = mlist->month->month_number;
	    year   = mlist->month->year_number;
	    s_type = MwMonthSelected;
	}
	else
	    week--;
	break;
    case MwDaySelected:
	get_rowcol_for_day ((MonthWidget) mw, day, month, year, &row, &column);
	if (row == 1)
	{
	    s_type = MwMonthSelected;
	    break;
	}
	else
	{
	    date_for_position (mw, row - 1, column, &day, &mlist);
	    if (day == 0)
	    {
		if (row == 2)
		{
		    s_type = MwMonthSelected;
		    break;
		}
		date_for_position (mw, row - 2, column, &day, &mlist);
	    }
	    month  = mlist->month->month_number;
	    year   = mlist->month->year_number;
	    s_type = MwDaySelected;
	}
	break;
    };

    MWSetSelection ((Widget)mw, s_type, day, week, month, year);

}

#if !defined(AUD)
static
#endif
void ACTION_MW_DOWN
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected  s_type;
    Cardinal	    day, down1day, down2day;
    Cardinal	    week;
    Cardinal	    month;
    Cardinal	    year;
    Cardinal	    row, column;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    int		    i;
    MiMonthInfoList	*mlist;

    MWGetSelection ((Widget)mw, &s_type, &day, &week, &month, &year);

    switch (s_type)
    {
    case MwNothingSelected:
	/*
	** From nada to month.
	*/
	s_type = MwMonthSelected;
	break;
    case MwYearSelected:
    case MwMonthSelected:
	/*
	** Either go to the week number or the day of the week.
	*/
	if (!MwShowWeekNumbers(mw,o))
	{
	    day = MwFirstDay(mw,o);
	    s_type = MwDaySelected;
	}
	else
	{
	    week = MwWeekNumbers(mw,o)[0];
	    s_type = MwWeekSelected;
	}	    
	break;
    case MwWeekSelected:
	/*
	** Find the selected week.
	*/
	for (i = 0; i < MwWeekDepth(mw,o); i++)
	{
	    if (MwWeekNumbers(mw,o)[i] == MwWeekNumbers(mw,o)[i+1]) continue;
	    if (week == MwWeekNumbers(mw,o)[i]) break;
	}
	/*
	** If at bottom wrap to month name.
	*/
	if ((i >= (MwWeekDepth(mw,o) - 1)) || (MwWeekNumbers(mw,o)[i+1] == 0))
	{
	    /*
	    ** Get the "right" month.
	    */
	    /*
	    ** Row 1 will either have stuff in column 1 or 7.  Sometimes
	    ** even both.  It should never have both empty.
	    */
	    date_for_position (mw, 1, 1, &day, &mlist);
	    if (day == 0) date_for_position (mw, 1, 7, &day, &mlist);
	    if (day == 0) return;
	    month  = mlist->month->month_number;
	    year   = mlist->month->year_number;
	    s_type = MwMonthSelected;
	    break;
	}
	/*
	** Else go to the selected week.
	*/
	week = MwWeekNumbers(mw,o)[i+1];
	break;
    case MwDaySelected:
	/*
	** Where are we.
	*/
	get_rowcol_for_day ((MonthWidget) mw, day, month, year, &row, &column);
	do
	{
	    if (row < MwWeekDepth(mw, o))
	    {
		/*
		** Redundant assumption that day selection stays the same.
		*/
		s_type = MwDaySelected;
		/*
		** Determine what date is in the new position. If good break
		** out to default code.
		*/
		row++;
		date_for_position (mw, row, column, &day, &mlist);
		if (day != 0) break;

		/*
		** Try once more to see if this column is blank in the first
		** row we tried.
		*/
		row++;
		date_for_position (mw, row, column, &day, &mlist);
		if (day != 0) break;

	    }
	    /*
	    ** Give up and assume month.  We need to determine the "right"
	    ** month.
	    */
	    s_type = MwMonthSelected;
	    /*
	    ** Row 1 will either have stuff in column 1 or 7.  Sometimes
	    ** even both.  It should never have both empty.
	    */
	    date_for_position (mw, 1, 1, &day, &mlist);
	    if (day == 0) date_for_position (mw, 1, 7, &day, &mlist);
	    if (day == 0) return;

	} while (False);

	month  = mlist->month->month_number;
	year   = mlist->month->year_number;

	break;
    };

    MWSetSelection ((Widget)mw, s_type, day, week, month, year);

}

#if !defined(AUD)
static
#endif
void ACTION_MW_LEFT
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    if (LAYOUT_DIRECTION(mw))
	mw_do_right (mw, event, params, num_params);
    else
	mw_do_left (mw, event, params, num_params);
}

#if !defined(AUD)
static
#endif
void ACTION_MW_RIGHT
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    if (LAYOUT_DIRECTION(mw))
	mw_do_left (mw, event, params, num_params);
    else
	mw_do_right (mw, event, params, num_params);
}

static void mw_do_left
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected  s_type;
    Cardinal	    day;
    Cardinal	    week;
    Cardinal	    month;
    Cardinal	    year;
    Cardinal	    row, column;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    int		    i;
    MiMonthInfoList	*mlist;

    MWGetSelection ((Widget)mw, &s_type, &day, &week, &month, &year);

    switch (s_type)
    {
    case MwNothingSelected:
	/*
	** From nada to month.
	*/
	s_type = MwMonthSelected;
	break;
    case MwYearSelected:
	/*
	** From nada to month.
	*/
	s_type = MwMonthSelected;
	break;
    case MwMonthSelected:
	/*
	** From nada to month.
	*/
	s_type = MwYearSelected;
	break;
    case MwWeekSelected:
	s_type = MwDaySelected;

	for (row = 1; row < MwWeekDepth(mw,o); row++)
	{
	    if (MwWeekNumbers(mw,o)[row-1] == week) break;
	}
	/*
	** Get the end column.
	*/
	column = 7;
	/*
	** First try the end of the current row.
	*/
	date_for_position (mw, row, column, &day, &mlist);
	if (day == 0)
	{
	    /*
	    ** Then try the end of the next row.  This covers the case where
	    ** the end of the week is in a new month.
	    */
	    date_for_position (mw, row+1, column, &day, &mlist);
	}
	if (day == 0)
	{
	    for (column--; column > 0; column--)
	    {
		date_for_position (mw, row, column, &day, &mlist);
		if (day != 0) break;
	    }
	    if (day==0) return;
	}
	month  = mlist->month->month_number;
	year   = mlist->month->year_number;

	break;
    case MwDaySelected:
	/*
	** Where are we.
	*/
	get_rowcol_for_day ((MonthWidget) mw, day, month, year, &row, &column);
	/*
	** If at the first day of the week, go to the week number (or the
	** last day of the week as appropriate).
	*/
	if (column == 1)
	{
	    if (MwShowWeekNumbers(mw,o))
	    {
		week = MwWeekNumbers(mw,o)[row-1];
		s_type = MwWeekSelected;
		break;
	    }
	    else
	    {
		/*
		** Get the end column.
		*/
		column = 7;
		/*
		** First try the end of the current row.
		*/
		date_for_position (mw, row, column, &day, &mlist);
		if (day == 0)
		{
		    /*
		    ** Then try the end of the next row.  This covers the case
		    ** where the end of the week is in a new month.
		    */
		    date_for_position (mw, row+1, column, &day, &mlist);
		}
		if (day == 0)
		{
		    for (column--; column > 0; column--)
		    {
			date_for_position (mw, row, column, &day, &mlist);
			if (day != 0) break;
		    }
		    if (day==0) return;
		}
		month  = mlist->month->month_number;
		year   = mlist->month->year_number;

		break;
	    }
	}
	else
	{
	    /*
	    ** Go to next column.
	    */
	    column--;
	    /*
	    ** Determine what date is in the new position.
	    */
	    date_for_position (mw, row, column, &day, &mlist);
	    /*
	    ** If none, try up one.  This is because of the setting
	    ** that determines whether the beginning of the next month is
	    ** displayed on the same row with the end of the current.
	    */
	    if (day == 0)
	    {
		/*
		** Go to the previous row.
		*/
		row--;

		/*
		** If off top, special handling.
		*/
		if (row < 1)
		{
		    if (MwShowWeekNumbers(mw,o))
		    {
			/*
			** Off the top.
			*/
			week = MwWeekNumbers(mw,o)[row];
			s_type = MwWeekSelected;
			break;
		    }
		    else
		    {
			row++;
			/*
			** Find the first day on that row.
			*/
			for (column = 7; column >= 1; column--)
			{
			    date_for_position (mw, row, column, &day, &mlist);
			    if (day != 0) break;
			}
			/*
			** If none, noop.
			*/
			if (day == 0)
			{
			    return;
			}
			month  = mlist->month->month_number;
			year   = mlist->month->year_number;
		    }
		}
		/*
		** Get the date.
		*/
		date_for_position (mw, row, column, &day, &mlist);
		/*
		** Two strikes and your out.
		*/
		if (day == 0)
		{
		    /*
		    ** Off the bottom, punt.
		    */
		    week = MwWeekNumbers(mw,o)[row];
		    s_type = MwWeekSelected;
		    break;
		}
	    }
	    /*
	    ** fill in month and year which may have changed.
	    */
	    month  = mlist->month->month_number;
	    year   = mlist->month->year_number;
	    s_type = MwDaySelected;
	}
	break;
    };

    MWSetSelection ((Widget)mw, s_type, day, week, month, year);
}

static void mw_do_right
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected  s_type;
    Cardinal	    day;
    Cardinal	    week;
    Cardinal	    month;
    Cardinal	    year;
    Cardinal	    row, column;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    int		    i;
    MiMonthInfoList	*mlist;

    MWGetSelection ((Widget)mw, &s_type, &day, &week, &month, &year);

    switch (s_type)
    {
    case MwNothingSelected:
	/*
	** From nada to month.
	*/
	s_type = MwMonthSelected;
	break;
    case MwYearSelected:
	/*
	** From nada to month.
	*/
	s_type = MwMonthSelected;
	break;
    case MwMonthSelected:
	/*
	** From nada to month.
	*/
	s_type = MwYearSelected;
	break;
    case MwWeekSelected:
	s_type = MwDaySelected;

	/*
	** Find the row we're on.
	*/
	for (row = 1; row < MwWeekDepth(mw,o); row++)
	{
	    if (MwWeekNumbers(mw,o)[row-1] == week) break;
	}
	/*
	** Find the first day on that row.
	*/
	for (column = 1; column <= 7; column++)
	{
	    date_for_position (mw, row, column, &day, &mlist);
	    if (day != 0) break;
	}
	/*
	** If none, noop.
	*/
	if (day == 0)
	{
	    return;
	}
	month  = mlist->month->month_number;
	year   = mlist->month->year_number;

	break;
    case MwDaySelected:
	/*
	** Where are we.
	*/
	get_rowcol_for_day ((MonthWidget) mw, day, month, year, &row, &column);

	/*
	** If at the end, wrap to week.
	*/
	if (column == 7 && MwShowWeekNumbers(mw,o))
	{
	    week = MwWeekNumbers(mw,o)[row-1];
	    s_type = MwWeekSelected;
	    break;
	}
	else if (column == 7)
	{
	    /*
	    ** Find the first day on this row.
	    */
	    for (column = 1; column <= 7; column++)
	    {
		date_for_position (mw, row, column, &day, &mlist);
		if (day != 0) break;
	    }
	}
	else
	{
	    /*
	    ** Go to next column.
	    */
	    column++;

	    /*
	    ** Make sure we haven't fallen off the end of the world..
	    */
	    date_for_position (mw, row, column, &day, &mlist);

	    /*
	    ** If we are off the end?
	    */
	    if (day == 0 && MwShowWeekNumbers(mw,o))
	    {
		week = MwWeekNumbers(mw,o)[row-1];
		s_type = MwWeekSelected;
		break;
	    }
	    if (day == 0)
	    {
		/*
		** Try down one.  This is a check for "fill blank days" set to
		** false which would cause us to split the row at the month
		** boundary.
		*/
		date_for_position (mw, row+1, column, &day, &mlist);
	    }
	    /*
	    ** If we still haven't hit, then we must try to wrap.
	    */
	    if (day == 0)
	    {
		/*
		** Find the first day on the row.
		*/
		for (column = 1; column <= 7; column++)
		{
		    date_for_position (mw, row, column, &day, &mlist);
		    if (day != 0) break;
		}
	    }
	}
	/*
	** Determine what date is in the new position.
	*/
	date_for_position (mw, row, column, &day, &mlist);
	/*
	** If none, try down one.  This is because of the setting
	** that determines whether the beginning of the next month is
	** displayed on the same row with the end of the current.
	*/
	if (day == 0)
	{
	    /*
	    ** Go to the next row.
	    */
	    row++;
	    /*
	    ** If off bottom, punt.
	    */
	    if (row > MwWeekDepth(mw,o))
	    {
		/*
		** Off the bottom.
		*/
		week = MwWeekNumbers(mw,o)[row-2];
		s_type = MwWeekSelected;
		break;
	    }
	    /*
	    ** Get the date.
	    */
	    date_for_position (mw, row, column, &day, &mlist);
	    /*
	    ** Two strikes and your out.
	    */
	    if (day == 0)
	    {
		/*
		** Off the bottom, punt.
		*/
		week = MwWeekNumbers(mw,o)[row-2];
		s_type = MwWeekSelected;
		break;
	    }
	    /*
	    ** fill in month and year which may have changed.
	    */
	    month  = mlist->month->month_number;
	    year   = mlist->month->year_number;
	    s_type = MwDaySelected;
	}
	break;
    };

    MWSetSelection ((Widget)mw, s_type, day, week, month, year);
}

#if !defined(AUD)
static
#endif
void ACTION_MW_FOCUSIN
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected  s_type;
    Cardinal	    day;
    Cardinal	    week;
    Cardinal	    month;
    Cardinal	    year;
    Cardinal	    row, column;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    int		    i;
    MiMonthInfoList	*mlist;

    MWGetSelection ((Widget)mw, &s_type, &day, &week, &month, &year);

    if ((month == MwMonth(mw,o)) && (year == MwYear(mw,o)))
	return;

    month = MwMonth(mw,o);
    year = MwYear(mw,o);
    s_type = MwMonthSelected;
    MWSetSelection ((Widget)mw, s_type, day, week, month, year);
}

#if !defined(AUD)
static
#endif
void ACTION_MW_FOCUSOUT
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw,
	XButtonEvent	*event,
	String		*params,
	Cardinal	num_params)
#else	/* no prototypes */
	(mw, event, params, num_params)
	MonthWidget	mw;
	XButtonEvent	*event;
	String		*params;
	Cardinal	num_params;
#endif	/* prototype */
{
    MwTypeSelected  s_type;
    Cardinal	    day;
    Cardinal	    week;
    Cardinal	    month;
    Cardinal	    year;
    Cardinal	    row, column;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    int		    i;
    MiMonthInfoList	*mlist;

}

void MWSetDate
#ifdef _DWC_PROTO_
	(
	Widget		w,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year)
#else	/* no prototypes */
	(w, day, month, year)
	Widget		w;
	Cardinal	day;
	Cardinal	month;
	Cardinal	year;
#endif	/* prototype */
    {
    Cardinal	    old_month;
    Cardinal	    new_month;
    Cardinal	    height;
    Cardinal	    clear_height;
    MonthWidget	    mw = (MonthWidget)w;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    XFontStruct	    *font;

    if ((MwDay(mw, o)   == day)   &&
        (MwMonth(mw, o) == month) &&
        (MwYear(mw, o)  == year))
    {
	return;
    }

    old_month = (MwNamedMonth(mw, o)->month->year_number * 12) +
	         MwNamedMonth(mw, o)->month->month_number;
					      
    MwDay(mw, o)   = day;
    MwMonth(mw, o) = month;
    MwYear(mw, o)  = year;
    
    mw_free_header_strings (mw);
    mw_create_header_strings (mw);
    mw_get_header_extents (mw);
    setup_month(mw);

    new_month = (MwNamedMonth(mw, o)->month->year_number * 12) +
	         MwNamedMonth(mw, o)->month->month_number;
    /*
    ** The old code tried to break the display up into equal size pieces
    ** by the number of weeks shown plus the day of the week header and the
    ** month-year header.  Unfortunately, the headers are not the same size
    ** as the weeks and the text can overlap.  The current solution clears
    ** the whole window on each scroll.  A better solution would find the
    ** location of the headers and then clear above and below the baseline
    ** by the height information of the text used.  RJS 21-JAN-1991.
    */
    if (XtIsRealized ((Widget) mw))
    {
	/*
	** First do the main area.
	*/

	/*
	** height is generic height of a "row" in the month display.  It
	** is determined by dividing the height of the window by the number
	** of rows.  The number of rows is equal to the number of weeks +
	** a row for the month and year and a row for the days of the week
	** header.
	*/
	height = XtHeight (mw) / (MwWeekDepth(mw, o) + 2);

	/*
	** Find the biggest font in the main area.
	*/
	clear_height = MAX (MwDayNumAscent(mw, o), height);

	/*
	** Find the beginning of space to be cleared.  If the font is bigger
	** than the size of the box allocated for it, then use the font size
	** rather than the box size.
	*/
	clear_height = (height * 3) - clear_height;

	XClearArea (XtDisplay (mw), XtWindow (mw), 0, clear_height, 0, 0, TRUE);

	/*
	** Then do the month region, if necessary.
	*/
	if (new_month != old_month)
	{
	    /*
	    ** Get the size of area to clear.  If the month display font is
	    ** bigger than the box, use the font size.
	    */
	    clear_height = MAX(height, (MwMonthHeight(mw,o) + 1));

	    XClearArea
		(XtDisplay(mw), XtWindow(mw), 0, 0, 0, clear_height, TRUE);
	}
    }
}

CalendarBlock *MWCreateCalendarBlock
#ifdef	_DWC_PROTO_
	(
	Cardinal    cache_size,
	XtCallbackProc  callback,
	dwcaddr_t	    tag)
#else	/* no prototypes */
	(cache_size, callback, tag)
	Cardinal    cache_size;
	XtCallbackProc	callback;
	dwcaddr_t	    tag;
#endif	/* prototypes */
{
    CalendarBlock   *cb;

    cb = (CalendarBlock *) XtMalloc (sizeof (CalendarBlock));
    cb->allmonths    = (MiAllMonthInfo *) XtMalloc (sizeof (MiAllMonthInfo));
    MIInitialiseAllMonthInfo
	(cb->allmonths, cache_size, (MonthInfoCallbackProc) callback, tag);
    cb->select_type  = MwNothingSelected;
    cb->monthwidgets_number = 0;

    return (cb);
    
}

void
MWGetDayStyleMarkup
#ifdef _DWC_PROTO_
	(
	Widget		w,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year,
	unsigned char	*markup,
	unsigned char	*style)
#else	/* no prototypes */
	(w, day, month, year, markup, style)
	Widget		w;
	Cardinal	day;
	Cardinal	month;
	Cardinal	year;
	unsigned char	*markup;
	unsigned char	*style;
#endif	/* prototype */
    {
    MiMonthInfoList   *item;
    MonthWidget	    mw = (MonthWidget) w;
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    
    item = MIGetMonthInfoListItem (month, year, MwCB(mw, o)->allmonths);

    *markup = item->month->markup [day - 1];
    *style  = item->month->style  [day - 1];

    MIRemoveMonthInfoListItem (item, MwCB(mw, o)->allmonths);

}

void MWSetDayStyleMarkup
#ifdef _DWC_PROTO_
	(
	Widget		w,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year,
	unsigned char	markup,
	unsigned char	style)
#else	/* no prototypes */
	(w, day, month, year, markup, style)
	Widget		w;
	Cardinal	day;
	Cardinal	month;
	Cardinal	year;
	unsigned char	markup;
	unsigned char	style;
#endif	/* prototype */
    {
    Cardinal	    row;
    Cardinal	    column;
    box		    day_box;
    int		    i;
    MiMonthInfoList   *item;
    MonthWidget	    mw;
    XmOffsetPtr	    mo;
    MonthWidget	    this_mw = (MonthWidget)w;
    XmOffsetPtr	    to = MwOffsetPtr(this_mw);
    
    item = MIGetMonthInfoListItem (month, year, MwCB(this_mw, to)->allmonths);

    item->month->markup [day - 1] = markup;
    item->month->style  [day - 1] = style;

    MIRemoveMonthInfoListItem (item, MwCB(this_mw, to)->allmonths);

    for (i = 0;  i < MwCB(this_mw, to)->monthwidgets_number;  i++)
    {
	mw = (MonthWidget) MwCB(this_mw, to)->monthwidgets [i];
	mo = MwOffsetPtr(mw);
	if (XtIsRealized ((Widget) mw))
	{
	    if (MwShowWeekNumbers(mw, mo))
	    {
		day_box.width = XtWidth (mw) / 8;
	    }
	    else
	    {
		day_box.width = XtWidth (mw) / 7;
	    }

	    day_box.height = XtHeight (mw) / (MwWeekDepth(mw, mo) + 2);

	    get_rowcol_for_day
		((MonthWidget) mw, day, month, year, &row, &column);

	    if (row != 0)
	    {
		if (!MwShowWeekNumbers(mw, mo))
		{
		    day_box.x_pos = day_box.width * (column - 1);
		}
		else
		{
		    day_box.x_pos = day_box.width * column;
		}

		day_box.y_pos = day_box.height * (row + 1);

		if (LAYOUT_DIRECTION(mw))
		{
		    day_box.x_pos = XtWidth(mw) - day_box.x_pos - day_box.width;
		}

		XClearArea
		(
		    XtDisplay(mw),
		    XtWindow(mw),
		    day_box.x_pos,
		    day_box.y_pos,
		    day_box.width,
		    day_box.height,
		    TRUE
		);
	    }
	}
    }

}

void
MWRegetAllDayMarkup
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    MiListOfMonthInfoList	*mill;
    MiMonthInfoList	*mil;
    MiMonthInfoBlock	        *this_mi;
    Cardinal		i;
    MonthWidget		this_mw = (MonthWidget)w;
    XmOffsetPtr		to = MwOffsetPtr(this_mw);
    
    MIPurgeMonthCache (MwCB(this_mw, to)->allmonths);

    this_mi = (MiMonthInfoBlock *) XtMalloc (sizeof (MiMonthInfoBlock));
    mill = MwCB(this_mw, to)->allmonths->listlist;

    while (mill != NULL) {
	mil = mill->first_mil;
	while (mil != NULL) {
	    this_mi->year_number  = mil->month->year_number;
	    this_mi->month_number = mil->month->month_number;
	    this_mi->month_length = mil->month->month_length;
	    MIGetMonthMarkup(this_mi, MwCB(this_mw, to)->allmonths);
	    for (i = 0;  i < this_mi->month_length;  i++)
	    {
		if ((this_mi->markup [i] != mil->month->markup [i]) ||
		    (this_mi->style  [i] != mil->month->style  [i]))
		{
		    MWSetDayStyleMarkup
		    (
			(Widget) this_mw,
			i + 1,
			this_mi->month_number,
			this_mi->year_number,
			this_mi->markup [i],
			this_mi->style [i]
		    );
		}
	    }
	    mil = mil->next;
	}
	mill = mill->next;
    }

    XtFree ((char *) this_mi);

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      create_entry_gcs - creates the GCs that are used by the monthwidget to
**	mark days which have an entry on them. (NOTE that the "special"
**	attribute takes precedence over the "entry" attribute.
**
**  FORMAL PARAMETERS:
**
**      MonthWidget mw;
**
**  RETURN VALUE:
**
**      NONE
**
**  SIDE EFFECTS:
**
**      Creates the following GCs:
**	MwGCInvertEntry(mw,o)
**	MwGCEntry(mw,o)
**
**  DESIGN:
**
**      Creates new GCS.
**
**
**  IMPLICIT INPUT PARAMETERS:
**   
**      Uses MwSpecialForeground(mw,o), MwBackground(mw,o),
**	MwFontWorkDaysEntry(mw,o) and MwFontNonWOrkDaysEntry(mw,o)
**   
**
**--
*/
static void
create_entry_gcs
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw)
#else	/* no prototypes */
	(mw)
	MonthWidget	mw;
#endif	/* prototype */
    {
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    XGCValues	    gcv;
    XFontStruct	    *font;


    gcv.foreground = MwEntryForeground(mw, o);
    gcv.background = MwBackground(mw,o);
    gcv.plane_mask = MwEntryForeground(mw,o) ^ MwBackground(mw,o);
    gcv.function = GXinvert;
    MwGCInvertEntry(mw, o) = XtGetGC
	((Widget) mw, GCFunction | GCPlaneMask, &gcv);

    MISCGetFontFromFontlist(MwFontWorkDaysEntry(mw, o), &font);
    gcv.font = font->fid;
    MwGCEntry(mw, o) = XtGetGC ((Widget) mw, GCFont | GCForeground, &gcv);

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      destroy_entry_gcs - destroys entry gcs (if they exist).
**
**  FORMAL PARAMETERS:
**
**      MonthWidget mw;
**
**  RETURN VALUE:
**
**      NONE
**
**  SIDE EFFECTS:
**
**      Destroys MwGCInvertEntry(mw,o), MwGCEntry(mw,o)
**
**
**--
*/
static void
destroy_entry_gcs
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw)
#else	/* no prototypes */
	(mw)
	MonthWidget	mw;
#endif	/* prototype */
{
    XmOffsetPtr	    o = MwOffsetPtr(mw);

    if (MwGCInvertEntry(mw,o) != NULL)
    {
	XtReleaseGC ((Widget) mw, MwGCInvertEntry(mw,o));
	MwGCInvertEntry(mw,o) = NULL;
    }

    if (MwGCEntry(mw,o) != NULL)
    {
	XtReleaseGC ((Widget) mw, MwGCEntry(mw,o));
	MwGCEntry(mw,o) = NULL;
    }

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      create_plain_gcs - creates the GCs that are used by the monthwidget to
**	for gcs which aren't "entry" or "special".
**
**  FORMAL PARAMETERS:
**
**      MonthWidget mw;
**
**  RETURN VALUE:
**
**      NONE
**
**  SIDE EFFECTS:
**
**      Creates the following GCs:
**
**	MwGCNormal(mw,o)
**	MwGCInvert(mw,o)
**	
**  DESIGN:
**
**      Creates new GCS.
**
**
**  IMPLICIT INPUT PARAMETERS:
**   
**   Uses MwForeground(mw,o), MwBackground(mw,o), MwFontWorkDays(mw,o),
**   MwFontNonWorkDays(mw,o), MwFontDayNames(mw,o), MwFontMonthName(mw, o),
**   MwFontWeekNumbers(mw, o)
**
**--
*/
static void
create_plain_gcs
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw)
#else	/* no prototypes */
	(mw)
	MonthWidget	mw;
#endif	/* prototype */
    {
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    XGCValues	    gcv;
    XFontStruct	    *font;


    MISCGetFontFromFontlist(MwFontWorkDays(mw, o), &font);
    gcv.font = font->fid;
    gcv.foreground = MwForeground(mw, o);
    gcv.background = MwBackground(mw, o);
    gcv.plane_mask = MwForeground(mw, o) ^ MwBackground(mw, o);
    gcv.function = GXinvert;
    MwGCNormal(mw, o) = XtGetGC ((Widget) mw, GCFont | GCForeground, &gcv);
    MwGCInvert(mw, o) = XtGetGC ((Widget) mw, GCFunction | GCPlaneMask, &gcv);

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      destroy_plain_gcs - destroys the non-"entry" and non-"special" gcs if
**	they exist.
**
**  FORMAL PARAMETERS:
**
**      MonthWidget mw;
**
**  RETURN VALUE:
**
**      NONE
**
**  SIDE EFFECTS:
**
**      Destroys:
**	MwGCNormal(mw,o)
**	MwGCInvert(mw,o)
**
**--
*/
static void destroy_plain_gcs
#ifdef _DWC_PROTO_
	(MonthWidget mw)
#else	/* no prototypes */
	(mw)
	MonthWidget	mw;
#endif	/* prototype */
{
    XmOffsetPtr	    o = MwOffsetPtr(mw);

    if (MwGCNormal(mw,o) != NULL)
    {
	XtReleaseGC ((Widget) mw, MwGCNormal(mw,o));
	MwGCNormal(mw,o) = NULL;
    }

    if (MwGCInvert(mw,o) != NULL)
    {
	XtReleaseGC ((Widget) mw, MwGCInvert(mw,o));
	MwGCInvert(mw,o) = NULL;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      create_special_gcs - creates the GCs that are used by the monthwidget to
**	mark days as "special" that the user has requested should be marked as
**	"special".
**
**  FORMAL PARAMETERS:
**
**      MonthWidget mw;
**
**  RETURN VALUE:
**
**      NONE
**
**  SIDE EFFECTS:
**
**      Creates the following GCs:
**	MwGCInvertSpecial(mw,o)
**	MwGCSpecialDays(mw,o)
**
**  DESIGN:
**
**      Creates new GCs.
**
**
**  IMPLICIT INPUT PARAMETERS:
**   
**      Uses MwSpecialForeground(mw,o), MwBackground(mw,o),
**	MwFontSpecialDays(mw,o) and MwFontSpecialDaysEntry(mw,o)
**   
**
**--
*/
static void create_special_gcs
#ifdef _DWC_PROTO_
    (MonthWidget mw)
#else	/* no prototypes */
    (mw)
    MonthWidget	mw;
#endif	/* prototype */
{
    XmOffsetPtr	    o = MwOffsetPtr(mw);
    XGCValues	    gcv;
    XFontStruct	    *font;


    gcv.foreground = MwSpecialForeground(mw, o);
    gcv.background = MwBackground(mw,o);
    gcv.plane_mask = MwSpecialForeground(mw,o) ^ MwBackground(mw,o);
    gcv.function = GXinvert;
    MwGCInvertSpecial(mw, o) = XtGetGC
	((Widget) mw, GCFunction | GCPlaneMask, &gcv);

    MISCGetFontFromFontlist(MwFontSpecialDays(mw, o), &font);
    gcv.font = font->fid;
    MwGCSpecialDays(mw, o) = XtGetGC ((Widget) mw, GCFont | GCForeground, &gcv);

    
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      destroy_special_gcs - destroys special gcs (if they exist).
**
**  FORMAL PARAMETERS:
**
**      MonthWidget mw;
**
**  RETURN VALUE:
**
**      NONE
**
**  SIDE EFFECTS:
**
**      Destroys MwGCInvertSpecial(mw,o), MwGCSpecialDays(mw,o)
**
**
**--
*/
static void destroy_special_gcs
#ifdef _DWC_PROTO_
    (MonthWidget	mw)
#else	/* no prototypes */
    (mw)
    MonthWidget	mw;
#endif	/* prototype */
{
    XmOffsetPtr	    o = MwOffsetPtr(mw);

    if (MwGCInvertSpecial(mw,o) != NULL)
    {
	XtReleaseGC ((Widget) mw, MwGCInvertSpecial(mw,o));
	MwGCInvertSpecial(mw,o) = NULL;
    }

    if (MwGCSpecialDays(mw,o) != NULL)
    {
	XtReleaseGC ((Widget) mw, MwGCSpecialDays(mw,o));
	MwGCSpecialDays(mw,o) = NULL;
    }

}

static void apply_style
#ifdef _DWC_PROTO_
    (
    MonthWidget		w,
    unsigned char	style,
    box			*sbox,
    GC			gc)
#else	/* no prototypes */
    (w, style, sbox, gc)
    MonthWidget		w;
    unsigned char	style;
    box			*sbox;
    GC			gc;
#endif	/* prototype */
/*
**++
**
** Draw extra "stuff" over or around a day in the month.
**
**--
*/
{
    int		    x1, x2, y1, y2;
    Display	    *display = XtDisplay(w);
    Window	    window = XtWindow(w);

    if (style == 0)  return;

    if ((style & MWSTYLE_CROSS_THRU) != 0)
    {
	x1 = sbox->x_pos + 1;
	y1 = sbox->y_pos - 1;
	x2 = sbox->x_pos + sbox->width  - 2;
	y2 = sbox->y_pos - sbox->height + 2;
	XDrawLine (display, window, gc, x1, y1, x2, y2);
	XDrawLine (display, window, gc, x1, y2, x2, y1);
    }

    if ((style & MWSTYLE_CIRCLE) != 0)
    {
	x1 = sbox->x_pos + 1;
	y1 = sbox->y_pos - sbox->height + 2;
	x2 = sbox->width  - 3;	/* last pixel is x+width-1 so last -2 is */
	y2 = sbox->height - 3;	/* x+width-3.  similarly for y and height */
	XDrawArc (display, window, gc, x1, y1, x2, y2, 0, 360 * 64);
    }

    if ((style & MWSTYLE_SQUARE) != 0)
    {
	x1 = sbox->x_pos + 1;
	y1 = sbox->y_pos - sbox->height + 2;
	x2 = sbox->width  - 3;
	y2 = sbox->height - 3;
	XDrawRectangle (display, window, gc, x1, y1, x2, y2);
    }

    if ((style & MWSTYLE_SLASH_THRU) != 0)
    {
	x1 = sbox->x_pos + 1;
	y1 = sbox->y_pos - 1;
	x2 = sbox->x_pos + sbox->width  - 2;
	y2 = sbox->y_pos - sbox->height + 2;
	XDrawLine (display, window, gc, x1, y1, x2, y2);
    }

}

static void get_xmstring_extents
#ifdef _DWC_PROTO_
(
    XmString	string,
    XmFontList	fontlist,
    int		*width,
    int		*height
)
#else	/* no prototypes */
(string, fontlist, width, height)
XmString	string;
XmFontList	fontlist;
int		*width;
int		*height;
#endif	/* prototype */
{
    *width = XmStringWidth (fontlist, string);
    /*
    ** Distance from the top to the baseline of the first line is the same
    ** as the ascent.
    */
    *height = XmStringBaseline (fontlist, string);
}

static void display_centred_xmstring
#ifdef _DWC_PROTO_
    (
    MonthWidget	w,
    XmString	string,
    box		*text_box,
    XmFontList	fontlist,
    GC		gc
    )
#else	/* no prototypes */
    (w, string, text_box, fontlist, gc)
    MonthWidget	w;
    XmString	string;
    box		*text_box;
    XmFontList	fontlist;
    GC		gc;
#endif	/* prototype */
/*
**++
**
** Draw the text string with its ASCENT centered in the given box.
**
**--
*/
{
    int		width, height;
    Cardinal	x, y;
    XmManagerWidget	p;

    /*
    ** Get the width and the ascent of this text string.
    */
    get_xmstring_extents(string, fontlist, &width, &height);

    /*
    ** Center that box in the display space.
    */
    x = text_box->x_pos + ((text_box->width  - width)  / 2);
    y = text_box->y_pos - ((text_box->height + height) / 2);

    p = (XmManagerWidget) XtParent(w);
    /*
    ** Draw the string at that location.
    */
    XmStringDraw
    (
	XtDisplay(w),
	XtWindow(w),
	fontlist,
	string,
	gc,
	x,
	y,
	width,
	XmALIGNMENT_BEGINNING,
	p->manager.string_direction,
	NULL
    );

}

void static center_xmstring_with_style
#ifdef _DWC_PROTO_
(
    MonthWidget		w,
    XmString		string,
    box			*text_box,
    XmFontList		fontlist,
    GC			gc_tns,
    unsigned char	style,
    GC			gc_invert,
    Boolean		invert
)
#else	/* no prototypes */
    (w, string, text_box, fontlist, gc_tns, style, gc_invert, invert)
    MonthWidget		w;
    XmString		string;
    box			*text_box;
    XmFontList		fontlist;
    GC			gc_tns;
    unsigned char	style;
    GC			gc_invert;
    Boolean		invert;
#endif	/* prototype */
{
    Display	*display = XtDisplay(w);
    Window	window = XtWindow(w);

    if (invert)
    {
	XClearArea
	(
	    display,
	    window,
	    text_box->x_pos,
	    text_box->y_pos - text_box->height + 1,
	    text_box->width,
	    text_box->height,
	    FALSE
	);
    }

    display_centred_xmstring
	(w, string, text_box, fontlist, gc_tns);

    apply_style (w, style, text_box, gc_tns);

    if (invert)
    {
	XFillRectangle
	(
	    display,
	    window,
	    gc_invert,
	    text_box->x_pos,
	    text_box->y_pos - text_box->height + 1,
	    text_box->width,
	    text_box->height
	);
    }
}

static void mw_create_header_strings
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw
	)
#else
	(mw)
	MonthWidget	mw;
#endif
{
    XmOffsetPtr		o = MwOffsetPtr(mw);
    dtb			date_time;
    Cardinal		today;

    if ((MwDay(mw,o) == 0) ||
	(MwMonth(mw,o) == 0) ||
	(MwYear(mw,o) == 0))
    {
	struct tm	*local_time;
	time_t	the_time;

	time (&the_time);
	local_time = localtime (&the_time);

	date_time.month = local_time->tm_mon + 1;
	date_time.year = local_time->tm_year + 1900;
    }
    else
    {
	date_time.month = MwMonth(mw,o);
	date_time.year  = MwYear(mw,o);
    }

    MwMonthName(mw,o) = DATEFORMATTimeToCS (dwc_k_mw_month_format, &date_time);
    MwYearName(mw,o) = DATEFORMATTimeToCS (dwc_k_mw_year_format, &date_time);
    MwSepString(mw,o) = DATEFORMATTimeToCS
	(dwc_k_mw_separator_format, &date_time);
}

static void mw_free_header_strings
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw
	)
#else
	(mw)
	MonthWidget	mw;
#endif
{
    XmOffsetPtr		o = MwOffsetPtr(mw);

    if (MwMonthName(mw,o) != NULL) XmStringFree(MwMonthName(mw,o));
    if (MwYearName(mw,o) != NULL) XmStringFree(MwYearName(mw,o));
    if (MwSepString(mw,o) != NULL) XmStringFree(MwSepString(mw,o));
}

static void mw_get_header_extents
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw
	)
#else
	(mw)
	MonthWidget	mw;
#endif
{
    int			dummy;
    XmOffsetPtr		o = MwOffsetPtr(mw);

    MwMonthNameHeight(mw,o) = 0;
    get_xmstring_extents
    (
	MwMonthName(mw,o),
	MwFontMonthName(mw,o),
	&MwMonthNameWidth(mw,o),
	&dummy
    );
    MwMonthNameHeight(mw,o) = MAX(dummy,MwMonthNameHeight(mw,o));

    get_xmstring_extents
    (
	MwYearName(mw,o),
	MwFontMonthName(mw,o),
	&MwYearNameWidth(mw,o),
	&dummy
    );
    MwMonthNameHeight(mw,o) = MAX(dummy,MwMonthNameHeight(mw,o));

    get_xmstring_extents
    (
	MwSepString(mw,o),
	MwFontMonthName(mw,o),
	&MwSepWidth(mw,o),
	&dummy
    );
    MwMonthNameHeight(mw,o) = MAX(dummy,MwMonthNameHeight(mw,o));

}

static void mw_create_dayname_strings
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw
	)
#else
	(mw)
	MonthWidget	mw;
#endif
{
    XmOffsetPtr		o = MwOffsetPtr(mw);
    Cardinal		today;
    dtb			date_time;

    for (today = 1; today <= 7; today++)
    {
	date_time.weekday = today;
	MwDayNames(mw,o)[today-1] = DATEFORMATTimeToCS
	    (dwc_k_mw_weekname_format, &date_time);
    }
}

static void mw_free_dayname_strings
#ifdef _DWC_PROTO_
	(
	MonthWidget	mw
	)
#else
	(mw)
	MonthWidget	mw;
#endif
{
    XmOffsetPtr		o = MwOffsetPtr(mw);
    Cardinal		today;

    for (today = 1; today <= 7; today++)
    {
	if (MwDayNames(mw,o)[today-1] != NULL)
	     XmStringFree(MwDayNames(mw,o)[today-1]);
    }
}

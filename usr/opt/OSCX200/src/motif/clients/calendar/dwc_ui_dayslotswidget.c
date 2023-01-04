/* dwc_ui_dayslotswidget.c */
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
**	This module implements the dayslots widget.
**
**--
*/

#include "dwc_compat.h"

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Xatom.h>			/* for XA_INTEGER */
#include <Xm/XmP.h>
#include <stdio.h>
#include <assert.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_timeslotwidget.h"
#include "dwc_ui_timebarwidget.h"
#include "dwc_ui_dayslotswidgetp.h"
#include "dwc_ui_mba.h"			/* for MbaAction */
#include "dwc_ui_misc.h"		/* for MISCGetFontFromFontlist */

#define forGCmask       GCForeground | GCBackground | GCLineWidth | GCDashList
#define formapGCmask    GCForeground | GCBackground | GCTile | GCFillStyle | GCLineWidth | GCDashList

#define backGCmask      GCForeground | GCBackground | GCLineWidth
#define backmapGCmask   GCForeground | GCBackground | GCTile | GCFillStyle | GCLineWidth

#define	TRANSLATE_TIME_TO_Y(time, dsw) \
    ((((time) * DswTimeLabelHeight(dsw, o)) / DswActualDayslotsSize(dsw, o)) +  \
     DswTopMargin(dsw, o) + 1)

#define	TIME_HEIGHT(time, dsw) \
    ((((time) * DswTimeLabelHeight(dsw, o)) / DswActualDayslotsSize(dsw, o)) - 1)

static int  AutoScrollTimerInterval = 30;

static XmString	zero_length_string;

typedef enum
   {DswEVText, DswEVDrag, DswEVResizeTop, DswEVResizeBottom}
DswEVKind;


static Boolean AcceptFocus PROTOTYPE ((Widget w, Time *time));

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

static void Resize PROTOTYPE ((
	Widget	w));

static Boolean
SetValues PROTOTYPE ((
	Widget	o,
	Widget	r,
	Widget	n));

#if !defined (AUD)
static
#endif
void ACTION_DSW_HELP PROTOTYPE ((
	Widget	w,
	XEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_DSW_INVERTCANCEL PROTOTYPE ((
	Widget		w,
	XButtonEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_DSW_INVERTEND PROTOTYPE ((
	Widget		w,
	XButtonEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_DSW_INVERTMOTION PROTOTYPE ((
	Widget		w,
	XButtonEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_DSW_INVERTSTART PROTOTYPE ((
	Widget		w,
	XButtonEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_DSW_QUICK_COPY PROTOTYPE ((
	Widget	w,
	XEvent	*event));

#if !defined (AUD)
static
#endif
void ACTION_DSW_QUICK_MOVE PROTOTYPE ((
	Widget	w,
	XEvent	*event));

static void allocate_more_columns PROTOTYPE ((
	int		**columns,
	Cardinal	*num_columns,
	DwcDswEntry	***entries,
	Cardinal	**num_entries));

static void
arrange_timeslots PROTOTYPE ((
	DayslotsWidget	dsw,
	DwcDswEntry	topmost,
	Time		time));

static void
auto_scroll_cancel PROTOTYPE ((
	DayslotsWidget	dsw));

static void
auto_scroll_timer_tick PROTOTYPE ((
	DayslotsWidget	dsw,
	XtIntervalId	*id));

static void
auto_scroll_to_position PROTOTYPE ((
	DayslotsWidget	dsw,
	int		y));

static void
change_inverted PROTOTYPE ((
	DayslotsWidget	dsw,
	int		y));

static void
change_inverted_area PROTOTYPE ((
	DayslotsWidget	dsw,
	int		oldtop,
	int		oldbottom,
	int		newtop,
	int		newbottom));

static void
change_inverted_to PROTOTYPE ((
	DayslotsWidget	dsw,
	int		entry_start_time,
	int		entry_end_time));

static void
change_outline PROTOTYPE ((
	DayslotsWidget	dsw));

static void
click_on_timeslot PROTOTYPE ((
	DayslotsWidget	dsw,
	Widget		w,
	Boolean		focus_only,
	Time		time));

static void
create_dayslots_gcs PROTOTYPE ((
	DayslotsWidget	dsw));

static void
create_fore_back_gcs PROTOTYPE ((
	DayslotsWidget	dsw));

static void
create_index_gcs PROTOTYPE ((
	DayslotsWidget	dsw));

static Widget
create_timeslot PROTOTYPE ((
	DayslotsWidget	dsw,
	Position	y_pos,
	Dimension	height,
	unsigned char	*icons,
	Cardinal	num_icons,
	Boolean		editable));

static void
display_dayslots PROTOTYPE ((
	DayslotsWidget	dsw,
	XRectangle	*expose_area,
	Region		region,
	Boolean		clear_anyway,
	Boolean		draw_index));

static void
display_index_time PROTOTYPE ((
	DayslotsWidget	dsw,
	XRectangle	*expose_area));

static void
display_slot_lines PROTOTYPE ((
	DayslotsWidget	dsw,
	XRectangle	*expose_area,
	Region		region));

static void
display_slot_times PROTOTYPE ((
	DayslotsWidget	dsw,
	XRectangle	*expose_area,
	Region		region));

static void
double_click_timeout PROTOTYPE ((
	caddr_t		tag,
	XtIntervalId	*id));

static void
draw_outline PROTOTYPE ((
	DayslotsWidget	dsw));

static void
erase_outline PROTOTYPE ((
	DayslotsWidget	dsw));

static void
force_entry_visible PROTOTYPE ((
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	DswEVKind	kind));

static void
get_default_timeslot_dimensions PROTOTYPE ((
	DayslotsWidget	dsw));

static Cardinal get_fine_increment_height PROTOTYPE ((DayslotsWidget dsw));

static void get_icons_pixmaps PROTOTYPE ((
	DayslotsWidget	dsw,
	unsigned char	*icons,
	Cardinal	num_icons,
	Pixmap		pixmaps[]));

static void get_timetext_width PROTOTYPE ((
	DayslotsWidget	dsw,
	XmString	*text,
	Cardinal	time,
	Dimension	*width));

static void get_timetext_widths PROTOTYPE ((DayslotsWidget dsw, Boolean reset));

static int IndexYPositionForTime PROTOTYPE ((
	Cardinal	time,
	DayslotsWidget	dsw));

static void invert_outline PROTOTYPE ((
	DayslotsWidget	dsw,
	XRectangle	*expose_area,
	Region		region,
	Position	x1,
	Position	y1,
	Position	x2,
	Position	y2));

static void
move_index PROTOTYPE ((
	DayslotsWidget	dsw,
	int		y));

static void
move_index_time PROTOTYPE ((
	DayslotsWidget	dsw,
	int		t));

static DwcDswEntry
over_which_timeslot PROTOTYPE ((
	DayslotsWidget	dsw,
	Position	y1,
	Position	y2));

static void
receive_client_message PROTOTYPE ((
	DayslotsWidget		dsw,
	caddr_t			foo,
	XClientMessageEvent	*event));

static void
request_edit_timeslot PROTOTYPE ((
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	Time		time));

static void
send_client_message PROTOTYPE ((
	DayslotsWidget	dsw,
	DswCMKind	kind,
	XRectangle	*rect));

static void
set_dayslots_stacking PROTOTYPE ((
	DayslotsWidget	dsw,
	Cardinal	max_columns,
	DwcDswEntry	**entries,
	Cardinal	*num_entries,
	DwcDswEntry	topmost));

static void
setup_dayslots_display PROTOTYPE ((
	DayslotsWidget	dsw,
	Boolean		geometry_direct,
	Boolean		reset_widths));

static void
setup_dayslots_positions PROTOTYPE ((
	DayslotsWidget	dsw));

static void
setup_slot_lines PROTOTYPE ((
	DayslotsWidget	dsw));

static void
sort_entries PROTOTYPE ((
	DayslotsWidget	dsw));

static void start_invert_motion PROTOTYPE ((
	Widget		w,
	XButtonEvent	*event));

static int
TimeForIndexYPosition PROTOTYPE ((
	DayslotsWidget	dsw,
	Position	y_pos));

static void
TIMESLOT_CANCEL_RESIZE_AND_DRAG PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_CLOSE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_DOUBLE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_END_RESIZE_AND_DRAG PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_EXTEND PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_FOCUS PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_HELP PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_LOSING_FOCUS PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_RESIZE_AND_DRAG PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_SINGLE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void
TIMESLOT_START_RESIZE_AND_DRAG PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static int TimeTextYPosition PROTOTYPE ((
	Cardinal	time,
	DayslotsWidget	dsw));

static int
translate_y_to_time PROTOTYPE ((
	DayslotsWidget	dsw,
	Position	y_pos,
	Boolean		round));

static void TIMESLOT_INCR_SIZE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));

static void TIMESLOT_INCR_POSI PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs));


static XtCallbackRec SingleCB [2] = {
    {(XtCallbackProc) TIMESLOT_SINGLE,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec DoubleCB [2] = {
    {(XtCallbackProc) TIMESLOT_DOUBLE,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec FocusCB [2] = {
    {(XtCallbackProc) TIMESLOT_FOCUS,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec LosingFocusCB[2] = {
    {(XtCallbackProc) TIMESLOT_LOSING_FOCUS,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec CloseCB [2] = {
    {(XtCallbackProc) TIMESLOT_CLOSE,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec ExtendCB [2] = {
    {(XtCallbackProc) TIMESLOT_EXTEND,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec TimeslotHelpCB [2] = {
    {(XtCallbackProc) TIMESLOT_HELP,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec StartResizeAndDragCB [2] = {
    {(XtCallbackProc) TIMESLOT_START_RESIZE_AND_DRAG, NULL},
    {NULL,				    NULL}
};

static XtCallbackRec EndResizeAndDragCB [2] = {
    {(XtCallbackProc) TIMESLOT_END_RESIZE_AND_DRAG,   NULL},
    {NULL,				    NULL}
};

static XtCallbackRec CancelResizeAndDragCB [2] = {
    {(XtCallbackProc) TIMESLOT_CANCEL_RESIZE_AND_DRAG,   NULL},
    {NULL,				    NULL}
};

static XtCallbackRec ResizeAndDragCB [2] = {
    {(XtCallbackProc) TIMESLOT_RESIZE_AND_DRAG,	    NULL},
    {NULL,				    NULL}
};

static XtCallbackRec IncrSizeCB [2] = {
    {(XtCallbackProc) TIMESLOT_INCR_SIZE,	NULL},
    {NULL,			NULL}
};

static XtCallbackRec IncrPositionCB [2] = {
    {(XtCallbackProc) TIMESLOT_INCR_POSI,	NULL},
    {NULL,			NULL}
};


static char translations [] =
    " @Help<BtnDown>:		ACTION_DSW_HELP()\n\
     ~@Help <Btn1Down>:		ACTION_DSW_INVERTSTART()\n\
     ~@Help Button1<Motion>:	ACTION_DSW_INVERTMOTION()\n\
     ~@Help <Btn1Up>:		ACTION_DSW_INVERTEND()\n\
     Button1<BtnDown>:		ACTION_DSW_INVERTCANCEL()\n\
     Button2<BtnDown>:		ACTION_DSW_INVERTCANCEL()\n\
     Button3<BtnDown>:		ACTION_DSW_INVERTCANCEL()\n\
     Button4<BtnDown>:		ACTION_DSW_INVERTCANCEL()\n\
     Button5<BtnDown>:		ACTION_DSW_INVERTCANCEL()\n\
     ~Ctrl <Btn2Down>:		ACTION_DSW_QUICK_COPY()\n\
      Ctrl <Btn2Down>:		ACTION_DSW_QUICK_MOVE()";

static XtActionsRec action_table [] = {
    {"ACTION_DSW_INVERTSTART",	(XtActionProc)ACTION_DSW_INVERTSTART},
    {"ACTION_DSW_INVERTMOTION",	(XtActionProc)ACTION_DSW_INVERTMOTION},
    {"ACTION_DSW_INVERTEND",	(XtActionProc)ACTION_DSW_INVERTEND},
    {"ACTION_DSW_INVERTCANCEL",	(XtActionProc)ACTION_DSW_INVERTCANCEL},
    {"ACTION_DSW_HELP",		(XtActionProc)ACTION_DSW_HELP},
    {"ACTION_DSW_QUICK_COPY",	(XtActionProc)ACTION_DSW_QUICK_COPY},
    {"ACTION_DSW_QUICK_MOVE",	(XtActionProc)ACTION_DSW_QUICK_MOVE},
    {NULL,			NULL}
};

/*
**  Dayslots size;  actual dayslots size
**  
**  Entries:  start, duration, text
**
**  Callbacks for change attempts etc.
**
**  Routines to change entries, get selected
**
**  European/Anglo style
**
**  Right-to-left
**
**  Callbacks for scroll over occluded areas (beyond end, before start)
**
**  Help
**
**  AM/PM
**
**  width, but not height.
*/

static unsigned short int default_dayslots_size  = 30;
static unsigned short int default_fine_increment = 5;
static Boolean	          default_on_the_line	 = TRUE;
static Boolean	          default_stack 	 = FALSE;
static int		  default_v_margin       = 1;
static int		  default_h_margin       = 4;
static int		  default_zero		 = 0;
static Cardinal		  default_work_start	 = ( 8 * 60) + 30;
static Cardinal		  default_work_finish 	 = (17 * 60) + 30;
static Cardinal		  default_lunch_start	 = (12 * 60) + 00;
static Cardinal		  default_lunch_finish 	 = (13 * 60) + 00;
static Boolean		  default_editable	 = TRUE;
static Boolean		  default_day_notes	 = FALSE;

static Dimension	  default_timebar_width  = 4;

static Dimension	  default_pixmap_height = 17;
static Dimension	  default_pixmap_width = 17;

static unsigned char
    *dayslot_styles_dashes [No_of_Dayslot_Styles] =
				 {  (unsigned char *)"\1\1",
				    (unsigned char *)"\1\3",
				    (unsigned char *)"\1\7",
				    (unsigned char *)"\1\17",
				    (unsigned char *)"\1\37"};
				    /* no pixels on, no pixels off */
typedef struct {
    unsigned char   increment;
    unsigned char   number_of_styles;
    unsigned char   *lines_per_style;
    unsigned char   *increments_for_style;
    unsigned char   *styles;
 } DswSlotLines;


static DswSlotLines slot_lines [] = {
    {(unsigned char) 60,    /* increment in minutes for the timeslots */
     (unsigned char) 1,	    /* how many styles are used across the increments */
     (unsigned char *)"\1", /* how many lines of increment for each style used */
     (unsigned char *)"\74", /* the increments for the various styles */
     (unsigned char *)"\0"}, /* the styles used */


    /*	  
    **	So for example, if our timeslot size is 30, we have two styles, a solid
    **	line "dash_style 0" and a style with one bit on and three off
    **	"dash_style 1". The first style will get one line (on the 76 octal, 60
    **	decimal line ) and the second style will get one line (on the 36 octal,
    **	30 decimal line). The first style displayed will be dash_style 0 and
    **	the second will be dash_style 1.
    */	  
    {(unsigned char) 30,
     (unsigned char) 2,
     (unsigned char *)"\1\1",
     (unsigned char *)"\74\36",
     (unsigned char *)"\0\1"},

    {(unsigned char) 20,
     (unsigned char) 2,
     (unsigned char *)"\1\2",
     (unsigned char *)"\74\24",
     (unsigned char *)"\0\1\1"},

    {(unsigned char) 15,
     (unsigned char) 3,
     (unsigned char *)"\1\1\2",
     (unsigned char *)"\74\36\17",
     (unsigned char *)"\0\2\1\2"},

    {(unsigned char) 12,
     (unsigned char) 2,
     (unsigned char *)"\1\4",
     (unsigned char *)"\74\14",
     (unsigned char *)"\0\1\1\1\1"},

    {(unsigned char) 10,
     (unsigned char) 3,
     (unsigned char *)"\1\1\4",
     (unsigned char *)"\74\36\12",
     (unsigned char *)"\0\2\2\1\2\2"},

    {(unsigned char) 6,
     (unsigned char) 3,
     (unsigned char *)"\1\1\10",
     (unsigned char *)"\74\36\06",
     (unsigned char *)"\0\2\2\2\2\1\2\2\2\2"},

    {(unsigned char) 5,
     (unsigned char) 4,
     (unsigned char *)"\1\1\2\10",
     (unsigned char *)"\74\36\17\05",
     (unsigned char *)"\0\3\3\2\3\3\1\3\3\2\3\3"},

    {(unsigned char) 4,
     (unsigned char) 3,
     (unsigned char *)"\1\2\14",
     (unsigned char *)"\74\24\04",
     (unsigned char *)"\0\2\2\2\2\1\2\2\2\2\1\2\2\2\2"},

    {(unsigned char) 3,
     (unsigned char) 4,
     (unsigned char *)"\1\1\2\20",
     (unsigned char *)"\74\36\17\03",
     (unsigned char *)"\0\3\3\3\3\2\3\3\3\3\1\3\3\3\3\2\3\3\3\3"},

    {(unsigned char) 2,
     (unsigned char) 4,
     (unsigned char *)"\1\1\4\30",
     (unsigned char *)"\74\36\12\02",
     (unsigned char *)"\0\3\3\3\3\2\3\3\3\3\2\3\3\3\3\1\3\3\3\3\2\3\3\3\3\2\3\3\3\3"},

    {(unsigned char) 1,
     (unsigned char) 5,
     (unsigned char *)"\1\1\2\10\60",
     (unsigned char *)"\74\36\17\05\01",
     (unsigned char *)"\0\4\4\4\4\3\4\4\4\4\3\4\4\4\4\2\4\4\4\4\3\4\4\4\4\3\4\4\4\4\1\4\4\4\4\3\4\4\4\4\3\4\4\4\4\2\4\4\4\4\3\4\4\4\4\3\4\4\4\4"}
};

extern void _XmBackgroundColorDefault();
extern void _XmForegroundColorDefault();

static XmPartResource resources [] = { 

   {DwcDswNpreferredDayslotsSize,   DwcDswCDayslotsSize,    XtRShort,
    sizeof (unsigned short int),    XmPartOffset (Dayslots, preferred_dayslots_size),
    XtRShort,			    (caddr_t) &default_dayslots_size},

   {DwcDswNpreferredFineIncrement,  DwcDswCFineIncrement,   XtRShort,
    sizeof (unsigned short int),    XmPartOffset (Dayslots, preferred_fine_increment),
    XtRShort,			    (caddr_t) &default_fine_increment},

   {DwcDswNonTheLine,		    DwcDswCOnTheLine,	    XtRBoolean,
    sizeof (Boolean),		    XmPartOffset (Dayslots, on_the_line),
    XtRBoolean,			    (caddr_t) &default_on_the_line},

   {DwcDswNtimeVerticalMargin,	    DwcDswCTimeMargin,	    XtRInt,
    sizeof (int),		    XmPartOffset (Dayslots, time_v_margin),
    XtRInt,			    (caddr_t) &default_v_margin},

   {DwcDswNtimeHorizontalMargin,    DwcDswCTimeMargin,	    XtRInt,
    sizeof (int),		    XmPartOffset (Dayslots, time_h_margin),
    XtRInt,			    (caddr_t) &default_h_margin},

   {DwcDswNstackTopDown,	    DwcDswCStackTopDown,    XtRBoolean,
    sizeof (Boolean),		    XmPartOffset (Dayslots, stack_top_down),
    XtRBoolean,			    (caddr_t) &default_stack},

   {DwcDswNindexForeground,	    XmCForeground,	    XmRPixel,
    sizeof (Pixel),		    XmPartOffset (Dayslots, index_foreground),
    XmRCallProc,		    (caddr_t) _XmForegroundColorDefault},

   {DwcDswNindexBackground,	    XmCBackground,	    XmRPixel,
    sizeof (Pixel),		    XmPartOffset (Dayslots, index_background_pixel),
    XmRCallProc,		    (caddr_t) _XmBackgroundColorDefault},

   {DwcDswNpixmaps,		    DwcDswCPixmaps,	    XtRPointer,
    sizeof (Pixmap *),		    XmPartOffset (Dayslots, pixmaps), 
    XtRPointer,			    (caddr_t) NULL},

   {DwcDswNpixmapHeight,	    DwcDswCPixmapHeight,    XtRDimension,
    sizeof (Dimension),		    XmPartOffset (Dayslots, pixmap_height), 
    XtRDimension,		    (caddr_t) &default_pixmap_height},

   {DwcDswNpixmapWidth,		    DwcDswCPixmapWidth,    XtRDimension,
    sizeof (Dimension),		    XmPartOffset (Dayslots, pixmap_width), 
    XtRDimension,		    (caddr_t) &default_pixmap_width},

   {DwcDswNdefaultIcons,	    DwcDswCDefaultIcons,    XtRPointer,
    sizeof (unsigned char *),	    XmPartOffset (Dayslots, default_icons), 
    XtRPointer,			    (caddr_t) NULL},

   {DwcDswNdefaultNumIcons,	    DwcDswCDefaultNumIcons, XtRInt,
    sizeof (Cardinal),		    XmPartOffset (Dayslots, default_num_icons), 
    XtRInt,			    (caddr_t) &default_zero},

   {DwcDswNtimeTexts,		    DwcDswCTimeTexts,	    XtRPointer,
    sizeof(XmString),		    XmPartOffset (Dayslots, time_texts), 
    XtRPointer,			    (caddr_t) NULL},

   {DwcDswNminuteTexts,		    DwcDswCMinuteTexts,	    XtRPointer,
    sizeof(XmString),		    XmPartOffset (Dayslots, minute_texts), 
    XtRPointer,			    (caddr_t) NULL},

   {DwcDswNeditable,		    DwcDswCEditable,	    XtRBoolean,
    sizeof (Boolean),		    XmPartOffset (Dayslots, editable), 
    XtRBoolean,			    (caddr_t) &default_editable},

   {DwcDswNdayNotes,		    DwcDswCDayNotes,	    XtRBoolean,
    sizeof (Boolean),		    XmPartOffset (Dayslots, is_day_note), 
    XtRBoolean,			    (caddr_t) &default_day_notes},

   {DwcDswNworkStart,		    DwcDswCWorkStart,	    XtRInt,
    sizeof (Cardinal),		    XmPartOffset (Dayslots, work_start), 
    XtRInt,			    (caddr_t) &default_work_start},

   {DwcDswNworkFinish,		    DwcDswCWorkFinish,	    XtRInt,
    sizeof (Cardinal),		    XmPartOffset (Dayslots, work_finish), 
    XtRInt,			    (caddr_t) &default_work_finish},

   {DwcDswNlunchStart,		    DwcDswCLunchStart,	    XtRInt,
    sizeof (Cardinal),		    XmPartOffset (Dayslots, lunch_start), 
    XtRInt,			    (caddr_t) &default_lunch_start},

   {DwcDswNlunchFinish,		    DwcDswCLunchFinish,	    XtRInt,
    sizeof (Cardinal),		    XmPartOffset (Dayslots, lunch_finish), 
    XtRInt,			    (caddr_t) &default_lunch_finish},

   {DwcDswNtimebarWidth,	    DwcDswCTimebarWidth,    XmRDimension,
    sizeof (Dimension),		    XmPartOffset (Dayslots, timebar_width), 
    XmRDimension,		    (caddr_t) &default_timebar_width},

   {DwcDswNwrkhrsTimebar,	    DwcDswCWrkhrsTimebar,  XtRPointer,
    sizeof (Widget),		    XmPartOffset (Dayslots, wrkhrs_timebar), 
    XtRPointer,			    (caddr_t) NULL},

   {DwcDswNgetTextCallback,	    XtCCallback,	    XtRCallback,
    sizeof (XtCallbackList),	    XmPartOffset (Dayslots, get_text_callback),
    XtRCallback,		    (caddr_t) NULL},

   {DwcDswNentryCallback,	    XtCCallback,	    XtRCallback,
    sizeof (XtCallbackList),	    XmPartOffset (Dayslots, entry_callback),
    XtRCallback,		    (caddr_t) NULL},

   {DwcDswNentryHelpCallback,	    XtCCallback,	    XtRCallback,
    sizeof (XtCallbackList),	    XmPartOffset (Dayslots, entry_help_callback),
    XtRCallback,		    (caddr_t) NULL},

   {DwcDswNscrollCallback,	    XtCCallback,	    XtRCallback,
    sizeof (XtCallbackList),	    XmPartOffset (Dayslots, scroll_callback),
    XtRCallback,		    (caddr_t) NULL},

   {DwcDswNfocusDisposeCallback,    XtCCallback,	    XtRCallback,
    sizeof (XtCallbackList),	    XmPartOffset (Dayslots, focus_dispose_callback),
    XtRCallback,		    (caddr_t) NULL},

   {DwcDswNfontList,		    XmCFontList,	    XmRFontList,
    sizeof(XmFontList),		    XmPartOffset(Dayslots, fontlist),
    XtRString,			    DXmDefaultFont}
};

/*
** Static initialization of the menu widget class record, must do each field
*/         

DayslotsClassRec dayslotsWidgetClassRec =
{
    /*
    ** Core Class record
    */
    {
	(WidgetClass) &xmManagerClassRec, /* superclass ptr		    */
	"Dayslots",			/* class_name			    */
	sizeof (DayslotsWidgetRec),	/* size of dayslots widget instance */
	ClassInitialize,		/* class init proc		    */
	NULL,				/* Class Part Initialize	    */
	FALSE,				/* Class is not initialised	    */
	(XtInitProc)Initialize,		/* Widget init proc		    */
	NULL,				/* Initialise hook		    */
	Realize,			/* Widget realise proc		    */
	action_table,			/* Class Action Table		    */
	XtNumber (action_table),
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
	(XtSetValuesFunc)SetValues,	/* class set_value proc		    */
	NULL,				/* set values hook		    */
	XtInheritSetValuesAlmost,	/* set values almost		    */
	NULL,				/* get values hook		    */
	AcceptFocus,			/* class accept focus proc	    */
	XtVersion,			/* version			    */
	NULL,				/* Callback Offsets		    */
	translations,			/* Tm_table			    */
	NULL,				/* disp accelerators		    */	
	NULL				/* extension			    */ 
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
	NULL				/* extension          */ 
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
    ** Dayslots Class record
    */
    {
	NULL,
	0				/* just a dummy field		    */
    }
};

/*
**  static widget class pointer
*/

WidgetClass dayslotsWidgetClass = (WidgetClass) &dayslotsWidgetClassRec;


Widget DSWDayslotsCreate
#if defined(_DWC_PROTO_)
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

#if defined(DEBUG)
    printf ("DayslotsWidget -- DSWDayslotsCreate Called\n");
#endif

    return (XtCreateWidget (name, dayslotsWidgetClass, parent,
			    arglist, argcount));

}

Widget DwcDayslots
#if defined(_DWC_PROTO_)
	(
	Widget			parent,
	char			*name,
	unsigned short int	dayslots_size,
	Position		y,
	Dimension		width,
	XtCallbackList		help_callback)
#else	/* no prototypes */
	(parent, name, dayslots_size, y, width, help_callback)
	Widget			parent;
	char			*name;
	unsigned short int	dayslots_size;
	Position		y;
	Dimension		width;
	XtCallbackList		help_callback;
#endif	/* prototype */
    {
    Arg			arglist [10];
    Cardinal		ac;

#if defined(DEBUG)
    printf ("DayslotsWidget -- DwcDayslots Called\n");
#endif

    ac = 0;
    XtSetArg(arglist[ac], XtNy, y); ac++;
    XtSetArg(arglist[ac], XtNwidth, width); ac++;
    XtSetArg(arglist[ac], DwcDswNpreferredDayslotsSize, dayslots_size); ac++;
    XtSetArg(arglist[ac], XmNhelpCallback, help_callback); ac++;
    assert (ac <= XtNumber(arglist));

    return (XtCreateWidget (name, dayslotsWidgetClass, parent, arglist, ac));

}

static void ClassInitialize
#if defined(_DWC_PROTO_)
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{
    long	byte_count, cvt_status;

#if defined(DEBUG)
    printf ("DayslotsWidget -- ClassInitialize Called\n");
#endif

    XmResolvePartOffsets
    (
	dayslotsWidgetClass,
	&dayslotsWidgetClassRec.dayslots_class.dayslotsoffsets
    );

    zero_length_string = DXmCvtFCtoCS ("", &byte_count, &cvt_status);

}

static void
get_default_timeslot_dimensions
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
{
    Widget	    timeslot;
    Window	    pm_root;
    int		    pm_x, pm_y;
    unsigned int    pm_border, pm_depth;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    Arg		    arglist[5];
 

    /*	  
    **  Go create the timeslot
    */	  
    timeslot = create_timeslot
    (
	dsw,
	0,		    /* y position */
	0,		    /* height */
	NULL,		    /* icons */
	0,		    /* num_icons */
	DswEditable(dsw, o)
    );

    /*	  
    **  If the parent dsw widget is realized then realize the timeslot too
    */	  
    if (XtIsRealized((Widget) dsw))
    {
	XtRealizeWidget (timeslot);
    }


    /*	  
    **  Figure out how high the timeslot is and save that
    */	  
    DswTimeslotMinHeight(dsw, o) = XtHeight(timeslot);


    /*	  
    **  Throw this timeslot on our stack of spare timeslots
    */	  
    DswSpareTimeslots(dsw, o) = (Widget *) XtRealloc
    (
	(char *)DswSpareTimeslots(dsw, o),
	sizeof (Widget *) * (DswNumSpareTimeslots(dsw, o) + 1)
    );

    DswSpareTimeslots(dsw, o) [DswNumSpareTimeslots(dsw, o)] = timeslot;
    DswNumSpareTimeslots(dsw, o)++;

}

static void Initialize
#if defined(_DWC_PROTO_)
	(
	Widget	request,
	Widget	new)
#else	/* no prototypes */
	(request, new)
	Widget	request;
	Widget	new;
#endif	/* prototype */
    {
    unsigned char   *icons;
    Cardinal	    size;
    Cardinal	    i;
    DayslotsWidget  dsw = (DayslotsWidget)new;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    Arg		    arglist[30];
    Cardinal	    ac;


#if defined(DEBUG)
    printf ("DayslotsWidget -- Initialize Called\n");
#endif

    
    DswGrayPixmap(dsw,o) = MISCXtGrayPixmap(XtScreen(dsw));

    /*	  
    **  Go create the foreground and background gcs
    */	  
    create_fore_back_gcs(dsw);

    DswMbaContext(dsw, o) = MBAInitContext(new, TRUE, TRUE, start_invert_motion);

    DswRealTimeTexts    (dsw, o) = DswTimeTexts(dsw, o);
    DswTimeTexts	(dsw, o) = (XmString *) &(DswTimeTexts(dsw, o));

    DswRealMinuteTexts  (dsw, o) = DswMinuteTexts(dsw, o);
    DswMinuteTexts	(dsw, o) = (XmString *) &(DswMinuteTexts(dsw, o));

    /*	  
    **  If a dayslot then we need to set time and minute stuff
    */	  
    if (! DswIsDayNote(dsw, o)) {
	DswTimeWidths(dsw, o) = (Dimension *)
	  XtCalloc ((24 * 60) + 1, sizeof (Dimension));
	DswMinuteWidths(dsw, o) = (Dimension *)
	  XtCalloc (60,            sizeof (Dimension));
    }

    if (DswDefaultNumIcons(dsw, o) == 0) {
	DswDefaultIcons(dsw, o)  = NULL;
    } else {
	size  = sizeof (unsigned char) * DswDefaultNumIcons(dsw, o);
	icons = (unsigned char *) XtMalloc (size);
	memcpy (icons, DswDefaultIcons(dsw, o), size);
	DswDefaultIcons(dsw, o) = icons;
    }
    
    DswSelected        (dsw, o) = NULL;
    DswOpenEntry       (dsw, o) = NULL;
    DswDragEntry       (dsw, o) = NULL;

    /*	  
    **  Indicate that nothing is inverted
    */	  
    DswStartTimeCurrInvert     (dsw, o) = - 1;
    DswEndTimeCurrInvert       (dsw, o) = - 1;

    DswExposeRegion    (dsw, o) = XCreateRegion ();
    DswNoexposeRegion  (dsw, o) = NULL;

    DswOutline         (dsw, o) = FALSE;
    DswOutlineVisible  (dsw, o) = FALSE;
    DswOutlineDrawable (dsw, o) = TRUE;
    DswOutlineEventSeq (dsw, o) = 1;
    DswOutlineEventLast(dsw, o) = 0;

    DswNumEntries      (dsw, o) = 0;
    DswEntries         (dsw, o) = NULL;
    
    DswNumSpareTimeslots(dsw, o) = 0;
    DswSpareTimeslots  (dsw, o) = NULL;

    /*
    **  Set these temporarily to reasonable numbers so that windows can get
    **  created...
    */
    DswTimeSlotX   (dsw, o) = 0;
    DswTimeSlotWidth(dsw, o) = 100;

    get_default_timeslot_dimensions(dsw);

    DswSlotLineSegments(dsw, o) = NULL;
    DswSlotLineIndex   (dsw, o) = 0;

    DswIndexTime       (dsw, o) = - 1;

    DswAutoScrollTimer (dsw, o) = (XtIntervalId) 0;
    DswAutoScrollState (dsw, o) = DswASSNotScrolling;
    DswWait2ndClickTimer(dsw, o) = (XtIntervalId) 0;

    DswTimeslotSequence(dsw, o) = 0;

    if (DswIsDayNote(dsw, o)) {
	DswSlotLineGCs(dsw, o) = (GC *) XtMalloc (sizeof (GC));
	DswSlotLineGCs(dsw, o) [0] = NULL;
    } else {
	DswSlotLineGCs(dsw, o) =
	  (GC *) XtMalloc (sizeof (GC) * No_of_Dayslot_Styles);

	for (i = 0;  i < No_of_Dayslot_Styles;  i++) {
	    DswSlotLineGCs(dsw, o) [i] = NULL;
	}
    }
    
    DswIndexForegroundGC(dsw, o) = NULL;
    DswIndexBackgroundGC(dsw, o) = NULL;
    DswInvertGC(dsw, o)          = NULL;
    DswOutlineGC(dsw, o)         = NULL;

    create_index_gcs(dsw);

    setup_dayslots_display (dsw, TRUE, FALSE);

    arrange_timeslots (dsw, NULL, MISCGetTimeFromEvent(NULL));

}

static void
create_index_gcs
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XGCValues	    gcv;
    XmOffsetPtr	    o  = DswOffsetPtr(dsw);
    XFontStruct	    *font;

    /*	  
    **  Don't bother if this is a daynote
    */	  
    if (DswIsDayNote(dsw, o)) {
	return;
    }
    
    if (DswIndexForegroundGC(dsw, o) != NULL)
    {
	XtReleaseGC ((Widget) dsw, DswIndexForegroundGC(dsw, o));
    }
    
    gcv.function   = GXcopy;

    gcv.foreground = DswIndexForegroundPixel(dsw, o);
    gcv.background = DswIndexBackgroundPixel(dsw, o);
    MISCGetFontFromFontlist(DswFontList(dsw,o), &font);    
    gcv.font       = font->fid;

    DswIndexForegroundGC(dsw, o) = XtGetGC
	((Widget) dsw, GCFunction | GCForeground | GCBackground | GCFont, &gcv);


    if (DswIndexBackgroundGC(dsw, o) != NULL)
    {
	XtReleaseGC ((Widget) dsw, DswIndexBackgroundGC(dsw, o));
    }

    gcv.background = DswIndexForegroundPixel(dsw, o);
    gcv.foreground = DswIndexBackgroundPixel(dsw, o);

    DswIndexBackgroundGC(dsw, o) = XtGetGC
	((Widget) dsw, GCFunction | GCForeground | GCBackground, &gcv);

}

static void
create_dayslots_gcs
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    Pixmap		pixmap;
    XGCValues		gcv;
    Cardinal		i, l;
    Cardinal		no_of_styles;
    XmOffsetPtr		o  = DswOffsetPtr(dsw);
    Display		*d = XtDisplay(dsw);
    Window		w  = XtWindow (dsw);
    static const char	bitmap_data [4] = {017, 015, 017, 017};
    int			bm_w = 4;
    int			bm_h = 4;


    if (DswInvertGC(dsw, o) != NULL)
    {
	XFreeGC (d, DswInvertGC(dsw, o));
    }

    pixmap = XCreateBitmapFromData (d, w, (char *)bitmap_data, bm_w, bm_h);

    gcv.foreground = DswForegroundPixel(dsw, o);
    gcv.background = DswBackgroundPixel(dsw, o);
    gcv.plane_mask = gcv.foreground ^ gcv.background;
    gcv.stipple    = pixmap;
    gcv.fill_style = FillStippled;
    gcv.function   = GXinvert;

    DswInvertGC(dsw, o) = XCreateGC
	(d, w, GCFunction | GCPlaneMask | GCStipple | GCFillStyle, &gcv);

    XFreePixmap (d, pixmap);
    
    if (DswIsDayNote(dsw, o))
    {
	no_of_styles = 1;
    }
    else
    {
	no_of_styles = No_of_Dayslot_Styles;

	if (DswOutlineGC(dsw, o) != NULL)
	{
	    XFreeGC (d, DswOutlineGC(dsw, o));
	}

	gcv.subwindow_mode = IncludeInferiors;
	gcv.line_width     = 2;
	
	gcv.function = GXinvert;
	DswOutlineGC(dsw, o) = XCreateGC (d, w,
	     GCFunction | GCLineWidth | GCSubwindowMode | GCPlaneMask, &gcv);
    }


    gcv.function = GXcopy;
    for (i = 0;  i < no_of_styles;  i++)
    {

	if (DswSlotLineGCs(dsw, o) [i] != NULL)
	{
	    XFreeGC (d, DswSlotLineGCs(dsw, o) [i]);
	}

	l = strlen ((char *)dayslot_styles_dashes[i]);

	gcv.line_style = (l == 0) ? LineSolid : LineOnOffDash;

	DswSlotLineGCs(dsw, o) [i] =
	  XCreateGC (d, w, GCFunction | GCForeground | GCLineStyle, &gcv);

	if (l != 0)
	{
	    XSetDashes
	    (
		d,
		DswSlotLineGCs(dsw, o)[i],
		0,
		(char *)dayslot_styles_dashes[i],
		l
	    );
	}

    }

}

static void
setup_dayslots_positions
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			font_width;
    int			timebar_space;
    XFontStruct		*font;
     
    MISCGetFontFromFontlist(DswFontList(dsw, o), &font);
    font_width = (font->max_bounds.rbearing -
		  font->min_bounds.lbearing) / 4;
    if (DswIsDayNote(dsw, o)) {
	timebar_space = - 1;
    } else {
	timebar_space = font_width * DswTimebarWidth(dsw, o);
	if (DswTimebarWidth(dsw, o) != 0) {
	    timebar_space = MAX (timebar_space, 3);
	}
    }

    if (timebar_space > 2) {
	DswRealTimebarWidth(dsw, o) = timebar_space - 2;
    } else {
	DswRealTimebarWidth(dsw, o) = 0;
	timebar_space = - 1;
	DswTimebarX(dsw, o) = - 1;
    }

    if (! DswDirectionRToL(dsw, o)) {
	if (timebar_space > 0) {
	    DswTimebarX (dsw, o) = 1;
	}
	DswDividingLine1X(dsw, o) = timebar_space;
	DswTimeLabelX   (dsw, o) = DswDividingLine1X(dsw, o) + 2;
	DswDividingLine2X(dsw, o) = DswTimeLabelX(dsw, o) +
				      DswTimeLabelWidth(dsw, o);
	DswTimeSlotX    (dsw, o) = DswDividingLine2X(dsw, o) + 1;
	DswTimeSlotWidth(dsw, o) = XtWidth(dsw) - DswTimeSlotX(dsw, o);
    } else {
	if (timebar_space > 0) {
	    DswTimebarX (dsw, o) = XtWidth(dsw) - 2 -
				      DswRealTimebarWidth(dsw, o);
	}
	DswDividingLine1X(dsw, o) = XtWidth(dsw) - timebar_space - 2;
	DswTimeLabelX   (dsw, o) = DswDividingLine1X(dsw, o) -
				      DswTimeLabelWidth(dsw, o);
	DswDividingLine2X(dsw, o) = DswTimeLabelX(dsw, o) - 2;
	DswTimeSlotX    (dsw, o) = 0;
	DswTimeSlotWidth(dsw, o) = DswDividingLine2X(dsw, o);
    }

}

static void
setup_dayslots_display
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Boolean		geometry_direct,
	Boolean		reset_widths)
#else	/* no prototypes */
	(dsw, geometry_direct, reset_widths)
	DayslotsWidget	dsw;
	Boolean		geometry_direct;
	Boolean		reset_widths;
#endif	/* prototype */
    {
    XtWidgetGeometry	request;
    XtGeometryResult	result;
    int			label_height;
    int			label_height_multiple;
    int			time;
    Dimension		height;
    Position		y;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			slot_size      = DswPreferredDayslotsSize(dsw, o);
    int			fine_increment = DswPreferredFineIncrement(dsw, o);
    XFontStruct		*font;

    /*
    **  Setup default widget size only if doing direct geometry...
    */
    if (geometry_direct) {
	if (XtWidth(dsw) == 0) {
	    XtWidth(dsw) = XtWidth (XtParent(dsw));
	}
    } else {
	time = MAX (0, translate_y_to_time(dsw, - XtY(dsw), FALSE));
    }
    
    if (DswIsDayNote(dsw, o)) {
	DswTimeMaxWidth(dsw, o) = DswPixmapWidth(dsw, o);
	DswActualDayslotsSize(dsw, o) = slot_size      = 24 * 60;
	DswActualFineIncrement(dsw, o) = fine_increment = 24 * 60;
    } else {

	/*
	**  Determine actual dayslots size we will use.
	*/
	if ((slot_size > 60) || (slot_size <= 0) || ((60 % slot_size) != 0))
	{
	    slot_size = default_dayslots_size;
	    XtAppWarning
	    (
		XtWidgetToApplicationContext((Widget) dsw),
		"Bad dayslots size given -- using default\n"
	    );
	}
	DswActualDayslotsSize(dsw, o) = slot_size;

	/*
	**  Determine actual fine increment.
	*/
	if ((fine_increment > 60) ||
	    (fine_increment <= 0) ||
	    ((slot_size % fine_increment) != 0))
	{
	    fine_increment = default_fine_increment;
	    XtAppWarning
	    (
		XtWidgetToApplicationContext((Widget) dsw),
		"Bad fine increment size given -- using default\n"
	    );
	}
	DswActualFineIncrement(dsw, o) = fine_increment;

	/*
	**  Get the width of the times provided
	*/
	get_timetext_widths (dsw, reset_widths);
    }

    /*
    **  Get the time labels' width and height.
    */
    MISCGetFontFromFontlist(DswFontList(dsw, o), &font);
    DswTimeLabelWidth(dsw, o) =
     DswTimeMaxWidth(dsw, o) + 4 +
      ((DswTimeHMargin(dsw, o) *
        (font->max_bounds.rbearing -
         font->min_bounds.lbearing)) / 4);

    label_height_multiple = slot_size / fine_increment;

    label_height = MAX (font->max_bounds.ascent,
			font->ascent)  +
		   MAX (font->max_bounds.descent,
		        font->descent) + 1;

    label_height = MAX (label_height, DswTimeslotMinHeight(dsw, o) + 1);

    label_height = label_height_multiple *
		   (((label_height - 1) / label_height_multiple) + 1);

    DswTimeLabelHeight(dsw, o) =
      label_height + (DswTimeVMargin(dsw, o) * label_height_multiple);

    /*
    **  Determine the top margin and the time slot width.  Then calculate
    **	various x positions depending upon direction.
    */
    
    if (DswIsDayNote(dsw, o)) {
	DswTopMargin(dsw, o) = - 1;
    } else {
	DswTopMargin(dsw, o) = DswTimeLabelHeight(dsw, o) / 2;
    }
    setup_dayslots_positions(dsw);
    
    /*
    **  Go setup the structures for our timeslot lines.
    */
    
    if (! DswIsDayNote(dsw, o))
    {
	setup_slot_lines(dsw);
    }

    /*
    **  Determine our height, either directly or by request....
    */
    
    if (DswIsDayNote(dsw, o)) {
	height = DswTimeLabelHeight(dsw, o);
    } else {
	height = ((24 * 60 / DswActualDayslotsSize(dsw, o)) + 1) *
		 DswTimeLabelHeight(dsw, o);
    }


    if (geometry_direct)
    {
	XtHeight(dsw) = height;
        if (! DswIsDayNote(dsw, o))
	{
	    DswTimebarGidget(dsw, o) = TBWCreateGidget
	    (
		(Widget) dsw,
		DswTimebarX(dsw, o),
		DswTopMargin(dsw, o) + 1,
		DswRealTimebarWidth(dsw, o),
		height - (DswTopMargin(dsw, o) * 2),
		DswForegroundPixel(dsw,o)
	    );
	}
    }
    else
    {
	if (! DswIsDayNote(dsw, o))
	{
	    TBWResizeGidget
	    (
		(Widget) dsw,
		DswTimebarGidget(dsw, o),
		DswTimebarX(dsw, o),
		DswTopMargin(dsw, o) + 1,
		DswRealTimebarWidth(dsw, o),
		height - (DswTopMargin(dsw, o) * 2)
	    );
	}
	request.request_mode = CWHeight;
	request.height	     = height;
	result = XtMakeGeometryRequest ((Widget) dsw, &request, NULL);
	if (! DswIsDayNote(dsw, o))
	{
	    DSWMoveDayslotsToTime (dsw, time);
	}
    }

}

static void move_index_time
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	int		t)
#else	/* no prototypes */
	(dsw, t)
	DayslotsWidget	dsw;
	int		t;
#endif	/* prototype */
    { 
    Position		new_y;
    int			time;
    XRectangle		expose_area;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			old_index_time = DswIndexTime(dsw, o);
    int			fine_inc = DswActualFineIncrement(dsw, o);

    time = t;
    if ((old_index_time == time) ||
        ((old_index_time >= 0) && (time >= 0) && (time % fine_inc != 0))) {
	return;
    }

    DswIndexTime(dsw, o) = time;
    
    if (old_index_time >= 0) {

	expose_area.x = DswTimeLabelX(dsw, o);
	expose_area.y = IndexYPositionForTime (old_index_time, dsw);
	expose_area.width  = DswTimeLabelWidth(dsw, o) - 1;
	expose_area.height = DswTimeLabelHeight(dsw, o) - 1;

	if (time >= 0) {

	    new_y = IndexYPositionForTime (time, dsw);

	    if ((new_y > expose_area.y) &&
	        (new_y < expose_area.y + expose_area.height)) {

		expose_area.height = new_y - expose_area.y;

	    } else {

		if ((expose_area.y > new_y) &&
		    (expose_area.y < new_y + DswTimeLabelHeight(dsw, o) - 2)) {
		    expose_area.height = expose_area.y - new_y;
		    expose_area.y = new_y + DswTimeLabelHeight(dsw, o) - 1;
		}

	    }
	}

	display_dayslots (dsw, &expose_area, NULL, TRUE, FALSE);
    }

    if (time >= 0 && (time != old_index_time)) {
	display_index_time (dsw, NULL);
    }

}

static void Realize
#if defined(_DWC_PROTO_)
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
    DayslotsWidget	    dsw = (DayslotsWidget) w;

#if defined(DEBUG)
    printf ("DayslotsWidget -- Realize Called\n");
#endif

    XtCreateWindow
	(w, InputOutput, CopyFromParent, *window_mask, window_attributes);

    create_dayslots_gcs (dsw);

    /* Set this up to receive ClientMessages */
    XtAddEventHandler
    (
	(Widget) dsw,
	NoEventMask,
	TRUE,
	(XtEventHandler) receive_client_message,
	NULL
    );

}

static Boolean AcceptFocus
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	Time	*time)
#else	/* no prototypes */
	(w, time)
	Widget	w;
	Time	*time;
#endif	/* prototype */
{
    DayslotsWidget  dsw   = (DayslotsWidget) w;
    XmOffsetPtr    o     = DswOffsetPtr(dsw);
    DwcDswEntry	    entry = DswOpenEntry(dsw, o);	    

    if (entry == NULL)
    {
	return (False);
    }
    else
    {
	return (True);
    }
}

static void
Destroy
#if defined(_DWC_PROTO_)
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    int		    	index;
    int			number_of_styles;
    int			i;
    DayslotsWidget	dsw = (DayslotsWidget) w;
    XmOffsetPtr		o   = DswOffsetPtr(dsw);
    Display		*d  = XtDisplay(dsw);
    
#if defined(DEBUG)
    printf ("DayslotsWidget -- Destroy Called\n");
#endif

    if (! DswIsDayNote(dsw, o))
    {
	TBWDestroyGidget ((Widget) dsw, DswTimebarGidget(dsw, o));
    }

    MBAFreeContext(DswMbaContext(dsw, o));
    
    XtRemoveAllCallbacks (w, DwcDswNgetTextCallback);
    XtRemoveAllCallbacks (w, DwcDswNentryCallback);
    XtRemoveAllCallbacks (w, DwcDswNentryHelpCallback);
    XtRemoveAllCallbacks (w, DwcDswNscrollCallback);

    if (! DswIsDayNote(dsw, o))
    {
	XtFree ((char *) DswTimeWidths (dsw, o));
	XtFree ((char *) DswMinuteWidths(dsw, o));
    }


    for (i = 0;  i < DswNumSpareTimeslots(dsw, o);  i++)
    {
	XtFree ((char *) DswSpareTimeslots(dsw, o) [i]);
    }
    XtFree ((char *) DswSpareTimeslots(dsw, o));


    for (i = 0;  i < DswNumEntries(dsw, o);  i++)
    {
	XtFree ((char *) DswEntries(dsw, o) [i]->icons);
	XtFree ((char *) DswEntries(dsw, o) [i]);
    }
    XtFree ((char *) DswEntries(dsw, o));


    if (DswIsDayNote(dsw, o))
    {
	number_of_styles = 1;
    }
    else
    {
	if (DswSlotLineSegments(dsw, o) != NULL)
	{
	    index = DswSlotLineIndex(dsw, o);
	    number_of_styles = slot_lines [index].number_of_styles;
	    for (i = 0;  i < number_of_styles;  i++)
	    {
		XtFree ((char *) DswSlotLineSegments(dsw, o) [i]);
	    }
	    XtFree ((char *) DswSlotLineSegments(dsw, o));
	}

	number_of_styles = No_of_Dayslot_Styles;
    }
    
    for (i = 0;  i < number_of_styles;  i++)
    {
	if (DswSlotLineGCs(dsw, o) [i] != NULL)
	{
	    XFreeGC (d, DswSlotLineGCs(dsw, o) [i]);;
	}
    }
    XtFree ((char *) DswSlotLineGCs(dsw, o));

    if (DswInvertGC(dsw, o) != NULL)
    {
	XFreeGC (d, DswInvertGC(dsw, o));
    }
    
    if (! DswIsDayNote(dsw, o))
    {
	if (DswIndexForegroundGC(dsw, o) != NULL)
	{
	    XtReleaseGC ((Widget) dsw, DswIndexForegroundGC(dsw, o));
	}
	
	if (DswIndexBackgroundGC(dsw, o) != NULL)
	{
	    XtReleaseGC ((Widget) dsw, DswIndexBackgroundGC(dsw, o));
	}
    }    

}

static Boolean SetValues
#if defined(_DWC_PROTO_)
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
    unsigned char	*icons;
    Cardinal		size;
    Arg			arglist [10];
    Cardinal		ac;
    Pixmap		pixmaps[32];	/* the maximum # in DB is 32 */
    Cardinal		i;
    DwcDswEntry		entry;
    DayslotsWidget	old = (DayslotsWidget) o;
    DayslotsWidget	new = (DayslotsWidget) n;
    XmOffsetPtr		oo = DswOffsetPtr (old);
    XmOffsetPtr		no = DswOffsetPtr (new);
    Boolean		texts_changed = FALSE;
    Boolean		redisplay = FALSE;

#if defined(DEBUG)
    printf ("DayslotsWidget -- SetValues Called\n");
#endif

    MISCUpdateCallback
    (
	o,
	&(DswGetTextCallback (old, oo)),
	n,
	&(DswGetTextCallback (new, no)),
	DwcDswNgetTextCallback
    );

    MISCUpdateCallback
    (
	o,
	&(DswScrollCallback (old, oo)),
	n,
	&(DswScrollCallback (new, no)),
	DwcDswNscrollCallback
    );

    MISCUpdateCallback
    (
	o,
	&(DswEntryCallback (old, oo)),
	n,
	&(DswEntryCallback (new, no)),
	DwcDswNentryCallback
    );

    MISCUpdateCallback
    (
	o,
	&(DswEntryHelpCallback (old, oo)),
	n,
	&(DswEntryHelpCallback (new, no)),
	DwcDswNentryHelpCallback
    );


    if ((DswDefaultNumIcons (old, oo) != DswDefaultNumIcons (new, no)) ||
        (DswDefaultIcons    (old, oo) != DswDefaultIcons    (new, no)))
    {

	if (DswDefaultNumIcons (new, no) == 0)
	{
	    DswDefaultIcons (new, no) = NULL;
	}
	else
	{
	    size  = sizeof (unsigned char) * DswDefaultNumIcons (new, no);
	    icons = (unsigned char *) XtMalloc (size);
	    memcpy (icons, DswDefaultIcons (new, no), size);
	    DswDefaultIcons (new, no) = icons;
	}
	XtFree ((char *) DswDefaultIcons (old, oo));
	if (DswIsDayNote (new, no))
	{
	    redisplay = TRUE;
	}
    }

    if ((DswIndexForegroundPixel(old, oo)!=DswIndexForegroundPixel(new, no)) ||
        (DswIndexBackgroundPixel(old, oo)!=DswIndexBackgroundPixel(new, no)) ||
	(DswFontList            (old, oo)!=DswFontList            (new, no)))
    {

	create_index_gcs (new);
	redisplay = redisplay || (DswIndexTime (new, no) >= 0);
    }
    

    if ((DswForegroundPixel (old, oo) != DswForegroundPixel (new, no)) ||
        (DswBackgroundPixel (old, oo) != DswBackgroundPixel (new, no)) ||
	(DswFontList        (old, oo) != DswFontList        (new, no)))
    {

	if (XtIsRealized ((Widget) new))
	{
	    create_fore_back_gcs((DayslotsWidget)new);
	    create_dayslots_gcs (new);
	    redisplay = TRUE;
	}
	if (! DswIsDayNote (new, no)) {
		TBWSetTilesAndStipples(DswWrkhrsTimebar(new,no));
	}
    }


    if ((DswPixmaps  (old, oo) != DswPixmaps  (new, no))  ||
        (DswFontList (old, oo) != DswFontList (new, no))) {

        /*	  
	**  Go figure out default timeslot dimensions
	*/	  
        get_default_timeslot_dimensions(new);

	for (i = 0;  i < DswNumEntries (new, no);  i++)
	{
	    entry = DswEntries (new, no) [i];

	    get_icons_pixmaps (new, entry->icons, entry->num_icons, pixmaps);

	    XtVaSetValues
	    (
		entry->timeslot,
		DwcTswNpixmapWidth, DswPixmapWidth(new, no),
		DwcTswNpixmapHeight, DswPixmapHeight(new,no),
		DwcTswNpixmaps, pixmaps,
		DwcTswNpixmapCount, entry->num_icons,
		DwcTswNfontList, DswFontList(new, no),
		NULL
	    );
	}
    }


    if (DswDirectionRToL (old, oo) != DswDirectionRToL (new, no)) {
	for (i = 0;  i < DswNumEntries (new, no);  i++) {
	    DswEntries (new, no) [i]->geometry_changed = TRUE;
            /*	  
	    **  Set the direction on the timeslot
	    */
	    XtVaSetValues
	    (
		DswEntries(new,no)[i]->timeslot,
		XmNstringDirection, DswDirectionRToL(new,no),
		NULL
	    );
	}
    }
    
    if (DswTimeTexts (old, oo) != DswTimeTexts (new, no)) {
	DswRealTimeTexts (new, no) = DswTimeTexts (new, no);
	texts_changed = TRUE;
    }

    if (DswMinuteTexts (old, oo) != DswMinuteTexts (new, no)) {
	DswRealMinuteTexts (new, no) = DswMinuteTexts (new, no);
	texts_changed = TRUE;
    }

    DswTimeTexts   (new, no) = (XmString *) &(DswTimeTexts   (new, no));
    DswMinuteTexts (new, no) = (XmString *) &(DswMinuteTexts (new, no));

    texts_changed =
     texts_changed || (DswFontList (old, oo) != DswFontList (new, no));

    if ((DswTimebarWidth (old, oo) != DswTimebarWidth (new, no)) ||
        (DswFontList     (old, oo) != DswFontList (new, no))) {
	if (DswWrkhrsTimebar (new, no) != NULL)
	    {
	    ac = 0;
	    XtSetArg(arglist[ac], XtNwidth, DswTimebarWidth(new, no)); ac++;
	    XtSetArg(arglist[ac], DwcTbwNfontList, DswFontList(new, no)); ac++;
	    assert (ac <= XtNumber(arglist));
	    XtSetValues (DswWrkhrsTimebar (new, no), arglist, ac);
	    }
    }

    if ((DswPreferredDayslotsSize  (old, oo) != DswPreferredDayslotsSize  (new, no)) ||
	(DswPreferredFineIncrement (old, oo) != DswPreferredFineIncrement (new, no)) ||
	(DswOnTheLine              (old, oo) != DswOnTheLine              (new, no)) ||
	(DswDirectionRToL          (old, oo) != DswDirectionRToL          (new, no)) ||
	(DswStackTopDown           (old, oo) != DswStackTopDown           (new, no)) ||
	(DswTimeVMargin            (old, oo) != DswTimeVMargin            (new, no)) ||
	(DswTimeHMargin            (old, oo) != DswTimeHMargin            (new, no)) ||
	(DswPixmapWidth            (old, oo) != DswPixmapWidth            (new, no)) ||
	(DswPixmapHeight           (old, oo) != DswPixmapHeight           (new, no)) ||
	(DswTimeslotMinHeight      (old, oo) != DswTimeslotMinHeight      (new, no)) ||
	(DswWorkStart              (old, oo) != DswWorkStart              (new, no)) ||
	(DswWorkFinish             (old, oo) != DswWorkFinish             (new, no)) ||
	(DswLunchStart             (old, oo) != DswLunchStart             (new, no)) ||
	(DswLunchFinish            (old, oo) != DswLunchFinish            (new, no)) ||
	(DswTimebarWidth           (old, oo) != DswTimebarWidth           (new, no)) ||
	(texts_changed)) {

	setup_dayslots_display (new, FALSE, texts_changed);

	arrange_timeslots
	    (new, DswOpenEntry (new, no), MISCGetTimeFromEvent(NULL));

	redisplay = TRUE;
    }


    return (redisplay && XtIsRealized ((Widget) new));

}

static void Resize
#if defined(_DWC_PROTO_)
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    DayslotsWidget  dsw  = (DayslotsWidget) w;
    XmOffsetPtr    o    = DswOffsetPtr(dsw);


    setup_dayslots_positions(dsw);

    if (! DswIsDayNote(dsw, o))
    {
	setup_slot_lines(dsw);

	TBWResizeGidget
	(
	    (Widget) dsw,
	    DswTimebarGidget(dsw, o),
	    DswTimebarX(dsw, o),
	    DswTopMargin(dsw, o) + 1,
	    DswRealTimebarWidth(dsw, o),
	    XtHeight(dsw) - (DswTopMargin(dsw, o) * 2)
	);
    }
    
    arrange_timeslots
	(dsw, DswOpenEntry(dsw, o), MISCGetTimeFromEvent(NULL));

}

static void quick_copy_or_move
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	int		reason,
	XButtonEvent	*event)
#else	/* no prototypes */
	(dsw, reason, event)
	DayslotsWidget	dsw;
	int		reason;
	XButtonEvent	*event;
#endif	/* prototype */
    {
    DSWEntryCallbackStruct    ecb;
    int			entry_start_time;
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    DswTimeslotSequence(dsw, o) ++;

    entry_start_time = translate_y_to_time(dsw, event->y, TRUE);
    if ((entry_start_time < 0) || (entry_start_time > 24 * 60)) {
	return;
    }

    entry_start_time = MIN (entry_start_time, (24 * 60) - DswActualDayslotsSize(dsw, o));

    ecb.reason       = reason;
    ecb.text_changed = FALSE;
    ecb.time_changed = FALSE;
    
    if (DswIsDayNote(dsw, o)) {
	ecb.start    = 0;
	ecb.duration = 0;
    } else {
	ecb.start    = entry_start_time;
	ecb.duration = DswActualDayslotsSize(dsw, o);
    }

    ecb.time	 = MISCGetTimeFromEvent ((XEvent *) event);
    ecb.tag	 = NULL;
    ecb.entry	 = NULL;
    ecb.event	 = (XEvent *) event;

    XtCallCallbackList ((Widget) dsw, DswEntryCallback(dsw,o), &ecb);
    
}

#if !defined (AUD)
static
#endif
void ACTION_DSW_QUICK_COPY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	XEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget	w;
	XEvent	*event;
#endif	/* prototype */
    {
    DayslotsWidget	dsw = (DayslotsWidget) w;

    quick_copy_or_move(dsw, DwcDswCREntryQuickCopy, (XButtonEvent *) event);
    
}

#if !defined (AUD)
static
#endif
void ACTION_DSW_QUICK_MOVE
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	XEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget	w;
	XEvent	*event;
#endif	/* prototype */
    {
    DayslotsWidget	dsw = (DayslotsWidget) w;

    quick_copy_or_move(dsw, DwcDswCREntryQuickMove, (XButtonEvent *) event);
    
}

static void TIMESLOT_HELP
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
{
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    XtCallCallbackList ((Widget) dsw, DswEntryHelpCallback(dsw,o), NULL);

}

#if !defined (AUD)
static
#endif
void ACTION_DSW_HELP
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	XEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget	w;
	XEvent	*event;
#endif	/* prototype */
{
    XmAnyCallbackStruct    cbs;
    DayslotsWidget	    dsw = (DayslotsWidget) w;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    cbs.reason = (int)XmCR_HELP;
    cbs.event  = event;

    XtCallCallbackList ((Widget) dsw, DswHelpCallback(dsw,o), &cbs);
    
}

static void display_dayslots
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	XRectangle	*expose_area,
	Region		region,
	Boolean		clear_anyway,
	Boolean		draw_index)
#else	/* no prototypes */
	(dsw, expose_area, region, clear_anyway, draw_index)
	DayslotsWidget	dsw;
	XRectangle	*expose_area;
	Region		region;
	Boolean		clear_anyway;
	Boolean		draw_index;
#endif	/* prototype */
    {
    Position	    ys, ye;
    Position	    px, py;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    Position	    dividing1_x	 = DswDividingLine1X(dsw, o);
    Position	    dividing2_x	 = DswDividingLine2X(dsw, o);
    int		    top_margin	 = DswTopMargin    (dsw, o);
    Dimension	    slot_width	 = DswTimeLabelWidth(dsw, o);
    GC		    line_gc	 = DswSlotLineGCs  (dsw, o) [0];
    GC		    fore_gc      = DswForegroundGC (dsw, o);
    GC		    back_gc      = DswBackgroundGC (dsw, o);
    GC		    invert_gc    = DswInvertGC     (dsw, o);
    GC		    outline_gc   = DswOutlineGC    (dsw, o);
    int		    curr_invert_start_time = DswStartTimeCurrInvert  (dsw, o);
    int		    curr_invert_end_time   = DswEndTimeCurrInvert    (dsw, o);
    int		    index_time   = DswIndexTime    (dsw, o);
    Dimension	    dsw_height	 = XtHeight (dsw);
    Display	    *d		 = XtDisplay(dsw);
    Window	    w		 = XtWindow (dsw);
    Position	    x		 = expose_area->x;    
    Position	    y		 = expose_area->y;    
    Dimension	    width	 = expose_area->width;    
    Dimension	    height	 = expose_area->height;    
    Boolean	    line_gc_set	 = FALSE;
    

    if (clear_anyway || (curr_invert_start_time >= 0)) {
	if (region == NULL) {
	    XClearArea (d, w, x, y, width, height, FALSE);
	} else {
	    MISCSetGCClipMask (d, back_gc, expose_area, region);
	    XFillRectangle (d, w, back_gc, x, y, width, height);
	    MISCSetGCClipMask (d, back_gc, NULL, NULL);
	}
    }

    if (! DswIsDayNote(dsw, o)) {
	display_slot_lines (dsw, expose_area, region);
    }

    if ((x <= dividing1_x) && (x + width >= dividing1_x)) {
	MISCSetGCClipMask (d, line_gc, expose_area, region);
	line_gc_set = TRUE;
	XDrawLine (d, w, line_gc, dividing1_x, 0, dividing1_x, dsw_height);
    }

    if ((x <= dividing2_x) && (x + width >= dividing2_x)) {
	if (! line_gc_set) {
	    MISCSetGCClipMask (d, line_gc, expose_area, region);
	}
	XDrawLine (d, w, line_gc, dividing2_x, top_margin + 1,
				  dividing2_x, dsw_height - top_margin);
    }

    if (DswIsDayNote(dsw, o)) {
        /*	  
	**  Display the default icons for the daynote area
	*/	  
	if (DswDefaultNumIcons(dsw, o) != 0) {
	    px = DswTimeLabelX(dsw, o) + ((DswTimeLabelWidth(dsw, o) -
					    DswPixmapWidth(dsw, o)) / 2);
	    py = (XtHeight(dsw) - DswPixmapHeight(dsw, o)) / 2;

	    if (!((x + width  <= px) || (x >= px + DswPixmapWidth(dsw, o)) ||
	          (y + height <= py) || (y >= py + DswPixmapHeight(dsw, o)))) {

		XCopyPlane
		(
		    d,
		    DswPixmaps(dsw,o)[DswDefaultIcons(dsw,o)[0]],
		    w,
		    fore_gc,
		    0, 0,
		    DswPixmapWidth(dsw, o), DswPixmapHeight(dsw, o),
		    px, py,
		    1
		);
	    }
	}
    } else {
	display_slot_times (dsw, expose_area, region);
    }

    if (curr_invert_start_time >= 0) {
        /*	  
	**  If something is currently inverted then show it.
	*/	  
        ys = TRANSLATE_TIME_TO_Y(curr_invert_start_time, dsw);
	ye = TRANSLATE_TIME_TO_Y(curr_invert_end_time,   dsw);
	ys = MAX (ys, y);
	ye = MIN (ye, y + height - 1);

	if (ye >= ys) {
	    MISCSetGCClipMask (d, invert_gc, expose_area, region);
	    XFillRectangle (d, w, invert_gc, DswTimeLabelX(dsw, o), ys,
					     slot_width - 1, ye - ys + 1);
	}
    }

    if (draw_index && (index_time >= 0)) {
	display_index_time (dsw, expose_area);
    }

    if (! DswIsDayNote(dsw, o)) {
	if (DswOutlineDrawable(dsw, o) && draw_index && 
	    DswOutlineVisible(dsw, o) && DswOutline(dsw, o)) {

	    invert_outline (dsw, expose_area, region,
			   DswOutlineVisX1(dsw, o), DswOutlineVisY1(dsw, o),
			   DswOutlineVisX2(dsw, o), DswOutlineVisY2(dsw, o));
	}

	TBWRedisplayGidget
	    ((Widget) dsw, DswTimebarGidget(dsw, o), expose_area);
    }

}

static void Redisplay
#if defined(_DWC_PROTO_)
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
    Region	    expose_region;
    DayslotsWidget  dsw = (DayslotsWidget) w;
    XmOffsetPtr    o = DswOffsetPtr(dsw);


#if defined(DEBUG)
    printf ("Dayslots Redisplay Called\n");
#endif

    if (DswNoexposeRegion(dsw, o) == NULL) {
	expose_region = region;
    } else {
	expose_region = DswExposeRegion(dsw, o);
	XSubtractRegion (region, DswNoexposeRegion(dsw, o), expose_region);
	if (XEmptyRegion (expose_region)) {
	    return;
	}
    }

    expose_area.x      = ee->x;
    expose_area.y      = ee->y;
    expose_area.width  = ee->width;
    expose_area.height = ee->height;

    display_dayslots (dsw, &expose_area, expose_region, TRUE, TRUE);
    
}

static void get_timetext_width
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	XmString	*text,
	Cardinal	time,
	Dimension	*width
	)
#else	/* no prototypes */
	(dsw, text, time, width)
	DayslotsWidget	dsw;
	XmString	*text;
	Cardinal	time;
	Dimension	*width;
#endif	/* prototype */
{
    DSWGetTextCallbackStruct	cbs;
    XmString	    time_text;
    XmOffsetPtr     o = DswOffsetPtr(dsw);
    XFontStruct	    *font;
    XmString	    xm_str;
    long	    byte_count, cvt_status;

    if (text[time] == NULL)
    {
	cbs.reason = DwcDswCRGetText;
	cbs.event  = NULL;
	cbs.times  = text;
	cbs.index  = time;
	XtCallCallbackList ((Widget) dsw, DswGetTextCallback(dsw,o), &cbs);
    }

    if (*width != 0) return;

    /*	  
    **  text[time] gets updated by the DwcDswNgetTextCallback. If that fails for
    **	some reason we'll have warned the user, but try to continue.
    */
    time_text = text[time];
    if (XmStringEmpty(time_text)) return;

    /*
    ** Measure the width;
    */
    *width = XmStringWidth (DswFontList(dsw,o), time_text);

}

static void get_timetext_widths
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Boolean		reset)
#else	/* no prototypes */
	(dsw, reset)
	DayslotsWidget	dsw;
	Boolean		reset;
#endif	/* prototype */
{
    Cardinal	    i;   
    int		    incr;
    Cardinal	    max_width = 0;
    XmOffsetPtr    o = DswOffsetPtr(dsw);
    int		    slot_size = DswActualDayslotsSize(dsw, o);


    if (reset)
    {
	for (i = 0;  i <= 24 * 60;  i++)
	{
	    DswTimeWidths(dsw, o) [i] = 0;
	}

	for (i = 0;  i < 60;  i++)
	{
	    DswMinuteWidths(dsw, o) [i] = 0;
	}
    }

    incr = slot_size;

    /*
    ** Get the texts for use in the titles of the time slots.
    */
    if (DswRealMinuteTexts(dsw, o) != NULL)
    {
	for (i = 0;  i < 60;  i = i + slot_size)
	{
	    (void) get_timetext_width
	    (
		dsw,
		DswRealMinuteTexts(dsw, o),
		i,
		&(DswMinuteWidths(dsw, o) [i])
	    );
	}
	incr = 60;
    }

    /*
    ** Get the times for the hours.  We can get more later.
    */
    for (i = 0;  i <= 24 * 60;  i += incr)
    {
	(void) get_timetext_width
	    (dsw, DswRealTimeTexts(dsw, o), i, &(DswTimeWidths(dsw, o) [i]));

	if (DswTimeWidths(dsw, o) [i] > max_width)
	{
	    max_width = DswTimeWidths(dsw, o) [i];
	}
    }

    DswTimeMaxWidth(dsw, o) = max_width;

}

static int TimeTextYPosition
#if defined(_DWC_PROTO_)
	(
	Cardinal	time,
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(time, dsw)
	Cardinal	time;
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    int		    y_pos;
    int		    descent;
    int		    ascent;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    int		    height = DswTimeLabelHeight(dsw, o);
    int		    text_height;
    XFontStruct	    *font;

    text_height = MISCFontListHeight(DswFontList(dsw,o));

    /*
    ** Top of slot.
    */
    y_pos = TRANSLATE_TIME_TO_Y(time, dsw);

    /*
    ** Top of centered text.
    */
    y_pos = y_pos + (height / 2) - (text_height / 2);

    /*
    ** Move up to be on the line.
    */
    if (DswOnTheLine(dsw, o))
    {
	y_pos = y_pos - DswTopMargin(dsw, o);
    }

    return (y_pos);

}

static int IndexYPositionForTime
#if defined(_DWC_PROTO_)
	(
	Cardinal	time,
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(time, dsw)
	Cardinal	time;
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    int		    y_pos;
    XmOffsetPtr    o = DswOffsetPtr(dsw);


    y_pos = TRANSLATE_TIME_TO_Y(time, dsw) ;
    if (DswOnTheLine(dsw, o)) {
	y_pos = y_pos - DswTopMargin(dsw, o) + 1;
    } else {
	y_pos = y_pos + 1;
    }

    return (y_pos);

}

static int TimeForIndexYPosition
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Position	y_pos)
#else	/* no prototypes */
	(dsw, y_pos)
	DayslotsWidget	dsw;
	Position	y_pos;
#endif	/* prototype */
    {
    int			y;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    y = y_pos;
    if (DswOnTheLine(dsw, o)) {
	y_pos = y_pos + DswTopMargin(dsw, o);
    } else {
	y_pos = y_pos - 1;
    }

    return (translate_y_to_time(dsw, y_pos, FALSE));

}

Cardinal DSWGetDayslotsHeight
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr    o = DswOffsetPtr(dsw);

    return (DswTimeLabelHeight(dsw, o));

}

static Cardinal get_fine_increment_height
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr    o = DswOffsetPtr(dsw);

    return ((DswTimeLabelHeight(dsw, o) * DswActualFineIncrement(dsw, o)) /
	    DswActualDayslotsSize(dsw, o));

}

/* change_inverted_area takes the currently inverted area and a new area,   */
/* and inverts both of them so that the first area is no longer inverted    */
/* and the second area is. It tries to be smart about overlapping */
static void change_inverted_area
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	int		oldtop,
	int		oldbottom,
	int		newtop,
	int		newbottom)
#else	/* no prototypes */
	(dsw, oldtop, oldbottom, newtop, newbottom)
	DayslotsWidget	dsw;
	int		oldtop;
	int		oldbottom;
	int		newtop;
	int		newbottom;
#endif	/* prototype */
    {
    int		    currinvertarea_top,   currinvertarea_bottom;
    int		    newinvertarea_top,   newinvertarea_bottom;
    int		    overlap_top, overlap_bottom;
    Boolean	    inverse_newinvertarea;

    XmOffsetPtr    o = DswOffsetPtr(dsw);
    Dimension	    slot_width	 = DswTimeLabelWidth(dsw, o);
    GC		    invert_gc    = DswInvertGC     (dsw, o);
    Display	    *d		 = XtDisplay(dsw);
    Window	    w		 = XtWindow (dsw);

    /*
    **	We have two areas to inverse, one to clear and one to set.  The trick
    **	to avoid "flashing" is to make sure we don't inverse the same parts of
    **	the screen twice....
    */
	
    currinvertarea_top    = oldtop;
    currinvertarea_bottom = oldbottom;
    newinvertarea_top    = newtop;
    newinvertarea_bottom = newbottom;

    /*
    **  Check if the two areas overlap.
    */
    if (((currinvertarea_top < newinvertarea_top) &&
	 (currinvertarea_bottom < newinvertarea_top)) ||
        ((newinvertarea_top < currinvertarea_top) &&
	 (newinvertarea_bottom < currinvertarea_top))) {

	/*
	**  If the two areas don't overlap then we have to paint both of them
	**  and the areas we have already are correct.
	*/
	inverse_newinvertarea  = TRUE;

    } else {

	/*
	**  If they do overlap then...
	*/
	if (currinvertarea_top == newinvertarea_top) {

	    /*
	    **  If the tops of the two areas are the same, then it is the lower
	    **	part only that we need to repaint.
	    */
	    currinvertarea_top = MIN(currinvertarea_bottom,
					newinvertarea_bottom) + 1;
	    currinvertarea_bottom = MAX(currinvertarea_bottom,
					newinvertarea_bottom);
	    inverse_newinvertarea = FALSE;

	} else {

	    if (currinvertarea_bottom == newinvertarea_bottom) {

		/*
		** If the bottoms of the two areas are the same, then it is
		** the upper part only that we need to repaint.
		*/
		currinvertarea_bottom = MAX(currinvertarea_top,
					    newinvertarea_top) - 1;
		currinvertarea_top = MIN(currinvertarea_top,
					    newinvertarea_top);
		inverse_newinvertarea = FALSE;

	    } else {

		/*
		**  General case, overlap without common top or bottom
		**  edges.  We will need to repaint two areas: work out
		**  what they are.
		*/
		inverse_newinvertarea  = TRUE;
		overlap_top = MAX(currinvertarea_top, newinvertarea_top);
		overlap_bottom = MIN(currinvertarea_bottom,
					newinvertarea_bottom);
		currinvertarea_top = MIN(currinvertarea_top,
					newinvertarea_top);
		newinvertarea_bottom = MAX(currinvertarea_bottom,
					    newinvertarea_bottom);
		currinvertarea_bottom = overlap_top - 1;
		newinvertarea_top = overlap_bottom + 1;

	    }
	}
    }

    /*
    **  Repaint the first area.
    */
    XFillRectangle(d,
		    w, invert_gc,
		    DswTimeLabelX(dsw, o),
		    currinvertarea_top, 
		    slot_width - 1,
		    currinvertarea_bottom - currinvertarea_top + 1);

    /*
    **  Repaint the second area only if needed.
    */
    if (inverse_newinvertarea) {
	XFillRectangle (d,
			w, invert_gc,
			DswTimeLabelX(dsw, o),
			newinvertarea_top, 
			slot_width - 1,
			newinvertarea_bottom - newinvertarea_top + 1);
    }
    
}

static void change_inverted_to
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	int		entry_start_time,
	int		entry_end_time)
#else	/* no prototypes */
	(dsw, entry_start_time, entry_end_time)
	DayslotsWidget	dsw;
	int		entry_start_time;
	int		entry_end_time;
#endif	/* prototype */
    {
    Position	    y_start_1, y_end_1;
    Position	    y_start_2, y_end_2;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    Dimension	    slot_width	 = DswTimeLabelWidth(dsw, o);
    GC		    invert_gc    = DswInvertGC    (dsw, o);
    int		    curr_invert_start_time = DswStartTimeCurrInvert(dsw, o);
    int		    curr_invert_end_time   = DswEndTimeCurrInvert(dsw, o);
    Display	    *d		 = XtDisplay(dsw);
    Window	    w		 = XtWindow (dsw);

    /*	  
    **  Already inverted.
    */	  
    if ((entry_start_time == curr_invert_start_time) &&
	(entry_end_time == curr_invert_end_time)) {
	return;
    }
     
    XSetClipMask (d, invert_gc, None);

    if ((curr_invert_start_time < 0) || (entry_start_time < 0)) {
	if (curr_invert_start_time >= 0) {
            /*	  
	    **  We've gotten an entry_start_time <0 (most likely -1) which is
	    **	supposed to tell us that we're to uninvert what is currently
	    **	inverted and leave nothing inverted (DswStartTimeCurrInvert and
	    **	DswEndTimeCurrInvert will be -1).
	    */	  
            y_start_1 = TRANSLATE_TIME_TO_Y(curr_invert_start_time, dsw);
	    y_end_1 = TRANSLATE_TIME_TO_Y(curr_invert_end_time,   dsw);
	    entry_start_time = entry_end_time = - 1;
	    DswOutline(dsw, o) = FALSE;
	} else {
            /*	  
	    **  Nothing is currently inverted (since DswStartTimeCurrInvert is <
	    **	0 (most likely -1), so we need to invert the entry delimited by
	    **	the passed in entry_start_time and entry_end_time.
	    */	  
            y_start_1 = TRANSLATE_TIME_TO_Y(entry_start_time, dsw);
	    y_end_1 = TRANSLATE_TIME_TO_Y(entry_end_time,   dsw);
	    DswOutlineY1(dsw, o) = y_start_1 + 1;
	    DswOutlineY2(dsw, o) = y_end_1 - 1;
	    DswOutline(dsw, o) = TRUE;
	}

	XFillRectangle(d,
			w, invert_gc,
			DswTimeLabelX(dsw, o),
			y_start_1,
			slot_width - 1,
			y_end_1 - y_start_1 + 1);

    } else {

        /*	  
	**  Something is inverted and we're being asked to invert something
	**  else. We want to uninvert the currently inverted area and invert
	**  the entry's area.
	*/	  
        y_start_1 = TRANSLATE_TIME_TO_Y(curr_invert_start_time, dsw);
	y_end_1 = TRANSLATE_TIME_TO_Y(curr_invert_end_time,   dsw);
	y_start_2 = TRANSLATE_TIME_TO_Y(entry_start_time,   dsw);
	y_end_2 = TRANSLATE_TIME_TO_Y(entry_end_time,     dsw);

	change_inverted_area(dsw, y_start_1, y_end_1, y_start_2, y_end_2);

	DswOutlineY1(dsw, o) = y_start_2 + 1;
	DswOutlineY2(dsw, o) = y_end_2 - 1;
	DswOutline(dsw, o) = TRUE;
    }

    /*	  
    **  Update our record of what is inverted.
    */	  
    DswStartTimeCurrInvert(dsw, o) = entry_start_time;
    DswEndTimeCurrInvert(dsw, o) = entry_end_time;


}

static int translate_y_to_time
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Position	y_pos,
	Boolean		round)
#else	/* no prototypes */
	(dsw, y_pos, round)
	DayslotsWidget	dsw;
	Position	y_pos;
	Boolean		round;
#endif	/* prototype */
    {        
    Position		y;
    int			time;
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    y = y_pos - DswTopMargin(dsw, o) - 1;
    if (y < 0) {
	return (- 1);
    }

    if (round) {
	time = (y / DswTimeLabelHeight(dsw, o)) *
	       DswActualDayslotsSize(dsw, o);
    } else {
	time = (y * DswActualDayslotsSize(dsw, o)) /
	       DswTimeLabelHeight(dsw, o);
    }
    
    return (time);

}

void DSWCloseEntry
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	Time		time)
#else	/* no prototypes */
	(dsw, entry, time)
	DayslotsWidget	dsw;
	DwcDswEntry	entry;
	Time		time;
#endif	/* prototype */
{
    XmOffsetPtr		o = DswOffsetPtr(dsw);
	    
    if (entry == NULL)
    {
	return;
    }

    (void) TSWTextChanged (entry->timeslot, TRUE);
    /*	  
    **  Go set the entry closed potentially redisplay stuf like resize knobs,
    **	etc.
    */	  
    TSWSetOpened (entry->timeslot, FALSE, time);

    if (entry == DswOpenEntry(dsw, o))
    {
        /*	  
        **  If there is a currently open entry and it is us then clear our
	**  record of it and undo the visual inversion that shows we're open by
	**  uninverting what is inverted.
        */	  
        DswOpenEntry (dsw, o) = NULL;
	change_inverted_to (dsw, -1, -1);
    }
    
    /*	  
    **  Go arrange the order of the timeslots and give focus to the text widget
    **	of the topmost one.
    */	  
    arrange_timeslots (dsw, DswOpenEntry(dsw, o), time);

}

void DSWSetEntryText
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry,
	char		*text)
#else	/* no prototypes */
	(entry, text)
	DwcDswEntry	entry;
	char		*text;
#endif	/* prototype */
{
	    
    if (entry == NULL)
    {
	return;
    }

    TSWSetText (entry->timeslot, text);
    
}

void DSWSetEntryCSText
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry,
	XmString	text)
#else	/* no prototypes */
	(entry, text)
	DwcDswEntry	entry;
	XmString	text;
#endif	/* prototype */
{
    
    if (entry == NULL)
    {
	return;
    }

    TSWSetCSText (entry->timeslot, text);

}

char *DSWGetEntryText
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry)
#else	/* no prototypes */
	(entry)
	DwcDswEntry	entry;
#endif	/* prototype */
{
	    
    if (entry == NULL)
    {
	return (NULL);
    }

    return (TSWGetText (entry->timeslot));
    
}

XmString DSWGetEntryCSText
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry)
#else	/* no prototypes */
	(entry)
	DwcDswEntry	entry;
#endif	/* prototype */
{
	    
    if (entry == NULL)
    {
	return (NULL);
    }

    return (TSWGetCSText (entry->timeslot));
    
}

void DSWSetEntryTag
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry,
	caddr_t		tag)
#else	/* no prototypes */
	(entry, tag)
	DwcDswEntry	entry;
	caddr_t		tag;
#endif	/* prototype */
    {
	    
    if (entry == NULL) {
	return;
    }

    entry->tag = tag;
    
}

void DSWSetEntryTimes
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry		entry,
	unsigned short int	time,
	unsigned short int	duration)
#else	/* no prototypes */
	(entry, time, duration)
	DwcDswEntry		entry;
	unsigned short int	time;
	unsigned short int	duration;
#endif	/* prototype */
    {
	    
    if (entry == NULL) {
	return;
    }

    if (entry->daynote) {
	entry->start    = 0;
	entry->duration = 24 * 60;
    } else {
	entry->start    = MIN (time, (24 * 60) - 1);
	entry->duration = MIN (duration, (24 * 60) - entry->start);
    }
    
}

void DSWCancelOperation
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr    o = DswOffsetPtr(dsw);

    /*	  
    **  Flag that the operation was not okay.
    */	  
    DswOperationOk(dsw, o) = FALSE;

}

Boolean DSWRequestOperationOk
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	w,
	Time		time)
#else	/* no prototypes */
	(w, time)
	DayslotsWidget	w;
	Time		time;
#endif	/* prototype */
    {
    DSWEntryCallbackStruct    cbs;
    DayslotsWidget	dsw = (DayslotsWidget) w;
    XmOffsetPtr		o     = DswOffsetPtr(dsw);

    /*	  
    **  Initialize to okay
    */	  
    DswOperationOk(dsw, o) = TRUE;

    cbs.reason = DwcDswCRCompleteOutstanding;
    cbs.time   = time;
    
    /*	  
    **  This is going to try to close out any open entries and it will set
    **	DswOperationOK to false if it couldn't.
    */	  
    XtCallCallbackList ((Widget) dsw, DswEntryCallback(dsw,o), &cbs);

    return (DswOperationOk(dsw, o));

}

Boolean DSWRequestCloseOpenEntry
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Time		time)
#else	/* no prototypes */
	(dsw, time)
	DayslotsWidget	dsw;
	Time	time;
#endif	/* prototype */
    {
    DSWEntryCallbackStruct    cbs;
    XmOffsetPtr	    o     = DswOffsetPtr(dsw);
    DwcDswEntry	    entry = DswOpenEntry(dsw, o);

    if (entry == NULL) {
	return (TRUE);
    }

    cbs.text_changed = TSWTextChanged (entry->timeslot, FALSE);

    /*	  
    **  Is this a brand new open entry, ie not previously created?
    */	  
    if (DswOpenStart(dsw, o) == - 1) {

	cbs.reason       = DwcDswCREntryCreate;
	cbs.time_changed = FALSE;

    } else {

	cbs.time_changed =
	  ((DswOpenStart(dsw, o) != entry->start) ||
	   (DswOpenEnd (dsw, o) != entry->start + entry->duration));

	if (cbs.text_changed || cbs.time_changed) {
	    cbs.reason = DwcDswCREntryChange;
	} else {
	    DSWCloseEntry (dsw, (DwcDswEntry) entry, time);
	    return (TRUE);
	}
    }

    if (DswIsDayNote(dsw, o)) {
	cbs.start    = 0;
	cbs.duration = 0;
    } else {
	cbs.start    = entry->start;
	cbs.duration = entry->duration;
    }
    cbs.time	 = time;
    cbs.tag	 = entry->tag;
    cbs.entry	 = (DwcDswEntry) entry;
    
    XtCallCallbackList ((Widget) dsw, DswEntryCallback(dsw,o), &cbs);

    return (DswOpenEntry(dsw, o) == NULL);

}

#if !defined (AUD)
static
#endif
void ACTION_DSW_INVERTSTART
#if defined(_DWC_PROTO_)
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
    {
    MbaAction		action;
    int			entry_start_time;
    int			entry_end_time;
    DayslotsWidget	dsw = (DayslotsWidget) w;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			slot_size   = DswActualDayslotsSize(dsw, o);
    int			slot_height = DswTimeLabelHeight(dsw, o);


    DswTimeslotSequence(dsw, o) ++;

    action = MBAMouseButton1Down(DswMbaContext(dsw, o), (XEvent *) event);
    if (action == MbaIgnoreEvent) {
	return;
    }
    
    /*	  
    **  If we're editable then we're interested in motion.
    */	  
    if (DswEditable(dsw, o)) {
	DswIgnoreMotion(dsw, o) = FALSE;
    } else {
	DswIgnoreMotion(dsw, o) = TRUE;
	return;
    }

    /*	  
    **  Where on the timescale did the user do the MB1 down? If it isn't in a
    **	day's range then ignore it.
    */	  
    entry_start_time = translate_y_to_time(dsw, event->y, FALSE);
    if ((entry_start_time < 0) || (entry_start_time > 24 * 60)) {
	return;
    }

    if (DswOpenEntry(dsw, o) == NULL)
	{
        /*	  
        **  If nothing is open go check to make sure it is okay to open
        */	  
        if (! DSWRequestOperationOk (dsw, event->time))
	    {
	    DswIgnoreMotion(dsw, o) = TRUE;
	    return;
	    }

        /*	  
        **  It is okay to open so we'll want to invert from the starting time to
	**  the end of whatever is the curren slotsize.
        */	  
        entry_start_time = translate_y_to_time(dsw, event->y, TRUE);
	entry_end_time = entry_start_time + slot_size;

	}
    else
	{
        /*	  
        **  There is an already open entry so if the entry_start_time is in the range
	**  of the previous invert then let's check to see if we're double
	**  clicking on an open entry and if so let's edit it.
        */	  
        if ((entry_start_time >= DswStartTimeCurrInvert(dsw, o)) &&
	    (entry_start_time <  DswEndTimeCurrInvert(dsw, o)))
	    {
	    if (action == MbaDoubleClick)
		{
		request_edit_timeslot(dsw, DswOpenEntry(dsw, o), event->time);
		return;
		}
            /*	  
            **  Not a double-click.
            */	  
            entry_start_time = DswStartTimeCurrInvert(dsw, o);
	    entry_end_time   = DswEndTimeCurrInvert(dsw, o);
	    }
	else
	    {
            /*	  
            **  There is an open entry and we clicked out side of it. Go close
	    **	the open entry.
            */	  
            if (! DSWRequestCloseOpenEntry(dsw, event->time))
		{
                /*	  
                **  We couldn't close the open entry so ignore the motion.
                */	  
                DswIgnoreMotion(dsw, o) = TRUE;
		return;
		}

            /*	  
            **  We've close the other open entry, figure out where this one is.
            */	  
            entry_start_time = translate_y_to_time(dsw, event->y, TRUE);
	    entry_end_time = entry_start_time + slot_size;

            arrange_timeslots (dsw, NULL, event->time);
	    DswOutlineDrawable (dsw, o) = FALSE;

	    }
	}

    if (DswOpenEntry(dsw, o) == NULL)
	{
        /*	  
        **  This wasn't an open entry so set it up on the margin.
        */	  
        DswOutlineX1(dsw, o) = DswTimeSlotX(dsw, o) + 1;
	DswOutlineX2(dsw, o) = DswTimeSlotX(dsw, o) + 
				 DswTimeSlotWidth(dsw, o) - 2;
	}
    else
	{
        /*	  
        **  Use the entry's existing location.
        */	  
        DswOutlineX1(dsw, o) = DswOpenEntry(dsw, o)->x + 1;
	DswOutlineX2(dsw, o) = DswOpenEntry(dsw, o)->x +
				 DswOpenEntry(dsw, o)->width;
	}

    DswOutlineY1(dsw, o) = TRANSLATE_TIME_TO_Y(entry_start_time, dsw) + 1;
    DswOutlineY2(dsw, o) = TRANSLATE_TIME_TO_Y(entry_end_time,   dsw) - 1;

    DswOriginalStartInvert(dsw, o) = entry_start_time;
    DswOriginalEndInvert (dsw, o) = entry_end_time;
    DswInvertGrab(dsw, o) = DswGrabNothing;

    DswCancelPosition(dsw, o) = XtY(dsw);
    
    /*	  
    **  Go show that the user has clicked on this entry.
    */	  
    change_inverted_to (dsw, entry_start_time, entry_end_time);
    DswOutline(dsw, o) = TRUE;

    DswAutoScrollOffset(dsw, o) = event->y_root - event->y;

    DswAutoScrollTop(dsw, o) =
     event->y_root - (event->y + XtY(dsw) + (int) XtBorderWidth(dsw)) +
      slot_height;

    DswAutoScrollBottom(dsw, o) =
     event->y_root - (event->y + XtY(dsw) + (int) XtBorderWidth(dsw)) +
       XtHeight (XtParent(dsw)) - slot_height - 1;

    DswAutoScrollFunction(dsw, o) = DswASFChangeInverted;
    DswAutoScrollState  (dsw, o) = DswASSNotScrolling;
    

}

static void change_inverted
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	int		y)
#else	/* no prototypes */
	(dsw, y)
	DayslotsWidget	dsw;
	int		y;
#endif	/* prototype */
    {
    int			time;
    int			entry_start_time;
    int			entry_end_time;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			slot_size   = DswActualDayslotsSize(dsw, o);
    int			slot_height = DswTimeLabelHeight(dsw, o);

    time = MAX (0, translate_y_to_time(dsw, y, FALSE));
    time = MIN (time, 24 * 60);
    
    switch (DswInvertGrab(dsw, o)) {

      case DswGrabNothing :
      case DswGrabInit    :

	if (time <= DswStartTimeCurrInvert(dsw, o)) {
	    entry_start_time = (time / slot_size) * slot_size;
	    entry_end_time   = DswEndTimeCurrInvert(dsw, o);
	    DswInvertGrab(dsw, o) = DswGrabStart;
	} else {
	    if (time >= DswEndTimeCurrInvert(dsw, o)) {
		entry_start_time = DswStartTimeCurrInvert(dsw, o);
		entry_end_time   = ((time / slot_size) + 1) * slot_size;
		DswInvertGrab(dsw, o) = DswGrabEnd;
	    } else {
		change_outline(dsw);
		return;
	    }
	}
        break;
	
      case DswGrabStart   :

	if (time >= DswEndTimeCurrInvert(dsw, o)) {
	    entry_start_time = DswOriginalStartInvert(dsw, o);
	    entry_end_time   = ((time / slot_size) + 1) * slot_size;
	    DswInvertGrab(dsw, o) = DswGrabEnd;
	} else {
	    entry_start_time = (time / slot_size) * slot_size;
	    entry_end_time   = DswEndTimeCurrInvert(dsw, o);
	}
        break;

      case DswGrabEnd     :

	if (time <= DswStartTimeCurrInvert(dsw, o)) {
	    entry_start_time = (time / slot_size) * slot_size;
	    entry_end_time   = DswOriginalEndInvert(dsw, o);
	    DswInvertGrab(dsw, o) = DswGrabStart;
	} else {
	    entry_start_time = DswStartTimeCurrInvert(dsw, o);
	    entry_end_time   = ((time / slot_size) + 1) * slot_size;
	}
        break;

    }

    entry_start_time = MIN (entry_start_time, 24 * 60);
    entry_start_time = MAX (entry_start_time, 0);
    entry_end_time   = MIN (entry_end_time,   24 * 60);
    entry_end_time   = MAX (entry_end_time,   0);

    change_inverted_to (dsw, entry_start_time, entry_end_time);
    change_outline(dsw);

}

static void start_invert_motion
#if defined(_DWC_PROTO_)
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
    {
    DayslotsWidget	dsw = (DayslotsWidget) w;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    DswTimeslotSequence(dsw, o) ++;

    if (DswOpenEntry(dsw, o) != NULL) {
	XRaiseWindow (XtDisplay (DswOpenEntry(dsw, o)->timeslot),
		      XtWindow  (DswOpenEntry(dsw, o)->timeslot));
    }

    DswInvertGrab(dsw, o) = DswGrabInit;
    if (! DswIsDayNote(dsw, o)) {
	send_client_message (dsw, DswCMOutlineEvent, NULL);
    }

}

#if !defined (AUD)
static
#endif
void ACTION_DSW_INVERTMOTION
#if defined(_DWC_PROTO_)
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
    {
    MbaAction		action;
    DayslotsWidget	dsw = (DayslotsWidget) w;
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    DswTimeslotSequence(dsw, o) ++;

    action = MBAMouseButton1Motion(DswMbaContext(dsw, o),
				       (XEvent *) event);

    if (DswIgnoreMotion(dsw, o) || (action == MbaIgnoreEvent)) {
	return;
    }

    if (action == MbaMotionStart) {
	start_invert_motion (w, event);
    }

    if (! DswIsDayNote(dsw, o)) {
	auto_scroll_to_position(dsw, event->y_root);
    }

}

static void double_click_timeout
#if defined(_DWC_PROTO_)
	(
	caddr_t		tag,
	XtIntervalId	*id)
#else	/* no prototypes */
	(tag, id)
	caddr_t		tag;
	XtIntervalId	*id;
#endif	/* prototype */
    {
    DswTimerBlock	tick_ctx = (DswTimerBlock) tag;
    DayslotsWidget	dsw  = tick_ctx->dsw;
    int			seq  = tick_ctx->sequence;
    Time		time = tick_ctx->time;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    /*	  
    **  We're done with our timer block
    */	  
    XtFree ((char *) tick_ctx);

    /*	  
    **  If the user hasn't done any other mousey things since we set the timer
    **	then go close the open entry if there is one.
    */	  
    if (seq == DswTimeslotSequence(dsw, o)) {
	DswTimeslotSequence(dsw, o) ++;
	DswWait2ndClickTimer(dsw, o) = (XtIntervalId) 0;

	(void) DSWRequestCloseOpenEntry (dsw, time);

	/*
	** Tell the creator to put focus somewhere else.
	*/
	XtCallCallbackList ((Widget) dsw, DswFocusDisposeCallback(dsw,o), NULL);
    }

}

extern int MbaTimerValue;

#if !defined (AUD)
static
#endif
void ACTION_DSW_INVERTEND
#if defined(_DWC_PROTO_)
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
    {
    DswTimerBlock	tick_ctx;
    MbaAction		action;
    int			duration;
    char		entry_text [50];
    DayslotsWidget	dsw = (DayslotsWidget) w;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			entry_start_time = DswStartTimeCurrInvert(dsw, o);
    int			entry_end_time   = DswEndTimeCurrInvert(dsw, o);


    DswTimeslotSequence(dsw, o) ++;

    action = MBAMouseButton1Up(DswMbaContext(dsw, o), (XEvent *) event);

    /*	  
    **  return if we're supposed to ignore this.
    */	  
    if (DswIgnoreMotion(dsw, o) || (action == MbaIgnoreEvent)) {
	return;
    }

    /*	  
    **  Turn off our specialized dsw auto_scrolling.
    */	  
    auto_scroll_cancel(dsw);

    /*	  
    **  Invert the outline (to remove it)
    */	  
    erase_outline(dsw);

    duration = entry_end_time - entry_start_time;

    /*	  
    **  This must be a new entry so create it
    */	  
    if (DswOpenEntry(dsw, o) == NULL)
    {
	DswOpenEntry(dsw, o) = DSWAddEntryCS
	(
	    dsw,
	    entry_start_time,
	    duration,
	    zero_length_string,
	    DswDefaultIcons(dsw, o),
	    DswDefaultNumIcons(dsw, o),
	    DswEditable(dsw, o),
	    NULL
	);
        /*  
	**  Indicate that this is a brand new open entry, ie not a previously
	**  created entry.
	*/	  
        DswOpenStart(dsw, o) = - 1;
	DswOpenEnd (dsw, o) = - 1;

	DSWUpdateDisplay(dsw);

	if (! DswIsDayNote(dsw, o)) {
	    force_entry_visible (dsw, DswOpenEntry(dsw, o), DswEVText);
	}

        /*	  
        **  Set this !selected, opened, and its editable value based on the
	**  current value of DswEditable
        */	  
        TSWSetState (DswOpenEntry(dsw, o)->timeslot,
			FALSE, TRUE, DswEditable(dsw, o), event->time);

	TSWTraverseToText (DswOpenEntry(dsw, o)->timeslot);

    } else {
        /*	  
        **  This is not a new entry. Decide whether to start up our timer.
        */	  
        if (DswInvertGrab(dsw, o) == DswGrabNothing)
	{
            /*	  
            **  We haven't gotten any kind of motion yet.
            */	  
            tick_ctx = (DswTimerBlock) XtMalloc (sizeof (DswTimerBlockRec));
	    DswTimeslotSequence(dsw, o)++;
	    tick_ctx->sequence = DswTimeslotSequence(dsw, o);
	    tick_ctx->time     = event->time;
	    tick_ctx->dsw      = dsw;
	    DswWait2ndClickTimer(dsw, o) = XtAppAddTimeOut
	    (
		XtWidgetToApplicationContext ((Widget)dsw),
		MbaTimerValue,
		(XtTimerCallbackProc) double_click_timeout,
		tick_ctx
	    );
	}
	else
	{
	    DswOpenEntry(dsw, o)->start    = entry_start_time;
	    DswOpenEntry(dsw, o)->duration = duration;
	    DswOpenEntry(dsw, o)->sequence = DswTimeslotSequence(dsw, o);
	    DswTimeslotSequence(dsw, o)++;
	    sort_entries(dsw);
	    arrange_timeslots (dsw, DswOpenEntry(dsw, o), event->time);
	}
    }

    DswOutline(dsw, o) = FALSE;

}

#if !defined (AUD)
static
#endif
void ACTION_DSW_INVERTCANCEL
#if defined(_DWC_PROTO_)
	(
	Widget		w,
	XButtonEvent	*event)
#else	/* no prototypes */
	(w, event)
	Widget		w;
	XButtonEvent	*event;
#endif	/* prototype */
    {
    MbaAction		action;
    int			entry_start_time;
    int			entry_end_time;
    DayslotsWidget	dsw = (DayslotsWidget) w;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    DwcDswEntry		entry = DswOpenEntry(dsw, o);

    DswTimeslotSequence(dsw, o) ++;

    action = MBAMouseButtonOther(DswMbaContext(dsw, o), (XEvent *) event);

    if (DswIgnoreMotion(dsw, o) || (action == MbaIgnoreEvent)) {
	return;
    }

    /*	  
    **  Turn off our specialized dsw auto_scrolling.
    */	  
    auto_scroll_cancel(dsw);

    /*	  
    **  Invert the outline (to remove it)
    */	  
    erase_outline(dsw);

    change_inverted_to (dsw, -1, -1);

    DswOutline(dsw, o) = FALSE;

    if (DswCancelPosition(dsw, o) != XtY(dsw)) {
	DSWMoveDayslots (dsw, DswCancelPosition(dsw, o));
    }

}

void DSWClearAllEntries
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    Cardinal		i;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    DswTimeslotSequence(dsw, o) ++;
    DswSelected(dsw, o) = NULL;

    if (DswOpenEntry(dsw, o) != NULL) {
        /*	  
	**  Turn off inversion.
	*/	  
        change_inverted_to (dsw, -1, -1);

	DswOpenEntry(dsw, o) = NULL;
    }
    
    if (DswNumEntries(dsw, o) != 0) {

	DswSpareTimeslots(dsw, o) = (Widget *) XtRealloc
	(
	    (char *) DswSpareTimeslots(dsw, o),
	    sizeof (Widget *) *
		(DswNumSpareTimeslots(dsw, o) + DswNumEntries(dsw, o))
	);

	for (i = 0;  i < DswNumEntries(dsw, o);  i++)
	{
	    TSWReset (DswEntries(dsw, o) [i]->timeslot);

	    DswSpareTimeslots(dsw, o) [DswNumSpareTimeslots(dsw, o)] =
					    DswEntries(dsw, o) [i]->timeslot;
	    DswNumSpareTimeslots(dsw, o)++;

	    XtFree ((char *) DswEntries(dsw, o) [i]->icons);
	    XtFree ((char *) DswEntries(dsw, o) [i]);
	}

	DswNumEntries(dsw, o) = 0;
	XtFree ((char *) DswEntries(dsw, o));
	DswEntries(dsw, o) = NULL;
    }

    XtUnmanageChildren (DswSpareTimeslots(dsw, o),
			DswNumSpareTimeslots(dsw, o));

    arrange_timeslots (dsw, NULL, MISCGetTimeFromEvent(NULL));

}

static void sort_entries
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    Cardinal	    i, j;
    DwcDswEntry	    entry;
    XmOffsetPtr    o = DswOffsetPtr(dsw);
    Cardinal	    num_entries = DswNumEntries(dsw, o);
    DwcDswEntry	    *entries    = DswEntries  (dsw, o);


    for (i = 0;  i < num_entries;  i++) {
	for (j = i + 1;  j < num_entries;  j++) {
	    if (entries [j]->start < entries [i]->start) {
		entry = entries [j];
		entries [j] = entries [i];
		entries [i] = entry;
	    } else {
		if (entries [j]->start == entries [i]->start) {
		    if (entries [j]->duration > entries [i]->duration) {
			entry = entries [j];
			entries [j] = entries [i];
			entries [i] = entry;
		    } else {
			if ((entries [j]->duration == entries [i]->duration) &&
			    (entries [j]->sequence  < entries [i]->sequence)) {
			    entry = entries [j];
			    entries [j] = entries [i];
			    entries [i] = entry;
			}
		    }
		}
	    }
	}
    }

}

static void get_icons_pixmaps
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	unsigned char	*icons,
	Cardinal	num_icons,
	Pixmap		pixmaps[])
#else	/* no prototypes */
	(dsw, icons, num_icons, pixmaps)
	DayslotsWidget	dsw;
	unsigned char	*icons;
	Cardinal	num_icons;
	Pixmap		pixmaps[];
#endif	/* prototype */
{
    Cardinal		i;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    for (i = 0;  i < num_icons;  i++)
    {
	pixmaps [i] = DswPixmaps(dsw, o) [icons [i]];
    }

    pixmaps[num_icons] = (Pixmap) 0;

    return;
    
}

static Widget create_timeslot
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Position	y_pos,
	Dimension	height,
	unsigned char	*icons,
	Cardinal	num_icons,
	Boolean		editable)
#else	/* no prototypes */
	(dsw, y_pos, height, icons, num_icons, editable)
	DayslotsWidget	dsw;
	Position	y_pos;
	Dimension	height;
	unsigned char	*icons;
	Cardinal	num_icons;
	Boolean		editable;
#endif	/* prototype */
{
    Arg			arglist[34];
    Cardinal		ac;
    Pixmap		pixmaps[32];	/* the maximum # in DB is 32 */
    Widget		timeslot;
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    get_icons_pixmaps (dsw, icons, num_icons, pixmaps);

    SingleCB			[0].closure = (caddr_t)dsw;
    DoubleCB			[0].closure = (caddr_t)dsw;
    StartResizeAndDragCB	[0].closure = (caddr_t)dsw;
    EndResizeAndDragCB		[0].closure = (caddr_t)dsw;
    CancelResizeAndDragCB	[0].closure = (caddr_t)dsw;
    ResizeAndDragCB		[0].closure = (caddr_t)dsw;
    FocusCB			[0].closure = (caddr_t)dsw;
    LosingFocusCB		[0].closure = (caddr_t)dsw;
    CloseCB			[0].closure = (caddr_t)dsw;
    ExtendCB			[0].closure = (caddr_t)dsw;
    TimeslotHelpCB		[0].closure = (caddr_t)dsw;
    IncrSizeCB			[0].closure = (caddr_t)dsw;
    IncrPositionCB		[0].closure = (caddr_t)dsw;

    ac = 0;
    XtSetArg(arglist[ac], XmNborderWidth, (Dimension)1); ac++;
    XtSetArg(arglist[ac], XmNstringDirection, DswDirectionRToL(dsw, o)); ac++;
    XtSetArg(arglist[ac], DwcTswNfontList, DswFontList(dsw, o)); ac++;
    XtSetArg(arglist[ac], XmNx, DswTimeSlotX(dsw, o)); ac++;
    XtSetArg(arglist[ac], XmNy, y_pos); ac++;
    XtSetArg(arglist[ac], XmNheight, height); ac++;
    XtSetArg(arglist[ac], XmNwidth, DswTimeSlotWidth(dsw, o)); ac++;
    XtSetArg(arglist[ac], DwcTswNpixmapWidth, DswPixmapWidth(dsw, o)); ac++;
    XtSetArg(arglist[ac], DwcTswNpixmapHeight, DswPixmapHeight(dsw, o)); ac++;
    XtSetArg(arglist[ac], DwcTswNpixmaps, pixmaps); ac++;
    XtSetArg(arglist[ac], DwcTswNpixmapCount, num_icons); ac++;

    if (DswIsDayNote(dsw, o))
	{
	XtSetArg(arglist[ac], DwcTswNpixmapMargin, 3); ac++;
	}

    XtSetArg(arglist[ac], DwcTswNeditable, editable); ac++;
    XtSetArg(arglist[ac], DwcTswNdragable, !DswIsDayNote(dsw, o)); ac++;
    XtSetArg(arglist[ac], DwcTswNsingleCallback, SingleCB); ac++;
    XtSetArg(arglist[ac], DwcTswNdoubleCallback, DoubleCB); ac++;
    XtSetArg(arglist[ac], DwcTswNresizeStartCallback, StartResizeAndDragCB); ac++;
    XtSetArg(arglist[ac], DwcTswNresizeEndCallback, EndResizeAndDragCB); ac++;
    XtSetArg(arglist[ac], DwcTswNresizeCancelCallback, CancelResizeAndDragCB); ac++;
    XtSetArg(arglist[ac], DwcTswNresizeCallback, ResizeAndDragCB); ac++;
    XtSetArg(arglist[ac], DwcTswNdragStartCallback, StartResizeAndDragCB); ac++;
    XtSetArg(arglist[ac], DwcTswNdragEndCallback, EndResizeAndDragCB); ac++;
    XtSetArg(arglist[ac], DwcTswNdragCancelCallback, CancelResizeAndDragCB); ac++;
    XtSetArg(arglist[ac], DwcTswNdragCallback, ResizeAndDragCB); ac++;
    XtSetArg(arglist[ac], DwcTswNfocusCallback, FocusCB); ac++;
    XtSetArg(arglist[ac], DwcTswNlosingFocusCallback, LosingFocusCB); ac++;
    XtSetArg(arglist[ac], DwcTswNcloseCallback, CloseCB); ac++;
    XtSetArg(arglist[ac], DwcTswNextendSelectCallback, ExtendCB); ac++;
    XtSetArg(arglist[ac], DwcTswNincrementSizeCallback, IncrSizeCB); ac++;
    XtSetArg(arglist[ac], DwcTswNincrementPositionCallback,IncrPositionCB);ac++;
    XtSetArg(arglist[ac], XmNhelpCallback, TimeslotHelpCB); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;
    assert (ac <= XtNumber(arglist));

    timeslot = TSWTimeslotCreate ((Widget) dsw, "Entry", arglist, ac);

    return (timeslot);
}

DwcDswEntry DSWAddEntry
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget		dsw,
	unsigned short int	time,
	unsigned short int	duration,
	char			*text,
	unsigned char		*icons,
	Cardinal		num_icons,
	Boolean			editable,
	caddr_t			tag)
#else	/* no prototypes */
	(dsw, time, duration, text, icons, num_icons, editable, tag)
	DayslotsWidget		dsw;
	unsigned short int	time;
	unsigned short int	duration;
	char			*text;
	unsigned char		*icons;
	Cardinal		num_icons;
	Boolean			editable;
	caddr_t			tag;
#endif	/* prototype */
{
    XmString	xm_text;
    long	byte_count, cvt_status;
    DwcDswEntry	return_val;

    xm_text = DXmCvtFCtoCS (text, &byte_count, &cvt_status);
    return_val = DSWAddEntryCS
	(dsw, time, duration, xm_text, icons, num_icons, editable, tag);
    XmStringFree (xm_text);
    return (return_val);
}

DwcDswEntry DSWAddEntryCS
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget		dsw,
	unsigned short int	time,
	unsigned short int	duration,
	XmString		text,
	unsigned char		*icons,
	Cardinal		num_icons,
	Boolean			editable,
	caddr_t			tag)
#else	/* no prototypes */
	(dsw, time, duration, text, icons, num_icons, editable, tag)
	DayslotsWidget		dsw;
	unsigned short int	time;
	unsigned short int	duration;
	XmString		text;
	unsigned char		*icons;
	Cardinal		num_icons;
	Boolean			editable;
	caddr_t			tag;
#endif	/* prototype */
{
    DwcDswEntry		entry;
    Position		y_pos;
    Dimension		height;
    Pixmap		pixmaps[32];	/* the maximum # in DB is 32 */
    int			e;
    int			i;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    
    entry = (DwcDswEntry) XtMalloc (sizeof (DwcDswEntryRecord));

    entry->daynote = DswIsDayNote(dsw, o);
    if (DswIsDayNote(dsw, o))
    {
	entry->start    = 0;
	entry->duration = 24 * 60;
    }
    else
    {
	entry->start    = MIN (time, (24 * 60) - 1);
	entry->duration = MIN (duration, (24 * 60) - entry->start);
    }
    
    entry->num_icons = num_icons;
    if (num_icons == 0)
    {
	entry->icons = NULL;
    }
    else
    {
	entry->icons = (unsigned char *) XtMalloc (num_icons);
	memcpy (entry->icons, icons, num_icons);
    }
    
    entry->tag = tag;
    
    y_pos  = - 9999; 
    height = MAX (DswTimeslotMinHeight(dsw, o),
	TIME_HEIGHT(entry->duration, dsw));
	     
    /*	  
    **  Can we use a created but free timeslot or do we have to create one from
    **	scratch?
    */	  
    if (DswNumSpareTimeslots(dsw, o) != 0)
    {

	DswNumSpareTimeslots(dsw, o)--;
	entry->timeslot = DswSpareTimeslots(dsw,o)[DswNumSpareTimeslots(dsw,o)];

	if (DswNumSpareTimeslots(dsw, o) == 0)
	{
	    XtFree ((char *) DswSpareTimeslots(dsw, o));
	    DswSpareTimeslots(dsw, o) = NULL;
	}
	else
	{
	    DswSpareTimeslots(dsw, o) = (Widget *) XtRealloc
	    (
		(char *) DswSpareTimeslots(dsw, o),
		sizeof(Widget *) * DswNumSpareTimeslots(dsw, o)
	    );
	}

	get_icons_pixmaps (dsw, entry->icons, entry->num_icons, pixmaps);

	XtVaSetValues
	(
	    entry->timeslot,
	    DwcTswNpixmapWidth, DswPixmapWidth(dsw, o),
	    DwcTswNpixmapHeight, DswPixmapHeight(dsw, o),
	    DwcTswNpixmaps, pixmaps,
	    DwcTswNpixmapCount, num_icons,
	    XmNy, y_pos,
	    XmNheight, height,
	    DwcTswNfontList, DswFontList(dsw, o),
	    DwcDswNeditable, editable,
	    NULL
	);
    }
    else
    {
	entry->timeslot = create_timeslot
	    (dsw, y_pos, height, icons, num_icons, editable);
    }

    TSWSetCSText (entry->timeslot, text);
    entry->geometry_changed = TRUE;
    
    DswEntries(dsw,o) = (DwcDswEntry *) XtRealloc
    (
	(char *) DswEntries(dsw,o),
	sizeof(DwcDswEntry) * (DswNumEntries(dsw,o)+1)
    );

    DswEntries(dsw,o) [DswNumEntries(dsw,o)] = entry;
    DswNumEntries(dsw,o)++;

    entry->sequence = DswTimeslotSequence(dsw,o);
    DswTimeslotSequence(dsw,o)++;

    TSWSetState (entry->timeslot,
			FALSE,
			FALSE,
			editable,
			MISCGetTimeFromEvent(NULL));
    XtManageChild (entry->timeslot);
    
    return (entry);
    
}

void DSWUpdateDisplay
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr	    o = DswOffsetPtr(dsw);

    sort_entries(dsw);
    arrange_timeslots(dsw,
			DswOpenEntry(dsw, o),
			MISCGetTimeFromEvent(NULL));

}   

void DSWSetEntryIcons
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	unsigned char	*icons,
	Cardinal	num_icons)
#else	/* no prototypes */
	(dsw, entry, icons, num_icons)
	DayslotsWidget	dsw;
	DwcDswEntry	entry;
	unsigned char	*icons;
	Cardinal	num_icons;
#endif	/* prototype */
{
    Pixmap		pixmaps[32];	/* the maximum # in DB is 32 */

    if (entry == NULL) return;

    if (num_icons == 0)
    {
	if (entry->icons != NULL) XtFree ((char *) entry->icons);
	entry->icons = NULL;
    }
    else
    {
	/*
	** No point in reallocating if the size hasn't changed.
	*/
	if (entry->num_icons != num_icons)
	{
	    XtFree ((char *) entry->icons);
	    entry->icons = (unsigned char *) XtMalloc (num_icons);
	}
	memcpy (entry->icons, icons, num_icons);
    }
    entry->num_icons = num_icons;

    if (num_icons == 0)
    {
	XtVaSetValues
	(
	    entry->timeslot,
	    DwcTswNpixmaps, NULL,
	    DwcTswNpixmapCount, num_icons,
	    NULL
	);
    }
    else
    {
	get_icons_pixmaps (dsw, entry->icons, entry->num_icons, pixmaps);

	XtVaSetValues
	(
	    entry->timeslot,
	    DwcTswNpixmaps, pixmaps,
	    DwcTswNpixmapCount, num_icons,
	    NULL
	);
    }
}

DwcDswEntry DSWGetOpenEntry
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
{
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    return ((DwcDswEntry) DswOpenEntry(dsw, o));

}

void DSWDeleteEntry
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	DwcDswEntry	entry)
#else	/* no prototypes */
	(dsw, entry)
	DayslotsWidget	dsw;
	DwcDswEntry	entry;
#endif	/* prototype */
{
    Cardinal		i, j;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    DswTimeslotSequence(dsw, o) ++;
    
    if (entry == NULL)
    {
	return;
    }

    if (entry == DswOpenEntry(dsw, o)) {
	DswOpenEntry(dsw, o) = NULL;
        /*	  
	**  Turn off inversion
	*/	  
        change_inverted_to (dsw, -1, -1);
    }

    DswSpareTimeslots(dsw, o) = (Widget *) XtRealloc
    (
	(char *) DswSpareTimeslots(dsw, o),
	sizeof (Widget *) * (DswNumSpareTimeslots(dsw, o) + 1)
    );

    DswSpareTimeslots(dsw, o) [DswNumSpareTimeslots(dsw, o)] = entry->timeslot;
    DswNumSpareTimeslots(dsw, o)++;

    TSWReset (entry->timeslot);
    XtUnmanageChild (entry->timeslot);


    for (i = 0;  i < DswNumEntries(dsw, o);  i++)
    {
	if (DswEntries(dsw, o) [i] == entry)
	{
	    XtFree ((char *) DswEntries(dsw, o) [i]->icons);
	    XtFree ((char *) DswEntries(dsw, o) [i]);

	    for (j = i + 1;  j < DswNumEntries(dsw, o);  j++) {
		DswEntries(dsw, o) [j - 1] = DswEntries(dsw, o) [j];
	    }
    
	    break;
	}
    }

    DswNumEntries(dsw, o)--;

    if (DswNumEntries(dsw, o) == 0)
    {

	XtFree ((char *) DswEntries(dsw, o));
	DswEntries(dsw, o) = NULL;

    }
    else
    {

	DswEntries(dsw, o) = (DwcDswEntry *) XtRealloc
	(
	    (char *) DswEntries(dsw, o),
	    sizeof (DwcDswEntry) * DswNumEntries(dsw, o)
	);
	sort_entries(dsw);
    }

    arrange_timeslots(dsw,
			DswOpenEntry(dsw, o),
			MISCGetTimeFromEvent(NULL));
    
}
void
DSWGetEntryData
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry			entry,
	DSWEntryDataStructPtr	data)
#else	/* no prototypes */
	(entry, data)
	DwcDswEntry			entry;
	DSWEntryDataStructPtr	data;
#endif	/* prototype */
    {

    if (entry->daynote) {
	data->start    = 0;
	data->duration = 0;
    } else {
	data->start    = entry->start;
	data->duration = entry->duration;
    }

    data->text_changed = TSWTextChanged (entry->timeslot, FALSE);
    data->tag	       = entry->tag;
    data->parent       = XtParent (entry->timeslot);

}

static void request_edit_timeslot
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	Time		time)
#else	/* no prototypes */
	(dsw, entry, time)
	DayslotsWidget	dsw;
	DwcDswEntry	entry;
	Time		time;
#endif	/* prototype */
{
    DSWEntryCallbackStruct    ecb;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    DswTimeslotSequence(dsw, o) ++;

    ecb.text_changed = TSWTextChanged (entry->timeslot, FALSE);

    if (entry == DswOpenEntry(dsw, o)) {
        /*	  
        **  The entry is already open
        */	  
        if (DswOpenStart(dsw, o) == - 1) {
            /*	  
            **  But it is a brand new entry, not previously created.
            */	  
            ecb.reason       = DwcDswCREntryEditNew;
	    ecb.time_changed = FALSE;
	} else {
            /*	  
            **  Then entry is open but a previously existing entry
            */	  
            ecb.reason       = DwcDswCREntryEdit;
	    ecb.time_changed =
	      ((DswOpenStart(dsw, o) != entry->start) ||
	       (DswOpenEnd (dsw, o) != entry->start + entry->duration));
	}
    } else {
        /*	  
        **  The entry wasn't open.
        */	  
        ecb.reason       = DwcDswCREntryEdit;
	ecb.time_changed = FALSE;
    }
    
    /*	  
    **  Set start and duration depending upon whether or not we're a daynote or
    **	and entry
    */	  
    if (DswIsDayNote(dsw, o)) {
	ecb.start    = 0;
	ecb.duration = 0;
    } else {
	ecb.start    = entry->start;
	ecb.duration = entry->duration;
    }
    ecb.time	 = time;
    ecb.tag	 = entry->tag;
    ecb.entry	 = (DwcDswEntry) entry;

    XtCallCallbackList ((Widget) dsw, DswEntryCallback(dsw,o), &ecb);
    
}

Boolean DSWGetEntryEditable
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry)
#else	/* no prototypes */
	(entry)
	DwcDswEntry	entry;
#endif	/* prototype */
    {

    return (TSWGetEditable ((Widget) entry->timeslot));

}

void DSWSetEntryEditable
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry,
	Boolean		editable,
	Time		time)
#else	/* no prototypes */
	(entry, editable, time)
	DwcDswEntry	entry;
	Boolean		editable;
	Time		time;
#endif	/* prototype */
    {

    TSWSetEditable ((Widget) entry->timeslot, editable, time);

}

static void TIMESLOT_DOUBLE
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    Cardinal		    i;
    Time		    time;
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr		    o = DswOffsetPtr(dsw);
    DwcDswEntry		    *entries = DswEntries (dsw, o);

    DswTimeslotSequence(dsw, o) ++;

    time = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *) cbs);
    
    i = 0;
    while (entries [i]->timeslot != w)  i++;
    
    request_edit_timeslot (dsw, entries [i], time);
    
}

/* This gets called by TIMESLOT_SINGLE and TIMESLOT_FOCUS.  If focus_only   */
/* is true then we open it, give focus to the text_widget and redisplay.    */
/* Otherwise we set the state to selected, open, and the value of editable  */

static void click_on_timeslot
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Widget		w,
	Boolean		focus_only,
	Time		time)
#else	/* no prototypes */
	(dsw, w, focus_only, time)
	DayslotsWidget	dsw;
	Widget		w;
	Boolean		focus_only;
	Time		time;
#endif	/* prototype */
{
    DSWEntryCallbackStruct    cbs;
    Boolean		    editable;
    Cardinal		    i;
    DwcDswEntry		    entry;
    XmOffsetPtr		    o = DswOffsetPtr(dsw);
    DwcDswEntry		    *entries   = DswEntries (dsw, o);
    DwcDswEntry		    open_entry = DswOpenEntry(dsw,o);
    Boolean		    found;

    if (open_entry == NULL) {
        /*	  
        **  Try to close any open entries.
        */	  
        if (! DSWRequestOperationOk (dsw, time)) {
	    return;
	}
    }

    /*	  
    **  We use different logic if we've got the keyboard traversal stuff turned
    **	on for the timeslots since we seem to get an extra callback through here
    **	when creating a new entry that causes an ACCVIO. In effect the code here
    **	ignores the widget if it isn't in its list of entries. Yes I know it is
    **	a hack but the interaction of the old XUI style and the traversal stuff
    **	is driving me crazy!
    */	  
    found = FALSE;
    for (i = 0; i < DswNumEntries(dsw,o); i++)
	{
	if (entries[i] == NULL)
	    {
            /*	  
	    **  This is a kludge but is necessitated by the fact that we getting
	    **	focus callbacks to the timeslot textwidget when we don't want
	    **	them (ie when our window gets focus)
	    */	  
/*          printf("click_on_timeslot null entry noop\n"); */
	    return;
	    }

	if (entries[i]->timeslot == w)
	    {
	    entry = entries[i];
	    found = TRUE;
	    break;
	    }	
	}

    /*	  
    **  If we got to the end of the list and still didn't find it then return.


    */	  
    if (!found)
	{
/*	printf("click_on_timeslot no matching entry noop\n"); */
	return;
	}
    /*	  
    **  Get its editing status.
    */	  
    editable = TSWGetEditable (entry->timeslot);
    
    /*	  
    **  If there is an open entry different from the one we clicked on and the
    **	one we clicked on is editable then go close out the open one.
    */	  
    if ((open_entry != entry) && editable)
    {
	if (! DSWRequestCloseOpenEntry (dsw, time))
	{
	    return;
	}
	open_entry = NULL;
    }

    /*	  
    **  If this is an entry (not a daynote) and we're not just simply giving
    **	focus, then make sure the entry is visible. This moves the entry into
    **	complete view if necessary.
    */	  
    if ((! DswIsDayNote(dsw, o)) && (! focus_only)) {
	force_entry_visible (dsw, entry, DswEVText);
    }

    /*	  
    **  Make sure we can be seen.
    */	  
    XRaiseWindow (XtDisplay (w), XtWindow (w));
    DswOutlineDrawable(dsw, o) = FALSE;

    /*	  
    **  Show that we have the input focus on the entry we clicked upon.
    */	  
    if (editable && (open_entry == NULL))
    {
	DswOpenEntry(dsw, o) = entry;
	DswOpenStart(dsw, o) = entry->start;
	DswOpenEnd (dsw, o) = entry->start + entry->duration;
        /*	  
	**  invert entry
	*/	  
        change_inverted_to (dsw, entry->start, entry->start + entry->duration);
    }

    if (focus_only) {
        /*	  
        **  If the timeslot isn't opened, then open it do an XmProcessTraversal
	**  to give focus to the text widget and redisplay (tweaking the
	**  appearance of things like the drag knobs, etc.). If it is already
	**  opened then this is a no-op.
        */
        TSWSetOpened (w, TRUE, time);
    } else {
        /*	  
        **  Set the state to selected, opened and the value of editable
        */	  
        TSWSetState (w, TRUE, TRUE, editable, time);

	cbs.text_changed = TSWTextChanged (w, FALSE);
        /*	  
	**  Is this a brand new entry, ie not one previously created?
	*/	  
        if (DswOpenStart(dsw, o) == - 1) {
	    cbs.time_changed = FALSE;
	} else {
	    cbs.time_changed =
	      ((DswOpenStart(dsw, o) != entry->start) ||
	       (DswOpenEnd (dsw, o) != entry->start + entry->duration));
	}

	if (DswIsDayNote(dsw, o)) {
	    cbs.start    = 0;
	    cbs.duration = 0;
	} else {
	    cbs.start    = entry->start;
	    cbs.duration = entry->duration;
	}

	cbs.time   = time;
	cbs.tag	   = entry->tag;
	cbs.entry  = (DwcDswEntry) entry;
	cbs.reason = DwcDswCREntrySelect;
	XtCallCallbackList ((Widget) dsw, DswEntryCallback(dsw,o), &cbs);
    }
}

static void TIMESLOT_SINGLE
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    Cardinal		    i;
    Time		    time;
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    DwcDswEntry		    *entries   = DswEntries (dsw, o);
    DwcDswEntry		    open_entry = DswOpenEntry(dsw, o);

    DswTimeslotSequence(dsw, o) ++;

    time = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *) cbs);

    click_on_timeslot (dsw, w, FALSE, time);

    /*
    ** Only traverse to sash if the sash is going to be there.
    */
    if (TSWGetEditable(w)) TSWTraverseToSash (w);
}

static void TIMESLOT_FOCUS
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    Cardinal		    i;
    Time		    time;
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr		    o = DswOffsetPtr(dsw);
    DwcDswEntry		    *entries   = DswEntries (dsw, o);
    DwcDswEntry		    open_entry = DswOpenEntry(dsw, o);

    DswTimeslotSequence(dsw, o) ++;

    time = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *) cbs);

    click_on_timeslot(dsw, w, TRUE, time); 

}

static void TIMESLOT_LOSING_FOCUS
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    Cardinal		    i;
    Time		    time;
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr		    o = DswOffsetPtr(dsw);

    DswTimeslotSequence(dsw, o) ++;

    time = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *) cbs);
    
    (void)DSWRequestCloseOpenEntry(dsw,time);

}

static void DswCallSelectExtend
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	Boolean		selected,
	Time		time)
#else	/* no prototypes */
	(dsw, entry, selected, time)
	DayslotsWidget	dsw;
	DwcDswEntry	entry;
	Boolean		selected;
	Time		time;
#endif	/* prototype */
{    
    DSWEntryCallbackStruct    ecb;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);


    /*	  
    **  Has the text changed?
    */	  
    ecb.text_changed = TSWTextChanged (entry->timeslot, FALSE);

    /*	  
    **  Has the entry time been changed?
    */	  
    if (DswOpenStart(dsw, o) == - 1) {
        /*	  
	**  This a brand new entry, ie not one previously created
	*/	  
        ecb.time_changed = FALSE;
    } else {
        /*	  
	**  This is an existing entry (previously closed)
	*/	  
        ecb.time_changed =
	  ((DswOpenStart(dsw, o) != entry->start) ||
	   (DswOpenEnd (dsw, o) != entry->start + entry->duration));
    }

    /*	  
    **  Figure out entry start and duration
    */	  
    if (DswIsDayNote(dsw, o)) {
	ecb.start    = 0;
	ecb.duration = 0;
    } else {
	ecb.start    = entry->start;
	ecb.duration = entry->duration;
    }

    ecb.time   = time;
    ecb.tag    = entry->tag;	/* probably contains the DayItem */
    ecb.entry  = (DwcDswEntry) entry;

    if (selected) {
        /*	  
	**  Add this entry to the existing selection.
	*/	  
        ecb.reason = DwcDswCREntryExtendSelect;
    } else {
        /*	  
	**  Remove this entry from the list of selected ones
	*/	  
        ecb.reason = DwcDswCREntryDeselect;
    }
    
    XtCallCallbackList ((Widget) dsw, DswEntryCallback(dsw,o), &ecb);

}

void DSWCallSelectExtendOnAll
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Time	time)
#else	/* no prototypes */
	(dsw, time)
	DayslotsWidget	dsw;
	Time	time;
#endif	/* prototype */
    {
    Cardinal	    i;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    DwcDswEntry	    *entries = DswEntries(dsw, o);


    /*	  
    **  For all the entries, "TRUE" means set them selected.
    */	  
    for (i = 0;  i < DswNumEntries(dsw, o);  i++) {
	DswCallSelectExtend (dsw, entries [i], TRUE, time);
    }
    
}

static void TIMESLOT_EXTEND
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    Time		    time;
    Cardinal		    i;
    DwcDswEntry		    entry;
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    DwcDswEntry		    *entries = DswEntries(dsw, o);

    DswTimeslotSequence(dsw, o) ++;

    i = 0;
    while (entries [i]->timeslot != w)  i++;
    entry = entries [i];

    time = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *) cbs);

    DswCallSelectExtend (dsw, entry, ! cbs->selected, time);

}

static void TIMESLOT_CLOSE
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
{
    Time		    time;
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);

    DswTimeslotSequence(dsw, o) ++;

    time = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *) cbs);
    
    (void) DSWRequestCloseOpenEntry (dsw, time);

    /*
    ** Tell the creator to put focus somewhere else.
    */
    XtCallCallbackList ((Widget) dsw, DswFocusDisposeCallback(dsw,o), NULL);
}

static void TIMESLOT_START_RESIZE_AND_DRAG
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    Time		    etime;
    int			    i;
    Position		    y_pos;
    Position		    dsw_y_pos;
    Position		    top;
    Position		    bottom;
    int			    time;
    DwcDswEntry		    entry;
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr		    o = DswOffsetPtr(dsw);
    XButtonEvent	    *event = (XButtonEvent *) cbs->event;
						
    DswTimeslotSequence(dsw, o) ++;
    DswIgnoreMotion(dsw, o) = FALSE;

    switch (cbs->reason) {

      case DwcTswCRResizeStart :

	entry = DswOpenEntry(dsw, o);

	DswIndexResize(dsw, o) = TRUE;
	DswResizeTop (dsw, o) = cbs->top_knob;

	if (cbs->top_knob) {
	    time = entry->start;
	} else {
	    time = entry->start + entry->duration;
	}
	break;

      case DwcTswCRDragStart   :

	if (DswOpenEntry(dsw, o) == NULL) {
	    if (! DSWRequestOperationOk (dsw, event->time)) {
		DswIgnoreMotion(dsw, o) = TRUE;
		return;
	    }
	}
	
	i = 0;
	while (DswEntries(dsw, o) [i]->timeslot != w)  i++;

	entry = DswEntries(dsw, o) [i];
	if (entry != DswOpenEntry(dsw, o)) {
	    etime = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *) cbs);
	    if (! DSWRequestCloseOpenEntry (dsw, etime)) {
		DswIgnoreMotion(dsw, o) = TRUE;
		return;
	    }
	    TSWSetEditable (w, TRUE, etime);
	    TSWSetOpened(w, TRUE, etime);

	    DswOpenEntry(dsw, o) = entry;
	    DswOpenStart(dsw, o) = entry->start;
	    DswOpenEnd (dsw, o) = entry->start + entry->duration;
	}

	DswIndexResize(dsw, o) = FALSE;
	DswDragEntry(dsw, o) = entry;
	time = entry->start;
	break;
    }

    y_pos = TRANSLATE_TIME_TO_Y(time, dsw); 
    dsw_y_pos = XtY (w) + XtBorderWidth (w) + event->y;

    DswIndexDragOffset(dsw, o) = y_pos - dsw_y_pos;
    DswStartDragTime(dsw, o) = time;

    DswCancelPosition(dsw, o) = XtY(dsw);

    DswAutoScrollOffset (dsw, o) = event->y_root - dsw_y_pos;
    DswAutoScrollFunction(dsw, o) = DswASFMoveIndex;
    DswAutoScrollState  (dsw, o) = DswASSNotScrolling;

    top    = event->y_root - dsw_y_pos - XtY(dsw);
    bottom = top + XtHeight (XtParent(dsw)) - 1;
    
    switch (cbs->reason) {
      case DwcTswCRDragStart   :
	force_entry_visible (dsw, entry, DswEVDrag);
	bottom = bottom - MAX (XtHeight (w), DswTimeLabelHeight(dsw, o));
	break;

      case DwcTswCRResizeStart :
	if (DswResizeTop(dsw, o)) {
	    force_entry_visible (dsw, entry, DswEVResizeTop);
	} else {
	    force_entry_visible (dsw, entry, DswEVResizeBottom);
	}

	bottom = bottom - DswTimeLabelHeight(dsw, o);
	if (DswOnTheLine(dsw, o)) {
	    bottom = bottom + DswTopMargin(dsw, o);
	}
	break;
    }
    

    DswAutoScrollTop(dsw, o) =
     top + get_fine_increment_height(dsw) - DswIndexDragOffset(dsw, o);

    if (DswOnTheLine(dsw, o)) {
	DswAutoScrollTop(dsw, o) = DswAutoScrollTop(dsw, o) +
				    DswTopMargin(dsw, o);
    }

    DswAutoScrollTop(dsw, o) =
     MIN (DswAutoScrollTop(dsw, o), event->y_root);


    DswAutoScrollBottom(dsw, o) =
     bottom - get_fine_increment_height(dsw) - DswIndexDragOffset(dsw, o);
       
    DswAutoScrollBottom(dsw, o) =
     MAX (DswAutoScrollBottom(dsw, o), event->y_root);


    XRaiseWindow (XtDisplay (w), XtWindow (w));
    DswOutlineDrawable(dsw, o) = FALSE;

    move_index_time (dsw, time);

    DswOutline (dsw, o) = TRUE;
    DswOutlineX1(dsw, o) = entry->x + 1;
    DswOutlineY1(dsw, o) = TRANSLATE_TIME_TO_Y(entry->start, dsw) + 1;
    DswOutlineX2(dsw, o) = entry->x + entry->width;
    DswOutlineY2(dsw, o) =
      TRANSLATE_TIME_TO_Y(entry->start + entry->duration, dsw) - 1;

    change_outline(dsw);

}

static void TIMESLOT_END_RESIZE_AND_DRAG
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    Cardinal		    duration;
    DwcDswEntry		    entry;
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    int			    time  = DswIndexTime(dsw, o);


    DswTimeslotSequence(dsw, o) ++;
    if (DswIgnoreMotion(dsw, o)) {
	return;
    }
    
    auto_scroll_cancel(dsw);
    erase_outline(dsw);

    switch (cbs->reason) {
      case DwcTswCRResizeEnd :
	entry = DswOpenEntry(dsw, o);
	if (cbs->top_knob) {
	    entry->duration = entry->duration + entry->start - time;
	    entry->start    = time;
	} else {
	    entry->duration = time - entry->start;
	}
	break;

      case DwcTswCRDragEnd   :
	entry = DswDragEntry(dsw, o);
	DswDragEntry(dsw, o) = NULL;
	entry->start = time;
	break;
    }
    
    DswStartDragTime(dsw, o) = - 1;
    move_index_time (dsw, -1);

    if (entry == DswOpenEntry(dsw, o)) {
        /*	  
	**  invert entry
	*/	  
        change_inverted_to (dsw, entry->start, entry->start + entry->duration);
    }
    DswOutline(dsw, o) = FALSE;

    entry->sequence = DswTimeslotSequence(dsw, o);
    DswTimeslotSequence(dsw, o)++;

    sort_entries(dsw);
    arrange_timeslots
    (
	dsw,
	DswOpenEntry(dsw, o),
	MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *)cbs)
    );

}

static void TIMESLOT_CANCEL_RESIZE_AND_DRAG
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);

    DswTimeslotSequence(dsw, o) ++;
    if (DswIgnoreMotion(dsw, o)) {
	return;
    }

    auto_scroll_cancel(dsw);
    erase_outline(dsw);    

    DswStartDragTime(dsw, o) = - 1;

    move_index_time (dsw, -1);
    DswOutline(dsw, o) = FALSE;

    if (DswCancelPosition(dsw, o) != XtY(dsw)) {
	DSWMoveDayslots (dsw, DswCancelPosition(dsw, o));
    }

}

static void move_index
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	int		y)
#else	/* no prototypes */
	(dsw, y)
	DayslotsWidget	dsw;
	int		y;
#endif	/* prototype */
    {
    Position	    y_pos;
    int		    time;
    DwcDswEntry	    entry;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    int		    fine_inc = DswActualFineIncrement(dsw, o);


    y_pos = y + DswIndexDragOffset(dsw, o);
    
    time = MAX (0, translate_y_to_time(dsw, y_pos, FALSE));
    time = MIN (time, 24 * 60);
    
    if (DswIndexResize(dsw, o)) {
        /*	  
	**  We're resizing
	*/	  
        entry = DswOpenEntry(dsw, o);
	if (DswResizeTop(dsw, o)) {
	    if (time >= entry->start + entry->duration) {
		time = entry->start + entry->duration - 1;
	    }
	    time = (time / fine_inc) * fine_inc;

	    DswOutlineY1(dsw, o) = TRANSLATE_TIME_TO_Y(time, dsw) + 1;
	} else {
	    time = MAX (time, entry->start + fine_inc);
	    time = (time / fine_inc) * fine_inc;

	    DswOutlineY2(dsw, o) = TRANSLATE_TIME_TO_Y(time, dsw) - 1;
	}

    } else {
        /*	  
	**  We're dragging
	*/	  
        entry = DswDragEntry(dsw, o);
	if (time > (24 * 60) - entry->duration) {
	    time = (24 * 60) - entry->duration;
	}

	time = (time / fine_inc) * fine_inc;

	DswOutlineY1(dsw, o) = TRANSLATE_TIME_TO_Y(time, dsw) + 1;
	DswOutlineY2(dsw, o) =
	  TRANSLATE_TIME_TO_Y(time + entry->duration, dsw) - 1;
    }

    move_index_time(dsw, time);
    change_outline(dsw);

}

static void TIMESLOT_RESIZE_AND_DRAG
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
    {
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr		    o = DswOffsetPtr(dsw);
    XButtonEvent	    *event = (XButtonEvent *) cbs->event;

    DswTimeslotSequence(dsw, o) ++;
    if (DswIgnoreMotion(dsw, o)) {
	return;
    }

    auto_scroll_to_position(dsw, event->y_root);

}

#define	ColumnsToAllocate   10

static void allocate_more_columns
#if defined(_DWC_PROTO_)
	(
	int		**columns,
	Cardinal	*num_columns,
	DwcDswEntry	***entries,
	Cardinal	**num_entries)
#else	/* no prototypes */
	(columns, num_columns, entries, num_entries)
	int		**columns;
	Cardinal	*num_columns;
	DwcDswEntry	***entries;
	Cardinal	**num_entries;
#endif	/* prototype */
    {
    Cardinal	    c	    = *num_columns;    
    int		    *cols   = *columns;
    Cardinal	    no_c    = *num_columns + ColumnsToAllocate;
    DwcDswEntry	    **ents  = *entries;
    Cardinal	    *no_e   = *num_entries;
        

    cols = (int *) XtRealloc ((char *) cols, sizeof (int) * no_c);

    ents = (DwcDswEntry **) XtRealloc
	((char *) ents, sizeof (DwcDswEntry *) * no_c);

    no_e = (Cardinal *) XtRealloc ((char *) no_e, sizeof (Cardinal) * no_c);

    while (c < no_c)
    {
	cols [c] = - 1;
	ents [c] = NULL;
	no_e [c] = 0;
	c++;
    }

    *columns     = cols;
    *num_columns = no_c;
    *entries     = ents;
    *num_entries = no_e;

}

static void set_dayslots_stacking
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Cardinal	max_columns,
	DwcDswEntry	**entries,
	Cardinal	*num_entries,
	DwcDswEntry	topmost)
#else	/* no prototypes */
	(dsw, max_columns, entries, num_entries, topmost)
	DayslotsWidget	dsw;
	Cardinal	max_columns;
	DwcDswEntry	**entries;
	Cardinal	*num_entries;
	DwcDswEntry	topmost;
#endif	/* prototype */
    {
    DwcDswEntry		entry;
    int			c, i, j;
    Position		x_pos;
    Position		x_offset;
    Dimension		width;
    Arg			arglist [20];
    Cardinal		ac;
    Window		*windows;
    Cardinal		num_windows;
    Cardinal		total_entries;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    XmStringDirection	r_to_l  = DswDirectionRToL(dsw, o);
    Boolean		topdown = DswStackTopDown(dsw, o);


    x_offset = TSWGetTrimmingsWidth (entries [0] [0]->timeslot) + 3;
    x_offset = MIN (x_offset, DswTimeSlotWidth(dsw, o) /
			      ((max_columns + 1) * 2));

    width    = DswTimeSlotWidth(dsw, o) - (max_columns * x_offset) - 2;

    total_entries = 0;
    for (c = 0;  c <= max_columns;  c++) {
	total_entries = total_entries + num_entries [c];
    }

    windows = (Window *) XtMalloc (sizeof (Window) * total_entries);
    num_windows = 0;

    if (topmost != NULL) {
	for (c = max_columns;  c >= 0;  c--) {
	    for (i = 0;  i < num_entries [c];  i++) {
		if (topmost == entries [c] [i]) {
		    if (! XtIsRealized (topmost->timeslot)) {
			XtRealizeWidget (topmost->timeslot);
		    }
		    
		    windows [num_windows] = XtWindow (topmost->timeslot);
		    num_windows++;
		    break; /* Really need to break out of both loops ??? */
		}
	    }
	}
    }
    
    if (topdown) {
	i = 0;
	while (DswEntries(dsw, o) [i] != entries [0] [0])  i++;
	for (j = total_entries - 1;  j >= 0;  j--) {
	    entry = DswEntries(dsw, o) [i + j];
	    if (entry != topmost) {
		if (! XtIsRealized (entry->timeslot)) {
		    XtRealizeWidget (entry->timeslot);
		}
		windows [num_windows] = XtWindow (entry->timeslot);
		num_windows++;
	    }
	}
    }

    for (c = max_columns;  c >= 0;  c--) {
	for (i = 0;  i < num_entries [c];  i++) {

	    entry = entries [c] [i];
	    
	    if (r_to_l)
	    {
		x_pos = DswTimeSlotX(dsw, o) + ((max_columns - c) * x_offset);
	    }
	    else
	    {
		x_pos = DswTimeSlotX(dsw, o) + (c * x_offset);
	    }
	    
	    entry->geometry_changed = entry->geometry_changed  ||
				      (entry->x      != x_pos) ||
				      (entry->width  != width);

	    if (entry->geometry_changed) {
		entry->x      = x_pos;
		entry->width  = width;

		ac = 0;
		XtSetArg(arglist[ac], XmNstringDirection,
			    DswDirectionRToL(dsw, o)); ac++;
		XtSetArg(arglist[ac], XmNx, entry->x); ac++;
		XtSetArg(arglist[ac], XmNy, entry->y); ac++;
		XtSetArg(arglist[ac], XmNwidth, entry->width); ac++;
		XtSetArg(arglist[ac], XmNheight, entry->height); ac++;
		XtSetArg(arglist[ac], DwcTswNfontList, DswFontList(dsw, o)); ac++;
		assert (ac <= XtNumber(arglist));
		XtSetValues (entry->timeslot, arglist, ac);

		entry->geometry_changed = FALSE;
	    }

	    if (! topdown) {
		if (entry != topmost) {
		    if (! XtIsRealized (entry->timeslot)) {
			XtRealizeWidget (entry->timeslot);
		    }
		    
		    windows [num_windows] = XtWindow (entry->timeslot);
		    num_windows++;
		}
	    }
	}
    }

    if (num_windows != 1) {
	XRestackWindows (XtDisplay(dsw), windows, num_windows);
	DswOutlineDrawable(dsw, o) = FALSE;
    }

    XtFree ((char *) windows);

}

static void setup_timebars
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Cardinal	start,
	Cardinal	finish,
	DwcTbwTimeKind	kind)
#else	/* no prototypes */
	(dsw, start, finish, kind)
	DayslotsWidget	dsw;
	Cardinal	start;
	Cardinal	finish;
	DwcTbwTimeKind	kind;
#endif	/* prototype */
    {
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    TBWSetupGidget(DswTimebarGidget(dsw, o), start, finish, kind);

    if (DswWrkhrsTimebar(dsw, o) != NULL) {
	TBWSetupTimebar(DswWrkhrsTimebar(dsw, o), start, finish, kind);
    }
    
}

static void arrange_timeslots
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	DwcDswEntry	topmost,
	Time		time)
#else	/* no prototypes */
	(dsw, topmost, time)
	DayslotsWidget	dsw;
	DwcDswEntry	topmost;
	Time		time;
#endif	/* prototype */
{
    DwcDswEntry		entry;
    int			c, e, i;
    int			start;
    int			duration;
    Boolean		all_empty;
    Position		y_pos;
    Dimension		height;
    XmOffsetPtr		o	     = DswOffsetPtr(dsw);
    int			*columns     = NULL;
    Cardinal		num_columns  = 0;
    Cardinal		max_columns  = 0;
    DwcDswEntry		**entries    = NULL;
    Cardinal		*num_entries = NULL;
    

    if (! DswIsDayNote(dsw, o)) {
	setup_timebars(dsw, 0, 24 * 60, DwcTbwNonWork);
	setup_timebars(dsw, DswWorkStart(dsw, o), DswWorkFinish(dsw, o),
		       DwcTbwWork);
	setup_timebars(dsw, DswLunchStart(dsw, o), DswLunchFinish(dsw, o),
		       DwcTbwNonWork);
    }

    if ((! XtIsRealized ((Widget) dsw)) ||
        (DswNumEntries(dsw, o) == 0)) {
	if (! DswIsDayNote(dsw, o)) {
	    TBWUpdateGidget ((Widget) dsw, DswTimebarGidget(dsw, o));
	    if (DswWrkhrsTimebar(dsw, o) != NULL)
		{
		TBWUpdateTimebar(DswWrkhrsTimebar(dsw, o));
		}
	}
	return;
    }
    
    /*
    **  Loop through all timeslots positioning them appropriately
    */
    for (e = 0;  e < DswNumEntries(dsw, o);  e++) {
	/*
	**  Get the data for this entry
	*/
	entry    = DswEntries(dsw, o) [e];
	start    = (int) entry->start;
	duration = (int) entry->duration;

	if (! DswIsDayNote(dsw, o)) {
	    setup_timebars(dsw, start, start + duration, DwcTbwBusy);
	}
	
	if (TIME_HEIGHT(duration, dsw) < DswTimeslotMinHeight(dsw, o)) {
	    duration = (((DswTimeslotMinHeight(dsw, o) + 1) *
		         DswActualDayslotsSize(dsw, o)) +
			DswTimeLabelHeight(dsw, o) - 1) /
		       DswTimeLabelHeight(dsw, o);
	}


	/*
	**  Look for a column where the time of the end of the previous
	**  entry in the column is before the start of this entry.
	*/
	c = 0;
	while (TRUE) {

	    /*
	    **  If we run out of columns create some more.
	    */
	    if (c == num_columns) {
		allocate_more_columns (&columns, &num_columns,
				     &entries, &num_entries);
	    }

	    /*
	    **  If the start time for this entry is past the end time of the
	    **	previous entry in this column, then use it.  Otherwise move onto
	    **	next column.
	    */
	    if (start > columns [c]) {
		break;
	    }
	    c++;
	}

	/*
	**  Note maximum column used so far....
	*/
	max_columns = MAX (max_columns, c);

	if ((c == 0) && (e != 0)) {
	    all_empty = TRUE;
	    for (i = 1;  i <= max_columns;  i++) {
		all_empty = all_empty && (start > columns [i]);
	    }

	    if (all_empty) {
		set_dayslots_stacking (dsw, max_columns, entries, num_entries,
				     topmost);

		for (i = 0;  i <= max_columns;  i++)
		{
		    XtFree ((char *) entries [i]);
		    entries [i] = NULL;
		    num_entries [i] = 0;
		}

		max_columns = 0;
	    }
	}
	
	/*
	**  Set the end time of this entry into the column.
	*/
	columns [c] = start + duration - 1;
	entries [c] = (DwcDswEntry *) XtRealloc
	(
	    (char *) ((DwcDswEntry *) entries [c]),
	    sizeof (DwcDswEntry) * (num_entries [c] + 1)
	);

	entries [c] [num_entries [c]] = entry;
	num_entries [c]++;
	
	/*
	**  Set the vertical position and height as determined by 'start'
	**  and 'duration'.
	*/
	y_pos  = TRANSLATE_TIME_TO_Y(start, dsw);
	height = TIME_HEIGHT(duration, dsw);

	entry->geometry_changed = entry->geometry_changed  ||
				  (entry->y      != y_pos) ||
				  (entry->height != height);
	entry->y      = y_pos;
	entry->height = height;

    }


    set_dayslots_stacking (dsw, max_columns, entries, num_entries, topmost);

    for (i = 0;  i <= max_columns;  i++)
    {
	XtFree ((char *) entries [i]);
    }

    XtFree ((char *) columns);
    XtFree ((char *) entries);
    XtFree ((char *) num_entries);

    if (topmost != NULL)
    {
	XtManageChild (topmost->timeslot);
    }

    if (! DswIsDayNote(dsw, o)) {
	TBWUpdateGidget ((Widget) dsw, DswTimebarGidget(dsw, o));
	if (DswWrkhrsTimebar(dsw, o) != NULL)
	    {
	    TBWUpdateTimebar(DswWrkhrsTimebar(dsw, o));
	    }
    }

}

static void setup_slot_lines
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    DswSlotLines	*slot_line;
    unsigned char	number_of_styles;
    unsigned char	*lines_per_style;
    unsigned char	*styles;
    XSegment		*segment;
    int			i, j, s;
    int			index;
    Position		left; 
    Position		right; 

    XmOffsetPtr		o          = DswOffsetPtr(dsw);
    int			increment  = DswActualDayslotsSize(dsw, o);
    int			sph        = 60 / increment;
    Position		y          = TRANSLATE_TIME_TO_Y(0000, dsw);
    Dimension		width      = XtWidth(dsw);
    Dimension		height     = DswTimeLabelHeight(dsw, o);
    XmStringDirection	r_to_l	   = DswDirectionRToL  (dsw, o);
    XSegment		**segments = DswSlotLineSegments(dsw, o);
    int			*number_of_segments = &(DswSlotLineNumSegments(dsw, o));
    int			allocation_size;
    int			previous_seg_size;


    if (segments != NULL)
    {
	index = DswSlotLineIndex(dsw, o);
	number_of_styles = slot_lines [index].number_of_styles;
	XtFree ((char *) segments);
    }


    index = 0;
    while (slot_lines [index].increment != increment) index++;

    slot_line = &(slot_lines [index]);
    number_of_styles = slot_line->number_of_styles;
    lines_per_style  = slot_line->lines_per_style;
    styles           = slot_line->styles;


    if (r_to_l)
    {
	left  = 0;
	right = DswDividingLine1X(dsw, o) - 1;
    }
    else
    {
	left  = DswDividingLine1X(dsw, o) + 1;
	right = width;
    }
    
    /*
    ** Alloc the whole chunk in one swell foop.  This requires a two pass to
    ** set this up, but it results in substantial memory overhead reduction.
    ** To get reasonable performance here, XSegment must be a longword multiple,
    ** fortunately it is!!!!!!
    */
    allocation_size = sizeof (XSegment *) * number_of_styles;
    allocation_size += (sizeof (XSegment) * 25);

    for (i = 1;  i < number_of_styles;  i++)
    {
	allocation_size += (sizeof (XSegment) * 24 * lines_per_style [i]);
    }

    /*
    ** Pointer to the whole chunk.
    */
    segments = (XSegment **) XtMalloc (allocation_size);
    previous_seg_size = (sizeof (XSegment *) * number_of_styles);

    /*
    ** First entry points to just after the length of the array.
    */
    segments [0] = (XSegment *)((char *)segments + previous_seg_size);
    number_of_segments [0] = 0;
    previous_seg_size = 25;

    for (i = 1;  i < number_of_styles;  i++)
    {
	/*
	** The location of each segment is the location of the previous segment
	** plus the size of the previous segment.
	*/
	segments [i] = segments[i-1] + previous_seg_size;
	number_of_segments [i] = 0;
	previous_seg_size = 24 * lines_per_style [i];
    }    


    for (i = 0;  i < 24;  i++)
    {
	for (j = 0;  j < sph;  j++)
	{
	    s = styles [j];
	    segment = &(segments [s] [number_of_segments [s]++]);
	    segment->x1 = left;
	    segment->y1 = y;
	    segment->x2 = right;
	    segment->y2 = y;
	    y = y + height;
	}
    }


    segment = &(segments [0] [number_of_segments [0]++]);
    segment->x1 = left;
    segment->y1 = y;
    segment->x2 = right;
    segment->y2 = y;


    DswSlotLineSegments(dsw, o) = segments;
    DswSlotLineIndex  (dsw, o) = index;
 
}

static void display_slot_lines
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	XRectangle	*expose_area,
	Region		region)
#else	/* no prototypes */
	(dsw, expose_area, region)
	DayslotsWidget	dsw;
	XRectangle	*expose_area;
	Region		region;
#endif	/* prototype */
    {
    int		    i;
    int		    first_slot,      last_slot;
    int		    prev_first_slot, prev_last_slot;
    int		    fs, ls;
    int		    entry_start_time, entry_end_time;
    int		    increment;
    XSegment	    *line_segs;
    
    XmOffsetPtr    o                     = DswOffsetPtr(dsw);
    XSegment	    **segments            = DswSlotLineSegments  (dsw, o);
    int		    *number_of_segments   = &(DswSlotLineNumSegments(dsw, o));
    int		    index                 = DswSlotLineIndex     (dsw, o);
    GC		    *gcs                  = DswSlotLineGCs       (dsw, o); 
    Dimension	    height		  = DswTimeLabelHeight   (dsw, o);
    int		    slot_size             = DswActualDayslotsSize(dsw, o);
    Position	    top_y		  = TRANSLATE_TIME_TO_Y(0000, dsw);
    Display	    *d			  = XtDisplay(dsw);
    Window	    w			  = XtWindow (dsw);

    DswSlotLines    *slot_line            = &(slot_lines [index]);
    unsigned char   number_of_styles      = slot_line->number_of_styles;
    unsigned char   *increments_for_style = slot_line->increments_for_style;

    int		    expose_top		  = expose_area->y;
    int		    expose_bottom         = expose_top +expose_area->height -1;


    entry_start_time = (expose_top    - top_y) / (int) height;
    entry_end_time   = (expose_bottom - top_y) / (int) height;

    entry_start_time = MAX (entry_start_time * slot_size, 0);
    entry_end_time   = MIN (entry_end_time   * slot_size, 24 * 60);

    prev_first_slot = 0;
    prev_last_slot  = 0;

    for (i = 0;  i < number_of_styles;  i++) {
	increment = increments_for_style [i];

	if (entry_start_time == 0) {
	    first_slot = 0;
	} else {
	    first_slot = ((entry_start_time - 1) / increment) + 1;
	}

	last_slot = entry_end_time / increment;

	fs = first_slot - prev_first_slot;
	ls = last_slot  - prev_last_slot + 1;
	ls = MIN (ls, number_of_segments [i] - 1);

	line_segs = segments [i];

	while (ls >= 0) {
	    if (line_segs [ls].y1 <= expose_bottom) {
		break;
	    }
	    ls--;
	}

	while (fs <= ls) {
	    if (line_segs [fs].y1 >= expose_top) {
		break;
	    }
	    fs++;
	}

	if (ls >= fs) {
	    MISCSetGCClipMask (d, gcs [i], expose_area, region);
	    XDrawSegments (d, w, gcs [i], &(line_segs [fs]), ls - fs + 1);
	}

	prev_first_slot = first_slot;
	prev_last_slot  = last_slot;

    }

}

static void display_slot_times
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	XRectangle	*expose_area,
	Region		region)
#else	/* no prototypes */
	(dsw, expose_area, region)
	DayslotsWidget	dsw;
	XRectangle	*expose_area;
	Region		region;
#endif	/* prototype */
    {
    Position	    x, y;
    Position	    top_y;
    int		    entry_start_time, entry_end_time;
    XmString	    time_text;
    Dimension	    time_width;
    Cardinal	    i;
    XmString	    xm_str;
    long	    byte_count, cvt_status;
    Position	    pos_y;
    
    XmOffsetPtr    o            = DswOffsetPtr(dsw);
    Position	    time_x	 = DswTimeLabelX	(dsw, o);
    Dimension	    width	 = DswTimeLabelWidth   (dsw, o);
    Dimension	    height	 = DswTimeLabelHeight  (dsw, o);
    int		    slot_size    = DswActualDayslotsSize(dsw, o);
    XmString	    *time_texts = DswRealTimeTexts    (dsw, o);
    Dimension	    *time_widths = DswTimeWidths       (dsw, o);
    XmString	    *min_texts  = DswRealMinuteTexts  (dsw, o);
    Dimension	    *min_widths  = DswMinuteWidths     (dsw, o);
    GC		    fore_gc	 = DswForegroundGC     (dsw, o);
    Display	    *d		 = XtDisplay(dsw);
    Window	    w		 = XtWindow (dsw);
    Boolean	    gc_set	 = FALSE;
    int		    expose_top	 = expose_area->y;
    int		    expose_bottom= expose_top + expose_area->height - 1;
    XFontStruct	    *font;
    Dimension	    time_height;

    top_y = TRANSLATE_TIME_TO_Y(0000, dsw);
    if (DswOnTheLine(dsw, o))
    {
	top_y = top_y - DswTopMargin(dsw, o);
    }

    entry_start_time = (expose_top    - top_y) / (int) height;
    entry_end_time   = (expose_bottom - top_y) / (int) height;

    entry_start_time = MAX (entry_start_time * slot_size, 0);
    if (DswOnTheLine(dsw, o))
    {
	entry_end_time = MIN (entry_end_time * slot_size,  24 * 60);
    }
    else
    {
	entry_end_time = MIN (entry_end_time * slot_size, (24 * 60) - 1);
    }


    if (entry_start_time <= entry_end_time)
    {

	int y_i;

	y = TimeTextYPosition (entry_start_time, dsw);
	y_i = y + MISCFontListAscent(DswFontList(dsw,o));

	MISCGetFontFromFontlist(DswFontList(dsw,o), &font);
	expose_top    = expose_top -
			  MAX (font->max_bounds.descent,
			       font->descent);
        expose_bottom = expose_bottom +
			  MAX (font->max_bounds.ascent,
			       font->ascent);

	for (i = entry_start_time;  i <= entry_end_time ;  i += slot_size)
	{
	    if ((expose_top < y_i) && (expose_bottom > y_i))
	    {

		if ((min_texts != NULL) && ((i % 60) != 0))
		{
		    time_text  = min_texts [i % 60];
		}
		else
		{
		    time_text  = time_texts [i];
		}

		x = time_x;	/* + (width / 2); */

		if (! gc_set)
		{
		    MISCSetGCClipMask (d, fore_gc, expose_area, region);
		    gc_set = TRUE;
		}
		XmStringDrawImage
		(
		    d,
		    w,
		    DswFontList(dsw,o),
		    time_text,
		    fore_gc, 
		    time_x,
		    y,
		    width,
		    XmALIGNMENT_CENTER,
		    XmSTRING_DIRECTION_L_TO_R,
		    NULL
		); 

	    }
	    y += height;
	    y_i += height;
	}
    }

    if (gc_set)
    {
	MISCSetGCClipMask (d, fore_gc, NULL, NULL);
    }
}

static void display_index_time
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	XRectangle	*expose_area)
#else	/* no prototypes */
	(dsw, expose_area)
	DayslotsWidget	dsw;
	XRectangle	*expose_area;
#endif	/* prototype */
    { 
    Cardinal		offset;
    Position		x, y;
    Position		x_t, y_t;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    GC			fore_gc	     = DswIndexForegroundGC(dsw, o);
    GC			back_gc      = DswIndexBackgroundGC(dsw, o);
    int			time	     = DswIndexTime       (dsw, o);
    Dimension		*time_widths = DswTimeWidths      (dsw, o);
    Dimension		width	     = DswTimeLabelWidth  (dsw, o);
    Dimension		height	     = DswTimeLabelHeight (dsw, o);
    XmString		*time_texts = DswRealTimeTexts   (dsw, o);
    Display		*d	     = XtDisplay(dsw);
    Window		w	     = XtWindow (dsw);
    XFontStruct		*font;
    XmString		xm_str;
    long		byte_count, cvt_status;
    Position		pos_y;
    int			time_height;

    x = DswTimeLabelX(dsw, o);
    y_t = TimeTextYPosition (time, dsw);
    y = IndexYPositionForTime (time, dsw);
    width  = width  - 2;
    height = height - 2;

    /*	  
    **  Make sure that the expose area is for the index area
    */	  
    if (expose_area != NULL)
    {
	if ((expose_area->x > x + width)                ||
	    (expose_area->y > y + height)               ||
	    (expose_area->x + expose_area->width  <= x) ||
	    (expose_area->y + expose_area->height <= y))
	{
	    return;
	}
    }

    xm_str = time_texts[time];

    /*	  
    **  Put in the background
    */
    XFillRectangle (d, w, back_gc, x, y, width, height);

    /*
    **  Draw in the time in the index
    */
    XmStringDrawImage
    (
	d,
	w,
	DswFontList(dsw,o),
	xm_str,
	fore_gc,
	x,
	y_t,
	width,
	XmALIGNMENT_CENTER,
	XmSTRING_DIRECTION_L_TO_R,
	NULL
    );

    /*
    **  Let's put a rectangle around it
    */	  
    XDrawRectangle (d, w, fore_gc, x, y, width, height);

}

static void auto_scroll_cancel
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    /*	  
    **  Turn off the specialized dsw auto_scrolling.
    */	  
    DswAutoScrollState(dsw, o) = DswASSNotScrolling;
    /*	  
    **  If we still have a timer event out there then get rid of it.
    */	  
    if (DswAutoScrollTimer(dsw, o) != (XtIntervalId) 0)
    {
	XtRemoveTimeOut (DswAutoScrollTimer(dsw, o));
	DswAutoScrollTimer(dsw, o) = (XtIntervalId) 0;
    }

}

static DwcDswEntry over_which_timeslot
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Position	y1,
	Position	y2)
#else	/* no prototypes */
	(dsw, y1, y2)
	DayslotsWidget	dsw;
	Position	y1;
	Position	y2;
#endif	/* prototype */
    { 
    Cardinal		e;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    Cardinal		num_entries = DswNumEntries(dsw, o);
    DwcDswEntry		*entries    = DswEntries  (dsw, o);


    for (e = 0;  e < num_entries;  e++) {
        /*	  
	**  If y1 is above or the same as the bottom of the entry
	**  and y2 is below or equal to the top of the entry then we're
	**  in the range of that entry.
        */	  
        if ((y1 <= entries [e]->y + entries [e]->height + 2) &&
	    (y2 >= entries [e]->y)) {

	    return (entries [e]);
	}
    }

    return (NULL);
    
}

void DSWDoScrollCallback
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {	
    DSWScrollCallbackStruct    cbs;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			new_y         = XtY(dsw);
    int			height        = (int) XtHeight(dsw);
    int			parent_height = (int) XtHeight (XtParent(dsw));
    int			slot_size     = DswActualDayslotsSize(dsw, o);
    int			label_height  = DswTimeLabelHeight(dsw, o);


    new_y = - new_y - DswTopMargin(dsw, o) - 1;

    cbs.reason    = DwcDswCRScroll;
    cbs.min_value = (- slot_size - 1) / 2;
    cbs.max_value = (24 * 60) + slot_size + cbs.min_value;
    cbs.value     = (new_y * slot_size) / label_height;
    cbs.shown     = (parent_height * ((24 * 60) + slot_size)) / height;
    cbs.inc       = slot_size;
    cbs.pageinc   = MAX (1, cbs.shown - slot_size);


    /*	  
    **  We do the following because the initial value of new_y is a very big
    **	negative.  We want to make sure we're not passing in totally bogus
    **	values.  XUI didn't complain but Motif does.
    */	  
    if (cbs.value > (cbs.max_value - cbs.shown))
	cbs.value = cbs.max_value - cbs.shown;

    if (cbs.value < (cbs.min_value))
	cbs.value = cbs.min_value;


    XtCallCallbackList ((Widget) dsw, DswScrollCallback(dsw,o), &cbs);

}

void DSWMoveDayslotsToTime
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	int	time)
#else	/* no prototypes */
	(dsw, time)
	DayslotsWidget	dsw;
	int	time;
#endif	/* prototype */
    { 
    int			new_y;
    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			slot_size     = DswActualDayslotsSize(dsw, o);
    int			label_height  = DswTimeLabelHeight(dsw, o);

    new_y = ((time * label_height) / slot_size) + DswTopMargin(dsw, o) + 1;

    DSWMoveDayslots (dsw, - new_y);

}

void DSWMoveDayslots
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Position	new_y)
#else	/* no prototypes */
	(dsw, new_y)
	DayslotsWidget	dsw;
	Position	new_y;
#endif	/* prototype */
    { 
    XRectangle		expose_area;
    XtWidgetGeometry	request;
    XtGeometryResult	result;
    DwcDswEntry		entry;
    Position		now_y;
    Position		old_y = XtY(dsw);
    Dimension		border_width  = XtBorderWidth(dsw);
    Dimension		height        = XtHeight(dsw);
    Dimension		parent_height = XtHeight (XtParent(dsw));
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    now_y = new_y;
    now_y = MIN (now_y, - (int) border_width);
    now_y = MAX (now_y, - (int) border_width - (int) height +
			  (int) parent_height);

    if (now_y != old_y) {

	expose_area.x      = 0;
	expose_area.width  = XtWidth(dsw);

	if (now_y < old_y) {
	    expose_area.y = - old_y + parent_height - (int) border_width;
	    expose_area.height = old_y - now_y;
	} else {
	    expose_area.y = - now_y - (int) border_width;
	    expose_area.height = now_y - old_y;
	}


	entry = over_which_timeslot (dsw, expose_area.y, expose_area.y +
						  expose_area.height - 1);

	if (entry != NULL) {
	    if (DswOutlineVisible(dsw, o)) {
		if ((DswOutlineVisY1(dsw, o) <= entry->y + entry->height +2) &&
		    (DswOutlineVisY2(dsw, o) >= entry->y)) {
		    erase_outline(dsw);
		}
	    }

	    DswOutlineDrawable(dsw, o) = FALSE;
	    DswOutlineEventSeq(dsw, o) = DswOutlineEventSeq(dsw, o) + 1;

	}

	DswAutoScrollOffset(dsw, o) =
	 DswAutoScrollOffset(dsw, o) - (old_y - now_y);

	request.y	     = now_y;
	request.request_mode = CWY;

	send_client_message (dsw, DswCMExposeEvent, &expose_area);
	result = XtMakeGeometryRequest ((Widget)dsw, &request, NULL);
	DswAutoScrollState(dsw, o) = DswASSScrolling;
	send_client_message (dsw, DswCMNoExposeEvent, NULL);

	display_dayslots(dsw, &expose_area, NULL, FALSE, TRUE);
	DSWDoScrollCallback(dsw);
    }

}

static void force_entry_visible
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	DswEVKind	kind)
#else	/* no prototypes */
	(dsw, entry, kind)
	DayslotsWidget	dsw;
	DwcDswEntry	entry;
	DswEVKind	kind;
#endif	/* prototype */
    { 
    Position		y_start;
    Position		y_end;
    Position		dsw_y;
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    switch (kind) {
      case DswEVText :
	y_start = XtY (entry->timeslot) - 1;
	y_end   = XtY (entry->timeslot) + XtHeight (entry->timeslot) +
		   (XtBorderWidth (entry->timeslot) * 2);
        break;

      case DswEVDrag :
        y_start = IndexYPositionForTime (entry->start, dsw) -
		   get_fine_increment_height(dsw);
	y_end   =
	  XtY (entry->timeslot) +
	  MAX (XtHeight (entry->timeslot), DswTimeLabelHeight(dsw, o)) +
	  (XtBorderWidth (entry->timeslot) * 2) +
	  get_fine_increment_height(dsw);
        break;

      case DswEVResizeTop :
        y_start = IndexYPositionForTime (entry->start, dsw);
	y_end   = y_start + DswTimeLabelHeight(dsw, o) +
		   get_fine_increment_height(dsw);
        y_start = y_start - get_fine_increment_height(dsw);
        break;

      case DswEVResizeBottom :
        y_start = IndexYPositionForTime (entry->start + entry->duration, dsw);
	y_end   = y_start + DswTimeLabelHeight(dsw, o) +
		   get_fine_increment_height(dsw);
        y_start = y_start - get_fine_increment_height(dsw);
        break;
    }

    dsw_y  = XtY(dsw);

    if (y_end > (int) XtHeight (XtParent(dsw)) - dsw_y) {
	dsw_y = - (y_end + 1 - (int) XtHeight (XtParent(dsw)));
    }
    
    if (y_start < - dsw_y) {
	dsw_y = - y_start;
    }

    dsw_y = MIN (dsw_y, - (int) XtBorderWidth(dsw));
    dsw_y = MAX (dsw_y, - (int) XtBorderWidth(dsw) - (int) XtHeight(dsw) +
			  (int) XtHeight (XtParent(dsw)));

    DswIndexDragOffset(dsw, o) = DswIndexDragOffset(dsw, o) -
				   (XtY(dsw) - dsw_y);

    DSWMoveDayslots (dsw, dsw_y);
    
}

#define MAX_OFFSET	500
#define	OFFSET_DIVISOR	2

static void auto_scroll_to_position
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	int		y)
#else	/* no prototypes */
	(dsw, y)
	DayslotsWidget	dsw;
	int		y;
#endif	/* prototype */
    { 
    XtWidgetGeometry	request;
    XtGeometryResult	result;
    Position		y_pos;
    int			offset;

    XmOffsetPtr		o = DswOffsetPtr(dsw);
    int			fine_inc      = DswActualFineIncrement(dsw, o);
    int			time_height   = DswTimeLabelHeight(dsw, o);
    int			slot_size     = DswActualDayslotsSize(dsw, o);
    Position	        dsw_y         = XtY(dsw);
    int			dsw_height    = XtHeight(dsw);
    int			parent_height = XtHeight (XtParent(dsw));
    int			border_width  = XtBorderWidth(dsw);
    Boolean		need_scroll   = FALSE;


    y_pos = y;
    DswAutoScrollLastY(dsw, o) = y;
    
    if ((dsw_y < - border_width) && (y < DswAutoScrollTop(dsw, o))) {

	offset = y - DswAutoScrollTop(dsw, o);
	y_pos  = DswAutoScrollTop(dsw, o);
	need_scroll = TRUE;

    } else {

	if ((dsw_y > parent_height - dsw_height - border_width) &&
	    (y     > DswAutoScrollBottom(dsw, o))) {

	    offset = y - DswAutoScrollBottom(dsw, o);
	    y_pos  = DswAutoScrollBottom(dsw, o);
	    need_scroll = TRUE;
	}

    }


    if (need_scroll) {

	if ((DswAutoScrollTimer(dsw, o) != (XtIntervalId) 0) ||
	    (DswAutoScrollState(dsw, o) != DswASSNotScrolling)) {
	    return;
	}

	if (offset > 0) {
	    offset = MIN (offset,  MAX_OFFSET);
	} else {
	    offset = MAX (offset, -MAX_OFFSET);
	}

	offset = offset / OFFSET_DIVISOR;
	
	if (dsw_y - offset > - border_width)
	{
	    offset = dsw_y + border_width;
	}
	else
	{
	    if (dsw_y - offset < parent_height - dsw_height - border_width)
	    {
		offset = dsw_y - (parent_height - dsw_height - border_width);
	    }
	}	

	if (offset != 0)
	{

	    DSWMoveDayslots (dsw, (dsw_y - offset));
	    DswAutoScrollTimer(dsw, o) = XtAppAddTimeOut
	    (
		XtWidgetToApplicationContext ((Widget) dsw),
		AutoScrollTimerInterval,
		(XtTimerCallbackProc) auto_scroll_timer_tick,
		(caddr_t) dsw
	    );
	}

    }
    else
    {

	if (DswAutoScrollTimer(dsw, o) != (XtIntervalId) 0)
	{
	    auto_scroll_cancel(dsw);
	}

    }

    y_pos = y_pos - DswAutoScrollOffset(dsw, o);

    switch (DswAutoScrollFunction(dsw, o)) {

      case DswASFChangeInverted :
	change_inverted(dsw, y_pos);
	break;

      case DswASFMoveIndex :
	move_index(dsw, y_pos);
	break;
    }

}

static void auto_scroll_timer_tick
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	XtIntervalId	*id)
#else	/* no prototypes */
	(dsw, id)
	DayslotsWidget	dsw;
	XtIntervalId	*id;
#endif	/* prototype */
    {
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    DswAutoScrollTimer(dsw, o) = (XtIntervalId) 0;
    if (DswAutoScrollState(dsw, o) == DswASSNotScrolling) {
	auto_scroll_to_position(dsw, DswAutoScrollLastY(dsw, o));
    } else {
	DswAutoScrollState(dsw, o) = DswASSNeedScroll;
    }

}

static void send_client_message
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	DswCMKind	kind,
	XRectangle	*rect)
#else	/* no prototypes */
	(dsw, kind, rect)
	DayslotsWidget	dsw;
	DswCMKind	kind;
	XRectangle	*rect;
#endif	/* prototype */
    {
    XClientMessageEvent	    cmev;
    Status		    status;
    XmOffsetPtr	    o = DswOffsetPtr(dsw);


    switch (kind) {
      case DswCMOutlineEvent :
	if (DswOutlineEventSeq(dsw, o) == DswOutlineEventLast(dsw, o)) {
	    return;
	}

	DswOutlineEventLast(dsw, o) = DswOutlineEventSeq(dsw, o);
	cmev.data.l [1] = DswOutlineEventSeq(dsw, o);
	break;

      case DswCMExposeEvent :
	/* we want to send the result of the expose event to the server	    */
	/* before we get the expose event so that we can can save time in   */
	/* moving and not have to wait for to get the expose event from the */
	/* server */
	cmev.data.l [1] = (int) rect->x;
	cmev.data.l [2] = (int) rect->y;
	cmev.data.l [3] = (int) rect->width;
	cmev.data.l [4] = (int) rect->height;
	break;

      case DswCMNoExposeEvent :
	break;

    }
    
    
    cmev.type	 = ClientMessage;
    cmev.display = XtDisplay(dsw);
    cmev.window  = XtWindow(dsw);
    cmev.message_type = XA_INTEGER;
    cmev.format  = 32;

    cmev.data.l [0] = (int) kind;
    
    /* Send ourselves the message */
    XSendEvent
    (
	XtDisplay(dsw),
	XtWindow(dsw),
	FALSE,		    /* don't propagate */
	(long) XtBuildEventMask ((Widget) dsw),
	(XEvent *) &cmev
    );

    /* Let's make sure the remote server handles this in a timely fashion */
    XFlush(XtDisplay(dsw));

}

/* ClientMessage event handler for the dayslotswidget */
static void receive_client_message
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget		dsw,
	caddr_t			client_data,
	XClientMessageEvent	*event)
#else	/* no prototypes */
	(dsw, client_data, event)
	DayslotsWidget		dsw;
	caddr_t			client_data;
	XClientMessageEvent	*event;
#endif	/* prototype */
    {
    XPoint		points [4];
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    if (event->type != ClientMessage) {
	return;
    }
    
    switch ((DswCMKind) event->data.l [0]) {

      case DswCMOutlineEvent :

	if (event->data.l [1] >= DswOutlineEventSeq(dsw, o)) {

	    DswOutlineEventSeq(dsw, o) = DswOutlineEventSeq(dsw, o) + 1;
	    DswOutlineDrawable(dsw, o) = TRUE;

	    if (DswOutline(dsw, o) && ! DswOutlineVisible(dsw, o)) {
		draw_outline(dsw);
	    }
	}
	break;


      case DswCMExposeEvent :
	/* We're short circuiting the round-trip between our move message   */
	/* to the server and it's expose to us. So, we've passed ourself    */
	/* the rectangle that will be exposed. */

	points [0].x = (short)  event->data.l [1];
	points [0].y = (short)  event->data.l [2];

	points [1].x = (short) (event->data.l [1] + event->data.l [3]);
	points [1].y = (short)  event->data.l [2];

	points [2].x = (short) (event->data.l [1] + event->data.l [3]);
	points [2].y = (short) (event->data.l [2] + event->data.l [4]);

	points [3].x = (short)  event->data.l [1];
	points [3].y = (short) (event->data.l [2] + event->data.l [4]);

	DswNoexposeRegion(dsw, o) = XPolygonRegion (points, 4, EvenOddRule);
	break;

      case DswCMNoExposeEvent :

	XDestroyRegion (DswNoexposeRegion(dsw, o));
	DswNoexposeRegion(dsw, o) = NULL;
	if (DswAutoScrollState(dsw, o) == DswASSNeedScroll) {
	    DswAutoScrollState(dsw, o) = DswASSNotScrolling;
	    auto_scroll_to_position(dsw, DswAutoScrollLastY(dsw, o));
	} else {
	    DswAutoScrollState(dsw, o) = DswASSNotScrolling;
	}
	break;

    }

}

static void invert_outline
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	XRectangle	*expose_area,
	Region		region,
	Position	x1,
	Position	y1,
	Position	x2,
	Position	y2)
#else	/* no prototypes */
	(dsw, expose_area, region, x1, y1, x2, y2)
	DayslotsWidget	dsw;
	XRectangle	*expose_area;
	Region		region;
	Position	x1;
	Position	y1;
	Position	x2;
	Position	y2;
#endif	/* prototype */
    {
    XmOffsetPtr    o = DswOffsetPtr(dsw);
    GC		    outline_gc = DswOutlineGC(dsw, o);
    Display	    *d	       = XtDisplay(dsw);
    Window	    w	       = XtWindow (dsw);


    if (! DswIsDayNote(dsw, o)) {
	MISCSetGCClipMask (d, outline_gc, expose_area, region);

	XDrawRectangle (d, w, outline_gc, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
    }
    
}

static void draw_outline
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr    o = DswOffsetPtr(dsw);

    invert_outline (dsw, NULL, NULL,
		   DswOutlineX1(dsw, o), DswOutlineY1(dsw, o),
		   DswOutlineX2(dsw, o), DswOutlineY2(dsw, o));

    DswOutlineVisX1(dsw, o) = DswOutlineX1(dsw, o);
    DswOutlineVisY1(dsw, o) = DswOutlineY1(dsw, o);
    DswOutlineVisX2(dsw, o) = DswOutlineX2(dsw, o);
    DswOutlineVisY2(dsw, o) = DswOutlineY2(dsw, o);

    DswOutlineVisible(dsw, o) = TRUE;

}

static void erase_outline
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr    o = DswOffsetPtr(dsw);


    if (DswOutlineVisible(dsw, o)) {
	invert_outline(dsw, NULL, NULL,
		       DswOutlineVisX1(dsw, o), DswOutlineVisY1(dsw, o),
		       DswOutlineVisX2(dsw, o), DswOutlineVisY2(dsw, o));
	DswOutlineVisible(dsw, o) = FALSE;
    }


}

static void change_outline
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    DwcDswEntry		entry;
    XmOffsetPtr		o = DswOffsetPtr(dsw);


    if (DswIsDayNote(dsw, o)) {
	return;
    }
    
    if ((DswOutline (dsw, o) == DswOutlineVisible(dsw, o)) &&
	(DswOutlineX1(dsw, o) == DswOutlineVisX1 (dsw, o)) &&
	(DswOutlineY1(dsw, o) == DswOutlineVisY1 (dsw, o)) &&
	(DswOutlineX2(dsw, o) == DswOutlineVisX2 (dsw, o)) &&
	(DswOutlineY2(dsw, o) == DswOutlineVisY2 (dsw, o))) {
	return;
    }

    erase_outline(dsw);

    if (! DswOutline(dsw, o)) {
	return;
    }
    

    if (DswOutlineDrawable(dsw, o)) {

	draw_outline(dsw);

    } else {

	entry = over_which_timeslot (dsw, DswOutlineY1(dsw, o),
				   DswOutlineY2(dsw, o));

	if (entry != NULL) {
	    send_client_message (dsw, DswCMOutlineEvent, NULL);
	} else {
	    draw_outline(dsw);
	    DswOutlineEventSeq(dsw, o) = DswOutlineEventSeq(dsw, o) + 1;
	}
    }

}

void DSWSetWorkHours
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Cardinal	start,
	Cardinal	finish)
#else	/* no prototypes */
	(dsw, start, finish)
	DayslotsWidget	dsw;
	Cardinal	start;
	Cardinal	finish;
#endif	/* prototype */
    {    
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    DswWorkStart(dsw, o) = start;
    DswWorkFinish(dsw, o) = finish;

}

void DSWSetEntrySelected
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry,
	Boolean		selected)
#else	/* no prototypes */
	(entry, selected)
	DwcDswEntry	entry;
	Boolean		selected;
#endif	/* prototype */
    {

    if (entry == NULL) {
	return;
    }

    /*	  
    **  Tell the timeslotwidget to select this entry
    */	  
    TSWSetSelected (entry->timeslot, selected);
    
}

Widget DSWGetEntryTextWidget
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry)
#else	/* no prototypes */
	(entry)
	DwcDswEntry	entry;
#endif	/* prototype */
    {

    if (entry == NULL) {
	return (NULL);
    }

    return (TSWGetTextWidget (entry->timeslot));
    
}

Boolean DSWCheckValues
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw,
	Pixmap		*pixmaps,
	XmFontList	fontlist,
	int		time_v_margin,
	int		slot_size,
	int		fine_increment)
#else	/* no prototypes */
	(dsw, pixmaps, fontlist, time_v_margin, slot_size, fine_increment)
	DayslotsWidget	dsw;
	Pixmap		*pixmaps;
	XmFontList	fontlist;
	int		time_v_margin;
	int		slot_size;
	int		fine_increment;
#endif	/* prototype */
    {
    Arg		    arglist[10];
    Cardinal	    ac;
    Widget	    timeslot;
    Window	    pm_root;
    int		    pm_x, pm_y;
    unsigned int    pm_border, pm_depth;
    int		    label_height;
    int		    label_height_multiple;
    int		    timeslot_min_height;
    int		    time_label_height;
    int		    height;
    XmOffsetPtr     o = DswOffsetPtr(dsw);
    XFontStruct	    *font;

    /*
    **  Now we create a timeslot
    */
    ac = 0;
    XtSetArg(arglist[ac], DwcTswNpixmapWidth, DswPixmapWidth(dsw,o)); ac++;
    XtSetArg(arglist[ac], DwcTswNpixmapHeight, DswPixmapHeight(dsw,o)); ac++;
    XtSetArg(arglist[ac], XmNheight, 0); ac++;
    XtSetArg(arglist[ac], XmNwidth, 100); ac++;
    XtSetArg(arglist[ac], DwcTswNfontList, fontlist); ac++;
    assert (ac <= XtNumber(arglist));
    timeslot = TSWTimeslotCreate ((Widget) dsw, "Entry", arglist, ac);

    /*
    **  We use its height as a minimun.  We no longer need this timeslot
    **	widget now....
    */
    
    timeslot_min_height = XtHeight (timeslot);

    XtDestroyWidget (timeslot);
    
    /*
    **  If we have 30 minute slots and a 5 minute fine increment, then our
    **	timeslots must be a multiple of 6 pixels high.
    */
    
    label_height_multiple = slot_size / fine_increment;

    /*
    **  To get the label height, use the larger of the ascent + descent
    **	parameters in the font.
    */
    
    MISCGetFontFromFontlist(fontlist, &font);
    label_height =
      MAX (font->max_bounds.ascent,  font->ascent)  +
      MAX (font->max_bounds.descent, font->descent) + 1;

    /*
    **  Label must be at least the minimum timeslot height and must be of
    **	appropriate multiple
    */

    label_height = MAX (label_height, timeslot_min_height + 1);
    label_height = label_height_multiple *
		   (((label_height - 1) / label_height_multiple) + 1);

    /*
    **  Add in extra spacing, and figure height for all timeslots.  Let caller
    **	know if it will work.
    */
    
    time_label_height = label_height + (time_v_margin * label_height_multiple);

    height = ((24 * 60 / slot_size) + 1) * time_label_height;

    return (height <= 32767);

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      create_fore_back_gcs
**
**  FORMAL PARAMETERS:
**
**      DayslotsWidget	dsw;
**
**  RETURN VALUE:
**
**      NONE
**
**  SIDE EFFECTS:
**
**      Sets DswForegroundGC and DswBackgroundGC for the specified dayslots
**	widget. Frees previously existing ones.
**
**  DESIGN:
**
**      {@description or none@}
**
**  [@logical properties@]...
**
**  IMPLICIT INPUT PARAMETERS:
**   
**      Expects DswGrayPixmap(dsw,o), DswFontList(dsw,o),
**	DswForegroundPixel(dsw,o), DswBackgroundPixmap(dsw,o) to have been set
**	prior to this routine being called.
**   
**  IMPLICIT OUTPUT PARAMETERS:
**
**	Sets DswForegroundGC(dsw,o) and DswBackgroundGC(dsw,o).
**--
*/
static void create_fore_back_gcs
#if defined(_DWC_PROTO_)
	(
	DayslotsWidget	dsw)
#else	/* no prototypes */
	(dsw)
	DayslotsWidget	dsw;
#endif	/* prototype */
    {
    XmOffsetPtr	    o = DswOffsetPtr(dsw);
    XGCValues	    values;
    int		    f_bit;
    XFontStruct	    *font;
    f_bit = 0;

    /*	  
    **  Free the previous ones if they exist
    */	  
    if (DswForegroundGC(dsw,o) != NULL)
    {
	XtReleaseGC ((Widget)dsw, DswForegroundGC(dsw,o));
    }
    if (DswBackgroundGC(dsw,o) != NULL)
    {
	XtReleaseGC ((Widget)dsw, DswBackgroundGC(dsw,o));
    }

    /* copied from dwtcommon.c */
    /* Font. let the default font be the first one. ABK */
    if (DswFontList(dsw,o) != NULL)
    {
	MISCGetFontFromFontlist(DswFontList(dsw,o), &font);    
	values.font       = font->fid;
	f_bit = GCFont;
    }

    /* set line width to 0 for server speed */
    values.line_width = 0;
    values.foreground = DswForegroundPixel(dsw,o);
    values.background = DswBackgroundPixel(dsw,o);
    values.tile       = DswGrayPixmap(dsw,o);
    values.fill_style = FillTiled;
    values.dashes     = 1;

    if (XtSensitive(dsw))
	DswForegroundGC(dsw,o) = XtGetGC
	    ((Widget) dsw, (forGCmask | f_bit), &values);
    else
	DswForegroundGC(dsw,o) = XtGetGC
	    ((Widget) dsw, (formapGCmask | f_bit), &values);    

    values.foreground = DswBackgroundPixel(dsw,o);
    values.background = DswForegroundPixel(dsw,o);
    values.tile       = DswBackgroundPixmap(dsw,o);

    /* this is to catch the parent relative and other x constants */
    if (DswBackgroundPixmap(dsw,o) > 10)
        DswBackgroundGC(dsw,o) = XtGetGC
	    ((Widget) dsw, (backmapGCmask | f_bit), &values);
    else
        DswBackgroundGC(dsw,o) = XtGetGC
	    ((Widget) dsw, (backGCmask | f_bit), &values);

}

Widget DSWGetEntryTimeslotWidget
#if defined(_DWC_PROTO_)
	(
	DwcDswEntry	entry)
#else	/* no prototypes */
	(entry)
	DwcDswEntry	entry;
#endif	/* prototype */
{

    if (entry == NULL)
    {
	return (NULL);
    }

    return (entry->timeslot);
    
}

static void TIMESLOT_INCR_SIZE
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
{
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr		    o = DswOffsetPtr(dsw);
    XButtonEvent	    *event = (XButtonEvent *) cbs->event;
    DwcDswEntry		    entry;
    int			    sign;
    int			    increment;
    int			    new_start, new_duration;

    /*
    ** Determine which direction we moved and how far.
    */
    entry = DswOpenEntry(dsw,o);
    sign = (cbs->reason > 0) ? 1 : -1;
    increment = ((cbs->reason * sign) == 1) ?
	DswActualFineIncrement(dsw,o) :
	DswActualDayslotsSize(dsw,o);

    /*
    ** Get the modified location and size.  If we are moving the top, both
    ** change.  If the bottom, only the duration.
    */
    if (cbs->top_knob)
    {
	new_duration = entry->duration - (increment * sign);
	new_start = entry->start + (increment * sign);
    }
    else
    {
	new_start = entry->start;
	new_duration = entry->duration + (increment * sign);
    }

    /*
    ** Test for boundary conditions.
    */
    if ((new_start < 0) ||
	(new_duration <= DswActualFineIncrement(dsw,o)) ||
	((new_start + new_duration) > 60 * 24)) return;

    /*
    ** OK, we got there, record the change.
    */
    entry->duration = new_duration;
    entry->start = new_start;    

    /*
    ** Set the area that indicates the open timeslot within the dayslots
    ** widget.
    */
    change_inverted_to (dsw, entry->start, entry->start + entry->duration);

    /*
    ** Redisplay the timeslots.
    */
    sort_entries (dsw);

    arrange_timeslots
    (
	dsw,
	DswOpenEntry(dsw,o),
	MISCGetTimeFromAnyCBS((XmAnyCallbackStruct *)cbs)
    );

    /*
    ** Keep this entry on the screen.
    */
    force_entry_visible (dsw, entry, DswEVText);
}

static void TIMESLOT_INCR_POSI
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcTswCallbackStructPtr	cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcTswCallbackStructPtr	cbs;
#endif	/* prototype */
{
    DayslotsWidget	    dsw = (DayslotsWidget) tag;
    XmOffsetPtr		    o = DswOffsetPtr(dsw);
    XButtonEvent	    *event = (XButtonEvent *) cbs->event;
    DwcDswEntry		    entry;
    int			    sign;
    int			    increment;
    int			    new_start;

    /*
    ** Determine which direction we moved and how far.
    */
    entry = DswOpenEntry(dsw,o);
    sign = (cbs->reason > 0) ? 1 : -1;
    increment = ((cbs->reason * sign) == 1) ?
	DswActualFineIncrement(dsw,o) :
	DswActualDayslotsSize(dsw,o);

    /*
    ** Get the modified location.
    */
    new_start = entry->start + (increment * sign);

    /*
    ** Test for boundary conditions.
    */
    if ((new_start < 0) || ((new_start + entry->duration) > 60 * 24)) return;

    /*
    ** OK, we got there, record the change.
    */
    entry->start = new_start;

    /*
    ** Set the area that indicates the open timeslot within the dayslots
    ** widget.
    */
    change_inverted_to (dsw, entry->start, entry->start + entry->duration);

    /*
    ** Redisplay the timeslots.
    */
    sort_entries (dsw);

    arrange_timeslots
    (
	dsw,
	DswOpenEntry(dsw,o),
	MISCGetTimeFromAnyCBS((XmAnyCallbackStruct *)cbs)
    );

    /*
    ** Keep this entry on the screen.
    */
    force_entry_visible (dsw, entry, DswEVText);

}

Boolean DSWTraverseToOpen
#ifdef _DWC_PROTO_
	(
	DayslotsWidget	    dsw
	)
#else
	(dsw)
	DayslotsWidget	    dsw;
#endif
{
    DwcDswEntry		entry;
    XmOffsetPtr		o = DswOffsetPtr(dsw);

    entry = DswOpenEntry(dsw,o);

    if (entry == NULL) return (False);

    return (TSWTraverseToText (entry->timeslot));

}

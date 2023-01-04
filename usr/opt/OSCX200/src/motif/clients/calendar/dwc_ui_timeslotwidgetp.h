#ifndef _timeslotwidgetp_h_
#define _timeslotwidgetp_h_
/*  $Header$ */
/* #module dwc_ui_timeslotwidgetp.h */
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
**	Marios Cleovoulou, December-1988
**
**  ABSTRACT:
**
**	This include file contains the private structures etc, for the timeslot
**	widget. 
**
**--
*/

#ifdef vaxc
#pragma nostandard
#endif
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>
#include <Xm/XmP.h>
#if (((XmVERSION == 1) && (XmREVISION == 2)) || XmVERSION == 2)
#include <Xm/ManagerP.h>
#endif
#include <Xm/TextP.h>
#include <Xm/SashP.h>
#include <DXm/DXmCSTextP.h>
#ifdef vaxc
#pragma standard
#endif

#define	TimeslotIndex    (XmManagerIndex + 1)

/*
**  Instance Part Record
*/

typedef struct
{
    Dimension		pixmap_width ;
    Dimension		pixmap_height ;
    Dimension		pixmap_margin ;
    
    Cardinal		pixmap_count ;
    Pixmap		*pixmaps ;
    
    Dimension		knob_size ;

    XtCallbackList	help_callback;
    XtCallbackList	single_callback;
    XtCallbackList	double_callback;
    XtCallbackList	drag_start_callback;
    XtCallbackList	drag_callback;
    XtCallbackList	drag_end_callback;
    XtCallbackList	drag_cancel_callback;
    XtCallbackList	resize_start_callback;
    XtCallbackList	resize_callback;
    XtCallbackList	resize_end_callback;
    XtCallbackList	resize_cancel_callback;
    XtCallbackList	focus_callback;
    XtCallbackList	losingfocus_callback;
    XtCallbackList	close_callback;
    XtCallbackList	extend_select_callback;
    XtCallbackList	incr_size_callback;
    XtCallbackList	incr_posi_callback;

    Boolean		editable;
    Boolean		dragable;
    
    Boolean		selected;
    Boolean		opened;
    Boolean		resizing;
    Boolean		dragging;
    Boolean		top_knob;

    DXmCSTextWidget	text_widget;

    Boolean		text_changed ;
    
    Pixmap		*real_pixmaps ;
    Cardinal		pixmap_rows ;

    Position		pixmap_area_x ;
    Dimension		pixmap_area_width ;

    Position		knobs_leftmargin_x ;
    Position		knobs_top_of_lower_y ;

    Position		text_area_x ;
    Dimension		text_area_width ;

    caddr_t		mba_context ;

    GC			background_gc;
    GC			foreground_gc;
    Pixmap		gray_pixmap;

    XmFontList		fontlist;

    XmSashWidget	top_sash;
    XmSashWidget	bottom_sash;
    XmSashWidget	move_sash;

} TimeslotPart ;

/*
**  Instance record
*/

typedef struct _TimeslotRec {
    CorePart		core ;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    TimeslotPart	timeslot ;
} TimeslotRec;

/*
**  Class part record
*/

typedef struct _TimeslotClassPart {
    XmOffsetPtr		timeslotoffsets ;
    int			reserved ;
} TimeslotClassPart ;

/*
**  Class record
*/

typedef struct _TimeslotClassRec {
    CoreClassPart	core_class ;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    XmManagerClassPart	manager_class;
    TimeslotClassPart	timeslot_class ;
} TimeslotClassRec;

extern TimeslotClassRec timeslotClassRec;

#define	TswOffsetPtr(w) \
(((TimeslotWidgetClass) ((TimeslotWidget) w)->core.widget_class)->timeslot_class.timeslotoffsets)

#define	TswBackgroundPixel(w, o) XmField (w,o,Core,background_pixel,Pixel)
#define	TswForegroundPixel(w, o) XmField (w,o,XmManager,foreground,Pixel)

#define	TswPixmapMargin(w, o)	    XmField (w,o,Timeslot,pixmap_margin,Dimension)
#define	TswKnobSize(w, o)	    XmField (w,o,Timeslot,knob_size,Dimension)
#define	TswSelected(w, o)	    XmField (w,o,Timeslot,selected,Boolean)
#define	TswOpened(w, o)		    XmField (w,o,Timeslot,opened,Boolean)
#define	TswResizing(w, o)	    XmField (w,o,Timeslot,resizing,Boolean)
#define	TswDragging(w, o)	    XmField (w,o,Timeslot,dragging,Boolean)
#define	TswTopKnob(w, o)	    XmField (w,o,Timeslot,top_knob,Boolean)
#define	TswTextWidget(w, o)	    XmField (w,o,Timeslot,text_widget,Widget)
#define	TswTextChanged(w, o)	    XmField (w,o,Timeslot,text_changed,Boolean)
#define	TswPixmapWidth(w, o)	    XmField (w,o,Timeslot,pixmap_width,Dimension)
#define	TswPixmapHeight(w, o)	    XmField (w,o,Timeslot,pixmap_height,Dimension)
#define	TswPixmapCount(w, o)	    XmField (w,o,Timeslot,pixmap_count,Cardinal)
#define	TswPixmaps(w, o)	    XmField (w,o,Timeslot,pixmaps,Pixmap *)

#define	TswRealPixmaps(w, o)	    XmField (w,o,Timeslot,real_pixmaps,Pixmap *)

#define	TswPixmapRows(w, o)	    XmField (w,o,Timeslot,pixmap_rows,Cardinal)

#define	TswPixmapAreaX(w, o)	    XmField (w,o,Timeslot,pixmap_area_x,Position)
#define	TswPixmapAreaWidth(w, o)    XmField (w,o,Timeslot,pixmap_area_width,Dimension)

#define	TswKnobsLeftMarginX(w, o)	    XmField (w,o,Timeslot,knobs_leftmargin_x,Position)
#define	TswKnobsTopOfLowerY(w, o)	    XmField (w,o,Timeslot,knobs_top_of_lower_y,Position)

#define	TswTextAreaX(w, o)	    XmField (w,o,Timeslot,text_area_x,Position)
#define	TswTextAreaWidth(w, o)      XmField (w,o,Timeslot,text_area_width,Dimension)

#define	TswBackgroundGC(w, o)	 XmField (w,o,Timeslot,background_gc, GC)
#define	TswForegroundGC(w, o)	 XmField (w,o,Timeslot,foreground_gc, GC)
#define	TswDirectionRToL(w, o)	 XmField (w,o,XmManager,string_direction,XmStringDirection)
#define TswBackgroundPixmap(w,o)    XmField(w,o,Core,background_pixmap,Pixmap)
#define	TswGrayPixmap(w, o)	 XmField (w,o,Timeslot,gray_pixmap,Pixmap)

#define	TswHelpCallback(w, o)	        XmField (w,o,Timeslot,help_callback,XtCallbackList)

#define TswSingleCallback(w, o)		XmField (w,o,Timeslot,single_callback,        XtCallbackList)
#define	TswDoubleCallback(w, o)		XmField (w,o,Timeslot,double_callback,    XtCallbackList)
#define TswDragStartCallback(w, o)	XmField (w,o,Timeslot,drag_start_callback,    XtCallbackList)
#define TswDragCallback(w, o)		XmField (w,o,Timeslot,drag_callback,    XtCallbackList)
#define TswDragEndCallback(w, o)	XmField (w,o,Timeslot,drag_end_callback,    XtCallbackList)
#define TswDragCancelCallback(w, o)	XmField (w,o,Timeslot,drag_cancel_callback,   XtCallbackList)
#define TswResizeStartCallback(w, o)	XmField (w,o,Timeslot,resize_start_callback,  XtCallbackList)
#define TswResizeCallback(w, o)		XmField (w,o,Timeslot,resize_callback,    XtCallbackList)
#define TswResizeEndCallback(w, o)	XmField (w,o,Timeslot,resize_end_callback,    XtCallbackList)
#define TswResizeCancelCallback(w, o)	XmField (w,o,Timeslot,resize_cancel_callback, XtCallbackList)
#define TswFocusCallback(w, o)		XmField (w,o,Timeslot,focus_callback,XtCallbackList)
#define TswLosingFocusCallback(w, o)	XmField (w,o,Timeslot,losingfocus_callback,XtCallbackList)
#define TswCloseCallback(w, o)		XmField (w,o,Timeslot,close_callback,XtCallbackList)
#define TswExtendSelectCallback(w, o)	XmField (w,o,Timeslot,extend_select_callback, XtCallbackList)
#define TswIncrementSizeCallback(w, o)	XmField (w,o,Timeslot,incr_size_callback,XtCallbackList)
#define TswIncrementPositionCallback(w, o)	XmField (w,o,Timeslot,incr_posi_callback,XtCallbackList)

#define	TswEditable(w, o)		XmField (w,o,Timeslot,editable,Boolean)
#define	TswDragable(w, o)		XmField (w,o,Timeslot,dragable,Boolean)

#define	TswMbaContext(w, o)		XmField (w,o,Timeslot,mba_context,caddr_t)
#define	TswFontList(w, o)		XmField(w,o,Timeslot,fontlist,XmFontList)

#define	TswTopSash(w, o)	    XmField (w,o,Timeslot,top_sash,Widget)
#define	TswBottomSash(w, o)	    XmField (w,o,Timeslot,bottom_sash,Widget)
#define	TswMoveSash(w, o)	    XmField (w,o,Timeslot,move_sash,Widget)
/*
**
*/
#include "dwc_ui_timeslotwidget.h"
#endif /* _timeslotwidgetp_h_ */

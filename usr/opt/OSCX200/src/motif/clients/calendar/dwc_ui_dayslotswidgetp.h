#ifndef _dayslotswidgetp_h_
#define _dayslotswidgetp_h_
/* $Header$ */
/* #module dwc_ui_dayslotswidgetp.h */
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
**	This include file contains the structures private to the dayslots
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
#ifdef vaxc
#pragma standard
#endif

#include "dwc_ui_timebarwidget.h"	    /* for DwcTbgTimebarGidget */
#include "dwc_compat.h"

typedef struct _DswEntryRecord
{
    Widget		timeslot ;

    Boolean		daynote ;
    
    unsigned short int	start ;
    unsigned short int	duration ;

    Cardinal		sequence ;

    Position		x ;
    Position		y ;
    Dimension		width ;
    Dimension		height ;
    Boolean		geometry_changed ;

    unsigned char	*icons ;
    Cardinal		num_icons ;
    
    caddr_t		tag ;
} DwcDswEntryRecord, *DwcDswEntry;

typedef enum {DswGrabNothing, DswGrabInit, DswGrabStart, DswGrabEnd} DswGrabKind ;

typedef enum {DswASFChangeInverted, DswASFMoveIndex} DswASFKind ;

typedef enum {DswASSNotScrolling, DswASSScrolling, DswASSNeedScroll} DswASSKind ;

typedef enum {DswCMOutlineEvent, DswCMNoExposeEvent, DswCMExposeEvent} DswCMKind ;

#define	No_of_Dayslot_Styles	  5


#define	DayslotsIndex    (XmManagerIndex + 1)

/*
**  Instance Part Record
*/

typedef struct
{
    unsigned short int	preferred_dayslots_size ;
    unsigned short int	preferred_fine_increment ;
    Boolean		on_the_line ;
    Boolean		stack_top_down ;
    int			time_v_margin ;
    int			time_h_margin ;
    Pixel		index_foreground;
    Pixel		index_background_pixel ;
    Pixmap		*pixmaps ;
    Dimension		pixmap_height;
    Dimension		pixmap_width;
    unsigned char	*default_icons ;
    Cardinal		default_num_icons ;
    XmString		*time_texts ;
    XmString		*minute_texts ;
    Boolean		editable ;
    Boolean		is_day_note ;
    Cardinal		work_start ;
    Cardinal		work_finish ;
    Cardinal		lunch_start ;
    Cardinal		lunch_finish ;
    Dimension		timebar_width ;
    Widget		wrkhrs_timebar ;
    XtCallbackList	get_text_callback ;
    XtCallbackList	scroll_callback ;
    XtCallbackList	entry_callback ;
    XtCallbackList	entry_help_callback ;
    XtCallbackList	focus_dispose_callback;

    unsigned short int	actual_dayslots_size ;
    unsigned short int	actual_fine_increment ;
    Position		timebar_x ;
    Dimension		real_timebar_width ;
    Position		dividing_line1_x ;
    Position		dividing_line2_x ;
    Position		time_label_x ;
    Dimension		time_label_width ;
    Dimension		time_label_height ;
    Position		time_slot_x ;
    Dimension		time_slot_width ;
    int			top_margin ;

    Dimension		timeslot_min_height ;

    Dimension		time_max_width ;

    DwcTbgTimebarGidget	timebar_gidget;

    Cardinal		num_slots ;
    XmString		*real_time_texts ;
    Dimension		*time_widths ;
    XmString		*real_minute_texts ;
    Dimension		*minute_widths ;

    DwcDswEntry		selected ;
    DwcDswEntry		open_entry ;
    DwcDswEntry		drag_entry ;
    Boolean		resize_top ;

    Boolean		operation_ok ;
    
    int			open_start ;
    int			open_end ;
    
    int			curr_invert_start_time ;
    int			curr_invert_end_time ;
    int			original_start_invert ;
    int			original_end_invert ;

    Position		cancel_position ;
    
    DswGrabKind		invert_grab ;
    int			start_drag_time ;
    
    Cardinal		num_entries ;
    DwcDswEntry		*entries ;
    
    Cardinal		num_spare_timeslots ;
    Widget		*spare_timeslots ;

    GC			index_foreground_gc ;
    GC			index_background_gc ;
    GC			invert_gc ;
    GC			outline_gc ;
    GC			*slot_line_gcs ;

    XSegment		**slot_line_segments ;
    int			slot_line_num_segments [No_of_Dayslot_Styles] ;
    int			slot_line_index ;

    Region		expose_region ;
    Region		noexpose_region ;

    Boolean		outline ;
    Position		outline_x1 ;
    Position		outline_y1 ;
    Position		outline_x2 ;
    Position		outline_y2 ;

    Boolean		outline_visible ;
    Position		outline_vis_x1 ;
    Position		outline_vis_y1 ;
    Position		outline_vis_x2 ;
    Position		outline_vis_y2 ;

    Boolean		outline_drawable ;
    Cardinal		outline_event_seq ;
    Cardinal		outline_event_last ;

    int			index_time ;
    Boolean		index_resize ;
    int			index_drag_offset ;

    caddr_t		mba_context ;        

    XtIntervalId	auto_scroll_timer ;
    int			auto_scroll_offset ;
    int			auto_scroll_top ;
    int			auto_scroll_bottom ;
    int			auto_scroll_last_y ;
    DswASFKind		auto_scroll_function ;
    DswASSKind		auto_scroll_state ;

    Cardinal		timeslot_sequence ;

    Boolean		ignore_motion ;

    XtIntervalId	wait_2nd_click_timer ;

    XmFontList		fontlist;
    GC			foreground_gc;
    GC			background_gc;
    Pixmap		gray_pixmap;
} DayslotsPart;

/*
**  Instance record
*/
typedef struct _DayslotsRec
{
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    DayslotsPart	dayslots;
} DayslotsWidgetRec, *DayslotsWidget;

/*
**  Class part record
*/

typedef struct _DayslotsClass {
    XmOffsetPtr		dayslotsoffsets ;
    int			reserved ;
} DayslotsClassPart ;

/*
**  Class record
*/

typedef struct _DayslotsClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart	constraint_class;
    XmManagerClassPart	manager_class;
    DayslotsClassPart	dayslots_class;
} DayslotsClassRec, *DayslotsWidgetClass;

#define	DswOffsetPtr(w) \
(((DayslotsWidgetClass) ((DayslotsWidget) w)->core.widget_class)->dayslots_class.dayslotsoffsets)

#define	DswBackgroundPixel(w, o) \
		    XmField (w,o,Core,background_pixel,Pixel)
#define	DswForegroundPixel(w, o) \
		    XmField (w,o,XmManager,foreground,Pixel)
#define	DswDirectionRToL(w, o) \
		    XmField(w,o,XmManager,string_direction,XmStringDirection)
#define	DswHelpCallback(w, o) \
		    XmField(w,o,XmManager,help_callback,XtCallbackList)
#define DswShadowThickness(w,o) \
		    XmField(w,o,XmManager,shadow_thickness,Dimension)
#define DswHighlightThickness(w,o) \
		    XmField(w,o,XmManager,highlight_thickness,Dimension)
#define DswTopShadowGC(w,o) \
		    XmField(w,o,XmManager,top_shadow_GC,GC)
#define DswBottomShadowGC(w,o) \
		    XmField(w,o,XmManager,bottom_shadow_GC,GC)
#define DswHighlightGC(w,o) \
		    XmField(w,o,XmManager,highlight_GC,GC)
#define	DswPreferredDayslotsSize(w, o) \
		    XmField (w,o,Dayslots,preferred_dayslots_size,unsigned short int)
#define	DswPreferredFineIncrement(w, o) \
		    XmField (w,o,Dayslots,preferred_fine_increment,unsigned short int)
#define	DswOnTheLine(w, o) \
		    XmField (w,o,Dayslots,on_the_line,Boolean)
#define	DswStackTopDown(w, o) \
		    XmField (w,o,Dayslots,stack_top_down,Boolean)
#define	DswTimeVMargin(w, o) \
		    XmField (w,o,Dayslots,time_v_margin,int)
#define	DswTimeHMargin(w, o) \
		    XmField (w,o,Dayslots,time_h_margin,int)
#define	DswFontList(w, o) \
		    XmField (w,o,Dayslots,fontlist,XmFontList)
#define	DswEditable(w, o) \
		    XmField (w,o,Dayslots,editable,Boolean)
#define	DswIsDayNote(w, o) \
		    XmField (w,o,Dayslots,is_day_note,Boolean)
#define	DswWorkStart(w, o) \
		    XmField (w,o,Dayslots,work_start,Cardinal)
#define	DswWorkFinish(w, o) \
		    XmField (w,o,Dayslots,work_finish,Cardinal)
#define	DswLunchStart(w, o) \
		    XmField (w,o,Dayslots,lunch_start,Cardinal)
#define	DswLunchFinish(w, o) \
		    XmField (w,o,Dayslots,lunch_finish,Cardinal)

#define	DswTimebarWidth(w, o) \
		    XmField (w,o,Dayslots,timebar_width,Dimension)
#define	DswWrkhrsTimebar(w, o) \
		    XmField (w,o,Dayslots,wrkhrs_timebar,Widget)
#define	DswTimebarGidget(w, o) \
		    XmField (w,o,Dayslots,timebar_gidget,DwcTbgTimebarGidget)

#define	DswIndexForegroundPixel(w, o) \
		    XmField (w,o,Dayslots,index_foreground,Pixel)
#define	DswIndexBackgroundPixel(w, o) \
		    XmField (w,o,Dayslots,index_background_pixel,Pixel)

#define DswBackgroundPixmap(w, o) \
		    XmField(w,o,Core,background_pixmap,Pixmap)
#define	DswPixmaps(w, o) \
		    XmField (w,o,Dayslots,pixmaps,Pixmap *)

#define	DswDefaultIcons(w, o) \
		    XmField (w,o,Dayslots,default_icons,unsigned char *)
#define	DswDefaultNumIcons(w, o) \
		    XmField (w,o,Dayslots,default_num_icons,Cardinal)

#define	DswGetTextCallback(w, o) \
		    XmField (w,o,Dayslots,get_text_callback,XtCallbackList)
#define	DswEntryCallback(w, o) \
		    XmField (w,o,Dayslots,entry_callback,XtCallbackList)
#define	DswEntryHelpCallback(w, o) \
		    XmField (w,o,Dayslots,entry_help_callback,XtCallbackList)
#define	DswScrollCallback(w, o) \
		    XmField (w,o,Dayslots,scroll_callback,XtCallbackList)
#define	DswFocusDisposeCallback(w, o) \
		    XmField(w,o,Dayslots,focus_dispose_callback,XtCallbackList)
#define	DswPixmapWidth(w, o) \
		    XmField (w,o,Dayslots,pixmap_width,Dimension)
#define	DswPixmapHeight(w, o) \
		    XmField (w,o,Dayslots,pixmap_height,Dimension)

#define	DswActualDayslotsSize(w, o) \
		    XmField (w,o,Dayslots,actual_dayslots_size,unsigned short int)
#define	DswActualFineIncrement(w, o) \
		    XmField (w,o,Dayslots,actual_fine_increment,unsigned short int)

#define	DswRealTimebarWidth(w, o) \
		    XmField (w,o,Dayslots,real_timebar_width,Dimension)
#define	DswTimebarX(w, o) \
		    XmField (w,o,Dayslots,timebar_x,Position)
#define	DswDividingLine1X(w, o) \
		    XmField (w,o,Dayslots,dividing_line1_x,Position)
#define	DswDividingLine2X(w, o) \
		    XmField (w,o,Dayslots,dividing_line2_x,Position)
#define	DswTimeLabelX(w, o) \
		    XmField (w,o,Dayslots,time_label_x,Position)
#define	DswTimeLabelWidth(w, o) \
		    XmField (w,o,Dayslots,time_label_width,Dimension)
#define	DswTimeLabelHeight(w, o) \
		    XmField (w,o,Dayslots,time_label_height,Dimension)
#define	DswTimeSlotX(w, o) \
		    XmField (w,o,Dayslots,time_slot_x,Position)
#define	DswTimeSlotWidth(w, o) \
		    XmField (w,o,Dayslots,time_slot_width,Dimension)
#define	DswTopMargin(w, o) \
		    XmField (w,o,Dayslots,top_margin,int)
#define	DswTimeMaxWidth(w, o) \
		    XmField (w,o,Dayslots,time_max_width,Dimension)

#define	DswTimeslotMinHeight(w, o) \
		    XmField (w,o,Dayslots,timeslot_min_height,Dimension)

#define	DswNumSlots(w, o) \
		    XmField (w,o,Dayslots,num_slots,Cardinal)

#define	DswTimeTexts(w, o) \
		    XmField (w,o,Dayslots,time_texts,XmString *)
#define	DswMinuteTexts(w, o) \
		    XmField (w,o,Dayslots,minute_texts,XmString *)

#define	DswRealTimeTexts(w, o) \
		    XmField (w,o,Dayslots,real_time_texts,XmString *)
#define	DswRealMinuteTexts(w, o) \
		    XmField (w,o,Dayslots,real_minute_texts,XmString *)

#define	DswTimeWidths(w, o) \
		    XmField (w,o,Dayslots,time_widths,Dimension *)
#define	DswMinuteWidths(w, o) \
		    XmField (w,o,Dayslots,minute_widths,Dimension *)

#define	DswSelected(w, o) \
		    XmField (w,o,Dayslots,selected,DwcDswEntry)
#define	DswOpenEntry(w, o) \
		    XmField (w,o,Dayslots,open_entry,DwcDswEntry)
#define	DswDragEntry(w, o) \
		    XmField (w,o,Dayslots,drag_entry,DwcDswEntry)
#define	DswResizeTop(w, o) \
		    XmField (w,o,Dayslots,resize_top,Boolean)

#define	DswOperationOk(w, o) \
		    XmField (w,o,Dayslots,operation_ok,Boolean)

#define	DswNumEntries(w, o) \
		    XmField (w,o,Dayslots,num_entries,Cardinal)
#define	DswEntries(w, o) \
		    XmField (w,o,Dayslots,entries,DwcDswEntry *)

#define	DswNumSpareTimeslots(w, o) \
		    XmField (w,o,Dayslots,num_spare_timeslots,Cardinal)
#define	DswSpareTimeslots(w, o) \
		    XmField (w,o,Dayslots,spare_timeslots,Widget *)

#define	DswForegroundGC(w, o) \
		    XmField (w,o,Dayslots,foreground_gc, GC)
#define	DswBackgroundGC(w, o) \
		    XmField (w,o,Dayslots,background_gc, GC)
#define	DswIndexForegroundGC(w, o) \
		    XmField (w,o,Dayslots,index_foreground_gc,GC)
#define	DswIndexBackgroundGC(w, o) \
		    XmField (w,o,Dayslots,index_background_gc,GC)
#define	DswInvertGC(w, o) \
		    XmField (w,o,Dayslots,invert_gc,GC)
#define	DswOutlineGC(w, o) \
		    XmField (w,o,Dayslots,outline_gc,GC)
#define	DswSlotLineGCs(w, o) \
		    XmField (w,o,Dayslots,slot_line_gcs,GC *)

#define DswGrayPixmap(w, o) \
		    XmField(w,o,Dayslots,gray_pixmap,Pixmap)

#define	DswSlotLineSegments(w, o) \
		    XmField (w,o,Dayslots,slot_line_segments,XSegment **)
#define	DswSlotLineNumSegments(w, o) \
		    XmField (w,o,Dayslots,slot_line_num_segments[0],int)
#define	DswSlotLineIndex(w, o) \
		    XmField (w,o,Dayslots,slot_line_index,int)

#define	DswIndexTime(w, o) \
		    XmField (w,o,Dayslots,index_time,int)
#define	DswIndexResize(w, o) \
		    XmField (w,o,Dayslots,index_resize,Boolean)
#define	DswIndexDragOffset(w, o) \
		    XmField (w,o,Dayslots,index_drag_offset,int)

#define	DswExposeRegion(w, o) \
		    XmField (w,o,Dayslots,expose_region,Region)
#define	DswNoexposeRegion(w, o) \
		    XmField (w,o,Dayslots,noexpose_region,Region)

#define	DswOutline(w, o) \
		    XmField (w,o,Dayslots,outline,Boolean)
#define	DswOutlineX1(w, o) \
		    XmField (w,o,Dayslots,outline_x1,Position)
#define	DswOutlineY1(w, o) \
		    XmField (w,o,Dayslots,outline_y1,Position)
#define	DswOutlineX2(w, o) \
		    XmField (w,o,Dayslots,outline_x2,Position)
#define	DswOutlineY2(w, o) \
		    XmField (w,o,Dayslots,outline_y2,Position)

#define	DswOutlineVisible(w, o) \
		    XmField (w,o,Dayslots,outline_visible,Boolean)
#define	DswOutlineVisX1(w, o) \
		    XmField (w,o,Dayslots,outline_vis_x1,Position)
#define	DswOutlineVisY1(w, o) \
		    XmField (w,o,Dayslots,outline_vis_y1,Position)
#define	DswOutlineVisX2(w, o) \
		    XmField (w,o,Dayslots,outline_vis_x2,Position)
#define	DswOutlineVisY2(w, o) \
		    XmField (w,o,Dayslots,outline_vis_y2,Position)

#define	DswOutlineDrawable(w, o) \
		    XmField (w,o,Dayslots,outline_drawable,Boolean)
#define	DswOutlineEventSeq(w, o) \
		    XmField (w,o,Dayslots,outline_event_seq,Cardinal)
#define	DswOutlineEventLast(w, o) \
		    XmField (w,o,Dayslots,outline_event_last,Cardinal)

#define	DswOpenStart(w, o) \
		    XmField (w,o,Dayslots,open_start,int)
#define	DswOpenEnd(w, o) \
		    XmField (w,o,Dayslots,open_end,int)

#define	DswStartDragTime(w, o) \
		    XmField (w,o,Dayslots,start_drag_time,int)
#define	DswStartTimeCurrInvert(w, o) \
		    XmField (w,o,Dayslots,curr_invert_start_time,int)
#define	DswEndTimeCurrInvert(w, o) \
		    XmField (w,o,Dayslots,curr_invert_end_time,int)
#define	DswOriginalStartInvert(w, o) \
		    XmField (w,o,Dayslots,original_start_invert,int)
#define	DswOriginalEndInvert(w, o) \
		    XmField (w,o,Dayslots,original_end_invert,int)
#define	DswInvertGrab(w, o) \
		    XmField (w,o,Dayslots,invert_grab,DswGrabKind)

#define	DswCancelPosition(w, o) \
		    XmField (w,o,Dayslots,cancel_position,Position)

#define	DswMbaContext(w, o) \
		    XmField (w,o,Dayslots,mba_context,caddr_t)

#define	DswAutoScrollTimer(w, o) \
		    XmField (w,o,Dayslots,auto_scroll_timer,XtIntervalId)
#define	DswAutoScrollOffset(w, o) \
		    XmField (w,o,Dayslots,auto_scroll_offset,int)
#define	DswAutoScrollTop(w, o) \
		    XmField (w,o,Dayslots,auto_scroll_top,int)
#define	DswAutoScrollBottom(w, o) \
		    XmField (w,o,Dayslots,auto_scroll_bottom,int)
#define	DswAutoScrollLastY(w, o) \
		    XmField (w,o,Dayslots,auto_scroll_last_y,int)
#define	DswAutoScrollFunction(w, o) \
		    XmField (w,o,Dayslots,auto_scroll_function,DswASFKind)
#define	DswAutoScrollState(w, o) \
		    XmField (w,o,Dayslots,auto_scroll_state,DswASSKind)

#define	DswTimeslotSequence(w, o) \
		    XmField (w,o,Dayslots,timeslot_sequence,Cardinal)

#define	DswIgnoreMotion(w, o) \
		    XmField (w,o,Dayslots,ignore_motion,Boolean)

#define	DswWait2ndClickTimer(w, o) \
		    XmField (w,o,Dayslots,wait_2nd_click_timer,XtIntervalId)


typedef struct _DswTimerBlockRec
{
    Cardinal	    sequence ;
    Time	    time ;
    DayslotsWidget  dsw ;
} DswTimerBlockRec, *DswTimerBlock;

/*
** Now include the public stuff.
*/
#include "dwc_ui_dayslotswidget.h"

#endif /* _dayslotswidgetp_h_ */

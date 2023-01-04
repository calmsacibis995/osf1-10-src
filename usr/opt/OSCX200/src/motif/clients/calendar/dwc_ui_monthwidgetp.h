#if !defined(_dwc_ui_monthwidgetp_h_)
#define _dwc_ui_monthwidgetp_h_ 1
/* $Header$ */
/* #module dwc_ui_monthwidgetp.h */
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
**	This include file contains the structures private to the month widget.
**
**--
*/

#if defined(vaxc) && !defined(__DECC)
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
#include <Xm/PrimitiveP.h>
#endif
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_compat.h"
#include "dwc_ui_monthinfo.h"	    /* for MiMonthInfoList */
#include "dwc_ui_monthwidget.h"

#define	MonthIndex (XmPrimitiveIndex + 1)

/*
**  Instance Part Record
*/

typedef struct
{
    Pixel		foreground;
    Pixel		entry_foreground;
    Pixel		special_foreground;
    XtCallbackList	help_callback; /*  */
    XtCallbackList	dc_callback; /*  */
    XmFontList		font_work_days;
    XmFontList		font_non_work_days;
    XmFontList		font_special_days;
    XmFontList		font_work_days_entry;
    XmFontList		font_non_work_days_entry;
    XmFontList		font_special_days_entry;
    XmFontList		font_day_names;
    XmFontList		font_month_name;
    XmFontList		font_week_numbers;
    Cardinal		day;
    Cardinal		month;
    Cardinal		year;
    CalendarBlock	*cb;
    int			mode;
    unsigned char	work_days;
    unsigned char	style_mask;
    Cardinal		first_day_of_week;
    int			week_depth;		/* Number of weeks to show */
    int			show_week_numbers;	/***** Should be boolean */
    int			week_numbers_starting_day;    
    int			week_numbers_starting_month;    

    GC			gc_normal;
    GC			gc_entry;
    GC			gc_special_days;
    GC			gc_invert;
    GC			gc_invert_entry;
    GC			gc_invert_special;

    unsigned char	first_day;	/* First day of first month to show */
    MiMonthInfoList	*first_month;	/* LL of data describing months */
    MiMonthInfoList	*last_month;	/* LL of data describing months */
    MiMonthInfoList	*named_month;	/* LL of data describing months */
    unsigned char	leading_blanks;
    unsigned char	*week_numbers;
    unsigned short int	*fiscal_years;

    int		day_num_ascent;
    int		month_height;
    int		day_name_width;
    int		day_name_height;

    Boolean	month_before_year;

    XmString	month_name;
    XmString	year_name;
    XmString	sep_string;
    XmString	week_head;
    XmString	ditto;
    XmString	*day_names;

    int		month_name_width;
    int		year_name_width;
    int		sep_width;
    int		month_name_height;
} MonthPart;

/*
**  Instance record
*/

typedef struct _MonthRec
{
    CorePart		core;
    XmPrimitivePart	primitive;
    MonthPart		month;
} MonthWidgetRec, *MonthWidget;

/*
**  Class part record
*/

typedef struct _MonthClass
{
    XmOffsetPtr		monthoffsets;
    int			mumble;
} MonthClassPart;

/*
**  Class record
*/

typedef struct _MonthClassRec
{
    CoreClassPart		core_class;
    XmPrimitiveClassPart	primitive_class;
    MonthClassPart		month_class;
} MonthClassRec, *MonthWidgetClass;

#define	MwOffsetPtr(w) \
(((MonthWidgetClass) ((MonthWidget) w)->core.widget_class)->month_class.monthoffsets)


#define MwForeground(w, o)		XmField(w,o,XmPrimitive,foreground,Pixel)
#define MwShadowThickness(w,o)		XmField(w,o,XmPrimitive,shadow_thickness,Dimension)
#define MwHighlightThickness(w,o)	XmField(w,o,XmPrimitive,highlight_thickness,Dimension)

#define MwTopShadowGC(w,o)		XmField(w,o,XmPrimitive,top_shadow_GC,GC)
#define MwBottomShadowGC(w,o)		XmField(w,o,XmPrimitive,bottom_shadow_GC,GC)
#define MwHighlightGC(w,o)		XmField(w,o,XmPrimitive,highlight_GC,GC)

#define MwEntryForeground(w,o)		XmField(w,o,Month,entry_foreground,Pixel)
#define MwSpecialForeground(w,o)	XmField(w,o,Month,special_foreground,Pixel)
#define MwBackground(w, o)		XmField(w,o,Core,background_pixel,Pixel)
#define MwHelpCallback(w, o)		XmField(w,o,Month,help_callback,XtCallbackList)
#define MwDcCallback(w, o)		XmField(w,o,Month,dc_callback,XtCallbackList)
#define MwFontWorkDays(w, o)		XmField(w,o,Month,font_work_days,XmFontList)
#define MwFontNonWorkDays(w, o)		XmField(w,o,Month,font_non_work_days,XmFontList)
#define MwFontSpecialDays(w, o)		XmField(w,o,Month,font_special_days,XmFontList)
#define MwFontWorkDaysEntry(w, o)	XmField(w,o,Month,font_work_days_entry,XmFontList)
#define MwFontNonWorkDaysEntry(w, o)	XmField(w,o,Month,font_non_work_days_entry,XmFontList)
#define MwFontSpecialDaysEntry(w, o)	XmField(w,o,Month,font_special_days_entry,XmFontList)
#define MwFontDayNames(w, o)		XmField(w,o,Month,font_day_names,XmFontList)
#define MwFontMonthName(w, o)		XmField(w,o,Month,font_month_name,XmFontList)
#define MwFontWeekNumbers(w, o)		XmField(w,o,Month,font_week_numbers,XmFontList)
#define MwDay(w, o)			XmField(w,o,Month,day, Cardinal)
#define MwMonth(w, o)			XmField(w,o,Month,month, Cardinal)
#define MwYear(w, o)			XmField(w,o,Month,year, Cardinal)
#define MwCB(w, o)			XmField(w,o,Month,cb,CalendarBlock *)
#define MwMode(w, o)			XmField(w,o,Month,mode, int)
#define MwWorkDays(w, o)		XmField(w,o,Month,work_days,unsigned char)
#define MwStyleMask(w, o)		XmField(w,o,Month,style_mask,unsigned char)
#define MwFirstDayOfWeek(w, o)		XmField(w,o,Month,first_day_of_week, Cardinal)
#define MwWeekDepth(w, o)		XmField(w,o,Month,week_depth,int)
#define MwShowWeekNumbers(w, o)		XmField(w,o,Month,show_week_numbers,int)
#define MwWeekNumStartDay(w, o)		XmField(w,o,Month,week_numbers_starting_day, int)
#define MwWeekNumStartMonth(w, o)	XmField(w,o,Month,week_numbers_starting_month, int)
#define MwGCNormal(w, o)		XmField(w,o,Month,gc_normal,GC)
#define MwGCEntry(w, o)			XmField(w,o,Month,gc_entry,GC)
#define MwGCSpecialDays(w, o)		XmField(w,o,Month,gc_special_days,GC)
#define MwGCInvert(w, o)		XmField(w,o,Month,gc_invert,GC)
#define MwGCInvertEntry(w, o)		XmField(w,o,Month,gc_invert_entry,GC)
#define MwGCInvertSpecial(w, o)		XmField(w,o,Month,gc_invert_special,GC)
#define MwFirstDay(w, o)		XmField(w,o,Month,first_day,unsigned char)
#define MwFirstMonth(w, o)		XmField(w,o,Month,first_month,MiMonthInfoList *)
#define MwLastMonth(w, o)		XmField(w,o,Month,last_month,MiMonthInfoList *)
#define MwNamedMonth(w, o)		XmField(w,o,Month,named_month,MiMonthInfoList *)
#define MwLeadingBlanks(w, o)		XmField(w,o,Month,leading_blanks,unsigned char)
#define MwWeekNumbers(w, o)		XmField(w,o,Month,week_numbers,unsigned char *)
#define MwFiscalYears(w, o)		XmField(w,o,Month,fiscal_years,unsigned short int *)
#define MwDayNumAscent(w, o)		XmField(w,o,Month,day_num_ascent,int)
#define MwMonthHeight(w, o)		XmField(w,o,Month,month_height,int)
#define MwDayNameWidth(w, o)		XmField(w,o,Month,day_name_width,int)
#define MwDayNameHeight(w, o)		XmField(w,o,Month,day_name_height,int)
#define MwMonthBeforeYear(w, o)		XmField(w,o,Month,month_before_year,Boolean)
#define MwMonthName(w, o)		XmField(w,o,Month,month_name,XmString)
#define MwYearName(w, o)		XmField(w,o,Month,year_name,XmString)
#define MwSepString(w, o)		XmField(w,o,Month,sep_string,XmString)
#define MwWeekHead(w, o)		XmField(w,o,Month,week_head,XmString)
#define MwDitto(w, o)			XmField(w,o,Month,ditto,XmString)
#define MwDayNames(w, o)		XmField(w,o,Month,day_names,XmString *)
#define MwMonthNameWidth(w, o)		XmField(w,o,Month,month_name_width,int)
#define MwYearNameWidth(w, o)		XmField(w,o,Month,year_name_width,int)
#define MwSepWidth(w, o)		XmField(w,o,Month,sep_width,int)
#define MwMonthNameHeight(w, o)		XmField(w,o,Month,month_name_height,int)

#endif /* _dwc_ui_monthwidgetp_h_ */

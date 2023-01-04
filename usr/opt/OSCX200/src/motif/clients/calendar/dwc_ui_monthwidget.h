#if !defined(_dwc_ui_monthwidget_h_)
#define _dwc_ui_monthwidget_h_ 1
/* $Header$ */
/* #module dwc_ui_monthwidget.h */
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
**	Marios Cleovoulou, Novemeber-1987
**
**  ABSTRACT:
**
**	This include file contains the public structures etc, for the month
**	widget. 
**
**--
*/

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include    <X11/Intrinsic.h>		    /* for Cardinal, Widget... */
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include    "dwc_compat.h"
#include    "dwc_ui_monthinfo.h"	    /* for MiAllMonthInfo */

typedef enum {MwNothingSelected, MwDaySelected, MwWeekSelected,
		MwMonthSelected, MwYearSelected} MwTypeSelected ;

typedef struct
{
    MiAllMonthInfo	*allmonths ;
    MwTypeSelected	select_type ;
    unsigned char	select_day ;
    unsigned char 	select_month ;
    unsigned char	select_week ;
    unsigned short int	select_year ;
    Cardinal		monthwidgets_number ;
    Widget		*monthwidgets ;
} CalendarBlock ;


typedef struct
{
    int			reason ;
    MwTypeSelected	select_type ;
    unsigned char	select_day ;
    unsigned char 	select_month ;
    unsigned char	select_week ;
    unsigned short int	select_year ;
} DwcMonthCallbackStruct ;

#if !defined (_dwc_ui_monthwidgetp_h_)
typedef Widget MonthWidget;
typedef WidgetClass MonthWidgetClass;
extern MonthWidgetClass monthWidgetClass;
#endif

#define DwcMwNforeground	"foreground"
#define DwcMwNstyleMask		"styleMask"
#define DwcMwNentryForeground	"entryForeground"
#define DwcMwNspecialForeground	"specialForeground"
#define DwcMwNhelpCallback	"helpCallback"
#define DwcMwNdcCallback	"dcCallback"
#define DwcMwNdayNamesFontList	"dayNamesFontList"
#define DwcMwNweekNumbersFontList	"weekNumbersFontList"
#define DwcMwNmonthNameFontList	"monthNameFontList"
#define DwcMwNworkDaysFontList	"workDaysFontList"
#define DwcMwNnonWorkDaysFontList	"nonWorkDaysFontList"
#define DwcMwNspecialDaysFontList	"specialDaysFontList"
#define DwcMwNworkDaysEntryFontList	"workDaysEntryFontList"
#define DwcMwNnonWorkDaysEntryFontList "nonWorkDaysEntryFontList"
#define DwcMwNspecialDaysEntryFontList "specialDaysEntryFontList"
#define DwcMwNday		"day"
#define DwcMwNmonth		"month"
#define DwcMwNyear		"year"
#define DwcMwNcalendarBlock	"calendarBlock"
#define DwcMwNmode		"mode"
#define DwcMwNworkDays		"workDays"
#define DwcMwNweekStart		"weekStart"
#define DwcMwNweekDepth		"weekDepth"
#define DwcMwNshowWeekNumbers	"showWeekNumbers"
#define DwcMwNweekNumbersStartingDay   "weekNumbersStartingDay"
#define DwcMwNweekNumbersStartingMonth "weekNumbersStartingMonth"
#define DwcMwNmonthBeforeYear "monthBeforeYear"

#define DwcMwCStyleMask	    "StyleMask"
#define DwcMwCDay	    "Day"
#define DwcMwCMonth	    "Month"
#define DwcMwCYear	    "Year"
#define DwcMwCCalendarBlock   "CalendarBlock"
#define DwcMwCMode	    "Mode"
#define DwcMwCWorkDays	    "WorkDays"
#define DwcMwCWeekStart	    "WeekStart"
#define DwcMwCWeekDepth	    "WeekDepth"
#define DwcMwCShowWeekNumbers "ShowWeekNumbers"
#define DwcMwCWeekNumbersStartingDay   "WeekNumbersStartingDay"
#define DwcMwCWeekNumbersStartingMonth "WeekNumbersStartingMonth"
#define DwcMwCMonthBeforeYear "MonthBeforeYear"

#define DwcDaySunday	    1
#define DwcDayMonday	    2
#define DwcDayTuesday	    3
#define DwcDayWednesday	    4
#define DwcDayThursday	    5
#define DwcDayFriday	    6
#define DwcDaySaturday	    7

#define	DwcMwModeDefault	0
#define	DwcMwModeWholeMonth	1
#define	DwcMwModeFillBlanks	2

#define	MWSTYLE_CROSS_THRU	(1L<<0)
#define MWSTYLE_CIRCLE		(1L<<1)
#define MWSTYLE_SQUARE		(1L<<2)
#define	MWSTYLE_SLASH_THRU	(1L<<3)

#define MWMARKUP_DEFAULT	0
#define MWMARKUP_WORK_DAY	1
#define MWMARKUP_NON_WORK_DAY	2
#define MWMARKUP_SPECIAL	3
#define MWMARKUP_ENTRY		(1L<<7)


Widget MWMonthCreate PROTOTYPE ((
	Widget	parent,
	char	*name,
	Arg	*arglist,
	int	argcount));

Widget Month PROTOTYPE ((
	Widget		parent,
	char		*name,
	Position	x,
	Position	y,
	int		day,
	int		month,
	int		year,
	CalendarBlock	*cb,
	XtCallbackList	help_callback));

CalendarBlock *MWCreateCalendarBlock PROTOTYPE ((
	Cardinal	    cache_size,
	XtCallbackProc	    callback,
	dwcaddr_t	    tag));

void MWGetSelection PROTOTYPE ((
	Widget		w,
	MwTypeSelected	*s_type,
	Cardinal	*day,
	Cardinal	*week,
	Cardinal	*month,
	Cardinal	*year));

void MWSetSelection PROTOTYPE ((
	Widget		w,
	MwTypeSelected	s_type,
	Cardinal	day,
	Cardinal	week,
	Cardinal	month,
	Cardinal	year));

void MWSetDate PROTOTYPE ((
	Widget		w,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year));

void MWGetDayStyleMarkup PROTOTYPE ((
	Widget		w,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year,
	unsigned char	*markup,
	unsigned char	*style));

void MWSetDayStyleMarkup PROTOTYPE ((
	Widget		w,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year,
	unsigned char	markup,
	unsigned char	style));

void MWRegetAllDayMarkup PROTOTYPE ((Widget w));

#endif /* _dwc_ui_monthwidget_h_ */

/* dwc_ui_view.c */
#ifndef lint
static char rcsid[] = "$Header$";
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
**	Marios Cleovoulou, March-1988 (dwc_ui_calendar.c)
**
**  ABSTRACT:
**
**	This is the module deals with view operations
**
**--
*/

#include "dwc_compat.h"

#include <time.h>

#if defined(vaxc)
#pragma nostandard
#endif
#include <Xm/XmP.h>	/* Widget, XmCR_MAP,etc... */
#if defined(vaxc)
#pragma standard
#endif

#include "dwc_ui_monthwidget.h"		/* MWGetSelection */
#include "dwc_ui_calendar.h"		/* CalendarDisplay */
#include "dwc_ui_misc.h"		/* MISCFindCalendarDisplay */

void VIEWMONTH_DC
#if defined(_DWC_PROTO_)
	(
	Widget			mw,
	caddr_t			tag,
	DwcMonthCallbackStruct	*cbs)
#else	/* no prototypes */
	(mw, tag, cbs)
	Widget			mw;
	caddr_t			tag;
	DwcMonthCallbackStruct	*cbs;
#endif	/* prototype */
{
    Cardinal		    day, month, year ;
    Boolean		    dont_care ;
    CalendarDisplay	    cd;
    int			    status;

    status = MISCFindCalendarDisplay( &cd, mw );
    
    switch (cbs->select_type)
    {
    case MwNothingSelected:
        break;

    case MwDaySelected:
	MISCChangeView
	(
	    cd,
	    show_day,
	    cbs->select_day,
	    cbs->select_month,
	    cbs->select_year
	);
        break;

    case MwWeekSelected:
	dont_care = MISCFirstDOWWithCondition
	(
	    cd,
	    cbs->select_week,
	    cbs->select_year,
	    TRUE,
	    FALSE,
	    &day,
	    &month,
	    &year
	);
	MISCChangeView (cd, show_day, day, month, year);
        break;

    case MwMonthSelected:
	MISCChangeView
	    (cd, show_month, 1, cbs->select_month, cbs->select_year);
        break;

    case MwYearSelected:
	MISCChangeView
	    (cd, show_year,  1, cbs->select_month, cbs->select_year);
        break;

    }

}

void VIEW_SELECTED
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    MwTypeSelected	s_type ;
    Cardinal		day ;
    Cardinal		week ;
    Cardinal		month ;
    Cardinal		year ;
    Arg			arglist [10] ;
    CalendarDisplay	    cd;
    int			    status;

    status = MISCFindCalendarDisplay (&cd, w);
    
    MWGetSelection (cd->month_display, &s_type, &day, &week, &month, &year);
    
    switch (s_type)
    {
    case MwNothingSelected:
        break;

    case MwDaySelected:
	MISCChangeView (cd, show_day, day, month, year);
        break ;

    case MwWeekSelected:
        break ;

    case MwMonthSelected:
	MISCChangeView (cd, show_month, 1, month, year);
        break;

    case MwYearSelected:
	MISCChangeView (cd, show_year,  1, month, year);
        break;

    }
    	    
}

void VIEW_TODAY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    CalendarDisplay	    cd;
    int			    status;
    struct tm		    *local_time;

    status = MISCFindCalendarDisplay (&cd, w);
    

    /*	  
    **  Get the local internal time
    */	  
    MISCGetTime(&local_time);

    MISCChangeView (cd, show_day, local_time->tm_mday,
			       local_time->tm_mon,
			       local_time->tm_year) ;

}

void VIEW_DAY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    MwTypeSelected	s_type ;
    Cardinal		day ;
    Cardinal		week ;
    Cardinal		month ;
    Cardinal		year ;
    CalendarDisplay	    cd;
    int			    status;

    status = MISCFindCalendarDisplay (&cd, w);
    
    MWGetSelection (cd->month_display, &s_type, &day, &week, &month, &year);
    if (s_type != MwDaySelected)
    {
	return;
    }

    MISCChangeView (cd, show_day, day, month, year);
	    
}

void VIEW_MONTH
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    MwTypeSelected	s_type;
    Cardinal		day;
    Cardinal		week;
    Cardinal		month;
    Cardinal		year;
    CalendarDisplay	cd;
    int			status;

    status = MISCFindCalendarDisplay (&cd, w);
    
    MWGetSelection (cd->month_display, &s_type, &day, &week, &month, &year);
    if ((s_type != MwDaySelected) && (s_type != MwMonthSelected))
    {
	month = cd->month;
	year  = cd->year;
    }

    MISCChangeView (cd, show_month, 1, month, year);
	    
}

void VIEW_YEAR
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    MwTypeSelected	s_type ;
    Cardinal		day ;
    Cardinal		week ;
    Cardinal		month ;
    Cardinal		year ;
    CalendarDisplay	    cd;
    int			    status;

    status = MISCFindCalendarDisplay (&cd, w);
    
    MWGetSelection (cd->month_display, &s_type, &day, &week, &month, &year);
    if ((s_type != MwDaySelected) &&
	(s_type != MwMonthSelected) &&
	(s_type != MwYearSelected))
    {
	month = cd->month;
	year  = cd->year;
    }

    MISCChangeView (cd, show_year, 1, month, year);
	    
}

void VIEWDO_VIEW_MENU
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    MwTypeSelected	s_type ;
    Cardinal		day ;
    Cardinal		week ;
    Cardinal		month ;
    Cardinal		year ;
    CalendarDisplay	    cd;
    int			    status;

    status = MISCFindCalendarDisplay (&cd, w);
    
    if (*reason != (int)XmCR_MAP)
    {
	return ;
    }
    
    if (MISCTestRepeatOpen (cd))
    {
	XtSetSensitive (w, FALSE);
	return;
    }

    XtSetSensitive (w, TRUE);

    MWGetSelection(cd->month_display, &s_type, &day, &week, &month, &year);

    XtSetSensitive (cd->pb_year, TRUE);

    if (s_type == MwNothingSelected)
    {
	XtSetSensitive (cd->pb_selected, FALSE);

	switch (cd->showing)
	{
	case show_day :
	    XtSetSensitive (cd->pb_today,    TRUE) ; /* ?????? Unless day is today */
	    XtSetSensitive (cd->pb_day,      FALSE) ;
/*
	    XtSetSensitive (cd->pb_week,     TRUE) ;
*/
	    XtSetSensitive (cd->pb_month,    TRUE) ;
	    break ;

	case show_month :
	    XtSetSensitive (cd->pb_today,    TRUE) ;
	    XtSetSensitive (cd->pb_day,      FALSE) ;
/*
	    XtSetSensitive (cd->pb_week,     FALSE) ;
*/
	    XtSetSensitive (cd->pb_month,    TRUE) ; /* Unless already	    */
						     /*	there ????????	    */
	break ;
	}

    }
    else
    {
	XtSetSensitive (cd->pb_selected, TRUE) ;

	switch (s_type)
	{
	case MwDaySelected :
	    XtSetSensitive (cd->pb_day,      TRUE) ;
/*
	    XtSetSensitive (cd->pb_week,     TRUE) ;
*/
	    XtSetSensitive (cd->pb_month,    TRUE) ; /* ?????? Unless it's	    */
						 /* current month ?	    */
	    break ;

	case MwWeekSelected :
	    XtSetSensitive (cd->pb_day,      FALSE) ;
/*
	    XtSetSensitive (cd->pb_week,     TRUE) ;
*/
	    XtSetSensitive (cd->pb_month,    TRUE) ; /* ?????? Unless it's	    */
						 /* current month ?	    */
	    break ;

	case MwMonthSelected :
	    XtSetSensitive (cd->pb_day,      FALSE) ;
/*
	    XtSetSensitive (cd->pb_week,     FALSE) ;
*/
	    XtSetSensitive (cd->pb_month,    TRUE) ; /* ?????? Unless it's	    */
						 /* current month ?	    */
	    break ;

	}
    }

}

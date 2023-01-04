/* dwc_ui_month.c */
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
**	Marios Cleovoulou, March-1988 (DWC_UI_CALENDAR.C)
**
**  ABSTRACT:
**
**	This is the module that deals with month displays
**
**--
*/

#include "dwc_compat.h"

#include <time.h>
#include <stdio.h>
#include <assert.h>

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/ScrollBarP.h>		/* XmScrollBarCallbackStruct */
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* for DWC$k_db_normal */

#include "dwc_ui_calendar.h"		/* for CalendarDisplay */
#include "dwc_ui_datestructures.h"
#include "dwc_ui_monthwidget.h"		/* for DwcMonthCallbackStruct */
#include "dwc_ui_view.h"		/* for VIEWMonthDC */
#include "dwc_ui_uil_values_const.h"	/* DWC$k_help... */
#include "dwc_ui_misc.h"		/* MISCGetTime */
#include "dwc_ui_layout.h"		/* for LwLayout */
#include "dwc_ui_help.h"
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_errorboxes.h"

static void
MONTH_HELP PROTOTYPE ((
	Widget			mw,
	caddr_t			closure,
	DwcMonthCallbackStruct	*cbs));

static void
MOVE_SCROLLBAR PROTOTYPE ((
	Widget				w,
	caddr_t				closure,
	XmScrollBarCallbackStruct	*scroll));

static void
RESIZE_MONTH_CALLBACK PROTOTYPE ((
	Widget	lw,
	caddr_t	closure,
	XmAnyCallbackStruct *cbs));

XtCallbackRec MONTHmonth_dc_cb [2] =
{
    {(XtCallbackProc) VIEWMONTH_DC,	NULL},
    {NULL,			NULL}
};

XtCallbackRec MONTHmonth_help_cb [2] =
{
    {(XtCallbackProc) MONTH_HELP,	NULL},
    {NULL,				NULL}
};

static XtCallbackRec resize_month_cb [2] =
{
    {(XtCallbackProc) RESIZE_MONTH_CALLBACK,	NULL},
    {NULL,					NULL}
};

static XtCallbackRec scroll_cb [2] =
{
    {(XtCallbackProc)MOVE_SCROLLBAR,	NULL},
    {NULL,				NULL}
};

static void MONTH_HELP
#if defined(_DWC_PROTO_)
	(
	Widget			mw,
	caddr_t			closure,
	DwcMonthCallbackStruct	*cbs)
#else	/* no prototypes */
	(mw, closure, cbs)
	Widget			mw;
	caddr_t			closure;
	DwcMonthCallbackStruct	*cbs;
#endif	/* prototype */
{
    int			    helpframe;
    char		    *helptopic;
    CalendarDisplay	    cd = (CalendarDisplay)closure;
    MwTypeSelected	    s_type;
    Cardinal		    s_day;
    Cardinal		    s_week;
    Cardinal		    s_month;
    Cardinal		    s_year;
    
    if (cbs == (DwcMonthCallbackStruct *)NULL)
    {
        /*	  
	**  We arrived in in such a way that we don't have info in the cbs, so
	**  we'll have to get the info some other way. Now let me see, hm...
	*/	  
	if (cd == (CalendarDisplay)NULL)
	    (void)MISCFindCalendarDisplay(&cd, mw);
	
	/*	  
	**  What does the monthwidget think is selected?
	*/	  
	MWGetSelection(cd->month_display, &s_type, &s_day, &s_week,
			 &s_month, &s_year);
    }
    else
    {
        /*	  
	**  We've got the info in the cbs so use that
	*/	  
        s_type = cbs->select_type;
    }

    /*	  
    **  Figure out what we think is selected and give the right help
    */	  
    switch (s_type)
    {
    case MwNothingSelected:
	helpframe = dwc_k_help_month_none;
	break;
    case MwDaySelected:
	helpframe = dwc_k_help_month_day;
	break;
    case MwWeekSelected:
	helpframe = dwc_k_help_month_week;
	break;
    case MwMonthSelected:
	helpframe = dwc_k_help_month_month;
	break;
    case MwYearSelected:
	helpframe = dwc_k_help_month_year;
	break;
    default:
	helpframe = dwc_k_help_month_none;
	break;
    }

    helptopic = (char *)MISCFetchDRMValue (helpframe);
#if defined(NEW_HYPERHELP) || defined(OLD_HYPERHELP)
    HELPDisplay ((CalendarDisplay) cd, cd->toplevel, helptopic);
#else
    HELPDisplay
	(cd->toplevel, cd->toplevel, &(cd->main_help_widget), helptopic, TRUE);
#endif

}

void MONTHConfigMonthDisplay
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    int		width, height;
    Widget	lw = (Widget) cd->lw_month_display;
    Dimension	border_width;
    Dimension	lw_thickness, lw_width, lw_height, lw_m_width, lw_m_height;
    Dimension	sb_width;

    XtVaGetValues
    (
	lw,
	XmNwidth, &lw_width,
	XmNheight, &lw_height,
	XmNshadowThickness, &lw_thickness,
	XmNmarginWidth, &lw_m_width,
	XmNmarginHeight, &lw_m_height,
	NULL
    );

    XtVaGetValues
    (
	cd->month_scrollbar,
	XmNwidth, &sb_width,
	NULL
    );

    width = MAX(
	1,
	(
	 (int)lw_width -
	 ((int)lw_thickness * 2) -
	 ((int)lw_m_width * 3) -
	 (int)sb_width
	)
    );

    height = MAX(
	1,
	((int)lw_height - ((int)lw_thickness * 2) - ((int)lw_m_height * 2))
    );

    if (cd->profile.directionRtoL)
    {
	XtVaSetValues
	(
	    cd->month_display,
	    XmNwidth, width,
	    XmNheight, height,
	    XmNx, sb_width + lw_thickness + (lw_m_width * 2),
	    XmNy, lw_thickness + lw_m_height,
	    NULL
	);

    }
    else
    {
	XtVaSetValues
	(
	    cd->month_display,
	    XmNwidth, width,
	    XmNheight, height,
	    XmNx, lw_thickness + lw_m_width,
	    XmNy, lw_thickness + lw_m_height,
	    NULL
	);
    }
    
    if (cd->profile.directionRtoL)
    {
	XtVaSetValues
	(
	    cd->month_scrollbar,
	    XmNheight, height,
	    XmNx, lw_thickness + lw_m_width,
	    XmNy, lw_thickness + lw_m_height,
	    NULL
	);
    }
    else
    {
	XtVaSetValues
	(
	    cd->month_scrollbar,
	    XmNheight, height,
	    XmNx, width + lw_thickness + (lw_m_width * 2),
	    XmNy, lw_thickness + lw_m_height,
	    NULL
	);
    }

}

static void RESIZE_MONTH_CALLBACK
#if defined(_DWC_PROTO_)
	(
	Widget	lw,
	caddr_t	closure,
	XmAnyCallbackStruct *cbs)
#else	/* no prototypes */
	(lw, closure, cbs)
	Widget	lw;
	caddr_t	closure;
	XmAnyCallbackStruct *cbs;
#endif	/* prototype */
{
    Dimension		height,width;

    CalendarDisplay	cd = (CalendarDisplay) closure;
    
    MONTHConfigMonthDisplay (cd);

    XtVaGetValues
    (
	cd->mainwid,
	XmNwidth, &width,
	XmNheight, &height,
	NULL
    );
    cd->profile.month_width = ((Cardinal)width * 100) / cd->screen_font_size;
    cd->profile.month_height = ((Cardinal)height * 100) / cd->screen_font_size;

}

void MONTHGetMonthMarkup
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Cardinal	month,
	Cardinal	year,
	Cardinal	days,
	unsigned char	markup[],
	unsigned char	style[])
#else	/* no prototypes */
	(cd, month, year, days, markup, style)
	CalendarDisplay	cd;
	Cardinal	month;
	Cardinal	year;
	Cardinal	days;
	unsigned char	markup[];
	unsigned char	style[];
#endif	/* prototype */
{
    int			dsbot;
    int			now;
    unsigned char       got_entry[31];
    unsigned char       marks[31];
    int			i;
    int			past;
    struct tm		*local_time;
    int			status;

    /*	  
    **  Get the internal local time
    */	  
    MISCGetTime(&local_time);

    /*	  
    **  Get the dsbot for the first day of the month.
    */	  
    dsbot = DATEFUNCDaysSinceBeginOfTime (1, month, year);

    /*	  
    **  Get the attributes for a range of days.
    */
    status = DWC$DB_Get_day_attr_rng
	(cd->cab, dsbot, dsbot + days - 1, marks, style, got_entry);  
    if (status != DWC$k_db_normal)
    {
        ERRORDisplayError (cd->mainwid, "ErrorDayAttr"); 
    }

    for (i = 0;  i < days;  i++)
    {
	if ((marks [i] & DWC$m_day_special) != 0)
	{
	    markup [i] = MWMARKUP_SPECIAL;
	}
	else
	{
	    marks [i] = marks [i] & ~DWC$m_day_defaulted;
	    if (marks [i] == DWC$k_day_workday)
	    {
		markup [i] = MWMARKUP_WORK_DAY;
	    }
	    else
	    {
		markup [i] = MWMARKUP_NON_WORK_DAY;
	    }
	}
	
	if (got_entry [i] == DWC$k_use_significant)
	{
	    markup [i] = markup [i] | MWMARKUP_ENTRY;
	}
    }
    
    now = DATEFUNCDaysSinceBeginOfTime
	(local_time->tm_mday, local_time->tm_mon, local_time->tm_year);

    past = MIN ((int) days, now - dsbot);
    if (past < 0)
    {
	return;
    }

    for (i = 0;  i < past;  i++)
    {
	style [i] = MWSTYLE_CROSS_THRU | MWSTYLE_SLASH_THRU;
    }

    if (past != days)
    {
	style [past] = MWSTYLE_SQUARE;
    }

}

void MONTHCreateMonthDisplay
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {
    Arg			arglist[25];
    Cardinal		ac;
    Cardinal		month;
    Cardinal		day;
    int			bit;
    struct tm		*local_time;

#define SLIDER_SIZE	6

    resize_month_cb [0].closure = (caddr_t) cd;
    cd->lw_month_display = LwLayout
    (
	cd->mainwid,
	"MonthDisplay",
	FALSE,
	(Position)0, (Position)0,
	(Dimension)0, (Dimension)0,
	(XtCallbackList)resize_month_cb,
	(XtCallbackList)NULL,
	(XtCallbackList)NULL,
	(XtCallbackList)MONTHmonth_help_cb,
	(caddr_t)cd
    );
    XtVaSetValues
    (
	(Widget)cd->lw_month_display,
	XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
	XmNtraversalOn, True,
	XmNshadowThickness, 2,
	XmNshadowType, XmSHADOW_OUT,
	XmNmarginWidth, 3,
	XmNmarginHeight, 3,
	NULL
    );

    /*
    ** Create the month widget.
    */
    MONTHmonth_dc_cb   [0].closure = (caddr_t) cd;
    MONTHmonth_help_cb [0].closure = (caddr_t) cd;
    
    /*	  
    **  Get the internal local time
    */	  
    MISCGetTime(&local_time);

    ac = 0;
    XtSetArg(arglist[ac], XmNtraversalOn, TRUE); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;
    XtSetArg(arglist[ac], DwcMwNday, local_time->tm_mday); ac++;
    XtSetArg(arglist[ac], DwcMwNmonth, local_time->tm_mon); ac++;
    XtSetArg(arglist[ac], DwcMwNyear, local_time->tm_year); ac++;
    XtSetArg(arglist[ac], DwcMwNhelpCallback, MONTHmonth_help_cb); ac++;
    XtSetArg(arglist[ac], DwcMwNdcCallback, MONTHmonth_dc_cb); ac++;
    XtSetArg(arglist[ac], DwcMwNcalendarBlock, cd->cb); ac++;
    XtSetArg(arglist[ac], DwcMwNworkDays, cd->profile.work_days); ac++;
    XtSetArg(arglist[ac], DwcMwNstyleMask, cd->profile.month_style_mask); ac++;
    XtSetArg(arglist[ac], DwcMwNshowWeekNumbers, cd->profile.show_week_numbers);
	ac++;

    XtSetArg(
	arglist[ac],
	DwcMwNweekNumbersStartingDay,
	cd->profile.week_numbers_start_day); ac++;

    XtSetArg(
	arglist[ac],
	DwcMwNweekNumbersStartingMonth,
	cd->profile.week_numbers_start_month); ac++;

    if (cd->profile.fill_blank_days)
    {
	XtSetArg(arglist[ac], DwcMwNmode, DwcMwModeFillBlanks); ac++;
    }
    else
    {
	XtSetArg(arglist[ac], DwcMwNmode, DwcMwModeDefault); ac++;
    }

    XtSetArg(arglist[ac], DwcMwNweekStart, cd->profile.first_day_of_week); ac++;
    XtSetArg(arglist[ac], XmNshadowThickness, 2); ac++;
    XtSetArg(arglist[ac], XmNtraversalOn, TRUE); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmTAB_GROUP); ac++;
    assert (ac <= XtNumber(arglist));

    cd->month_display = MWMonthCreate
	((Widget) cd->lw_month_display, "Month", arglist, ac);

    /*
    ** Create the scroll bar.
    */
    scroll_cb [0].closure = (caddr_t) cd;

    ac = 0;
    XtSetArg(arglist[ac], XmNx, 0); ac++;
    XtSetArg(arglist[ac], XmNy, 0); ac++;
    XtSetArg(arglist[ac], XmNwidth, 0); ac++;
    XtSetArg(arglist[ac], XmNheight, 50); ac++;
    XtSetArg(arglist[ac], XmNincrement, 1); ac++;
    XtSetArg(arglist[ac], XmNpageIncrement, 1); ac++;
    XtSetArg(arglist[ac], XmNsliderSize, SLIDER_SIZE); ac++;
    XtSetArg(arglist[ac], XmNvalue, 0); ac++;
    XtSetArg(arglist[ac], XmNminimum, 0); ac++;
    XtSetArg(arglist[ac], XmNtraversalOn, TRUE); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;
    /*	  
    **  XUI gave a callback when at the end of the scrollbar. Motif doesn't so
    **	leave a space at each end. 52 weeks plus the size of the slider plus a
    **	space at each end.
    */	  
    XtSetArg(arglist[ac], XmNmaximum, 52 + SLIDER_SIZE + 2); ac++;
    XtSetArg(arglist[ac], XmNorientation, XmVERTICAL); ac++;
    XtSetArg(arglist[ac], XmNincrementCallback, scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNdecrementCallback, scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNpageIncrementCallback, scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNpageDecrementCallback, scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNtoTopCallback, scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNtoBottomCallback, scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNdragCallback, scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNvalueChangedCallback, scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    XtSetArg(arglist[ac], XmNhighlightThickness, 2); ac++;
    XtSetArg(arglist[ac], XmNtraversalOn, TRUE); ac++;
    assert (ac <= XtNumber(arglist));
    cd->month_scrollbar = XmCreateScrollBar
	((Widget)cd->lw_month_display, "VScrollbar", arglist, ac);

    XtManageChildren
    (
	DWC_CHILDREN(cd->lw_month_display),
	DWC_NUM_CHILDREN(cd->lw_month_display)
    );

}

static void MOVE_SCROLLBAR
#if defined(_DWC_PROTO_)
	(
	Widget				w,
	caddr_t				closure,
	XmScrollBarCallbackStruct	*scroll)
#else	/* no prototypes */
	(w, closure, scroll)
	Widget				w;
	caddr_t				closure;
	XmScrollBarCallbackStruct	*scroll;
#endif	/* prototype */
    {
    int				dsbot, value;
    int				day, month, year;
    CalendarDisplay		cd = (CalendarDisplay) closure;
    caddr_t			tag;

    /*	  
    **  Wait for the value changed callback if we're not direct scrolling.
    */	  
    if ((scroll->reason == (int)XmCR_DRAG) &&
        (! cd->profile.direct_scroll_coupling))
    {
	return;
    }

    /*	  
    **  If we're direct scrolling then ignore the value changed callback and
    **	depend on the XmCR_DRAG callbacks.
    */	  
    if ((scroll->reason == (int)XmCR_VALUE_CHANGED) &&
	(cd->profile.direct_scroll_coupling))
    {
	return;
    }

    if (scroll->reason == (int)XmCR_HELP)
    {
	tag = (caddr_t)MISCFetchDRMValue (dwc_k_help_month_scroll);
	HELPForWidget (w, tag, NULL);
	return;
    }

    /*	  
    **  Figure out what we're currently looking at.
    */	  
    day   = cd->day;
    month = cd->month;
    year  = cd->year;


    /*	  
    **  The scrollbar for the MonthDisplay is initially set up to increment by
    **	weeks. This routine translates the weeks to a day in history and then
    **	calls MISCChangeView to change the MonthDisplay to that particular day.
    */	  
    switch (scroll->reason)
    {
    /*	  
    **  Come up with the offset in days from the beginning of the year to
    **  where we want to display, ie. if we want to display the first week of
    **  the year, value would be 7, 2nd week would be 14 etc.
    */	  
    case XmCR_INCREMENT:
        /*	  
	**  Increase by a week
	*/	  
        value = ((scroll->value - 1) * 7) + 7;
	break;
	
    case XmCR_DECREMENT:
        /*	  
	**  Decrease by a week.
	*/	  
        value = ((scroll->value - 1) * 7) - 7;
	break;
	
    case XmCR_PAGE_INCREMENT:
        /*	  
	**  Increase to beginning of next month.
	*/	  
        if (month != 12)
	{
	    month = month + 1;
	}
	else
	{
	    month = 1;
	    year  = year + 1;
	}
	value = DATEFUNCDaysSinceBeginOfYear (1, month, year);
	break;
	
    case XmCR_PAGE_DECREMENT:
        /*	  
	**  Go back to beginning of month.
	*/	  
        if (day <= 7)
	{
	    if (month != 1)
	    {
		month = month - 1;
	    }
	    else
	    {
		month = 12;
		year  = year - 1;
	    }
	}
	value = DATEFUNCDaysSinceBeginOfYear (1, month, year);
	break;
	
    case XmCR_TO_TOP:
        /*	  
	**  Go to the beginning of the year.
	*/	  
        value = 0;
	break;
	
    case XmCR_TO_BOTTOM:
        /*	  
	**  Go the end of the year (we back up one from the end)
	*/	  
        value = (scroll->value - 1) * 7;
	break;

    default:
	value = (scroll->value - 1) * 7;
	break;

    }

    dsbot = DATEFUNCDaysSinceBeginOfTime (1, 1, year);
    DATEFUNCDateForDayNumber(dsbot + value, &day, &month, &year);

    MISCChangeView(cd, show_month, day, month, year);
    XSync (XtDisplay (w), FALSE);

}

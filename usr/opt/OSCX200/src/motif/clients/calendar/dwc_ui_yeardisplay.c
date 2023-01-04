/* dwc_ui_yeardisplay.c */
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
**	Marios Cleovoulou, March-1988
**
**  ABSTRACT:
**
**	This is the module that deals with year displays.
**
**--
*/

#include "dwc_compat.h"

#include <stdio.h>
#include <assert.h>

#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/XmP.h>			/* for XmVoidProc */
#include <Xm/ScrollBarP.h>		/* for XmScrollBarCallbackStruct */
#ifdef vaxc
#pragma standard
#endif

#include "dwc_ui_calendar.h"		/* for CalendarDisplay */
#include "dwc_ui_yeardisplay.h"		/* for forward declarations */
#include "dwc_ui_misc.h"		/* for MISCFindCalendarDisplay */
#include "dwc_ui_layout.h"		/* declaration for LwLayout */
#include "dwc_ui_errorboxes.h"

static void YEAR_LAYOUT_RESIZE PROTOTYPE ((
	Widget	lw,
	dwcaddr_t	tag,
	int	*reason));

static void YEAR_DISPLAY_SCROLL PROTOTYPE ((
	Widget				w,
	dwcaddr_t				tag,
	XmScrollBarCallbackStruct	*scroll));

static XtCallbackRec year_layout_resizecb [2] =
{
    {(XtCallbackProc) YEAR_LAYOUT_RESIZE,	NULL},
    {NULL,					NULL}
};

static XtCallbackRec year_display_scroll_cb [2] =
{
    {(XtCallbackProc) YEAR_DISPLAY_SCROLL,	NULL},
    {NULL,					NULL}
};

void YEARConfigureYearDisplay
#ifdef _DWC_PROTO_
	(
	YearDisplay	*yd,
	Boolean		backwards,
	Boolean		resize_actions)
#else	/* no prototypes */
	(yd, backwards, resize_actions)
	YearDisplay	*yd;
	Boolean		backwards;
	Boolean		resize_actions;
#endif	/* prototype */
{
    Dimension		mw_height;
    Dimension		mw_width;
    Position		yd_offset;
    Dimension		yd_height;
    Dimension		yd_width;
    int			rows;
    int			columns;
    int			row;
    int			column;
    Cardinal		month;
    Position		margin_x;
    Position		margin_y;
    int			scroll_value, slider_size, inc, pageinc;
    int			computed_value, computed_slider_size,
			    computed_inc, computed_pageinc;
    int			status;
    CalendarDisplay	cd;
    int			max;
    Dimension		margin;
    Dimension		shadow_thickness;
    Boolean		rtol;
        
    status = MISCFindCalendarDisplay (&cd, (Widget)yd->layout);
#ifdef DEC_MOTIF_EXTENSION
    rtol = (yd->layout->manager.dxm_layout_direction ==
	(int)DXmLAYOUT_LEFT_DOWN);
#else
    rtol = (yd->layout->manager.string_direction ==
	(int)XmSTRING_DIRECTION_R_TO_L);
#endif

    mw_height = XtHeight(yd->months [0]);
    mw_width  = XtWidth(yd->months [0]);

    XtVaGetValues
    (
	(Widget)yd->layout,
	XmNmarginHeight, &margin,
	XmNshadowThickness, &shadow_thickness,
	NULL
    );

    yd_height = XtHeight(yd->layout) - XtHeight(yd->horiz_scrollbar);
    if (yd->vert_scrollbar == NULL)
    {
        /*	  
        **  This means we're in the dayview or month view
        */	  
        yd_width = XtWidth(yd->layout);
	rows     = 1;
    }
    else
    {
	yd_width = XtWidth(yd->layout) - XtWidth(yd->vert_scrollbar);
	rows     = (int)((yd_height - margin) / (mw_height + margin));
    }

    columns = (int)((yd_width  - margin) / (mw_width  + margin));

    rows    = MAX (rows, 1);
    columns = MAX (columns, 1);

    if (rows * columns > 12)
    {
        /*	  
        **  We've got a non-standard year display, force it to be standard. We
	**  won't show more than 12 months at a time.
        */	  
        if ((rows > 4) && (columns > 4))
	{
	    rows    = 4;
	    columns = 3;
	}
	else
	{
	    if (rows > columns)
	    {
		rows    = 12 / columns;
	    }
	    else
	    {
		columns = 12 / rows;
	    }
	}
    }

    if ((rows != 1) && (columns == 5))
    {
	columns = 4;
    }

    /*	  
    **  Save the number of rows and columns we've come up with.
    */	  
    yd->columns = columns;
    yd->rows    = rows;

    if (rows == 1)
    {
	yd_width  = XtWidth(yd->layout);
	yd_offset = 0;
    }
    else
    {
	yd_height = XtHeight(yd->layout);
	if (cd->profile.directionRtoL ^ rtol)
	{
	    yd_offset = (Position)XtWidth(yd->vert_scrollbar);
	}
	else
	{
	    yd_offset = 0;
	}
    }

    yd_width  = MAX (yd_width,  3);
    yd_height = MAX (yd_height, 3);

    margin_x = (yd_width  - (mw_width  * (Dimension)columns)) / (Dimension)(columns + 1);
    margin_y = (yd_height - (mw_height * (Dimension)rows))    / (Dimension)(rows    + 1);

    for (month = columns * rows;  month < 12;  month++)
    {
	XtUnmanageChild (yd->months [month]);
    }
    
    if (backwards)
    {
	month = (rows * columns) - 1;
	for (row = rows - 1;  row >= 0;  row--)
	{
	    for (column = columns - 1;  column >= 0;  column--)
	    {
		Position    month_x;
		month_x = yd_offset + margin_x +
		    (Position)((Dimension)column * (mw_width  + margin_x));
		if (rtol)
		    month_x = XtWidth(yd->layout) - month_x - mw_width;

		XtMoveWidget
		(
		    yd->months [month],
		    month_x,
		    margin_y +
			(Position)((Dimension)row * (mw_height + margin_y))
		);
		XtManageChild (yd->months [month]);
		month--;
	    }
	}
    }
    else
    {
	month = 0;
	for (row = 0;  row < rows;  row++)
	{
	    for (column = 0;  column < columns;  column++)
	    {
		Position    month_x;
		month_x = yd_offset + margin_x +
			(Position)((Dimension)column * (mw_width  + margin_x));
		if (rtol)
		    month_x = XtWidth(yd->layout) - month_x - mw_width;
		XtMoveWidget
		(
		    yd->months [month],
		    month_x,
		    margin_y +
			(Position)((Dimension)row * (mw_height + margin_y))
		);
		XtManageChild (yd->months [month]);
		month++;
	    }
	}
    }

    /*	  
    **  This gives an index in month units into the scrollbar. It assumes that
    **	the scrollbar size is [(years_on_scrollbar *12)/columns] + computed_slider_size..
    */
    computed_value =
	((yd->first_year % yd->years_on_scrollbar) * 12) + yd->first_month;

    if (rows == 1)
    { 
	computed_slider_size = columns;
	computed_inc = 1;
	computed_pageinc = computed_slider_size - 1;

	max = ((yd->years_on_scrollbar * 12)/rows) + computed_slider_size + 1;
	XtVaSetValues (yd->horiz_scrollbar, XmNmaximum, max, NULL);

	XmScrollBarGetValues
	    (yd->horiz_scrollbar, &scroll_value, &slider_size, &inc, &pageinc);

        /*	  
	**  Make sure we really need to set this.
	*/	  
        if ((scroll_value != computed_value) |
	    (slider_size != computed_slider_size) |
	    (inc != computed_inc) |
	    (pageinc != computed_pageinc))
	{
	    XmScrollBarSetValues
	    (
		yd->horiz_scrollbar,
		computed_value,		/* where on the scrollbar */
		computed_slider_size,	/* size of the slider */
		computed_inc,		/* increment */
		computed_pageinc,	/* page_increment */
		FALSE			/* don't call valuechanged  */
	    );
	}

    }
    else
    {
	computed_slider_size = rows;
	computed_inc = 1;
	computed_pageinc = computed_slider_size - 1;

	max = ((yd->years_on_scrollbar*12)/columns) + computed_slider_size + 1;

	XtVaSetValues (yd->vert_scrollbar, XmNmaximum, max, NULL);

	XmScrollBarGetValues
	    (yd->vert_scrollbar, &scroll_value, &slider_size, &inc, &pageinc);

        /*	  
	**  Make sure we really need to set this.
	*/	  
        if ((scroll_value != (computed_value/columns) + 1) |
	    (slider_size != computed_slider_size) |
	    (inc != computed_inc) |
	    (pageinc != computed_pageinc))
	{
	    /*	  
	    **	We set the scrollbar value to (value/columns) + 1 since we
	    **	never want the scrollbar to be at value 0 since that won't let
	    **	the user advance into the next decade, so if columns = 3 then
	    **	Jan = (1/3) + 1 or 1, and Apr = (4/3)+1 or 2, and if columns =
	    **	2 then Jan = (1/2) + 1 or 1 and Mar = (3/2) + 1 or 2, etc.
	    */	  
	    XmScrollBarSetValues
	    (
		yd->vert_scrollbar,
		(computed_value / columns) + 1,	/* where */
		computed_slider_size,		/* size of the slider */
		computed_inc,			/* increment */
		computed_pageinc,		/* page_increment */
		FALSE				/* don't call valuechanged */
	    );
	}

    }

    if (resize_actions)
    {
	/*	  
	**  Okay, bear with me 'cause this is complicated.  Calendar does
	**  something interesting with it's scrollbars in the Year and Month
	**  displays (or at least it tries to). Basically, the scrollbar is set
	**  up for some number of years (yd->years_on_scrollbar is 10 in this case)
	**  and the user is allowed to scroll up and down through the decade.
	**  Once the user gets to the "end" of the scrollbar in either
	**  direction, Calendar moves them to a new "decade" and moves the
	**  slider to the opposite side of the scroll bar to reflect that fact.
	**
	**  Under XUI this was a little simpler than under Motif since Calendar
	**  got a callback when the user clicked on the arrows or dragged the
	**  slider or clicked in the trough. Under Motif, Calendar only gets
	**  the callback if the user action resulted in the slider moving. This
	**  means that we have to pad the scroll bar in either direction so
	**  that there is someplace to move the slider even when we're at the
	**  "end" of the display.
	**
	**  Let's give some examples of the weird forumulas we're using,
	**  assuming that we're using 3 columns. If we want to display the line
	**  containing Jan 1990, it should be at slider value 1 since 0 is
	**  reserved for our "growing" into the next decade. Apr 90 would be
	**  slider value 2, etc. using the formula slider_value for a given
	**  month = (((year_of_decade * 12) + month of year)/columns) + 1.
	**  Simple isn't it? 8-). So, Jan 90 would be 1, Oct 90 would be 4, and
	**  Oct 99 would be 40.
	**
	**  Now for Oct 99, we would want the slider to be all the way at the
	**  bottom of the scrollarea - 1 for our expansion into the next
	**  decade, which would mean that we wanted our maximum value
	**  (different from XmNmaximum) to be 40 + 1 or 41 and our scrollbar
	**  size to be 41 + the slider_size (which is equal to the number of
	**  rows) + the 1 at the beginning of the scrollbar. So if we had 4
	**  rows, our XmNmaximum for the scrollbar would be 45.
	**
	**  The trick below is that we have to figure out the last month of a
	**  particular decade that can be displayed based on the columns and
	**  rows since we don't know which is the last month that can be
	**  displayed otherwise. If the rows = 1 then the last month of the
	**  year that can be the first position is obviously December (12).  If
	**  rows != 1, then we look at the number of columns. If we've got 4
	**  rows, and 3 columns (ie a year on the display at once), then the
	**  last month of the decade that can be displayed in the first
	**  position for the decade is October (eg Oct Nov Dec) month #118. So
	**  the last month that can be displayed is (months in decade) -
	**  columns + 1. If columns is 4 that will give us 117 ((10*12) -
	**  columns + 1) or Sep as the last month (eg Sep Oct Nov Dec).  So,
	**  substituting that knowlege in the above formula gives us the
	**  following (in the else clause).
	*/	  
	if (rows == 1)
	{
            /*	  
            **  Because Motif doesn't give callbacks when at the end of sliders
	    **	(unlike XUI) make sure that the sliders are big enough so that
	    **	the user can click to get to the next year (that is the 2 at the
	    **	end of the forumula, for 1 at each end).
            */
	    if (yd->vert_scrollbar != NULL)
	    {
		XtUnmanageChild (yd->vert_scrollbar);
	    }
	    XtUnmanageChild (yd->horiz_scrollbar);

	    XtConfigureWidget
	    (
		yd->horiz_scrollbar,
		margin + shadow_thickness,
		yd_height - margin,
		yd_width - (margin * 2) - (shadow_thickness * 2),
		(Dimension)XtHeight (yd->horiz_scrollbar),
		XtBorderWidth(yd->horiz_scrollbar)
	    );
	    XtManageChild   (yd->horiz_scrollbar);
	}
	else
	{
	    XtUnmanageChild (yd->vert_scrollbar);
	    XtUnmanageChild (yd->horiz_scrollbar);

	    if (cd->profile.directionRtoL)
	    {
		XtConfigureWidget
		(
		    yd->vert_scrollbar,
		    margin + shadow_thickness,
		    margin + shadow_thickness,
		    (Dimension)XtWidth (yd->vert_scrollbar),
		    yd_height - (2 * margin) - (2 * shadow_thickness),
		    XtBorderWidth(yd->vert_scrollbar)
		);
	    }
	    else
	    {
		XtConfigureWidget
		(
		    yd->vert_scrollbar,
		    yd_width - margin - shadow_thickness,
		    margin + shadow_thickness,
		    (Dimension)XtWidth (yd->vert_scrollbar),
		    yd_height - (2 * margin) - (2 * shadow_thickness),
		    XtBorderWidth(yd->vert_scrollbar)
		);
	    }

	    XtManageChild (yd->vert_scrollbar);
	}
    }
}

static void ErrorZeroTime
#ifdef _DWC_PROTO_
	(
	Widget	w,
	dwcaddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	dwcaddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    YearDisplay	    *yd = (YearDisplay *) tag;

    XtUnmanageChild (w);
    yd->zero_time_up = FALSE;
    
}

void YEARSetYearDisplayDate
#ifdef _DWC_PROTO_
	(
	YearDisplay	*yd,
	Cardinal	initial_month,
	Cardinal	initial_year)
#else	/* no prototypes */
	(yd, initial_month, initial_year)
	YearDisplay	*yd;
	Cardinal	initial_month;
	Cardinal	initial_year;
#endif	/* prototype */
{
    Cardinal	    month;
    Cardinal	    year;
    int		    old_index;
    int		    new_index;
    int		    difference;
    Cardinal	    start;
    Cardinal	    finish;
    Cardinal	    i;
    Boolean	    backwards;
    Widget	    widgets [12];
    
    if (initial_year < 1600)
    {
	if (! yd->zero_time_up)
	{
	    ERRORReportError
	    (
		(Widget)yd->layout,
		"ErrorZeroTime",
		(XtCallbackProc) ErrorZeroTime,
		(char *)yd
	    );
	    yd->zero_time_up = TRUE;
	}
	return;
    }

    month = initial_month;
    year  = initial_year;
    
    if ((yd->rows != 1) && (yd->columns != 1))
    {
        /*	  
        **  For multi-column month displays (and non-day_view), we need to
	**  figure out which month should appear at the far left side of the
	**  row.  This take the initial_month and comes up with the month that
	**  will be the first in the row. So, if inital_month is March, this
	**  will produce January, if initial_month is December, this will
	**  produce October.
        */	  
        month = (((month - 1) / yd->columns) * yd->columns) + 1;
    }
    
    old_index = (yd->first_year * 12) + yd->first_month;    
    new_index = (year * 12) + month;    

    yd->first_month = month;
    yd->first_year  = year;

    /*	  
    **	Figure out the difference in months between what is in the YearDisplay
    **	structure and what we're being asked to setup.
    */	  
    difference = new_index - old_index;

    if (difference == 0)
    {
        /*	  
	**  We're already setup just fine, thanks. Our 'initial_' stuff is the
	**  same as the 'first_' stuff in the YearDisplay structure. No need to
	**  do any mucking around
        */	  
        return;
    }

    backwards = FALSE;
    if ((difference <= -12) || (difference >= 12))
    {
        /*	  
        **  The difference is more than one year.
        */	  
	start  = 0; 
	finish = 11;
    }
    else
    {
        /*	  
        **  The difference is between -11 and 11.
        */	  
        if (difference > 0)
	{
            /*	  
            **  We want to advance the display in time, so save the current
	    **	month widgets that we'll overwrite which will be at the
	    **	beginning of the array.
            */	  
            for (i = 0;  i < difference;  i++)
	    {
		XtUnmanageChild (yd->months [i]);
		widgets [i] = yd->months [i];
	    }

            /*	  
            **  Move the month widgets up in order towards the beginning of the
	    **	array.
            */	  
            for (i = 0;  i < 12 - difference;  i++)
	    {
		yd->months [i] = yd->months [i + difference];
	    }

            /*	  
            **  Figure out where we will backfill with month widgets.
            */	  
            start  = 12 - difference;
	    finish = 11;

	    month = month + start;
            /*	  
            **  Are we going to end up displaying part of another year?
            */	  
            if (month > 12)
	    {
                /*	  
                **  Yes, reset the month to stay in the range of 1-12 and reset
		**  the year.
                */	  
                month = month - 12;
		year  = year + 1;
	    }

	}
	else
	{

            /*	  
            **  We're going back in time.
            */	  
            backwards  = TRUE;
	    difference = - difference;

	    for (i = 11;  i >= 12 - difference; i--)
	    {
		XtUnmanageChild (yd->months [i]);
		widgets [11 - i] = yd->months [i];
	    }

	    for (i = 11;  i >= difference;  i--)
	    {
		yd->months [i] = yd->months [i - difference];
	    }

	    start  = 0;
	    finish = difference - 1;

	}

	for (i = 0;  i < difference;  i++)
	{
	    int	foo;
	    foo = i + start;
	    yd->months [foo] = widgets [i];
	}
    }



    for (i = start;  i <= finish;  i++)
    {

	XtVaSetValues
	(
	    yd->months[i],
	    DwcMwNday, 1,
	    DwcMwNmonth, month,
	    DwcMwNyear,  year,
	    NULL
	);
	month = month + 1;
	if (month > 12)
	{
	    month = 1;
	    year = year + 1;
	}
    }

    YEARConfigureYearDisplay (yd, backwards, FALSE);

}

static void YEAR_LAYOUT_RESIZE
#ifdef _DWC_PROTO_
	(
	Widget	lw,
	dwcaddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(lw, tag, reason)
	Widget	lw;
	dwcaddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    YearDisplay	    *yd = (YearDisplay *) tag;
    
    YEARConfigureYearDisplay(yd, FALSE, TRUE);
    YEARSetYearDisplayDate (yd, yd->first_month, yd->first_year);

    if (yd->resize_callback != NULL)
    {
	(* yd->resize_callback) (yd, yd->tag);
    }

}

static void YEAR_DISPLAY_SCROLL
#ifdef _DWC_PROTO_
	(
	Widget				w,
	dwcaddr_t				tag,
	XmScrollBarCallbackStruct	*scroll)
#else	/* no prototypes */
	(w, tag, scroll)
	Widget				w;
	dwcaddr_t				tag;
	XmScrollBarCallbackStruct	*scroll;
#endif	/* prototype */
{
    int		    value, scroll_max, scroll_min, slider_size, inc, pageinc;
    int		    bottom_of_scrollbar;
    int		    msbosr;	/* months since beginning of scroll */
				/* region */
    int		    month, year;
    int		    status;
    CalendarDisplay cd;
    YearDisplay	    *yd = (YearDisplay *) tag;
    XmScrollBarWidget	sbw = (XmScrollBarWidget) w;
        
    status = MISCFindCalendarDisplay (&cd, w);
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

    /*	  
    **	Get the offset in months from the beginning of the scrollregion to
    **	where we were before we got this scroll callback. This divided by the
    **	number of columns should give us an index into the scrollregion.
    */	  
    msbosr = ((yd->first_year % yd->years_on_scrollbar) * 12) +
			yd->first_month -1;

    if (yd->rows == 1)
    {
        /*	  
	**  Horizontal scrolling
	*/	  
        switch (scroll->reason)
	{
	case XmCR_INCREMENT :
	    msbosr = msbosr + yd->rows;
	    break;
	case XmCR_DECREMENT :
	    msbosr = msbosr - yd->rows;
	    break;
	case XmCR_PAGE_INCREMENT :
	    msbosr = msbosr + ((yd->columns * yd->rows) - yd->rows);
	    break;
	case XmCR_PAGE_DECREMENT :
	    msbosr = msbosr - ((yd->columns * yd->rows) - yd->rows);
	    break;
	case XmCR_TO_TOP :
	    msbosr = 0;
	    break;
	case XmCR_TO_BOTTOM :
	    msbosr = (yd->years_on_scrollbar * 12) - (yd->rows);
	    break;
	default :
	    /*	  
	    **  msbosr is an offset into the year, ie Jan is 0, etc,
	    **  However, scroll->value runs from 0 - 14, so a scroll->value
	    **  of 1 (Jan) should be offset of 0, and a scroll->value of 12
	    **  (Dec) should be offset of 11.
	    */	  
	    msbosr = (scroll->value - 1) * yd->rows;
	    break;
	}
    }
    else
    {
        /*	  
	**  Vertical scrolling
	*/	  
        switch (scroll->reason)
	{
	case XmCR_INCREMENT :
	    msbosr = msbosr + yd->columns;
	    break;
	case XmCR_DECREMENT :
	    msbosr = msbosr - yd->columns;
	    break;
	case XmCR_PAGE_INCREMENT :
	    msbosr = msbosr + ((yd->columns * yd->rows) - yd->columns);
	    break;
	case XmCR_PAGE_DECREMENT :
	    msbosr = msbosr - ((yd->columns * yd->rows) - yd->columns);
	    break;
	case XmCR_TO_TOP :
	    /*	  
	    **  Our offset from the beginning is going to be 0
	    */	  
	    msbosr = 0;
	    break;
	case XmCR_TO_BOTTOM :
	    /*	  
	    **  Our offset from the beginning is going to be the size of the
	    **  scroll area minus the number of months in the display so
	    **  that the last month is the last month in the display.
	    */	  
	    msbosr = (yd->years_on_scrollbar * 12) - (yd->columns);
	    break;
	default :
	    /*	  
	    **  msbosr is an offset into the decade, ie Jan00 is 0, etc,
	    **  However, scroll->value runs from 0 - 44, so a scroll->value
	    **  of 1 (Jan) should be offset of 0, and a scroll->value of 40
	    **  (Oct09) should be offset of 117.
	    */	  
	    msbosr = (scroll->value - 1) * yd->columns ;
	    break;
	}
    }

    /*	  
    **	Get the number of months (since year 0) to the beginning of our
    **	scrollregion and then add the necessary months since the beginning of
    **	our scrollregion that we've been asked to move.
    */	  
    month = ((yd->first_year / yd->years_on_scrollbar) * yd->years_on_scrollbar * 12)
		    + msbosr;
    /*	  
    **  Get the year number
    */	  
    year  = month / 12;

    /*	  
    **  Get the month
    */
    month = (month % 12) + 1;

    YEARSetYearDisplayDate(yd, month, year);

    /*
    **  Let's make sure we're not too far at either end.
    */
    XtVaGetValues
    (
	w,
    	XmNmaximum, &scroll_max,
    	XmNminimum, &scroll_min,
    	XmNvalue, &value,
    	XmNsliderSize, &slider_size,
    	XmNincrement, &inc,
    	XmNpageIncrement, &pageinc,
	NULL
    );

    /*	  
    **  We can't let the slider be at zero since that will preclude the user
    **	from sliding into the next year.
    */	  
    if (value <= scroll_min)
    {
	value = scroll_min + 1;
	XmScrollBarSetValues(w, value, slider_size, inc, pageinc, FALSE);
    }
    else
    {
	/*	  
	**  If the slider is at the end of the scrollbar we want to back it up
	**  one so that the user has room to slide into the next year with an
	**  increment.
	*/	  
	bottom_of_scrollbar = scroll_max - slider_size;
	if (value >= bottom_of_scrollbar)
	{
	    value = value - 1;
	    XmScrollBarSetValues(w, value, slider_size, inc, pageinc, FALSE);
	}
    }

    XSync (XtDisplay (w), FALSE);
}

YearDisplay *YEARCreateYearDisplay
#ifdef _DWC_PROTO_
	(
	Widget			parent,
	char			*name,
	Dimension		width,
	Dimension		height,
	Boolean			vertical_scrollbar,
	Cardinal		first_year,
	Cardinal		years_on_scrollbar,
	Arg			*passed_arglist,
	Cardinal		num_passed_args,
	XtCallbackList		year_help_cb,
	XtCallbackList		scroll_help_cb,
	YearResizeCallbackProc	resize_callback,
	dwcaddr_t		tag
	)
#else	/* no prototypes */
	(parent, name, width,height,vertical_scrollbar, first_year,
	years_on_scrollbar, passed_arglist, num_passed_args, year_help_cb,
	scroll_help_cb, resize_callback, tag)
        Widget			parent;
	char			*name;
	Dimension		width;
	Dimension		height;
	Boolean			vertical_scrollbar;
	Cardinal		first_year;
	Cardinal		years_on_scrollbar;
	Arg			*passed_arglist;
	Cardinal		num_passed_args;
	XtCallbackList		year_help_cb;
	XtCallbackList		scroll_help_cb;
	YearResizeCallbackProc	resize_callback;
	dwcaddr_t		tag;
#endif	/* prototype */
{
    YearDisplay		*yd;
    Cardinal		month;
    Cardinal		ac;
    Arg			*arglist;
    Cardinal		scroll_args = 21;
    Cardinal		month_args  = 6;
    Cardinal		slider_size = 1;
    Boolean		rtol;

    yd = (YearDisplay *) XtMalloc (sizeof (YearDisplay));
    
    yd->first_month	= 1;
    yd->first_year	= first_year;

    /*	  
    **  Save number of years to display in scrollbar, eg. if years_on_scrollbar is 10
    **	then the scrollbar will show a decade at a time.
    */	  
    yd->years_on_scrollbar	= years_on_scrollbar;
    yd->rows		= 1;
    yd->columns		= 1;
    yd->resize_callback = resize_callback;
    yd->tag	        = tag;
    yd->zero_time_up    = FALSE;
    
    year_layout_resizecb [0].closure = (dwcaddr_t) yd;
    yd->layout = LwLayout
    (
	parent,
	name,
	False,
	(Position)0, (Position)0,
	(Dimension)width, (Dimension)height,
	(XtCallbackList)year_layout_resizecb,
	(XtCallbackList)NULL,
	(XtCallbackList)NULL,
	(XtCallbackList)year_help_cb,
	(dwcaddr_t) yd
    );
    XtVaSetValues
    (
	(Widget) yd->layout,
	XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
	XmNtraversalOn, True,
	XmNshadowThickness, 2,
	XmNshadowType, XmSHADOW_OUT,
	XmNmarginHeight, 3,
	XmNmarginWidth, 3,
	NULL
    );

    arglist = (Arg *) XtMalloc
	(sizeof (Arg) * MAX (scroll_args, month_args + num_passed_args));

    year_display_scroll_cb [0].closure = (dwcaddr_t) yd;

    /*
    ** Make the months widgets for the year.
    */
    for (ac = 0;  ac < num_passed_args;  ac++)
    {
	arglist [ac] = passed_arglist [ac];
    }

    /*
    ** Force it because osf compiler is confused.
    */
    ac = num_passed_args;
    XtSetArg (arglist [ac], DwcMwNday,  1);                  ac++;
    XtSetArg (arglist [ac], DwcMwNyear, first_year);         ac++;
    XtSetArg (arglist [ac], DwcMwNmode, DwcMwModeWholeMonth);  ac++;
    XtSetArg (arglist [ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;
    XtSetArg (arglist [ac], XmNtraversalOn, True);	    ac++;

    for (month = 1;  month <= 12;  month++)
    {
	XtSetArg (arglist [ac], DwcMwNmonth, month);

	assert ((ac + 1) <= MAX (scroll_args, month_args + num_passed_args));

	yd->months [month - 1] = MWMonthCreate
	    ((Widget) (yd->layout), "Months", arglist, ac + 1);
    }

    /*
    ** Create the scroll bars.
    */
    ac = 0;
    XtSetArg(arglist[ac], XmNincrement,	1); ac++;
    XtSetArg(arglist[ac], XmNpageIncrement, 1); ac++;
    XtSetArg(arglist[ac], XmNsliderSize, slider_size); ac++;
    XtSetArg(arglist[ac], XmNvalue, 1); ac++;
    XtSetArg(arglist[ac], XmNminimum, 0); ac++;

    /*	  
    **  XUI gave a callback when at the end of the scrollbar. Motif doesn't so
    **	leave a space at each end. (year * 12) plus the size of the slider plus
    **	a space at each end.
    */
    XtSetArg(arglist[ac], XmNmaximum, (years_on_scrollbar * 12) + 1); ac++;
    XtSetArg(arglist[ac], XmNtraversalOn, True); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;
    XtSetArg(arglist[ac], XmNhelpCallback, scroll_help_cb); ac++;
    XtSetArg(arglist[ac], XmNincrementCallback, year_display_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNdecrementCallback, year_display_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNpageIncrementCallback, year_display_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNpageDecrementCallback, year_display_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNtoTopCallback, year_display_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNtoBottomCallback, year_display_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNvalueChangedCallback, year_display_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNdragCallback, year_display_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    XtSetArg(arglist[ac], XmNhighlightThickness, 2); ac++;

    if (vertical_scrollbar)
    {
	XtSetArg(arglist[ac], XmNorientation, XmVERTICAL); ac++;

	assert (ac <= MAX (scroll_args, month_args + num_passed_args));

	yd->vert_scrollbar = XmCreateScrollBar
	    ((Widget)yd->layout, "VScrollbar", arglist, ac);
	ac--;
    }
    else
    {
	yd->vert_scrollbar = NULL;
    }

    XtSetArg(arglist[ac], XmNorientation, XmHORIZONTAL);  ac++;

#ifdef DEC_MOTIF_EXTENSION
    XtSetArg(
	arglist[ac],
	DXmNlayoutDirection,
	yd->layout->manager.dxm_layout_direction
    ); ac++;
#else
    rtol = (yd->layout->manager.string_direction == XmSTRING_DIRECTION_R_TO_L);
    if (rtol)
    {
	XtSetArg(arglist[ac], XmNprocessingDirection, XmMAX_ON_LEFT); ac++;
    }
    else
    {
	XtSetArg(arglist[ac], XmNprocessingDirection, XmMAX_ON_RIGHT); ac++;
    }
#endif

    assert (ac <= MAX (scroll_args, month_args + num_passed_args));

    yd->horiz_scrollbar = XmCreateScrollBar
	((Widget)yd->layout, "HScrollbar", arglist, ac);

    XtFree ((char *) arglist);

    return (yd);
    
}

/* Returns the height of a single row of the month widget */
Dimension YEARSingleRowHeight
#ifdef _DWC_PROTO_
	(
	YearDisplay	*yd)
#else	/* no prototypes */
	(yd)
	YearDisplay	*yd;
#endif	/* prototype */
{
    Dimension		mw_height;
    Dimension		sb_height;
    Dimension		margin = 10;


    mw_height = XtHeight (yd->months [0]);
    sb_height = XtHeight (yd->horiz_scrollbar);

    return (mw_height + sb_height + (margin * 2));
    
}

void YEARSetAllMonths
#ifdef _DWC_PROTO_
	(
	YearDisplay	*yd,
	Arg		*arglist,
	Cardinal	ac)
#else	/* no prototypes */
	(yd, arglist, ac)
	YearDisplay	*yd;
	Arg		*arglist;
	Cardinal	ac;
#endif	/* prototype */
{
    Cardinal		i;

    for (i = 0;  i < 12;  i++)
    {
	XtSetValues (yd->months [i], arglist, ac);
    }

}

void YEARResize
#ifdef _DWC_PROTO_
	(
	YearDisplay	*yd,
	dwcaddr_t		tag)
#else	/* no prototypes */
	(yd, tag)
	YearDisplay	*yd;
	dwcaddr_t		tag;
#endif	/* prototype */
{

    Dimension		width,height;

    CalendarDisplay	cd = (CalendarDisplay) tag;

    XtVaGetValues (cd->mainwid, XmNwidth, &width, XmNheight, &height, NULL);

    cd->profile.year_width = (((Cardinal)width * 100) -
	(cd->screen_font_size - 1))/ cd->screen_font_size;
    cd->profile.year_height = (((Cardinal)height * 100) -
	(cd->screen_font_size - 1))/ cd->screen_font_size;
}

Widget YEARGetMonthWidget
#if defined(_DWC_PROTO_)
	(
	YearDisplay	*yd,
	int		month
	)
#else
	(yd, month)
	YearDisplay	*yd;
	int		month;
#endif
/*
**++
**
**  This routine scans the month array and finds the month widget displaying
**  the requested month number.
**
**--
*/
{
    int		i;
    int		mw_month;

    for (i=0; i<12; i++)
    {
	if (yd->months[i] == NULL) continue;

	XtVaGetValues (yd->months[i], DwcMwNmonth, &mw_month, NULL);

	if (mw_month == month) return ((Widget)yd->months[i]);
    }

    return ((Widget)0);
}

/* $Id$ */
#ifndef _yeardisplay_h_
#define _yeardisplay_h_ 1
/* #module dwc_ui_yeardisplay.h */
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
**	Generic year display information.
**
**--
*/


#include    "dwc_compat.h"
#include    "dwc_ui_layout.h"		/* for LayoutWidget */

/*
**  Year display widgets...
*/
struct _YearDisplay;

typedef void (*YearResizeCallbackProc) PROTOTYPE ((
	struct _YearDisplay	*yd,
	dwcaddr_t	tag));

typedef struct _YearDisplay {
    LayoutWidget    layout ;
    Widget	    horiz_scrollbar ;
    Widget	    vert_scrollbar ;
    Widget	    months [12] ;  
    Cardinal        first_month ;  
    Cardinal        first_year ;   
    Cardinal        columns ;      
    Cardinal        rows ;         
    Cardinal	    years_on_scrollbar ;
    YearResizeCallbackProc  resize_callback;
    dwcaddr_t	    tag ;
    Boolean	    zero_time_up ;
} YearDisplay ;


void YEARSetYearDisplayDate PROTOTYPE ((
	YearDisplay	*yd,
	Cardinal	initial_month,
	Cardinal	initial_year));

void YEARConfigureYearDisplay PROTOTYPE ((
	YearDisplay	*yd,
	Boolean		backwards,
	Boolean		resize_actions));

YearDisplay *YEARCreateYearDisplay PROTOTYPE ((
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
	dwcaddr_t		tag));

void YEARResize PROTOTYPE ((
	YearDisplay	*yd,
	dwcaddr_t		tag));

Dimension YEARSingleRowHeight PROTOTYPE ((
	YearDisplay	*yd));

void YEARSetAllMonths PROTOTYPE ((
	YearDisplay	*yd,
	Arg		*arglist,
	Cardinal	ac));

Widget YEARGetMonthWidget PROTOTYPE ((
	YearDisplay	*yd,
	int		month));

#endif /* _yeardisplay_h_ */

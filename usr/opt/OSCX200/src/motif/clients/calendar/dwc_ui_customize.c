/* dwc_ui_customize.c */
#ifndef lint
static char rcsid[] = "$Header$";
#endif /* lint */
/*
**   Copyright (c) Digital Equipment Corporation, 1990
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
**
*++
**  FACILITY:
**
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Marios Cleovoulou, November-1987
**	Ken Cowan February-1989
**
**  ABSTRACT:
**
**	This module contains the support for user customization.
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
#include <X11/Intrinsic.h>		/* stuff like Boolean */
#include <Xm/Xm.h>			/* for XmToggleButtonCallbackStruct */
#include <Xm/Scale.h>			/* for XmScaleSet.. */
#include <Xm/ToggleB.h>
#include <X11/DECwI18n.h>
#include <DXm/DECspecific.h>
#include <dwi18n_lib.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_dateformat.h"   	/* dtb... */
#include "dwc_ui_calendar.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* DWC$m_*, etc. */
#include "dwc_ui_iconboxwidget.h" 	/* DwcIbwCallbackStruc, etc. */
#include "dwc_ui_custom.h"		/* procedure declarations */
#include "dwc_ui_misc.h"	  	/* MISCFindCalendarDisplay */
#include "dwc_ui_month.h"		/* MONTHConfigMonthDisplay */
#include "dwc_ui_day.h"			/* for DAYConfigureDayDisplay */
#include "dwc_ui_yeardisplay.h"		/* for YEARConfigureYearDisplay */
#include "dwc_ui_monthwidget.h"		/* for MWMARKUP and MWSTYLE */
#include "dwc_ui_alarms.h"
#include "dwc_ui_errorboxes.h"
#include "dwc_ui_icons.h"
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_sloteditor.h"		/* for SLOTEDITORWIDGETS */

extern XmString	*time_texts_24;		/* defined in dwc_ui_calendar.c */
extern XmString	*time_texts_12;		/* defined in dwc_ui_calendar.c */
extern XmString *minute_texts;		/* defined in dwc_ui_calendar.c */
extern Pixmap	small_pixmaps[];	/* defined in dwc_ui_misc.c */
extern Pixmap	medium_pixmaps[];	/* defined in dwc_ui_misc.c */
extern Pixmap	defdep_medium_pixmaps[];/* defined in dwc_ui_misc.c */
extern Pixmap	big_pixmaps[];		/* defined in dwc_ui_misc.c */
extern Pixmap	dwc_bell_negative_32X32;	/* defined in dwc_ui_misc.c */
extern Pixmap	dwc_bell_negative_12X12;	/* defined in dwc_ui_misc.c */


static void register_names PROTOTYPE ((void));

static int month_lengths [12] =
    {31, 28, 31, 30, 31, 30, 31, 31, 30,  31, 30, 31};

void fetch_alarms_new PROTOTYPE ((CalendarDisplay cd, ProfileStructure *new));

void fetch_dayview_new PROTOTYPE ((CalendarDisplay cd, ProfileStructure *new));

void fetch_general_new PROTOTYPE ((CalendarDisplay cd, ProfileStructure *new));

void fetch_iconbox_new PROTOTYPE ((CalendarDisplay cd, ProfileStructure *new));

static void increment_time_widget PROTOTYPE ((Widget w, CalendarDisplay cd));

static void set_work_hours_buttons PROTOTYPE ((
	Widget			start_hours,
	Widget			start_mins,
	Widget			start_ampm,
	Widget			finish_hours,
	Widget			finish_mins,
	Widget			finish_ampm,
	unsigned short int	start,
	unsigned short int	finish,
	Boolean			time_am_pm));

static void setup_icon PROTOTYPE ((CalendarDisplay cd));

/*
** Callback routines
*/

void DO_ICONBOX_RESET PROTOTYPE ((Widget w, caddr_t tag));

void DO_ALARMS_OK PROTOTYPE ((Widget w, caddr_t	tag));

void DO_ALARMS_CANCEL PROTOTYPE ((Widget w, caddr_t tag));

Boolean DO_ALARMS_APPLY PROTOTYPE ((Widget w, caddr_t tag));

void DO_ALARMS_RESET PROTOTYPE ((Widget w, caddr_t tag));

void DO_DAYVIEW_OK PROTOTYPE ((Widget w, caddr_t tag));

void DO_DAYVIEW_CANCEL PROTOTYPE ((Widget w, caddr_t tag));

Boolean DO_DAYVIEW_APPLY PROTOTYPE ((Widget w, caddr_t tag));

void DO_DAYVIEW_RESET PROTOTYPE ((Widget w, caddr_t tag));

Boolean DO_GENERAL_APPLY PROTOTYPE ((Widget w, caddr_t tag));

void DO_GENERAL_CANCEL PROTOTYPE ((Widget w, caddr_t tag));

void DO_ICONBOX_OK PROTOTYPE ((Widget w, caddr_t tag));

void DO_ICONBOX_CANCEL PROTOTYPE ((Widget w, caddr_t tag));

Boolean DO_ICONBOX_APPLY PROTOTYPE ((Widget w, caddr_t tag));

static void FINE_INCR_CONSISTENCY PROTOTYPE ((Widget w));

void FIRST_WEEK_CONSISTENCY PROTOTYPE ((
	Widget				w,
	caddr_t				tag,
	XmAnyCallbackStruct		*cb));

static void TIME_SET PROTOTYPE ((
	Widget				w,
	caddr_t				tag,
	XmAnyCallbackStruct		*cbs));

static void TOGGLE_ALARM_SCALE PROTOTYPE ((
	Widget				w,
	caddr_t				tag,
	XmToggleButtonCallbackStruct	*cbs));


void CUSTDO_GENERAL_DISPLAY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget	w;
	caddr_t	tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    XtWidgetGeometry	request ;
    XtGeometryResult	result ;
    CalendarDisplay	cd ;
    int			status ;
    MrmType		class ;
    Widget		*children;
    Cardinal		num_children;

    status = MISCFindCalendarDisplay (&cd, w) ;

    if (cd->db_general_up) {
	request.request_mode = CWStackMode ;
	request.stack_mode   = Above ;
    	result = XtMakeGeometryRequest (cd->general_db, &request, NULL) ;
	return ;
    }

    cd->original.first_display            = cd->profile.first_display ;
    cd->original.week_numbers_start_month = cd->profile.week_numbers_start_month ;
    cd->original.week_numbers_start_day   = cd->profile.week_numbers_start_day ;
    cd->original.work_days                = cd->profile.work_days ;
    cd->original.first_day_of_week        = cd->profile.first_day_of_week ;

    cd->original.fill_blank_days          = cd->profile.fill_blank_days ;
    cd->original.direct_scroll_coupling   = cd->profile.direct_scroll_coupling ;
    cd->original.time_am_pm               = cd->profile.time_am_pm ;
    cd->original.start_iconized           = cd->profile.start_iconized ;
    cd->original.show_week_numbers        = cd->profile.show_week_numbers ;
    cd->original.directionRtoL            = cd->profile.directionRtoL ;
    cd->original.print_blank_days	  = cd->profile.print_blank_days ;
    cd->original.month_style_mask         = cd->profile.month_style_mask ;

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor) ;

    if (! cd->db_general_created)
    {
	/*
	** Make sure all pixmaps are loaded.
	*/
	MISCLoadPixmapArrays (cd->mainwid, 0);
	MISCLoadPixmapArrays (cd->mainwid, 1);
	MISCLoadPixmapArrays (cd->mainwid, 2);

	register_names () ;
        /*	  
	**  Just realize it, don't manage it.
	*/	  
        MISCFetchWidget(w, "customize_general_box", NULL, FALSE) ;

	CUSTSetGeneralFromProfile (cd, &cd->original) ;
	cd->db_general_created = TRUE ;

	/*
	** Clean up the layout between when we create the windows and get it
	** on the screen.
	*/
	children = DXmChildren ((CompositeWidget)cd->pb_gen_controls);
	num_children = DXmNumChildren ((CompositeWidget)cd->pb_gen_controls);
	MISCWidthButtonsEqually (children, num_children);

	XtManageChild (cd->general_db);

	MISCSpaceButtonsEqually (children, num_children);

	/*
	** Make the CLOSE item in the window manager menu call the CANCEL
	** callback.
	*/
	MISCAddProtocols
	(
	    XtParent(cd->general_db),
	    (XtCallbackProc) DO_GENERAL_CANCEL,
	    NULL
	);

    }
    else
    {
	XtManageChild (cd->general_db);
    }

    /*
    ** Tell the dialog to take focus if no explict focusing has been done.
    */
    MISCFocusOnMap (XtParent(cd->general_db), NULL);

    ICONSWaitCursorRemove (cd->mainwid) ;
    cd->db_general_up = TRUE ;

}

void CUSTSetGeneralFromProfile
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*profile)
#else	/* no prototypes */
	(cd, profile)
	CalendarDisplay		cd;
	ProfileStructure	*profile;
#endif	/* prototype */
{
    int			bit ;
    int			day ;
    Arg			arglist [1] ;


    switch (profile->first_display) {
      case show_day   :
	XtSetArg(arglist[0], XmNmenuHistory, cd->first_dpy_day);
	break ;
      case show_month :
	XtSetArg(arglist[0], XmNmenuHistory, cd->first_dpy_month);
	break ;
      case show_year :
	XtSetArg(arglist[0], XmNmenuHistory, cd->first_dpy_year);
	break ;
    }
    XtSetValues (cd->om_first_dpy, arglist, 1);

    /* 
    **  Set the month of the first week 
    */

    XtSetArg(arglist[0], XmNmenuHistory, 
	cd->week_num_start_month_pbs[profile->week_numbers_start_month - 1]);
    XtSetValues (cd->week_num_start_month_om, arglist, 1);
    FIRST_WEEK_CONSISTENCY(cd->week_num_start_month_pbs[profile->week_numbers_start_month - 1], 
		    NULL, NULL);

    /*	  
    **  Set the scale in the general customize box to the start day for the week
    **	numbers.
    */	  
    XmScaleSetValue (cd->week_num_start_day_scale,
			profile->week_numbers_start_day) ;
    FIRST_WEEK_CONSISTENCY(cd->week_num_start_day_scale, NULL, NULL) ;


    bit = 0100 ;
    for (day = 0 ;  day < 7 ;  day++) {
	XmToggleButtonSetState
	  (cd->wkd [day],  ((bit & profile->work_days) != 0), FALSE) ;

	if (profile->first_day_of_week == day + 1) {
	    XtSetArg(arglist[0], XmNmenuHistory, cd->fdow [day]);
	    XtSetValues (cd->fdow_option_menu, arglist, 1);
	}

	bit = bit >> 1 ;
    }


    XmToggleButtonSetState (cd->time_am_pm,    profile->time_am_pm,        FALSE) ;
    XmToggleButtonSetState (cd->start_icon_tb, profile->start_iconized,    FALSE) ;
    XmToggleButtonSetState (cd->month_swn,     profile->show_week_numbers, FALSE) ;
    XmToggleButtonSetState (cd->r_to_l_tb,     profile->directionRtoL,     FALSE) ;

    XmToggleButtonSetState (cd->month_fbd,
	                     profile->fill_blank_days,        FALSE) ;
    XmToggleButtonSetState (cd->scrollbar_coupling_tb,
			     profile->direct_scroll_coupling, FALSE) ;

    XmToggleButtonSetState (cd->print_blank_tb,
			     profile->print_blank_days, FALSE) ;

    XmToggleButtonSetState (cd->box_current_day_tb,
			     ((profile->month_style_mask & MWSTYLE_SQUARE) != 0),
			     FALSE) ;
    if ((profile->month_style_mask & MWSTYLE_SLASH_THRU) != 0) {
	XtSetArg(arglist[0], XmNmenuHistory, cd->month_style_slash_pb);
    } else {
	if ((profile->month_style_mask & MWSTYLE_CROSS_THRU) != 0) {
	    XtSetArg(arglist[0], XmNmenuHistory, cd->month_style_cross_pb);
	} else {
	    XtSetArg(arglist[0], XmNmenuHistory, cd->month_style_clean_pb);
	}
    }
    XtSetValues (cd->month_style_menu, arglist, 1);


}

/*	  
**  This routine ensures consistency between the scale widget and the option
**  menu of pushbuttons in the customize general box used for selecting the
**  day and month of the first week of the year.
*/	  
void FIRST_WEEK_CONSISTENCY
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cb)
#else	/* no prototypes */
	(w, tag, cb)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cb;
#endif	/* prototype */
{
    int				month, day_first_week ;
    int				oldmax ;
    Arg				arglist [1] ;
    CalendarDisplay		cd ;
    int				status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    /*	  
    **  See if we got a scale widget
    */	  
    if (w == cd->week_num_start_day_scale) {
	XmScaleGetValue (cd->week_num_start_day_scale, &day_first_week) ;
	for (month = 0 ;  month < 12 ;  month++) {
	    if (month_lengths [month] != 31) {
                /*	  
                **  If this month has less days than the current value of the
		**  scale then set it insensitive since it shouldn't be
		**  selectable
                */	  
                XtSetSensitive (cd->week_num_start_month_pbs[month], 
				(day_first_week <= month_lengths [month])) ;
	    }
	}
    } else {		    
        /*	  
        **  We don't have a scale widget but something else (probably one of the
	**  pushbuttons with month names on it for selecting the first month of
	**  the year)
        */	  
        XtSetArg (arglist[0], XmNmaximum, &oldmax);
        /*	  
        **  Get the maximum value on the scale widget.
        */	  
        XtGetValues (cd->week_num_start_day_scale, arglist, 1) ;
	for (month = 0 ;  month < 12 ;  month++) {
	    if (w == cd->week_num_start_month_pbs[month]) {
		if (oldmax != month_lengths [month]) {
                    /*	  
                    **  If necesssary update the maximum of the scale widget to
		    **	match the size of the currently selected month.
                    */	  
                    XtSetArg (arglist [0], XmNmaximum, month_lengths [month]);
		    XtSetValues (cd->week_num_start_day_scale, arglist, 1) ;
		}
		break ;
	    }
	}
    }

}

Boolean DO_GENERAL_APPLY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
{
    CalendarDisplay cd ;
    int		    status ;
    ProfileStructure   new;

    status = MISCFindCalendarDisplay (&cd, w) ;

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor) ;

    new = cd->profile;
    fetch_general_new( cd, &new );

    if (CUSTApplyProfile ( cd, &new )) {
	ICONSWaitCursorRemove (cd->mainwid) ;
	return (TRUE) ;
    } else {
	return (FALSE) ;
    }
    
}

void fetch_general_new
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*new)
#else	/* no prototypes */
	(cd, new)
	CalendarDisplay		cd;
	ProfileStructure	*new;
#endif	/* prototype */
{
    int		    bit;
    Cardinal	    ac;
    Arg		    arglist [10];
    Widget	    option;
    int		    day;
    int		    i;
    int		    temp;

    /*
    **  Get work days
    */
    
    bit = 0100 ;
    new->work_days = 0 ;
    for (day = 0 ;  day < 7 ;  day++) {
	if (XmToggleButtonGetState (cd->wkd [day])) {
	    new->work_days = new->work_days | bit ;
	}
	bit = bit >> 1 ;
    }

    /*
    **  Get first display
    */
    
    XtSetArg(arglist[0], XmNmenuHistory, &option);
    XtGetValues (cd->om_first_dpy, arglist, 1);

    if (option == cd->first_dpy_day) {
	new->first_display = show_day ;
    } else {
	if (option == cd->first_dpy_month) {
	    new->first_display = show_month ;
	} else {
	    if (option = cd->first_dpy_year) {
		new->first_display = show_year ;
	    }
	}
    }

    new->start_iconized = XmToggleButtonGetState (cd->start_icon_tb) ;
    new->direct_scroll_coupling = 
			 XmToggleButtonGetState (cd->scrollbar_coupling_tb) ;
    new->print_blank_days       = 
			 XmToggleButtonGetState (cd->print_blank_tb) ;

    /* 
    **  Get the month & day of the first week from the option menu 
    */

    XtSetArg(arglist[0], XmNmenuHistory, &option);
    XtGetValues (cd->week_num_start_month_om, arglist, 1);

    for (i = 0 ;  i < 12 ;  i++) {
	if (option == cd->week_num_start_month_pbs[i]) {
	    new->week_numbers_start_month = i + 1 ;
	    break ;
	}
    }

    XmScaleGetValue (cd->week_num_start_day_scale, &temp ) ;
    new->week_numbers_start_day = temp;

    /*
    **  Get first day of week
    */
    
    XtSetArg(arglist[0], XmNmenuHistory, &option);
    XtGetValues (cd->fdow_option_menu, arglist, 1);

    for (day = 0 ;  day < 7 ;  day++) {
	if (option == cd->fdow [day]) {
	    new->first_day_of_week = day + 1 ;
	    break ;
	}
    }

    /*
    **  Get rest of toggles
    */
    
    new->time_am_pm        = XmToggleButtonGetState (cd->time_am_pm) ;
    new->show_week_numbers = XmToggleButtonGetState (cd->month_swn) ;
    new->directionRtoL     = XmToggleButtonGetState (cd->r_to_l_tb) ;
    new->fill_blank_days   = XmToggleButtonGetState (cd->month_fbd) ;

    if (XmToggleButtonGetState (cd->box_current_day_tb)) {
	new->month_style_mask = MWSTYLE_SQUARE ;
    } else {
	new->month_style_mask = 0 ;
    }

    XtSetArg(arglist[0], XmNmenuHistory, &option);
    XtGetValues (cd->month_style_menu, arglist, 1);

    if (option == cd->month_style_slash_pb) {
	new->month_style_mask = new->month_style_mask | MWSTYLE_SLASH_THRU ;
    } else {
	if (option == cd->month_style_cross_pb) {
	    new->month_style_mask = new->month_style_mask | MWSTYLE_CROSS_THRU ;
	}
    }

    return ;
}

void
DO_GENERAL_OK
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    if (DO_GENERAL_APPLY (w, tag)) {
	cd->db_general_up = FALSE ;
	XtUnmanageChild (cd->general_db) ;
    }

}

void
DO_GENERAL_RESET
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    CUSTSetGeneralFromProfile(cd, &cd->original);

}

void
DO_GENERAL_CANCEL
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    DO_GENERAL_RESET (w, tag) ;
    DO_GENERAL_OK    (w, tag) ;

}

extern int MbaTimerValue ;

static void time_inc_timer_tick
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{

    increment_time_widget(cd->button_timer_widget, cd);

    cd->button_timer_id = XtAppAddTimeOut
    (
	CALGetAppContext(),
	MbaTimerValue,
	(XtTimerCallbackProc)time_inc_timer_tick,
	cd
    );

    cd->button_timer_fired  = TRUE;

}

static void TIME_SET
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget	w;
	caddr_t	tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay	cd ;
    int		        status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    switch (cbs->reason)
    {
    case XmCR_ARM: 
	cd->button_timer_id = XtAppAddTimeOut
	(
	    CALGetAppContext(),
	    MbaTimerValue * 2,
	    (XtTimerCallbackProc)time_inc_timer_tick,
	    cd
	);
        cd->button_timer_widget = w;
        cd->button_timer_fired  = FALSE;
	return;
    case XmCR_DISARM: 
	XtRemoveTimeOut (cd->button_timer_id);
	cd->button_timer_id = 0;
	return;
    }

    if (!cd->button_timer_fired)
    {
	increment_time_widget (w, cd);
    }

}

static void
set_work_hours_buttons
#if defined(_DWC_PROTO_)
	(
	Widget			start_hours,
	Widget			start_mins,
	Widget			start_ampm,
	Widget			finish_hours,
	Widget			finish_mins,
	Widget			finish_ampm,
	unsigned short int	start,
	unsigned short int	finish,
	Boolean			time_am_pm)
#else	/* no prototypes */
	(start_hours, start_mins, start_ampm, finish_hours, finish_mins, finish_ampm, start, finish, time_am_pm)
	Widget			start_hours;
	Widget			start_mins;
	Widget			start_ampm;
	Widget			finish_hours;
	Widget			finish_mins;
	Widget			finish_ampm;
	unsigned short int	start;
	unsigned short int	finish;
	Boolean			time_am_pm;
#endif	/* prototype */
    {
    XmString	hour_text ;
    XmString	mins_text ;
    XmString	ampm_text ;
    int			hour_format ;
    Arg			arglist [1] ;
    dtb			date_time ;

        
    if (time_am_pm) {
	hour_format = dwc_k_cust_hour_ampm_format ;
    } else {
	hour_format = dwc_k_cust_hour_format ;
    }

    date_time.hour   = start / 60 ;
    date_time.minute = start % 60 ;

    hour_text = DATEFORMATTimeToCS(hour_format, &date_time) ;
    mins_text = DATEFORMATTimeToCS(dwc_k_cust_min_format, &date_time) ;

    if (time_am_pm) {
	ampm_text = DATEFORMATTimeToCS(dwc_k_cust_ampm_format, &date_time) ;
    } else {
	ampm_text = XmStringCreateSimple("") ;
    }

    XtSetArg (arglist[0], XmNlabelString, hour_text);
    XtSetValues (start_hours, arglist, 1) ;
    XmStringFree (hour_text) ;

    XtSetArg (arglist[0], XmNlabelString, mins_text);
    XtSetValues (start_mins,  arglist, 1) ;
    XmStringFree (mins_text) ;

    XtSetArg (arglist[0], XmNlabelString, ampm_text);
    XtSetValues (start_ampm,  arglist, 1);
    XmStringFree (ampm_text) ;


    date_time.hour   = finish / 60 ;
    date_time.minute = finish % 60 ;

    hour_text = DATEFORMATTimeToCS(hour_format, &date_time) ;
    mins_text = DATEFORMATTimeToCS(dwc_k_cust_min_format, &date_time) ;

    if (time_am_pm) {
	ampm_text = DATEFORMATTimeToCS(dwc_k_cust_ampm_format, &date_time) ;
    } else {
	ampm_text = XmStringCreateSimple("") ;
    }

    XtSetArg (arglist[0], XmNlabelString, hour_text);
    XtSetValues (finish_hours, arglist, 1) ;
    XmStringFree (hour_text) ;

    XtSetArg (arglist[0], XmNlabelString, mins_text);
    XtSetValues (finish_mins,  arglist, 1) ;
    XmStringFree (mins_text) ;

    XtSetArg (arglist[0], XmNlabelString, ampm_text);
    XtSetValues (finish_ampm,  arglist, 1) ;
    XmStringFree (ampm_text) ;

}

static void increment_time_widget
#if defined(_DWC_PROTO_)
	(
	Widget		w,
	CalendarDisplay	cd)
#else	/* no prototypes */
	(w, cd)
	Widget		w;
	CalendarDisplay	cd;
#endif	/* prototype */
    {
    unsigned short int	time ;
    unsigned char	mins ;
    Boolean		start ;
    Boolean		lunch ;
    Boolean		sethours ;
    Boolean		changeampm ;
    XmString		time_text ;
    XmString		ampm_text ;
    Arg			arglist [1] ;
    dtb			date_time ;

        
    lunch = FALSE ;
    
    if ((w == cd->pb_work_hours_start_minutes) ||
	(w == cd->pb_work_hours_start_hours)) {
	time     = cd->work_hours_start_time ;
	start    = TRUE ;
	sethours = (w == cd->pb_work_hours_start_hours) ;
    } else {
	if ((w == cd->pb_work_hours_finish_minutes) ||
	    (w == cd->pb_work_hours_finish_hours)) {
	    time     = cd->work_hours_finish_time ;
	    start    = FALSE ;
	    sethours = (w == cd->pb_work_hours_finish_hours) ;
	} else {
	    lunch = TRUE ;
	    if ((w == cd->lunch_start_minute_pb) ||
	        (w == cd->lunch_start_hour_pb)) {
		time     = cd->lunch_hours_start_time ;
		start    = TRUE ;
		sethours = (w == cd->lunch_start_hour_pb) ;
	    } else {
		time     = cd->lunch_hours_finish_time ;
		start    = FALSE ;
		sethours = (w == cd->lunch_finish_hour_pb) ;
	    }
	}
    }
    
    changeampm = FALSE ;
    if (sethours) {

	time = time + 60 ;
	if (time >= (24 * 60)) {
	    time = time - (24 * 60) ;
	    changeampm = TRUE ;
	} else {
	    if ((time / 60) == 12) {
		changeampm = TRUE ;
	    }
	}

	date_time.hour = time / 60 ;
	if (cd->profile.time_am_pm) {
	    time_text = DATEFORMATTimeToCS(dwc_k_cust_hour_ampm_format, &date_time) ;
	    if (changeampm) {
		ampm_text = DATEFORMATTimeToCS(dwc_k_cust_ampm_format, &date_time) ;
	    }
	} else {
	    changeampm = FALSE ;
	    time_text = DATEFORMATTimeToCS(dwc_k_cust_hour_format, &date_time) ;
	}

    } else {

	mins = (time % 60) + 5 ;
	if (mins == 60) {
	    mins = 0 ;
	}
	time = ((time / 60) * 60) + mins ;
	date_time.minute = mins ;
	time_text = DATEFORMATTimeToCS(dwc_k_cust_min_format, &date_time) ;

    }

    XtSetArg (arglist[0], XmNlabelString, time_text);
    XtSetValues (w, arglist, 1) ;
    XmStringFree (time_text) ;

    if (changeampm) {
	XtSetArg (arglist[0], XmNlabelString, ampm_text);
	if (lunch) {
	    if (start) {
		XtSetValues (cd->lunch_start_am_pm_lb,  arglist, 1) ;
	    } else {
		XtSetValues (cd->lunch_finish_am_pm_lb, arglist, 1) ;
	    }
	} else {
	    if (start) {
		XtSetValues (cd->lb_work_hours_start_ampm,  arglist, 1) ;
	    } else {
		XtSetValues (cd->lb_work_hours_finish_ampm, arglist, 1) ;
	    }
	}
	XmStringFree (ampm_text) ;
    }


    if (lunch) {
	if (start) {
	    cd->lunch_hours_start_time  = time ;
	} else {
	    cd->lunch_hours_finish_time = time ;
	}
    } else {
	if (start) {
	    cd->work_hours_start_time   = time ;
	} else {
	    cd->work_hours_finish_time  = time ;
	}
    }
    
}

void
CUSTTellDBWorkDays
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	unsigned char	work_days)
#else	/* no prototypes */
	(cd, work_days)
	CalendarDisplay	cd;
	unsigned char	work_days;
#endif	/* prototype */
    {
    Cardinal		i ;
    unsigned char	ph = 0 ;
    unsigned char	mc = work_days ;

    for (i = 0 ;  i < 7 ;  i++) {
	ph = ph << 1 ;
	ph = ph | (mc & 1) ;
	mc = mc >> 1 ;
    }

    (void) DWC$DB_Set_workweek (cd->cab, ph) ;

}

void CUSTDO_DAYVIEW_DISPLAY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget	w;
	caddr_t	tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
    {
    XtWidgetGeometry	request ;
    XtGeometryResult	result ;
    CalendarDisplay	cd ;
    int			status ;
    MrmType		class;
    Widget		*children;
    Cardinal		num_children;
        			    
    status = MISCFindCalendarDisplay (&cd, w) ;

    if (cd->db_dayview_up) {
	request.request_mode = CWStackMode ;
	request.stack_mode   = Above ;
    	result = XtMakeGeometryRequest (cd->dayview_db, &request, NULL) ;
	return ;
    }

    cd->original.day_show_notes         = cd->profile.day_show_notes ;
    cd->original.day_show_months        = cd->profile.day_show_months ;
    cd->original.day_show_full_times    = cd->profile.day_show_full_times ;
    cd->original.times_on_the_line      = cd->profile.times_on_the_line ;
    cd->original.entries_significant    = cd->profile.entries_significant ;
    cd->original.notes_significant      = cd->profile.notes_significant ;
    cd->original.day_v_spacing          = cd->profile.day_v_spacing ;
    cd->original.day_h_spacing          = cd->profile.day_h_spacing ;
    cd->original.day_timebar_width      = cd->profile.day_timebar_width ;
    cd->original.timeslot_stacking      = cd->profile.timeslot_stacking ;
    cd->original.day_font_size          = cd->profile.day_font_size ;
    cd->original.day_icon_size          = cd->profile.day_icon_size ;
    cd->original.work_minutes_start     = cd->profile.work_minutes_start ;
    cd->original.work_minutes_end       = cd->profile.work_minutes_end ;
    cd->original.lunch_minutes_start    = cd->profile.lunch_minutes_start ;
    cd->original.lunch_minutes_end      = cd->profile.lunch_minutes_end ;
    cd->original.timeslot_size          = cd->profile.timeslot_size ;
    cd->original.fine_increment         = cd->profile.fine_increment ;
    cd->original.default_entry_icon     = cd->profile.default_entry_icon ;
    cd->original.default_notes_icon     = cd->profile.default_notes_icon ;
    cd->original.auto_click_on_clock	= cd->profile.auto_click_on_clock ;

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor) ;

    if (! cd->db_dayview_created)
    {
	/*
	** Make sure all pixmaps are loaded.
	*/
	MISCLoadPixmapArrays (cd->mainwid, 0);
	MISCLoadPixmapArrays (cd->mainwid, 1);
	MISCLoadPixmapArrays (cd->mainwid, 2);

	register_names () ;
        /*	  
        **  This will will stuff cd->dayview_db with a DialogShell whose
	**  unmanaged child is a form. Just realize it, don't manage it.
        */	  
        MISCFetchWidget(w, "customize_dayview_box", NULL, FALSE);

	CUSTSetDayviewFromProfile (cd, &cd->original) ;
	cd->db_dayview_created = TRUE ;

	/*
	** Adjust the sizes of the bottom toggles.
	*/
	children = (Widget *)XtMalloc (sizeof(Widget) * 4);
	num_children = 4;
	children[0] = cd->day_entry_significant;
	children[1] = cd->day_daynote_significant;
	children[2] = cd->day_daynote_default_icon_tb;
	children[3] = cd->day_slot_default_icon_tb;
	MISCHeightButtonsEqually (children, num_children);

	children[0] = cd->pb_day_dummy1;
	children[1] = cd->pb_day_dummy2;
	MISCSizeButtonsEqually (children, 2);

	children[0] = cd->timebar_width_scale;
	children[1] = cd->horizontal_spacing_scale;
	MISCWidthButtonsEqually (children, 2);

	XtFree ((char *)children);

	/*
	** Clean up the layout between when we create the windows and get it
	** on the screen.
	*/
	children = DXmChildren ((CompositeWidget)cd->pb_day_controls);
	num_children = DXmNumChildren ((CompositeWidget)cd->pb_day_controls);
	MISCWidthButtonsEqually (children, num_children);

	XtManageChild (cd->dayview_db);

	MISCSpaceButtonsEqually (children, num_children);

	/*
	** Make the CLOSE item in the window manager menu call the CANCEL
	** callback.
	*/
	MISCAddProtocols
	(
	    XtParent(cd->dayview_db),
	    (XtCallbackProc) DO_DAYVIEW_CANCEL,
	    NULL
	);

	XtVaSetValues
	(
	    cd->stacking_om,
	    XmNleftAttachment, XmATTACH_NONE,
	    XmNrightAttachment, XmATTACH_FORM,
	    NULL
	);

	XtVaSetValues
	(
	    cd->fine_incr_om,
	    XmNleftAttachment, XmATTACH_NONE,
	    XmNrightAttachment, XmATTACH_FORM,
	    NULL
	);

	XtVaSetValues
	(
	    cd->om_tss,
	    XmNleftAttachment, XmATTACH_NONE,
	    XmNrightAttachment, XmATTACH_FORM,
	    NULL
	);

    }
    else
    {
        /*	  
        **  We have to do this if we're not going through
	**  CUSTSetDayviewFromProfile since the radio buttons aren't working
	**  like they're supposed to and don't come up right when we pop the
	**  form dialog up.
        */
	XmToggleButtonSetState(cd->day_slot_default_icon_tb, True, True);
	XmToggleButtonSetState(cd->day_daynote_default_icon_tb, True, True);

	XtManageChild (cd->dayview_db);
    }    

    /*
    ** Tell the dialog to take focus if no explict focusing has been done.
    */
    MISCFocusOnMap (XtParent(cd->dayview_db), NULL);

    ICONSWaitCursorRemove (cd->mainwid) ;
    cd->db_dayview_up = TRUE ;
}

void CUSTSetDayviewFromProfile
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*profile)
#else	/* no prototypes */
	(cd, profile)
	CalendarDisplay		cd;
	ProfileStructure	*profile;
#endif	/* prototype */
    {
    Widget		temp_w ;
    int			ac ;
    Arg			arglist [10] ;

    XmToggleButtonSetState (cd->day_show_months,
			     profile->day_show_months,        FALSE) ;

    XmToggleButtonSetState (cd->show_day_notes_tb,
			     profile->day_show_notes,         FALSE) ;

    XmToggleButtonSetState (cd->show_full_times_tb,
			     profile->day_show_full_times,    FALSE) ;

    XmToggleButtonSetState (cd->times_on_line_tb,
			     profile->times_on_the_line,      FALSE) ;


    XmToggleButtonSetState (cd->day_entry_significant,
			     profile->entries_significant,    FALSE) ;

    XmToggleButtonSetState (cd->day_daynote_significant,
			     profile->notes_significant,      FALSE) ;

    XmToggleButtonSetState (cd->auto_click_clock_tb,
			     profile->auto_click_on_clock,    FALSE) ;

    XmScaleSetValue (cd->vertical_spacing_scale,   profile->day_v_spacing) ;
    XmScaleSetValue (cd->horizontal_spacing_scale, profile->day_h_spacing) ;
    XmScaleSetValue (cd->timebar_width_scale,      profile->day_timebar_width) ;


    if (profile->timeslot_stacking) {
	XtSetArg(arglist[0], XmNmenuHistory, cd->stack_top_down_pb);
    } else {
	XtSetArg(arglist[0], XmNmenuHistory, cd->stack_r_l_pb);
    }
    XtSetValues (cd->stacking_om, arglist, 1);


    /*	  
    **  Set the day_font_size toggle button. For some strange reason, it isn't
    **	enough to set one since the others don't get unset even though we're
    **	inside a radio box, so explictly set all of them. HACK 5/25/1990 PGLF
    */	  
    switch (profile->day_font_size) {
        case 0  :
	    XmToggleButtonSetState (cd->font_small_tb, TRUE, TRUE) ;
	    XmToggleButtonSetState (cd->font_medium_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->font_large_tb, FALSE, TRUE) ;
	    break;
        case 1  :
	    XmToggleButtonSetState (cd->font_small_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->font_medium_tb, TRUE, TRUE) ;
	    XmToggleButtonSetState (cd->font_large_tb, FALSE, TRUE) ;
	    break;
        case 2  :
	    XmToggleButtonSetState (cd->font_small_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->font_medium_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->font_large_tb, TRUE, TRUE) ;
	    break;
        default :
	    XmToggleButtonSetState (cd->font_small_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->font_medium_tb, TRUE, TRUE) ;
	    XmToggleButtonSetState (cd->font_large_tb, FALSE, TRUE) ;
	    break;
    }


    /*	  
    **  Set the day_icon_size toggle button. For some strange reason, it isn't
    **	enough to set one since the others don't get unset even though we're
    **	inside a radio box, so explictly set all of them. HACK 5/25/1990 PGLF
    */	  
    switch (profile->day_icon_size) {
        case 0  :
	    XmToggleButtonSetState (cd->icon_small_tb, TRUE, TRUE) ;
	    XmToggleButtonSetState (cd->icon_medium_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->icon_large_tb, FALSE, TRUE) ;
	    break;
        case 1  :
	    XmToggleButtonSetState (cd->icon_small_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->icon_medium_tb, TRUE, TRUE) ;
	    XmToggleButtonSetState (cd->icon_large_tb, FALSE, TRUE) ;
	    break;
        case 2  :
	    XmToggleButtonSetState (cd->icon_small_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->icon_medium_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->icon_large_tb, TRUE, TRUE) ;
	    break;
        default :
	    XmToggleButtonSetState (cd->icon_small_tb, FALSE, TRUE) ;
	    XmToggleButtonSetState (cd->icon_medium_tb, TRUE, TRUE) ;
	    XmToggleButtonSetState (cd->icon_large_tb, FALSE, TRUE) ;
	    break;
    }


    set_work_hours_buttons 
      (cd->pb_work_hours_start_hours,  cd->pb_work_hours_start_minutes,
       cd->lb_work_hours_start_ampm,
       cd->pb_work_hours_finish_hours, cd->pb_work_hours_finish_minutes,
       cd->lb_work_hours_finish_ampm,
       profile->work_minutes_start,    profile->work_minutes_end,
       cd->profile.time_am_pm) ;

    cd->work_hours_start_time  = profile->work_minutes_start ;
    cd->work_hours_finish_time = profile->work_minutes_end ;

    set_work_hours_buttons 
      (cd->lunch_start_hour_pb,      cd->lunch_start_minute_pb,
       cd->lunch_start_am_pm_lb,
       cd->lunch_finish_hour_pb,     cd->lunch_finish_minute_pb,
       cd->lunch_finish_am_pm_lb,
       profile->lunch_minutes_start, profile->lunch_minutes_end,
       cd->profile.time_am_pm) ;

    cd->lunch_hours_start_time  = profile->lunch_minutes_start ;
    cd->lunch_hours_finish_time = profile->lunch_minutes_end ;


    switch (profile->fine_increment) {
	case 60 :  temp_w = cd->custom_buttons.fine_incr_60_pb ;  break ;
	case 30 :  temp_w = cd->custom_buttons.fine_incr_30_pb ;  break ;
	case 20 :  temp_w = cd->custom_buttons.fine_incr_20_pb ;  break ;
	case 15 :  temp_w = cd->custom_buttons.fine_incr_15_pb ;  break ;
	case 12 :  temp_w = cd->custom_buttons.fine_incr_12_pb ;  break ;
	case 10 :  temp_w = cd->custom_buttons.fine_incr_10_pb ;  break ;
	case  6 :  temp_w = cd->custom_buttons.fine_incr_6_pb ;   break ;
	case  5 :  temp_w = cd->custom_buttons.fine_incr_5_pb ;   break ;
	case  4 :  temp_w = cd->custom_buttons.fine_incr_4_pb ;   break ;
	case  3 :  temp_w = cd->custom_buttons.fine_incr_3_pb ;   break ;
	case  2 :  temp_w = cd->custom_buttons.fine_incr_2_pb ;   break ;
	case  1 :  temp_w = cd->custom_buttons.fine_incr_1_pb ;   break ;
	default :  temp_w = cd->custom_buttons.fine_incr_5_pb ;   break ;
    }

    XtSetArg(arglist[0], XmNmenuHistory, temp_w);
    XtSetValues (cd->fine_incr_om, arglist, 1);

    
    switch (profile->timeslot_size) {
	case 60 :  temp_w = cd->custom_buttons.tb_tss_60 ;  break ;
	case 30 :  temp_w = cd->custom_buttons.tb_tss_30 ;  break ;
	case 20 :  temp_w = cd->custom_buttons.tb_tss_20 ;  break ;
	case 15 :  temp_w = cd->custom_buttons.tb_tss_15 ;  break ;
	case 12 :  temp_w = cd->custom_buttons.tb_tss_12 ;  break ;
	case 10 :  temp_w = cd->custom_buttons.tb_tss_10 ;  break ;
	case  6 :  temp_w = cd->custom_buttons.tb_tss_06 ;  break ;
	case  5 :  temp_w = cd->custom_buttons.tb_tss_05 ;  break ;
	case  4 :  temp_w = cd->custom_buttons.tb_tss_04 ;  break ;
	case  3 :  temp_w = cd->custom_buttons.tb_tss_03 ;  break ;
	case  2 :  temp_w = cd->custom_buttons.tb_tss_02 ;  break ;
	case  1 :  temp_w = cd->custom_buttons.tb_tss_01 ;  break ;
	default :  temp_w = cd->custom_buttons.tb_tss_30 ;  break ;
    }

    XtSetArg(arglist[0], XmNmenuHistory, temp_w);
    XtSetValues (cd->om_tss, arglist, 1);
    
    FINE_INCR_CONSISTENCY (temp_w) ;

    cd->current_entry_icon = profile->default_entry_icon ;
    cd->current_notes_icon = profile->default_notes_icon ;

    XmToggleButtonSetState(cd->day_slot_default_icon_tb, True, True);
    XmToggleButtonSetState(cd->day_daynote_default_icon_tb, True, True);


}

static void ICON_TOGGLE_VALUE_CHANGED
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	DwcIbwCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	DwcIbwCallbackStruct	*cbs;
#endif	/* prototype */
{
    unsigned char	icon;
    Widget		icon_toggle;
    Widget		switch_toggle;
    Pixmap		pixmap;
    CalendarDisplay	cd;
    int			status;
    Display		*disp;
    Window		wind;


    if (cbs->reason == (int)XmCR_VALUE_CHANGED)
    {
	status = MISCFindCalendarDisplay (&cd, w) ;
	
	icon = cbs->selected_icon ;
	
	if (cd->current_is_entry)
	{
	    cd->current_entry_icon = icon ;
	    switch_toggle = cd->day_slot_default_icon_tb ;
	}
	else
	{
	    cd->current_notes_icon = icon ;
	    switch_toggle = cd->day_daynote_default_icon_tb ;
	}

	/*
	** Get the pixmap if necessary.
	*/
	if (defdep_medium_pixmaps[icon] == 0)
	{
	    defdep_medium_pixmaps[icon] = MISCCreatePixmapFromBitmap
		(switch_toggle, medium_pixmaps[icon], 17, 17);
	}

	/*
	** Set the pixmap on the toggle.
	*/
	XtVaSetValues
	    (switch_toggle, XmNlabelPixmap, defdep_medium_pixmaps[icon], NULL);

	disp = XtDisplay(switch_toggle);
	wind = XtWindow(switch_toggle);
	XClearArea (disp, wind, 0, 0, 0, 0, True);

    }
}

static void ICON_SWITCH_VALUE_CHANGED
#if defined(_DWC_PROTO_)
	(
	Widget				w,
	caddr_t				tag,
	XmToggleButtonCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget				w;
	caddr_t				tag;
	XmToggleButtonCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay	cd ;
    int			status ;
    Widget		switch_toggle;
    Pixmap		pixmap;
    Display		*disp;
    Window		wind;

    unsigned char	icon;

    if (cbs->reason == (int)XmCR_VALUE_CHANGED)
    {
	status = MISCFindCalendarDisplay (&cd, w) ;

        cd->current_is_entry = !cbs->set ;

	/*
	** Get the pixmap if necessary.
	*/
	icon = cd->current_notes_icon;
	switch_toggle = cd->day_daynote_default_icon_tb;
	if (defdep_medium_pixmaps[icon] == 0)
	{
	    defdep_medium_pixmaps[icon] = MISCCreatePixmapFromBitmap
		(switch_toggle, medium_pixmaps[icon], 17, 17);
	}

	/*
	** Set the pixmap on the toggle.
	*/
	XtVaSetValues
	    (switch_toggle, XmNlabelPixmap, defdep_medium_pixmaps[icon], NULL);

	/*
	** Force a redisplay.
	*/
	disp = XtDisplay(switch_toggle);
	wind = XtWindow(switch_toggle);
	XClearArea (disp, wind, 0, 0, 0, 0, True);

	/*
	** Get the pixmap if necessary.
	*/
	icon = cd->current_entry_icon;
	switch_toggle = cd->day_slot_default_icon_tb;
	if (defdep_medium_pixmaps[icon] == 0)
	{
	    defdep_medium_pixmaps[icon] = MISCCreatePixmapFromBitmap
		(switch_toggle, medium_pixmaps[icon], 17, 17);
	}

	/*
	** Set the pixmap on the toggle.
	*/
	XtVaSetValues
	    (switch_toggle, XmNlabelPixmap, defdep_medium_pixmaps[icon], NULL);

	/*
	** Force a redisplay.
	*/
	disp = XtDisplay(switch_toggle);
	wind = XtWindow(switch_toggle);
	XClearArea (disp, wind, 0, 0, 0, 0, True);

	/*
	** Now change the icon box widget.
	*/
	if (cbs->set)
	{
	    icon = cd->current_notes_icon;
	}
	else
	{
	    icon = cd->current_entry_icon;
	}
	DwcIbwSetSelectedIcon (cd->day_default_icon_box_widget, icon);

    }
}

typedef struct _TimeslotSizeWidgets {
    unsigned char   time ;
    Cardinal	    slot_size ;
    Cardinal	    fine_incr ;
} TimeslotSizeWidgets ;

#define	NumTimeslotSizeWidgets	12

static TimeslotSizeWidgets timeslot_size_widgets [NumTimeslotSizeWidgets] = {
    {60, XtOffset (CalendarDisplay, custom_buttons.tb_tss_60), XtOffset (CalendarDisplay, custom_buttons.fine_incr_60_pb)},
    {30, XtOffset (CalendarDisplay, custom_buttons.tb_tss_30), XtOffset (CalendarDisplay, custom_buttons.fine_incr_30_pb)},
    {20, XtOffset (CalendarDisplay, custom_buttons.tb_tss_20), XtOffset (CalendarDisplay, custom_buttons.fine_incr_20_pb)},
    {15, XtOffset (CalendarDisplay, custom_buttons.tb_tss_15), XtOffset (CalendarDisplay, custom_buttons.fine_incr_15_pb)},
    {12, XtOffset (CalendarDisplay, custom_buttons.tb_tss_12), XtOffset (CalendarDisplay, custom_buttons.fine_incr_12_pb)},
    {10, XtOffset (CalendarDisplay, custom_buttons.tb_tss_10), XtOffset (CalendarDisplay, custom_buttons.fine_incr_10_pb)},
    { 6, XtOffset (CalendarDisplay, custom_buttons.tb_tss_06), XtOffset (CalendarDisplay, custom_buttons.fine_incr_6_pb)},
    { 5, XtOffset (CalendarDisplay, custom_buttons.tb_tss_05), XtOffset (CalendarDisplay, custom_buttons.fine_incr_5_pb)},
    { 4, XtOffset (CalendarDisplay, custom_buttons.tb_tss_04), XtOffset (CalendarDisplay, custom_buttons.fine_incr_4_pb)},
    { 3, XtOffset (CalendarDisplay, custom_buttons.tb_tss_03), XtOffset (CalendarDisplay, custom_buttons.fine_incr_3_pb)},
    { 2, XtOffset (CalendarDisplay, custom_buttons.tb_tss_02), XtOffset (CalendarDisplay, custom_buttons.fine_incr_2_pb)},
    { 1, XtOffset (CalendarDisplay, custom_buttons.tb_tss_01), XtOffset (CalendarDisplay, custom_buttons.fine_incr_1_pb)}
};

static void FINE_INCR_CONSISTENCY
#if defined(_DWC_PROTO_)
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
    {
    Cardinal	    ac ;
    Arg		    arglist [1] ;
    Widget	    fiw ;
    Cardinal	    tss ;
    Cardinal	    i ;
    Cardinal	    index ;
    CalendarDisplay cd ;
    char	    *cd_base ;
    Widget	    *cdw_ptr ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;
    cd_base = (char *) cd ;

    XtSetArg(arglist[0], XmNmenuHistory, &fiw);
    XtGetValues (cd->fine_incr_om, arglist, 1);

    for (i = 0 ;  i < NumTimeslotSizeWidgets ;  i++) {
	cdw_ptr = (Widget *) (cd_base + timeslot_size_widgets [i].slot_size) ;

	if (w == *cdw_ptr) {
	    index = i ;
	    break ;
	}

	cdw_ptr = (Widget *) (cd_base + timeslot_size_widgets [i].fine_incr) ;
	XtSetSensitive (*cdw_ptr, FALSE) ;

	if (*cdw_ptr == fiw) {
	    fiw = NULL ;
	}
    }

    tss = timeslot_size_widgets [index].time ;

    for (i = index ;  i < NumTimeslotSizeWidgets ;  i++) {
	cdw_ptr = (Widget *) (cd_base + timeslot_size_widgets [i].fine_incr) ;

	if ((tss % timeslot_size_widgets [i].time) == 0) {

	    XtSetSensitive (*cdw_ptr, TRUE) ;
	    if (fiw == NULL) {
		fiw = *cdw_ptr ;
		XtSetArg(arglist[0], XmNmenuHistory, fiw);
		XtSetValues (cd->fine_incr_om, arglist, 1);
	    }

	} else {

	    XtSetSensitive (*cdw_ptr, FALSE) ;
	    if (*cdw_ptr == fiw) {
		fiw = NULL ;
	    }
	}
    }

}

void
DO_DAYVIEW_OK
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    if (DO_DAYVIEW_APPLY (w, tag)) {
	cd->db_dayview_up = FALSE ;
	XtUnmanageChild(cd->dayview_db);
    }

}

Boolean
DO_DAYVIEW_APPLY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay	cd ;
    int			status ;
    Boolean		reconfigure_day = FALSE ;
    ProfileStructure	new;    

    status = MISCFindCalendarDisplay (&cd, w) ;

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor) ;

    new = cd->profile;
    fetch_dayview_new ( cd, &new );

    if (CUSTApplyProfile ( cd, &new )) {
	ICONSWaitCursorRemove (cd->mainwid) ;
	return (TRUE) ;
    } else {
	return (FALSE) ;
    }
    
}

void
fetch_dayview_new
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*new)
#else	/* no prototypes */
	(cd, new)
	CalendarDisplay		cd;
	ProfileStructure	*new;
#endif	/* prototype */
{
    char	    *cd_base;
    Arg		    arglget [ 1 ];
    Widget	    option ;
    Cardinal	    ac ;
    Widget	    *cdw_ptr;
    int		    i;
    int		    temp;

    new->day_show_notes = XmToggleButtonGetState (cd->show_day_notes_tb);
    new->day_show_months     = XmToggleButtonGetState (cd->day_show_months) ;
    new->entries_significant = XmToggleButtonGetState (cd->day_entry_significant) ;
    new->notes_significant   = XmToggleButtonGetState (cd->day_daynote_significant);

    new->day_show_full_times = XmToggleButtonGetState (cd->show_full_times_tb) ;
    new->times_on_the_line   = XmToggleButtonGetState (cd->times_on_line_tb) ;

    XmScaleGetValue (cd->vertical_spacing_scale,   &temp) ;
    new->day_v_spacing = temp;
    XmScaleGetValue (cd->horizontal_spacing_scale, &temp);
    new->day_h_spacing = temp;
    XmScaleGetValue (cd->timebar_width_scale,      &temp);
    new->day_timebar_width = temp;


    XtSetArg(arglget[0], XmNmenuHistory, &option);
    XtGetValues (cd->stacking_om, arglget, 1);
    new->timeslot_stacking = (option == cd->stack_top_down_pb) ;


    if (XmToggleButtonGetState (cd->font_small_tb)) {
	new->day_font_size = 0 ;
    } else {
        if (XmToggleButtonGetState (cd->font_medium_tb)) {
	    new->day_font_size = 1 ;
	} else {
	    new->day_font_size = 2 ;
	}
    }


    if (XmToggleButtonGetState (cd->icon_small_tb)) {
	new->day_icon_size = 0 ;
    } else {
        if (XmToggleButtonGetState (cd->icon_medium_tb)) {
	    new->day_icon_size = 1 ;
	} else {
	    new->day_icon_size = 2 ;
	}
    }

    new->work_minutes_start  = cd->work_hours_start_time ;
    new->work_minutes_end    = cd->work_hours_finish_time ;
    new->lunch_minutes_start = cd->lunch_hours_start_time ;
    new->lunch_minutes_end   = cd->lunch_hours_finish_time ;

    cd_base = (char *) cd ;

    XtSetArg(arglget[0], XmNmenuHistory, &option);
    XtGetValues(cd->fine_incr_om, arglget, 1);

    for (i = 0 ;  i < NumTimeslotSizeWidgets ;  i++) {
	cdw_ptr = (Widget *) (cd_base + timeslot_size_widgets [i].fine_incr);
	if (option == *cdw_ptr)
	{
	    new->fine_increment = timeslot_size_widgets [i].time ;
	    break ;
	}
    }

    XtSetArg(arglget[0], XmNmenuHistory, &option);
    XtGetValues (cd->om_tss, arglget, 1);

    for (i = 0 ;  i < NumTimeslotSizeWidgets ;  i++) {
	cdw_ptr = (Widget *) (cd_base + timeslot_size_widgets [i].slot_size) ;
	if (option == *cdw_ptr) {
	    new->timeslot_size = timeslot_size_widgets [i].time ;
	    break ;
	}
    }

    new->auto_click_on_clock =
			    XmToggleButtonGetState (cd->auto_click_clock_tb) ;

/* ?????  Check w/ Marios */

    new->default_entry_icon = cd->current_entry_icon;
    new->default_notes_icon = cd->current_notes_icon;

    return ;
}

void
DO_DAYVIEW_RESET
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;


    status = MISCFindCalendarDisplay (&cd, w) ;

    CUSTSetDayviewFromProfile(cd, &cd->original) ;

}

void
DO_DAYVIEW_CANCEL
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    DO_DAYVIEW_RESET (w, tag) ;
    DO_DAYVIEW_OK    (w, tag) ;

}

void CUSTDO_ICONBOX_DISPLAY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget	w;
	caddr_t	tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    XtWidgetGeometry	request ;
    XtGeometryResult	result ;
    CalendarDisplay	cd ;
    int			status ;
    MrmType		class ;
    Widget		*children;
    Cardinal		num_children;

    status = MISCFindCalendarDisplay (&cd, w) ;

    if (cd->db_iconbox_up) {
	request.request_mode = CWStackMode ;
	request.stack_mode   = Above ;
    	result = XtMakeGeometryRequest (cd->iconbox_db, &request, NULL) ;
	return ;
    }


    memcpy (cd->original.icon_text, cd->profile.icon_text, 32) ;

    cd->original.icon_show_text     = cd->profile.icon_show_text ;
/* probably obsolete with Mwm window manager icons.
    cd->original.icon_nl_after_text = cd->profile.icon_nl_after_text ;
*/
    cd->original.icon_show_day      = cd->profile.icon_show_day ;
    cd->original.icon_full_day      = cd->profile.icon_full_day ;
/* probably obsolete with Mwm window manager icons.
    cd->original.icon_nl_after_day  = cd->profile.icon_nl_after_day ;
*/
    cd->original.icon_show_time     = cd->profile.icon_show_time ;

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor) ;

    if (! cd->db_iconbox_created)
    {
	/*
	** Make sure all pixmaps are loaded.
	*/
	MISCLoadPixmapArrays (cd->mainwid, 0);
	MISCLoadPixmapArrays (cd->mainwid, 1);
	MISCLoadPixmapArrays (cd->mainwid, 2);

	register_names () ;
        /*	  
	**  Just realize it, don't manage it.
	*/	  
        MISCFetchWidget (w, "customize_iconbox_box", NULL, FALSE);

	CUSTSetIconboxFromProfile (cd, &cd->original) ;
	cd->db_iconbox_created = TRUE ;

	/*
	** Clean up the layout between when we create the windows and get it
	** on the screen.
	*/
	children = DXmChildren ((CompositeWidget)cd->pb_icon_controls);
	num_children = DXmNumChildren ((CompositeWidget)cd->pb_icon_controls);
	MISCWidthButtonsEqually (children, num_children);

	XtManageChild (cd->iconbox_db);

	MISCSpaceButtonsEqually (children, num_children);

	/*
	** Make the CLOSE item in the window manager menu call the CANCEL
	** callback.
	*/
	MISCAddProtocols
	(
	    XtParent(cd->iconbox_db),
	    (XtCallbackProc) DO_ICONBOX_CANCEL,
	    NULL
	);

    }
    else
    {
	Cardinal	len;
	Time		time;

	time = XtLastTimestampProcessed(XtDisplay(w));

	XtManageChild (cd->iconbox_db);

	len = DXmCSTextGetLastPosition
	    ((DXmCSTextWidget)cd->iconbox_text_stext);
	if (len != 0)
	{
	    DXmCSTextSetSelection (cd->iconbox_text_stext, 0, len, time);
	}
    }

    /*
    ** Tell the dialog to take focus if no explict focusing has been done.
    */
    MISCFocusOnMap (XtParent(cd->iconbox_db), NULL);

    ICONSWaitCursorRemove (cd->mainwid);
    cd->db_iconbox_up = TRUE;

}

static void ICONBOX_SHOW_TEXT_CHANGE
#if defined(_DWC_PROTO_)
	(
	Widget				w,
	caddr_t				tag,
	XmToggleButtonCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget				w;
	caddr_t				tag;
	XmToggleButtonCallbackStruct	*cbs;
#endif	/* prototype */
    {
    char		*text ;
    Cardinal		len ;
    Time		time;
    CalendarDisplay	cd ;
    int			status ;
    Arg			arglist [10] ;
    Cardinal		ac ;
    XmString		xm_text;
    long		byte_count, cvt_status;

    if (cbs->reason == (int)XmCR_VALUE_CHANGED)
    {
	status = MISCFindCalendarDisplay (&cd, w) ;

	ac = 0 ;
	XtSetArg(arglist[ac], XmNeditable,              cbs->set); ac++;
	XtSetArg(arglist[ac], XmNcursorPositionVisible, cbs->set); ac++;
	XtSetArg(arglist[ac], XmNtopPosition,           0); ac++;
	XtSetArg(arglist[ac], XmNcursorPosition,     0); ac++;
	XtSetValues (cd->iconbox_text_stext, arglist, ac) ;

	time = XtLastTimestampProcessed(XtDisplay(w));

	if (cbs->set)
	{
	    len = DXmCSTextGetLastPosition
		((DXmCSTextWidget) cd->iconbox_text_stext);
	    if (len != 0)
	    {
		DXmCSTextSetSelection (cd->iconbox_text_stext, 0, len, time);
	    }
	}
	else
	{
	    DXmCSTextClearSelection (cd->iconbox_text_stext, time);
	}

    }

}

static void ICONBOX_SHOW_DAY_CHANGE
#if defined(_DWC_PROTO_)
	(
	Widget				w,
	caddr_t				tag,
	XmToggleButtonCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget				w;
	caddr_t				tag;
	XmToggleButtonCallbackStruct	*cbs;
#endif	/* prototype */
    {
    CalendarDisplay	cd ;
    int			status ;


    if (cbs->reason == (int)XmCR_VALUE_CHANGED)
    {
	status = MISCFindCalendarDisplay (&cd, w) ;

	XtSetSensitive (cd->iconbox_full_day_toggle,     cbs->set) ;
/* probably obsolete with Mwm window manager icons.
	XtSetSensitive (cd->iconbox_nl_after_day_toggle, cbs->set) ;
*/
    }
    
}

void CUSTSetIconboxFromProfile
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*profile)
#else	/* no prototypes */
	(cd, profile)
	CalendarDisplay		cd;
	ProfileStructure	*profile;
#endif	/* prototype */
{
    XmString		xm_text;
    long		byte_count, cvt_status;

    xm_text = DXmCvtFCtoCS (profile->icon_text, &byte_count, &cvt_status);
    DXmCSTextSetString ((DXmCSTextWidget)cd->iconbox_text_stext, xm_text);
    XmStringFree (xm_text);

    XmToggleButtonSetState
      (cd->iconbox_full_day_toggle, profile->icon_full_day, FALSE) ;

    XmToggleButtonSetState
      (cd->iconbox_show_time_toggle, profile->icon_show_time, FALSE) ;

    XmToggleButtonSetState
      (cd->iconbox_show_text_tb, ! profile->icon_show_text, TRUE) ;
    XmToggleButtonSetState
      (cd->iconbox_show_text_tb, profile->icon_show_text, TRUE) ;

    XmToggleButtonSetState
      (cd->iconbox_show_day_toggle, ! profile->icon_show_day, TRUE) ;
    XmToggleButtonSetState
      (cd->iconbox_show_day_toggle, profile->icon_show_day, TRUE) ;
    

}

void
DO_ICONBOX_OK
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    if (DO_ICONBOX_APPLY (w, tag)) {
	cd->db_iconbox_up = FALSE ;
	XtUnmanageChild (cd->iconbox_db) ;
    }
    
}

Boolean
DO_ICONBOX_APPLY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;
    ProfileStructure new;

    status = MISCFindCalendarDisplay (&cd, w) ;

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor) ;

    new = cd->profile;
    fetch_iconbox_new( cd, &new );
    if (CUSTApplyProfile ( cd, &new )) {
	ICONSWaitCursorRemove (cd->mainwid) ;
	return (TRUE) ;
    } else {
	return (FALSE) ;
    }

}

void fetch_iconbox_new
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*new)
#else	/* no prototypes */
	(cd, new)
	CalendarDisplay		cd;
	ProfileStructure	*new;
#endif	/* prototype */
    {
    Arg		    arglist [10] ;
    Cardinal	    ac ;
    int		    len ;
    char	    *text ;
    XmString		xm_text;
    long		byte_count, cvt_status;
    
    new->icon_show_text = XmToggleButtonGetState (cd->iconbox_show_text_tb);

    xm_text = DXmCSTextGetString (cd->iconbox_text_stext);
    if (xm_text)
    {
	text = DXmCvtCStoFC(xm_text, &byte_count, &cvt_status);
	XmStringFree(xm_text);
    }
    else
    {
	text = NULL;
    }
    if (text != NULL)
    {
	len  = MIN (strlen (text), 31);
	memcpy (new->icon_text, text, len);
	new->icon_text [len] = '\0';
	XtFree (text);
    }    

/* probably obsolete with Mwm window manager icons.
    new->icon_nl_after_text = 
      XmToggleButtonGetState (cd->iconbox_nl_after_text_toggle) ;
*/
    new->icon_show_day     =
      XmToggleButtonGetState (cd->iconbox_show_day_toggle) ;
    new->icon_full_day     =
      XmToggleButtonGetState (cd->iconbox_full_day_toggle) ;
/* probably obsolete with Mwm window manager icons.
    new->icon_nl_after_day =
      XmToggleButtonGetState (cd->iconbox_nl_after_day_toggle) ;
*/
    new->icon_show_time =
      XmToggleButtonGetState (cd->iconbox_show_time_toggle) ;

    return ;
}

void
DO_ICONBOX_RESET
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    CUSTSetIconboxFromProfile(cd, &cd->original) ;

}

void
DO_ICONBOX_CANCEL
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    DO_ICONBOX_RESET (w, tag) ;
    DO_ICONBOX_OK    (w, tag) ;
    
}

static void setup_icon
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {
    dtb		    date_time ;
    struct tm	    *local_time;

    /*	  
    **  Update our internal time
    */	  
    MISCUpdateTime();

    /*	  
    **  Get the full internal time
    */	  
    MISCGetTime(&local_time);


    date_time.day     = local_time->tm_mday ;
    date_time.hour    = local_time->tm_hour ;
    date_time.minute  = local_time->tm_min ;
    date_time.weekday = DATEFUNCDayOfWeek (date_time.day, 
				     local_time->tm_mon,
				     local_time->tm_year) ;

    ICONSSetIconTime (cd->toplevel, &date_time, cd->profile.icon_show_text,
		    cd->profile.icon_text, FALSE,
		    cd->profile.icon_show_day, cd->profile.icon_full_day,
		    FALSE,
		    cd->profile.icon_show_time, cd->profile.time_am_pm) ;

}

void CUSTDO_ALARMS_DISPLAY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget	w;
	caddr_t	tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    Cardinal		i ;
    XtWidgetGeometry	request ;
    XtGeometryResult	result ;
    CalendarDisplay	cd ;
    int			status ;
    MrmType	        class ;
    Widget		*children;
    Cardinal		num_children;

    status = MISCFindCalendarDisplay (&cd, w) ;

    if (cd->db_alarms_up) {
	request.request_mode = CWStackMode ;
	request.stack_mode   = Above ;
    	result = XtMakeGeometryRequest (cd->alarms_db, &request, NULL) ;
	return ;
    }

    cd->original.alarms_mask = cd->profile.alarms_mask ;
    for (i = 0 ;  i < 8 ;  i++) {
	cd->original.alarms [i] = cd->profile.alarms [i] ;
    }

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor) ;

    if (! cd->db_alarms_created)
    {
	/*
	** Make sure all pixmaps are loaded.
	*/
	MISCLoadPixmapArrays (cd->mainwid, 0);
	MISCLoadPixmapArrays (cd->mainwid, 1);
	MISCLoadPixmapArrays (cd->mainwid, 2);

	register_names ();
        /*
	**  Just realize it, don't manage it.
	*/	  
        MISCFetchWidget(w, "customize_alarms_box", NULL, FALSE) ;
	
        /*	  
        **  Pass in the original profile structure
        */	  
        CUSTSetAlarmsFromProfile (cd, &cd->original) ;

	cd->db_alarms_created = TRUE ;

	/*
	** Clean up the layout between when we create the windows and get it
	** on the screen.
	*/
	children = DXmChildren ((CompositeWidget)cd->pb_alarm_controls);
	num_children = DXmNumChildren ((CompositeWidget)cd->pb_alarm_controls);
	MISCWidthButtonsEqually (children, num_children);

	XtManageChild (cd->alarms_db);

	MISCSpaceButtonsEqually (children, num_children);

	/*
	** Make the CLOSE item in the window manager menu call the CANCEL
	** callback.
	*/
	MISCAddProtocols
	(
	    XtParent(cd->alarms_db),
	    (XtCallbackProc) DO_ALARMS_CANCEL,
	    NULL
	);

    }
    else
    {
	XtManageChild (cd->alarms_db);
    }

    /*
    ** Tell the dialog to take focus if no explict focusing has been done.
    */
    MISCFocusOnMap (XtParent(cd->alarms_db), NULL);

    ICONSWaitCursorRemove (cd->mainwid) ;
    cd->db_alarms_up = TRUE ;
}

void
CUSTSetAlarmsFromProfile
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*profile)
#else	/* no prototypes */
	(cd, profile)
	CalendarDisplay		cd;
	ProfileStructure	*profile;
#endif	/* prototype */
    {
    Boolean		alarm_entry ;
    Boolean		alarm_short ;
    Boolean		alarm_medium ;
    Boolean		alarm_long ;
    Boolean		alarm_days ;
    unsigned short int	scale_short ;
    unsigned short int	scale_medium ;
    unsigned short int	scale_long ;
    unsigned short int	scale_days ;


    alarm_entry  = ((profile->alarms_mask & 001) != 0) ;
    alarm_short  = ((profile->alarms_mask & 002) != 0) ;
    alarm_medium = ((profile->alarms_mask & 004) != 0) ;
    alarm_long   = ((profile->alarms_mask & 010) != 0) ;
    alarm_days   = ((profile->alarms_mask & 020) != 0) ;

    scale_short  = profile->alarms [1] ;
    scale_medium = profile->alarms [2] ;
    scale_long   = profile->alarms [3] / 6 ;
    scale_days   = profile->alarms [4] / (24 * 6) ;
    
    XmToggleButtonSetState (cd->alarm_entry_tb,   alarm_entry,  FALSE) ;
    XmToggleButtonSetState (cd->alarm_minutes_tb, alarm_short,  FALSE) ;
    XmToggleButtonSetState (cd->alarm_min_bis_tb, alarm_medium, FALSE) ;
    XmToggleButtonSetState (cd->alarm_hours_tb,   alarm_long,   FALSE) ;
    XmToggleButtonSetState (cd->alarm_days_tb,    alarm_days,   FALSE) ;

    XmScaleSetValue (cd->alarm_minutes_scale, scale_short) ;
    XmScaleSetValue (cd->alarm_min_bis_scale, scale_medium) ;
    XmScaleSetValue (cd->alarm_hours_scale,   scale_long) ;
    XmScaleSetValue (cd->alarm_days_scale,    scale_days) ;

    XtSetSensitive (cd->alarm_minutes_scale, alarm_short) ;
    XtSetSensitive (cd->alarm_min_bis_scale, alarm_medium) ;
    XtSetSensitive (cd->alarm_hours_scale,   alarm_long) ;
    XtSetSensitive (cd->alarm_days_scale,    alarm_days) ;

}

static void
TOGGLE_ALARM_SCALE
#if defined(_DWC_PROTO_)
	(
	Widget				w,
	caddr_t				tag,
	XmToggleButtonCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget				w;
	caddr_t				tag;
	XmToggleButtonCallbackStruct	*cbs;
#endif	/* prototype */
    {
    Widget		scale ;
    CalendarDisplay	cd ;
    int			status ;


    if (cbs->reason == (int)XmCR_VALUE_CHANGED) {
	status = MISCFindCalendarDisplay (&cd, w) ;

	if (w == cd->alarm_minutes_tb) {
	    scale = cd->alarm_minutes_scale ;
	} else {
	    if (w == cd->alarm_min_bis_tb) {
		scale = cd->alarm_min_bis_scale ;
	    } else {
		if (w == cd->alarm_hours_tb) {
		    scale = cd->alarm_hours_scale ;
		} else {
		    scale = cd->alarm_days_scale ;
		}
	    }
	}

	XtSetSensitive (scale, cbs->set) ;
    }

}

void
DO_ALARMS_OK
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    if (DO_ALARMS_APPLY (w, tag)) {
	cd->db_alarms_up = FALSE ;
	XtUnmanageChild(cd->alarms_db) ;
    }
    
}

void CUSTGetProfileAlarms
#if defined(_DWC_PROTO_)
	(
	ProfileStructure	*profile,
	int			*ret_alarms_number,
	unsigned short int	**ret_alarms_times)
#else	/* no prototypes */
	(profile, ret_alarms_number, ret_alarms_times)
	ProfileStructure	*profile;
	int			*ret_alarms_number;
	unsigned short int	**ret_alarms_times;
#endif	/* prototype */
    {
    Cardinal		alarms_number ;
    unsigned short int  *alarms_times ;
    Cardinal		alarm ;


    alarms_number = 0 ;
    if ((profile->alarms_mask & 001) != 0)  alarms_number++ ;
    if ((profile->alarms_mask & 002) != 0)  alarms_number++ ;
    if ((profile->alarms_mask & 004) != 0)  alarms_number++ ;
    if ((profile->alarms_mask & 010) != 0)  alarms_number++ ;
    if ((profile->alarms_mask & 020) != 0)  alarms_number++ ;

    if (alarms_number == 0) {
	alarms_times = NULL ;
    } else {
	alarms_times = (unsigned short int *)
		       XtMalloc (sizeof (unsigned short int) * alarms_number) ;
	alarm = 0 ;

	if ((profile->alarms_mask & 001) != 0) {
	    alarms_times [alarm++] = 0 ;
	}
	
	if ((profile->alarms_mask & 002) != 0) {
	    alarms_times [alarm++] = profile->alarms [1] ;
	}
	
	if ((profile->alarms_mask & 004) != 0) {
	    alarms_times [alarm++] = profile->alarms [2] ;
	}

	if ((profile->alarms_mask & 010) != 0) {
	    alarms_times [alarm++] = profile->alarms [3] ;
	}

	if ((profile->alarms_mask & 020) != 0) {
	    alarms_times [alarm++] = profile->alarms [4] ;
	}
    }

    *ret_alarms_number = alarms_number ;
    *ret_alarms_times  = alarms_times ;

}

Boolean
DO_ALARMS_APPLY
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    ProfileStructure	new;
    CalendarDisplay	cd ;
    int			status ;
    

    status = MISCFindCalendarDisplay (&cd, w) ;

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor) ;

    new = cd->profile;
    fetch_alarms_new( cd, &new );
    if (CUSTApplyProfile ( cd, &new )) {
	ICONSWaitCursorRemove (cd->mainwid) ;
	return (TRUE) ;
    } else {
	return (FALSE) ;
    }

}

void
fetch_alarms_new
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*new)
#else	/* no prototypes */
	(cd, new)
	CalendarDisplay		cd;
	ProfileStructure	*new;
#endif	/* prototype */
    {
    Boolean		alarm_entry ;
    Boolean		alarm_short ;
    Boolean		alarm_medium ;
    Boolean		alarm_long ;
    Boolean		alarm_days ;
    int			scale_short ;
    int			scale_medium ;
    int			scale_long ;
    int			scale_days ;
    Cardinal		default_icons_number ;
    unsigned char	*default_icons ;
    int			temp;

    alarm_entry  = XmToggleButtonGetState (cd->alarm_entry_tb) ;
    alarm_short  = XmToggleButtonGetState (cd->alarm_minutes_tb) ;
    alarm_medium = XmToggleButtonGetState (cd->alarm_min_bis_tb) ;
    alarm_long   = XmToggleButtonGetState (cd->alarm_hours_tb) ;
    alarm_days   = XmToggleButtonGetState (cd->alarm_days_tb) ;

    XmScaleGetValue (cd->alarm_minutes_scale, &scale_short) ;
    XmScaleGetValue (cd->alarm_min_bis_scale, &scale_medium) ;
    XmScaleGetValue (cd->alarm_hours_scale,   &scale_long) ;
    XmScaleGetValue (cd->alarm_days_scale,    &scale_days) ;

    new->alarms [0] = 0 ;
    new->alarms [1] = scale_short ;
    new->alarms [2] = scale_medium ;
    new->alarms [3] = scale_long * 6 ;
    new->alarms [4] = scale_days * 24 * 6 ;

    new->alarms_mask = 0 ;

    if (alarm_entry)   new->alarms_mask = new->alarms_mask | 001 ;
    if (alarm_short)   new->alarms_mask = new->alarms_mask | 002 ;
    if (alarm_medium)  new->alarms_mask = new->alarms_mask | 004 ;
    if (alarm_long)    new->alarms_mask = new->alarms_mask | 010 ;
    if (alarm_days)    new->alarms_mask = new->alarms_mask | 020 ;

    return ;
}

void
DO_ALARMS_RESET
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    CUSTSetAlarmsFromProfile(cd, &cd->original) ;

}

void
DO_ALARMS_CANCEL
#if defined(_DWC_PROTO_)
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w) ;

    DO_ALARMS_RESET (w, tag) ;
    DO_ALARMS_OK    (w, tag) ;

}

static void register_names
#if defined(_DWC_PROTO_)
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{

    static MrmRegisterArg regvec [] =
    {
	/*
	**  Identifiers
	*/
	{"off_pixmap_array",		    (caddr_t) medium_pixmaps},
	{"dwc_bell_12X12",		    (caddr_t)0},
	{"dwc_bell_17X17",		    (caddr_t)0},
	{"dwc_bell_32X32",		    (caddr_t)0},
	
	/*
	**  Callbacks	
	*/

	{"time_set",			   (caddr_t) TIME_SET},
	{"do_change_settings_ok",	   (caddr_t) DO_GENERAL_OK},
	{"do_change_settings_cancel",	   (caddr_t) DO_GENERAL_CANCEL},
	{"do_change_settings_apply",	   (caddr_t) DO_GENERAL_APPLY},
	{"do_change_settings_reset",	   (caddr_t) DO_GENERAL_RESET},
	{"first_week_consistency",	   (caddr_t) FIRST_WEEK_CONSISTENCY},
	{"do_dayview_ok",		   (caddr_t) DO_DAYVIEW_OK},
	{"do_dayview_cancel",		   (caddr_t) DO_DAYVIEW_CANCEL},
	{"do_dayview_apply",		   (caddr_t) DO_DAYVIEW_APPLY},
	{"do_dayview_reset",		   (caddr_t) DO_DAYVIEW_RESET},
	{"do_iconbox_ok",		   (caddr_t) DO_ICONBOX_OK},
	{"do_iconbox_cancel",		   (caddr_t) DO_ICONBOX_CANCEL},
	{"do_iconbox_apply",		   (caddr_t) DO_ICONBOX_APPLY},
	{"do_iconbox_reset",		   (caddr_t) DO_ICONBOX_RESET},
	{"do_alarms_ok",		   (caddr_t) DO_ALARMS_OK},
	{"do_alarms_cancel",		   (caddr_t) DO_ALARMS_CANCEL},
	{"do_alarms_apply",		   (caddr_t) DO_ALARMS_APPLY},
	{"do_alarms_reset",		   (caddr_t) DO_ALARMS_RESET},
        {"iconbox_show_text_change",	   (caddr_t) ICONBOX_SHOW_TEXT_CHANGE},
	{"iconbox_show_day_change",	   (caddr_t) ICONBOX_SHOW_DAY_CHANGE},
	{"fine_incr_consistency",	   (caddr_t) FINE_INCR_CONSISTENCY},
	{"toggle_alarm_scale",		   (caddr_t) TOGGLE_ALARM_SCALE},
	{"day_default_icon_value_changed", (caddr_t) ICON_TOGGLE_VALUE_CHANGED},
	{"icon_radio_value_changed",	   (caddr_t) ICON_SWITCH_VALUE_CHANGED}
    };

    MrmCount regnum = XtNumber(regvec);

    /*
    **  Careful here!
    */
    regvec[1].value = (caddr_t)small_pixmaps[k_pixmap_bell];

    regvec[2].value = (caddr_t)medium_pixmaps[k_pixmap_bell];

    regvec[3].value = (caddr_t)big_pixmaps[k_pixmap_bell];


    /*
    **  Let's register our routines
    */
    MrmRegisterNames(regvec, regnum);

    /*
    **  That's it
    */
    return;
    }

Boolean CUSTApplyProfile
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	ProfileStructure	*new)
#else	/* no prototypes */
	(cd, new)
	CalendarDisplay		cd;
	ProfileStructure	*new;
#endif	/* prototype */
{
    Boolean	    change_alarm_icon;
    int		    i;
    unsigned char   junk;
    unsigned char   marks;
    Boolean	    reconfigure_day = FALSE;
    unsigned char   *default_icons;
    Cardinal	    default_icons_number;
    Pixmap	    *pixmaps;
    XmFontList	    fontlist;
    Arg		    arglget [ 1 ];

    Cardinal		ac;
    Arg			arglist [ 10 ];
    Widget		option;
    Boolean		rtol_changed = FALSE;
    Boolean		reget_day = FALSE;
    Boolean		ampm_changed = FALSE;
    char		applname [256] ;
    Dimension		pixmap_size;
    int			appllen;
    long		byte_count, cvt_status;
    XmString		xm_applname;
    Boolean		change_fine_increment = False;

/*
*/
    if ((! new->day_show_notes) && (cd->profile.day_show_notes))
    {
	if ((DSWGetOpenEntry ((DayslotsWidget)(cd->daynotes.widget)) != NULL) ||
	    (SEAnyNoteEditorsUpAndRunning (cd)))
	{

	    ERRORDisplayError (cd->mainwid, "ErrorCloseDaynote") ;
	    ICONSWaitCursorRemove (cd->mainwid) ;
	    return (FALSE) ;
	}
    }

    if (new->work_days != cd->profile.work_days)
    {
	if (DAYTestAnyOpenEntries(cd))
	{
	    ERRORDisplayError(cd->mainwid, "ErrorCloseAllMark") ;
	    ICONSWaitCursorRemove(cd->mainwid) ;
	    return (FALSE) ;
	}
    }

    switch (new->day_icon_size)
    {
    case 0:
	pixmaps = small_pixmaps;
	pixmap_size = 12;
	break;
    case 1:
	pixmaps = medium_pixmaps;
	pixmap_size = 17;
	break;
    case 2:
	pixmaps = big_pixmaps;
	pixmap_size = 32;
	break;
    }

    switch (new->day_font_size)
    {
    case 0 :
	option = cd->timeslot_small_font_label;
	break;
    case 1 :
	 option = cd->timeslot_medium_font_label;
	 break;
    case 2 :
	 option = cd->timeslot_large_font_label;
	 break;
    }
    XtSetArg (arglget [0], XmNfontList, &fontlist); ac++;
    XtGetValues (option, arglget, 1) ;

    if
    (!DSWCheckValues
	(
	    (DayslotsWidget) (cd->dayslots.widget),
	    pixmaps,
	    fontlist, 
	    new->day_v_spacing,
	    new->timeslot_size,
	    new->fine_increment
	)
    )
    {
	ERRORDisplayError (cd->mainwid, "ErrorUnconfigurable") ;
	ICONSWaitCursorRemove (cd->mainwid) ;
	return (FALSE) ;
    }

    /* 
	First alarms stuff 
    */

    change_alarm_icon = (cd->profile.alarms_mask == 0) || 
	    (new->alarms_mask == 0);
    change_alarm_icon = change_alarm_icon && 
	    (cd->profile.alarms_mask != new->alarms_mask );
    for(i = 0; i < 5; i++ )
	cd->profile.alarms[i] = new->alarms[i];

    cd->profile.alarms_mask = new->alarms_mask;

/*
    General stuff 
*/

    cd->profile.first_display = new->first_display;
    cd->profile.start_iconized = new->start_iconized;
    cd->profile.direct_scroll_coupling = new->direct_scroll_coupling;
    cd->profile.print_blank_days = new->print_blank_days;

    if (new->directionRtoL != cd->profile.directionRtoL)
    {
	cd->profile.directionRtoL = new->directionRtoL;
	rtol_changed = TRUE ;
    }

    cd->profile.auto_click_on_clock = new->auto_click_on_clock;

    /*
    **  Day Display stuff
    */
    
    ac = 0 ;

    if (new->day_font_size != cd->profile.day_font_size)
    {
	XtSetArg(arglist[ac], DwcDswNfontList, fontlist); ac++;
    }

    if (new->day_icon_size != cd->profile.day_icon_size)
    {
	XtSetArg(arglist[ac], DwcDswNpixmaps, pixmaps); ac++;
	XtSetArg(arglist[ac], DwcDswNpixmapWidth, pixmap_size); ac++;
	XtSetArg(arglist[ac], DwcDswNpixmapHeight, pixmap_size); ac++;
    }

    if (new->day_v_spacing != cd->profile.day_v_spacing)
    {
	XtSetArg(arglist[ac], DwcDswNtimeVerticalMargin, new->day_v_spacing); ac++;
    }

    if (new->day_h_spacing != cd->profile.day_h_spacing)
    {
	XtSetArg(arglist[ac], DwcDswNtimeHorizontalMargin, new->day_h_spacing); ac++;
    }

    if (ac != 0)
    {
	XtSetValues (cd->daynotes.widget, arglist, ac) ;
	reconfigure_day = TRUE ;
    }

    if (new->day_timebar_width != cd->profile.day_timebar_width)
    {
	XtSetArg(arglist[ac], DwcDswNtimebarWidth, new->day_timebar_width); ac++;
    }

    if (new->timeslot_stacking != cd->profile.timeslot_stacking)
    {
	XtSetArg(arglist[ac], DwcDswNstackTopDown, new->timeslot_stacking); ac++;
    }

    if (new->times_on_the_line != cd->profile.times_on_the_line)
    {
	XtSetArg(arglist[ac], DwcDswNonTheLine, new->times_on_the_line); ac++;
    }

    if (new->timeslot_size != cd->profile.timeslot_size)
    {
	XtSetArg(arglist[ac], DwcDswNpreferredDayslotsSize, new->timeslot_size); ac++;
    }

    if (new->fine_increment != cd->profile.fine_increment)
    {
	XtSetArg(
	    arglist[ac], DwcDswNpreferredFineIncrement, new->fine_increment);
	ac++;
	change_fine_increment = True;
    }


    if (cd->showing == show_day)
    {
	if (DWC$DB_Get_day_attr (cd->cab, cd->dsbot, &marks, &junk, &junk) ==
							      DWC$k_db_normal)
	{
	    marks = marks & ~(DWC$m_day_defaulted | DWC$m_day_special) ;
	    if (marks == DWC$k_day_workday)
	    {
		if (new->work_minutes_start != cd->profile.work_minutes_start)
		{
		    XtSetArg(arglist[ac], DwcDswNworkStart, new->work_minutes_start); ac++;
		}
		if (new->work_minutes_end != cd->profile.work_minutes_end)
		{
		    XtSetArg(arglist[ac], DwcDswNworkFinish, new->work_minutes_end); ac++;
		}
		if (new->lunch_minutes_start != cd->profile.lunch_minutes_start)
		{
		    XtSetArg(arglist[ac], DwcDswNlunchStart, new->lunch_minutes_start); ac++;
		}
		if (new->lunch_minutes_end != cd->profile.lunch_minutes_end)
		{
		    XtSetArg(arglist[ac], DwcDswNlunchFinish, new->lunch_minutes_end); ac++;
		}
	    }
	} else {
	    ERRORDisplayError (cd->mainwid, "ErrorDayAttr") ;
	}
    }

    if (new->day_show_full_times != cd->profile.day_show_full_times)
    {
	if (new->day_show_full_times)
	{
	    XtSetArg(arglist[ac], DwcDswNminuteTexts, NULL); ac++;
	}
	else
	{
	    XtSetArg(arglist[ac], DwcDswNminuteTexts, minute_texts); ac++;
	}
    }

    if (ac != 0) {
	XtSetValues (cd->dayslots.widget, arglist, ac) ;
    }

    if ((cd->profile.day_show_months   != new->day_show_months) ||
        (cd->profile.day_show_notes    != new->day_show_notes)  ||
        (cd->profile.day_timebar_width != new->day_timebar_width))
    {
	reconfigure_day = TRUE ;
    }

    if (new->entries_significant)
    {
	cd->dayslots.default_item->flags = 
	    cd->dayslots.default_item->flags & ~DWC$m_item_insignif;
    }
    else
    {
	cd->dayslots.default_item->flags = 
	    cd->dayslots.default_item->flags |  DWC$m_item_insignif;
    }

    if (new->notes_significant)
    {
	cd->daynotes.default_item->flags = 
	    cd->daynotes.default_item->flags & ~DWC$m_item_insignif;
    }
    else
    {
	cd->daynotes.default_item->flags = 
	    cd->daynotes.default_item->flags |  DWC$m_item_insignif;
    }

    XtFree ((char *)cd->dayslots.default_item->alarms_times);

    CUSTGetProfileAlarms
    (
	new,
	&cd->dayslots.default_item->alarms_number,
	&cd->dayslots.default_item->alarms_times
    );

    if ((cd->profile.default_entry_icon != new->default_entry_icon) ||
	(change_alarm_icon))
    {
	cd->profile.default_entry_icon = new->default_entry_icon;
	if (cd->dayslots.default_item->alarms_number == 0)
	{
	    default_icons = (unsigned char *) 
		    XtMalloc (sizeof (unsigned char)) ;
	    default_icons_number = 1 ;
	}
	else
	{
	    default_icons = (unsigned char *) 
		    XtMalloc (sizeof (unsigned char) * 2);
	    default_icons_number = 2 ;
	    default_icons [1] = k_pixmap_bell ;
	}
	default_icons [0] = new->default_entry_icon ;

	ac = 0;
	XtSetArg(arglist[ac], DwcDswNdefaultNumIcons, default_icons_number);
	    ac++;
	XtSetArg(arglist[ac], DwcDswNdefaultIcons, default_icons) ; ac++;
	XtSetValues (cd->dayslots.widget, arglist, ac);

	XtFree ((char *)cd->dayslots.default_item->icons);

	if (cd->dayslots.default_item->alarms_number != 0)
	{
	    XtRealloc ((char *)default_icons, sizeof (unsigned char)) ;
	    default_icons_number = 1 ;
	}

	cd->dayslots.default_item->icons        = default_icons ;
	cd->dayslots.default_item->icons_number = default_icons_number ;
    }


    if (new->default_notes_icon != cd->profile.default_notes_icon)
    {
	cd->profile.default_notes_icon = new->default_notes_icon;
	default_icons = (unsigned char *) XtMalloc (sizeof (unsigned char)) ;
	default_icons_number = 1 ;
	default_icons [0] = new->default_notes_icon ;

	ac = 0 ;
	XtSetArg(arglist[ac], DwcDswNdefaultNumIcons, default_icons_number);
	    ac++;
	XtSetArg(arglist[ac], DwcDswNdefaultIcons, default_icons) ; ac++;
	XtSetValues (cd->daynotes.widget, arglist, ac) ;

	XtFree ((char *)cd->daynotes.default_item->icons) ;
	cd->daynotes.default_item->icons        = default_icons ;
	cd->daynotes.default_item->icons_number = default_icons_number ;
    }
    
   cd->profile.day_show_months = new->day_show_months;
   cd->profile.day_show_notes = new->day_show_notes;
   cd->profile.day_show_full_times = new->day_show_full_times;
   cd->profile.timeslot_size = new->timeslot_size;
   cd->profile.fine_increment = new->fine_increment;
   cd->profile.work_minutes_start = new->work_minutes_start;
   cd->profile.work_minutes_end = new->work_minutes_end;
   cd->profile.lunch_minutes_start = new->lunch_minutes_start;
   cd->profile.lunch_minutes_end = new->lunch_minutes_end;
   cd->profile.day_v_spacing = new->day_v_spacing;
   cd->profile.day_h_spacing = new->day_h_spacing;
   cd->profile.day_timebar_width = new->day_timebar_width;
   cd->profile.times_on_the_line = new->times_on_the_line;
   cd->profile.timeslot_stacking = new->timeslot_stacking;
   cd->profile.day_font_size = new->day_font_size;
   cd->profile.day_icon_size = new->day_icon_size;
   cd->profile.entries_significant = new->entries_significant;
   cd->profile.notes_significant = new->notes_significant;

   /* 
	Iconbox stuff 

    */

    cd->profile.icon_show_text = new->icon_show_text;
    memcpy(cd->profile.icon_text, new->icon_text, 32);
/* probably obsolete with Mwm window manager icons.
    cd->profile.icon_nl_after_text = new->icon_nl_after_text;
*/
    cd->profile.icon_show_day = new->icon_show_day;
    cd->profile.icon_full_day = new->icon_full_day;
/* probably obsolete with Mwm window manager icons.
    cd->profile.icon_nl_after_day = new->icon_nl_after_day;
*/
    cd->profile.icon_show_time = new->icon_show_time;

    appllen = strlen(cd->appl_title);
    memcpy (applname, cd->appl_title, appllen);
    if (new->icon_show_text)
    {
	strcpy (&applname[appllen], new->icon_text);
    }
    else
    {
	strcpy (&applname[appllen], cd->filename);
    }

    xm_applname = DXmCvtFCtoCS(applname, &byte_count, &cvt_status);
    DWI18n_SetTitle(cd->toplevel, xm_applname);
    XmStringFree(xm_applname);

   /*
    **  Now setup months in day and year display
    */
    
    ac = 0 ;
    if (new->first_day_of_week != cd->profile.first_day_of_week)
	{
	XtSetArg(arglist[ac], DwcMwNweekStart, new->first_day_of_week); ac++;
	}

    if (new->week_numbers_start_day != cd->profile.week_numbers_start_day)
	{
	XtSetArg(arglist[ac], DwcMwNweekNumbersStartingDay, new->week_numbers_start_day); ac++;
	}

    if (new->week_numbers_start_month != cd->profile.week_numbers_start_month)
	{
	XtSetArg(arglist[ac], DwcMwNweekNumbersStartingMonth, new->week_numbers_start_month); ac++;
	}

    if (new->show_week_numbers != cd->profile.show_week_numbers)
	{
	XtSetArg(arglist[ac], DwcMwNshowWeekNumbers, new->show_week_numbers); ac++;
	}

    if (new->work_days != cd->profile.work_days)
	{
	XtSetArg(arglist[ac], DwcMwNworkDays, new->work_days); ac++;
	}

    if (new->month_style_mask != cd->profile.month_style_mask)
	{
	XtSetArg(arglist[ac], DwcMwNstyleMask, new->month_style_mask); ac++;
	}

    if (ac != 0) {
	YEARSetAllMonths(cd->day_yd, arglist, ac);

	if (cd->yd != NULL) {
	    YEARSetAllMonths(cd->yd, arglist, ac);
	}

	if (new->work_days != cd->profile.work_days) {
	    CUSTTellDBWorkDays (cd, new->work_days) ;
	    MWRegetAllDayMarkup (cd->month_display) ;
	}
    }

    /*
    **  Month display stuff
    */
    
    if (new->fill_blank_days != cd->profile.fill_blank_days) {
	if (new->fill_blank_days) {
	    XtSetArg(arglist[ac], DwcMwNmode, DwcMwModeFillBlanks) ; ac++;
	} else {
	    XtSetArg(arglist[ac], DwcMwNmode, DwcMwModeDefault) ; ac++;
	}
    }

    if ((ac != 0) && (cd->lw_month_display != NULL)) {
	XtSetValues (cd->month_display, arglist, ac) ;
    }

    cd->profile.first_day_of_week = new->first_day_of_week;
    cd->profile.show_week_numbers = new->show_week_numbers;
    cd->profile.week_numbers_start_day = new->week_numbers_start_day;
    cd->profile.week_numbers_start_month = new->week_numbers_start_month;
    cd->profile.fill_blank_days = new->fill_blank_days;
    cd->profile.month_style_mask = new->month_style_mask;

    /*
    **  Now things that affect the day display...
    */
    
    if (new->time_am_pm != cd->profile.time_am_pm) {
	ampm_changed = TRUE ;
	cd->profile.time_am_pm = new->time_am_pm;
    }
    
    if ((cd->showing == show_day) && (new->work_days != cd->profile.work_days)) {
	reget_day = TRUE ;
    }
    cd->profile.work_days = new->work_days;

    ac = 0 ;
    if (rtol_changed) {
	XtSetArg(arglist[ac], XmNstringDirection, new->directionRtoL) ; ac++;
    }

    if (ac != 0) {
	XtSetValues (cd->daynotes.widget, arglist, ac) ;
    }

    ac = 0;
    if (ampm_changed) {
	if (new->time_am_pm) {
	    XtSetArg(arglist[ac], DwcDswNtimeTexts, time_texts_12) ; ac++;
	} else {
	    XtSetArg(arglist[ac], DwcDswNtimeTexts, time_texts_24) ; ac++;
	}
    }

    if (rtol_changed) {
	XtSetArg(arglist[ac], XmNstringDirection, new->directionRtoL) ; ac++;
    }

    if (ac != 0) {
	XtSetValues (cd->dayslots.widget, arglist, ac) ;
    }


    setup_icon(cd );

    if (ampm_changed) {
	ALARMSUpdateTimeDisplay (cd) ;
	ALARMSSetAlarmsAmPm (cd, new->time_am_pm) ;
	SEChangeAMPM( cd );
	reconfigure_day = TRUE ;
    }

    if (rtol_changed) {
	if (cd->yd != NULL) {
	    YEARConfigureYearDisplay (cd->yd, FALSE, TRUE) ;
	}

	if (cd->lw_month_display != NULL) {
	    MONTHConfigMonthDisplay (cd) ;
	}

	SEUpdateSlotEditorScrollBar (cd, new->directionRtoL) ;
	ALARMSSetAlarmsDirection(cd, new->directionRtoL) ;
	reconfigure_day = TRUE ;
    }
    
    if (reconfigure_day) {
	DAYConfigureDayDisplay(cd);
    }

    if (reget_day) {
	MISCChangeView (cd, show_day, cd->day, cd->month, cd->year) ;
    }
    
/*  now things that affect the customize day view box */

    if (ampm_changed && (cd->pb_work_hours_start_hours != NULL)) {
	set_work_hours_buttons 
	  (cd->pb_work_hours_start_hours,  cd->pb_work_hours_start_minutes,
	   cd->lb_work_hours_start_ampm,
	   cd->pb_work_hours_finish_hours, cd->pb_work_hours_finish_minutes,
	   cd->lb_work_hours_finish_ampm,
	   cd->work_hours_start_time,    cd->work_hours_finish_time,
	   new->time_am_pm) ;

	set_work_hours_buttons 
	  (cd->lunch_start_hour_pb,      cd->lunch_start_minute_pb,
	   cd->lunch_start_am_pm_lb,
	   cd->lunch_finish_hour_pb,     cd->lunch_finish_minute_pb,
	   cd->lunch_finish_am_pm_lb,
	   cd->lunch_hours_finish_time, cd->lunch_hours_finish_time,
	   new->time_am_pm) ;
    }

    if (change_fine_increment) CALStartTimeTextWorkProc (cd);

    return (TRUE) ;
}

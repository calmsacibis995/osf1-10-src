/* dwc_ui_misc.c */
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
**	Marios Cleovoulou, March-1988 (DWC_UI_MISC.C)
**
**  ABSTRACT:
**
**	This is the module contains miscellaneous support routines
**
**--
*/

#include "dwc_compat.h"

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#if defined(vaxc)
#pragma nostandard
#endif
#include <Xm/Xm.h>			/* XmAnyCallbackStruct */
#include <Xm/MainW.h>			/* XmMainWindowSetAreas */
#include <Xm/Text.h>			/* XmTextWidget */
#include <Xm/Label.h>			/* XmLabelWidget */
#include <Xm/ScrollBar.h>		/* for XmScrollBarSetValues */
#include <Xm/DialogS.h>			/* for XmIsDialogShell */
#include <Xm/MessageB.h>		/* for XmIsMessageBox */
#include <Xm/Protocols.h>		/* for XmAddWmProtocolCallback */
#include <Xm/AtomMgr.h>			/* for XmInternAtom */
#include <Mrm/MrmPublic.h>		/* MrmCode, etc. */
#include <DXm/DXmCSText.h>		/* DXmCSTextWidget */
#include <DXm/DECspecific.h>
#if defined(vaxc)
#pragma standard
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* for DWC$k_db_normal */

#include "dwc_ui_dateformat.h"
#include "dwc_ui_calendar.h"		/* CalendarDisplay */
#include "dwc_ui_catchall.h"		/* DWC$DRM */
#include "dwc_ui_misc.h"	
#include "dwc_ui_month.h"		/* for MONTHCreateMonthDisplay */
#include "dwc_ui_day.h"			/* for DAYGetDayItems */
#include "dwc_ui_monthwidget.h"		/* for MWSetDate */
#include "dwc_ui_clipboard.h"		/* CLIPDeselectAllEntries */
#include "dwc_ui_yeardisplay.h"		/* YEARSetYearDisplayDate */
#if MEMEX
#include "dwc_ui_memex.h"		/* MEMEXSurrogateHighlightForDay */
#endif
#include "dwc_ui_icons.h"
#include "dwc_ui_errorboxes.h"
#include "dwc_ui_datefunctions.h"

#undef _XmGetFlagsBit
#define _XmGetFlagsBit(field, bit) \
      ((field[ ((unsigned int)bit >> 3) ]) & (1 << ((unsigned int)bit & 0x07)))

int find_index PROTOTYPE ((CalendarDisplay cd, Widget w));
static void save_current_view_xy PROTOTYPE ((CalendarDisplay cd));

/*
**  Global Data Definitions
*/
Pixmap    small_pixmaps[256];
Pixmap    medium_pixmaps[256];
Pixmap	  defdep_medium_pixmaps[256];
Pixmap    big_pixmaps[256];
Pixmap	  main_icons[4] = {0, 0, 0, 0};
Pixmap	  alarm_icons[4] = {0, 0, 0, 0};
Pixmap    dwc_bell_negative_32X32;
Pixmap    dwc_bell_negative_12X12;
unsigned int	pixmap_foreground;

/*
**  External Data Declarations
*/
extern AllDisplaysRecord ads;		/* defined in dwc_ui_calendar.c */


/* the following time variables are used to maintain the ui's internal time */
static struct tm	*master_local_time;


static int related_dialog_table[ K_CALENDAR_WIDGET_COUNT ];
static int related_dialog_init = 0;
static int offset_table[ ] =
{
    XtOffset( CalendarDisplay, pb_open ),
    XtOffset( CalendarDisplay, pb_open_new ),
    XtOffset( CalendarDisplay, pb_close ),
    XtOffset( CalendarDisplay, pb_nameas ),
    XtOffset( CalendarDisplay, pb_delete ),
    XtOffset( CalendarDisplay, pb_print ),
    XtOffset( CalendarDisplay, pb_file_include ),
    XtOffset( CalendarDisplay, pb_file_exit ),
    XtOffset( CalendarDisplay, pb_edit_cut ),
    XtOffset( CalendarDisplay, pb_edit_copy ),
    XtOffset( CalendarDisplay, pb_edit_paste ),
    XtOffset( CalendarDisplay, pb_edit_clear ),
    XtOffset( CalendarDisplay, pb_edit_selall ),
    XtOffset( CalendarDisplay, pb_selected ),
    XtOffset( CalendarDisplay, pb_today ),
    XtOffset( CalendarDisplay, pb_day ),
    XtOffset( CalendarDisplay, pb_week ),
    XtOffset( CalendarDisplay, pb_month ),
    XtOffset( CalendarDisplay, pb_year ),
    XtOffset( CalendarDisplay, pull_memex),
    XtOffset( CalendarDisplay, pull_entry ),
    XtOffset( CalendarDisplay, pb_entry_menu ),
    XtOffset( CalendarDisplay, pb_delete_menu ),
    XtOffset( CalendarDisplay, pb_close_menu ),      
    XtOffset( CalendarDisplay, pb_reset_menu ),
    XtOffset( CalendarDisplay, pb_options_cust ),
    XtOffset( CalendarDisplay, pb_custom_save ),
    0xf0f0f0f, /* restore */
    0xf0f0f0f, /* use saved */
    XtOffset( CalendarDisplay, pb_options_mark ),
    XtOffset( CalendarDisplay, pb_mark_default ),
    XtOffset( CalendarDisplay, pb_mark_normal ),
    XtOffset( CalendarDisplay, pb_mark_work_day ),
    XtOffset( CalendarDisplay, pb_mark_non_work_day ),
    XtOffset( CalendarDisplay, pb_mark_special ),

    XtOffset( CalendarDisplay, db_nameas ),
    XtOffset( CalendarDisplay, tw_nameas ),
    XtOffset( CalendarDisplay, db_print ),
    XtOffset( CalendarDisplay, db_print_lb_week ),
    XtOffset( CalendarDisplay, db_print_lb_date ),
    XtOffset( CalendarDisplay, db_print_lb_start ),
    XtOffset( CalendarDisplay, db_print_lb_finish ),
    XtOffset( CalendarDisplay, tb_print_create ),
    XtOffset( CalendarDisplay, tb_print_append ),
    XtOffset( CalendarDisplay, tw_print ),
    XtOffset( CalendarDisplay, sw_print ),
    XtOffset( CalendarDisplay, tb_print_limit ),
    XtOffset( CalendarDisplay, day_show_months ),
    XtOffset( CalendarDisplay, day_entry_significant ),
    XtOffset( CalendarDisplay, day_daynote_significant ),

    XtOffset( CalendarDisplay, day_daynote_default_icon_tb ),
    XtOffset( CalendarDisplay, day_slot_default_icon_tb ),
    XtOffset( CalendarDisplay, day_default_icon_box_widget ),

#if defined(__DECC) && !defined(vms)
    XtOffset( CalendarDisplay, wkd) + (0 * sizeof(Widget)),
    XtOffset( CalendarDisplay, wkd) + (1 * sizeof(Widget)),
    XtOffset( CalendarDisplay, wkd) + (2 * sizeof(Widget)),
    XtOffset( CalendarDisplay, wkd) + (3 * sizeof(Widget)),
    XtOffset( CalendarDisplay, wkd) + (4 * sizeof(Widget)),
    XtOffset( CalendarDisplay, wkd) + (5 * sizeof(Widget)),
    XtOffset( CalendarDisplay, wkd) + (6 * sizeof(Widget)),
#else
    XtOffset( CalendarDisplay, wkd [0] ),
    XtOffset( CalendarDisplay, wkd [1] ),
    XtOffset( CalendarDisplay, wkd [2] ),
    XtOffset( CalendarDisplay, wkd [3] ),
    XtOffset( CalendarDisplay, wkd [4] ),
    XtOffset( CalendarDisplay, wkd [5] ),
    XtOffset( CalendarDisplay, wkd [6] ),
#endif
    XtOffset( CalendarDisplay, month_swn ),
    XtOffset( CalendarDisplay, month_fbd ),
    XtOffset( CalendarDisplay, fdow_option_menu ),
#if defined(__DECC) && !defined(vms)
    XtOffset( CalendarDisplay, fdow) + (0 * sizeof(Widget)),
    XtOffset( CalendarDisplay, fdow) + (1 * sizeof(Widget)),
    XtOffset( CalendarDisplay, fdow) + (3 * sizeof(Widget)),
    XtOffset( CalendarDisplay, fdow) + (4 * sizeof(Widget)),
    XtOffset( CalendarDisplay, fdow) + (5 * sizeof(Widget)),
    XtOffset( CalendarDisplay, fdow) + (6 * sizeof(Widget)),
    XtOffset( CalendarDisplay, fdow) + (7 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (0 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (1 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (2 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (3 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (4 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (5 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (6 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (7 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (8 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (9 * sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (10* sizeof(Widget)),
    XtOffset( CalendarDisplay, week_num_start_month_pbs) + (11* sizeof(Widget)),
#else
    XtOffset( CalendarDisplay, fdow [0] ),
    XtOffset( CalendarDisplay, fdow [1] ),
    XtOffset( CalendarDisplay, fdow [2] ),
    XtOffset( CalendarDisplay, fdow [3] ),
    XtOffset( CalendarDisplay, fdow [4] ),
    XtOffset( CalendarDisplay, fdow [5] ),
    XtOffset( CalendarDisplay, fdow [6] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 0] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 1] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 2] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 3] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 4] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 5] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 6] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 7] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 8] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[ 9] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[10] ),
    XtOffset( CalendarDisplay, week_num_start_month_pbs[11] ),
#endif
    XtOffset( CalendarDisplay, week_num_start_day_scale ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_60 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_30 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_20 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_15 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_12 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_10 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_06 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_05 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_04 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_03 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_02 ),
    XtOffset( CalendarDisplay, custom_buttons.tb_tss_01 ),
    XtOffset( CalendarDisplay, first_dpy_day ),
    XtOffset( CalendarDisplay, first_dpy_week ),
    XtOffset( CalendarDisplay, first_dpy_month ),
    XtOffset( CalendarDisplay, first_dpy_year ),
    XtOffset( CalendarDisplay, start_icon_tb ),
    XtOffset( CalendarDisplay, time_am_pm ),
    XtOffset( CalendarDisplay, pb_work_hours_start_hours ),
    XtOffset( CalendarDisplay, pb_work_hours_start_minutes ),
    XtOffset( CalendarDisplay, lb_work_hours_start_ampm ),
    XtOffset( CalendarDisplay, pb_work_hours_finish_hours ),
    XtOffset( CalendarDisplay, pb_work_hours_finish_minutes ),
    XtOffset( CalendarDisplay, lb_work_hours_finish_ampm ),
    XtOffset( CalendarDisplay, db_open ),
    XtOffset( CalendarDisplay, db_include_file ),
    XtOffset( CalendarDisplay, cb_delete ),
    XtOffset( CalendarDisplay, menubar ),
    XtOffset( CalendarDisplay, lb_repeat_prompt ),
    0xF0F0F0F,
    XtOffset( CalendarDisplay, rb_repeat_questions ),
    XtOffset( CalendarDisplay, tb_repeat_this ),
    XtOffset( CalendarDisplay, tb_repeat_future ),
    XtOffset( CalendarDisplay, tb_repeat_all ),
    XtOffset( CalendarDisplay, pb_repeat_ok ),
    XtOffset( CalendarDisplay, pb_repeat_cancel ),
    XtOffset( CalendarDisplay, week_num_start_month_om ),
    XtOffset( CalendarDisplay, om_tss ),
    XtOffset( CalendarDisplay, om_first_dpy ),
    XtOffset( CalendarDisplay, r_to_l_tb ),
    XtOffset( CalendarDisplay, fine_incr_om ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_60_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_30_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_20_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_15_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_12_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_10_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_6_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_5_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_4_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_3_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_2_pb ),
    XtOffset( CalendarDisplay, custom_buttons.fine_incr_1_pb ),
    XtOffset( CalendarDisplay, stacking_om ),
    XtOffset( CalendarDisplay, stack_r_l_pb ),
    XtOffset( CalendarDisplay, stack_top_down_pb ),
    XtOffset( CalendarDisplay, font_small_tb ),
    XtOffset( CalendarDisplay, font_medium_tb ),
    XtOffset( CalendarDisplay, font_large_tb ),
    XtOffset( CalendarDisplay, icon_small_tb ),
    XtOffset( CalendarDisplay, icon_medium_tb ),
    XtOffset( CalendarDisplay, icon_large_tb ),
    XtOffset( CalendarDisplay, scrollbar_coupling_tb ),
    XtOffset( CalendarDisplay, times_on_line_tb ),
    XtOffset( CalendarDisplay, show_full_times_tb ),
    XtOffset( CalendarDisplay, show_day_notes_tb ),
    XtOffset( CalendarDisplay, vertical_spacing_scale ),
    XtOffset( CalendarDisplay, horizontal_spacing_scale ),
    XtOffset( CalendarDisplay, timebar_width_scale ),
    XtOffset( CalendarDisplay, lunch_start_hour_pb ),
    XtOffset( CalendarDisplay, lunch_start_minute_pb ),
    XtOffset( CalendarDisplay, lunch_start_am_pm_lb ),
    XtOffset( CalendarDisplay, lunch_finish_hour_pb ),
    XtOffset( CalendarDisplay, lunch_finish_minute_pb ),
    XtOffset( CalendarDisplay, lunch_finish_am_pm_lb ),
    XtOffset( CalendarDisplay, print_blank_tb ),
    XtOffset( CalendarDisplay, iconbox_show_text_tb ),
    XtOffset( CalendarDisplay, iconbox_text_stext ),
    XtOffset( CalendarDisplay, iconbox_nl_after_text_toggle ),
    XtOffset( CalendarDisplay, iconbox_show_day_toggle ),
    XtOffset( CalendarDisplay, iconbox_full_day_toggle ),
    XtOffset( CalendarDisplay, iconbox_nl_after_day_toggle ),
    XtOffset( CalendarDisplay, iconbox_show_time_toggle ),
    XtOffset( CalendarDisplay, general_pb ),
    XtOffset( CalendarDisplay, dayview_pb ),
    XtOffset( CalendarDisplay, iconbox_pb ),
    XtOffset( CalendarDisplay, alarms_pb ),
    XtOffset( CalendarDisplay, general_db ),
    XtOffset( CalendarDisplay, dayview_db ),
    XtOffset( CalendarDisplay, iconbox_db ),
    XtOffset( CalendarDisplay, alarms_db ),
    XtOffset( CalendarDisplay, alarm_entry_tb),
    XtOffset( CalendarDisplay, alarm_hours_tb),
    XtOffset( CalendarDisplay, alarm_hours_scale),
    XtOffset( CalendarDisplay, alarm_minutes_tb),
    XtOffset( CalendarDisplay, alarm_minutes_scale),
    XtOffset( CalendarDisplay, alarm_min_bis_tb),
    XtOffset( CalendarDisplay, alarm_min_bis_scale),
    XtOffset( CalendarDisplay, month_style_slash_pb),
    XtOffset( CalendarDisplay, month_style_cross_pb),
    XtOffset( CalendarDisplay, month_style_clean_pb),
    XtOffset( CalendarDisplay, alarm_days_tb),
    XtOffset( CalendarDisplay, alarm_days_scale),
    XtOffset( CalendarDisplay, auto_click_clock_tb),
    XtOffset( CalendarDisplay, month_style_menu),
    XtOffset( CalendarDisplay, box_current_day_tb),
    XtOffset( CalendarDisplay, db_print_dg),
    XtOffset( CalendarDisplay, db_open_new),
    XtOffset( CalendarDisplay, wrapper),
    XtOffset( CalendarDisplay, pb_gen_controls),
    XtOffset( CalendarDisplay, pb_gen_dummy1),
    XtOffset( CalendarDisplay, pb_gen_dummy2),
    XtOffset( CalendarDisplay, pb_gen_dummy3),
    XtOffset( CalendarDisplay, pb_gen_dummy4),
    XtOffset( CalendarDisplay, pb_icon_controls),
    XtOffset( CalendarDisplay, pb_icon_dummy1),
    XtOffset( CalendarDisplay, pb_icon_dummy2),
    XtOffset( CalendarDisplay, pb_icon_dummy3),
    XtOffset( CalendarDisplay, pb_icon_dummy4),
    XtOffset( CalendarDisplay, pb_alarm_controls),
    XtOffset( CalendarDisplay, pb_alarm_dummy1),
    XtOffset( CalendarDisplay, pb_alarm_dummy2),
    XtOffset( CalendarDisplay, pb_alarm_dummy3),
    XtOffset( CalendarDisplay, pb_alarm_dummy4),
    XtOffset( CalendarDisplay, pb_day_controls),
    XtOffset( CalendarDisplay, pb_day_dummy1),
    XtOffset( CalendarDisplay, pb_day_dummy2),
    XtOffset( CalendarDisplay, pb_day_dummy3),
    XtOffset( CalendarDisplay, fm_nameas_controls)
};

Widget MISCFetchWidget
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cb_data,
	Boolean			manage_it)
#else	/* no prototypes */
	(w, tag, cb_data, manage_it)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cb_data;
	Boolean			manage_it;
#endif	/* prototype */
{
    char *name = (char *)tag;
    CalendarDisplay cd;
    MrmType	class;
    int		status;
    Widget	*related_dialog_ptr;
    int		button_index, dialog_index;
    Widget	temp_widget;
    
/* If the indirection table has not be initialized, initialize	    */
/* it.  This ought to be doable at compile/link-time, but I don't   */
/* know how to do it.						    */
/*								    */

    if (!related_dialog_init)
    {
	memset( related_dialog_table, 0, sizeof(int) * K_CALENDAR_WIDGET_COUNT );
	related_dialog_table[ k_open_pb ] = k_open_db;
	related_dialog_table[ k_open_new_pb ] = k_open_new_db;
	related_dialog_table[ k_nameas_pb ] = k_nameas_db;
	related_dialog_table[ k_delete_pb ] = k_delete_cb;
	related_dialog_table[ k_print_pb ] = k_print_db;
	related_dialog_table[ k_include_pb ] = k_include_db;
	related_dialog_table[ k_general_pb ] =	k_general_db;
	related_dialog_table[ k_dayview_pb ] =	k_dayview_db;
	related_dialog_table[ k_iconbox_pb ] =	k_iconbox_db;
	related_dialog_table[ k_alarms_pb ] =	k_alarms_db;
	related_dialog_init = 1;
    };

/*									    */
/* First, find the calendar display structure that this widget		    */
/* belong to.  Most likely, w is the widget id of a pushbutton in a menu.   */
/*									    */

    status = MISCFindCalendarDisplay( &cd, w );

/* Find the index of this widget in the index to offset table */

    button_index = find_index( cd, w );

/* Use the index of w (a pushbutton) to find a place to store the	    */
/* index of thing we are fetching (probably a file selection box ).  Go	    */
/* indirectly through an offset table */

    dialog_index = related_dialog_table[button_index];
    if (dialog_index == 0)
    {
	related_dialog_ptr = &temp_widget;
	temp_widget = NULL;
    }
    else
	related_dialog_ptr =
	    (Widget *)((char *)cd + offset_table[dialog_index]);


    status = MrmFetchWidget( cd->ads->hierarchy,
		name,
		cd->toplevel,
		related_dialog_ptr,
		&class );

    if (status != MrmSUCCESS)
	DWC$UI_Catchall(DWC$DRM_FETCHMANAGE, status, 0);

    /*	  
    **  If we've been asked to manage it then do it, else just realize it. Why
    **	do we do this? Well, so that the fitformdialog stuff will work without
    **	flicker.
    */	  
    if (manage_it)
    {
	XtManageChild( *related_dialog_ptr );
    }    
    else
    {
	XtRealizeWidget( *related_dialog_ptr);
    }

    return *related_dialog_ptr;
}

void
MISCFetchModal
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cb_data)
#else	/* no prototypes */
	(w, tag, cb_data)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cb_data;
#endif	/* prototype */
{
    Widget  widget;

    /*	  
    **  We want to mange it
    */	  
    widget = MISCFetchWidget( w, tag, cb_data, TRUE );

    ICONSInactiveCursorDisplay( widget, ads.inactive_cursor );

    return;
}

int
find_index
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Widget		w)
#else	/* no prototypes */
	(cd, w)
	CalendarDisplay	cd;
	Widget		w;
#endif	/* prototype */
{
    Widget *widget_ptr;
    int i;
    
    for(i = 0; i < K_CALENDAR_WIDGET_COUNT; i++)
    {
	if (offset_table[i] <= sizeof( *cd ))
	{
	    widget_ptr = (Widget *)((char *)cd +  offset_table[i]);
	    if (*widget_ptr ==  w) return ( i );
	};
    }

    printf("widget %08X not found\n", w);
    return ( -1 );
}

void
MISCGenericCreateProc
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	int			*tag,
	XmAnyCallbackStruct	*callback_data)
#else	/* no prototypes */
	(widget, tag, callback_data)
	Widget			widget;
	int			*tag;
	XmAnyCallbackStruct	*callback_data;
#endif	/* prototype */
{
    CalendarDisplay cd;
    int		    status;
    Widget	    *widget_ptr;

    /*
    ** Assert that the offset table has enough stuff in it
    */
    assert (XtNumber(offset_table) == K_CALENDAR_WIDGET_COUNT);

    /*
    ** Which Calendar Display? 
    */
    status = MISCFindCalendarDisplay(&cd, widget);
    
    widget_ptr = (Widget *)((char *)cd + offset_table[ *tag ]);
    *widget_ptr = widget;

    return;
}

/* This function changes the look of the time pushbutton at the bottom of
** the DayView.  When there is no shadow around it then autoscroll is on
** otherwise autoscroll is off
*/
void MISCSetAutoScroll
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Boolean		onoff)
#else	/* no prototypes */
	(cd, onoff)
	CalendarDisplay	cd;
	Boolean		onoff;
#endif	/* prototype */
{
    short	    shadowthickness;

    if (cd->auto_scroll_day == onoff)
    {
	return;
    }
    
    XtUnmanageChild (cd->pb_day_time);

    /* the default XmNshadow Thickness is 2 */

    /*
    ** The should really use XtGetSubresources to get the basic width and
    ** then reapply it.
    */
    XtVaSetValues (cd->pb_day_time, XmNshadowThickness, (!onoff)*2, NULL);

    XtManageChild   (cd->pb_day_time);

    cd->auto_scroll_day = onoff;

}

static void ErrorZeroTime
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
    CalendarDisplay	cd = (CalendarDisplay) tag;

    XtUnmanageChild (w);
    cd->zero_time_up = FALSE;
    
}

void MISCChangeView
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	show_kind	view,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year)
#else	/* no prototypes */
	(cd, view, day, month, year)
	CalendarDisplay	cd;
	show_kind	view;
	Cardinal	day;
	Cardinal	month;
	Cardinal	year;
#endif	/* prototype */
{
    Cardinal	    md_msbot;
    Cardinal	    dd_msbot;
    MwTypeSelected  s_type;
    Cardinal	    s_day;
    Cardinal	    s_week;
    Cardinal	    s_month;
    Cardinal	    s_year;
    Cardinal	    value;
    Widget	    view_w;
    Dimension	    main_width;
    Dimension	    main_height;
    Dimension	    profile_format_width;
    Dimension	    profile_format_height;
    Position	    profile_format_x;
    Position	    profile_format_y;
    Position	    main_x;
    Position	    main_y;
    Position	    old_x;
    Position	    old_y;
    Boolean	    work_day;
    unsigned char   entries;
    unsigned char   marks;
    unsigned char   style;
    struct tm	    *local_time;
    Boolean	    first_time = True;
    Boolean	    get_surrogates = True;
    Widget	    trav_month;

    if (cd->active_widget != NULL) first_time = False;
    /*	  
    **  First order of business is to attempt to close any open entries we may
    **	have. If we don't do this first, the entries will end up on the day that
    **	we've been asked to change to (since they take their dsbot from
    **	cd->dsbot which will have been changed).
    */
    if (cd->dayslots.widget != NULL)
    {
	if (! DSWRequestCloseOpenEntry
	    (
		(DayslotsWidget) cd->dayslots.widget,
		MISCGetTimeFromEvent((XEvent *)NULL)
	    )
	   )
	{
	    return;
	}
	if (! DSWRequestCloseOpenEntry
	    (
		(DayslotsWidget) cd->daynotes.widget,
		MISCGetTimeFromEvent((XEvent *)NULL)
	    )
	   )
	{
	    return;
	}
    }

    /*	  
    **  Get the local internal time
    */	  
    MISCGetTime(&local_time);

    /*	  
    **  Make sure we're not too early.
    */	  
    if (year < 1600)
    {
	if (! cd->zero_time_up)
	{
	    ERRORReportError
	    (
		cd->mainwid,
		"ErrorZeroTime",
		(XtCallbackProc)ErrorZeroTime,
		(char *)cd
	    );
	    cd->zero_time_up = TRUE;
	}
	return;
    }


    /*	  
    **  Save the date that we'll be looking at
    */	  
    cd->day   = day;
    cd->month = month;
    cd->year  = year;
    cd->dsbot = DATEFUNCDaysSinceBeginOfTime (day, month, year);



    /*	  
    **  If we're already realized, tell the user we'll be waiting and deselect
    **	selected entries since we're changing views.
    */	  
    if (XtIsRealized (cd->mainwid))
    {
	ICONSWaitCursorDisplay(cd->mainwid, cd->ads->wait_cursor);
	CLIPDeselectAllEntries (cd, MISCGetTimeFromEvent((XEvent *)NULL));
    }
    
    /*	  
    **  What does the monthwidget think is selected?
    */	  
    MWGetSelection(cd->month_display, &s_type, &s_day, &s_week,
		     &s_month, &s_year);

    if ((s_type != MwDaySelected) && (s_type != MwNothingSelected))
    {
	if (cd->last_day == 0)
	{
	    /*	  
	    **  We must be starting up.
	    */	  
    	    s_type = MwNothingSelected;
	}
	else
	{
	    s_type = MwDaySelected;
	    s_day   = cd->last_day;
	    s_month = cd->last_month;
	    s_year  = cd->last_year;
	}
    }

    /*	  
    **  Set up some things depending upon the view we're being asked to show
    **	next. 
    */	  
    switch (view)
    {
    case show_day:
	view_w	    = (Widget) cd->lw_day_display;
	profile_format_width  = (Dimension)cd->profile.day_width;
	profile_format_height = (Dimension)cd->profile.day_height;
	profile_format_x      = cd->profile.day_x;
	profile_format_y      = cd->profile.day_y;

	s_type = MwDaySelected;
	s_day = day;
	s_month = month;
	s_year = year;

	cd->last_day = day;
	cd->last_month = month;
	cd->last_year = year;
	
	/*	  
	**  Get marks for this day.
	*/	  
	if (DWC$DB_Get_day_attr (cd->cab, cd->dsbot, &marks, &style,
				     &entries) != DWC$k_db_normal)
	{
	    ERRORDisplayError (cd->mainwid, "ErrorDayAttr");
	    work_day = TRUE;
	}
	else
	{
	    marks = marks & ~(DWC$m_day_special | DWC$m_day_defaulted);
	    work_day = (marks == DWC$k_day_workday);
	}

	/*	  
	**  Set work hours
	*/
	if (work_day)
	{
	    DSWSetWorkHours
	    (
		(DayslotsWidget) cd->dayslots.widget,
		cd->profile.work_minutes_start,
		cd->profile.work_minutes_end
	    );
	}
	else
	{
	    DSWSetWorkHours ((DayslotsWidget) cd->dayslots.widget, 0, 0);
	}

	/*	  
	**  Go get all the items from the db and create them.
	*/	  
	DAYGetDayItems(cd, day, month, year);

	/*	  
	**  Get the right name for the day label
	*/	  
	DAYSetupDayNameWidget(cd->day_label, day, month, year);

	md_msbot = (cd->day_yd->first_year * 12) + cd->day_yd->first_month - 1;
	dd_msbot = (year * 12) + month - 1;

	/*	  
	**	See if what is currently stored in the YearDisplay structure is
	**	the same as what we're being asked to display.
	*/	  
	if (dd_msbot < md_msbot)
	{
	    /*	  
	    **  Go back in time.
	    */	  
	    YEARSetYearDisplayDate (cd->day_yd, month, year);
	}
	else
	{ 
	    if (dd_msbot >= md_msbot + cd->day_yd->columns)
	    {
		/*	  
		**  The month we're being asked to display isn't in the line of
		**  months that are currently being display. So figure out the
		**  month that should be displayed as the first month of the
		**  column in order for our desired month to be displayed.
		*/	  
		md_msbot = dd_msbot - cd->day_yd->columns + 1;
		YEARSetYearDisplayDate (cd->day_yd, (md_msbot % 12) + 1,
						     (md_msbot - 1) / 12);
	    }
	}

	/*	  
	**	Under some circumstances we're not getting the autoscroll when
	**	we go from the month view to the day view for the first time.
	**	I'm not yet sure why. PGLF 5/25/90
	*/	  
	if ((! cd->profile.auto_click_on_clock)   ||
	    (day   != local_time->tm_mday) ||
	    (month != local_time->tm_mon)  ||
	    (year  != local_time->tm_year))
	{
	    MISCSetAutoScroll(cd, FALSE);
	}
	else
	{
	    /*	  
	    **  Update the dayview and turn on autoscroll
	    */	  
	    MISCDayClock(NULL, (caddr_t) cd, NULL);
	}

	break;

    case show_week  :
	profile_format_width  = (Dimension)cd->profile.week_width;
	profile_format_height = (Dimension)cd->profile.week_height;
	profile_format_x      = cd->profile.week_x;
	profile_format_y      = cd->profile.week_y;
	return;
	break;

    case show_month :
	if (cd->lw_month_display == NULL)
	{
	    MONTHCreateMonthDisplay (cd);
	}

	view_w	    = (Widget) cd->lw_month_display;
	profile_format_width  = (Dimension)cd->profile.month_width;
	profile_format_height = (Dimension)cd->profile.month_height;
	profile_format_x      = cd->profile.month_x;
	profile_format_y      = cd->profile.month_y;

	MWSetDate(cd->month_display, day, month, year);
	/*	  
	**  The value should range from 1 to 52
	*/	  
	value = (DATEFUNCDaysSinceBeginOfYear(day, month, year) / 7) + 1;
	XmScrollBarSetValues (cd->month_scrollbar, value, 6, 1, 1, FALSE);

	break;

    case show_year  :
	if (cd->yd == NULL)
	{
	    CALCreateYearDisplay (cd);
	}

	view_w      = (Widget) cd->yd->layout;
	profile_format_width  = (Dimension)cd->profile.year_width;
	profile_format_height = (Dimension)cd->profile.year_height;
	profile_format_x      = cd->profile.year_x;
	profile_format_y      = cd->profile.year_y;

	YEARSetYearDisplayDate(cd->yd, month, year);

	trav_month = YEARGetMonthWidget(cd->yd, month);

	break;

    }

    /*	  
    **  We've decided what we'll be looking at so tell the month widget
    */	  
    MWSetSelection(cd->month_display, s_type, s_day, s_week, s_month, s_year);
    

    /*	  
    **  If we're being asked to change the view to a view different than the one
    **	we're looking at, then we don't want things mapped when managed.
    */	  
    if (cd->active_widget != view_w)
    {

	XtSetMappedWhenManaged (cd->mainwid, False);

	/*
	** Try to push input to the right place.
	*/
	switch (view)
	{
	case show_day:
	    MISCFocusOnMap ((Widget)cd->mainwid, (Widget)cd->pb_day_time);
	    break;

	case show_month:
	    MISCFocusOnMap ((Widget)cd->mainwid, (Widget)cd->month_display);
	    break;

	case show_year:
	    MISCFocusOnMap ((Widget)cd->mainwid, (Widget)trav_month);
	    break;
	}

        /*	  
	**  If we have an active view, unmanage it while we tweak things.
	*/
        if (cd->active_widget != NULL)
	{
	    XtUnmanageChild (cd->active_widget);
	}


        /*	  
	**  Only have the Entry menu item if we're on the day view
	*/	  
	if (cd->pull_entry != NULL)
	{
	    if (view == show_day)
		XtManageChild(cd->pull_entry);
	    else
		XtUnmanageChild(cd->pull_entry);
	}	    

        /*	  
	**  If we already have a view up, then grab the existing geometry since
	**  the user may have moved things around and we want to keep it for the
	**  next time they show that view.
	*/	  
	save_current_view_xy(cd);

	XtManageChild(view_w);

        /*	  
	**  Plug it into the mainwindow
	*/	  
        XmMainWindowSetAreas(cd->mainwid, cd->menubar, NULL, NULL, NULL,view_w);

        /*	  
        **  Convert from pixels to our internal format
        */	  
/* debugging info to catch layout resize notification problem
printf("\tMISCChangeView GetScreenPixelPosition width: %d height: %d\n",
	    profile_format_width, profile_format_height);
*/
	MISCGetScreenPixelPosition (cd, profile_format_x, profile_format_y,
				    profile_format_width, profile_format_height,
				    &main_width, &main_height,
				    &main_x, &main_y);
	
/* debugging info to catch layout resize notification problem
    printf("\tMISCChangeView setting mainwindow width: %d height: %d\n",
	    main_width, main_height);
*/
	XtVaSetValues
	(
	    cd->mainwid,
	    XmNwidth,  main_width,
	    XmNheight, main_height,
	    NULL
	);

/* debugging info to catch layout resize notification problem
    printf("\tMISCChangeView set done mainwindow width: %d height: %d\n",
	    main_width, main_height);
*/
        /*	  
	**  Move the toplevel window to where it should be based upon the stuff
	**  that the user saved in the profile.
	*/  
	XtVaSetValues
	(
	    cd->toplevel,
	    XmNx, main_x,
	    XmNy, main_y,
	    XmNheight, main_height,
	    XmNwidth, main_width,
	    NULL
	);


        /*	  
	**  Keep track of what is now become the active view.
	*/	  
        cd->active_widget = view_w;
	cd->showing = view;

	if (view == show_day)
	{
	    if ((cd->profile.auto_click_on_clock)     &&
		(day   == local_time->tm_mday) &&
		(month == local_time->tm_mon)  &&
		(year  == local_time->tm_year))
		{
		MISCDayClock (NULL, (caddr_t) cd, NULL);
	    }
	}

	XtSetMappedWhenManaged (cd->mainwid, True);
    }

#if MEMEX
    if (first_time && (cd->profile.first_display != show_day))
	get_surrogates = False;

    if ((view == show_day) && get_surrogates)
    {
	/*
	**  If we were successful in creating the his dw ui (done in
	**	MEMEXCreateDwUi) then go highlight any surrogates on this day
	*/
	if (cd->hisdwui != (lwk_dxm_ui)0)
	    MEMEXSurrogateHighlightForDay(cd);
    }
#endif	/* MEMEX */

    if (XtIsRealized (cd->mainwid))
    {
	ICONSWaitCursorRemove (cd->mainwid);
    }

}

void MISCDayClock
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    Time		event_time;
    int			time;
    CalendarDisplay	cd  = (CalendarDisplay) tag;
    Widget		dsw = cd->dayslots.widget;
    struct tm		*local_time;

    /*	  
    **  If we're already autoscrolling and we're here as a result of the
    **	day_clock callback then don't bother doing autoscrolling again.
    */	  
    if (cd->auto_scroll_day && (w != NULL))
    {
	return;
    }
    
    event_time = MISCGetTimeFromAnyCBS (cbs);
    
    /*	  
    **  Get the local internal time
    */	  
    MISCGetTime(&local_time);

    if ((local_time->tm_mday != cd->day)   ||
        (local_time->tm_mon  != cd->month) ||
        (local_time->tm_year != cd->year))
    {
        /*	  
        **  If we can't close any entry that is open in the dayslots widget then
	**  forget trying to update things. Why aren't we also concerned about
	**  the daynotes widget having an open entry?
        */	  
	if ((dsw != NULL) &&
	    (! DSWRequestCloseOpenEntry ((DayslotsWidget) dsw, event_time)))
	{
	    return;
	}

	MISCChangeView(cd, show_day, local_time->tm_mday,
		     local_time->tm_mon, local_time->tm_year);
    }

    if (dsw != NULL)
    {
	time = (local_time->tm_hour * 60) + local_time->tm_min;
	DSWMoveDayslotsToTime ((DayslotsWidget) dsw, time);
    }

    MISCSetAutoScroll (cd, TRUE);

}

Boolean MISCTestDayConditions
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Boolean		workday,
	Boolean		has_entries,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year)
#else	/* no prototypes */
	(cd, workday, has_entries, day, month, year)
	CalendarDisplay	cd;
	Boolean		workday;
	Boolean		has_entries;
	Cardinal	day;
	Cardinal	month;
	Cardinal	year;
#endif	/* prototype */
{
    Cardinal		dsbot;
    unsigned char	entries;
    unsigned char	marks;
    unsigned char	style;


    dsbot = DATEFUNCDaysSinceBeginOfTime (day, month, year);
    if (DWC$DB_Get_day_attr (cd->cab, dsbot, &marks, &style,
				 &entries) != DWC$k_db_normal)
    {
	ERRORDisplayError (cd->mainwid, "ErrorDayAttr");
	return (FALSE);
    }

    if ((entries != DWC$k_use_empty) && has_entries)
    {
	return (TRUE); /* Found a day with an entry */
    }

    if (workday)
    {
	marks = marks & ~(DWC$m_day_special | DWC$m_day_defaulted);
	return (marks == DWC$k_day_workday);
    }

    return (FALSE); /* Didn't meet either condition */

}

 Boolean
MISCFirstDOWWithCondition
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Cardinal	week,
	Cardinal	fiscal_year,
	Boolean		workday,
	Boolean		entries,
	Cardinal	*day,
	Cardinal	*month,
	Cardinal	*year)
#else	/* no prototypes */
	(cd, week, fiscal_year, workday, entries, day, month, year)
	CalendarDisplay	cd;
	Cardinal	week;
	Cardinal	fiscal_year;
	Boolean		workday;
	Boolean		entries;
	Cardinal	*day;
	Cardinal	*month;
	Cardinal	*year;
#endif	/* prototype */
{
    Cardinal		dsbot;
    Cardinal		i;
    Cardinal		tday, tmonth, tyear;

    DATEFUNCStartDateForWeekNo
    (
	week,
	fiscal_year,
	cd->profile.first_day_of_week,
	cd->profile.week_numbers_start_day,
	cd->profile.week_numbers_start_month,
	(int *)day,
	(int *)month,
	(int *)year
    );
    tday   = *day;
    tmonth = *month;
    tyear  = *year;

    if ((! workday) && (! entries))
    {
	return (FALSE);
    }
    
    dsbot = DATEFUNCDaysSinceBeginOfTime (tday, tmonth, tyear);
    for (i = 0;  i < 7;  i++)
    {
	if (MISCTestDayConditions (cd, workday, entries, tday, tmonth, tyear))
	{
	    *day   = tday;
	    *month = tmonth;
	    *year  = tyear;
	    return (TRUE);
	}
	dsbot++;
	DATEFUNCDateForDayNumber
	    (dsbot, (int *)&tday, (int *)&tmonth, (int *)&tyear);
    }

    return (FALSE);
}


Boolean MISCTestRepeatOpen
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    DSWEntryDataStruct   data;
    DwcDswEntry		entry;
    DwcDayItem		di;


    if (cd->dayslots.widget == NULL) return (FALSE);

    entry = DSWGetOpenEntry((DayslotsWidget) cd->dayslots.widget);
    if (entry == NULL)
    {
	entry = DSWGetOpenEntry((DayslotsWidget) cd->daynotes.widget);
	if (entry == NULL)
	{
	    return (FALSE);
	}
    }

    DSWGetEntryData(entry, &data);
    di = (DwcDayItem) data.tag;
    if (di == NULL)
    {
	return (FALSE);
    }
    else
    {
	return (di->repeat_p1 != DWC$k_db_none);
    }

}

Boolean MISCIncludeFileIntoText
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	char		*file_spec,
	DXmCSTextWidget	tw)
#else	/* no prototypes */
	(cd, file_spec, tw)
	CalendarDisplay	cd;
	char		*file_spec;
	DXmCSTextWidget	tw;
#endif	/* prototype */
{
    struct DWC$db_interchange   *work_context;
    int			err_line;
    int			status;
    Cardinal		insert_point;
#ifndef VMS
    int			vaxc$errno;
#endif
    XmString		xm_texts;


    status = DWCDB_LoadInterchange
	(cd->cab, file_spec, &xm_texts, (int *)&errno, (int *)&vaxc$errno);
    if (status != DWC$k_db_normal)
    {
	return (k_include_fail);
    }

    if (XmStringEmpty (xm_texts))
    {
	XmStringFree (xm_texts);
	return (k_include_empty);
    };

    XtVaGetValues ((Widget)tw, XmNcursorPosition, &insert_point, NULL);

    status = DWCDB_ParseInterchange
	(cd->cab, xm_texts, &work_context, &err_line);
    if (status == DWC$k_db_normal)
    {
	XmStringFree (xm_texts);
	DWCDB_GetAllITexts (cd->cab, work_context, &xm_texts);
	DWCDB_RundownInterchange (cd->cab, work_context);
	DXmCSTextReplace ((Widget)tw, insert_point, insert_point, xm_texts);
    }
    else
    {
	DXmCSTextReplace ((Widget)tw, insert_point, insert_point, xm_texts);
    }

    XmStringFree (xm_texts);

    return (k_include_success);

}

char *MISCGetTextFromCS
#if defined(_DWC_PROTO_)
	(
	XmString	cs)
#else	/* no prototypes */
	(cs)
	XmString	cs;
#endif	/* prototype */
{
    long int	length;
    long int	status;
    
    return (DXmCvtCStoFC(cs, &length, &status));
}

void MISCRaiseToTheTop
#if defined(_DWC_PROTO_)
	(
	Widget	widget)
#else	/* no prototypes */
	(widget)
	Widget	widget;
#endif	/* prototype */
{
    if (XtIsManaged(widget))
    {
	XRaiseWindow (XtDisplay(widget), XtWindow(XtParent(widget)));
    }

    /*
    **  That's it
    */
    return;
}

int
MISCFindCalendarDisplay
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	*cd,
	Widget		widget)
#else	/* no prototypes */
	(cd, widget)
	CalendarDisplay	*cd;
	Widget		widget;
#endif	/* prototype */
{
    int	   index;
    Widget son = NULL;
    
    /*
    **  This is a bit of a hack... We go up the widget run-time hierarchy until
    **	we find the toplevel widget.
    */

    do
    {
	son = widget;
	widget = XtParent(widget);
    }
    while ( widget != 0 );

    /*
    **	'son' now holds the application shell widget. We go thru all views
    **	to find the corresponding view...
    */
    
    for ( index=0; index<ads.number_of_calendars; index++)
    {
	if (ads.cds[index]->toplevel == son)
	{
	    *cd = ads.cds[index];
	    return TRUE;
	}
    }    

#if defined(DEBUG)
    printf("Couldn't find Calendar display\n");
#endif
    /*
    **  That's it
    */
    return FALSE;
}

int MISCFetchIconLiteral_1
#if defined(_DWC_PROTO_)
(
    MrmHierarchy	hierarchy_id,
    String		index,
    Screen		*screen,
    Display		*display,
    Pixmap		*pixmap,
    unsigned int	width,
    unsigned int	height
)
#else
(hierarchy_id, index, screen, display, pixmap, width, height)
MrmHierarchy	hierarchy_id;
String		index;
Screen		*screen;
Display		*display;
Pixmap		*pixmap;
unsigned int	width;
unsigned int	height;
#endif
{
    Dimension		t_width, t_height;
    Pixmap		temp;
    Pixmap		real;
    Window		root = RootWindowOfScreen (screen);
    unsigned int	depth;
    GC			gc;
    static const unsigned long mask =
	GCForeground | GCBackground | GCGraphicsExposures;
    XGCValues		gcv;
    int			status;
    
#ifndef nofetch
    status = MrmFetchBitmapLiteral
	(hierarchy_id, index, screen, display, &temp, &t_width, &t_height);
    if (status != MrmSUCCESS)
    {
	*pixmap = (Pixmap)0;
    }
    else
    {
	*pixmap = temp;
    }
#else
    status = MrmFetchIconLiteral
	(hierarchy_id, index, screen, display, 1, 0, &temp);
    if (status != MrmSUCCESS) return (status);

    real = XCreatePixmap (display, root, width, height, 1);

    gcv.foreground = 1;
    gcv.background = 0;
    gcv.graphics_exposures = False;
    gc = XCreateGC (display, real, mask, &gcv);

    XCopyPlane (display, temp, real, gc, 0, 0, width, height, 0, 0, 1);

    XFreePixmap (display, temp);
    XFreeGC (display, gc);

    *pixmap = real;
#endif
    return (status);
}

void MISCLoadPixmapArrays
#if defined(_DWC_PROTO_)
	(
	Widget		parent,
	unsigned char	size)
#else	/* no prototypes */
	(parent, size)
	Widget	parent;
	unsigned char	size;
#endif	/* prototype */
{
    MrmType	    class;
    XmLabelWidget	    widget;
    int		    status;
    Pixel	    foreground;
    Pixel	    background;
    static struct
    {
	char		*name;
	int		name_len;
	unsigned char	index;
    } pixmap_array[] =
    {
	{ "meeting",		7,		k_pixmap_meeting},
	{ "faces",		5,		k_pixmap_faces},
	{ "slides",		6,		k_pixmap_slides},
	{ "announce",		8,		k_pixmap_announce},
	{ "car",		3,		k_pixmap_car},
	{ "plane",		5,		k_pixmap_plane},
	{ "phone",		5,		k_pixmap_phone},
	{ "letter",		6,		k_pixmap_letter},
	{ "pencil",		6,		k_pixmap_pencil},
	{ "note",		4,		k_pixmap_note},
	{ "milestone",		9,		k_pixmap_milestone},
	{ "leave",		5,		k_pixmap_leave},
	{ "coffee",		6,		k_pixmap_coffee},
	{ "lunch",		5,		k_pixmap_lunch},
	{ "doctor",		6,		k_pixmap_doctor},
	{ "money",		5,		k_pixmap_money},
	{ "info",		4,		k_pixmap_info},
	{ "exclam",		6,		k_pixmap_exclam},
	{ "question",		8,		k_pixmap_question},
	{ "cake",		4,		k_pixmap_cake},
	{ "flag",		4,		k_pixmap_flag},
	{ "saints",		6,		k_pixmap_saints},
	{ "school",		6,		k_pixmap_school},
	{ "games",		5,		k_pixmap_games},

	{ "memex",		5,		k_pixmap_memex},
	{ "bell",		4,		k_pixmap_bell},
	{ "repeat",		6,		k_pixmap_repeat},
	{ "repeatstart",	11,		k_pixmap_repeatstart},
	{ "repeatend",		9,		k_pixmap_repeatend}
    };
	
    static char	    small	    [] = "_12X12";
    static char	    medium	    [] = "_17X17";
    static char	    big		    [] = "_32X32";
    static char	    *size_name;
    int		    pixmap_count = XtNumber(pixmap_array);
    char	    working_string[30];	/* Make sure this array is big	    */
    int		    i;
    Pixmap	    *pixmaps;
    unsigned int    width, height;
    int		    pixmap_name_len;
    int		    size_name_len = 6;  /* small, medium, large above */

    /*
    ** Select the size to go with.
    */
    switch (size)
    {
    case 0:
	pixmaps = small_pixmaps;
	size_name = small;
	width = height = 12;
	break;
    case 1:
	pixmaps = medium_pixmaps;
	size_name = medium;
	width = height = 17;
	break;
    case 2:
	pixmaps = big_pixmaps;
	size_name = big;
	width = height = 32;
	break;
    }

    /*
    ** If already done, return.
    */
    if (pixmaps[0] != 0) return;

    XtVaGetValues
	(parent, XmNbackground, &background, XmNforeground, &foreground, NULL);
    
    /*
    **	!!! pixmap_count shouldn't be modified, and should be equal to the
    **	value defined in DWC_UI_CALENDAR.H (k_pixmap_count).
    */
    for (i=0; i<pixmap_count; i++)
    {
	pixmap_name_len = pixmap_array[i].name_len;
	memcpy(working_string, pixmap_array[i].name, pixmap_name_len);
	memcpy(&working_string[pixmap_name_len], size_name, size_name_len + 1);

	status = MISCFetchIconLiteral_1
	(
	    ads.hierarchy,
	    working_string,
	    XtScreen(parent),
	    XtDisplay(parent),
	    &pixmaps[pixmap_array[i].index],
	    width,
	    height
	);

	if (status != MrmSUCCESS)
	    DWC$UI_Catchall (DWC$DRM_FETCHICON, status, 0);

    }

    /*
    ** Only get these the first time.
    */
    if (dwc_bell_negative_32X32 != 0) return;

    /*
    **	We need to get two other icons: the negative bells, 32X32 and 12X12
    */
    status = MrmFetchIconLiteral
    (
	ads.hierarchy,
	"bell_negative_32X32",
	XtScreen(parent),
	XtDisplay(parent),
	foreground,
	background,
	&dwc_bell_negative_32X32
    );

    if (status != MrmSUCCESS)
	DWC$UI_Catchall(DWC$DRM_FETCHICON, status, 0);

    status = MrmFetchIconLiteral
    (
	ads.hierarchy,
	"bell_negative_12X12",
	XtScreen(parent),
	XtDisplay(parent),
	foreground,
	background,
	&dwc_bell_negative_12X12
    );

    if (status != MrmSUCCESS)
	DWC$UI_Catchall(DWC$DRM_FETCHICON, status, 0);

    /*
    **	That's it
    */
    return;
}



struct int_to_name_entry
{
    char *value_name;
    XmString value;
};

static struct int_to_name_entry value_table [] =
{
    {"dwc_t_abr_day_names", 0 },
    {"dwc_t_alarm_format", 0 },
    {"dwc_t_alarm_from_ampm_format", 0 },
    {"dwc_t_alarm_from_format", 0 },
    {"dwc_t_alarm_icon_date_format", 0 },
    {"dwc_t_alarm_icon_now_format", 0 },
    {"dwc_t_alarm_icon_tmrow_format", 0 },
    {"dwc_t_alarm_title_ampm_format", 0 },
    {"dwc_t_alarm_title_format", 0 },
    {"dwc_t_alarm_to_ampm_format", 0 },
    {"dwc_t_alarm_to_format", 0 },
    {"dwc_t_am_pm_texts", 0 },
    {"dwc_t_clip_entry_date_format", 0 },
    {"dwc_t_clip_from_ampm_format", 0 },
    {"dwc_t_clip_from_format", 0 },
    {"dwc_t_clip_note_date_format", 0 },
    {"dwc_t_clip_to_ampm_format", 0 },
    {"dwc_t_clip_to_format", 0 },
    {"dwc_t_cust_ampm_format", 0 },
    {"dwc_t_cust_hour_ampm_format", 0 },
    {"dwc_t_cust_hour_format", 0 },
    {"dwc_t_cust_min_format", 0 },
    {"dwc_t_day_dayname_format", 0 },
    {"dwc_t_day_pb_ampm_format", 0 },
    {"dwc_t_day_pb_format", 0 },
    {"dwc_t_icon_day_format", 0 },
    {"dwc_t_icon_dayname_full_format", 0 },
    {"dwc_t_icon_dayname_short_format", 0 },
    {"dwc_t_icon_month_format", 0 },
    {"dwc_t_icon_time_ampm_format", 0 },
    {"dwc_t_icon_time_format", 0 },
    {"dwc_t_long_day_names", 0 },
    {"dwc_t_long_month_names", 0 },
    {"dwc_t_long_week_text", 0 },
    {"dwc_t_mw_ditto_format", 0 },
    {"dwc_t_mw_month_format", 0 },
    {"dwc_t_mw_month_sep_format", 0 },
    {"dwc_t_mw_separator_format", 0 },
    {"dwc_t_mw_week_heading_format", 0 },
    {"dwc_t_mw_weekname_format", 0 },
    {"dwc_t_mw_year_format", 0 },
    {"dwc_t_print_dash_from_ampm_fmt", 0 },
    {"dwc_t_print_dash_from_fmt", 0 },
    {"dwc_t_print_dash_to_ampm_fmt", 0 },
    {"dwc_t_print_dash_to_fmt", 0 },
    {"dwc_t_print_from_ampm_format", 0 },
    {"dwc_t_print_from_format", 0 },
    {"dwc_t_print_full_date_format", 0 },
    {"dwc_t_print_time_ampm_format", 0 },
    {"dwc_t_print_time_format", 0 },
    {"dwc_t_print_to_ampm_format", 0 },
    {"dwc_t_print_to_format", 0 },
    {"dwc_t_short_day_names", 0 },
    {"dwc_t_short_month_names", 0 },
    {"dwc_t_short_week_text", 0 },
    {"dwc_t_century_suffix_texts", 0 },
    {"dwc_t_timeslots_12hr_format", 0 },
    {"dwc_t_timeslots_24hr_format", 0 },
    {"dwc_t_timeslots_mins_format", 0 },
    {"dwc_t_ts_date_format", 0 },
    {"dwc_t_ts_first_x_after_format", 0 },
    {"dwc_t_ts_first_x_before_format", 0 },
    {"dwc_t_ts_from_ampm_format", 0 },
    {"dwc_t_ts_from_format", 0 },
    {"dwc_t_ts_last_day_format", 0 },
    {"dwc_t_ts_last_weekday_format", 0 },
    {"dwc_t_ts_long_ampm_format", 0 },
    {"dwc_t_ts_long_format", 0 },
    {"dwc_t_ts_long_scale_title", 0 },
    {"dwc_t_ts_medium_ampm_format", 0 },
    {"dwc_t_ts_medium_format", 0 },
    {"dwc_t_ts_medium_scale_title", 0 },
    {"dwc_t_ts_nth_day_format", 0 },
    {"dwc_t_ts_days_month_end_format", 0 },
    {"dwc_t_ts_day_month_end_format", 0 },
    {"dwc_t_ts_nth_weekday_format", 0 },
    {"dwc_t_ts_short_ampm_format", 0 },
    {"dwc_t_ts_short_format", 0 },
    {"dwc_t_ts_short_scale_title", 0 },
    {"dwc_t_ts_spec_day_after_format", 0 },
    {"dwc_t_ts_spec_day_before_format", 0 },
    {"dwc_t_ts_the_day_at_or_format", 0 },
    {"dwc_t_ts_to_ampm_format", 0 },
    {"dwc_t_ts_to_format", 0 },
    {"dwc_t_ts_vlong_ampm_format", 0 },
    {"dwc_t_ts_vlong_format", 0 },
    {"dwc_t_ts_vlong_scale_title", 0 },
    {"dwc_t_prev_pb_text", 0 },
    {"dwc_t_next_pb_text", 0 },
    {"dwc_k_pw_week_starting", 0},
    {"dwc_t_calendar_title_text", 0},
    {"dwc_t_help_month_none", 0},
    {"dwc_t_help_month_day", 0},
    {"dwc_t_help_month_week", 0},
    {"dwc_t_help_month_month", 0},
    {"dwc_t_help_month_year", 0},
    {"dwc_t_help_month_scroll", 0},
    {"dwc_t_help_day_scroll", 0},
    {"dwc_t_help_day_general", 0},
    {"dwc_t_help_day_months", 0},
    {"dwc_t_help_day_timebar", 0},
    {"dwc_t_help_day_last", 0},
    {"dwc_t_help_day_next", 0},
    {"dwc_t_help_day_date", 0},
    {"dwc_t_help_day_clock", 0},
    {"dwc_t_help_year_general", 0},
    {"dwc_t_help_day_daynotes", 0},
    {"dwc_t_help_day_daynote_entry", 0},
    {"dwc_t_help_day_timeslots", 0},
    {"dwc_t_help_day_timeslots_entry", 0},
    {"dwc_t_pose_delete_text", 0 },
    {"dwc_t_pose_modify_text", 0 },
    {"dwc_t_memex_timeslot_format", 0 },
    {"dwc_t_memex_day_format", 0 },
    {"dwc_t_memex_month_format", 0 }, 
    {"dwc_t_memex_year_format", 0 },
    {"dwc_t_print_daynote_text", 0},
    {"dwc_t_calendar_copyright_text", 0},
    {"dwc_t_number_year",0},
    {"dwc_t_number_month",0},
    {"dwc_t_number_day",0},
    {"dwc_t_number_hour",0},
    {"dwc_t_number_min",0},
    {"dwc_t_number_sec",0},
    {"dwc_t_number_hund",0},
    {"dwc_t_lz_number_year",0},
    {"dwc_t_lz_number_month",0},
    {"dwc_t_lz_number_day",0},
    {"dwc_t_lz_number_hour",0},
    {"dwc_t_lz_number_min",0},
    {"dwc_t_lz_number_sec",0},
    {"dwc_t_lz_number_hund",0},
    {"dwc_t_number_week",0},
    {"dwc_t_lz_number_week",0},
    {"dwc_t_number_century",0},
    {"dwc_t_lz_number_century",0},
    {"dwc_t_year_suffix_texts", 0 },
    {"dwc_t_month_suffix_texts", 0 },
    {"dwc_t_day_suffix_texts", 0 },
    {"dwc_t_hour_suffix_texts", 0 },
    {"dwc_t_hour_12_suffix_texts", 0 },
    {"dwc_t_minute_suffix_texts", 0 },
    {"dwc_t_second_suffix_texts", 0 },
    {"dwc_t_sec_part_suffix_texts", 0 }
};

/*	  
**++
**
**  CALLING SEQUENCE:
**
**	XmString MISCFetchDRMValue( int value_index)
**
**  FUNCTIONAL DESCRIPTION:
**
**	Tries to find the value in a cache of values. Failing that it does a
**	MrmFetchLiteral to grab it from the MrmHierarchy that was loaded by the
**	inital MrmOpenHierarchy. If it isn't able to get the literal it will
**	exit with a catchall error.
**
**  FORMAL PARAMETERS:
**
**	int	value_index;	    index of UIL literal
**
**  RETURNS:
**
**	XmString literal
**
**  SIDE EFFECTS:
**
**	May result in a Catchall error.
**
*/	  
XmString
MISCFetchDRMValue
#if defined(_DWC_PROTO_)
	(
	int	value_index)
#else	/* no prototypes */
	(value_index)
	int	value_index;
#endif	/* prototype */
{
    int		status;
    MrmCode	code;
    
    assert (XtNumber(value_table) == dwc_k_num_UIL_values);

    /*	  
    **  value_table acts as a cache since going out to DRM each time was slow
    */	  
    if (value_table[ value_index ].value != NULL)
	return value_table[ value_index ].value;

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	value_table[value_index].value_name,
	XtDisplay(ads.root),
	(char **)&value_table[value_index].value,
	&code
    );
    if (status !=MrmSUCCESS) 
    {
	value_table[value_index].value = NULL;		/* just to be sure  */
	DWC$UI_Catchall (DWC$DRM_FETCHLITERAL, status, 0);
    }

    return (value_table[value_index].value);
}

MISCFreeAllDRMValues ()
{
    int	i;
    for (i = 0; i < XtNumber(value_table); i++)
	if (value_table[i].value != NULL) XmStringFree (value_table[i].value);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      MISCGetFontFromFontlist(fontlist, font)
**
**	Creates a context for the fontlist, grabs the first font and returns it.
**	If it can't create the context or return a valid font it will do a
**	DWC$UI_Catchall and die.
**
**  FORMAL PARAMETERS:
**
**      XmFontList  fontlist;
**	XFontStruct **font;
**
**  RETURN VALUE:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**--
*/
void MISCGetFontFromFontlist
#if defined(_DWC_PROTO_)
	(
	XmFontList	fontlist,
	XFontStruct	**font)
#else	/* no prototypes */
	(fontlist, font)
	XmFontList	fontlist;
	XFontStruct	**font;
#endif	/* prototype */
{
    XmFontContext	fontlist_context;
    XmStringCharSet	charset;

    if (!XmFontListInitFontContext(&fontlist_context, fontlist))
    {
	printf("%Unable to initialize font context\n");
	DWC$UI_Catchall(DWC$UI_NOFONTINIT, 0,0);
    }

    if (!XmFontListGetNextFont(fontlist_context, &charset, font))
    {
	printf("%Unable to get font\n");
	DWC$UI_Catchall(DWC$UI_NOFONT, 0,0);
    }
    XtFree(charset);

    XmFontListFreeFontContext(fontlist_context);

}

int MISCFontListWidth
#if defined(_DWC_PROTO_)
	(XmFontList	fontlist)
#else
	(fontlist)
	XmFontList	fontlist;
#endif
{
    XmFontContext		fontlist_context;
    XmStringCharSet		charset;
    XFontStruct			*font;
    int				width;

    if (!XmFontListInitFontContext(&fontlist_context, fontlist))
    {
	printf("%Unable to initialize font context\n");
	DWC$UI_Catchall(DWC$UI_NOFONTINIT, 0,0);
	return (0);
    }

    if (!XmFontListGetNextFont(fontlist_context, &charset, &font))
    {
	printf("%Unable to get font\n");
	DWC$UI_Catchall(DWC$UI_NOFONT, 0,0);
	XmFontListFreeFontContext(fontlist_context);
	return (0);
    }

    XtFree(charset);

    /*
    ** This was changed from max_bounds.width + min_bounds.width, because
    ** in some locales, the typical font is monospaced and thus makes this
    ** calculation come out huge.  What we really want is the width of the
    ** calendar plus some padding.
    */
    width = (font->max_bounds.width * 6) / 5;

    while (XmFontListGetNextFont(fontlist_context, &charset, &font))
    {
	XtFree(charset);
	width = MAX (width, ((font->max_bounds.width * 6) / 5));
    }

    XmFontListFreeFontContext(fontlist_context);

    return (width);
}

int MISCFontListAscent
#if defined(_DWC_PROTO_)
	(XmFontList	fontlist)
#else
	(fontlist)
	XmFontList	fontlist;
#endif
{
    XmFontContext	fontlist_context;
    XmStringCharSet	charset;
    XFontStruct		*font;
    int			ascent;

    if (!XmFontListInitFontContext(&fontlist_context, fontlist))
    {
	printf("%Unable to initialize font context\n");
	DWC$UI_Catchall(DWC$UI_NOFONTINIT, 0,0);
	return (0);
    }

    if (!XmFontListGetNextFont(fontlist_context, &charset, &font))
    {
	printf("%Unable to get font\n");
	DWC$UI_Catchall(DWC$UI_NOFONT, 0,0);
	XmFontListFreeFontContext(fontlist_context);
	return (0);
    }

    XtFree(charset);
    ascent = font->ascent;

    while (XmFontListGetNextFont(fontlist_context, &charset, &font))
    {
	XtFree(charset);
	ascent = MAX (ascent, font->ascent);
    }

    XmFontListFreeFontContext(fontlist_context);

    return (ascent);
}

int MISCFontListHeight
#if defined(_DWC_PROTO_)
	(XmFontList	fontlist)
#else
	(fontlist)
	XmFontList	fontlist;
#endif
{
    XmFontContext	fontlist_context;
    XmStringCharSet	charset;
    XFontStruct		*font;
    int			ascent, descent;

    if (!XmFontListInitFontContext(&fontlist_context, fontlist))
    {
	printf("%Unable to initialize font context\n");
	DWC$UI_Catchall(DWC$UI_NOFONTINIT, 0,0);
	return (0);
    }

    if (!XmFontListGetNextFont(fontlist_context, &charset, &font))
    {
	printf("%Unable to get font\n");
	DWC$UI_Catchall(DWC$UI_NOFONT, 0,0);
	XmFontListFreeFontContext(fontlist_context);
	return (0);
    }

    XtFree(charset);
    ascent = font->ascent;
    descent = font->descent;

    while (XmFontListGetNextFont(fontlist_context, &charset, &font))
    {
	XtFree(charset);
	ascent = MAX (ascent, font->ascent);
	descent = MAX (descent, font->descent);
    }

    XmFontListFreeFontContext(fontlist_context);

    return (ascent + descent);
}

typedef struct _PixmapCache
{
    Screen *screen;
    Pixmap pixmap;
    struct _PixmapCache *next;
} CacheEntry;

static CacheEntry *pixmapCache = NULL;


Pixmap MISCXtGrayPixmap (screen)
    Screen *screen;
/*
 *	This routine was previously called XtGrayPixmap and was contained 
 *	in the toolkit, but was never included in Motif.  This local copy is
 *	needed since Leo T. says it will never be included in Motif.  It had
 *	to be renamed because the Mail group was getting errors when they
 *	linked the Motif SVN code and the XUI toolkit; they needed to do
 *	this in order to pick up some code that had not been ported to
 *	Motif yet.
 *
 *	Creates a gray pixmap of depth DefaultDepth(screen)
 *	caches these so that multiple requests share the pixmap
 */
{
    Display *display = DisplayOfScreen(screen);
    CacheEntry *cachePtr;
    Pixmap gray_pixmap;
    static char pixmap_bits[] =
    {
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55,
	0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55
    };

/*
**	Creates a gray pixmap of depth DefaultDepth(screen)
**	caches these so that multiple requests share the pixmap
*/
#define pixmap_width 32
#define pixmap_height 32

    /* see if we already have a pixmap suitable for this screen */
    for (cachePtr = pixmapCache; cachePtr; cachePtr = cachePtr->next)
    {
	if (cachePtr->screen == screen)
	    return (cachePtr->pixmap);
    }

    /* nope, we'll have to construct one now */
    gray_pixmap = XCreatePixmapFromBitmapData
    (
	display,
	RootWindowOfScreen(screen),
	pixmap_bits,
	pixmap_width,
	pixmap_height,
	BlackPixelOfScreen(screen),
	WhitePixelOfScreen(screen),
	DefaultDepthOfScreen(screen)
    );

    /* and insert it at the head of the cache */
    cachePtr = XtNew(CacheEntry);

    cachePtr->screen = screen;
    cachePtr->pixmap = gray_pixmap;
    cachePtr->next = pixmapCache;
    pixmapCache = cachePtr;

    return (gray_pixmap);
}

/*	  
**  This routine updates "our" internal time
*/	  
void
MISCUpdateTime
#if defined(_DWC_PROTO_)
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{
    time_t	the_time;

    time (&the_time);
    master_local_time = localtime (&the_time);

    master_local_time->tm_mon  = master_local_time->tm_mon + 1;
    master_local_time->tm_year = master_local_time->tm_year + 1900;

}

/*	  
**  This returns our local time which is only known to this module.
**  master_local_time gets updated by calls to MISCUpdateTime.
*/	  
void
MISCGetTime
#if defined(_DWC_PROTO_)
	(
	struct tm	**time_block)
#else	/* no prototypes */
	(time_block)
	struct tm	**time_block;
#endif	/* prototype */
{
    
    /*	  
    **  We want time_block to point to the real stuff
    */	  
    *time_block = master_local_time;

}

/* this routine converts dimensions and positions from the profile format   */
/* to explict x and y and pixel values */
void
MISCGetScreenPixelPosition
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Position	percentage_of_screen_x,
	Position	percentage_of_screen_y,
	Dimension	profile_format_width,
	Dimension	profile_format_height,
	Dimension	*return_width,
	Dimension	*return_height,
	Position	*return_x,
	Position	*return_y)
#else	/* no prototypes */
	(cd, percentage_of_screen_x, percentage_of_screen_y,
	 profile_format_width, profile_format_height,
	 return_width, return_height, return_x, return_y)
	CalendarDisplay	cd;
	Position	percentage_of_screen_x;
	Position	percentage_of_screen_y;
	Dimension	profile_format_width;
	Dimension	profile_format_height;
	Dimension	*return_width;
	Dimension	*return_height;
	Position	*return_x;
	Position	*return_y;
#endif	/* prototype */
{
    Position		object_center_x;
    Position		object_center_y;
    Position		top_left_x;
    Position		top_left_y;
    
    /*	  
    **  Get the right size of the object in pixels after allowing for the
    **	particular font that is the default. In effect this converts from the
    **	saved format in the profile to actual pixels that correspond to the
    **	particular font in use.
    */	  
    *return_width  = MISCCvtUnitsToPixels(profile_format_width, cd->screen_font_size);
    *return_height = MISCCvtUnitsToPixels(profile_format_height, cd->screen_font_size);

    /*	  
    **  Get the x and y of the place on the screen which will be the center of
    **	the object. A percentage of 50 would be passed in as 5000.
    */	  
    object_center_x =
	(((Position)cd->width_of_screen  * percentage_of_screen_x) + 9999)
			/ 10000;
    object_center_y =
	(((Position)cd->height_of_screen * percentage_of_screen_y) + 9999)
			/ 10000;

    top_left_x = object_center_x - (Position)(*return_width / 2);
    top_left_y = object_center_y - (Position)(*return_height / 2);

    if (*return_width < cd->width_of_screen)
    {
        /*	  
	**  We're okay the width is narrower than the screen.
	*/	  
        *return_x = top_left_x;
    }
    else
    {
        /*	  
	**  Our width is wider than the screen so set our left margin at the
	**  edge of the screen.
	*/	  
        *return_x = 0;
    }

    if ((*return_height < cd->height_of_screen) && (top_left_y >= 10))
    {
        /*	  
	**  We're okay with the height shorter than the screen and the top of
	**  the window is on the screen.
	*/	  
        *return_y = top_left_y;
    }
    else
    {
        /*	  
	**  If our height is taller than the screen or our top left edge (with
	**  the all important menu bar) is off the top of the screen, then force
	**  it onto the screen. We've got a problem since the 0 doesn't allow
	**  space to fit the window manager decoration stuff so the window
	**  manager decoration ends up off the screen. Not sure how to fix.
	**  PGLF 8/14/90
	*/	  
        *return_y = 10;
    }
}

void
MISCGetScreenFractionalPosition
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Dimension	object_width,
	Dimension	object_height,
	Position	x,
	Position	y,
	Position	*percentage_of_screen_x,
	Position	*percentage_of_screen_y)
#else	/* no prototypes */
	(cd, object_width, object_height, x, y, percentage_of_screen_x, percentage_of_screen_y)
	CalendarDisplay	cd;
	Dimension	object_width;
	Dimension	object_height;
	Position	x;
	Position	y;
	Position	*percentage_of_screen_x;
	Position	*percentage_of_screen_y;
#endif	/* prototype */
{
    Dimension		width;
    Dimension		height;
    

    /*	  
    **  Get the right size of the object in pixels after allowing for the
    **	particular font that is the default. In effect this converts from the
    **	saved format in the profile to actual pixels that correspond to the
    **	particular font in use.
    */	  
    width  = MISCCvtUnitsToPixels(object_width, cd->screen_font_size);
    height = MISCCvtUnitsToPixels(object_height, cd->screen_font_size);

    /*	  
    **  This routine converts from the top left x and y based on a particular
    **	screen to the percentage of screen, center-of-object units used in the
    **	profile. So, the middle of the screen is expressed as 5000 or 50.00%
    */	  
    *percentage_of_screen_x =
	(
	    (
		(x + (Position)(width  / 2)) *
		10000
	    ) -
	    (Position)(cd->width_of_screen - 1)
	) /
	(Position)cd->width_of_screen;
    *percentage_of_screen_y =
	(
	    (
		(y + (Position)(height / 2)) *
		10000
	    ) -
	    (Position)(cd->height_of_screen - 1)
	) /
	(Position)cd->height_of_screen;

}

Dimension
MISCCvtUnitsToPixels
#if defined(_DWC_PROTO_)
	(
	Dimension	internal_unit_dimension,
	Dimension	converting_unit)
#else	/* no prototypes */
	(internal_unit_dimension, converting_unit)
	Dimension	internal_unit_dimension;
	Dimension	converting_unit;
#endif	/* prototype */
{
    /*	  
    **  We need to convert our internal units to Pixels. We do that by
    **  multiplying by the converting unit and then adding one less than the
    **  divisor to handle roundoff.
    */	  
    return (((internal_unit_dimension * converting_unit) + 99) / 100);

}   

void
MISCSetGCClipMask
#if defined(_DWC_PROTO_)
	(
	Display		*d,
	GC		gc,
	XRectangle	*expose_area,
	Region		region)
#else	/* no prototypes */
	(d, gc, expose_area, region)
	Display		*d;
	GC		gc;
	XRectangle	*expose_area;
	Region		region;
#endif	/* prototype */
{

    if (region == NULL)
    {
	if (expose_area == NULL)
	{
	    XSetClipMask (d, gc, None);
	}
	else
	{
	    XSetClipRectangles (d, gc, 0, 0, expose_area, 1, Unsorted);
	}
    }
    else
    {
	XSetRegion (d, gc, region);
    }

}

Time
MISCGetTimeFromEvent
#if defined(_DWC_PROTO_)
	(
	XEvent	*event)
#else	/* no prototypes */
	(event)
	XEvent	*event;
#endif	/* prototype */
{

static	Time lasteventtime	= CurrentTime;


    /*	  
    **  We want to save the last event time in case we get an event that doesn't
    **	have a time associated with it, such as FocusIn.
    */	  
    if (event != NULL)
    {
	switch (event->type)
	{
	case KeyPress:
	    lasteventtime = ((XKeyPressedEvent *) event)->time;
	    break;
	case KeyRelease:
	    lasteventtime = ((XKeyReleasedEvent *) event)->time;
	    break;
	case ButtonPress:
	    lasteventtime = ((XButtonPressedEvent *) event)->time;
	    break;
	case ButtonRelease:
	    lasteventtime = ((XButtonReleasedEvent *) event)->time;
	    break;
	case MotionNotify:
	    lasteventtime = ((XPointerMovedEvent *) event)->time;
	    break;
	case EnterNotify:
	    lasteventtime = ((XEnterWindowEvent *) event)->time;
	    break;
	case LeaveNotify:
	    lasteventtime = ((XLeaveWindowEvent *) event)->time;
	    break;
	case PropertyNotify:
	    lasteventtime = ((XPropertyEvent *) event)->time;
	    break;
	case SelectionClear:
	    lasteventtime = ((XSelectionClearEvent *) event)->time;
	    break;
	case SelectionRequest:
	    lasteventtime = ((XSelectionRequestEvent *) event)->time;
	    break;
	case SelectionNotify:
	    lasteventtime = ((XSelectionEvent *) event)->time;
	    break;
	default:
	    break;
	}
    }

    return(lasteventtime);
}

Time
MISCGetTimeFromAnyCBS
#if defined(_DWC_PROTO_)
	(
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(cbs)
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{

    /*	  
    **  If it is null  then get the latest time that we had.
    */	  
    if (cbs == NULL)
	return (MISCGetTimeFromEvent(NULL));

    return (MISCGetTimeFromEvent(cbs->event));

}

/*	  
**  MISCFitFormDialogOnScreen
**  This routine fits a formdialog to the screen. The formdialog needs to have
**  been  set up in UIL with the following structure:
**	FormDialog
**	    |
**	ScrolledWindow (policy XmAUTOMATIC)
**	    |
**	Form (contains the "real" info)
**
**  See the "customize_dayview_box" in DWC_UI_CUSTOM_OBJECTS.UIL for an
**  example.
**
**  Basically, this routine, gets the size of the form, sets the scrolledwindow
**  to the smaller of that size or the screen size for width and height.
**
**  SIDE EFFECTS:
**	plays around with the sizes of the children of the FormDialog passed in.
**	
*/	  
void
MISCFitFormDialogOnScreen
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Widget		formdialog)
#else	/* no prototypes */
	(cd, formdialog)
	CalendarDisplay	cd;
	Widget		formdialog;
#endif	/* prototype */
{
    int			ac;
    Dimension		margin_height,
			margin_width,
			height,
			width;
    CompositeWidget	scrolledwclip_widget;
    CompositeWidget	scrolledwindow_widget;
    CompositeWidget	form_widget;
    int			i;

#define	    DISTANCE_TO_EDGE_OF_SCREEN  20
#define	    SCROLLWINDOW_DRESSING   6

    /*	  
    **  Let's do some runtime twiddling. (get first child)
    */	  
    scrolledwindow_widget =
	    (CompositeWidget)
		(((CompositeWidget)formdialog)->composite.children[0]);
    if (XmIsScrolledWindow(scrolledwindow_widget))
    {
	/*	  
	**  Get the size of the scrolledwindows margins for latter figuring.
	*/	  
	XtVaGetValues
	(
	    (Widget) scrolledwindow_widget,
	    XmNscrolledWindowMarginWidth, &margin_width,
	    XmNscrolledWindowMarginHeight,&margin_height,
	    NULL
	);
	/*	  
	**  If the margins on the scrolled window are 0 then we better do
	**	something about it.
	*/	  
	if (margin_width == 0)
	    margin_width = SCROLLWINDOW_DRESSING;

	if (margin_height == 0)
	    margin_height = SCROLLWINDOW_DRESSING;
	
	/*	  
	**  This should be the ScrolledWindowClipWindow (first child)
	*/	  
	scrolledwclip_widget =
	    (CompositeWidget)
	    (((CompositeWidget)scrolledwindow_widget)->composite.children[0]);

	/*	  
	**  Let's find the form under the scrolled window clip window
	*/	  
	for( i=0; i < ((scrolledwclip_widget)->composite.num_children); i++)
	{
	    form_widget =
	    (CompositeWidget)((scrolledwclip_widget)->composite.children[i]);
	    if ( XmIsForm( form_widget))
	    {
		/*	  
		**	This is the form kid of the scrolledwindow. Find out
		**	how big it is.
		*/
		XtVaGetValues
		(
		    (Widget)form_widget,
		    XmNwidth, &width,
		    XmNheight, &height,
		    NULL
		);

		/*	  
		**	We want to make the scrolledwindow the right size. We
		**	add the scrollwindow margins so that we don't get the
		**	scrollbars by default since the scrollwindow will
		**	compare its size (minus thickness) to the form to
		**	decide whether or not to display the scrollbars. We
		**	want to make sure that we also fit on the screen (with
		**	some margin "DISTANCE_TO_EDGE_OF_SCREEN").
		*/	  
		width = MIN(width + (margin_width * 2),
			    cd->width_of_screen -
				(DISTANCE_TO_EDGE_OF_SCREEN * 2));
		height = MIN(height + (margin_height * 2),
			    cd->height_of_screen -
				(DISTANCE_TO_EDGE_OF_SCREEN * 2));

		XtVaSetValues
		(
		    (Widget)scrolledwindow_widget,
		    XmNwidth, width,
		    XmNheight, height,
		    NULL
		);
		break;  /* we've found what we were looking for */
	    }
	} /* end of the for */
    } /* end of scrolled window if */

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**	MISCFindParentShell
**      This routine finds the parent shell for a widget.
**
**  FORMAL PARAMETERS:
**
**
**  IMPLICIT INPUTS:
**
**
**  IMPLICIT OUTPUTS:
**
**
**  {@function value or completion codes@}
**
**      [@description or none@]
**
**  SIDE EFFECTS:
**
**      [@description or none@]
**
**--
*/
Widget
MISCFindParentShell
#if defined(_DWC_PROTO_)
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    Widget pw, parent;

    parent = pw = w;
    while (pw != NULL)
    {
	if (XtIsSubclass (pw, shellWidgetClass))
	{
	    parent = pw;
	    break;
	} 
	pw = XtParent (pw);
    }

    return parent;
}

void
MISCUpdateCallback
#if defined(_DWC_PROTO_)
	(
	Widget		r,		/* the real widget*/
	XtCallbackList	*rstruct,	/* the real callback list */
	Widget		s,		/* the scratch widget*/
	XtCallbackList	*sstruct,	/* the scratch callback list */
	char		*argname)
#else	/* no prototypes */
	(r, rstruct, s, sstruct, argname)
	Widget		r;		/* the real widget*/
	XtCallbackList	*rstruct;	/* the real callback list */
	Widget		s;		/* the scratch widget*/
	XtCallbackList	*sstruct;	/* the scratch callback list */
	char		*argname;
#endif	/* prototype */
{
    XtCallbackList	    list;

    /*
    ** if a new callback has been specified in the scratch widget, remove and
    ** deallocate old callback and init new
    */

    if (*rstruct != *sstruct)
    {

	list = *sstruct;

	/*
	**  Copy the old callback list into the new widget, since
	**  XtRemoveCallbacks needs the "real" widget
    	*/

        *sstruct = *rstruct;
	XtRemoveAllCallbacks (s, argname);
	*sstruct = NULL;
	XtAddCallbacks (s, argname, list);
    }

}


/* This geometry manager is always agreeable, no matter what the requestor  */
/* asks! 8-) */
XtGeometryResult
MISCAgreeableGeometryManager
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	XtWidgetGeometry	*request,
	XtWidgetGeometry	*reply)
#else	/* no prototypes */
	(w, request, reply)
	Widget			w;
	XtWidgetGeometry	*request;
	XtWidgetGeometry	*reply;
#endif	/* prototype */
{
    
    if ((request->request_mode & CWX) != 0)
    {
	/* accept the change to the x origin relative to the parent */
	XtX (w) = request->x;
    }

    if ((request->request_mode & CWY) != 0)
    {
	/* accept the change to the y origin relative to the parent */
	XtY (w) = request->y;
    }

    if ((request->request_mode & CWWidth) != 0)
    {
	XtWidth (w) = request->width;
    }

    if ((request->request_mode & CWHeight) != 0)
    {
	XtHeight (w) = request->height;
    }
    
    if ((request->request_mode & CWBorderWidth) != 0)
    {
	XtBorderWidth (w) = request->border_width;
    }

    /*
    **  Be agreeable...!
    */

    return (XtGeometryYes);
    
}

/* Grabs the x and y off the top_level and depending upon which view is	    */
/* currently showing, translates the top left x and y into the center	    */
/* percentage based x and y that is saved in the profile. */
static void
save_current_view_xy
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	    cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	    cd;
#endif	/* prototype */
{
    Position	current_x, current_y;

    /*	  
    **  If we already have a view up, then grab the existing geometry since
    **  the user may have moved things around and we want to keep it for the
    **  next time they show that view.
    */	  
    if (cd->active_widget != NULL)
    {
	XtVaGetValues (cd->toplevel, XmNx, &current_x, XmNy, &current_y, NULL);

        /*	  
	**  Depending on the view save it to the right place in the profile. The
	**  user will still need to do a SAVE CURRENT for this stuff to get
	**  saved across a program exit.
	*/	  
        switch (cd->showing)
	{
	case show_day:
	    MISCGetScreenFractionalPosition
		(cd, (Dimension)cd->profile.day_width,
		(Dimension)cd->profile.day_height,
		current_x, current_y,
		&cd->profile.day_x, &cd->profile.day_y);
	    break;

	case show_week:
	    MISCGetScreenFractionalPosition
		(cd, (Dimension)cd->profile.week_width,
		(Dimension)cd->profile.week_height,
		current_x, current_y,
		&cd->profile.week_x, &cd->profile.week_y);
	    break;

	case show_month:
	    MISCGetScreenFractionalPosition
		(cd, (Dimension)cd->profile.month_width,
		(Dimension)cd->profile.month_height,
		current_x, current_y,
		&cd->profile.month_x, &cd->profile.month_y);
	    break;
	    
	case show_year:
	    MISCGetScreenFractionalPosition
		(cd, (Dimension)cd->profile.year_width,
		(Dimension)cd->profile.year_height,
		current_x, current_y,
		&cd->profile.year_x, &cd->profile.year_y);
	    break;

	} /* end of switch */
    } /* end of active_widget != NULL */

}

/* Puts any messagebox children of widget w on top by raising them */
void MISCPutMessagesOnTop
#if defined(_DWC_PROTO_)
	(
	Widget		mainwid_widget)
#else	/* no prototypes */
	(mainwid_widget)
	Widget		mainwid_widget;
#endif	/* prototype */
{
    CompositeWidget	dialogshell_widget;
    Widget		dialogchild_widget;
    int			i,j;
    XtWidgetGeometry	request ;
    XtGeometryResult	result ;

#if 0
    for (i=0; i < ((mainwid_widget)->core.num_popups); i++)
    {
	dialogshell_widget = (CompositeWidget)
	    ((mainwid_widget)->core.popup_list[i]);
	if (XmIsDialogShell(dialogshell_widget))
	{
	    for (j=0; j < ((dialogshell_widget)->composite.num_children); j++)
	    {
		dialogchild_widget = (Widget)
		    (dialogshell_widget)->composite.children[j];
		if ( XmIsMessageBox( dialogchild_widget))
		{
		    if ( XtIsManaged(dialogchild_widget) )
		    {
			XFlush(XtDisplay(mainwid_widget));
			/*	  
			**  Let's try to see that it goes on top
			*/	  
			request.request_mode = CWStackMode;
			request.stack_mode = Above;
			result = XtMakeGeometryRequest
			    ((Widget)dialogshell_widget, &request, NULL);

		    }
		}
	    }
	}
    } /* end of the for */
#endif
}

void MISCSizeButtonsEqually
#if defined(_DWC_PROTO_)
	(
	Widget		*widgets,
	Cardinal	num_widgets
	)
#else
(widgets, num_widgets)
	Widget		*widgets;
	Cardinal	num_widgets;
#endif
/**
*** Traverse a table of widgets and set them all to the size of the largest.
**/
{
    Widget	    w;
    Cardinal	    i;
    Dimension	    max_width = 0;
    Dimension	    max_height = 0;
    
    /*
    ** Traverse the list getting the size.  We save a little time by using
    ** XtHeight & XtWidth instead of XtGetValues.
    */
    for (i = 0; i < num_widgets; i++)
    {
	w = widgets[i];
	if (XtWidth(w) > max_width) max_width = XtWidth(w);
	if (XtHeight(w) > max_height) max_height = XtHeight(w);
    }

    /*
    ** Change the sizes.  We need to use set values here.
    */
    for (i = 0; i < num_widgets; i++)
    {
	w = widgets[i];
	XtVaSetValues (w, XmNwidth, max_width, XmNheight, max_height, NULL);
    }
}

void MISCHeightButtonsEqually
#if defined(_DWC_PROTO_)
	(
	Widget		*widgets,
	Cardinal	num_widgets
	)
#else
(widgets, num_widgets)
	Widget		*widgets;
	Cardinal	num_widgets;
#endif
/**
*** Traverse a table of widgets and set them all to the height of the largest.
**/
{
    Widget	    w;
    Cardinal	    i;
    Dimension	    max_height = 0;
    
    /*
    ** Traverse the list getting the height.  We save a little time by using
    ** XtHeight instead of XtGetValues.
    */
    for (i = 0; i < num_widgets; i++)
    {
	w = widgets[i];
	if (XtHeight(w) > max_height) max_height = XtHeight(w);
    }

    /*
    ** Change the sizes.  We need to use set values here.
    */
    for (i = 0; i < num_widgets; i++)
    {
	w = widgets[i];
	XtVaSetValues (w, XmNheight, max_height, NULL);
    }
}

void MISCWidthButtonsEqually
#if defined(_DWC_PROTO_)
	(
	Widget		*widgets,
	Cardinal	num_widgets
	)
#else
(widgets, num_widgets)
	Widget		*widgets;
	Cardinal	num_widgets;
#endif
/**
*** Traverse a table of widgets and set them all to the width of the largest.
**/
{
    Widget	    w;
    Cardinal	    i;
    Dimension	    max_width = 0;
    
    /*
    ** Traverse the list getting the width.  We save a little time by using
    ** XtWidth instead of XtGetValues.
    */
    for (i = 0; i < num_widgets; i++)
    {
	w = widgets[i];
	if (XtWidth(w) > max_width) max_width = XtWidth(w);
    }

    /*
    ** Change the sizes.  We need to use set values here.
    */
    for (i = 0; i < num_widgets; i++)
    {
	w = widgets[i];
	XtVaSetValues (w, XmNwidth, max_width, NULL);
    }
}

void MISCSpaceButtonsEqually
#if defined(_DWC_PROTO_)
	(
	Widget		*widgets,
	Cardinal	num_widgets
	)
#else
(widgets, num_widgets)
	Widget		*widgets;
	Cardinal	num_widgets;
#endif
{
    Widget	parent = XtParent(widgets[0]);
    int		i;
    int		fraction_base;
    int		position;
    int		offset;
    int		width;

    /*
    ** Have to have something.
    */
    if (!widgets || (num_widgets == 0) || !XmIsForm(parent)) return;

    /*
    ** Get the form's fraction base, so that we know how to modify the
    ** positions of the widgets.
    */
    XtVaGetValues (parent, XmNfractionBase, &fraction_base, NULL);

    /*
    ** Get the width of one of the widgets.
    */
    width = (int)XtWidth(widgets[0]);

    /*
    ** Change each one to be centered over the appropriate fraction point.
    */
    for (i = 0; i < num_widgets; i++)
    {
	/*
	** The "fraction" for each step is the space between widgets.  This
	** will be 1 more than the number of widgets, dividing the fraction
	** base.
	*/
	position = ((i + 1) * fraction_base) / (num_widgets + 1);

	/*
	** The offset is calculated so that you will have a decreasing
	** fraction of the widget to the left of the attacment point.  It
	** makes it so that each section contains the same amount of button.
	*/
	offset = (width * (i-(int)num_widgets)) / ((int)num_widgets+1);

	/*
	** Reset the horizontal placement completely.
	*/
	XtVaSetValues
	(
	    widgets[i],
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition, position,
	    XmNleftOffset, offset,
	    XmNrightAttachment, XmATTACH_NONE,
	    NULL
	);
    }

}

Boolean DXIsXUIWMRunning
#if defined(_DWC_PROTO_)
(Widget widget, Boolean check)
#else
(widget, check)
Widget	widget;
Boolean	check;
#endif
/**
*** This exists for an external module that insists on it being here.  It help
*** eliminate duplication.
**/
{
    return (MISCIsXUIWMRunning (widget, check));
}

Boolean MISCIsXUIWMRunning
#if defined(_DWC_PROTO_)
	(
	Widget	widget,
	Boolean	check
	)
#else
	(widget, check)
	Widget	widget;
	Boolean	check;
#endif
/*
** Check to see if XUI is running in order to determine if we need to do
** iconify pixmap.  It is possible for this routine to give a false positive
** but never a false negative.
**
** If check is false, we just return the result of a previous test.  This is
** because there are times when we need to know, but know that the last check
** is good enough.  Check is generally only set to true at ReparentNotify.
*/
{
    typedef struct
    {
	CARD32 title_font;
	CARD32 icon_font;
	CARD32 border_width;
	CARD32 title_height;
	CARD32 non_title_width;
	CARD32 icon_name_width;
	CARD32 iconify_width;
	CARD32 iconify_height;
    } internalDecorationGeometryRec, *internalDecorationGeometry;

#define WmNumDecorationGeometryElements \
    (sizeof(internalDecorationGeometryRec)/sizeof(CARD32))

typedef struct {
    CARD32 value_mask;
    CARD32 iconify_pixmap;
    CARD32 icon_box_x;
    CARD32 icon_box_y;
    CARD32 tiled;
    CARD32 sticky;
    CARD32 no_iconify_button;
    CARD32 no_lower_button;
    CARD32 no_resize_button;
} internalDECWmHintsRec, *internalDECWmHints;

#define WmNumDECWmHintsElements (sizeof(internalDECWmHintsRec)/sizeof(CARD32))

    static int		result = False;
    Screen		*scrn = XtScreen(widget);
    Display		*dpy = XtDisplay(widget);
    /*
    ** Variables for XGetWindowProperty calls.
    */
    static Atom			dec_geom = None;
    static Atom			dec_hints = None;

    internalDecorationGeometry	prop = 0;
    internalDECWmHints		hprop = 0;
    Atom			actual_type;
    int				actual_format;
    unsigned long		leftover;
    unsigned long		nitems;

    /*
    ** If we don't need to go to the server, then return the cached value.
    */
    if (!check) return (result);

    /*
    ** Once we get this atom, we don't need to refetch it.  Atoms are never
    ** destroyed without restarting the server.
    */
    if (dec_geom == None)
    {
	dec_geom = XmInternAtom(dpy, "DEC_WM_DECORATION_GEOMETRY", True);
    }

    /*
    ** If still None, then XUI WM can not have run yet (or has been cleaned
    ** up by DEC's mwm).
    */
    if (dec_geom == None)
    {
	result = False;
	return (result);
    }

    /*
    ** Check for an undocumented property name to find out if the
    ** XUI WM has been run on this server.  Of course, this test doesn't
    ** tell you if the window manager is still running - So, there is
    ** room for improvment here.
    */
    XGetWindowProperty
    (
	dpy,
	RootWindowOfScreen(scrn),
	dec_geom,
	0L,
	(long)WmNumDecorationGeometryElements,
	False,
	dec_geom,
	&actual_type,
	&actual_format,
	&nitems,
	&leftover,
	(unsigned char **)&prop
    );
    if (prop != 0) XFree ((char *)prop);

    /*
    ** Check to see a property with the given name exists.
    ** The XUI WM is the only client we know
    ** about that sets a property with this name.  Therefore,
    ** if the property exists, we assume the XUI WM is running.
    */
    if ((actual_type != dec_geom) ||
	(nitems < WmNumDecorationGeometryElements) ||
	(actual_format != 32))
    {
	result = False;
	return (result);
    }

    /*
    ** If it hasn't bailed out yet, we still are not sure.
    ** We now do a check for a different property.  This one will be on
    ** our shell if XUI WM has run since this application started.  Once
    ** it is there, it doesn't go away (as far as I know).  Therefore, if
    ** the previous result was True, we know that it will still be True and
    ** we can skip this check.
    */
/* This doesn't seem to work.  Check with Ken Ravitz for more info. */
#if defined(DWC_ATTEMPT_TO_FIND_MORE_DXWM_STUFF)
    if (result) return (result);
    else
    {
	/*
	** Check for cached atom.
	*/
	if (dec_hints == None)
	{
	    dec_hints = XmInternAtom (dpy, "DEC_WM_HINTS", True);
	}

	/*
	** Still not there, we're outa here.
	*/
	if (dec_hints == None)
	{
	    result = False;
	    return (result);
	}

	/*
	** Try to get the DEC WM HINTS property.
	*/
	XGetWindowProperty
	(
	    dpy,
	    XtWindow(widget),
	    dec_hints,
	    0L, (long)WmNumDECWmHintsElements,
	    False,
	    dec_hints,
	    &actual_type,
	    &actual_format,
	    &nitems,
	    &leftover,
	    (unsigned char **)&hprop
	);

	/*
	** Free up the actual property, we're not really interested in it,
	** just its existence.
	*/
	if (hprop != 0) XFree ((char *) hprop);

	/*
	** Check whether we got it.
	*/
	if ((actual_type != dec_hints) ||
	    (nitems < WmNumDECWmHintsElements) ||
	    (actual_format != 32))
	{
	    result = False;
	    return (result);
	}

    }
#endif
    result = True;
    return (result);

}

void MISCAddProtocols
#if defined(_DWC_PROTO_)
	(
	Widget	widget,
	XtCallbackProc	delete,
	XtCallbackProc	save
	)
#else
	(widget, delete, save)
	Widget	widget;
	XtCallbackProc	delete;
	XtCallbackProc	save;
#endif
/**
*** Add callbacks for either WM_DELETE_WINDOW, WM_SAVE_YOURSELF or both.
*** The routine caches the atoms used for these protocols.  This is redundant
*** but saves a routine call each time.
**/
{
    static Atom	DwcWmDeleteWindowAtom = None;
    static Atom	DwcWmSaveYourselfAtom = None;

    /*
    ** Try to get the atoms necessary to do this job.
    */
    if (DwcWmDeleteWindowAtom == None)
    {
	DwcWmDeleteWindowAtom = XmInternAtom
	    (XtDisplay(widget), "WM_DELETE_WINDOW", False);
    }
    if (DwcWmSaveYourselfAtom == None)
    {
	DwcWmSaveYourselfAtom = XmInternAtom
	    (XtDisplay(widget), "WM_SAVE_YOURSELF", False);
    }

    /*
    ** Add the callbacks that do what's needed.
    */
    if (delete != NULL)
    {
	XmAddWMProtocolCallback
	    (widget, DwcWmDeleteWindowAtom, (XtCallbackProc)delete, NULL);
	XmActivateWMProtocol (widget, DwcWmDeleteWindowAtom);
    }

    if (save != NULL)
    {
	XmAddWMProtocolCallback
	    (widget, DwcWmSaveYourselfAtom, (XtCallbackProc)save, NULL);
	XmActivateWMProtocol (widget, DwcWmSaveYourselfAtom);
    }
}

Pixmap MISCCreatePixmapFromBitmap
#if defined(_DWC_PROTO_)
    (
    Widget	w,
    Pixmap	src,
    Dimension	width,
    Dimension	height
    )
#else
    (w, src, width, height)
    Widget	w;
    Pixmap	src;
    Dimension	width;
    Dimension	height;
#endif
/**
*** This routine generates a default depth of screen pixmap from an existing
*** bitmap.
**/
{
    Pixmap		pixmap;
    static Display	*disp = NULL;
    static Screen	*scrn = NULL;
    static Pixel	foreground = 1;
    static Pixel	background = 0;
    Pixel		t_fore, t_back;
    static GC		gc = NULL;

    /*
    ** Get the foreground and background of the widget for which this pixmap
    ** is going to be used.
    */
    XtVaGetValues
	(w, XmNforeground, &t_fore, XmNbackground, &t_back, NULL);

    /*
    ** We cache the gc used here.  This is done so that the we can reduce
    ** protocol to the server.  The things that might change are:
    **  display, screen, foreground color, background color.
    */
    if ((disp != XtDisplay(w)) || (scrn != XtScreen(w)) ||
	(t_fore != foreground) || (t_back != background) || (gc == 0))
    {
	XGCValues   gcv;

	disp = XtDisplay(w);
	scrn = XtScreen(w);
	foreground = t_fore;
	background = t_back;
	gcv.foreground = foreground;
	gcv.background = background;

	if (gc != 0)
	{
	    XtReleaseGC (w, gc);
	    gc = 0;
	}

	gc = XtGetGC (w, GCForeground | GCBackground, &gcv);

    }

    /*
    ** Create the pixmap on the same screen as the widget in question of the
    ** same size as the bitmap.
    */
    pixmap = XCreatePixmap
	(disp, XtWindow(w), width, height, DefaultDepthOfScreen(scrn));

    /*
    ** Copy the image from the bitmap to the pixmap.
    */
    XCopyPlane (disp, src, pixmap, gc, 0, 0, width, height, 0, 0, 1);

    return (pixmap);
}

void MISCGetBestIconSize
#if defined(_DWC_PROTO_)
    (
    Widget	w,
    int		*rheight,
    int		*rwidth
    )
#else
    (w, rheight, rwidth)
    Widget	w;
    int		*rheight;
    int		*rwidth;
#endif
/**
*** This routine returns the height and width of the biggest icon size supported
*** by the current server/window manager.
**/
{
    XIconSize		icon_size_hints, *i_sizes;
    int			numsizes, currsize;
    static const int	height = 64;
    static const int	width = 64;
    int			i;

    /*
    ** Ask the server about supported icon sizes.
    */
    if (XGetIconSizes
	(XtDisplay(w), RootWindowOfScreen(XtScreen(w)), &i_sizes, &numsizes)) 
    {
	/*
	** Look for largest allowable icon size.  Loop from the 2nd item
	** to the last, comparing against the biggest so far.
	*/
	currsize=0;
	for (i=1; i<numsizes; i++)
	{
	    /*
	    ** Only take it if both width and height bigger.
	    */
	    if (i_sizes[i].max_width >= i_sizes[currsize].max_width &&
		i_sizes[i].max_height >= i_sizes[currsize].max_height)
	    {
		currsize = i;
	    }
	}
	/*
	** If the best so far is negative, use the default.  Otherwise, use
	** that one.
	*/
	if (i_sizes[currsize].max_width <= 0 ||
	    i_sizes[currsize].max_height <= 0)
	{
	    *rwidth = width;
	    *rheight = height;
	}
	else
	{
	    *rwidth = i_sizes[currsize].max_width;
	    *rheight = i_sizes[currsize].max_height;
	}
	XFree ((char *)i_sizes);
    }
    else
    {
	/*
	** If no iconsize information, use the default.
	*/
	*rwidth = width;
	*rheight = height;
    }
}

typedef struct
{
    Time    t;
    Widget  w;
} focusHandleRec, *FocusHandle;

static void MiscHandleFocusOnMap
#if defined(_DWC_PROTO_)
    (
    Widget	w,
    caddr_t	fh_in,
    XEvent	*event
    )
#else
    (w, fh_in, event)
    Widget	w;
    caddr_t	fh_in;
    XEvent	*event;
#endif
/*
**+
**
**  DESCRIPTION:
**
**	This routine is the event handler which will attempt to get focus in
**	the right place.  It will check what the focus policy is at this
**	point and try to push focus where we told it.
**
**  ARGUMENTS:
**
**  w - The widget that has just been mapped.  This widget may be a shell.
**	In which case, XSetInputFocus might be used to try to force focus
**	here.
**
**  fh_in - A pointer to an XtMalloced structure which contains the timestamp
**	for focus management and the widget which will accept traversal.
**
**  event - A pointer to the event structure (MapNotify) that got us here.
**
**-
*/
{
    FocusHandle		fh = (FocusHandle) fh_in;
    Widget		to_be_traversed = fh->w;
    Time		time_stamp = fh->t;
    unsigned char	focus_policy;

    /*
    ** Only do map events.
    */
    if (event->type != MapNotify) return;

    /*
    ** Turn yourself off (only need to do this once).
    */
    XtRemoveEventHandler
    (
	w,
	StructureNotifyMask,
	False,
	(XtEventHandler) MiscHandleFocusOnMap,
	(caddr_t) fh
    );

    /*
    ** Free the data structure.  This will be rebuilt the next time it is
    ** needed.
    */
    XtFree ((char *) fh);

    /*
    ** If we gave it a shell, force focus to it.
    */
    if (XtIsShell(w))
    {
	/*
	** Only force focus in EXPLICIT mode.
	*/
	if (_XmGetFocusPolicy(w) == (int)XmEXPLICIT)
	{
	    XSetInputFocus
		(XtDisplay(w), XtWindow(w), RevertToParent, time_stamp);
	}
    }

    /*
    ** Control focus within the shell.
    */
    if (to_be_traversed != NULL)
    {
	/*
	** Process traversal is called twice because of a bug in Motif that
	** causes it to sometimes not work on the first attempt.
	*/
	XmProcessTraversal (to_be_traversed, XmTRAVERSE_CURRENT);
	XmProcessTraversal (to_be_traversed, XmTRAVERSE_CURRENT);
    }

}

void MISCFocusOnMap
#if defined(_DWC_PROTO_)
    (
    Widget	to_be_mapped,
    Widget	to_be_traversed
    )
#else
    (to_be_mapped, to_be_traversed)
    Widget	to_be_mapped;
    Widget	to_be_traversed;
#endif
/*
**+
**
**  DESCRIPTION:
**
**	This routine establishes an event handler which will set focus in the
**	appropriate place once the given widget is mapped.  It is done in the
**	Map notification because of the fact that the focus handling can't be
**	reliably done until the window is on the screen.
**
**  ARGUMENTS:
**
**  to_be_mapped - The widget upon whose mapping this all turns.  There
**	will be slightly different behaviors if this is a shell.  That is
**	because we can force focus here using a timestamp we cache now for
**	use in the event handler.
**
**  to_be_traversed - The widget where keyboard traversal will be forced if
**	in the event handler.  This can be NULL (implying to allow the
**	default).
**
**-
*/
{
    FocusHandle	    fh;

    /*
    ** Pass in the timestamp for setting focus and the widget to which
    ** focus is going to be set.  It is safe to use XtLastTimestampProcessed
    ** here, because we will be acting on the primary event of a user action
    ** This structure will be freed in the in event handler.
    */
    fh = (FocusHandle) XtMalloc (sizeof(focusHandleRec));
    fh->t = XtLastTimestampProcessed(XtDisplay(to_be_mapped));
    fh->w = to_be_traversed;

    /*
    ** Add the event handler.
    */
    XtAddEventHandler
    (
	to_be_mapped,
	StructureNotifyMask,
	False,
	(XtEventHandler) MiscHandleFocusOnMap,
	(caddr_t) fh
    );
    /*
    ** Do some more controlling of focus behaviors.  This forces the shell
    ** into passive mode.
    */
    if (XtIsShell (to_be_mapped))
    {
	XmDeactivateWMProtocol
	(
	    to_be_mapped,
	    XmInternAtom (XtDisplay(to_be_mapped), "WM_TAKE_FOCUS", False)
	);
	XtVaSetValues (to_be_mapped, XmNinput, False, NULL);
    }

}

MISCXmStringCharCount
#if defined(_DWC_PROTO_)
    (
    XmString	cs
    )
#else
    (cs)
    XmString	cs;
#endif
{
    XmStringContext	context;
    XmStringCharSet	charset;
    XmStringDirection	dir;
    Boolean		sep;
    char		*text;
    int			count = 0;
    Boolean		static first_time = True;
    static XrmQuark	cns,
			dtscs,
			gb_0, gb_1,
			jis_0, jis_1,
			ksc_0, ksc_1,
			latin1;
    XrmQuark		Qc;

    /*
    ** Get the Quarks for comparison.
    */
    if (first_time)
    {
	latin1 = XrmStringToQuark ("ISO8859-1");
	cns   = XrmStringToQuark ("DEC.CNS11643.1986-2");
	dtscs = XrmStringToQuark ("DEC.DTSCS.1990-2");
	gb_0  = XrmStringToQuark ("GB2312.1980-0");
	gb_1  = XrmStringToQuark ("GB2312.1980-1");
	jis_0 = XrmStringToQuark ("JISX0208.1983-0");
	jis_1 = XrmStringToQuark ("JISX0208.1983-1");
	ksc_0 = XrmStringToQuark ("KSC5601.1987-0");
	ksc_1 = XrmStringToQuark ("KSC5601.1987-1");
	first_time = False;
    }

    if (!XmStringInitContext (&context, cs)) return (0);

    while (XmStringGetNextSegment (context, &text, &charset, &dir, &sep))
    {
	/*
	** Compare character sets for character size.
	*/
	Qc = XrmStringToQuark (charset);

	/*
	** Make most common be the shortest path.
	*/
	if (Qc == latin1)
	{
	    count += strlen (text);
	}
	else if (Qc == cns || Qc == dtscs || Qc == gb_0 || Qc == gb_1 ||
	    Qc == jis_0 || Qc == jis_1 || Qc == ksc_0 || Qc == ksc_1)
	{
	    count += strlen (text) / 2;
	}
	else
	{
	    count += strlen (text);
	}

	/*
	** Throw away what we got.
	*/
	XtFree (text);
	XtFree (charset);
    }

    XmStringFreeContext (context);

    return (count);
}

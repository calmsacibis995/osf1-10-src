/* dwc_ui_profile.c */
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
**	This is the module that deals with profiles
**
**--
*/

#include "dwc_compat.h"

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* for DWC$k_db_normal */

#include "dwc_ui_calendar.h"		/* CalendarDisplay */
#include "dwc_ui_day.h"			/* DAYTestAnyOpenEntries */
#include "dwc_ui_profile.h"		/* forward declarations */
#include "dwc_ui_misc.h"		/* MISCFindCalendarDisplay */
#include "dwc_ui_custom.h"		/* CUSTApplyProfile */
#include "dwc_ui_sloteditor.h"		/* for SLOTEDITORWIDGETS */
#if MEMEX
#include "dwc_ui_memex.h"		/* so that we can force	    */
					/* recompilation if we turn off */
					/* the MEMEX flag */
#endif
#include "dwc_ui_monthwidget.h"		/* for MWSTYLE */
#include "dwc_ui_errorboxes.h"

void restore_profile_settings PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	Boolean	use_profile));

/*
** External Data Declarations.
*/
extern AllDisplaysRecord ads;		/* defined in dwc_ui_calendar.c */

void PROFILESetDefaults
#ifdef _DWC_PROTO_
	(
	ProfileStructure	*profile)
#else	/* no prototypes */
	(profile)
	ProfileStructure	*profile;
#endif	/* prototype */
    {
    show_kind	*view;
    int		*temp_boolean;
    int		*temp_int;
    int		*temp_int_table;
    char	*temp_string;
    int		status;
    MrmCode	code;
    int		icon_text_len;

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_first_display",
	XtDisplay(ads.root),
	(caddr_t *)&view,
	&code
    );
    profile->first_display = *view;  /* show_month */


    /*	  
    **  The width and height numbers are in fontunits. The 14 is the size of the
    **	standard font for 75dpi screens. The extra 100 on the end is to get
    **	fixed point to two places since Marios was getting rounding errors.
    **	Hence profile->day_height = 65000 /14 gives a font-unit height of 650.00
    **	pixels for the standard font size of 14.  This can be converted to
    **	pixels on a screen with a different font by the following formula:
    **	  height = ((day_height * cd->screen_font_size) +
    **			99)/100.
    **
    **	The x and y coordinates are the center of the screen to two places in
    **	percentages, ie an x of 5000 is 50.00%. MISCGetScreenPixelPosition and
    **	MISCGetScreenFractionalPosition work with these.
    */	  
    
    profile->day_width    = 40000 / 14 ;
    profile->day_height   = 65000 / 14 ;
    profile->day_x        = 5000 ;
    profile->day_y        = 5000 ;

    profile->week_width   = 100000 / 14 ;
    profile->week_height  = 65000  / 14 ;
    profile->week_x	  = 5000 ;
    profile->week_y       = 5000 ;

    profile->month_width  = 34000 / 14;
    profile->month_height = 22500 / 14 ;

    profile->month_x      = 5000 ;
    profile->month_y      = 5000 ;

    profile->year_width   = 64000 / 14 ;
    profile->year_height  = 64000 / 14 ;
    profile->year_x       = 5000 ;
    profile->year_y       = 5000 ;

    profile->alarm_width  = 36000 / 14 ;
    profile->alarm_height = 25000 / 14 ;
    profile->alarm_x	  = 5000 ;
    profile->alarm_y	  = 5000 ;

    profile->sloteditor_width  = 45000 / 14 ;
    profile->sloteditor_height = 60000 / 14 ;
    profile->sloteditor_x      = 5000 ;
    profile->sloteditor_y      = 5000 ;

    profile->noteeditor_width  = 45000 / 14 ;
    profile->noteeditor_height = 40000 / 14 ;
    profile->noteeditor_x      = 5000 ;
    profile->noteeditor_y      = 5000 ;


    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_first_day_of_week",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->first_day_of_week = *temp_int; /* 1 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_days_to_include",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->days_to_include   = *temp_int; /* 076 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_work_days",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->work_days         = *temp_int;  /* 076 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_show_week_numbers",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->show_week_numbers = (Boolean) *temp_boolean;  /* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_week_numbers_start_day",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->week_numbers_start_day = *temp_int;   /* 4 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_week_numbers_start_month",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->week_numbers_start_month = *temp_int;   /* 1 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_timeslot_size",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->timeslot_size      = *temp_int;	/* 30 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_work_minutes_start",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->work_minutes_start = *temp_int;	/* (8 * 60) + 30 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_work_minutes_end",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->work_minutes_end   = *temp_int;	/* (17 * 60) + 30 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_day_show_months",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->day_show_months  = (Boolean) *temp_boolean;	/* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_week_show_months",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->week_show_months = (Boolean) *temp_boolean;	/* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_fill_blank_days",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->fill_blank_days  = (Boolean) *temp_boolean;	/* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_start_iconized",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->start_iconized   = (Boolean) *temp_boolean;	/* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_time_am_pm",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->time_am_pm       = (Boolean) *temp_boolean;	/* TRUE */

    profile->size_font_units  = 0 ;
    profile->screen_width     = 0 ;
    profile->screen_height    = 0 ;
    
    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_directionRtoL",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->directionRtoL       = (Boolean) *temp_boolean;   /* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_day_show_notes",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->day_show_notes      = (Boolean) *temp_boolean;   /* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_day_show_full_times",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->day_show_full_times = (Boolean) *temp_boolean;   /* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_fine_increment",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->fine_increment      = *temp_int;	    /* 5 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_lunch_minutes_start",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->lunch_minutes_start = *temp_int;	/* 12 * 60 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_lunch_minutes_end",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->lunch_minutes_end   = *temp_int;	/* 13 * 60 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_day_v_spacing",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->day_v_spacing     = *temp_int;	/* 1 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_day_h_spacing",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->day_h_spacing     = *temp_int;	/* 3 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_day_timebar_width",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->day_timebar_width = *temp_int;	/* 4 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_times_on_the_line",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->times_on_the_line = (Boolean) *temp_boolean;	/* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_timeslot_stacking",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->timeslot_stacking = (Boolean) *temp_boolean;	/* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_day_font_size",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->day_font_size = *temp_int;		/* 1 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_day_icon_size",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->day_icon_size = *temp_int;		/* 1 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_default_entry_icon",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->default_entry_icon = *temp_int;	/* k_pixmap_meeting */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_default_notes_icon",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->default_notes_icon = *temp_int;	/* k_pixmap_pencil */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_direct_scroll_coupling",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->direct_scroll_coupling = (Boolean) *temp_boolean;	/* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_entries_significant",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->entries_significant    = (Boolean) *temp_boolean;	/* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_notes_significant",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->notes_significant      = (Boolean) *temp_boolean;	/* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_icon_text",
	XtDisplay(ads.root),
	&temp_string,
	&code
    );
    icon_text_len = strlen (temp_string);
    memcpy (profile->icon_text, temp_string, icon_text_len);	/* "" */
    profile->icon_text[icon_text_len]='\0';

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_icon_show_text",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->icon_show_text = (Boolean) *temp_boolean;		/* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_icon_nl_after_text",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->icon_nl_after_text = (Boolean) *temp_boolean;	/* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_icon_show_day",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->icon_show_day      = (Boolean) *temp_boolean;    /* FALSE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_icon_full_day",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->icon_full_day      = (Boolean) *temp_boolean;    /* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_icon_nl_after_day",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->icon_nl_after_day  = (Boolean) *temp_boolean;    /* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_icon_show_time",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->icon_show_time     = (Boolean) *temp_boolean;    /* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_alarms",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int_table,
	&code
    );
    profile->alarms [0] = temp_int_table[0];	/* 0 */
    profile->alarms [1] = temp_int_table[1];	/* 5 */
    profile->alarms [2] = temp_int_table[2];	/* 30 */
    profile->alarms [3] = temp_int_table[3];	/* 120 */
    profile->alarms [4] = temp_int_table[4];	/* 10080 */
    profile->alarms [5] = temp_int_table[5];	/* 0 */
    profile->alarms [6] = temp_int_table[6];	/* 0 */
    profile->alarms [7] = temp_int_table[7];	/* 0 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_alarms_mask",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->alarms_mask = *temp_int;		/* 01 | 02 */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_print_blank_days",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->print_blank_days	 = (Boolean) *temp_boolean;   /* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_auto_click_on_clock",
	XtDisplay(ads.root),
	(caddr_t *)&temp_boolean,
	&code
    );
    profile->auto_click_on_clock = (Boolean) *temp_boolean;   /* TRUE */

    status = MrmFetchLiteral
    (
	ads.hierarchy,
	"k_profdef_month_style_mask",
	XtDisplay(ads.root),
	(caddr_t *)&temp_int,
	&code
    );
    profile->month_style_mask    = *temp_int;	    /* MWSTYLE_SQUARE */

}

void
PROFILEDO_SAVE_CURRENT_SETTINGS
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    Cardinal	    i ;
    Position	    x, y ;
    Dimension	    width, height ;
    Arg		    arglist [10] ;
    Cardinal	    ac ;
    CalendarDisplay cd;
    int		    status;


    status = MISCFindCalendarDisplay( &cd, w );

    XtSetArg (arglist [0], XmNx, &x) ;
    XtSetArg (arglist [1], XmNy, &y) ;
    XtGetValues (cd->toplevel, arglist, 2) ;
    switch (cd->showing) {
      case show_day   :
	MISCGetScreenFractionalPosition(cd,
			    (Dimension)cd->profile.day_width,
			    (Dimension)cd->profile.day_height,
			    x, y,
			    &cd->profile.day_x,
			    &cd->profile.day_y) ;
	break ;

      case show_week  :
	MISCGetScreenFractionalPosition(cd,
			    (Dimension)cd->profile.week_width,
			    (Dimension)cd->profile.week_height,
			    x, y,
			    &cd->profile.week_x,
			    &cd->profile.week_y) ;
	break ;

      case show_month :
	MISCGetScreenFractionalPosition(cd,
			    (Dimension)cd->profile.month_width,
			    (Dimension)cd->profile.month_height,
			    x, y,
			    &cd->profile.month_x,
			    &cd->profile.month_y) ;
	break ;
	
      case show_year  :
	MISCGetScreenFractionalPosition(cd,
			    (Dimension)cd->profile.year_width,
			    (Dimension)cd->profile.year_height,
			    x, y,
			    &cd->profile.year_x,
			    &cd->profile.year_y) ;
	break ;

    }


    if (cd->alarms_out_number != 0) {
	if (cd->alarms_out [0]->popup != NULL) {
	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNx, &x); ac++;
	    XtSetArg(arglist[ac], XtNy, &y); ac++;
	    XtGetValues (XtParent (cd->alarms_out [0]->popup), arglist, ac) ;

	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNwidth,  &width); ac++;
	    XtSetArg(arglist[ac], XtNheight, &height); ac++;
	    XtGetValues (cd->alarms_out [0]->popup, arglist, ac) ;

	    cd->profile.alarm_width  =
		    (Cardinal)((width * 100) / cd->screen_font_size);
	    cd->profile.alarm_height =
		    (Cardinal)((height * 100) / cd->screen_font_size);

	    MISCGetScreenFractionalPosition
		(cd,
		(Dimension)cd->profile.alarm_width,
		(Dimension)cd->profile.alarm_height,
		x, y, &cd->profile.alarm_x, &cd->profile.alarm_y) ;
	}
    }

    for (i = 0 ;  i < cd->number_of_sloteditors ; i++) {
	if (cd->ses [i]->editor == SlotEditor) {
	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNx, &x); ac++;
	    XtSetArg(arglist[ac], XtNy, &y); ac++;
	    XtGetValues
	     (XtParent (SLOTEDITORWIDGETS (cd->ses [i])
			[k_ts_popup_attached_db]), arglist, ac) ;

	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNwidth, &width); ac++;
	    XtSetArg(arglist[ac], XtNheight, &height); ac++;
	    XtGetValues
	     (SLOTEDITORWIDGETS (cd->ses [i]) [k_ts_popup_attached_db],
	      arglist, ac) ;

	    width  = (width  * 100) / cd->screen_font_size;
	    height = (height * 100) / cd->screen_font_size;

	    MISCGetScreenFractionalPosition (cd, width, height, x, y, &x, &y) ;

	    cd->profile.sloteditor_x = x ;
	    cd->profile.sloteditor_y = y ;
	    cd->profile.sloteditor_width  = (Cardinal)width ;
	    cd->profile.sloteditor_height = (Cardinal)height ;
	    break ;
	}
    }
	
    for (i = 0 ;  i < cd->number_of_sloteditors ; i++) {
	if (cd->ses [i]->editor == DaynoteEditor) {
	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNx, &x); ac++;
	    XtSetArg(arglist[ac], XtNy, &y); ac++;
	    XtGetValues
	     (XtParent (SLOTEDITORWIDGETS (cd->ses [i])
			[k_ts_popup_attached_db]), arglist, ac) ;

	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNwidth, &width); ac++;
	    XtSetArg(arglist[ac], XtNheight, &height); ac++;
	    XtGetValues
	     (SLOTEDITORWIDGETS (cd->ses [i]) [k_ts_popup_attached_db],
	      arglist, ac) ;

	    width  = (width  * 100) / cd->screen_font_size;
	    height = (height * 100) / cd->screen_font_size;

	    MISCGetScreenFractionalPosition (cd, width, height, x, y, &x, &y) ;

	    cd->profile.noteeditor_x = x ;
	    cd->profile.noteeditor_y = y ;
	    cd->profile.noteeditor_width  = (Cardinal)width ;
	    cd->profile.noteeditor_height = (Cardinal)height ;
	    break ;
	}
    }

    cd->profile.size_font_units = (Dimension)cd->screen_font_size ;
    
    PROFILEByteSwap(&cd->profile);
    if (DWC$DB_Modify_profile (cd->cab, &cd->profile,
				 sizeof (ProfileStructure)) != DWC$k_db_normal) {
	ERRORDisplayErrno (cd->mainwid, "ErrorWriteProfile") ;
    }
    PROFILEByteSwap(&cd->profile);
}

void
restore_profile_settings
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag,
	Boolean	use_profile)
#else	/* no prototypes */
	(w, tag, use_profile)
	Widget	w;
	caddr_t	tag;
	Boolean	use_profile;
#endif	/* prototype */
    {
    Position	    main_x ;
    Position	    main_y ;
    Dimension	    main_width ;
    Dimension	    main_height ;
    Dimension	    profile_format_width ;
    Dimension	    profile_format_height ;
    Arg		    argldim [2] ;
    Arg		    arglpos [2] ;
    Arg		    arglist [4] ;
    Cardinal	    ac ;
    Position	    x, y ;
    Dimension	    width, height ;
    Cardinal	    i ;
    CalendarDisplay cd ;
    ProfileStructure restored_profile ;
    int		    status ;

    status = MISCFindCalendarDisplay (&cd, w);

    PROFILESetDefaults(&restored_profile);
    if (use_profile)
    {
	PROFILEByteSwap(&restored_profile);
	status = DWC$DB_Get_profile
	    (cd->cab, sizeof(ProfileStructure), &restored_profile);
	if (status != DWC$k_db_normal)
	{
	    ERRORDisplayErrno (cd->mainwid, "ErrorGetProfile") ;
	}
	PROFILEByteSwap(&restored_profile);
    }

    if (cd->db_general_created)
    {
	CUSTSetGeneralFromProfile(cd, &restored_profile) ;
    }

    if (cd->db_dayview_created)
    {
	CUSTSetDayviewFromProfile(cd, &restored_profile) ;
    }

    if (cd->db_iconbox_created)
    {
	CUSTSetIconboxFromProfile(cd, &restored_profile) ;
    }

    if (cd->db_alarms_created)
    {
	CUSTSetAlarmsFromProfile(cd,  &restored_profile) ;
    }

    if (! CUSTApplyProfile ( cd, &restored_profile ))
    {
	return ;
    }

    cd->profile.day_height   = restored_profile.day_height ;
    cd->profile.day_width    = restored_profile.day_width ;
    cd->profile.day_x        = restored_profile.day_x ;
    cd->profile.day_y        = restored_profile.day_y ;

    cd->profile.week_height  = restored_profile.week_height ;
    cd->profile.week_width   = restored_profile.week_width ;
    cd->profile.week_x       = restored_profile.week_x ;
    cd->profile.week_y       = restored_profile.week_y ;

    cd->profile.month_height = restored_profile.month_height ;
    cd->profile.month_width  = restored_profile.month_width ;
    cd->profile.month_x      = restored_profile.month_x ;
    cd->profile.month_y      = restored_profile.month_y ;

    cd->profile.year_height  = restored_profile.year_height ;
    cd->profile.year_width   = restored_profile.year_width ;
    cd->profile.year_x       = restored_profile.year_x ;
    cd->profile.year_y       = restored_profile.year_y ;

    cd->profile.alarm_height = restored_profile.alarm_height ;
    cd->profile.alarm_width  = restored_profile.alarm_width ;
    cd->profile.alarm_x      = restored_profile.alarm_x ;
    cd->profile.alarm_y      = restored_profile.alarm_y ;

    cd->profile.sloteditor_height  = restored_profile.sloteditor_height ;
    cd->profile.sloteditor_width   = restored_profile.sloteditor_width ;
    cd->profile.sloteditor_x       = restored_profile.sloteditor_x ;
    cd->profile.sloteditor_y       = restored_profile.sloteditor_y ;

    cd->profile.noteeditor_height  = restored_profile.noteeditor_height ;
    cd->profile.noteeditor_width   = restored_profile.noteeditor_width ;
    cd->profile.noteeditor_x       = restored_profile.noteeditor_x ;
    cd->profile.noteeditor_y       = restored_profile.noteeditor_y ;

    PROFILEOldDBProfileHack (cd) ;

    switch (cd->showing) {
      case show_day   :
	profile_format_width  = (Dimension)cd->profile.day_width ;
	profile_format_height = (Dimension)cd->profile.day_height ;
	main_x      = cd->profile.day_x ;
	main_y      = cd->profile.day_y ;
	break ;

      case show_week  :
	profile_format_width  = (Dimension)cd->profile.week_width ;
	profile_format_height = (Dimension)cd->profile.week_height ;
	main_x      = cd->profile.week_x ;
	main_y      = cd->profile.week_y ;
	break ;

      case show_month :
	profile_format_width  = (Dimension)cd->profile.month_width ;
	profile_format_height = (Dimension)cd->profile.month_height ;
	main_x      = cd->profile.month_x ;
	main_y      = cd->profile.month_y ;
	break ;

      case show_year  :
	profile_format_width  = (Dimension)cd->profile.year_width ;
	profile_format_height = (Dimension)cd->profile.year_height ;
	main_x      = cd->profile.year_x ;
	main_y      = cd->profile.year_y ;
	break ;
    }

    MISCGetScreenPixelPosition(cd,
			main_x, main_y,
			profile_format_width, profile_format_height,
			&main_width, &main_height,
			&main_x, &main_y) ;

    XtSetArg (argldim [0], XmNwidth, main_width);
    XtSetArg (argldim [1], XmNheight, main_height);
    XtSetValues (cd->mainwid,  argldim, 2) ;


    XtSetArg (arglpos [0], XmNx, main_x) ;
    XtSetArg (arglpos [1], XmNy, main_y) ;
    XtSetValues (cd->toplevel, arglpos, 2) ;

    /*	  
    **  We've been having timing problems, ie things coming up before the
    **  SetValues seemed to have taken place. Throw this in to see if it
    **  helps.  PGLF 8/28/90
    */	  
    XFlush(XtDisplay(cd->mainwid));


    if (cd->alarms_out_number != 0) {
	if (cd->alarms_out [0]->popup != NULL) {
	    MISCGetScreenPixelPosition
		(cd, cd->profile.alarm_x, cd->profile.alarm_y,
		(Dimension)cd->profile.alarm_width,
		(Dimension)cd->profile.alarm_height,
		&width, &height,
		&x, &y) ;

	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNx, x);ac++;
	    XtSetArg(arglist[ac], XtNy, y); ac++;
	    XtSetArg(arglist[ac], XtNwidth, width); ac++;
	    XtSetArg(arglist[ac], XtNheight, height); ac++;
	    XtSetValues (cd->alarms_out [0]->popup, arglist, ac) ;
	}
    }

    for (i = 0 ;  i < cd->number_of_sloteditors ; i++) {
	if (cd->ses [i]->editor == SlotEditor) {
	    MISCGetScreenPixelPosition(cd,
			    cd->profile.sloteditor_x,
			    cd->profile.sloteditor_y,
			    (Dimension)cd->profile.sloteditor_width,
			    (Dimension)cd->profile.sloteditor_height,
			    &width, &height,
			    &x, &y) ;

	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNwidth, width); ac++;
	    XtSetArg(arglist[ac], XtNheight, height); ac++;
	    XtSetValues 
	     (SLOTEDITORWIDGETS (cd->ses [i]) [k_ts_popup_attached_db],
	      arglist, ac) ;

	    /*	  
	    **	The form was moving down and to the right upon successive
	    **	invocations of the sloteditor. We'll set the x and y on the
	    **	parent instead of on the sloteditor.
	    */	  
	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNx, x);ac++;
	    XtSetArg(arglist[ac], XtNy, y); ac++;
	    XtSetValues 
	     (XtParent(SLOTEDITORWIDGETS(cd->ses[i])[k_ts_popup_attached_db]),
	      arglist, ac);

	    break ;
	}
    }

    for (i = 0 ;  i < cd->number_of_sloteditors ; i++) {
	if (cd->ses [i]->editor == DaynoteEditor) {
	    MISCGetScreenPixelPosition(cd,
			    cd->profile.noteeditor_x,
			    cd->profile.noteeditor_y,
			    (Dimension)cd->profile.noteeditor_width,
			    (Dimension)cd->profile.noteeditor_height,
			    &width, &height,
			    &x, &y) ;

	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNwidth, width); ac++;
	    XtSetArg(arglist[ac], XtNheight, height); ac++;
	    XtSetValues 
	     (SLOTEDITORWIDGETS (cd->ses [i]) [k_ts_popup_attached_db],
	      arglist, ac) ;

	    /*	  
	    **	The form was moving down and to the right upon successive
	    **	invocations of the sloteditor. We'll set the x and y on the
	    **	parent instead of on the sloteditor.
	    */	  
	    ac = 0 ;
	    XtSetArg(arglist[ac], XtNx, x);ac++;
	    XtSetArg(arglist[ac], XtNy, y); ac++;
	    XtSetValues 
	     (XtParent(SLOTEDITORWIDGETS(cd->ses[i])[k_ts_popup_attached_db]),
	      arglist, ac);

	    break ;
	}
    }

}

void
PROFILEDO_RESTORE_SETTINGS
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd;
    int			status;

    status = MISCFindCalendarDisplay( &cd, w );

    if (DAYTestAnyOpenEntries (cd)) {
	ERRORDisplayError (cd->mainwid, "ErrorCloseAllRestore") ;
	return ;
    }

    restore_profile_settings (w, tag, TRUE) ;

}

void
PROFILEDO_USE_DEFAULT_SETTINGS
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag)
#else	/* no prototypes */
	(w, tag)
	Widget	w;
	caddr_t	tag;
#endif	/* prototype */
    {
    CalendarDisplay cd;
    int			status;

    status = MISCFindCalendarDisplay( &cd, w );

    if (DAYTestAnyOpenEntries (cd)) {
	ERRORDisplayError (cd->mainwid, "ErrorCloseAllRestore") ;
	return ;
    }

    restore_profile_settings (w, tag, FALSE) ;

}

void PROFILEOldDBProfileHack
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {

    /*
    **  Real hack to get old dbs....
    */
    
    if (cd->profile.week_width == 1000) {
	cd->profile.day_height   = (cd->profile.day_height   * 100) / 14 ;
	cd->profile.day_width    = (cd->profile.day_width    * 100) / 14 ;
	cd->profile.week_height  = (cd->profile.week_height  * 100) / 14 ;
	cd->profile.week_width   = (cd->profile.week_width   * 100) / 14 ;
	cd->profile.month_height = (cd->profile.month_height * 100) / 14 ;
	cd->profile.month_width  = (cd->profile.month_width  * 100) / 14 ;
	cd->profile.year_width   = (cd->profile.year_width   * 100) / 14 ;
	cd->profile.year_height  = (cd->profile.year_height  * 100) / 14 ;

	MISCGetScreenFractionalPosition
	    (cd,
	    (Dimension)cd->profile.day_width,
	    (Dimension)cd->profile.day_height,
	    cd->profile.day_x, cd->profile.day_y,
	    &cd->profile.day_x, &cd->profile.day_y) ;

	MISCGetScreenFractionalPosition
	    (cd,
	    (Dimension)cd->profile.week_width,
	    (Dimension)cd->profile.week_height,
	    cd->profile.week_x, cd->profile.week_y,
	    &cd->profile.week_x, &cd->profile.week_y) ;

	MISCGetScreenFractionalPosition
	    (cd,
	    (Dimension)cd->profile.month_width,
	    (Dimension)cd->profile.month_height,
	    cd->profile.month_x, cd->profile.month_y,
	    &cd->profile.month_x, &cd->profile.month_y) ;

	MISCGetScreenFractionalPosition
	    (cd,
	    (Dimension)cd->profile.year_width,
	    (Dimension)cd->profile.year_height,
	    cd->profile.year_x, cd->profile.year_y,
	    &cd->profile.year_x, &cd->profile.year_y) ;
    }

}

void PROFILEByteSwap
#ifdef _DWC_PROTO_
	(ProfileStructure *Profile)
#else
	(Profile)
	ProfileStructure *Profile;
#endif
{
#if BYTESWAP
	int i;
	DWC$$DB_Byte_swap_field (&Profile->first_display, sizeof(show_kind));
	DWC$$DB_Byte_swap_field (&Profile->day_width, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->day_height, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->week_width, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->week_height, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->month_width, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->month_height, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->year_width, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->year_height, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field
	    (&Profile->work_minutes_start, sizeof(short int));
	DWC$$DB_Byte_swap_field (&Profile->work_minutes_end, sizeof(short int));
	DWC$$DB_Byte_swap_field (&Profile->day_x, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->day_y, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->week_x, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->week_y, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->month_x, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->month_y, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->year_x, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->year_y, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->size_font_units, sizeof(Dimension));
	DWC$$DB_Byte_swap_field (&Profile->screen_width, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->screen_height, sizeof(Position));
	DWC$$DB_Byte_swap_field
	    (&Profile->lunch_minutes_start, sizeof(short int));
	DWC$$DB_Byte_swap_field
	    (&Profile->lunch_minutes_end, sizeof(short int));
	DWC$$DB_Byte_swap_field (&Profile->alarm_x, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->alarm_width, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->alarm_height, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->alarm_y, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->sloteditor_x, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->sloteditor_width, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->sloteditor_height, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->sloteditor_y, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->noteeditor_x, sizeof(Position));
	DWC$$DB_Byte_swap_field (&Profile->noteeditor_width, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->noteeditor_height, sizeof(Cardinal));
	DWC$$DB_Byte_swap_field (&Profile->noteeditor_y, sizeof(Position));
	for (i=0; i<8; i++)
	    DWC$$DB_Byte_swap_field (&Profile->alarms[i], sizeof(short int));
#endif
}

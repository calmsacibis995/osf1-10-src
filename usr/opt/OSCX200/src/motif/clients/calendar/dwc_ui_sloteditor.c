/* dwc_ui_sloteditor.c */
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
**	Marios Cleovoulou,  November-1987
**	Ken Cowan,	    February-1989
**	Denis Lacroix,	    February-1989
**
**  ABSTRACT:
**
**	This module contains the support for time-slot editing.
**
**  MODIFICATION HISTORY:
**
**      td      11/22/93
**              Tested and put in single fix for  QAR 17085 
**		This also makes the following QARs go away:
**			17052 16785 05162 13873 13891 14343 00172 
**
**--
*/

#include "dwc_compat.h"

#if defined(vaxc)
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <Xm/ToggleB.h>
#include <DXm/DECspecific.h>
#include <Xm/Protocols.h>
#if defined(vaxc)
#pragma standard
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* for DWC$k_db_normal */

#include "dwc_ui_calendar.h"		    /* for sloteditor stuff? */
#include "dwc_ui_catchall.h"		    /* DWC$DRM */
#include "dwc_ui_clipboard.h"		    /* DO_TSE_CUT... */
#include "dwc_ui_datestructures.h"	    /* dtb */
#include "dwc_ui_day.h"			    /* DAYCreateDayItemUpdateRecord */
#include "dwc_ui_edit.h"		    /* DO_TSE_EDIT_MENU */
#include "dwc_ui_iconboxwidget.h"	    /* DwcIbwCallbackStruct */
#include "dwc_ui_misc.h"		    /* k_include_success... */
#include "dwc_ui_sloteditor.h"		    /* forward declarations */
#include "dwc_ui_uil_values_const.h"	    /* dwc_k_ts... */
#include "dwc_ui_dateformat.h"		    /* for DATEFORMATTimeToCS */
#include "dwc_ui_icons.h"
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_errorboxes.h"
#include "dwc_ui_help.h"

#undef _XmGetFlagsBit
#define _XmGetFlagsBit(field, bit) \
      ((field[ ((unsigned int)bit >> 3) ]) & (1 << ((unsigned int)bit & 0x07)))

static void SE_get_entry_alarms PROTOTYPE ((
	CalendarDisplay		cd,
	int			*alarms_number_return,
	unsigned short int	**alarms_times_return));

static void SE_update_scrollbar PROTOTYPE ((
	Sloteditor	se,
	Boolean		directionRtoL));

static void SE_close_action PROTOTYPE ((
	Widget		w,
	XEvent		*keyevent));

static void SE_do_text_cb PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_timeslot_delete_cb PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_timeslot_reset_cb PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_timeslot_cancel_cb PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_timeslot_close_cb PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_create_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*callback_data));

static void SE_formdialog_create_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*callback_data));

static void SE_text_changed_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_alarm_changed_cb PROTOTYPE ((
	Widget				widget,
	caddr_t				*tag,
	XmToggleButtonCallbackStruct	*cb));

static void SE_alarm_value_changed_cb PROTOTYPE ((
	Widget				widget,
	caddr_t				*tag,
	XmToggleButtonCallbackStruct	*cb));

static void SE_sloteditor_alarm_scale_changed_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmScaleCallbackStruct	*cb));

static void SE_include_activate_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*callback_data));

static void SE_repeat_interval_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_attibutes_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_special_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_condition_value_changed_cb PROTOTYPE ((
	Widget				widget,
	caddr_t				*tag,
	XmToggleButtonCallbackStruct	*cb));

static void SE_work_nonwork_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_move_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_select_iconbox_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	DwcIbwCallbackStruct	*cb));

static void SE_order_iconbox_cb PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	DwcIbwCallbackStruct	*cb));

static void SE_flag_changed_cb PROTOTYPE ((
	Widget				widget,
	caddr_t				*tag,
	XmToggleButtonCallbackStruct	*cbs));

static void SE_find_sloteditor PROTOTYPE ((
	Sloteditor	*se,
	CalendarDisplay	cd,
	Widget		widget,
	int		index));

static void SE_setup_labels PROTOTYPE ((
	Sloteditor	se));

static void SE_setup_alarms PROTOTYPE ((
	Sloteditor	se));

static void SE_setup_alarm_scale PROTOTYPE ((
	Sloteditor	se,
	int		which));

static void SE_setup_alarm_value PROTOTYPE ((
	Sloteditor	se,
	int		which,
	Cardinal	minutes_prior));

static void SE_setup_icons PROTOTYPE ((Sloteditor se));

static void SE_setup_repeats PROTOTYPE ((Sloteditor se));

static void SE_setup_repeat_attributes PROTOTYPE ((Sloteditor se));

static void SE_setup_repeat_conditions PROTOTYPE ((Sloteditor se));

static void SE_setup_flags PROTOTYPE ((Sloteditor se));

static void SE_readonly_editor PROTOTYPE ((Sloteditor se));

/*
**  
*/
static void SE_get_alarms PROTOTYPE ((Sloteditor se, DwcDayItem new));

static void SE_get_icons PROTOTYPE ((Sloteditor se, DwcDayItem new));

static void SE_get_repeats PROTOTYPE ((Sloteditor se, DwcDayItem new));

static void SE_get_repeat_attributes PROTOTYPE((Sloteditor se, DwcDayItem new));

static void SE_get_repeat_conditions PROTOTYPE ((
	Sloteditor se,
	DwcDayItem new));

static void SE_get_flags PROTOTYPE ((Sloteditor se, DwcDayItem new));

static void SE_tse_include_cb PROTOTYPE ((
	Widget					w,
	caddr_t					*tag,
	XmFileSelectionBoxCallbackStruct	*cbs));

static void SE_tse_include_unmap_cb PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

static void SE_create_tse PROTOTYPE ((Sloteditor se));

static void SE_create_dne PROTOTYPE ((Sloteditor se));

static void SE_fillin_tse PROTOTYPE ((Sloteditor se));

static void SE_fillin_dne PROTOTYPE ((Sloteditor se));

static void SE_save_editor_position PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	first_editor));


/*
**
*/
extern AllDisplaysRecord ads;		    /* defined in dwc_ui_calendar.c */
extern Pixmap	medium_pixmaps[];	    /* defined in dwc_ui_misc.c */


/*									    
**  Static Data Definitions
*/

static Sloteditor static_sloteditor;


void SEUpdateSlotEditorScrollBar
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Boolean		directionRtoL)
#else	/* no prototypes */
	(cd, directionRtoL)
	CalendarDisplay	cd;
	Boolean		directionRtoL;
#endif	/* prototype */
{
    int index;
    
    for (index=0; index< cd->number_of_sloteditors; index++)
	{
	SE_update_scrollbar(cd->ses[index], directionRtoL);
	}
    
    return;
}

static void SE_update_scrollbar
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	Boolean		directionRtoL)
#else	/* no prototypes */
	(se, directionRtoL)
	Sloteditor	se;
	Boolean		directionRtoL;
#endif	/* prototype */
{

    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext],
	XmNscrollLeftSide, directionRtoL,
	NULL
    );

    return;    
}

Boolean SEAnySlotEditorsUpAndRunning
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {
    Boolean return_value = FALSE;
    int	    index;
        
    for (index=0; index< cd->number_of_sloteditors; index++)
	{
	if ( (cd->ses[index]->in_use) &&
	     (cd->ses[index]->editor == SlotEditor) )
	    {
	    return_value = TRUE;
	    break;
	    }
	}
	
    /*
    **  That's it
    */
    return(return_value);
    }

Boolean SEAnyNoteEditorsUpAndRunning
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {
    Boolean return_value = FALSE;
    int	    index;
    
    for (index=0; index< cd->number_of_sloteditors; index++)
	{
	if ( (cd->ses[index]->in_use) &&
	     (cd->ses[index]->editor == DaynoteEditor) )
	    {
	    return_value = TRUE;
	    break;
	    }
	}

    /*
    **  That's it
    */
    return(return_value);
    }

static void SE_setup_alarms
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
{
    CalendarDisplay cd = se->cd;
    Boolean	    entry_time_toggle	    = FALSE;
    Boolean	    short_toggle	    = FALSE;
    Boolean	    medium_toggle	    = FALSE;
    Boolean	    long_toggle		    = FALSE;
    Boolean	    vlong_toggle	    = FALSE;
    int		    i;
    Boolean	    one_is_on		    = FALSE;
    int		    which_one;
            
    for (i=0; i < MIN(5, se->old->alarms_number); i++)
    {
	if ( se->old->alarms_times[i] == 0 )
	{
	    /*
	    **  At entry time
	    */
	    entry_time_toggle = TRUE;
	}
	else if ( (se->old->alarms_times[i] <= 15) && !short_toggle )
	{
	    /*
	    **  Short alarm
	    */
	    short_toggle = TRUE;
	    se->scale_latest_value[0] = se->old->alarms_times[i];
	}
	else if ( (se->old->alarms_times[i] <= 60) && !medium_toggle )
	{
	    /*
	    **  Medium alarm
	    */
	    medium_toggle = TRUE;
	    se->scale_latest_value[1] = se->old->alarms_times[i];
	}
	else if ( (se->old->alarms_times[i] <= 24*60) && !long_toggle )
	{
	    /*
	    **  Long alarm
	    */	    
	    long_toggle = TRUE;
	    se->scale_latest_value[2] = se->old->alarms_times[i];
	}
	else
	{
	    /*
	    **  Very long alarm
	    */
	    vlong_toggle = TRUE;
	    se->scale_latest_value[3] = se->old->alarms_times[i];
	}
    }

    /*
    **
    */
    XmToggleButtonSetState
    (
	SLOTEDITORWIDGETS(se)[k_ts_entry_time_toggle],
	entry_time_toggle,
	FALSE
    );

    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_entry_time_toggle],
	XmNvalue, entry_time_toggle,
	NULL
    );
    
    /*
    **
    */
    XmToggleButtonSetState (SLOTEDITORWIDGETS(se)[k_ts_short_toggle],
	short_toggle, FALSE);
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_short_toggle],
	XmNvalue, short_toggle,
	NULL
    );

    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_short_val_toggle], short_toggle);
    XmToggleButtonSetState
	(SLOTEDITORWIDGETS(se)[k_ts_short_val_toggle], FALSE, FALSE);
    if (!short_toggle)
    {
	se->scale_latest_value [0] = cd->profile.alarms [1];
    }
    else if (!one_is_on)
    {
	one_is_on = TRUE;
	which_one = k_ts_short_val_toggle;
    }
    SE_setup_alarm_value
	(se, k_ts_short_val_toggle, se->scale_latest_value[0]);
    
    /*
    **
    */
    XmToggleButtonSetState
	(SLOTEDITORWIDGETS(se)[k_ts_medium_toggle], medium_toggle, FALSE);
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_medium_toggle],
	XmNvalue, medium_toggle,
	NULL
    );

    XtSetSensitive
	(SLOTEDITORWIDGETS(se)[k_ts_medium_val_toggle], medium_toggle);
    XmToggleButtonSetState
	(SLOTEDITORWIDGETS(se)[k_ts_medium_val_toggle], FALSE, FALSE);
    if (!medium_toggle)
    {
	se->scale_latest_value[1] = cd->profile.alarms [2];
    }
    else if (!one_is_on)
    {
	one_is_on = TRUE;
	which_one = k_ts_medium_val_toggle;
    }
    SE_setup_alarm_value
	(se, k_ts_medium_val_toggle, se->scale_latest_value[1]);

    /*
    **
    */
    XmToggleButtonSetState
	(SLOTEDITORWIDGETS(se)[k_ts_long_toggle], long_toggle, FALSE);
    XtVaSetValues
	(SLOTEDITORWIDGETS(se)[k_ts_long_toggle], XmNvalue, long_toggle, NULL);

    XtSetSensitive
	(SLOTEDITORWIDGETS(se)[k_ts_long_val_toggle], long_toggle);
    XmToggleButtonSetState
	(SLOTEDITORWIDGETS(se)[k_ts_long_val_toggle], FALSE, FALSE);
    if (!long_toggle)
    {
	se->scale_latest_value[2] = cd->profile.alarms [3];
    }
    else if (!one_is_on)
    {
	one_is_on = TRUE;
	which_one = k_ts_long_val_toggle;
    }
    SE_setup_alarm_value
	(se, k_ts_long_val_toggle, se->scale_latest_value[2]);

    /*
    **
    */
    XmToggleButtonSetState
	(SLOTEDITORWIDGETS(se)[k_ts_vlong_toggle], vlong_toggle, FALSE);
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_vlong_toggle],
	XmNvalue, vlong_toggle,
	NULL
    );

    XtSetSensitive
	(SLOTEDITORWIDGETS(se)[k_ts_vlong_val_toggle], vlong_toggle);
    XmToggleButtonSetState
	(SLOTEDITORWIDGETS(se)[k_ts_vlong_val_toggle], FALSE, FALSE);
    if (!vlong_toggle)
    {
	se->scale_latest_value[3] = cd->profile.alarms [4];
    }
    else if (!one_is_on)
    {
	one_is_on = TRUE;
	which_one = k_ts_vlong_val_toggle;
    }
    SE_setup_alarm_value
	(se, k_ts_vlong_val_toggle, se->scale_latest_value[3]);
    
    /*
    **
    */
    if (one_is_on)
    {
	se->current_scale = which_one;
	SE_setup_alarm_scale (se, se->current_scale);
	XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_alarm_scale], TRUE);
	XmToggleButtonSetState
	    (SLOTEDITORWIDGETS(se)[se->current_scale], TRUE, FALSE);
    }
    else
    {
	se->current_scale = 0;
	SE_setup_alarm_scale (se, k_ts_short_val_toggle);
	XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_alarm_scale], FALSE);
    }

    /*
    **  That's it
    */
    return;
}

static void SE_setup_alarm_scale
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	int		which)
#else	/* no prototypes */
	(se, which)
	Sloteditor	se;
	int		which;
#endif	/* prototype */
    {
    int		    min;
    int		    max;
    short	    points;
    int		    value;
    XmString   scale_title;        
      
    /*
    **	
    */
    se->ignore_scale_value_changed = TRUE;
    
    switch (which)
	{
	case k_ts_short_val_toggle:
	    min = 1;
	    max = 15;
	    points = 0;
	    value = se->scale_latest_value[which - k_ts_short_val_toggle];
	    scale_title = MISCFetchDRMValue(dwc_k_ts_short_scale_title);
	    break;
	case k_ts_medium_val_toggle:
	    min = 1;
	    max = 60;
	    points = 0;
	    value = se->scale_latest_value[which - k_ts_short_val_toggle];
	    scale_title = MISCFetchDRMValue(dwc_k_ts_medium_scale_title);
	    break;
	case k_ts_long_val_toggle:
	    min = 10;
	    max = 240;
	    points = 1;
	    value = se->scale_latest_value[which - k_ts_short_val_toggle] / 6;
	    scale_title = MISCFetchDRMValue(dwc_k_ts_long_scale_title);
	    break;
	case k_ts_vlong_val_toggle:
	    min = 10;
	    max = 10 * DWC$k_db_max_alarm_time / DWC$k_db_calendar_precision;
	    value = se->scale_latest_value[which - k_ts_short_val_toggle] * 10
			/ 1440;
	    points = 1;
	    scale_title = MISCFetchDRMValue(dwc_k_ts_vlong_scale_title);
	    break;
	}

    XtUnmanageChild( SLOTEDITORWIDGETS(se)[k_ts_alarm_scale] );
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_alarm_scale],
	XmNtitleString, scale_title,
	XmNminimum, min,
	XmNmaximum, max,
	XmNvalue, value,
	XmNdecimalPoints, points,
	NULL
    );
    XtManageChild( SLOTEDITORWIDGETS(se)[k_ts_alarm_scale] );
    
    /*
    **	
    */
    se->ignore_scale_value_changed = FALSE;

    /*
    **  That's it
    */
    return;
}

static void SE_setup_alarm_value
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	int		which,
	Cardinal	minutes_prior)
#else	/* no prototypes */
	(se, which, minutes_prior)
	Sloteditor	se;
	int		which;
	Cardinal	minutes_prior;
#endif	/* prototype */
{
    int		    msbot;
    dtb		    date_time;
    int		    day,
		    month,
		    year;
    XmString	    text;

    msbot = se->dsbot*1440 + se->start_time - minutes_prior;

    DATEFUNCDateForDayNumber (msbot / 1440, &day, &month, &year);
    date_time.weekday = DATEFUNCDayOfWeek (day, month, year);	
    date_time.day   = day;
    date_time.month = month;
    date_time.year  = year;
    msbot = msbot % 1440;
    date_time.hour	= msbot / 60;
    date_time.minute	= msbot % 60;

    switch (which)
    {
    case k_ts_short_val_toggle:
	if (se->cd->profile.time_am_pm)
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_short_ampm_format, &date_time);
	else
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_short_format, &date_time);
	break;
    case k_ts_medium_val_toggle:
	if (se->cd->profile.time_am_pm)
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_medium_ampm_format, &date_time);
	else
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_medium_format, &date_time);
	break;
    case k_ts_long_val_toggle:
	if (se->cd->profile.time_am_pm)
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_long_ampm_format, &date_time);
	else
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_long_format, &date_time);
	break;
    case k_ts_vlong_val_toggle:
	if (se->cd->profile.time_am_pm)
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_vlong_ampm_format, &date_time);
	else
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_vlong_format, &date_time);
	break;
    }

    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[which],
	XmNlabelString, text,
	NULL
    );

    XmStringFree (text);
    	
    /*
    **  That's it
    */
    return;
}

static void SE_setup_icons
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
{
    int	    i;
    
    /*
    **  Let's set up the select icon box widget
    */
    DwcIbwSetSelectedIcons
    (
	SLOTEDITORWIDGETS(se)[k_ts_select_ibw],
	se->old->icons,
	se->old->icons_number
    );

    /*
    **  Let's set up the order icon box widget
    */
    DwcIbwSetSelectedIcons
    (
	SLOTEDITORWIDGETS(se)[k_ts_order_ibw],
	se->old->icons,
	se->old->icons_number
    );

    /*
    **  That's it
    */
    return;
}

static void SE_setup_repeats
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
    {
    Widget	local_widget;
    Arg		arglist[1];
    Cardinal	repeat_interval_index;
    Cardinal	attributes_index;
    								      
    /*
    **  Let's cleanup the dialog box (the option menus may have retained some
    **	old values, and the toggle buttons too)
    */

    /*
    **  Default repeat interval is NONE
    */
    XtSetArg(arglist[0], XmNmenuHistory,
	SLOTEDITORWIDGETS(se)[k_ts_none_button]);
    XtSetValues(SLOTEDITORWIDGETS(se)[k_ts_repeat_optmenu],
	arglist, 1);
    se->repeat_interval = k_ts_none_button;

    /*
    **  The attributes option menu is insensitive by default
    */
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_attributes_optmenu], FALSE);
		
    /*
    **  The condition toggle and the work/nonwork and move option menu have a
    **	default value but are insensitive by default
    */
    XtSetArg(arglist[0], XmNmenuHistory,
	SLOTEDITORWIDGETS(se)[k_ts_nonwork_button]);
    XtSetValues(SLOTEDITORWIDGETS(se)[k_ts_work_nonwork_optmenu],
	arglist, 1);
    se->work_nonwork = k_ts_nonwork_button;

    XtSetArg(arglist[0], XmNmenuHistory,
	SLOTEDITORWIDGETS(se)[k_ts_skip_button]);
    XtSetValues(SLOTEDITORWIDGETS(se)[k_ts_move_optmenu],
	arglist, 1);
    se->move = k_ts_skip_button;

    XmToggleButtonSetState(SLOTEDITORWIDGETS(se)[k_ts_condition_toggle],
	FALSE, FALSE);
    se->condition = FALSE;
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_condition_toggle], FALSE);
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_work_nonwork_optmenu], FALSE);
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_move_optmenu], FALSE);
			
    if (se->old->item_id == 0)
        {
	/*
	**  New item
	*/
	}
    else
        {
	/*
	**
	*/
        switch (se->old->repeat_p1)
	    {
	    case DWC$k_db_none:
		repeat_interval_index = k_ts_none_button;
		se->repeat_interval = repeat_interval_index;
		break;
	    case DWC$k_db_absolute:
		switch (se->old->repeat_p2)
		    {
		    case 1440:	    /* 24 * 60				    */
			repeat_interval_index = k_ts_daily_button;	
			break;
		    case 10080:	    /* 24 * 60 * 7			    */
			repeat_interval_index = k_ts_weekly_button;
			break;
		    case 20160:	    /* 24 * 60 * 7 * 2			    */
			repeat_interval_index = k_ts_fortnight_button;
			break;
		    case 40320:	    /* 24 * 60 * 7 * 4			    */
			repeat_interval_index = k_ts_fourweek_button;
			break;
		    }
		XtSetArg(arglist[0], XmNmenuHistory,
		    SLOTEDITORWIDGETS(se)[repeat_interval_index]);
		XtSetValues(SLOTEDITORWIDGETS(se)[k_ts_repeat_optmenu],
		    arglist, 1);
		se->repeat_interval = repeat_interval_index;
		break;
	    case DWC$k_db_abscond:
		switch (se->old->repeat_p2)
		    {
		    case 1:
			repeat_interval_index = k_ts_daily_button;	
			break;
		    case 7:
			repeat_interval_index = k_ts_weekly_button;
			break;
		    case 14:
			repeat_interval_index = k_ts_fortnight_button;
			break;
		    case 28:
			repeat_interval_index = k_ts_fourweek_button;
			break;
		    }
		XtSetArg(arglist[0], XmNmenuHistory,
		    SLOTEDITORWIDGETS(se)[repeat_interval_index]);
		XtSetValues(SLOTEDITORWIDGETS(se)[k_ts_repeat_optmenu],
		    arglist, 1);
		se->repeat_interval = repeat_interval_index;
		break;
	    case DWC$k_db_nth_day:
	    case DWC$k_db_nth_day_end:
	    case DWC$k_db_nth_xday:
	    case DWC$k_db_last_weekday:
	    case DWC$k_db_nth_day_cwd:

	    	switch (se->old->repeat_p2)
		    {
		    case 1:
			repeat_interval_index = k_ts_monthly_button;
			break;
		    case 2:
			repeat_interval_index = k_ts_bimonthly_button;	
			break;
		    case 3:
			repeat_interval_index = k_ts_quarterly_button;	
			break;
		    case 4:
			repeat_interval_index = k_ts_triannually_button;	
			break;
		    case 6:
			repeat_interval_index = k_ts_biannually_button;	
			break;
		    case 12:
			repeat_interval_index = k_ts_annually_button;	
			break;
		    }
		
		XtSetArg(arglist[0], XmNmenuHistory,
		    SLOTEDITORWIDGETS(se)[repeat_interval_index]);
		XtSetValues(SLOTEDITORWIDGETS(se)[k_ts_repeat_optmenu],
		    arglist, 1);

		se->repeat_interval = repeat_interval_index;

		XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_attributes_optmenu],
		    TRUE);
		break;
	    default:			    
		break;
	    /* !!! there's a missing one here! Fix that! */
	    }	
	}    

    SE_setup_repeat_attributes(se);

    SE_setup_repeat_conditions(se);
        
    /*
    **  That's it
    */
    return;
    } 

static void SE_setup_repeat_attributes
#if defined(_DWC_PROTO_)
	(Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
{
    XmString	    text;
    Widget	    widget;
    dtb		    date_time;
    int		    day,
		    month,
		    year,
		    weekday;
    int		    cwd_day_of_month;
    int		    cwd_day_of_week;
    int		    status;
    int		    P4,
		    P5,
		    P6;
    Cardinal	    repeat_attributes_index;
    Cardinal	    dummy_index;
    struct DWC$db_time_ctx
		    time_context;
    int		    index;
    int		    temp_day;
    
    DATEFUNCDateForDayNumber (se->dsbot, &day, &month, &year);
    weekday		= DATEFUNCDayOfWeek (day, month, year);
    date_time.weekday	= weekday;
    date_time.day	= day;
    date_time.month	= month;
    date_time.year	= year;


    /*
    **	Repeat 'monthly' 'on the 23rd day'
    */    
    if ( (se->old->item_id != 0) &&
	(se->old->repeat_p1 == DWC$k_db_nth_day) )
    {
	status = DWC$DB_Examine_r_params
	    (se->cd->cab, se->old->item_id, &P4, &P5, &P6);
	if (status != DWC$k_db_normal)
	{
	}    
	date_time.day = P5;		
    }
    text = (XmString)DATEFORMATTimeToCS (dwc_k_ts_nth_day_format, &date_time);
    date_time.day = day;
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_nth_day_button],
	XmNlabelString, text,
	NULL
    );
    XmStringFree (text);

    /*
    **	Repeat 'monthly' 5 days before month end
    */    
    if ( (se->old->item_id != 0) &&
	(se->old->repeat_p1 == DWC$k_db_nth_day_end) )
    {
	status = DWC$DB_Examine_r_params
	    (se->cd->cab, se->old->item_id, &P4, &P5, &P6);
	if (status != DWC$k_db_normal)
	{
	}    
    }
    else
    {
	status = DWC$DB_Evaluate_r_params
	    (se->cd->cab, DWC$k_db_nth_day_end, se->dsbot, &P4, &P5, &P6);	
	if (status != DWC$k_db_normal)
	{
	}    
    }
    date_time.day = P5-1;
    if (date_time.day == 0)
    {
	text = DATEFORMATTimeToCS (dwc_k_ts_last_day_format, &date_time);
    }
    else if (date_time.day == 1)
    {
	text = DATEFORMATTimeToCS(dwc_k_ts_nth_from_last_format_si, &date_time);
    }
    else
    {
	text = DATEFORMATTimeToCS(dwc_k_ts_nth_from_last_format_pl, &date_time);
    }
    date_time.day = day;
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_nth_day_end_button],
	XmNlabelString, text,
	NULL
    );
    XmStringFree (text);
    
    /*
    **	'On Second Friday of the month'
    */
    if ( (se->old->item_id != 0) &&
	(se->old->repeat_p1 == DWC$k_db_nth_xday) )
    {	
	status = DWC$DB_Examine_r_params
	    (se->cd->cab, se->old->item_id, &P4, &P5, &P6);
	if (status != DWC$k_db_normal)
	{
	}
	date_time.weekday = P5;
    }
    else
    {
	status = DWC$DB_Evaluate_r_params
	    (se->cd->cab, DWC$k_db_nth_xday, se->dsbot, &P4, &P5, &P6);
	if (status != DWC$k_db_normal)
	{
	    P6 = 5;
	}
    }
    date_time.day = P6;
    text = DATEFORMATTimeToCS (dwc_k_ts_nth_weekday_format, &date_time);
    date_time.day = day;
    date_time.weekday = weekday;
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_nth_xday_button],
	XmNlabelString, text,
	XmNsensitive, (status == DWC$k_db_normal),
	NULL
    );
    XmStringFree (text);
    
    /*
    **	'On the last Friday of the month'
    */
    if ( (se->old->item_id != 0) &&
	(se->old->repeat_p1 == DWC$k_db_last_weekday) )
    {	
	status = DWC$DB_Examine_r_params
	    (se->cd->cab, se->old->item_id, &P4, &P5, &P6);
	if (status != DWC$k_db_normal)
	{
	}
	date_time.weekday = P5;	
    }
    else
    {
	status = DWC$DB_Evaluate_r_params
	    (se->cd->cab, DWC$k_db_last_weekday, se->dsbot, &P4, &P5, &P6);
	if (status != DWC$k_db_normal)
	{
	}
    }
    text = DATEFORMATTimeToCS (dwc_k_ts_last_weekday_format, &date_time);
    date_time.weekday = weekday;
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_last_weekday_button],
	XmNlabelString, text,
	XmNsensitive, (status == DWC$k_db_normal),
	NULL
    );
    XmStringFree (text);

    /*
    **  Special Day: for things like the 'first Wednesday at or after the 12th
    **  of the month.
    */
    DWC$DB_Build_time_ctx (se->cd->cab, se->dsbot, 1, &time_context);
    temp_day = day;
    date_time.day = temp_day;
    if ( (se->old->item_id != 0) &&
	(se->old->repeat_p1 == DWC$k_db_nth_day_cwd) )
    {
	status = DWC$DB_Examine_r_params
	    (se->cd->cab, se->old->item_id, &P4, &P5, &P6);
	if (status != DWC$k_db_normal)
	{
	}
	cwd_day_of_week = P5;
    }
    else
    {
	cwd_day_of_week = date_time.weekday;
    }
    date_time.weekday = cwd_day_of_week;
    text = DATEFORMATTimeToCS (dwc_k_ts_spec_day_after_format, &date_time);
    date_time.day = day;
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_special_day_menu_entry],
	XmNlabelString, text,
	NULL
    );
    XmStringFree(text);

    for (index = k_ts_after_0_button; index >= k_ts_after_6_button; index--)
    {
	if (temp_day == 0)
	{
	    temp_day = time_context.DWC$b_dbtc_days_prev_month;
	}
	date_time.day = temp_day;
	text = DATEFORMATTimeToCS (dwc_k_ts_spec_day_after_format, &date_time);
	date_time.day = day;
	XtVaSetValues
	    (SLOTEDITORWIDGETS(se)[index], XmNlabelString, text, NULL);
	XmStringFree (text);		
	temp_day--;
    }

    temp_day = day;
    for (index = k_ts_before_0_button; index <= k_ts_before_6_button; index++)
    {
	if (temp_day > time_context.DWC$b_dbtc_days_in_month)
	{
	    temp_day = 1;
	}
	date_time.day = temp_day;   
	text = DATEFORMATTimeToCS (dwc_k_ts_spec_day_before_format, &date_time);
	date_time.day = day;
	XtVaSetValues
	    (SLOTEDITORWIDGETS(se)[index], XmNlabelString, text, NULL);
	XmStringFree(text);
	temp_day++;
    }

    text = DATEFORMATTimeToCS(dwc_k_ts_the_day_at_or_format, &date_time);
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_the_day_at_or_label],
	XmNlabelString, text,
	NULL
    );
    XmStringFree (text);
    date_time.weekday = weekday;

    /*
    **
    */
    if (se->old->item_id == 0)
    {
	/*
	**  New item
	*/
	repeat_attributes_index = k_ts_nth_day_button;
    }
    else
    {
	/*
	**
	*/
        switch (se->old->repeat_p1)
	{
	case DWC$k_db_nth_day:
	    /*
	    **
	    */		
	    repeat_attributes_index = k_ts_nth_day_button;
	    break;
	case DWC$k_db_nth_day_end:
	    /*
	    **
	    */		
	    repeat_attributes_index = k_ts_nth_day_end_button;
	    break;
	case DWC$k_db_nth_xday:
	    /*
	    **
	    */		
	    repeat_attributes_index = k_ts_nth_xday_button;
	    break;
	case DWC$k_db_last_weekday:
	    /*
	    **
	    */		
	    repeat_attributes_index = k_ts_last_weekday_button;
	    break;
	case DWC$k_db_nth_day_cwd:
	    /*
	    **  Conditional weekdays
	    */
	    repeat_attributes_index = k_ts_special_day_menu_entry;
	    cwd_day_of_month = ((se->old->repeat_p3 >> DWC$v_cond_day) &
		DWC$m_cond_mask) & ~DWC$m_cond_flags;
	    date_time.day = cwd_day_of_month;
	    date_time.weekday = cwd_day_of_week;

	    if ( ((se->old->repeat_p3 >> DWC$v_cond_day) &
		DWC$m_cond_flags) == DWC$m_cond_fwd )
	    {
		/*
		**	At or after
		*/
		text = DATEFORMATTimeToCS 
		    (dwc_k_ts_first_x_after_format, &date_time);
	    }
	    else
	    {
		/*
		**	At or before
		*/
		text = DATEFORMATTimeToCS
		    (dwc_k_ts_first_x_before_format, &date_time);
	    }
	    XtVaSetValues
	    (
		SLOTEDITORWIDGETS(se)[k_ts_special_day_menu_entry],
		XmNlabelString, text,
		NULL
	    );
	    XmStringFree (text);
	    date_time.day	    = day;
	    date_time.weekday   = weekday;
	    break;
	default:
	    repeat_attributes_index = k_ts_nth_day_button;
	    break;
	}
    }			

    /*
    **  Now, this is incredibly silly
    */
    XtVaGetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_attributes_optmenu],
	XmNmenuHistory, &widget,
	NULL
    );

    if (widget == SLOTEDITORWIDGETS(se)[repeat_attributes_index])
    {
	if ( repeat_attributes_index < k_ts_special_day_menu_entry )
	    dummy_index = repeat_attributes_index + 1;
	else
	    dummy_index = k_ts_nth_day_button;
	XtVaSetValues
	(
	    SLOTEDITORWIDGETS(se)[k_ts_attributes_optmenu],
	    XmNmenuHistory, SLOTEDITORWIDGETS(se)[dummy_index],
	    NULL
	);
    }

    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_attributes_optmenu],
	XmNmenuHistory, SLOTEDITORWIDGETS(se)[repeat_attributes_index],
	NULL
    );
	
    se->attributes = repeat_attributes_index;

    /*
    **  That's it
    */
    return;			    
}

static void SE_setup_repeat_conditions
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
    {
    Arg		arglist[1];
    int		required_type_of_day;
    
    /*
    **
    */
    if (se->old->item_id == 0)
        {
	/*
	**  New item
	*/
	}
    else
	{
	/*
	**
	*/
	switch (se->old->repeat_p1)
	    {
	    case DWC$k_db_none:
	    case DWC$k_db_absolute:
		/*
		**  We do nothing: the toggle button will remain insensitive
		*/
		break;
	    case DWC$k_db_abscond:
	    case DWC$k_db_nth_day:
	    case DWC$k_db_nth_day_end:
	    case DWC$k_db_nth_xday:
	    case DWC$k_db_last_weekday:
	    case DWC$k_db_nth_day_cwd:
		/*
		**  The toggle button is sensitive
		*/
		XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_condition_toggle],
		    TRUE);

		if ( se->old->repeat_p1 != DWC$k_db_nth_day_cwd )
		    required_type_of_day = se->old->repeat_p3 &
			~DWC$m_cond_flags;
		else
		    required_type_of_day = (se->old->repeat_p3 &
			DWC$m_cond_mask) & ~DWC$m_cond_flags;
		    
		/*
		**  Let's set up the work/nonwork and move option menus, as well
		**  as the condition toggle state
		*/
		if ( required_type_of_day != DWC$k_day_default )
		    {
		    se->condition = TRUE;
		    XmToggleButtonSetState(
			SLOTEDITORWIDGETS(se)[k_ts_condition_toggle],
			se->condition, FALSE);
			
		    switch ( required_type_of_day )
			{
			case DWC$k_day_workday:
			    se->work_nonwork = k_ts_nonwork_button;
			    break;
			case DWC$k_day_nonwork:
			    se->work_nonwork = k_ts_work_button;
			    break;
			}
		    XtSetArg(arglist[0], XmNmenuHistory,
			SLOTEDITORWIDGETS(se)[se->work_nonwork]);
		    XtSetValues(
			SLOTEDITORWIDGETS(se)[k_ts_work_nonwork_optmenu],
			arglist, 1);			
		    XtSetSensitive(
			SLOTEDITORWIDGETS(se)[k_ts_work_nonwork_optmenu],
			TRUE);
			
		    switch ( se->old->repeat_p3 & DWC$m_cond_flags )
			{
			case DWC$m_cond_none:
			    se->move = k_ts_skip_button;
			    break;
			case DWC$m_cond_fwd:
			    se->move = k_ts_forward_button;
			    break;
			case DWC$m_cond_bck:
			    se->move = k_ts_backward_button;
			    break;
			}
		    XtSetArg(arglist[0], XmNmenuHistory,
			SLOTEDITORWIDGETS(se)[se->move]);
		    XtSetValues(SLOTEDITORWIDGETS(se)[k_ts_move_optmenu],
			arglist, 1);
		    XtSetSensitive(
			SLOTEDITORWIDGETS(se)[k_ts_move_optmenu],
			TRUE);
		    }
		break;
	    default:
		break;
	    }	    
	}

    /*
    **  That's it
    */
    return;
    }

static void SE_setup_flags
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
    {
    if (se->old->flags & DWC$m_item_insignif)
	XmToggleButtonSetState( SLOTEDITORWIDGETS(se)[k_ts_flags_toggle],
	    FALSE, FALSE);
    else
	XmToggleButtonSetState( SLOTEDITORWIDGETS(se)[k_ts_flags_toggle],
	    TRUE, FALSE);
    
    /*
    **  That's it
    */
    return;
    }

static void SE_setup_labels
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
{
    dtb		    date_time;
    XmString	    text;
    CalendarDisplay cd = se->cd;

    date_time.weekday = DATEFUNCDayOfWeek (cd->day, cd->month, cd->year);
    date_time.day     = cd->day;
    date_time.month   = cd->month;
    date_time.year    = cd->year;

    /*
    **	Setting the value of the date label
    */
    text = (XmString)DATEFORMATTimeToCS (dwc_k_ts_date_format, &date_time);
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_date_label],
	XmNlabelString, text,
	NULL
    );
    XmStringFree (text);
    /*
    **	We only put up the from and to label if the editor shown is the
    **	Slot Editor
    */    
    if (se->editor == SlotEditor)
	{
	/*
	**  Setting the value of the 'from' label
	*/    
	date_time.hour   = se->start_time / 60;
	date_time.minute = se->start_time % 60;
	if (cd->profile.time_am_pm)
	{
	    text = (XmString)DATEFORMATTimeToCS (dwc_k_ts_from_ampm_format, &date_time);
	}
	else
	{
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_from_format, &date_time);
	}
	XtVaSetValues
	(
	    SLOTEDITORWIDGETS(se)[k_ts_from_label],
	    XmNlabelString, text,
	    NULL
	);
	XmStringFree(text);

	/*
	**	Setting the value of the 'to' label
	*/
	date_time.hour   = (se->start_time +
			    se->duration) / 60;
	date_time.minute = (se->start_time +
			    se->duration) % 60;
	if (cd->profile.time_am_pm)
	{
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_to_ampm_format, &date_time);
	}
	else
	{
	    text = (XmString)DATEFORMATTimeToCS
		(dwc_k_ts_to_format, &date_time);
	} 
	XtVaSetValues
	(
	    SLOTEDITORWIDGETS(se)[k_ts_to_label],
	    XmNlabelString, text,
	    NULL
	);
	XmStringFree (text);

    }

    /*
    **  That's it
    */
    return;	   		       
}

static void SE_readonly_editor
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */    
{
    /*
    **	The include menu item isn't selectable
    */
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_include_menu_item], FALSE);

    /*
    **	The text widget should be insentive so that no text can be entered
    **	in it
    */
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext],
	XmNcursorPositionVisible, FALSE,
	XmNeditable, FALSE,
	NULL
    );

    /*
    **	If we are dealing with a timeslot editor, then the alarm toggles
    **	and the scale should be set insensitive
    */
    if (se->editor == SlotEditor)
	{
	XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_entry_time_toggle],	FALSE);
	XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_short_toggle],	FALSE);
	XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_medium_toggle],	FALSE);
	XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_long_toggle],		FALSE);
	XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_vlong_toggle],	FALSE);
	XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_alarm_scale],		FALSE);
	};
	
    /*
    **  The icon box widgets should be insensitive
    */
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_select_ibw], FALSE);
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_order_ibw], FALSE);

    /*
    **  All repeat-related options menus and toggles should be
    **	insensitive
    */
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_repeat_optmenu],	    FALSE);
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_attributes_optmenu],  FALSE);
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_condition_toggle],    FALSE);
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_work_nonwork_optmenu],FALSE);
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_move_optmenu],	    FALSE);

    /*
    **	The 'flags' toggle should be insensitive
    */
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_flags_toggle], FALSE);

    /*
    **	The delete and reset buttons should be insensitive
    */
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_reset_button], FALSE);
    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_delete_button], FALSE);
    
    /*
    **  That's it
    */
    return;
    }

static void SE_get_alarms
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	DwcDayItem	new)
#else	/* no prototypes */
	(se, new)
	Sloteditor	se;
	DwcDayItem	new;
#endif	/* prototype */
    {
    int	i;
        
    /*
    **	We go and take a look at all toggles to find how many alarms were set;
    **	using that count, an arrays of alarms is then allocated.
    */
    new->alarms_number	= 0;

    if (XmToggleButtonGetState(SLOTEDITORWIDGETS(se)[k_ts_entry_time_toggle]))
	new->alarms_number++;

    for (i=0; i<=3; i++)
	{
	if (XmToggleButtonGetState(SLOTEDITORWIDGETS(se)[k_ts_short_toggle+i]))
	    new->alarms_number++;
	}

    if (new->alarms_number>0)
	new->alarms_times = (unsigned short int *)XtMalloc(sizeof(Cardinal) *
	    new->alarms_number);
    else
	new->alarms_times = NULL;
		
    /*
    **	We now fill in the previously allocated array with the alarms's
    **	values.
    */
    new->alarms_number	= 0;

    if (XmToggleButtonGetState(
	SLOTEDITORWIDGETS(se)[k_ts_entry_time_toggle] ) )
	{
	new->alarms_times[new->alarms_number] = 0;
	new->alarms_number++;
	}

    for (i=0; i<=3; i++)
	{
	if (XmToggleButtonGetState(SLOTEDITORWIDGETS(se)[k_ts_short_toggle+i]))
	    {
	    new->alarms_times[new->alarms_number] = se->scale_latest_value[i];
	    new->alarms_number++;
	    }
	}

    /*
    **  That's it
    */
    return;
    }

static void SE_get_icons
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	DwcDayItem	new)
#else	/* no prototypes */
	(se, new)
	Sloteditor	se;
	DwcDayItem	new;
#endif	/* prototype */
    {
    int i,
	count = 0;

    if (new->icons != NULL)        
	XtFree((char *) new->icons);

    DwcIbwGetSelectedIcons
    (
	SLOTEDITORWIDGETS(se)[k_ts_order_ibw],
	&new->icons,
	&new->icons_number
    );

    /*
    **  That's it
    */
    return;
    }

static void SE_get_repeats
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	DwcDayItem	new)
#else	/* no prototypes */
	(se, new)
	Sloteditor	se;
	DwcDayItem	new;
#endif	/* prototype */
    {
    switch (se->repeat_interval)
        {
        case k_ts_none_button:
            new->repeat_p1 = DWC$k_db_none;
            break;
        case k_ts_daily_button:
	    /* old style db pre rev 10
	    new->repeat_p1 = DWC$k_db_absolute;
	    new->repeat_p2 = 60 * 24;
	    */
	    new->repeat_p1 = DWC$k_db_abscond;
	    new->repeat_p2 = 1;
	    SE_get_repeat_conditions(se, new);
            break;
        case k_ts_weekly_button:
	    /* old style db pre rev 10
	    new->repeat_p1 = DWC$k_db_absolute;
	    new->repeat_p2 = 60 * 24 * 7;
	    */
	    new->repeat_p1 = DWC$k_db_abscond;
	    new->repeat_p2 = 7;
	    SE_get_repeat_conditions(se, new);
            break;
        case k_ts_fortnight_button:
	    /* old style db pre rev 10
	    new->repeat_p1 = DWC$k_db_absolute;
	    new->repeat_p2 = 60 * 24 * 7 * 2;
	    */
	    new->repeat_p1 = DWC$k_db_abscond;
	    new->repeat_p2 = 14;
	    SE_get_repeat_conditions(se, new);
            break;
	case k_ts_fourweek_button:
	    /* old style db pre rev 10
	    new->repeat_p1 = DWC$k_db_absolute;
	    new->repeat_p2 = 60 * 24 * 7 * 4;
	    */
	    new->repeat_p1 = DWC$k_db_abscond;
	    new->repeat_p2 = 28;
	    SE_get_repeat_conditions(se, new);
	    break;
	case k_ts_monthly_button:
	    new->repeat_p2 = 1;
	    SE_get_repeat_attributes(se, new);
	    SE_get_repeat_conditions(se, new);
	    break;
	case k_ts_bimonthly_button:
	    new->repeat_p2 = 2;
	    SE_get_repeat_attributes(se, new);
	    SE_get_repeat_conditions(se, new);
	    break;
	case k_ts_quarterly_button:
	    new->repeat_p2 = 3;
	    SE_get_repeat_attributes(se, new);
	    SE_get_repeat_conditions(se, new);
	    break;
	case k_ts_triannually_button:
	    new->repeat_p2 = 4;
	    SE_get_repeat_attributes(se, new);
	    SE_get_repeat_conditions(se, new);
	    break;
	case k_ts_biannually_button:
	    new->repeat_p2 = 6;
	    SE_get_repeat_attributes(se, new);
	    SE_get_repeat_conditions(se, new);
	    break;
	case k_ts_annually_button:
	    new->repeat_p2 = 12;
	    SE_get_repeat_attributes(se, new);
	    SE_get_repeat_conditions(se, new);
	    break;
	default:
	    break;
	/*
	**   Missing cases here
	*/
	}    

    /*
    **	That's it
    */
    return;
    }    

static void SE_get_repeat_attributes
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	DwcDayItem	new)
#else	/* no prototypes */
	(se, new)
	Sloteditor	se;
	DwcDayItem	new;
#endif	/* prototype */
    {
    /*
    **	The selected option in the repeat interval option menu is one of
    **	k_ts_monthly_button, k_ts_bimonthly_button, k_ts_quarterly_button,
    **	k_ts_biannually_button, k_ts_annually_button.
    */
    
    switch(se->attributes)
	{
	case k_ts_nth_day_button:
	    new->repeat_p1 = DWC$k_db_nth_day;
	    break;
	case k_ts_nth_day_end_button:
	    new->repeat_p1 = DWC$k_db_nth_day_end;
	    break;
	case k_ts_nth_xday_button:
	    new->repeat_p1 = DWC$k_db_nth_xday;
	    break;
	case k_ts_last_weekday_button:
	    new->repeat_p1 = DWC$k_db_last_weekday;
	    break;
	case k_ts_special_day_menu_entry:
	    /*
	    **
	    */
	    new->repeat_p1 = DWC$k_db_nth_day_cwd;
	    if (se->cwd_cond_forward)
		new->repeat_p3 = DWC$m_cond_fwd;
	    else
		new->repeat_p3 = DWC$m_cond_bck;
	    new->repeat_p3 = new->repeat_p3 | se->cwd_day_of_month;
	    new->repeat_p3 = new->repeat_p3 << DWC$v_cond_day;
	    break;
	default:
	    /*
	    **  Shouldn't be possible to get here; we should probably signal
	    **	something.
	    */
	    break;
	}
    
    /*
    **	That's it
    */
    return;
    }

static void SE_get_repeat_conditions
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	DwcDayItem	new)
#else	/* no prototypes */
	(se, new)
	Sloteditor	se;
	DwcDayItem	new;
#endif	/* prototype */
    {
    if (new->repeat_p1 != DWC$k_db_nth_day_cwd)
        new->repeat_p3 = 0;
	
    if (se->condition)
	{
	switch (se->work_nonwork)
	    {
	    case k_ts_work_button:
		new->repeat_p3 = new->repeat_p3 | DWC$k_day_nonwork;
		break;
	    case k_ts_nonwork_button:
		new->repeat_p3 = new->repeat_p3 | DWC$k_day_workday;
		break;
	    }
	switch (se->move)
	    {
	    case k_ts_skip_button:
		new->repeat_p3 = new->repeat_p3 | DWC$m_cond_none;
		break;
	    case k_ts_forward_button:
		new->repeat_p3 = new->repeat_p3 | DWC$m_cond_fwd;
		break;
	    case k_ts_backward_button:
		new->repeat_p3 = new->repeat_p3 | DWC$m_cond_bck;
		break;
	    }
	}
    else
	{
	new->repeat_p3 = new->repeat_p3 | DWC$k_day_default;
	}
	
    /*
    **  That's it
    */
    return;
    }

static void SE_get_flags
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se,
	DwcDayItem	new)
#else	/* no prototypes */
	(se, new)
	Sloteditor	se;
	DwcDayItem	new;
#endif	/* prototype */
    {
    new->flags = 0;
    
    if ( ! XmToggleButtonGetState(SLOTEDITORWIDGETS(se)[k_ts_flags_toggle]) )
        new->flags = new->flags | DWC$m_item_insignif;
	
    /*
    **  That's it
    */
    return;
    }

static void SE_tse_include_cb
#if defined(_DWC_PROTO_)
	(
	Widget					w,
	caddr_t					*tag,
	XmFileSelectionBoxCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget					w;
	caddr_t					*tag;
	XmFileSelectionBoxCallbackStruct	*cbs;
#endif	/* prototype */
{
    Sloteditor		se = (Sloteditor)tag;
    CalendarDisplay	cd = se->cd;
    char		*filespec;
    int			status;
    Time		time;

    time = MISCGetTimeFromAnyCBS( (XmAnyCallbackStruct *)cbs );

    ICONSInactiveCursorRemove( w );

    if (cbs->reason == (int)XmCR_CANCEL)
    {
	XtUnmanageChild (w);
    }
    else
    {
	filespec = MISCGetTextFromCS(cbs->value);

	if (filespec == NULL)
	{
	    ERRORDisplayError
	    (
		SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext],
		"ErrorIncludeSpec"
	    );
	    return;
	}

	ICONSWaitCursorDisplay
	(
	    SLOTEDITORWIDGETS(se)[k_ts_popup_attached_db],
	    cd->ads->wait_cursor
	);

	status = MISCIncludeFileIntoText
	(
	    cd,
	    filespec,
	    (DXmCSTextWidget)SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext]
	);
	if (status == k_include_success)
	{
	    XtUnmanageChild (w);
	    XtFree (filespec);
	    ICONSWaitCursorRemove
		(SLOTEDITORWIDGETS(se)[k_ts_popup_attached_db]);
	}
	else if (status == k_include_empty ) 
	{
	    XtFree (filespec);
	    ERRORDisplayError
	    (
		SLOTEDITORWIDGETS(se)[k_ts_popup_attached_db],
		"ErrorEmptyInclude"
	    );
	    ICONSWaitCursorRemove
		(SLOTEDITORWIDGETS(se)[k_ts_popup_attached_db]);
	    return;
	}
	else
	{
	    ERRORDisplayErrno
	    (
		SLOTEDITORWIDGETS(se)[k_ts_popup_attached_db],
		"ErrorInclude"
	    );
	    XtFree (filespec);
	    ICONSWaitCursorRemove
		(SLOTEDITORWIDGETS(se)[k_ts_popup_attached_db]);
	    return;
	} 
    }

    XmProcessTraversal
	(SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext], XmTRAVERSE_CURRENT);

    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */

} 

/*
**++
**  Functional Description:
**	This routine gets called when the modal include dialog box invoked
**	from the timeslot or daynote editors is unmapped. We just loop thru
**	the popup list of the widget looking for a help widget which could
**	have been created, and unmanage it.
**
**  Keywords:
**	None.
**
**  Arguments:
**	TBD.
**
**  Result:
**	TBD..
**
**  Exceptions:
**	None.
**--
*/
static void SE_tse_include_unmap_cb
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    HELPDestroyForModal (w, (int *)tag, cbs );
}

void SELinkItemToEntry
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Cardinal	index,
	DwcDswEntry	link)
#else	/* no prototypes */
	(cd, index, link)
	CalendarDisplay	cd;
	Cardinal	index;
	DwcDswEntry	link;
#endif	/* prototype */
{
    cd->ses [index]->link = link;
}

void SEUnlinkAllItems
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    Cardinal		i;
    
    for (i = 0;  i < cd->number_of_sloteditors;  i++) {
	cd->ses [i]->link = NULL;
    }
}

int SEFindItem
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Cardinal	dsbot,
	int		item_id,
	DwcDswEntry	link,
	Boolean		popup,
	int		*return_start,
	int		*return_duration,
	XmString	*return_text)
#else	/* no prototypes */
	(cd, dsbot, item_id, link, popup, return_start, return_duration, return_text)
	CalendarDisplay	cd;
	Cardinal	dsbot;
	int		item_id;
	DwcDswEntry	link;
	Boolean		popup;
	int		*return_start;
	int		*return_duration;
	XmString	*return_text;
#endif	/* prototype */
    {
    Cardinal		i;
    Boolean		found;
    
    for (i = 0;  i < cd->number_of_sloteditors;  i++) {

	if (cd->ses [i]->in_use) {

	    found = FALSE;
	    if (cd->ses [i]->old->item_id == item_id) {
		if (item_id < 0) {
		    found = TRUE;
		} else {
		    if (cd->ses [i]->dsbot == dsbot) {
			if (item_id == 0) {
			    found = (cd->ses [i]->link == link);
			} else {
			    found = TRUE;
			}
		    }
		}
	    }

	    if (found) {
		if (popup) {
		    /*
		    **
		    */
		    /*
		    XtUnmanageChild (SLOTEDITORWIDGETS (cd->ses [i])
					    [k_ts_popup_attached_db]);
		    XtManageChild   (SLOTEDITORWIDGETS (cd->ses [i])
					    [k_ts_popup_attached_db]);
		    */
		    MISCRaiseToTheTop(SLOTEDITORWIDGETS (cd->ses [i])
			[k_ts_popup_attached_db]);
		}

		if (return_start != NULL) {
		    *return_start = cd->ses [i]->start_time;
		}

		if (return_duration != NULL) {
		    *return_duration = cd->ses [i]->duration;
		}

		if (return_text != NULL) {
		    *return_text = cd->ses [i]->text;
		}

		return (i);
	    }
	}
    }
    
    return (-1);
}

int SEGetNewItems
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Cardinal	dsbot,
	Cardinal	index,
	DwcDayItem	*return_di,
	XmString	*return_text)
#else	/* no prototypes */
	(cd, dsbot, index, return_di, return_text)
	CalendarDisplay	cd;
	Cardinal	dsbot;
	Cardinal	index;
	DwcDayItem	*return_di;
	XmString	*return_text;
#endif	/* prototype */
{
    Cardinal		i;
    DwcDayItem		di;

    
    for (i = index;  i < cd->number_of_sloteditors;  i++) {
	if ((cd->ses [i]->in_use) && (cd->ses [i]->dsbot == dsbot) &&
	    (cd->ses [i]->old->item_id == 0)) {

	    di = DAYCloneDayItem (cd, cd->ses [i]->old);
	    di->start_time = cd->ses [i]->start_time;
	    di->duration   = cd->ses [i]->duration;

	    *return_di   = di;
	    *return_text = cd->ses [i]->text;
	    return (i);
	}
    }
    return (-1);
    
}

void SECreateSlotEditor
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	WhichEditor	editor,
	DwcDaySlots	ds,
	DwcDswEntry	entry,
	Boolean		text_changed)
#else	/* no prototypes */
	(cd, editor, ds, entry, text_changed)
	CalendarDisplay	cd;
	WhichEditor	editor;
	DwcDaySlots	ds;
	DwcDswEntry	entry;
	Boolean		text_changed;
#endif	/* prototype */
    {
    dtb			    date_time;
    XmString		    text;
    char		    *slot_text;
    Cardinal		    increment;
    Cardinal		    slot_start;
    Cardinal		    slot_end;
    int			    index;
    int			    number_before;
    int			    first_editor;
    Cardinal		    i;
    DSWEntryDataStruct   data;
    char		    *entry_text;
    DwcDayItem		    di;
    Position		    percentage_of_screen_x, percentage_of_screen_y;
    Position		    sloteditor_x, sloteditor_y;
    Dimension		    profile_format_width, profile_format_height;
    Dimension		    width,height;
    Boolean		    found_unused_one = FALSE;
    XtTranslations	    parsed_translation_table;
    Widget		    form;
    Widget		    button_form;
    Widget		    text_widget;
    WidgetList		    children;
    Cardinal		    num_children;
    Widget		    ok_button;

    static char		    translation_table [] =
	"<Key>0xff8d:		    SE_close_action()";

    static XtActionsRec	    action_table [] =
    {
	{"SE_close_action",	(XtActionProc)SE_close_action},
	{NULL,			NULL}
    };

    /*
    **  This may take a while...
    */
    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor);

    /*
    ** Make sure the medium pixmaps are loaded.
    */
    MISCLoadPixmapArrays (cd->mainwid, 1);

    /*
    **	Let's parse the translation table and register our action routine for
    **	the text widget in the slot editors.
    */
    parsed_translation_table = XtParseTranslationTable(translation_table);
    XtAppAddActions (CALGetAppContext(), action_table, 1);
  
    /*
    **  Do we have to create a dialog box from scratch, or is there one we can
    **	reuse?
    */
    if (cd->number_of_sloteditors == 0)
    {
	cd->ses = (Sloteditor *)  XtMalloc( sizeof(Sloteditor) );
	cd->number_of_sloteditors++;
	index=0;
    }
    else
    {
	for (index=0; index< cd->number_of_sloteditors; index++)
	{
	    if ( ( !cd->ses[index]->in_use ) &&
		 (  cd->ses[index]->editor == editor) )
	    {
		/*
		**  We have found an unused editor of the right type
		*/
		found_unused_one = TRUE;
		break;
	    }
	}
	if (!found_unused_one)
	{
	    cd->ses = (Sloteditor *)  XtRealloc
	    (
		(char *)cd->ses,
		sizeof(Sloteditor) * (cd->number_of_sloteditors + 1)
	    );
	    index = cd->number_of_sloteditors;
	    cd->number_of_sloteditors++;
	}
    }

    if (!found_unused_one)
    {
	/*
	**  We had to create an entry from scratch in the array; let's
	**  initialize this entry.
	*/
	cd->ses[index] = (Sloteditor) XtMalloc( sizeof(SloteditorRecord) );
	cd->ses[index]->cd = cd;
	cd->ses[index]->editor = editor;
	cd->ses[index]->file_selection_widget = NULL;

	switch (editor)
	{
	case SlotEditor:
	    SE_create_tse(cd->ses[index]);
	    break;
	case DaynoteEditor:
	    SE_create_dne(cd->ses[index]);
	    break;
	}

	/*
	**  Fix the text widget's translation to support the ENTER key as a
	**  short cut for the Ok button
	*/
	XtOverrideTranslations(
	    SLOTEDITORWIDGETS(cd->ses[index])[k_ts_timeslot_stext],
	    parsed_translation_table);		

	/*
	**  Update the sloteditor or daynote editor scroll bar according
	**  to the content of the profile
	*/
	if (cd->profile.directionRtoL)
	    SE_update_scrollbar
		(cd->ses[index], cd->profile.directionRtoL);	
    }

    first_editor  = -1;
    number_before = 0;
    for (i = 0;  i < index; i++) {
	if (cd->ses[i]->editor == editor) {
	    number_before++;
	    if (first_editor < 0) {
		first_editor = i;
	    }
	}
    }

    if (first_editor < 0) {
	first_editor = index;
    }

    if (number_before != 0) {
	SE_save_editor_position (cd, first_editor);
    }

    switch (editor) {
      case SlotEditor :
	percentage_of_screen_x      = cd->profile.sloteditor_x;
	percentage_of_screen_y      = cd->profile.sloteditor_y;
	profile_format_width  = (Dimension)cd->profile.sloteditor_width;
	profile_format_height = (Dimension)cd->profile.sloteditor_height;
	break;

      case DaynoteEditor :
	percentage_of_screen_x      = cd->profile.noteeditor_x;
	percentage_of_screen_y      = cd->profile.noteeditor_y;
	profile_format_width  = (Dimension)cd->profile.noteeditor_width;
	profile_format_height = (Dimension)cd->profile.noteeditor_height;
	break;

    }
    
    /*	  
    **  Convert the profile format x,y and width and height to real numbers we
    **	can use.
    */	  
    MISCGetScreenPixelPosition(cd,
		    percentage_of_screen_x,
		    percentage_of_screen_y,
		    profile_format_width,
		    profile_format_height,
		    &width, &height,
		    &sloteditor_x,
		    &sloteditor_y);

    DSWGetEntryData (entry, &data);

    if (data.tag == NULL) {
	di = DAYCloneDayItem (cd, ds->default_item);
	di->start_time = data.start;
	di->duration   = data.duration;
    } else {
	di = DAYCloneDayItem (cd, (DwcDayItem) data.tag);
    }

    cd->ses[index]->in_use       = TRUE;
    cd->ses[index]->dsbot        = cd->dsbot;
    cd->ses[index]->old          = di;
    cd->ses[index]->start_time   = data.start;
    cd->ses[index]->duration     = data.duration;
    cd->ses[index]->text         = DSWGetEntryCSText (entry);
    /*
    **	text_changed tells if the entry was opened and the text changed before
    **	the user double-clicked  
    */    
    cd->ses[index]->text_changed = text_changed;
    cd->ses[index]->link         = entry;
    cd->ses[index]->slots        = ds;
            
    /*
    **  Let's fill in all the widgets
    */
    if (cd->ses[index]->editor == SlotEditor)
	SE_fillin_tse(cd->ses[index]);
    else
	SE_fillin_dne(cd->ses[index]);

    /*
    **  Let's initialize the change booleans (see stext_changed boolean below)
    */
    cd->ses[index]->flag_changed  = FALSE;
    cd->ses[index]->icons_changed = FALSE;
    cd->ses[index]->alarms_changed= FALSE;
    cd->ses[index]->repeat_changed= FALSE;

    /*
    **  Realize it first so that the FitFormDialog stuff can work
    */
    form = SLOTEDITORWIDGETS(cd->ses[index])[k_ts_popup_attached_db];

    XtVaSetValues 
    (
	form,
	XmNtraversalOn, TRUE,
	XtNx, sloteditor_x + (25 * number_before),
	XtNy, sloteditor_y + (25 * number_before),
	NULL
    );

    /*
    ** First we fix the button widths without any other changes to the
    ** widget.  This must happen before "Fit on screen"
    */
    ok_button = SLOTEDITORWIDGETS(cd->ses[index])[k_ts_ok_button];

    button_form = XtParent(ok_button);

    /*
    ** Control buttons at the bottom the same size.  This may cause a resize
    ** of the Form that contains them.
    */
    children = DXmChildren ((CompositeWidget)button_form);
    num_children = DXmNumChildren ((CompositeWidget)button_form);
    MISCWidthButtonsEqually (children, num_children);

    /*
    ** Realize the form.
    */
    XtManageChild (form);

    /*
    ** Tell the sloteditor to take focus if no explicit focus redirection
    ** has happened.  Also make sure the the focus goes to the text widget.
    */
    text_widget = SLOTEDITORWIDGETS(cd->ses[index])[k_ts_timeslot_stext];
    MISCFocusOnMap (XtParent(form), text_widget);

    /*
    ** Switch to positional attachments of the control buttons.
    */
    MISCSpaceButtonsEqually (children, num_children);

    /*
    ** Get the form on the screen.
    */
    XtSetMappedWhenManaged (form, True);

    /*
     * comment out line, fix for QAR 17085

    XtMapWidget (form);

     */

    /*
    ** Make the button box stretchy.
    */
    XtVaSetValues
    (
	button_form,
	XmNrightAttachment, XmATTACH_FORM,
	XmNresizePolicy, XmRESIZE_ANY,
	NULL
    );

    /*
     * comment out line, fix for QAR 17085

    XtMapWidget (XtParent(form));

     */

    /*
     * added the following line, fix for QAR 17085
     */
	XtPopup(XtParent(form), XtGrabNone);

    /*
    ** Make the CLOSE item in the window manager menu call the CANCEL
    ** callback.
    */
    MISCAddProtocols
	(XtParent(form), (XtCallbackProc) SE_timeslot_close_cb, NULL);

    /*
    **  This is very brain damaged: managing the dialog box causes the text
    **	widget to be resized, which causes in turn the SE_text_changed_cb
    **	callback to be activated, which trashes the stext_changed. Therefore,
    **	stext_changed needs to be initialized AFTER the widget is managed
    */
    cd->ses[index]->stext_changed = FALSE;

    ICONSWaitCursorRemove (cd->mainwid);

    /*
    **  That's it
    */
    return;
    }

static void SE_save_editor_position
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Cardinal	first_editor)
#else	/* no prototypes */
	(cd, first_editor)
	CalendarDisplay	cd;
	Cardinal	first_editor;
#endif	/* prototype */
{
    Position		x, y;
    Dimension		width, height;

    XtVaGetValues
    (
	XtParent
	    (SLOTEDITORWIDGETS(cd->ses[first_editor])[k_ts_popup_attached_db]),
	XtNx, &x,
	XtNy, &y,
	NULL
    );

    XtVaGetValues
    (
	SLOTEDITORWIDGETS (cd->ses [first_editor]) [k_ts_popup_attached_db],
	XtNwidth,  &width,
	XtNheight, &height,
	NULL
    );

    width = (width * 100) / cd->screen_font_size;
    height = (height * 100) / cd->screen_font_size;

    MISCGetScreenFractionalPosition (cd, width, height, x, y, &x, &y);

    switch (cd->ses [first_editor]->editor)
    {
    case SlotEditor :
	cd->profile.sloteditor_x = x;
	cd->profile.sloteditor_y = y;
	cd->profile.sloteditor_width  = (Cardinal)width;
	cd->profile.sloteditor_height = (Cardinal)height;
	break;

    case DaynoteEditor :
	cd->profile.noteeditor_x = x;
	cd->profile.noteeditor_y = y;
	cd->profile.noteeditor_width  = (Cardinal)width;
	cd->profile.noteeditor_height = (Cardinal)height;
	break;
    }

}

static void SE_create_tse
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
{
    static MrmRegisterArg regvec[] =
	{
	    /*
	    **  Identifiers
	    */
	    {"sloteditor",		    (caddr_t)0},    /* See code	    */
	    {"sloteditor_off_pixmap_array", (caddr_t)medium_pixmaps},
	    
	    /*
	    **  Creation callbacks
	    */
	    {"sloteditor_create_proc",	    (caddr_t)SE_create_cb},
	    {"sloteditor_padb_create_proc", (caddr_t)SE_formdialog_create_cb},
	    /*
	    **	
	    */
	    {"sloteditor_stext_changed",
		(caddr_t)SE_text_changed_cb},
	    {"sloteditor_alarm_changed",
		(caddr_t)SE_alarm_changed_cb},
	    {"sloteditor_alarm_val_changed",
		(caddr_t)SE_alarm_value_changed_cb},
	    {"sloteditor_alarm_scale_changed",
		(caddr_t)SE_sloteditor_alarm_scale_changed_cb},
	    {"sloteditor_repeat_interval_proc",
		(caddr_t)SE_repeat_interval_cb},
	    {"sloteditor_attributes_proc",
		(caddr_t)SE_attibutes_cb},
	    {"sloteditor_special_proc",
		(caddr_t)SE_special_cb},
	    {"sloteditor_work_nonwork_proc",
		(caddr_t)SE_work_nonwork_cb},	
	    {"sloteditor_move_proc",
		(caddr_t)SE_move_cb},
	    {"sloteditor_select_ibw",
		(caddr_t)SE_select_iconbox_cb},
	    {"sloteditor_order_ibw",
		(caddr_t)SE_order_iconbox_cb},
	    {"sloteditor_flag_changed",
		(caddr_t)SE_flag_changed_cb},
	    /*
	    **
	    */
	    {"condition_value_changed",	    (caddr_t)SE_condition_value_changed_cb},
	    /*
	    **  Buttons at the bottom of the dialog box
	    */
	    {"sloteditor_ok_proc",	    (caddr_t)SE_do_text_cb},
	    {"sloteditor_delete_proc",	    (caddr_t)SE_timeslot_delete_cb},
	    {"sloteditor_reset_proc",	    (caddr_t)SE_timeslot_reset_cb},
	    {"sloteditor_cancel_proc",	    (caddr_t)SE_timeslot_cancel_cb},
	    /*
	    **	Menus (File, Edit and Help)
	    */
	    {"sloteditor_inc_activate_proc",
		(caddr_t)SE_include_activate_cb},

	    {"sloteditor_edit_map_proc",	    (caddr_t)DO_TSE_EDIT_MENU},
	    {"sloteditor_cut_activate_proc",	    (caddr_t)DO_TSE_CUT},
	    {"sloteditor_copy_activate_proc",	    (caddr_t)DO_TSE_COPY},
	    {"sloteditor_paste_activate_proc",	    (caddr_t)DO_TSE_PASTE},
	    {"sloteditor_clear_activate_proc",	    (caddr_t)DO_TSE_CLEAR},
	    {"sloteditor_sel_activate_proc",	    (caddr_t)DO_TSE_SELALL},
	    /*
	    **
	    */	    
	    {"do_tse_include",		(caddr_t)SE_tse_include_cb},
	    {"do_tse_include_unmap",	(caddr_t)SE_tse_include_unmap_cb},
	};
    MrmCount regnum = XtNumber(regvec);
    MrmType	    class;
    Widget	    widget = NULL;
    int		    status;

    /*        
    **	Careful here!
    */
    regvec[0].value = (caddr_t)se;

    /*
    **  Let's register our routines
    */
    MrmRegisterNames(regvec, regnum);
    
    /*
    **  Let's fetch the popup attached dialog box
    */
    status = MrmFetchWidget
    (
	se->cd->ads->hierarchy,
	"sloteditor_formdialog",
	se->cd->toplevel,
	&widget,
	&class
    );

    if (status != MrmSUCCESS)
	DWC$UI_Catchall(DWC$DRM_NOSLOTED, status, 0);



    return;
}

static void SE_create_dne
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
    {
    static MrmRegisterArg regvec[] =
	{
	    /*
	    **  Identifiers
	    */
	    {"sloteditor",		    (caddr_t)0},    /* See code	    */
	    {"help_context",		    (caddr_t)0},    /* below.	    */
	    {"sloteditor_off_pixmap_array", (caddr_t)medium_pixmaps},
	    /*
	    **  Creation callbacks
	    */
	    {"sloteditor_create_proc",	    (caddr_t)SE_create_cb},
	    {"sloteditor_padb_create_proc", (caddr_t)SE_formdialog_create_cb},
	    /*
	    **	
	    */
	    {"sloteditor_stext_changed",
		(caddr_t)SE_text_changed_cb},
	    {"sloteditor_repeat_interval_proc",
		(caddr_t)SE_repeat_interval_cb},
	    {"sloteditor_attributes_proc",
		(caddr_t)SE_attibutes_cb},
	    {"sloteditor_special_proc",
		(caddr_t)SE_special_cb},
	    {"sloteditor_work_nonwork_proc",
		(caddr_t)SE_work_nonwork_cb},	
	    {"sloteditor_move_proc",
		(caddr_t)SE_move_cb},	
	    {"sloteditor_select_ibw",
		(caddr_t)SE_select_iconbox_cb},
	    {"sloteditor_order_ibw",
		(caddr_t)SE_order_iconbox_cb},
	    {"sloteditor_flag_changed",
		(caddr_t)SE_flag_changed_cb},
	    /*
	    **
	    */
	    {"condition_value_changed",	    (caddr_t)SE_condition_value_changed_cb},
	    /*
	    **  Buttons at the bottom of the dialog box
	    */
	    {"sloteditor_ok_proc",	    (caddr_t)SE_do_text_cb},
	    {"sloteditor_delete_proc",	    (caddr_t)SE_timeslot_delete_cb},
	    {"sloteditor_reset_proc",	    (caddr_t)SE_timeslot_reset_cb},
	    {"sloteditor_cancel_proc",	    (caddr_t)SE_timeslot_cancel_cb},
	    /*
	    **	Menus (File, Edit and Help)
	    */
	    {"sloteditor_inc_activate_proc",
		(caddr_t)SE_include_activate_cb},

	    {"sloteditor_edit_map_proc",	    (caddr_t)DO_TSE_EDIT_MENU},
	    {"sloteditor_cut_activate_proc",	    (caddr_t)DO_TSE_CUT},
	    {"sloteditor_copy_activate_proc",	    (caddr_t)DO_TSE_COPY},
	    {"sloteditor_paste_activate_proc",	    (caddr_t)DO_TSE_PASTE},
	    {"sloteditor_clear_activate_proc",	    (caddr_t)DO_TSE_CLEAR},
	    {"sloteditor_sel_activate_proc",	    (caddr_t)DO_TSE_SELALL},
	    /*
	    **
	    */
	    {"do_tse_include",		(caddr_t)SE_tse_include_cb},
	    {"do_tse_include_unmap",	(caddr_t)SE_tse_include_unmap_cb},
	};
    MrmCount regnum = XtNumber(regvec);
    MrmType	    class;
    Widget	    widget;
    int		    status;

    /*        
    **	Careful here!
    */
    regvec[0].value = (caddr_t)se;
    
    /*
    **  Let's register our routines
    */
    MrmRegisterNames(regvec, regnum);
    
    /*
    **  Let's fetch the popup attached dialog box
    */
    status = MrmFetchWidget(se->cd->ads->hierarchy,
			    "daynoteeditor_formdialog",
			    se->cd->toplevel,
			    &widget,
			    &class);

    if (status != MrmSUCCESS)
	DWC$UI_Catchall(DWC$DRM_NODAYNOTEED, status, 0);

    SLOTEDITORWIDGETS(se)[k_ts_popup_attached_db] = widget;

    /*
    **	This is a ugly hack !!!
    */
    static_sloteditor = se;
    
    /*
    **  That's it
    */
    return;
    }

static void SE_fillin_tse
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
{
    XmString	    xm_text;
    long	    byte_count, cvt_status;

    SE_setup_labels(se);

    DXmCSTextSetString
	((DXmCSTextWidget)SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext], se->text);
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext],
	XmNtopPosition, 0,
	XmNcursorPosition, MISCXmStringCharCount(se->text),
	XmNautoShowCursorPosition, True,
	XmNcursorPositionVisible, True,
	NULL
    );

    SE_setup_repeats(se);				      

    SE_setup_flags (se);

    SE_setup_icons(se);

    SE_setup_alarms(se);

    if (se->cd->read_only)
    {
	SE_readonly_editor(se);
    }

    return;
}

static void SE_fillin_dne
#if defined(_DWC_PROTO_)
	(
	Sloteditor	se)
#else	/* no prototypes */
	(se)
	Sloteditor	se;
#endif	/* prototype */
{
    XmString	    xm_text;
    long	    byte_count, cvt_status;

    SE_setup_labels(se);

    DXmCSTextSetString
	((DXmCSTextWidget)SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext], se->text);

    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext],
	XmNtopPosition, 0,
	XmNcursorPosition, 0,
	XmNcursorPositionVisible, TRUE,
	XmNautoShowCursorPosition, TRUE,
	NULL
    );
	
    SE_setup_repeats(se);				      

    SE_setup_flags(se);

    SE_setup_icons(se);
        
    if (se->cd->read_only)
    {
	SE_readonly_editor(se);
    }

    return;
}

static void SE_text_changed_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
    {
    Sloteditor se = (Sloteditor)tag;
    
    se->stext_changed = TRUE;    
    
    return;
}

static void SE_alarm_changed_cb
#if defined(_DWC_PROTO_)
	(
	Widget				widget,
	caddr_t				*tag,
	XmToggleButtonCallbackStruct	*cb)
#else	/* no prototypes */
	(widget, tag, cb)
	Widget				widget;
	caddr_t				*tag;
	XmToggleButtonCallbackStruct	*cb;
#endif	/* prototype */
    {
    int		    status;
    CalendarDisplay cd;
    Sloteditor	    se;

    status = MISCFindCalendarDisplay(&cd, widget);
    SE_find_sloteditor(&se, cd, widget, (int)*tag);

    se->alarms_changed = TRUE;

    if ((int)*tag != k_ts_entry_time_toggle)
	{
	/*
	**  We only care about the toggles which require a scale and a second
	**  toggle on their right
	*/

        if (cb->set)
	    {
	    /*
	    **  An alarm is turned on
	    */
	    if (se->current_scale != 0)
		XmToggleButtonSetState (
		    SLOTEDITORWIDGETS(se)[se->current_scale],
		    FALSE, FALSE);
            /*	  
            **  Set the ...val_toggle sensitive and true. Take a look at
	    **	dwc_ui_sloteditor_const.h to figure out the +4.
            */	  
            XtSetSensitive(SLOTEDITORWIDGETS(se)[((int)(*tag))+4], TRUE);
	    XmToggleButtonSetState (SLOTEDITORWIDGETS(se)[((int)(*tag))+4],
		TRUE, FALSE);
	    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_alarm_scale], TRUE);
	    SE_setup_alarm_scale(se, ((int)(*tag))+4);
	    se->current_scale = ((Cardinal)(*tag))+4;
	    }
	    else
	    {
	    /*								    
	    **  The alarm was turned off
	    */
	    if ( se->current_scale == (((Cardinal)(*tag))+4) )
	    	{
		XmToggleButtonSetState (
		    SLOTEDITORWIDGETS(se)[se->current_scale], FALSE, FALSE);
		XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_alarm_scale], FALSE);
		se->current_scale = 0;
		}
	    XtSetSensitive( SLOTEDITORWIDGETS(se)[((int)(*tag))+4], FALSE);
	    }
	}
    
    return;
}

static void SE_alarm_value_changed_cb
#if defined(_DWC_PROTO_)
	(
	Widget				widget,
	caddr_t				*tag,
	XmToggleButtonCallbackStruct	*cb)
#else	/* no prototypes */
	(widget, tag, cb)
	Widget				widget;
	caddr_t				*tag;
	XmToggleButtonCallbackStruct	*cb;
#endif	/* prototype */
{
    int		    status;
    CalendarDisplay cd;
    Sloteditor	    se;

    status = MISCFindCalendarDisplay(&cd, widget);
    SE_find_sloteditor(&se, cd, widget, (int)*tag);

    se->alarms_changed = TRUE;

    if (cb->set)
	{
	if (se->current_scale != 0)
	    {
	    /*
	    **  If there is another scale value turned on else where, we
	    **  turn it off.
	    */
	    XmToggleButtonSetState (
		SLOTEDITORWIDGETS(se)[se->current_scale],
		FALSE, FALSE);
	    }
	if (!cd->read_only)
	    {
	    /*
	    **	The scale is set sensitive only if database isn't read
	    **	ony; if it is, it stays insensitive, but it's value
	    **	get updated.
	    */	    
	    XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_alarm_scale], TRUE);
	    }
	SE_setup_alarm_scale(se, (int)*tag);
	se->current_scale = (int)*tag;
	}
    else
	{
	/*
	**  The currently selected scale value is turned off
	*/
	XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_alarm_scale], FALSE);
	se->current_scale = 0;
	}
	
    return;
}

static void SE_sloteditor_alarm_scale_changed_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmScaleCallbackStruct	*cb)
#else	/* no prototypes */
	(widget, tag, cb)
	Widget			widget;
	caddr_t			*tag;
	XmScaleCallbackStruct	*cb;
#endif	/* prototype */
{
    Sloteditor	se = (Sloteditor)tag;
    Cardinal	minutes_prior;

    se->alarms_changed = TRUE;

    if (se->ignore_scale_value_changed)
	return;

    if ((cb->reason == (int)XmCR_DRAG) &&
        (! se->cd->profile.direct_scroll_coupling))
    {
	return;
    }

    switch (se->current_scale)
	{
	case k_ts_short_val_toggle:
	    minutes_prior = cb->value;
	    break;
	case k_ts_medium_val_toggle:
	    minutes_prior = cb->value;
	    break;
	case k_ts_long_val_toggle:
	    minutes_prior = (cb->value * 60) / 10;
	    break;
	case k_ts_vlong_val_toggle:
	    minutes_prior = (cb->value * 60 * 24) / 10;
	    break;
	}

    se->scale_latest_value[se->current_scale - k_ts_short_val_toggle]
	= minutes_prior;

    SE_setup_alarm_value(se, se->current_scale, minutes_prior);
    
    return;
}

static void SE_include_activate_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*callback_data)
#else	/* no prototypes */
	(widget, tag, callback_data)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*callback_data;
#endif	/* prototype */
{
    Sloteditor	se = (Sloteditor)tag;
    int		status;
    MrmType	class;

    /*
    **  This may take a while
    */
    ICONSWaitCursorDisplay (se->cd->mainwid, se->cd->ads->wait_cursor);
    
    /*
    **  If the modal include dialog box hasn't been reused before, we create it
    **	from scractch; otherwise, we just managed the old one
    */
    
    if (se->file_selection_widget == NULL)
	{	
	status = MrmFetchWidget( se->cd->ads->hierarchy,
	    "timeslot_include_filebox",
	    se->cd->toplevel,
	    &se->file_selection_widget,
	    &class);

	if (status != MrmSUCCESS)
	    DWC$UI_Catchall(DWC$DRM_NOINCLUDE, status, 0);
	}

    /*
    **  On the screen
    */
    XtManageChild(se->file_selection_widget);

    ICONSWaitCursorRemove (se->cd->mainwid);
    ICONSInactiveCursorDisplay( se->file_selection_widget, 
	    se->cd->ads->inactive_cursor );

    return;
}

static void SE_formdialog_create_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*callback_data)
#else	/* no prototypes */
	(widget, tag, callback_data)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*callback_data;
#endif	/* prototype */
{
    Sloteditor se = (Sloteditor)tag;

    SLOTEDITORWIDGETS(se)[k_ts_popup_attached_db] = widget;

    /*
    **	This is a ugly hack !!! This is done so that when we get the
    **	MrmNcreateCallback from creating the various parts of the sloteditor we
    **	have a place to store the individual widgets.
    */
    static_sloteditor = se;

    return;
}

static void SE_create_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*callback_data)
#else	/* no prototypes */
	(widget, tag, callback_data)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*callback_data;
#endif	/* prototype */
{
    SLOTEDITORWIDGETS(static_sloteditor)[(int)(*tag)] = widget;

    return;   
}				   

#if 0
/*	this looks commented out					    */
/*									    */
static void SE_schedule_update_entry
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Boolean		update)
#else	/* no prototypes */
	(cd, update)
	CalendarDisplay	cd;
	Boolean		update;
#endif	/* prototype */
{
    Cardinal		repeat_interval;
    char		*text;
    XmString	        xm_text;
    long	        byte_count, cvt_status;

    xm_text = DXmCSTextGetString (SLOTEDITORWIDGETS(cd)[k_ts_timeslot_stext]);

    cd->timeslot_diu->text = xm_text;
    if (text != NULL)
	XtFree(text);
    cd->timeslot_diu->new = DAYCloneDayItem (cd, cd->timeslot_diu->old);
    if (cd->timeslot_diu->old == NULL)
    {
	cd->timeslot_diu->new->start_time =
	    cd->slot_start * cd->profile.timeslot_size;
	cd->timeslot_diu->new->duration =
	    (cd->slot_end - cd->slot_start + 1) * cd->profile.timeslot_size;
    }

    XtFree (cd->timeslot_diu->new->alarms_times);
    cd->timeslot_diu->new->alarms_times  = NULL;
    cd->timeslot_diu->new->alarms_number = 0;
    SE_get_entry_alarms
    (
	cd,
	&cd->timeslot_diu->new->alarms_number,
	&cd->timeslot_diu->new->alarms_times
    );

    switch (cd->repeat_interval)
    {
    case k_ts_none:
	repeat_interval = 0;
	break;
    case k_ts_daily:
	repeat_interval = 24 * 60;
	break;
    case k_ts_weekly:
	repeat_interval = 24 * 60 * 7;
	break;
    case k_ts_fortnight:
	repeat_interval = 24 * 60 * 7 * 2;
	break;
    case k_ts_fourweek:
	repeat_interval = 24 * 60 * 7 * 4;
	break;
    }

    if (repeat_interval == 0)
    {
	cd->timeslot_diu->new->repeat_p1 = _REPEAT_NONE;
    }
    else
    {
	cd->timeslot_diu->new->repeat_p1 = _REPEAT_ABSOLUTE;
    }
    cd->timeslot_diu->new->repeat_p2 = repeat_interval;

    /*
    **  Now here is a hack and a half....blow away the stored text so that
    **	DwcTswCompleteOutstanding will decide it has changed and call our
    **	callback.  This is to deal with the case where the text has not been
    **	modified, but the alarms etc have.
    */

/*
    if ((cd->texts [cd->slot_start] != NULL) && (! cd->read_only)) {
	XtFree (cd->texts [cd->slot_start]);
	cd->texts [cd->slot_start] = XtMalloc (2);
	cd->texts [cd->slot_start][0] = ~text [0];
	cd->texts [cd->slot_start][1] = '\0';
    }	

    if (update) {
	DwcTsdSetSelectedText (cd->day_tsd, text);
    }
*/
    
}
#endif

static void SE_close_action
#if defined(_DWC_PROTO_)
	(
	Widget		w,
	XEvent		*keyevent)
#else	/* no prototypes */
	(w, keyevent)
	Widget		w;
	XEvent		*keyevent;
#endif	/* prototype */
/*
**++
**  Functional Description:
**	SE_close_action
**	This is the action routine called when the user hits the ENTER key in
**	one of the slot editor's text widget. It calls the activate routine for
**	the 'Ok' push button. It packages the XEvent as a callback.
**
**  Keywords:
**	None.
**
**  Arguments:
**	TBD.
**
**  Result:
**	TBD.
**
**  Exceptions:
**	None.
**--
*/
{
    int		    status;
    XmAnyCallbackStruct constructed_cbs;
    CalendarDisplay cd;
    Sloteditor	    se;

    status = MISCFindCalendarDisplay(&cd, w);
    SE_find_sloteditor(&se, cd, w, k_ts_timeslot_stext);

    constructed_cbs.reason = 1;             /* dummy reason */
    constructed_cbs.event = keyevent;
    /*
    ** we want to act as though the user hit the ok button
    */
    SE_do_text_cb (w, (caddr_t *) se, &constructed_cbs);

    return;     
}     

static void SE_do_text_cb
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    RepeatChangeKind	    repeat_change_kind = RCKNotRepeat;
    Boolean		    ask_repeating_entry_question = TRUE;
    Boolean		    update_ok;
    int			    status;
    Cardinal		    repeat_interval;
    char		    *text;
    DSWEntryDataStruct	    data;
    DwcDayItem		    new;
    DayItemUpdate	    diu;
    Time		    time;
    Widget		    se_adb;
    Cardinal		    i;
    Boolean		    something_changed;
    Sloteditor		    se = (Sloteditor) tag;
    CalendarDisplay	    cd = se->cd;
    XmString		    xm_text;
    long		    byte_count, cvt_status;

    time = MISCGetTimeFromAnyCBS (cbs);

    /*	  
    **  Put input focus back on the main window
    */
    XmProcessTraversal ((Widget)cd->pb_day_time, XmTRAVERSE_CURRENT);

    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */

    if (cd->read_only)
    {
	/*
	**  If the Calendar is read only, then clicking on Ok is equivalent
	**  to clicking on cancel.
	*/	
	SE_timeslot_cancel_cb(w, tag, cbs);
	return;
    }

    se_adb = SLOTEDITORWIDGETS (se) [k_ts_popup_attached_db];

    for (i = 0; i < cd->number_of_sloteditors; i++)
    {
	if (cd->ses [i]->editor == se->editor)
	{
	    if (cd->ses [i] == se)
	    {
		SE_save_editor_position (cd, i);
	    }
	    break;
	}
    }

    if (se->old->item_id == 0)
    {
	/*
	**  This is a new entry
	*/
	new  = se->old;
	new->start_time = se->start_time;
	new->duration   = se->duration;
	new->entry      = se->link;

	SE_get_repeats(se, new);

        SE_get_flags(se, new);

	SE_get_icons(se, new);
	
	if (se->editor == SlotEditor)
	{
	    SE_get_alarms(se, new);
	} 

	xm_text = DXmCSTextGetString
	    (SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext]);

	diu  = DAYCreateDayItemUpdateRecord
	    (cd, se->dsbot, xm_text, NULL, new, RCKNotRepeat);
	diu->se = se;
	diu->slots = se->slots;

	update_ok = DAYDoDayItemUpdate (diu, time);

	if (update_ok)
	{
	    XtUnmanageChild (se_adb);
	    XtUnmapWidget (XtParent(se_adb));
	    se->in_use = FALSE;
	    XmStringFree (se->text);
	}
	DAYDestroyDayItemUpdate (diu);    
	return;
    } 

    /*
    **  Not a new item...
    */
    something_changed = se->text_changed    ||
	    (se->start_time != se->old->start_time) ||
	    (se->duration   != se->old->duration)   ||
	    se->stext_changed	||
	    se->icons_changed	||
	    se->alarms_changed	||
	    se->repeat_changed	||
	    se->flag_changed;

    if (! something_changed)
    {
        /*	  
	**  Nothing changed
	*/	  
        if (se->link != NULL)
	{
	    DSWCloseEntry ((DayslotsWidget) se->slots->widget, se->link, time);
	    DSWSetEntryEditable (se->link, ! cd->read_only, time);
	    se->link = NULL;
	} 

	XtUnmanageChild (se_adb);
	XtUnmapWidget (XtParent(se_adb));
	se->in_use = FALSE;
	DAYDestroyDayItem (se->old);
	XmStringFree (se->text);
	/*
	**  That's it
	*/
	return;
    } 
    
    /*
    **	Something did change... Let's query various parts of the dialog box
    **	and fill in 'new'.
    */
    
    new  = DAYCloneDayItem (cd, se->old);
    new->start_time = se->start_time;
    new->duration   = se->duration;
    new->entry      = se->link;

    SE_get_repeats (se, new);

    if ( (se->old->repeat_p1 != new->repeat_p1) ||
	 (se->old->repeat_p2 != new->repeat_p2) ||
	 (se->old->repeat_p3 != new->repeat_p3) )
    {
	repeat_change_kind = RCKRepeatChange;
	ask_repeating_entry_question = FALSE;
    }
    else if (new->repeat_p1 == DWC$k_db_none)
    {
	/*
	**  The repeat parameters were not touched, but it's a non repeating
	**  entry, so we shouldn't ask any question.
	*/
	ask_repeating_entry_question = FALSE;
    }
	
    SE_get_flags(se, new);

    SE_get_icons(se, new);

    if (se->editor == SlotEditor)
    {
	SE_get_alarms(se, new);
    } 

    xm_text = DXmCSTextGetString (SLOTEDITORWIDGETS (se) [k_ts_timeslot_stext]);

    diu  = DAYCreateDayItemUpdateRecord
    (
	cd,
	se->dsbot,
	xm_text,
	DAYCloneDayItem(cd, se->old),
	new,
	repeat_change_kind
    );
    diu->se = se;
    diu->slots = se->slots;
    
    if (ask_repeating_entry_question)
    {
	SEPoseRepeatingEntryQuestions (cd, dwc_k_pose_modify_text, diu);
	return;
    } 

    update_ok = DAYDoDayItemUpdate (diu, time);

    if (update_ok)
    {
	XtUnmanageChild (se_adb);
	XtUnmapWidget (XtParent(se_adb));
	se->in_use = FALSE;
	XmStringFree (se->text);
	DAYDestroyDayItem (se->old);	
    }
	
    DAYDestroyDayItemUpdate (diu);    
    
    /*
    **  That's it
    */	    
    return;
} 

static void SE_schedule_entry_delete
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget			widget;
	caddr_t			tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    DayItemUpdate	    diu;
    Widget		    se_adb;
    Time		    time;
    Sloteditor		    se = (Sloteditor) tag;
    CalendarDisplay	    cd = se->cd;
    Boolean		    update_ok;

    se_adb = SLOTEDITORWIDGETS (se) [k_ts_popup_attached_db];

    time = MISCGetTimeFromAnyCBS (cbs);

    if (cbs->reason == (int)XmCR_CANCEL)
    {
	XmProcessTraversal (se_adb, XmTRAVERSE_CURRENT);

	/*
	** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
	*/
    }
    else
    {
	diu = DAYCreateDayItemUpdateRecord
	    (cd, se->dsbot, NULL, se->old, NULL, RCKNotRepeat);
	diu->slots = se->slots;
	update_ok = DAYDoDayItemUpdate (diu, time);
	if (update_ok)
	{
	    XtUnmanageChild (se_adb);
	    XtUnmapWidget (XtParent(se_adb));
	    XmStringFree (se->text);
	    se->in_use = FALSE;	    
	}
	DAYDestroyDayItemUpdate (diu);    
    } 
        
}

static void SE_timeslot_delete_cb
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    DayItemUpdate	diu;
    Cardinal		i;
    Sloteditor		se = (Sloteditor) tag;
    CalendarDisplay	cd = se->cd;


    /*	  
    **  If this is a new entry, just do a cancel
    */	  
    if (se->old->item_id == 0)
	{
	SE_timeslot_cancel_cb(w, tag, cbs);
	return;
	}


    for (i = 0; i < cd->number_of_sloteditors; i++) {
	if (cd->ses [i]->editor == se->editor) {
	    if (cd->ses [i] == se) {
		SE_save_editor_position (cd, i);
	    }
	    break;
	}
    }

    se->old->entry = se->link;

    if (se->old->repeat_p1 == DWC$k_db_none)
    {
	ERRORReportError
	(
	    SLOTEDITORWIDGETS (se) [k_ts_popup_attached_db],
	    "CautionDelete",
	    (XtCallbackProc)SE_schedule_entry_delete,
	    (caddr_t)se
	);
    }
    else
    {
	diu = DAYCreateDayItemUpdateRecord
	(
	    cd,
	    se->dsbot,
	    NULL,
	    DAYCloneDayItem(cd, se->old),
	    NULL,
	    RCKNotRepeat
	);
	diu->se = se;
	diu->slots = se->slots;
	SEPoseRepeatingEntryQuestions (cd, dwc_k_pose_delete_text, diu);
    }

}

static void SE_timeslot_reset_cb
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    Sloteditor		    se = (Sloteditor) tag;
    CalendarDisplay	    cd = se->cd;

    if (se->editor == SlotEditor)
	SE_fillin_tse(se);
    else
	SE_fillin_dne(se);

    /*
    **  That's it
    */
    return;
} 

static void SE_timeslot_cancel_cb
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    Time		    time;
    Widget		    se_adb;
    Cardinal		    i;
    Sloteditor		    se = (Sloteditor) tag;
    CalendarDisplay	    cd = se->cd;

    se_adb = SLOTEDITORWIDGETS (se) [k_ts_popup_attached_db];

    XtUnmanageChild (se_adb);

    for (i = 0; i < cd->number_of_sloteditors; i++)
    {
	if (cd->ses [i]->editor == se->editor)
	{
	    if (cd->ses [i] == se)
	    {
		SE_save_editor_position (cd, i);
	    }
	    break;
	}
    }

    XtUnmapWidget(XtParent(se_adb));

    time = MISCGetTimeFromAnyCBS (cbs);

    if (se->link != NULL)
    {
	DAYResetTimeslotEntry (cd, se->slots, se->dsbot, se->link, time);
    }
    
    XmStringFree (se->text);

    DAYDestroyDayItem (se->old);
    se->old  = NULL;
    se->link = NULL;

    se->in_use = FALSE;

    XmProcessTraversal ((Widget)cd->pb_day_time, XmTRAVERSE_CURRENT);

}

static void SE_timeslot_close_cb
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    WidgetList		children;
    Widget		form;
    Widget		appshell;
    Sloteditor		se;
    CalendarDisplay	cd;
    int			status;
    int			num_children;
    int			i;

    /*
    ** Find the form child of the shell.  This is done by look for the child
    ** of the shell which is a form.  There may be other children (protocol
    ** widgets) which we need to ignore.
    */
    children = DXmChildren ((CompositeWidget) w);
    num_children = DXmNumChildren ((CompositeWidget) w);

    for (i = 0; i < num_children; i++)
	if (XmIsForm(children[i])) break;

    if (i >= num_children) return;  /* no form found, punt */

    form = children[i];

    /*
    ** Find its main window shell.
    */
    XtVaGetValues (w, XmNtransientFor, &appshell, NULL);

    /*
    ** Get the CalendarDisplay struct.
    */
    status = MISCFindCalendarDisplay (&cd, appshell);

    /*
    ** Now that we have that, we can get the sloteditor struct.
    */
    SE_find_sloteditor (&se, cd, form, k_ts_popup_attached_db);

    /*
    ** Actually do the cancel using the "standard" callback.
    */
    SE_timeslot_cancel_cb (w, (caddr_t *)se, cbs);
}

static void SE_repeat_interval_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    Widget	    local_widget;
    int		    status;
    CalendarDisplay cd;    
    Sloteditor	    se;
    
    status = MISCFindCalendarDisplay(&cd, widget);
    SE_find_sloteditor(&se, cd, widget, (int)*tag);				      

    se->repeat_changed = TRUE;
		
    se->repeat_interval = (int)*tag;

    if (se->repeat_interval >= k_ts_daily_button)
    {
	if (!XtIsSensitive(SLOTEDITORWIDGETS(se)[k_ts_condition_toggle]))
	{
	    XtSetSensitive
		(SLOTEDITORWIDGETS(se)[k_ts_condition_toggle], True);
	    XmToggleButtonSetState
		(SLOTEDITORWIDGETS(se)[k_ts_condition_toggle], False, False);
	    se->condition = FALSE;
	}
    }
    else
    {
        if (XtIsSensitive(SLOTEDITORWIDGETS(se)[k_ts_condition_toggle]))
	{    
	    XmToggleButtonSetState
		(SLOTEDITORWIDGETS(se)[k_ts_condition_toggle], False, False);
	    se->condition = FALSE;
	    XtSetSensitive
		(SLOTEDITORWIDGETS(se)[k_ts_condition_toggle], False);
	    XtSetSensitive
		(SLOTEDITORWIDGETS(se)[k_ts_work_nonwork_optmenu], False);
	    XtSetSensitive
		(SLOTEDITORWIDGETS(se)[k_ts_move_optmenu], False);
	}
    }
	    
    if (se->repeat_interval >= k_ts_monthly_button)
    {
	XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_attributes_optmenu], True);
    }
    else
    {
	XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_attributes_optmenu], False);
    }
						      
    /*
    **  That's it
    */
    return;
}

static void SE_attibutes_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    int		    status;				      
    CalendarDisplay cd;    
    Sloteditor	    se;
    				      
    status = MISCFindCalendarDisplay(&cd, widget);
    SE_find_sloteditor(&se, cd, widget, (int)*tag);				      

    se->repeat_changed = TRUE;

    se->attributes = (Cardinal)*tag;

    /*
    **  That's it
    */
    return;
}

static void SE_special_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
/*
**++
**  Functional Description:
**	SE_special_cb
**	This is the callback routine activated when the user selects one of
**	the items in the sub-pulldown menu hanging from the attributes option
**	menu, or when the user selects the corresponding menu entry in the
**	option menu.
**
**  Keywords:
**	None.
**
**  Arguments:
**	TBD.
**
**  Result:
**	TBD..
**
**  Exceptions:
**	None.
**--
*/
#endif	/* prototype */
{
    XmString			text;
    int				status;
    dtb				date_time;
    int				day,
				temp_day,
				month,
				year;
    CalendarDisplay		cd;
    Sloteditor			se;
    struct DWC$db_time_ctx	time_context;
    int				index;
    Boolean			found = FALSE;
    			      
    /*
    **	Let's look for the associated sloteditor
    */
    status = MISCFindCalendarDisplay(&cd, widget);
    SE_find_sloteditor(&se, cd, widget, (int)*tag);				      

    /*
    **  We write down that the user has started messing with repeats
    */
    se->repeat_changed = TRUE;

    /*
    **  'attributes' will hold the currently chosen repeat attribute
    */
    se->attributes = k_ts_special_day_menu_entry;

    /*
    **	'cwd_day_of_month' and 'cwd_cond_forward' will hold the state of this
    **	attribute.
    */
    DATEFUNCDateForDayNumber (se->dsbot, &day, &month, &year);
    date_time.weekday = DATEFUNCDayOfWeek (day, month, year);	
    date_time.day   = day;
    date_time.month = month;
    date_time.year  = year;

    DWC$DB_Build_time_ctx (se->cd->cab, se->dsbot, 1, &time_context);

    temp_day = day;

    if ( widget == SLOTEDITORWIDGETS(se)[k_ts_special_day_menu_entry])
	{
	found = TRUE;
	se->cwd_day_of_month    = temp_day;
	se->cwd_cond_forward    = TRUE;	
	}

    if (!found)
	{
	temp_day = day;
	for (index = k_ts_after_0_button; index >= k_ts_after_6_button; index--)
	    {
	    if (temp_day == 0)
		{
		temp_day = time_context.DWC$b_dbtc_days_prev_month;	    
		}
	    if (index == (int)*tag)
		{
		se->cwd_day_of_month    = temp_day;
		se->cwd_cond_forward    = TRUE;
		found		    = TRUE;
		break;
		}
	    temp_day--;
	    }
	}

	
    if (!found)
	{
	temp_day = day;
	for (index= k_ts_before_0_button; index<= k_ts_before_6_button; index++)
	    {
	    if (temp_day > time_context.DWC$b_dbtc_days_in_month)
		{
		temp_day = 1;
		}
	    if (index == (int)*tag)
		{
		se->cwd_day_of_month = temp_day;
		se->cwd_cond_forward = FALSE;
		break;
		}
	    temp_day++;
	    }	
	}

    date_time.day = se->cwd_day_of_month;
    if (se->cwd_cond_forward)
	text = (XmString)DATEFORMATTimeToCS
	    (dwc_k_ts_first_x_after_format, &date_time);
    else
	text = (XmString)DATEFORMATTimeToCS
	    (dwc_k_ts_first_x_before_format, &date_time);
    XtVaSetValues
    (
	SLOTEDITORWIDGETS(se)[k_ts_special_day_menu_entry],
	XmNlabelString, text,
	NULL
    );
    XmStringFree(text);    
	
    /*
    **  That's it
    */
    return;
}

static void SE_condition_value_changed_cb
#if defined(_DWC_PROTO_)
	(
	Widget				widget,
	caddr_t				*tag,
	XmToggleButtonCallbackStruct	*cb)
#else	/* no prototypes */
	(widget, tag, cb)
	Widget				widget;
	caddr_t				*tag;
	XmToggleButtonCallbackStruct	*cb;
#endif	/* prototype */
{
    Sloteditor se = (Sloteditor)tag;

    se->repeat_changed = TRUE;

    se->condition = cb->set;		

    if (se->condition)
    {
        XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_work_nonwork_optmenu], TRUE);
        XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_move_optmenu], TRUE);
    }
    else
    {
        XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_work_nonwork_optmenu], FALSE);
        XtSetSensitive(SLOTEDITORWIDGETS(se)[k_ts_move_optmenu], FALSE);
    }
			 
    return;		
}	   

static void SE_work_nonwork_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    int		    status;				      
    CalendarDisplay cd;    
    Sloteditor	    se;
    				      
    status = MISCFindCalendarDisplay(&cd, widget);
    SE_find_sloteditor(&se, cd, widget, (int)*tag);				      

    se->repeat_changed = TRUE;

    se->work_nonwork = (Cardinal)*tag;

    return;		
}					 

static void SE_move_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    int		    status;				      
    CalendarDisplay cd;    
    Sloteditor	    se;
    				      
    status = MISCFindCalendarDisplay(&cd, widget);
    SE_find_sloteditor(&se, cd, widget, (int)*tag);				      

    se->repeat_changed = TRUE;

    se->move = (Cardinal)*tag;

    return;		
}

static void SE_select_iconbox_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	DwcIbwCallbackStruct	*cb)
#else	/* no prototypes */
	(widget, tag, cb)
	Widget			widget;
	caddr_t			*tag;
	DwcIbwCallbackStruct	*cb;
#endif	/* prototype */
{
    Sloteditor	    se = (Sloteditor)tag;
    Cardinal	    order_num_icons;
    unsigned char   *order_icons;
    unsigned char   *new_order_icons;
    Cardinal	    i,
		    j;
    
    se->icons_changed = TRUE;

    DwcIbwGetSelectedIcons
    (
	SLOTEDITORWIDGETS(se)[k_ts_order_ibw],
	&order_icons,
	&order_num_icons
    );

    if (cb->selected_icon_on)
    {
	/*
	**
	*/
	order_icons = (unsigned char *) XtRealloc
	(
	    (char *) order_icons,
	    sizeof(unsigned char) * (order_num_icons + 1)
	);
	order_icons[order_num_icons] = cb->selected_icon;
	DwcIbwSetSelectedIcons
	(
	    SLOTEDITORWIDGETS(se)[k_ts_order_ibw],
	    order_icons,
	    order_num_icons + 1
	);
    }
    else
    {
	/*
	**
	*/
	j = 0;
	new_order_icons = (unsigned char *)XtMalloc(
	    sizeof(unsigned char) * (order_num_icons - 1) );
	for (i=0; i< order_num_icons; i++)
	{
	    if ( order_icons[i] != cb->selected_icon )
	    {
		new_order_icons[j] = order_icons[i];
		j++;
	    }
	}
	DwcIbwSetSelectedIcons
	(
	    SLOTEDITORWIDGETS(se)[k_ts_order_ibw],
	    new_order_icons,
	    order_num_icons - 1
	);
	XtFree ((char *)new_order_icons);
    }
	    
    XtFree ((char *)order_icons);

    return;		
}

static void SE_order_iconbox_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	DwcIbwCallbackStruct	*cb)
#else	/* no prototypes */
	(widget, tag, cb)
	Widget			widget;
	caddr_t			*tag;
	DwcIbwCallbackStruct	*cb;
#endif	/* prototype */
{
    Sloteditor	    se = (Sloteditor)tag;

    se->icons_changed = TRUE;
    
    return;
}

static void SE_flag_changed_cb
#if defined(_DWC_PROTO_)
	(
	Widget				widget,
	caddr_t				*tag,
	XmToggleButtonCallbackStruct	*cbs)
#else	/* no prototypes */
	(widget, tag, cbs)
	Widget				widget;
	caddr_t				*tag;
	XmToggleButtonCallbackStruct	*cbs;
#endif	/* prototype */
{
    Sloteditor	    se = (Sloteditor)tag;

    se->flag_changed = TRUE;
    
    return;
}				    

static void SE_repeat_questions_cb
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    Time		time;
    CalendarDisplay	cd  = (CalendarDisplay)tag;
    DayItemUpdate	diu = cd->timeslot_diu;
    Boolean		update_ok;

    ICONSInactiveCursorRemove (cd->db_repeat_questions);

    XtUnmanageChild (cd->db_repeat_questions);

    time = MISCGetTimeFromAnyCBS (cbs);

    if (XmToggleButtonGetState (cd->tb_repeat_this)) {
	diu->kind = RCKThisInstance;
    } else {
	if (XmToggleButtonGetState (cd->tb_repeat_future)) {
	    diu->kind = RCKThisAndFuture;
	} else {
	    diu->kind = RCKAllInstances;
	}
    }

    update_ok = DAYDoDayItemUpdate (diu, time);

    if ( update_ok && (diu->se != NULL) )
	{
	XtUnmanageChild (SLOTEDITORWIDGETS (diu->se) [k_ts_popup_attached_db]);
	XmStringFree (diu->se->text);
	diu->se->in_use = FALSE;
	DAYDestroyDayItem (diu->se->old);	
	}
	
    DAYDestroyDayItemUpdate (diu);    

    DAYTraverseToClose (cd);

    return;
}

static void SE_repeat_cancel_cb
#if defined(_DWC_PROTO_)
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay	cd  = (CalendarDisplay)tag;
    DayItemUpdate	diu = cd->timeslot_diu;

    ICONSInactiveCursorRemove (cd->db_repeat_questions);

    XtUnmanageChild (cd->db_repeat_questions);

    DAYDestroyDayItemUpdate (diu);    

    DAYTraverseToOpen (cd);
}

void SEPoseRepeatingEntryQuestions
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	int		text_index,
	DayItemUpdate	diu)
#else	/* no prototypes */
	(cd, text_index, diu)
	CalendarDisplay	cd;
	int		text_index;
	DayItemUpdate	diu;
#endif	/* prototype */
{
    XmString		cs;
    WidgetList		children;
    int			num_children;

    static  MrmRegisterArg pose_regvec[] =
    {
	/*
	**  Identifiers
	*/
	{"diu",			(caddr_t)0},
	{"repeat_questions",	(caddr_t)SE_repeat_questions_cb},
	{"repeat_cancel",	(caddr_t)SE_repeat_cancel_cb}
    };
    static  MrmCount pose_regnum = XtNumber(pose_regvec);
    MrmType	    class;
    int		    status;


    if (cd->db_repeat_questions == NULL)
    {
	pose_regvec[0].value = (caddr_t)cd;
	MrmRegisterNames(pose_regvec, pose_regnum);

	status = MrmFetchWidget( cd->ads->hierarchy,
	    "repeat_questions_box",
	    cd->toplevel,
	    &cd->db_repeat_questions,
	    &class );

	if (status != MrmSUCCESS)
	    DWC$UI_Catchall(DWC$DRM_NOREPEAT, status, 0);

	/*
	** Force all of the children to the same size.
	*/
	children = DXmChildren
	    ((CompositeWidget)XtParent(cd->pb_repeat_cancel));
	num_children = DXmNumChildren
	    ((CompositeWidget)XtParent(cd->pb_repeat_cancel));
	MISCWidthButtonsEqually (children, num_children);

    } 
    else
    {
	XmToggleButtonSetState( cd->tb_repeat_this, TRUE, TRUE );

	children = DXmChildren
	    ((CompositeWidget)XtParent(cd->pb_repeat_cancel));
	num_children = DXmNumChildren
	    ((CompositeWidget)XtParent(cd->pb_repeat_cancel));

	};

    cd->timeslot_diu = diu;
    
    cs = MISCFetchDRMValue (text_index);		/* don't free string */
    XtVaSetValues (cd->lb_repeat_prompt, XmNlabelString, cs, NULL);

    /*
    ** Make the CLOSE item in the window manager menu call the CANCEL
    ** callback.
    */
    MISCAddProtocols
    (
	XtParent(cd->db_repeat_questions),
	(XtCallbackProc) SE_repeat_cancel_cb,
	NULL
    );

    /*
    ** Get it on the screen.
    */
    XtManageChild (cd->db_repeat_questions);    

    /*
    ** Now that we are sure it is up, we can make the box stretch to the
    ** parent.
    */
    XtVaSetValues
    (
	XtParent(cd->pb_repeat_cancel),
	XmNrightAttachment, XmATTACH_FORM,
	NULL
    );

    /*
    ** Change the button attachments.
    */
    MISCSpaceButtonsEqually (children, num_children);

    /*
    ** Tell the dialog to take focus if no explict focusing has been done.
    */
    MISCFocusOnMap (XtParent(cd->db_repeat_questions), cd->pb_repeat_cancel);

    ICONSInactiveCursorDisplay (cd->db_repeat_questions, ads.inactive_cursor);

}

#if 0
/*	This looks commented out.					    */
/*									    */
static void SE_file_activate_cb
#if defined(_DWC_PROTO_)
	(
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*callback_data)
#else	/* no prototypes */
	(widget, tag, callback_data)
	Widget			widget;
	caddr_t			*tag;
	XmAnyCallbackStruct	*callback_data;
#endif	/* prototype */
{
}
#endif

static void SE_get_entry_alarms
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	int			*alarms_number_return,
	unsigned short int	**alarms_times_return)
#else	/* no prototypes */
	(cd, alarms_number_return, alarms_times_return)
	CalendarDisplay		cd;
	int			*alarms_number_return;
	unsigned short int	**alarms_times_return;
#endif	/* prototype */
{
    Cardinal		alarms_number;
    unsigned short int  *alarms_times;
    Boolean		alarm_entry;
    Boolean		alarm_short;
    Boolean		alarm_medium;
    Boolean		alarm_long;
    Cardinal		alarm;
    int			scale;

    alarm_entry =
	XmToggleButtonGetState(SLOTEDITORWIDGETS(cd)[k_ts_entry_time_toggle]); 
    alarm_short =
	XmToggleButtonGetState(SLOTEDITORWIDGETS(cd)[k_ts_short_toggle]);
    alarm_medium =
	XmToggleButtonGetState(SLOTEDITORWIDGETS(cd)[k_ts_medium_toggle]);
    alarm_long   =
	XmToggleButtonGetState(SLOTEDITORWIDGETS(cd)[k_ts_long_toggle]);

    alarms_number = 0;
    if (alarm_entry)   alarms_number++;
    if (alarm_short)   alarms_number++;
    if (alarm_medium)  alarms_number++;
    if (alarm_long)    alarms_number++;

    if (alarms_number == 0) {
	alarms_times = NULL;
    } else {
	alarms_times = (unsigned short int *)
		       XtMalloc (sizeof (unsigned short int) * alarms_number);
	alarm = 0;
	if (alarm_entry) {
	    alarms_times [alarm++] = 0;
	}
	if (alarm_short) {
	    /*
	    XmScaleGetValue (SLOTEDITORWIDGETS(cd)[k_ts_short_scale],
		&scale);
	    */
	    alarms_times [alarm++] = (unsigned short int) scale;
	}
	if (alarm_medium) {
	    /*
	    XmScaleGetValue (SLOTEDITORWIDGETS(cd)[k_ts_medium_scale],
		&scale);
	    */
	    alarms_times [alarm++] = (unsigned short int) scale;
	}
	if (alarm_long) {
	    /*
	    XmScaleGetValue (SLOTEDITORWIDGETS(cd)[k_ts_long_scale],
		&scale);
	    */
	    alarms_times [alarm] = (unsigned short int) scale;
	    alarms_times [alarm] = alarms_times [alarm] * 60;
	}
    }

    *alarms_number_return = alarms_number;
    *alarms_times_return  = alarms_times;

}

static void SE_find_sloteditor
#if defined(_DWC_PROTO_)
	(
	Sloteditor	*se,
	CalendarDisplay	cd,
	Widget		widget,
	int		index)
#else	/* no prototypes */
	(se, cd, widget, index)
	Sloteditor	*se;
	CalendarDisplay	cd;
	Widget		widget;
	int		index;
#endif	/* prototype */
{
    int i;

    for (i=0; i < cd->number_of_sloteditors; i++)
	{
	if (   ( cd->ses[i]->in_use )
	    && ( SLOTEDITORWIDGETS(cd->ses[i])[index] == widget ) )
	    {
	    *se = cd->ses[i];
	    return;
       	    }
	}
}				      

void SEChangeAMPM
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    int		i;

    for (i = 0; i < cd->number_of_sloteditors; i++)
	if ( cd->ses[ i ]->in_use )
	    SE_setup_labels( cd->ses[ i ]);

    return;
}

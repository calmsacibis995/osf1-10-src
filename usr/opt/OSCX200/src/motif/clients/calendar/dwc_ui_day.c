/* dwc_ui_day.c */
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
**	This is the module that deals with day displays.
**
**--
*/

#include "dwc_compat.h"

#if defined(vaxc)
#pragma nostandard
#endif
#include <X11/Intrinsic.h>		/* for MIN */
#include <Xm/Xm.h>
#if (((XmVERSION == 1) && (XmREVISION == 2)) || XmVERSION == 2)
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/ScrollBar.h>		/* for XmScrollBarSetValues */
#if defined(vaxc)
#pragma standard
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

#include "dwc_ui_calendar.h"
#include "dwc_ui_dateformat.h"		/* dtb... */
#include "dwc_ui_timebarwidget.h"	/* DwcTbwNverticalMargin */
#include "dwc_ui_layout.h"		/* LwCallback... etc. */
#include "dwc_ui_day.h"	
#include "dwc_ui_catchall.h"		/* DWC$DRM */
#include "dwc_ui_misc.h"		/* for MISCSetAutoScroll */
#include "dwc_ui_alarms.h"		/* for ALARMSSetupNextAlarm */
#include "dwc_ui_yeardisplay.h"		/* for YEARSingleRowHeight */
#include "dwc_ui_clipboard.h"		/* for CLIPImportInterchange */
#include "dwc_ui_sloteditor.h"		/* for WhichEditor */
#include "dwc_ui_monthwidget.h"		/* for MWMARKUP and MWSTYLE */
#include "dwc_ui_icons.h"
#include "dwc_ui_errorboxes.h"
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_help.h"

typedef enum {DdiuAdd, DdiuDelete, DdiuChange, DdiuRemove} ddiu_action;
typedef enum {DdeaAdd, DdeaChange, DdeaClose, DdeaRemove} ddea_action;


static int DAY_change_day_item PROTOTYPE ((
	struct DWC$db_access_block	*cab,
	Cardinal			dsbot,
	DwcDayItem			old,
	DwcDayItem			new,
	XmString			text,
	RepeatChangeKind		kind));

static void DAY_entry_cb PROTOTYPE ((
	Widget				dsw,
	caddr_t				tag,
	DSWEntryCallbackStruct	*cbs));

static int DAY_delete_day_item PROTOTYPE ((
	struct DWC$db_access_block	*cab,
	Cardinal			dsbot,
	DwcDayItem			di,
	RepeatChangeKind		kind));

static void DAY_daynote_entry_cb PROTOTYPE ((
	Widget				dsw,
	caddr_t				tag,
	DSWEntryCallbackStruct	*cbs));

static void DAY_do_day_cb PROTOTYPE ((
	CalendarDisplay			cd,
	DwcDaySlots			ds,
	DSWEntryCallbackStruct	*cbs));

static Boolean DAY_do_day_item_update_action PROTOTYPE ((
	CalendarDisplay	cd,
	DayItemUpdate	diu,
	ddiu_action	action,
	Boolean		repeat_changed,
	Time		time));

static void DAY_entry_delete_cb PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

static void DAY_focus_cb PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	LwCallbackStruct	*cbs));

static void DAY_move_day_scrollbar_cb PROTOTYPE ((
	Widget				w,
	caddr_t				tag,
	XmScrollBarCallbackStruct	*scroll));

static void DAY_prev_day_cb PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs));

static void DAY_next_day_cb PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs));

static void DAY_position_day_scrollbar_cb PROTOTYPE ((
	Widget				w,
	caddr_t				tag,
	DSWScrollCallbackStruct	*cbs));

static int DAY_put_day_item PROTOTYPE ((
	struct DWC$db_access_block	*cab,
	Cardinal			dsbot,
	DwcDayItem			di,
	XmString			text));

static void DAY_quick_copy_or_move PROTOTYPE ((
	CalendarDisplay			cd,
	DwcDaySlots			ds,
	DSWEntryCallbackStruct	*cbs,
	Boolean				move));

static void DAY_reget_flags_for_item PROTOTYPE ((
	CalendarDisplay	cd,
	DwcDayItem	di,
	Cardinal	dsbot));

static void DAY_resize_cb PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs));

static void DAY_dispose_focus_cb PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs));

XtCallbackRec DAYday_entry_cb [2] =
{
    {(XtCallbackProc) DAY_entry_cb,		NULL},
    {NULL,					NULL}
};

XtCallbackRec DAYday_scroll_cb [2] =
{
    {(XtCallbackProc) DAY_move_day_scrollbar_cb,	NULL},
    {NULL,						NULL}
};

XtCallbackRec DAYdns_entry_cb [2] =
{
    {(XtCallbackProc) DAY_daynote_entry_cb,	NULL},
    {NULL,					NULL}
};

XtCallbackRec DAYfocus_day_cb [2] =
{
    {(XtCallbackProc) DAY_focus_cb,		NULL},
    {NULL,					NULL}
};

XtCallbackRec DAYpb_last_day_cb [2] =
{
    {(XtCallbackProc) DAY_prev_day_cb,		NULL},
    {NULL,					NULL}
};

XtCallbackRec DAYpb_next_day_cb [2] =
{
    {(XtCallbackProc) DAY_next_day_cb,		NULL},
    {NULL,					NULL}
};

XtCallbackRec DAYposition_scrollbar_cb [2] =
{
    {(XtCallbackProc) DAY_position_day_scrollbar_cb,	NULL},
    {NULL,						NULL}
};

XtCallbackRec DAYresize_day_cb [2] =
{
    {(XtCallbackProc) DAY_resize_cb,		NULL},
    {NULL,					NULL}
};

XtCallbackRec DAYdisp_focus_cb [2] =
{
    {(XtCallbackProc) DAY_dispose_focus_cb,	NULL},
    {NULL,					NULL}
};

Boolean DAYTestAnyOpenEntries
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {

    if (DSWGetOpenEntry ((DayslotsWidget) (cd->daynotes.widget)) != NULL)
    {
	return (TRUE);
    }

    if (DSWGetOpenEntry ((DayslotsWidget) (cd->dayslots.widget)) != NULL)
    {
	return (TRUE);
    }

    if (SEAnySlotEditorsUpAndRunning (cd))
    {
	return (TRUE);
    }
    
    if (SEAnyNoteEditorsUpAndRunning (cd))
    {
	return (TRUE);
    }
	
    
    return (FALSE);
    
}

void DAYGetIconsForDayItem
#if defined(_DWC_PROTO_)
	(
	DwcDayItem	di,
	unsigned char	**ret_icons,
	Cardinal	*ret_num_icons)
#else	/* no prototypes */
	(di, ret_icons, ret_num_icons)
	DwcDayItem	di;
	unsigned char	**ret_icons;
	Cardinal	*ret_num_icons;
#endif	/* prototype */
{
    Cardinal	    num_icons;
    unsigned char   icon;
    unsigned char   *icons;
    int		    size;
    Cardinal	    last;
    int		    flags;


    last = num_icons = di->icons_number;

    if (di->repeat_p1 != DWC$k_db_none)	num_icons++;
    if (di->alarms_number != 0) 	num_icons++;
    if (di->memexized)			num_icons++;


    *ret_num_icons = num_icons;
    if (num_icons == 0)
    {
	*ret_icons = NULL;
	return;
    }

    icons = (unsigned char *) XtMalloc (sizeof (unsigned char) * num_icons);
    if (last != 0)
    {
	size = sizeof (unsigned char) * last;
	memcpy (icons, di->icons, size);
    }

    if (di->repeat_p1 != DWC$k_db_none)
    {
	flags = di->flags & DWC$m_item_rpos;
	switch (flags)
	{
	case DWC$k_item_first:
	    icon = k_pixmap_repeatstart;
	    break;
	case DWC$k_item_middle:
	    icon = k_pixmap_repeat;
	    break;
	case DWC$k_item_last:
	    icon = k_pixmap_repeatend;
	    break;
	default:
	    icon = k_pixmap_repeat;
	    break;
	}
        icons [last++] = icon;
    }

    if (di->alarms_number != 0) 	 icons [last++] = k_pixmap_bell;
    if (di->memexized)			icons [last++] = k_pixmap_memex;

    *ret_icons = icons;

}

DwcDayItem DAYCreateDayItemRecord
#if defined(_DWC_PROTO_)
	(
	Cardinal		alarms_number,
	unsigned short int	*alarms_times,
	Cardinal		icons_number,
	unsigned char		*icons)
#else	/* no prototypes */
	(alarms_number, alarms_times, icons_number, icons)
	Cardinal		alarms_number;
	unsigned short int	*alarms_times;
	Cardinal		icons_number;
	unsigned char		*icons;
#endif	/* prototype */
    {
    DwcDayItem		new;
    Cardinal		size;
    
    new  = (DwcDayItem) XtMalloc (sizeof (DwcDayItemRecord));

    new->item_id       = 0;
    new->start_time    = 0;
    new->duration      = 0;
    new->repeat_p1     = DWC$k_db_none;
    new->repeat_p2     = 0;
    new->repeat_p3     = 0;
    new->alarms_number = alarms_number;
    new->icons_number  = icons_number;
    new->last_day      = 0;
    new->last_time     = 0;
    new->entry         = NULL;
    new->flags         = 0;
    new->memexized     = FALSE;

    if (new->alarms_number == 0)
    {
	new->alarms_times = NULL;
    }
    else
    {
	size = sizeof (unsigned short int) * new->alarms_number;
	new->alarms_times = (unsigned short int *) XtMalloc (size);
	memcpy (new->alarms_times, alarms_times, size);
    }

    if (new->icons_number == 0)
    {
	new->icons = NULL;
    }
    else
    {
	size = sizeof (unsigned char) * new->icons_number;
	new->icons = (unsigned char *) XtMalloc (size);
	memcpy (new->icons, icons, size);
    }

    return (new);
    
}

DwcDayItem DAYCloneDayItem
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	DwcDayItem	old)
#else	/* no prototypes */
	(cd, old)
	CalendarDisplay	cd;
	DwcDayItem	old;
#endif	/* prototype */
    {
    DwcDayItem	    new;
    Cardinal	    size;
    
    new  = (DwcDayItem) XtMalloc (sizeof (DwcDayItemRecord));

    memcpy (new, old, sizeof (DwcDayItemRecord));

    if (new->alarms_number != 0) {
	size = sizeof (unsigned short int) * new->alarms_number;
	new->alarms_times = (unsigned short int *) XtMalloc (size);
	memcpy (new->alarms_times, old->alarms_times, size);
    }

    if (new->icons_number != 0) {
	size = sizeof (unsigned char) * new->icons_number;
	new->icons = (unsigned char *) XtMalloc (size);
	memcpy (new->icons, old->icons, size);
    }

    return (new);
    
}

void DAYDestroyDayItem
#if defined(_DWC_PROTO_)
	(
	DwcDayItem	di)
#else	/* no prototypes */
	(di)
	DwcDayItem	di;
#endif	/* prototype */
{

    if (di != NULL)
    {
	XtFree ((char *)di->alarms_times);

	if (di->icons != NULL)
	    XtFree ((char *)di->icons);

	XtFree ((char *)di);
    }
    
}

DayItemUpdate DAYCreateDayItemUpdateRecord
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay		cd,
	Cardinal		dsbot,
	XmString		text,
	DwcDayItem		old,
	DwcDayItem		new,
	RepeatChangeKind	kind)
#else	/* no prototypes */
	(cd, dsbot, text, old, new, kind)
	CalendarDisplay		cd;
	Cardinal		dsbot;
	XmString		text;
	DwcDayItem		old;
	DwcDayItem		new;
	RepeatChangeKind	kind;
#endif	/* prototype */
{
    DayItemUpdate	diu;

    diu = (DayItemUpdate) XtMalloc (sizeof (DayItemUpdateRecord));

    diu->cd      = cd;
    diu->se	 = NULL;
    diu->dsbot   = dsbot;
    diu->text    = text;
    diu->old     = old;
    diu->new     = new;
    diu->kind    = kind;
    diu->changed = FALSE;

    return (diu);
    
}

void DAYDestroyDayItemUpdate
#if defined(_DWC_PROTO_)
	(
	DayItemUpdate	diu)
#else	/* no prototypes */
	(diu)
	DayItemUpdate	diu;
#endif	/* prototype */
{

    if (diu != NULL)
    {
	DAYDestroyDayItem (diu->old);
	DAYDestroyDayItem (diu->new);
	if (diu->text != NULL)
	    XmStringFree (diu->text);
    }

}

void DAYSetupDayNameWidget
#if defined(_DWC_PROTO_)
	(
	Widget		lb,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year)
#else	/* no prototypes */
	(lb, day, month, year)
	Widget		lb;
	Cardinal	day;
	Cardinal	month;
	Cardinal	year;
#endif	/* prototype */
{
    dtb		    date_time;
    XmString	    text;


    date_time.weekday = DATEFUNCDayOfWeek (day, month, year);
    date_time.day     = day;
    date_time.month   = month;
    date_time.year    = year;

    text = DATEFORMATTimeToCS (dwc_k_day_dayname_format, &date_time);

    XtVaSetValues
    (
	lb,
	XmNlabelString, text,
	NULL
    );

    XmStringFree (text);

}

void DAYClearAllItems
#if defined(_DWC_PROTO_)
	(
	DwcDaySlots	ds)
#else	/* no prototypes */
	(ds)
	DwcDaySlots	ds;
#endif	/* prototype */
{
    Cardinal	    i;

    DSWClearAllEntries((DayslotsWidget) (ds->widget));

    for (i = 0;  i < ds->number_of_items;  i++)
    {
	if (ds->items [i] != NULL)
	{
	    DAYDestroyDayItem (ds->items [i]);
	}
    }

    XtFree ((char *)ds->items);
    ds->items = NULL;
    ds->number_of_items = 0;

}

void DAYGetDayItems
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year)
#else	/* no prototypes */
	(cd, day, month, year)
	CalendarDisplay	cd;
	Cardinal	day;
	Cardinal	month;
	Cardinal	year;
#endif	/* prototype */
{
    Cardinal		dsbot;
    int			repeat_start_day;
    int			repeat_start_min;
    int			repeat_p1;
    int			repeat_p2;
    int			repeat_p3;
    int			end_day;
    int			end_min;
    int			item_id;
    int			start_time;
    int			duration;
    int			duration_days;
    int			alarms_number;
    unsigned short int	*alarms_times;
    int			alarm_time;
    int			was_repeated;
    XmString		text;
    XmString		slot_text;
    Boolean		free_text;
    int			text_length;
    int			text_class;
    int			slot;
    int			size;
    Cardinal		i;
    int			status;
    int			entry_flags;
    Boolean		editable;
    DwcDayItem		di;
    int			seitem;
    unsigned char	*icons;
    Cardinal		icons_number;
    DwcDaySlots		ds;
        
    /*
    **  Free up existing items
    */
    
    SEUnlinkAllItems (cd);

    DAYClearAllItems(&cd->dayslots);
    DAYClearAllItems(&cd->daynotes);


    dsbot = DATEFUNCDaysSinceBeginOfTime (day, month, year);

    /*	  
    **  Let's get the items that exist on this day from the db
    */	  
    while
    (
	status = DWCDB_GetRItem
	(
	    cd->cab,
	    dsbot,
	    &item_id,
	    &repeat_start_day,
	    &repeat_start_min,
	    &start_time,
	    &duration_days,
	    &duration, 
	    &alarms_number,
	    &alarms_times,
	    &entry_flags,
	    (unsigned char **)&text,
	    &text_class,
	    &icons_number,
	    &icons,
	    &repeat_p1,
	    &repeat_p2,
	    &repeat_p3,
	    &end_day,
	    &end_min
	) == DWC$k_db_normal
    )
    {

	di = (DwcDayItem) XtMalloc (sizeof (DwcDayItemRecord));

	if (icons != NULL)
	{
	    di->icons = (unsigned char *)XtMalloc (icons_number);
	    memcpy (di->icons, icons, icons_number);
	    di->icons_number = icons_number;
	}
	else
	{
	    di->icons = NULL;
	    di->icons_number = 0;
	}

	/*
	**  MEMEX: If the duration is 1 and the start_time is 0 then ignore
	**  this entry because it is a fake one. It was created when the user
	**  created the first entry to avoid recycling item_id's. This is a real
	**  kludge and we need to come up with a better way of holding a "day"
	**  so that we can find it again given the info we've stored in MEMEX.
	**  If the user has an actual entry with a start_time of 0 and a
	**  duration of 1 then this stuff will make it invisible, unfortunately.
	*/
	if (duration == 0)
	{
	    ds = &(cd->daynotes);
	}
	else
	    if (duration == 1 && start_time == 0)
	    {
                /*	  
                **  MEMEX "hold the day around" entry
                */	  
                continue;
	    }
	    else
	    {
		ds = &(cd->dayslots);
	    }
	
	slot = ds->number_of_items;
	ds->number_of_items++;
	ds->items = (DwcDayItem *) XtRealloc
	    ((char *)ds->items, sizeof (DwcDayItem) * ds->number_of_items);

	ds->items [slot] = di;

	di->item_id    = item_id;
	di->start_time = start_time;
	di->duration   = duration;
	di->repeat_p1  = repeat_p1;
	di->repeat_p2  = repeat_p2;
	di->repeat_p3  = repeat_p3;
	di->memexized  = FALSE;

	/*
	**  Converted databases often use DWC$k_db_absolute for repeats; these
	**  repeats are converted in the UI to DWC$k_db_abscond. The conversion
	**  will only be saved to the disk if the user changes the entry.
	*/
	if (di->repeat_p1 == DWC$k_db_absolute)
	{
	    di->repeat_p1 = DWC$k_db_abscond;
	    di->repeat_p2 = di->repeat_p2 / 1440;
	    di->repeat_p3 = DWC$k_day_default | DWC$m_cond_none;
	}
	di->last_day   = end_day;
	di->last_time  = end_min;
	di->flags      = entry_flags;

	di->alarms_number = alarms_number;
	if (alarms_number == 0)
	{
	    di->alarms_times = NULL;
	}
	else
	{
	    di->alarms_times = (unsigned short int *)
		  XtMalloc (sizeof (unsigned short int) * alarms_number);
	    for (i = 0;  i < alarms_number;  i++)
	    {
		di->alarms_times [i] = alarms_times [i];
	    }
	}

        /*	  
	**  Unpack the text field, to separate the text and the icons.
	*/	  

	slot_text = NULL;
	seitem = SEFindItem
	(
	    cd,
	    dsbot,
	    item_id,
	    NULL,
	    FALSE,
	    &start_time,
	    &duration,
	    &slot_text
	);

	if (slot_text == NULL)
	{
	    slot_text = text;
	}

	DAYGetIconsForDayItem (di, &icons, &icons_number);
	editable = ((seitem < 0) && (! cd->read_only));

	di->entry = DSWAddEntryCS
	(
	    (DayslotsWidget) (ds->widget),
	    start_time,
	    duration,
	    slot_text,
	    (unsigned char *)icons,
	    icons_number,
	    editable,
	    (caddr_t) di
	);

	XtFree ((char *)icons);
	XmStringFree (text);
	
	if (seitem >= 0)
	{
	    SELinkItemToEntry (cd, seitem, di->entry);
	}	    

    }
    
    i = 0;
    while (TRUE)
    {
	seitem = SEGetNewItems (cd, dsbot, i, &di, &slot_text);
	if (seitem < 0)
	{
	    break;
	}

	if (di->duration == 0)
	{
	    ds = &(cd->daynotes);
	}
	else
	{
	    ds = &(cd->dayslots);
	}

	DAYGetIconsForDayItem (di, &icons, &icons_number);
	di->entry = DSWAddEntryCS
	(
	    (DayslotsWidget) (ds->widget),
	    di->start_time,
	    di->duration,
	    slot_text,
	    (unsigned char *)icons,
	    icons_number,
	    FALSE,
	    NULL
	);

	SELinkItemToEntry (cd, seitem, di->entry);
	DAYDestroyDayItem (di);
	XtFree ((char *)icons);
	i = seitem + 1;
    }
    
    DSWUpdateDisplay ((DayslotsWidget) (cd->dayslots.widget));
    DSWUpdateDisplay ((DayslotsWidget) (cd->daynotes.widget));

    if (status == DWC$k_db_failure)
    {
	ERRORDisplayErrno (cd->mainwid, "ErrorGetDayItems");
    }

}

void DAYConfigureDayDisplay
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {
    int			value, shown, inc, pageinc;
    Dimension		lw_width;
    Dimension		lw_height;

    Dimension		lb_width;
    Dimension		lb_height;
    Position		lb_y;
    Dimension		lb_highlight;

    Dimension		mw_height;
    Dimension		mw_width;
    Dimension		dn_height;
    Dimension		dn_width;
    Dimension		ds_height;
    Dimension		ds_width;

    Dimension		sw_height;
    Dimension		sw_width;
    Dimension		sw_highlight;

    Dimension		tb_height;
    Dimension		sb_height;
    Dimension		ww_height;
    Dimension		tb_width;
    Position		tb_space;
    Position		x_scrolledwindow;
    Position		x_scrollbar;
    Position		x_dayslotswidget;
    Position		x_timebar;
    int 		slots;
    Position		next_x;
    Position		next_y;
    Dimension		tbw_v_margin;
    Dimension		shadow_thickness;
    Dimension		sw_shadow_thickness;
    Dimension		fm_shadow_thickness;
    Dimension		lw_shadow_thickness;
    Dimension		margin = 14;

    Dimension		h_margin;
    Dimension		v_margin;
    Dimension		h_spacing;
    Dimension		v_spacing;

    Dimension		sep_width;
    Dimension		lab_width;

    XmOffsetPtr		lw_o = LwOffsetPtr(cd->lw_day_display);

#define DwcRealHeight(w) (XtHeight(w)+(2*XtBorderWidth(w)))
#define DwcRealWidth(w) (XtWidth(w)+(2*XtBorderWidth(w)))
#define DwcVertIncr(w) (Position)(DwcRealHeight(w) + v_spacing)
#define DwcHoriIncr(w) (Position)(DwcRealWidth(w) + h_spacing)

    /*	  
    **  Figure out how much space we have to fit everything in.
    */
    lw_shadow_thickness = MGR_ShadowThickness(cd->lw_day_display);
    lw_height = XtHeight(cd->lw_day_display);
    lw_width = XtWidth(cd->lw_day_display);
    v_margin = XmField(cd->lw_day_display, lw_o, Layout, margin_height, Dimension);
    h_margin = XmField(cd->lw_day_display, lw_o, Layout, margin_width, Dimension);

    next_y = (Position)lw_shadow_thickness;
    v_spacing = (v_margin + 1) / 2;

    if (cd->profile.day_show_months)
    {
        /*	  
        **  Get the height of a single row of months plus the scrollbar and
	**  margins.
	*/
	mw_height = YEARSingleRowHeight (cd->day_yd);
	mw_width  = MAX (1, lw_width - (lw_shadow_thickness * 2));
	XtConfigureWidget
	(
	    (Widget)cd->day_yd->layout,
	    (Position)lw_shadow_thickness, next_y,
	    mw_width, mw_height,
	    XtBorderWidth(cd->day_yd->layout)
	);
	XtManageChild ((Widget)cd->day_yd->layout);
	/*	  
	**  Keep track of where we are as we fill up the container
	*/	  
	next_y += (Position)(mw_height);
    }
    else
    {
	XtUnmanageChild ((Widget)cd->day_yd->layout);
    }

    /*
    ** Separator between months and current day stuff.
    */
    sep_width = MAX (1, lw_width - (2 * lw_shadow_thickness));
    XtConfigureWidget
    (
	cd->day_sep_1,
	lw_shadow_thickness,
	next_y,
	sep_width,
	XtHeight(cd->day_sep_1),
	XtBorderWidth(cd->day_sep_1)
    );
    next_y += DwcVertIncr(cd->day_sep_1);

    /*
    **  Let's display the date, time and year label
    **
    **  We want to center the fulldate label in the daydisplay.
    */
    lab_width = MAX(1, lw_width - (2 * (lw_shadow_thickness + h_margin)));
    XtConfigureWidget
    (
	cd->day_label,
	lw_shadow_thickness + h_margin,
	next_y,
	lab_width,
	XtHeight(cd->day_label),
	XtBorderWidth(cd->day_label)
    );
    next_y += DwcVertIncr(cd->day_label);

    /*
    **  Display the daynotes. (Remember it is inside a frame!)
    */
    if (cd->profile.day_show_notes)
    {
        /*	  
	**  We want to center the daynote in the daydisplay.
	*/
	dn_width = MAX (1, lw_width - (h_margin * 2));
        /*	  
	**  Move the parent frame.
	*/	  
	XtConfigureWidget
	(
	    XtParent(cd->daynotes.widget),
	    (Position)h_margin,
	    next_y,
	    dn_width,
	    XtHeight(XtParent(cd->daynotes.widget)),
	    XtBorderWidth(XtParent(cd->daynotes.widget))
	);
	XtManageChild(cd->daynotes.widget);
	XtManageChild(XtParent(cd->daynotes.widget));

	next_y +=
	    DwcVertIncr(XtParent(cd->daynotes.widget)) +
	    (Position)lw_shadow_thickness;	/* improve vertical centering */
						/* to the daynote and dayslot */
    }
    else
    {
	if (XtIsManaged(XtParent(cd->daynotes.widget)))
	{
	    XtUnmanageChild(XtParent(cd->daynotes.widget));
	}
    }

    /*
    ** Position 2nd separator.
    */
    XtConfigureWidget
    (
	cd->day_sep_2,
	lw_shadow_thickness,
	next_y,
	sep_width,
	XtHeight(cd->day_sep_2),
	XtBorderWidth(cd->day_sep_1)
    );
    next_y += DwcVertIncr(cd->day_sep_2);

    /*
    ** Now, let's jump to the bottom and do the time label/pushbutton and
    ** the next/previous day buttons.  This will determine how much space
    ** is left for the day slots.
    */
    lb_height = DwcRealHeight(cd->pb_day_last);
    lb_width = DwcRealWidth(cd->pb_day_time);
    lb_y = (Position)(lw_height - v_margin - lb_height);
    lb_highlight =
	((XmPrimitiveWidget)(cd->pb_day_next))->primitive.highlight_thickness;

    XtMoveWidget
    (
	cd->pb_day_time,
	(Position)((lw_width - lb_width) / 2),
	lb_y
    );
    XtMoveWidget
    (
	cd->pb_day_last,
	(Position)h_margin - lb_highlight,
	lb_y
    );

    XtMoveWidget
    (
	cd->pb_day_next,
	(Position)(lw_width -
	    h_margin - DwcRealWidth(cd->pb_day_next) + lb_highlight),  
	lb_y
    );

    /*	  
    **  The scrollwindow height (the container for the dayslotswidget), is what
    **	we have left after we take the total, and subtract the pushbutton height
    **	at the bottom and a margin.
    */
    sw_height = lb_y - (Dimension)next_y - v_spacing;
    sw_height = MAX(1, sw_height);
    sw_height = MIN((Dimension)XtHeight(cd->dayslots.widget), sw_height);
    /*	  
    **  Fit at least one dayslot in there.
    */	  
    sw_height = MAX(
	sw_height,
	DSWGetDayslotsHeight ((DayslotsWidget) (cd->dayslots.widget)) + 1
    );

    XtUnmanageChild (XtParent(cd->day_wrkhrsbar));

    h_spacing = h_margin / 3;
    sw_highlight =
	((XmPrimitiveWidget)(cd->day_scrollbar))->primitive.highlight_thickness;
    sw_shadow_thickness = MGR_ShadowThickness(XtParent(cd->lw_day_scrollwin));

    if (cd->profile.day_timebar_width == 0)
    {
	tb_width = 0;
	tb_space = -((Position)sw_highlight);
    }
    else
    {
	tb_width = DwcRealWidth(XtParent(cd->day_wrkhrsbar));
	tb_space = tb_width + h_spacing;
    }

    if (cd->profile.directionRtoL)
    {
	/*
	**  We're right to left so allow room on the left for the timebar
	*/
	x_timebar = (Position)h_margin;
	x_scrollbar = x_timebar + tb_space;
        x_scrolledwindow = x_scrollbar + DwcHoriIncr(cd->day_scrollbar);
	sw_width = lw_width - x_scrolledwindow - h_margin;
    }
    else
    {
	x_timebar = lw_width - tb_width - h_margin;
	x_scrollbar =
	    lw_width - h_margin - tb_space - DwcRealWidth(cd->day_scrollbar);
	x_scrolledwindow = h_margin;
	sw_width = x_scrollbar - x_scrolledwindow - h_spacing;
    }

    sw_width = MAX(
	(2 * (sw_shadow_thickness + XtBorderWidth(cd->lw_day_scrollwin))) + 1,
	sw_width
    );

    x_dayslotswidget = sw_shadow_thickness;
    ds_width =
	sw_width -
	(2 * (sw_shadow_thickness + XtBorderWidth(cd->lw_day_scrollwin)));
    ds_width = MAX(1, ds_width);

    XmScrollBarGetValues (cd->day_scrollbar, &value, &shown, &inc, &pageinc);

    XtConfigureWidget
    (
	XtParent(cd->lw_day_scrollwin),
	x_scrolledwindow,
	next_y + sw_highlight,
	sw_width,
	MAX(1,sw_height - (2 * sw_highlight)),
	XtBorderWidth(cd->lw_day_scrollwin)
    );

    XtConfigureWidget
    (
	(Widget)cd->lw_day_scrollwin,
	sw_shadow_thickness,
	sw_shadow_thickness,
	ds_width,
	MAX(1, sw_height - (2 * (sw_highlight + sw_shadow_thickness))),
	XtBorderWidth(cd->lw_day_scrollwin)
    );

    XtConfigureWidget
    (
	cd->day_scrollbar,
	x_scrollbar,
	next_y,
	XtWidth (cd->day_scrollbar),
	MAX(1, sw_height),
	XtBorderWidth(cd->day_scrollbar)
    );

    XtConfigureWidget
    (
	cd->dayslots.widget,
	0,
	XtY(cd->dayslots.widget),
	ds_width,
	(Dimension)XtHeight(cd->dayslots.widget),
	XtBorderWidth(cd->dayslots.widget)
    );

    DSWMoveDayslotsToTime ((DayslotsWidget) (cd->dayslots.widget), value);

    if (cd->profile.day_timebar_width != 0)
    {
	tbw_v_margin = sw_highlight;

	fm_shadow_thickness = MGR_ShadowThickness(XtParent(cd->day_wrkhrsbar));
	tb_height = sw_height - (fm_shadow_thickness * 2);

	XtResizeWidget
	(
	    cd->day_wrkhrsbar,
	    XtWidth(cd->day_wrkhrsbar),
	    MAX(1, tb_height - (2 * tbw_v_margin)),
	    XtBorderWidth(cd->day_wrkhrsbar)
	);

	XtConfigureWidget
	(
	    XtParent(cd->day_wrkhrsbar),
	    x_timebar,
	    next_y + tbw_v_margin,
	    XtWidth(XtParent(cd->day_wrkhrsbar)),
	    MAX(1, sw_height - (2 * tbw_v_margin)),
	    XtBorderWidth(XtParent(cd->day_wrkhrsbar))
	);

	XtManageChild (cd->day_wrkhrsbar);
	XtManageChild (XtParent(cd->day_wrkhrsbar));
    }


    DSWDoScrollCallback ((DayslotsWidget) (cd->dayslots.widget));

}

static void DAY_resize_cb
#if defined(_DWC_PROTO_)
	(
	Widget	lw,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(lw, tag, cbs)
	Widget	lw;
	caddr_t	tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
    {
    CalendarDisplay	cd = (CalendarDisplay) tag;
    Dimension		mainwid_width,
			mainwid_height;
    
    DAYConfigureDayDisplay (cd);

    XtVaGetValues
    (
	cd->mainwid,
	XmNheight, &mainwid_height,
	XmNwidth,  &mainwid_width,
	NULL
    );

/* debugging info to catch layout resize notification problem
    printf("DAY_resize_cb width: %d height: %d\n",
	mainwid_width,mainwid_height);
*/
    /*	  
    **  Convert from pixels to our internal format
    */	  
    cd->profile.day_width  =
	    (Cardinal)((mainwid_width * 100 ) / cd->screen_font_size);
    cd->profile.day_height =
	    (Cardinal)((mainwid_height * 100 ) / cd->screen_font_size);

}

static void DAY_dispose_focus_cb
#if defined(_DWC_PROTO_)
	(
	Widget	dsw,
	caddr_t	tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(lw, tag, cbs)
	Widget	lw;
	caddr_t	tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay	cd = (CalendarDisplay) tag;

    /*
    ** Force focus to standard place on closing.
    */
    DAYTraverseToClose (cd);
}

static void DAY_focus_cb
#if defined(_DWC_PROTO_)
	(
	Widget			lw,
	caddr_t			tag,
	LwCallbackStruct	*cbs)
#else	/* no prototypes */
	(lw, tag, cbs)
	Widget			lw;
	caddr_t			tag;
	LwCallbackStruct	*cbs;
#endif	/* prototype */
{
    Time		time = cbs->time;
    CalendarDisplay	cd = (CalendarDisplay) tag;

    CLIPMainwidOwnPRIMARY (cd, time);

    /*
    ** What this really means is check if it has an entry.
    */
    if (XtCallAcceptFocus (cd->dayslots.widget, &time))
    {
	LwGotFocus (lw);
    }
    else if (XtCallAcceptFocus (cd->daynotes.widget, &time))
    {
	LwGotFocus (lw);
    }
    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */
}

static void DAY_next_day_cb
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
    Cardinal		day;
    Cardinal		month;
    Cardinal		year;
    Cardinal		dsbot;
    CalendarDisplay	cd = (CalendarDisplay) tag;

    dsbot = DATEFUNCDaysSinceBeginOfTime (cd->day, cd->month, cd->year) + 1;
    DATEFUNCDateForDayNumber (dsbot, (int *)&day, (int *)&month, (int *)&year);

    MISCChangeView(cd, show_day, day, month, year);

}

static void DAY_prev_day_cb
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
    Cardinal		day;
    Cardinal		month;
    Cardinal		year;
    Cardinal		dsbot;
    CalendarDisplay	cd = (CalendarDisplay) tag;

    dsbot = DATEFUNCDaysSinceBeginOfTime (cd->day, cd->month, cd->year) - 1;
    DATEFUNCDateForDayNumber (dsbot, (int *)&day, (int *)&month, (int *)&year);

    MISCChangeView(cd, show_day, day, month, year);

}

void DAYResetTimeslotEntry
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	DwcDaySlots	ds,
	Cardinal	dsbot,
	DwcDswEntry	entry,
	Time		time)
#else	/* no prototypes */
	(cd, ds, dsbot, entry, time)
	CalendarDisplay	cd;
	DwcDaySlots	ds;
	Cardinal	dsbot;
	DwcDswEntry	entry;
	Time		time;
#endif	/* prototype */
    {
    int			    repeat_start_day;
    int			    repeat_start_min;
    int			    repeat_p1;
    int			    repeat_p2;
    int			    repeat_p3;
    int			    start_time;
    int			    duration;
    int			    duration_days;
    int			    alarms_number;
    unsigned short int	    *alarms_times;
    XmString		    entry_text;
    int			    text_length;
    int			    text_class;
    int			    last_day;
    int			    last_time;
    int			    status;
    int			    entry_flags;
    XmString		    slot_text;
    Cardinal		    num_icons;
    unsigned char	    *entry_icons;
    DSWEntryDataStruct   data;
    DwcDayItem		    di;

    DSWGetEntryData (entry, &data);
    if (data.tag == NULL)
    {
	DAYRemoveEntry (cd, ds, entry, time);
	return;
    }
    else
    {
	di = (DwcDayItem) data.tag;
	if (di->item_id == 0)
	{
	    DAYRemoveEntry (cd, ds, entry, time);
	    return;
	}
    }

    status = DWCDB_GetSpecificRItem
    (
	cd->cab,
	dsbot,
	di->item_id,
	&repeat_start_day,
	&repeat_start_min,
	&start_time,
	&duration_days,
	&duration, 
	&alarms_number,
	&alarms_times,
	&entry_flags,
	&entry_text,
	&text_class,
	&num_icons,
	&entry_icons,
	&repeat_p1,
	&repeat_p2,
	&repeat_p3,
	&last_day,
	&last_time
    );

    if (status != DWC$k_db_normal)
    {
	ERRORDisplayError (cd->mainwid, "ErrorDatabase");
	return;
    }

    slot_text = entry_text;
    DSWSetEntryCSText (entry, slot_text);

    DSWSetEntryTimes (entry, start_time, duration);
    DSWCloseEntry    ((DayslotsWidget) (ds->widget), entry, time);
    DSWSetEntryEditable (entry, ! cd->read_only, time);

    /*
    ** Tell the creator to put focus somewhere else.
    */
    DAYTraverseToClose (cd);
}

void DAYDO_ENTRY_EDIT
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
/*
**++
**  Functional Description:
**	DAYDO_ENTRY_EDIT
**	This routine is called when the user invokes by the 'Timeslot Edit'
**	menu item: an entry is selected in the day view, and the sloteditor
**	dialog box is brought up to edit that entry.
**
**  Keywords:
**	None.
**
**  Arguments:
**	Standard widget arguments.
**
**  Result:
**	None.
**
**  Exceptions:
**	None.
**--
*/
{
    DSWEntryDataStruct   data;
    DwcDswEntry		    entry;
    CalendarDisplay	    cd;
    int			    status;
    Time		    time;
    DwcDaySlots		    ds;
    WhichEditor		    editor;
    
    /*
    **	Looking for the Calendar Display from which the 'Timeslot Edit' menu
    **	item was selected.
    */    
    status = MISCFindCalendarDisplay (&cd, w);

    ds = &(cd->dayslots);
    entry = DSWGetOpenEntry ((DayslotsWidget) (ds->widget));

    if (entry == NULL)
	{
	/*
	**  The currently open entry isn't a regular timeslot entry. Has to
	**  be a day note entry.
	*/	
	ds	= &(cd->daynotes);
	entry	= DSWGetOpenEntry ((DayslotsWidget) (ds->widget));
	if (entry == NULL)
	    {
	    /*
	    **  We should NEVER come down here
	    */
	    return;
	    }
	else
	    {
	    editor = DaynoteEditor;	    
	    }
	}
    else
	{
	editor = SlotEditor;
	}

    time = MISCGetTimeFromAnyCBS (cbs);

    DSWGetEntryData (entry, &data);
    DSWCloseEntry ((DayslotsWidget) (ds->widget), entry, time);
    DSWSetEntryEditable (entry, FALSE, time);

    SECreateSlotEditor (cd, editor, ds, entry, data.text_changed);

    /*
    **  That's it
    */
    return;
} 

static void DAY_entry_delete_cb
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
    Time		    time;
    DayItemUpdate	    diu = (DayItemUpdate)tag;

    time = MISCGetTimeFromAnyCBS (cbs);

    if (cbs->reason == (int)XmCR_OK) (void) DAYDoDayItemUpdate (diu, time);

    DAYDestroyDayItemUpdate (diu);

    return;
}

void DAYDO_ENTRY_DELETE
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
    DwcDswEntry			entry;
    DSWEntryDataStruct	data;
    DwcDaySlots			ds;
    DwcDayItem			old;
    DayItemUpdate		diu;
    CalendarDisplay		cd;
    int				status;

    status = MISCFindCalendarDisplay (&cd, w);

    ds    = &(cd->dayslots);


    /*	  
    **  You can only delete an open entry, so if there are no open entries then
    **	don't delete.
    */	  
    entry = DSWGetOpenEntry((DayslotsWidget) (ds->widget));
    if (entry == NULL)
	{
	ds    = &(cd->daynotes);
	entry = DSWGetOpenEntry ((DayslotsWidget) (ds->widget));
	if (entry == NULL)
	    {
	    return; /* ???? Uh??? */
	    }
	}

    DSWGetEntryData(entry, &data);
    old = (DwcDayItem)data.tag;

    if (old == NULL)
	{
        /*	  
	**  If there isn't an old entry (ie this is a new one) then create a
	**  dummy one so that other routines are happy.
	*/	  
        old = DAYCreateDayItemRecord( 0, NULL, 0, NULL );
	old->entry = entry;
	}
    else
	{
	old = DAYCloneDayItem (cd, old);
	}

    diu = DAYCreateDayItemUpdateRecord (cd, cd->dsbot, NULL, old, NULL, RCKNotRepeat);
    diu->slots = ds;

    /*	  
    **  Depending upon whether it is a repeat entry or not, ask the right
    **	questions.
    */	  
    if (old->repeat_p1 == DWC$k_db_none)
	{
	ERRORReportError
	(
	    cd->mainwid,
	    "CautionDelete",
	    (XtCallbackProc)DAY_entry_delete_cb,
	    (caddr_t)diu
	);
	}
    else
	{
	SEPoseRepeatingEntryQuestions (cd, dwc_k_pose_delete_text, diu);
	}

}

void DAYDO_ENTRY_CLOSE
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
    Time			time;
    DwcDswEntry			entry;
    DSWEntryDataStruct	data;
    CalendarDisplay		cd;
    int				status;

    status = MISCFindCalendarDisplay (&cd, w);

    entry = DSWGetOpenEntry ((DayslotsWidget) (cd->dayslots.widget));
    if (entry == NULL)
    {
	entry = DSWGetOpenEntry ((DayslotsWidget) (cd->daynotes.widget));
	if (entry == NULL)
	{
            /*	  
	    **  If nothing is open then we can't close it. We could put an error
	    **	message here I suppose.
	    */	  
            return;
	}
    }

    DSWGetEntryData (entry, &data);
    time = MISCGetTimeFromAnyCBS (cbs);
    (void) DSWRequestCloseOpenEntry ((DayslotsWidget) (data.parent), time);

    /*
    ** Force focus to standard place on closing.
    */
    DAYTraverseToClose (cd);
}

void DAYDO_ENTRY_RESET
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
    CalendarDisplay	    cd;
    DwcDswEntry		    entry;
    Time		    time;
    int			    status;

    status = MISCFindCalendarDisplay (&cd, w);

    time  = MISCGetTimeFromAnyCBS (cbs);
    entry = DSWGetOpenEntry ((DayslotsWidget) (cd->dayslots.widget));
    if (entry != NULL)
    {
	DAYResetTimeslotEntry (cd, &(cd->dayslots), cd->dsbot, entry, time);
    }
    else
    {
	entry = DSWGetOpenEntry ((DayslotsWidget) (cd->daynotes.widget));
	if (entry != NULL)
	{
	    DAYResetTimeslotEntry (cd, &(cd->daynotes), cd->dsbot, entry, time);
	}
    }
   
}

void DAYDO_ENTRY_MENU
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
    Arg			arglist [1];
    CalendarDisplay	cd;
    int			status;
    Boolean		choice = FALSE;    
    Boolean		repeat = FALSE;
    Boolean		changed = FALSE;
    DwcDswEntry		entry;
    DwcDayItem		old;
    DSWEntryDataStruct data;
    Boolean		empty_entry = FALSE;

    if (cbs->reason != (int)XmCR_MAP)
    {
	return;
    }
    
    status = MISCFindCalendarDisplay (&cd, w);

    if (cd->showing == show_day)
    {
	entry = DSWGetOpenEntry ((DayslotsWidget) (cd->dayslots.widget));
	if (entry == NULL)
	    entry = DSWGetOpenEntry ((DayslotsWidget) (cd->daynotes.widget));
	choice = (entry != NULL);
		 
        /*	  
	**  Make sure we actually found an entry or daynote open
	*/	  
        if (choice)
	{
            /*	  
	    **  See if this is a repeat entry
	    */	  
            repeat = MISCTestRepeatOpen(cd);

	    DSWGetEntryData( entry, &data );
	    empty_entry = (data.tag == NULL);
	    old = (DwcDayItem)data.tag;
	    if (old != NULL)
		if ((data.text_changed) ||
		    (data.start != old->start_time) ||
		    (data.duration != old->duration))
			changed = TRUE;
	}
    }

    XtSetSensitive (cd->pb_entry_menu,  choice);
    XtSetSensitive (cd->pb_reset_menu,  choice);
    XtSetSensitive (cd->pb_delete_menu, choice);
    XtSetSensitive (cd->pb_close_menu,  choice);

    if (repeat && changed && choice)
    {
	XtSetArg (arglist [0], XmNlabelString, "k_close_rep_entry_label_text");
    }
    else
    {
	XtSetArg (arglist [0], XmNlabelString, "k_close_entry_label_text");
    }

    status = MrmFetchSetValues
	(cd->ads->hierarchy, cd->pb_close_menu, arglist, 1);
    if (status != MrmSUCCESS)
    {
	DWC$UI_Catchall (DWC$DRM_PBCLOSE, status, 0);
	return;
    }

}

static void DAY_move_day_scrollbar_cb
#if defined(_DWC_PROTO_)
	(
	Widget				w,
	caddr_t				tag,
	XmScrollBarCallbackStruct	*scroll)
#else	/* no prototypes */
	(w, tag, scroll)
	Widget				w;
	caddr_t				tag;
	XmScrollBarCallbackStruct	*scroll;
#endif	/* prototype */
    {
    Position			y_pos;
    CalendarDisplay		cd = (CalendarDisplay) tag;
    DayslotsWidget		dsw = (DayslotsWidget) cd->dayslots.widget;
    caddr_t			pass_tag;
    int				value;
        
    /*	  
    **  If we're still dragging and we don't have 'live' scrolling turned on
    **	then ignore this.
    */	  
    if ((scroll->reason == (int)XmCR_DRAG) &&
	(! cd->profile.direct_scroll_coupling))
    {
	return;
    }

    if (scroll->reason == (int)XmCR_HELP)
    {
	pass_tag = (caddr_t)MISCFetchDRMValue (dwc_k_help_day_scroll);
	HELPForWidget (w, pass_tag, NULL);
	return;
    }

    /*	  
    **  Make the dayslotswidget position itself correctly.
    */	  
    DSWMoveDayslotsToTime (dsw, scroll->value);

    /*	  
    **  We're not on the current time so turn off autoscroll (ie turn on the
    **	shadows around the time pushbutton.
    */	  
    MISCSetAutoScroll (cd, FALSE);


}

static void DAY_position_day_scrollbar_cb
#if defined(_DWC_PROTO_)
	(
	Widget				w,
	caddr_t				tag,
	DSWScrollCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget				w;
	caddr_t				tag;
	DSWScrollCallbackStruct	*cbs;
#endif	/* prototype */
    {
    CalendarDisplay		 cd  = (CalendarDisplay) tag;
    Widget			 scr = cd->day_scrollbar;
    
    if ((cbs->min_value != cd->day_scroll_min) ||
        (cbs->max_value != cd->day_scroll_max))
    {
	XtVaSetValues
	(
	    scr,
	    XmNminimum, cbs->min_value,
	    XmNmaximum, cbs->max_value,
	    NULL
	);

	cd->day_scroll_min = cbs->min_value;
        cd->day_scroll_max = cbs->max_value;
    }
    
    XmScrollBarSetValues
	(scr, cbs->value, cbs->shown, cbs->inc, cbs->pageinc, FALSE);

    MISCSetAutoScroll (cd, FALSE);

}

static void DAY_reget_flags_for_item
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	DwcDayItem	di,
	Cardinal	dsbot)
#else	/* no prototypes */
	(cd, di, dsbot)
	CalendarDisplay	cd;
	DwcDayItem	di;
	Cardinal	dsbot;
#endif	/* prototype */
{
    int				repeat_start_day;
    int				repeat_start_min;
    int				repeat_p1;
    int				repeat_p2;
    int				repeat_p3;
    int				start_time;
    int				duration;
    int				duration_days;
    int				alarms_number;
    unsigned short int		*alarms_times;
    XmString			entry_text;
    int				text_class;
    int				last_day;
    int				last_time;
    int				status;
    int				entry_flags;
    unsigned char		*icons;
    Cardinal			num_icons;

    status = DWCDB_GetSpecificRItem
    (
	cd->cab,
	dsbot,
	di->item_id,
	&repeat_start_day,
	&repeat_start_min,
	&start_time,
	&duration_days,
	&duration, 
	&alarms_number,
	&alarms_times,
	&di->flags,
	&entry_text,
	&text_class,
	&num_icons,
	&icons,
	&repeat_p1,
	&repeat_p2,
	&repeat_p3,
	&last_day,
	&last_time
    );

}

static int DAY_put_day_item
#if defined(_DWC_PROTO_)
	(
	struct DWC$db_access_block	*cab,
	Cardinal			dsbot,
	DwcDayItem			di,
	XmString			text)
#else	/* no prototypes */
	(cab, dsbot, di, text)
	struct DWC$db_access_block	*cab;
	Cardinal			dsbot;
	DwcDayItem			di;
	XmString			text;
#endif	/* prototype */
{
    int				status;
    int				fake_item_id;
    DwcDayItem			dummy_memex_holder;

    /*	  
    **  Create our special text field (with icons and text together)
    */
    status = DWCDB_PutRItem
    (
	cab,
	&di->item_id,
	dsbot,
	di->start_time,
	0,
	di->duration,
	di->alarms_number,
	di->alarms_times,
	di->flags,
	text,
	DWC$k_item_cstr,
	di->icons_number,
	di->icons,
	di->repeat_p1,
	di->repeat_p2,
	di->repeat_p3,
	di->last_day,
	di->last_time
    );

    if (di->repeat_p1 != DWC$k_db_none)
    {
	di->flags = di->flags | DWC$k_item_first;
    }
    
    /*
    **	MEMEX: If this is the first entry ever created in this day, create a
    **	fake one with a duration of 1 and a start_time of 0. The user will
    **	never see this one and item_id's will never be recycled for this day
    **	even if the user deletes all the entries for this day. This is a big
    **	HACK since it means that the db will continue to grow, grow, grow and it
    **	completely defeats the idea of days not staying around if they aren't
    **	needed, but it will do for now.
    */

    if ((di->item_id == 1) || (di->item_id == -1))
    {
	static XmString    memex_text = NULL;

	if (memex_text == NULL)
	{
	    memex_text = XmStringCreateSimple ("MEMEX holder entry");
	}

	dummy_memex_holder = DAYCreateDayItemRecord (0, NULL, 0, NULL);
        /*	  
	**  Create our special text field (with icons and text together)
	*/

	status = DWCDB_PutRItem
	(
	    cab,
	    &fake_item_id,
	    dsbot,
	    0,
	    0,
	    1,
	    0,
	    NULL,
	    DWC$k_use_normal,
	    memex_text,
	    DWC$k_item_cstr,
	    dummy_memex_holder->icons_number,
	    dummy_memex_holder->icons,
	    DWC$k_db_none,
	    0,
	    0,
	    0,
	    0
	);
    
	XtFree((char *)dummy_memex_holder);
    }
    
    return (status);
    
}

static int DAY_delete_day_item
#if defined(_DWC_PROTO_)
	(
	struct DWC$db_access_block	*cab,
	Cardinal			dsbot,
	DwcDayItem			di,
	RepeatChangeKind		kind)
#else	/* no prototypes */
	(cab, dsbot, di, kind)
	struct DWC$db_access_block	*cab;
	Cardinal			dsbot;
	DwcDayItem			di;
	RepeatChangeKind		kind;
#endif	/* prototype */
{
    int			repeat_start_day;
    int			repeat_start_min;
    int			repeat_p1;
    int			repeat_p2;
    int			repeat_p3;
    int			start_time;
    int			duration;
    int			duration_days;
    int			alarms_number;
    unsigned short int	*alarms_times;
    XmString		entry_text;
    int			text_length;
    int			text_class;
    int			last_day;
    int			last_time;
    int			status;
    int			entry_flags;
    Cardinal		icon_length;
    unsigned char	*entry_icons;

    if (kind != RCKThisAndFuture)
    {
	if ((kind == RCKAllInstances) || (di->item_id > 0))
	{
            /*	  
	    **  If, we've been told to delete all instances or if the item is
	    **	not a repeat item (ie a > 0 id), then delete all instances.
	    */	  
	    status = DWC$DB_Delete_r_item (cab, di->item_id, dsbot, 1);
	    di->item_id = 0;
	}
	else
	{
	    /* only delete instance at start day */
	    status = DWC$DB_Delete_r_item (cab, di->item_id, dsbot, 0);
	}
	return (status);
    }
    else
    {
        /*	  
	**  kind is RCKThisAndFuture. Get info about this repeat item
	*/
	status = DWCDB_GetSpecificRItem
	(
	    cab,
	    dsbot,
	    di->item_id,
	    &repeat_start_day,
	    &repeat_start_min,
	    &start_time,
	    &duration_days,
	    &duration, 
	    &alarms_number,
	    &alarms_times,
	    &entry_flags,
	    &entry_text,
	    &text_class,
	    &icon_length,
	    &entry_icons,
	    &repeat_p1,
	    &repeat_p2,
	    &repeat_p3,
	    &last_day,
	    &last_time
	);	  
        if (status != DWC$k_db_normal)
	{
	    return (status);
	}
        /*	  
	**  By modifying this repeat item to end one minute before this day
	**  we're effectively deleting all future instances of this repeat
	*/
	status = DWCDB_ModifyRItem
	(
	    cab,
	    di->item_id,
	    dsbot,
	    start_time,
	    0,
	    0,
	    duration_days,
	    duration,
	    alarms_number,
	    alarms_times,
	    entry_flags,
	    entry_text,
	    text_class,
	    icon_length,
	    entry_icons,
	    repeat_p1,
	    repeat_p2,
	    repeat_p3,
	    dsbot - 1,
	    (24 * 60) - 1
	);

        return (status);

    }

}

static int DAY_change_day_item
#if defined(_DWC_PROTO_)
	(
	struct DWC$db_access_block	*cab,
	Cardinal			dsbot,
	DwcDayItem			old,
	DwcDayItem			new,
	XmString			text,
	RepeatChangeKind		kind)
#else	/* no prototypes */
	(cab, dsbot, old, new, text, kind)
	struct DWC$db_access_block	*cab;
	Cardinal			dsbot;
	DwcDayItem			old;
	DwcDayItem			new;
	XmString			text;
	RepeatChangeKind		kind;
#endif	/* prototype */
{
    int				status;

    if ((kind == RCKNotRepeat) || (kind == RCKAllInstances))
    {
	/*	  
	**  Create our special text field (with icons and text together)
	*/
	status = DWCDB_ModifyRItem
	(
	    cab,
	    new->item_id,
	    dsbot,
	    new->start_time,
	    0,
	    0,
	    0,
	    new->duration,
	    new->alarms_number,
	    new->alarms_times,
	    new->flags,
	    (unsigned char *)text,
	    DWC$k_item_cstr,
	    new->icons_number,
	    new->icons,
	    new->repeat_p1,
	    new->repeat_p2,
	    new->repeat_p3,
	    new->last_day,
	    new->last_time
	);
	return (status);
    }
    
    if ((kind == RCKThisInstance) ||
        ((kind == RCKRepeatChange) && (old->repeat_p1 == DWC$k_db_none)))
    {
	status = DAY_delete_day_item (cab, dsbot, old, RCKThisInstance);
	if (status != DWC$k_db_normal) return (status);

	if (kind == RCKThisInstance)
	{
	    new->repeat_p1 = DWC$k_db_none;
	    new->repeat_p2 = 0;
	    new->repeat_p3 = 0;
	}

	return (DAY_put_day_item (cab, dsbot, new, text));
    }

    if ((kind == RCKRepeatChange) && (new->repeat_p1 == DWC$k_db_none))
    {
	new->repeat_p1	= old->repeat_p1;
	new->repeat_p2	= old->repeat_p2;
	new->repeat_p3	= old->repeat_p3;

        /*	  
	**  Create our special text field (with icons and text together)
	*/	  
	status = DWCDB_ModifyRItem
	(
	    cab,
	    new->item_id,
	    dsbot,
	    new->start_time,
	    0,
	    0,
	    0,
	    new->duration,
	    new->alarms_number,
	    new->alarms_times,
	    new->flags, 
	    text,
	    DWC$k_item_cstr,
	    new->icons_number,
	    new->icons,
	    new->repeat_p1,
	    new->repeat_p2,
	    new->repeat_p3,
	    dsbot,
	    (24 * 60) - 1
	);
	new->flags = (new->flags & ~DWC$m_item_rpos) | DWC$k_item_last;
	return (status);
    }

    status = DAY_delete_day_item (cab, dsbot, old, RCKThisAndFuture);
    if (status != DWC$k_db_normal) return (status);

    if (kind == RCKRepeatChange)
    {
	new->last_day  = 0;
	new->last_time = 0;
    }

    return (DAY_put_day_item (cab, dsbot, new, text));

}

void DAYRemoveEntry
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	DwcDaySlots	ds,
	DwcDswEntry	entry,
	Time		time)
#else	/* no prototypes */
	(cd, ds, entry, time)
	CalendarDisplay	cd;
	DwcDaySlots	ds;
	DwcDswEntry	entry;
	Time		time;
#endif	/* prototype */
    {
    DSWEntryDataStruct   data;
    Cardinal		    i;
    Cardinal		    j;

    if (entry != NULL)
    {
	CLIPSMRemoveSelected(cd, entry, time);
	DSWGetEntryData(entry, &data);
	DSWDeleteEntry ((DayslotsWidget) (ds->widget), entry);
	if (data.tag != NULL)
	{
	    for (i = 0;  i < ds->number_of_items;  i++)
	    {
		if (ds->items [i] == (DwcDayItem)data.tag)
		{
		    DAYDestroyDayItem(ds->items [i]);
		    for (j = i + 1;  j < ds->number_of_items;  j++)
		    {
			ds->items [j - 1] = ds->items [j];
		    }
		    ds->number_of_items--;
		    break;
		}
	    }
	}
    }
}

void DAYUpdateMonthMarkup
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	Cardinal	first,
	Cardinal	last,
	Boolean		all)
#else	/* no prototypes */
	(cd, first, last, all)
	CalendarDisplay	cd;
	Cardinal	first;
	Cardinal	last;
	Boolean		all;
#endif	/* prototype */
{
    Cardinal		i;
    Cardinal		day;
    Cardinal		month;
    Cardinal		year;
    unsigned char	*entry_type;
    unsigned char	*marks;
    unsigned char	*junk;
    unsigned char	new_markup;
    unsigned char	old_markup;
    unsigned char	style;


    if (all)
    {
	MWRegetAllDayMarkup (cd->month_display);
    }
    else
    {
	i = last - first + 1;
	entry_type = (unsigned char *) XtMalloc (sizeof (unsigned char) * i);
	marks      = (unsigned char *) XtMalloc (sizeof (unsigned char) * i);
	junk       = (unsigned char *) XtMalloc (sizeof (unsigned char) * i);

	if (DWC$DB_Get_day_attr_rng
		(cd->cab, first, last, marks, junk, entry_type) !=
	    DWC$k_db_normal)
	{
	    ERRORDisplayError (cd->mainwid, "ErrorDayAttr");
	    return;
	}

	for (i = 0;  i < last - first + 1;  i++)
	{
	    DATEFUNCDateForDayNumber
		(first + i, (int *)&day, (int *)&month, (int *)&year);

	    MWGetDayStyleMarkup(cd->month_display, day, month, year,
					  &old_markup, &style);


	    if ((marks [i] & DWC$m_day_special) != 0)
	    {
		new_markup = MWMARKUP_SPECIAL;
	    }
	    else
	    {
		marks [i] = marks [i] & ~DWC$m_day_defaulted;
		if (marks [i] == DWC$k_day_workday)
		{
		    new_markup = MWMARKUP_WORK_DAY;
		}
		else
		{
		    new_markup = MWMARKUP_NON_WORK_DAY;
		}
	    }
	    
	    if (entry_type [i] == DWC$k_use_significant)
	    {
		new_markup = new_markup | MWMARKUP_ENTRY;
	    }

	    if (new_markup != old_markup)
	    {
		MWSetDayStyleMarkup (cd->month_display, day, month,
					      year, new_markup, style);
	    }
	}

	XtFree ((char *)entry_type);
	XtFree ((char *)marks);
	XtFree ((char *)junk);
    }

}

static Boolean DAY_do_day_item_update_action
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd,
	DayItemUpdate	diu,
	ddiu_action	action,
	Boolean		repeat_changed,
	Time		time)
#else	/* no prototypes */
	(cd, diu, action, repeat_changed, time)
	CalendarDisplay	cd;
	DayItemUpdate	diu;
	ddiu_action	action;
	Boolean		repeat_changed;
	Time		time;
#endif	/* prototype */
    {
    DSWEntryDataStruct   data;
    Cardinal		    slot;
    int			    status;
    int			    trgsts;
    DwcDayItem		    di;
    ddea_action		    ddea;    
    unsigned char	    *icons;
    Cardinal		    num_icons;

    DwcDayItem		    old = diu->old;
    DwcDayItem		    new = diu->new;
    DwcDaySlots		    ds  = diu->slots;


    switch (action)
    {
    case DdiuRemove:
	DAYRemoveEntry (cd, ds, new->entry, time);
	return (TRUE);

    case DdiuAdd:
	status = DAY_put_day_item (cd->cab, diu->dsbot, new, diu->text);
	if (status != DWC$k_db_normal) 	break;

	ddea = DdeaAdd;
	if (new->item_id > 0)
	{
	    if (cd->dsbot != diu->dsbot)
	    {
		return (TRUE);
	    }
	}
	else
	{
	    trgsts = DWC$DB_Check_trigger (cd->cab, new->item_id, cd->dsbot);
	    if (trgsts != DWC$k_db_triggers)
	    {
		if (trgsts == DWC$k_db_notrigger)
		{
		    ddea = DdeaRemove;
		    old = new;
		}
		else
		{
		    status = trgsts;
		}
	    }
	}
	break;	

      case DdiuDelete :
	status = DAY_delete_day_item (cd->cab, diu->dsbot, old, diu->kind);
	if (status != DWC$k_db_normal) 	break;

	ddea = DdeaRemove;
	if (old->item_id < 0)
	{
	    trgsts = DWC$DB_Check_trigger (cd->cab, old->item_id, cd->dsbot);
	    if (trgsts != DWC$k_db_notrigger)
	    {
		if (trgsts == DWC$k_db_triggers)
		{
		    ddea = DdeaClose;
		}
		else
		{
		    status = trgsts;
		}
	    }
	}
	break;	

      case DdiuChange :
	status = DAY_change_day_item
	    (cd->cab, diu->dsbot, old, new, diu->text, diu->kind);
	if (status != DWC$k_db_normal) 	break;

	ddea = DdeaChange;
	if (new->item_id >= 0)
	{
	    if (cd->dsbot == diu->dsbot)
	    {
		break;
	    }
	}
	else
	{
	    ddea = DdeaClose;
	    trgsts = DWC$DB_Check_trigger (cd->cab, new->item_id, cd->dsbot);
	    if (trgsts == DWC$k_db_triggers)
	    {
		if (new->entry == NULL)
		{
		    ddea = DdeaAdd;
		}
		else
		{
		    ddea = DdeaChange;
		}
		break;
	    }
	    else
	    {
		if (trgsts != DWC$k_db_notrigger)
		{
		    status = trgsts;
		    break;
		}
	    }
	}

	ddea = DdeaRemove;
	if (old->item_id < 0)
	{
	    trgsts = DWC$DB_Check_trigger (cd->cab, old->item_id, cd->dsbot);
	    if (trgsts != DWC$k_db_notrigger)
	    {
		if (trgsts == DWC$k_db_triggers)
		{
		    ddea = DdeaClose;
		}
		else
		{
		    status = trgsts;
		}
	    }
	}
	else
	{
	    if (old->item_id > 0)
	    {
		ddea = DdeaClose;
	    }
	}

	break;	
	
    }

    if (status == DWC$k_db_dayfull )
    {
	ERRORDisplayError (cd->mainwid, "ErrorDayFull");
	return (FALSE);
    }
    else if (status == DWC$k_db_toobig )
    {
	ERRORDisplayError (cd->mainwid, "ErrorTooBig");
	return (FALSE);
    }
    else if (status != DWC$k_db_normal)
    {
	ERRORDisplayError (cd->mainwid, "ErrorDatabase");
	return (FALSE);
    }

    if (repeat_changed)
    {
	switch (ddea)
	{
	case DdeaAdd :
	case DdeaChange :
	    DAY_reget_flags_for_item(cd, new, cd->dsbot);
	    break;
	    
	case DdeaClose :
	    DAY_reget_flags_for_item(cd, old, cd->dsbot);
	    break;

	case DdeaRemove :
	    break;

	}
    }

    switch (ddea)
    {
    case DdeaAdd :
	slot = ds->number_of_items;
	ds->number_of_items++;
	ds->items = (DwcDayItem *) XtRealloc
	    ((char *)ds->items, sizeof (DwcDayItem) * ds->number_of_items);

	ds->items [slot] = di = DAYCloneDayItem (cd, new);

	DAYGetIconsForDayItem (di, &icons, &num_icons);
	if (di->entry == NULL)
	{
	    di->entry = DSWAddEntryCS
	    (
		(DayslotsWidget) (ds->widget),
		di->start_time,
		di->duration,
		diu->text,
		(unsigned char *)icons,
		num_icons,
		!cd->read_only,
		(caddr_t) di
	    );
	    DSWUpdateDisplay ((DayslotsWidget) (ds->widget));
	}
	else
	{
	    DSWSetEntryIcons
	    (
		(DayslotsWidget) (ds->widget),
		di->entry,
		(unsigned char *)icons,
		num_icons
	    );
	    DSWSetEntryCSText (di->entry, diu->text);
	    DSWSetEntryTag  (di->entry, (caddr_t) di);
	    DSWCloseEntry   ((DayslotsWidget) (ds->widget), di->entry, time);
	    DSWSetEntryEditable (di->entry, ! cd->read_only, time);
	}
	XtFree ((char *)icons);
	break;
	
    case DdeaRemove :
	DAYRemoveEntry (cd, ds, old->entry, time);
	break;

    case DdeaChange :
	DSWGetEntryData (new->entry, &data);
	if (data.tag != NULL)
	{
	    for (slot = 0;  slot < ds->number_of_items;  slot++)
	    {
		if (ds->items [slot] == (DwcDayItem) data.tag)
		{
		    DAYDestroyDayItem (ds->items [slot]);
		    ds->items [slot] = di = DAYCloneDayItem (cd, new);
		    DAYGetIconsForDayItem (di, &icons, &num_icons);

		    DSWSetEntryIcons
		    (
			(DayslotsWidget) (ds->widget),
			di->entry,
			(unsigned char *)icons,
			num_icons
		    );
		    DSWSetEntryCSText  (di->entry, diu->text);
		    DSWSetEntryTag   (di->entry, (caddr_t) di);
		    DSWSetEntryTimes
			(di->entry, new->start_time, new->duration);
		    DSWCloseEntry
			((DayslotsWidget) (ds->widget), di->entry, time);
		    DSWSetEntryEditable (di->entry, !cd->read_only, time);
		    XtFree ((char *)icons);
		    break;
		}
	    }
	}
	break;

    case DdeaClose :
	DAYGetIconsForDayItem (old, &icons, &num_icons);
	DSWSetEntryIcons
	    ((DayslotsWidget) (ds->widget), old->entry, (unsigned char *)icons, num_icons);
	DSWCloseEntry ((DayslotsWidget) (ds->widget), old->entry, time);
	DSWSetEntryEditable (old->entry, ! cd->read_only, time);
	XtFree ((char *)icons);
	break;

    }
    
    return (TRUE);

}

Boolean DAYDoDayItemUpdate
#if defined(_DWC_PROTO_)
	(
	DayItemUpdate	diu,
	Time		time)
#else	/* no prototypes */
	(diu, time)
	DayItemUpdate	diu;
	Time		time;
#endif	/* prototype */
    {
    Boolean		status;
    Boolean		repeat_changed;
    ddiu_action		action;
    
    CalendarDisplay	cd   = diu->cd;
    DwcDayItem		old  = diu->old;
    DwcDayItem		new  = diu->new;


    ICONSWaitCursorDisplay(cd->mainwid, cd->ads->wait_cursor);

    repeat_changed = FALSE;
    if (diu->kind != RCKNotRepeat)
    {
        /*	  
	**  This update record has repeat info
	*/	  
        repeat_changed = TRUE;
    }
    else
    {
        /*	  
	**  This update record doesn't have repeat info
	*/	  
        if (old != NULL)
	{
            /*	  
	    **  Was it previously a repeat?
	    */	  
            repeat_changed = (old->repeat_p1 != DWC$k_db_none);
	}


	if ((! repeat_changed) && (new != NULL))
	{
            repeat_changed = (new->repeat_p1 != DWC$k_db_none);
	}
    }

    if (old == NULL)
    {
	if (XmStringEmpty(diu->text))
	{
	    action = DdiuRemove;
	}
	else
	{
	    action = DdiuAdd;
	}
    }
    else
    {
	if (new == NULL)
	{
	    if (old->item_id == 0)
	    {
		action = DdiuRemove;
		diu->new = diu->old;
		diu->old = NULL;
	    }
	    else
	    {
		action = DdiuDelete;
	    }
	}
	else
	{
	    if (XmStringEmpty(diu->text))
	    {
		action = DdiuDelete;
	    }
	    else
	    {
		action = DdiuChange;
	    }
	}	
    }

    status = DAY_do_day_item_update_action
	(cd, diu, action, repeat_changed, time);

    if (status)
    {
	ALARMSSetupNextAlarm (cd, FALSE);
	DAYUpdateMonthMarkup(cd, diu->dsbot, diu->dsbot, repeat_changed);
    }

    ICONSWaitCursorRemove (cd->mainwid);

    return (status);
    
}

static void DAY_quick_copy_or_move
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay			cd,
	DwcDaySlots			ds,
	DSWEntryCallbackStruct	*cbs,
	Boolean				move)
#else	/* no prototypes */
	(cd, ds, cbs, move)
	CalendarDisplay			cd;
	DwcDaySlots			ds;
	DSWEntryCallbackStruct	*cbs;
	Boolean				move;
#endif	/* prototype */
    {
    Boolean			status;
    DSWEntryDataStruct	data;
    DwcDayItem			old;
    DwcDayItem			new;
    Cardinal			i;
    DayItemUpdate		diu;
    XmString			xm_text;
    int				length;
    ClipContents	    cbc;

    if (CLIPSMTestSelectionOurs (cd))
    {
    
	for (i = 0;  i < cd->number_of_selected_entries;  i++)
	{
	    xm_text = DSWGetEntryCSText (cd->selected_entries [i]);

	    DSWGetEntryData (cd->selected_entries [i], &data);

	    old = (DwcDayItem) data.tag;    
	    if (old == NULL)
	    {
		new = DAYCloneDayItem (cd, ds->default_item);
	    }
	    else
	    {
		new = DAYCloneDayItem (cd, old);
		new->entry = NULL;
		new->repeat_p1 = DWC$k_db_none;
	    }

	    diu = DAYCreateDayItemUpdateRecord
		(cd, cd->dsbot, xm_text, NULL, new, RCKNotRepeat);

	    diu->new->start_time = cbs->start;

	    if (cbs->duration == 0)
	    {
		diu->new->duration = 0;
	    }
	    else
	    {
		if (diu->new->duration == 0)
		{
		    diu->new->duration = cbs->duration;
		}
	    }

	    diu->slots = ds;
	    
	    status = DAYDoDayItemUpdate (diu, cbs->time);

	    DAYDestroyDayItemUpdate (diu);

	    if (!status) return;
	}
    }
    else
    {
	cbc = CLIPSMTestTargets (cd, cbs->time);
	switch (cbc)
	{
	case ClipHasEntry:
	    if (!CLIPSMGetEntriesSelection (cd, cbs->time))
		return;
	    break;
	case ClipHasString:
	case ClipHasCT:
	case ClipHasDDIF:
	    if (!CLIPSMGetComplexStringSelection
		(cd, cbc, &xm_text, (unsigned long *)&length, cbs->time))
	    {
		return;
	    }
	    else
	    {
		if (!CLIPImportInterchange (cd, xm_text, cbs->time))
		{
		    new = DAYCloneDayItem (cd, ds->default_item);
		    diu = DAYCreateDayItemUpdateRecord
			(cd, cd->dsbot, xm_text, NULL, new, RCKNotRepeat);

		    diu->new->start_time = cbs->start;
		    diu->new->duration   = cbs->duration;
		    diu->slots           = ds;

		    status = DAYDoDayItemUpdate (diu, cbs->time);
		    DAYDestroyDayItemUpdate (diu);
		    if (!status) return;
		}
	    }
	    break;
	default:
	    return;
	}
    }

    if (move)
    {
	CLIPSMSendKillSelection (cd, cbs->time);
    }
    
}

static void DAY_do_day_cb
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay			cd,
	DwcDaySlots			ds,
	DSWEntryCallbackStruct	*cbs)
#else	/* no prototypes */
	(cd, ds, cbs)
	CalendarDisplay			cd;
	DwcDaySlots			ds;
	DSWEntryCallbackStruct	*cbs;
#endif	/* prototype */
    {
    Boolean			found;
    Cardinal			i;
    DSWEntryDataStruct	data;
    DwcDayItem			old;
    DwcDayItem			new;
    DayItemUpdate		diu;
    Widget			dsw;
    int				item_id;
    XmString			xm_text;

    switch (cbs->reason)
    {
    case DwcDswCREntryQuickCopy:
	if (! cd->read_only)
	{
	    DAY_quick_copy_or_move (cd, ds, cbs, FALSE);
	}
	break;

    case DwcDswCREntryQuickMove:
	if (! cd->read_only)
	{
	    DAY_quick_copy_or_move (cd, ds, cbs, TRUE);
	}
	break;

    case DwcDswCREntrySelect:
	CLIPMainwidOwnPRIMARY (cd, cbs->time);
        /*	  
	**  Make this one the only selected entry
	*/	  
        CLIPSMSetSelected (cd, cbs->entry, cbs->time);
	break;

    case DwcDswCREntryExtendSelect:
	CLIPMainwidOwnPRIMARY(cd, cbs->time);
	CLIPSMAddSelected (cd, cbs->entry, cbs->time);
	break;

    case DwcDswCREntryDeselect:
	CLIPMainwidOwnPRIMARY(cd, cbs->time);
	found = FALSE;
	for (i = 0;  i < cd->number_of_selected_entries;  i++)
	{
	    if (cbs->entry == cd->selected_entries [i])
	    {
		found = TRUE;
		break;
	    }
	}

        /*	  
	**  If this entry is in the selected list, take it out, otherwise add it
	**  (kind of a toggle action)
	*/	  
        if (found)
	{
            CLIPSMRemoveSelected (cd, cbs->entry, cbs->time);
	}
	else
	{
            CLIPSMAddSelected (cd, cbs->entry, cbs->time);
	}
	break;
	
    case DwcDswCRCompleteOutstanding:
	if (ds == &(cd->dayslots))
	{
	    dsw = cd->daynotes.widget;
	}
	else
	{
	    dsw = cd->dayslots.widget;
	}

	if (! DSWRequestCloseOpenEntry((DayslotsWidget) (dsw), cbs->time))
	{
            /*	  
            **  We weren't able to close so flag that the operation wasn't okay.
            */	  
            DSWCancelOperation ((DayslotsWidget) (ds->widget));
	}
	break;

    case DwcDswCREntryEdit:
	DSWGetEntryData(cbs->entry, &data);
	old = (DwcDayItem) data.tag;    
	if (old == NULL)
	{
	    item_id = 0;
	}
	else
	{
	    item_id = old->item_id;
	}
	
	if (SEFindItem (cd, cd->dsbot, item_id, cbs->entry,
			   TRUE, NULL, NULL, NULL) >= 0)
	{
	    break;
	}

	/* Fall into EditNew... */

    case DwcDswCREntryEditNew:
        /*
        **  Close the entry that the user wants to edit and set it not editable
        */
        DSWCloseEntry ((DayslotsWidget) (ds->widget), cbs->entry, cbs->time);
	DSWSetEntryEditable (cbs->entry, FALSE, cbs->time);

        /*	  
        **  Create the appropriate editor
        */	  
        if (ds == &(cd->daynotes))
	{
	    SECreateSlotEditor
		(cd, DaynoteEditor, ds, cbs->entry, cbs->text_changed);
	}
	else
	{
	    SECreateSlotEditor
		(cd, SlotEditor, ds, cbs->entry, cbs->text_changed);
	}
	break;

    case DwcDswCREntryCreate:
	xm_text = DSWGetEntryCSText (cbs->entry);

	if (XmStringEmpty(xm_text))
	{
	    DAYRemoveEntry(cd, ds, cbs->entry, cbs->time);
	    return;
	}

	new = DAYCloneDayItem (cd, ds->default_item);

	diu = DAYCreateDayItemUpdateRecord
	    (cd, cd->dsbot, xm_text, NULL, new, RCKNotRepeat);

	diu->new->start_time = cbs->start;
	diu->new->duration   = cbs->duration;
	diu->new->entry      = cbs->entry;
	diu->slots	     = ds;
	
	(void) DAYDoDayItemUpdate (diu, cbs->time);

	DAYDestroyDayItemUpdate (diu);

	break;

    case DwcDswCREntryChange:
	old  = DAYCloneDayItem (cd, (DwcDayItem) cbs->tag);

	xm_text = DSWGetEntryCSText (cbs->entry);

	if (XmStringEmpty(xm_text))
	{
	    new = NULL;
	}
	else
	{
	    new = DAYCloneDayItem (cd, old);
	    new->start_time = cbs->start;
	    new->duration   = cbs->duration;
	}

	diu = DAYCreateDayItemUpdateRecord
	    (cd, cd->dsbot, xm_text, old, new, RCKNotRepeat);
	diu->slots = ds;

	if (old->repeat_p1 == DWC$k_db_none)
	{
	    (void) DAYDoDayItemUpdate (diu, cbs->time);

	    DAYDestroyDayItemUpdate (diu);
	}
	else
	{
	    SEPoseRepeatingEntryQuestions (cd, dwc_k_pose_modify_text, diu);
	}

	break;

    }

}

static void DAY_entry_cb
#if defined(_DWC_PROTO_)
	(
	Widget				dsw,
	caddr_t				tag,
	DSWEntryCallbackStruct	*cbs)
#else	/* no prototypes */
	(dsw, tag, cbs)
	Widget				dsw;
	caddr_t				tag;
	DSWEntryCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay		cd = (CalendarDisplay) tag;

    DAY_do_day_cb (cd, &(cd->dayslots), cbs);
    
}

static void DAY_daynote_entry_cb
#if defined(_DWC_PROTO_)
	(
	Widget				dsw,
	caddr_t				tag,
	DSWEntryCallbackStruct	*cbs)
#else	/* no prototypes */
	(dsw, tag, cbs)
	Widget				dsw;
	caddr_t				tag;
	DSWEntryCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay		cd = (CalendarDisplay) tag;

    DAY_do_day_cb (cd, &(cd->daynotes), cbs);

}

void DAYGetIconsFromText
#if defined(_DWC_PROTO_)
	(
	int		text_class,
	char		**text_p,
	int		*length_p,
	Cardinal	*icons_num_p,
	unsigned char	**icons_p)
#else	/* no prototypes */
	(text_class, text_p, length_p, icons_num_p, icons_p)
	int		text_class;
	char		**text_p;
	int		*length_p;
	Cardinal	*icons_num_p;
	unsigned char	**icons_p;
#endif	/* prototype */
    {
    char	    *text;
    int		    text_length;
    Cardinal	    icons_number;
    unsigned char   *icons;
    int		    size, i;

    text = *text_p;
    text_length = *length_p;

	switch (text_class) {
	  case DWC$k_item_text  :
	    icons_number = 1;
	    icons        = (unsigned char *)
	      XtMalloc (sizeof (unsigned char) * icons_number);
	    icons [0]    = k_pixmap_meeting;
	    break;

	  case DWC$k_item_texti :
	    icons_number = text [0];
	    text++;  text_length--;
	    if (icons_number == 0) {
		icons = NULL;
	    } else {
		size = (sizeof (unsigned char) * icons_number);
		icons = (unsigned char *) XtMalloc (size);
		memcpy (icons, text, size);
		text = text + size;
		text_length = text_length - size;

		for (i = 0;  i < icons_number;  i++) {
		    if ((icons [i] < k_pixmap_setable_start) ||
			(icons [i] > k_pixmap_setable_end)) {
			icons [i] = k_pixmap_question;
		    }
		}
	    }
	    break;

	}

    *length_p = text_length;
    *text_p = text;
    *icons_p = icons;
    *icons_num_p = icons_number;

    return;
}

Boolean DAYTraverseToOpen
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay			cd
	)
#else
	(cd)
	CalendarDisplay			cd;
#endif
{
    DwcDswEntry		entry;

    if (DSWTraverseToOpen ((DayslotsWidget) (cd->dayslots.widget)))
	return (True);

    return (DSWTraverseToOpen ((DayslotsWidget) (cd->daynotes.widget)));
}

Boolean DAYTraverseToClose
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay			cd
	)
#else
	(cd)
	CalendarDisplay			cd;
#endif
{
    return (XmProcessTraversal ((Widget)cd->pb_day_time, XmTRAVERSE_CURRENT));
}

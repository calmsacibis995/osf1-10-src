/* dwc_ui_calendar.c */
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
**	Marios Cleovoulou   November-1987
**	Ken Cowan	    February-1989
**	Denis G. Lacroix    February-1989
**
**  ABSTRACT:
**
**	This is the main program for the DECWindows Calendar.
**
**--
*/

#include    "dwc_compat.h"

#include <time.h>
#include <assert.h>
#include <stdio.h>
#if defined(VMS)
#include <descrip.h>
#endif

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Intrinsic.h>		/* for stuff like Boolean */
#include <Xm/Xm.h>
#include <X11/Xproto.h>			/* for X_SetInputFocus */
#include <X11/cursorfont.h>		/* for various cursors */
#include <Xm/XmP.h>			/* for XmVoidProc, among other things */
#include <Xm/Label.h>			/* for XmCreatLabel... */
#include <Xm/ScrollBar.h>		/* for XmCreateScrollBar */
#include <Xm/PushB.h>			/* for XmCreatePushButton */
#include <Xm/ArrowB.h>			/* for XmCreateArrowButton */
#include <Xm/Separator.h>		/* for XmCreateSeparator */
#include <Xm/Protocols.h>		/* for XmAddWmProtocolCallback */
#include <Xm/AtomMgr.h>			/* for XmInternAtom */
#include <Xm/Frame.h>
#include <X11/DECwI18n.h>
#include <DXm/DECspecific.h>
#include <dwi18n_lib.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* for DWC$m_item_insignif */

#include "dwc_ui_calendar.h"
#include "dwc_ui_dateformat.h"   	/* for dtb */
#include "dwc_ui_help.h"	  	/* for HELPForWidget */
#include "dwc_ui_catchall.h"		/* for DWC$DRM code */
#include "dwc_ui_timebarwidget.h"	/* for DwcTbwNhorizontalMargin */
#include "dwc_ui_iconboxwidget.h"	/* DwcIbwIconboxCreate declaration */
#include "dwc_ui_clipboard.h"		/* CLIPSMReceiveClientMessage declra */
#include "dwc_ui_misc.h"  		/* MISCGenericCreateproc declaration */
#include "dwc_ui_file.h"	  	/* do_file_open,etc. declaration */
#include "dwc_ui_edit.h"	  	/* do_edit_menu declaration */
#include "dwc_ui_view.h"	  	/* do_view_menu declaration */
#include "dwc_ui_day.h"			/* DAYDO_?_menu declaration */
#include "dwc_ui_options.h"		/* custom_menu declaration */
#include "dwc_ui_custom.h"		/* do_general_display declar */
#include "dwc_ui_profile.h"		/* *settings declarations */
#include "dwc_ui_errorboxes.h"   	/* ERRORDestroyErrorBox declaration */
#include "dwc_ui_layout.h"	    	/* for LwLayoutCreate */
#include "dwc_ui_month.h"		/* for MONTHCreateMonthDisplay */
#include "dwc_ui_alarms.h"		/* for ALARMSClockTimerTick */
#if MEMEX
#include "dwc_ui_memex.h"		/* for MEMEXCreateDwUi */
#endif
#include "dwc_ui_icons.h"


/*
**  Global Data Definitions ( used to be globaldefs )
*/
AllDisplaysRecord ads;
XmString    *time_texts_24;
XmString    *time_texts_12;
XmString    *minute_texts;

/*
**  External Data Declarations
*/
extern Pixmap	small_pixmaps[];	/* defined in dwc_ui_misc.c */
extern Pixmap	medium_pixmaps[];	/* defined in dwc_ui_misc.c */
extern Pixmap	big_pixmaps[];		/* defined in dwc_ui_misc.c */

extern XtCallbackRec HELPwidget_help_cb[];	/* defined in dwc_ui_help.c */
extern XtCallbackRec DAYday_entry_cb[];
extern XtCallbackRec DAYday_scroll_cb[];
extern XtCallbackRec DAYdns_entry_cb[];
extern XtCallbackRec DAYfocus_day_cb[];
extern XtCallbackRec DAYpb_last_day_cb[];
extern XtCallbackRec DAYpb_next_day_cb[];
extern XtCallbackRec DAYposition_scrollbar_cb[];
extern XtCallbackRec DAYresize_day_cb[];
extern XtCallbackRec DAYdisp_focus_cb[];
extern XtCallbackRec MONTHmonth_help_cb[];
extern XtCallbackRec MONTHmonth_dc_cb[];
	

Atom	DwcIconStateAtom;
Atom	DwcWmChangeStateAtom;

static Boolean	pixmap_loaded = FALSE;	/* indicates if we've loaded the pixmaps */
static XtAppContext app_context;    /* Application context from XtAppInitialize */

static XtCallbackRec day_clock_cb [2] =
{
    {(XtCallbackProc) MISCDayClock,	NULL},
    {NULL,				NULL}
};

static void create_day_pbs PROTOTYPE ((CalendarDisplay cd));

static void create_daydisplay PROTOTYPE ((CalendarDisplay cd));
	
static void create_daynote PROTOTYPE ((CalendarDisplay cd));

static void create_dayslotswidget PROTOTYPE ((CalendarDisplay cd));

static void create_fulldate_lb PROTOTYPE ((CalendarDisplay cd));

char *db_filespec PROTOTYPE (());

static int ErrorHandler PROTOTYPE (());

static void get_timetext PROTOTYPE ((
	Widget				w,
	caddr_t				tag,
	DSWGetTextCallbackStruct	*cbs));


static void init_resources PROTOTYPE ((CalendarDisplay cd));
	
static XtCallbackRec get_timetext_cb [2] =
{
    {(XtCallbackProc) get_timetext,	NULL},
    {NULL,				NULL}
};

static void register_names PROTOTYPE ((void));

static void set_mainwid_widthheight PROTOTYPE ((CalendarDisplay cd));
	
void FILE_PRINT_MENU PROTOTYPE ((
	Widget			widget,
	int			*tag,
	XmAnyCallbackStruct	*callback_data));

Cursor ICONSInactiveCursorCreate PROTOTYPE ((Widget w));


Cursor ICONSWaitCursorCreate PROTOTYPE ((Widget w));

void MONTHGetMonthMarkup PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	month,
	Cardinal	year,
	Cardinal	days,
	unsigned char	markup[],
	unsigned char	style[]));


static XtResource DwcApplResources [] =
{
   {DwcNiconDayFontList,	XmCFontList,		XmRFontList,
    sizeof (XmFontList),	XtOffset (CalendarDisplay, icon_day_fontlist),
    XmRString,			DXmDefaultFont},

   {DwcNiconMonthFontList,	XmCFontList,		XmRFontList,
    sizeof (XmFontList),	XtOffset (CalendarDisplay, icon_month_fontlist),
    XmRString,			DXmDefaultFont},

   {DwcNtitleDayFontList,	XmCFontList,		XmRFontList,
    sizeof(XmFontList),		XtOffset (CalendarDisplay, title_day_fontlist),
    XmRString,			DXmDefaultFont}

};

/*** ??????? Take these out for the real thing -- here just so we can set   */
/* breakpoints ???????? */

static void OurWarning
#if defined(_DWC_PROTO_)
	(
	char	*text)
#else	/* no prototypes */
	(text)
	char	*text;
#endif	/* prototype */
{
    printf ("%s\n",text);
}   

static void OurError
#if defined(_DWC_PROTO_)
	(
	char	*text)
#else	/* no prototypes */
	(text)
	char	*text;
#endif	/* prototype */
{
    printf (text);
    printf ("\nFatal X Toolkit error\n");
    exit
    (
#if defined(VMS)
    1
#else
    0
#endif
    );
}   

static void init_time_texts
#if defined(_DWC_PROTO_)
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{

    time_texts_24 = (XmString *) XtCalloc ((24 * 60) + 1, sizeof (char *));
    time_texts_12 = (XmString *) XtCalloc ((24 * 60) + 1, sizeof (char *));
    minute_texts  = (XmString *) XtCalloc (60, sizeof (char *));

}

static void get_timetext
#if defined(_DWC_PROTO_)
	(
	Widget				w,
	caddr_t				tag,
	DSWGetTextCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget				w;
	caddr_t				tag;
	DSWGetTextCallbackStruct	*cbs;
#endif	/* prototype */
{
    dtb		    date_time;

    if (cbs->times == minute_texts)
    {
	date_time.minute = cbs->index;
	minute_texts [cbs->index] =
	    DATEFORMATTimeToCS (dwc_k_timeslots_mins_format, &date_time);
	return;
    }

    date_time.hour   = cbs->index / 60;
    date_time.minute = cbs->index % 60;

    if (cbs->times == time_texts_24)
    {
	time_texts_24[cbs->index] =
	    DATEFORMATTimeToCS (dwc_k_timeslots_24hr_format, &date_time);
	return;
    }

    if (cbs->times == time_texts_12)
    {
	time_texts_12[cbs->index] =
	    DATEFORMATTimeToCS (dwc_k_timeslots_12hr_format, &date_time);
	return;
    }

}

typedef struct
{
    XmString	*times;
    Cardinal	index;
    Cardinal	increment;
} get_timetext_rec, *get_timetext_ptr;
    
Boolean get_timetext_wp
#if defined(_DWC_PROTO_)
	(
	caddr_t	tag)
#else	/* no prototypes */
	(tag)
	caddr_t	tag;
#endif	/* prototype */
{
    DSWGetTextCallbackStruct	cbs;
    get_timetext_ptr  ptr = (get_timetext_ptr) tag;

    while (ptr->index <= ((24 * 60) + 1))
    {
	if (ptr->times [ptr->index] == NULL)
	{
	    cbs.times = ptr->times;
	    cbs.index = ptr->index;
	    get_timetext (NULL, NULL, &cbs);
	    ptr->index = ptr->index + ptr->increment;
	    return (FALSE);
	}
	ptr->index = ptr->index + ptr->increment;
    }

    XtFree ((char *)ptr);
    return (TRUE);
    
}

void CALStartTimeTextWorkProc
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    get_timetext_ptr	GTTPtr;
    XtWorkProcId	workprocid;
    
    GTTPtr = (get_timetext_ptr) XtMalloc (sizeof (get_timetext_rec));

    if (cd->profile.time_am_pm)
    {
	GTTPtr->times = time_texts_12;
    }
    else
    {
	GTTPtr->times = time_texts_24;
    }
    GTTPtr->index = 0;
    GTTPtr->increment = cd->profile.fine_increment;

    workprocid = XtAppAddWorkProc
	(CALGetAppContext(), (XtWorkProc)get_timetext_wp, (Opaque)GTTPtr);

}

static void CALReIcon
#if defined(_DWC_PROTO_)
    (
    Widget	w,
    caddr_t	cd_in,
    XEvent	*event
    )
#else
    (w, cd_in, event)
    Widget	w;
    caddr_t	cd_in;
    XEvent	*event;
#endif
{
    CalendarDisplay	cd = (CalendarDisplay) cd_in;
    dtb			date_time;
    struct tm		*local_time;

    if (event->type == ReparentNotify)
    {
	MISCGetTime(&local_time);

	date_time.day   = local_time->tm_mday;
	date_time.month = local_time->tm_mon;

	/*
	** Do the check and make it stick!
	*/
	if (MISCIsXUIWMRunning(cd->toplevel, True))
	    ICONSSetIconifyIcon (cd->toplevel, 0);

	ICONSSetIconBoxIcon
	(
	    cd->toplevel,
	    NULL,
	    &date_time,
	    cd->icon_day_fontlist,
	    cd->icon_month_fontlist,
	    False,
	    True,
	    cd->mapped
	);
    }
}

static void CALMap
#if defined(_DWC_PROTO_)
    (
    Widget	w,
    caddr_t	cd_in,
    XEvent	*event
    )
#else
    (w, cd_in, event)
    Widget	w;
    caddr_t	cd_in;
    XEvent	*event;
#endif
{
    CalendarDisplay	cd = (CalendarDisplay) cd_in;
    dtb			date_time;
    struct tm		*local_time;

    if (event->type == MapNotify)
    {
	cd->mapped = True;
	XtRemoveEventHandler
	(
	    cd->toplevel,
	    StructureNotifyMask,
	    False,
	    (XtEventHandler)CALMap,
	    (caddr_t)cd
	);
	XtAddEventHandler
	(
	    cd->toplevel,
	    StructureNotifyMask,
	    False,
	    (XtEventHandler)CALReIcon,
	    (caddr_t)cd
	);
    }
}

static void CALTitle
#if defined(_DWC_PROTO_)
    (
    Widget	w,
    caddr_t	cd_in,
    XEvent	*event
    )
#else
    (w, cd_in, event)
    Widget	w;
    caddr_t	cd_in;
    XEvent	*event;
#endif
{
    CalendarDisplay	cd = (CalendarDisplay) cd_in;
    dtb			date_time;
    struct tm		*local_time;
    char		applname[256];
    static const unsigned long	mask = KeyPressMask | ButtonPressMask;
    XmString		cs;
    int			i;
    int			appllen;
    long		byte_count, cvt_status;
    XmString		xm_applname;

    cs = MISCFetchDRMValue (dwc_k_calendar_title_text);	    /* don't free CS */
    cd->appl_title = MISCGetTextFromCS(cs);

    appllen = strlen(cd->appl_title);
    memcpy (applname, cd->appl_title, appllen);
    if (cd->profile.icon_show_text)
    {
	strcpy (&applname[appllen], cd->profile.icon_text);
    }
    else
    {
	strcpy (&applname[appllen], cd->filename);
    }

    xm_applname = DXmCvtFCtoCS(applname, &byte_count, &cvt_status);
    DWI18n_SetTitle(cd->toplevel, xm_applname);
    XmStringFree(xm_applname);

    /*
    ** BLOW 'EM ALL AWAY!!!!
    */
    /*
    ** Menubar.
    */
    XtRemoveEventHandler
    (
	cd->menubar,
	mask,
	False,
	(XtEventHandler)CALTitle,
	(caddr_t)cd
    );

    if (cd->profile.first_display == show_month)
    {
	/*
	** Month widget and scrollbar in month view.
	*/
	XtRemoveEventHandler
	(
	    cd->month_display,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (XtEventHandler)(caddr_t)cd
	);
	XtRemoveEventHandler
	(
	    cd->month_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
    }
    else if (cd->profile.first_display == show_day)
    {
	/*
	** Pushbuttons, dayslots, daynotes and months in day view.
	*/
	XtRemoveEventHandler
	(
	    cd->pb_day_next,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtRemoveEventHandler
	(
	    cd->pb_day_last,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtRemoveEventHandler
	(
	    cd->pb_day_time,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtRemoveEventHandler
	(
	    cd->dayslots.widget,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtRemoveEventHandler
	(
	    cd->daynotes.widget,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtRemoveEventHandler
	(
	    cd->day_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);

	XtRemoveEventHandler
	(
	    cd->day_yd->horiz_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	for (i = 0; i < 12; i++)
	{
	    XtRemoveEventHandler
	    (
		cd->day_yd->months[i],
		mask,
		False,
		(XtEventHandler)CALTitle,
		(caddr_t)cd
	    );
	}
    }
    else if (cd->profile.first_display == show_year)
    {
	/*
	** Months in yearview.
	*/
	XtRemoveEventHandler
	(
	    cd->yd->horiz_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtRemoveEventHandler
	(
	    cd->yd->vert_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	for (i = 0; i < 12; i++)
	{
	    XtRemoveEventHandler
	    (
		cd->yd->months[i],
		mask,
		False,
		(XtEventHandler)CALTitle,
		(caddr_t)cd
	    );
	}
    }
}

void CALAddCopyrightHandlers
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    static const unsigned long		mask = KeyPressMask | ButtonPressMask;
    int					i;

    /*
    ** Menubar.
    */
    XtAddEventHandler
    (
	cd->menubar,
	mask,
	False,
	(XtEventHandler)CALTitle,
	(caddr_t)cd
    );

    if (cd->profile.first_display == show_month)
    {
	/*
	** Month widget and scrollbar in month view.
	*/
	XtAddEventHandler
	(
	    cd->month_display,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtAddEventHandler
	(
	    cd->month_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
    }
    else if (cd->profile.first_display == show_day)
    {
	/*
	** Pushbuttons, dayslots, daynotes and months in day view.
	*/
	XtAddEventHandler
	(
	    cd->pb_day_next,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtAddEventHandler
	(
	    cd->pb_day_last,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtAddEventHandler
	(
	    cd->pb_day_time,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtAddEventHandler
	(
	    cd->dayslots.widget,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtAddEventHandler
	(
	    cd->daynotes.widget,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtAddEventHandler
	(
	    cd->day_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);

	XtAddEventHandler
	(
	    cd->day_yd->horiz_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	for (i = 0; i < 12; i++)
	{
	    XtAddEventHandler
	    (
		cd->day_yd->months[i],
		mask,
		False,
		(XtEventHandler)CALTitle,
		(caddr_t)cd
	    );
	}
    }
    else if (cd->profile.first_display == show_year)
    {
	/*
	** Months in yearview.
	*/
	XtAddEventHandler
	(
	    cd->yd->horiz_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	XtAddEventHandler
	(
	    cd->yd->vert_scrollbar,
	    mask,
	    False,
	    (XtEventHandler)CALTitle,
	    (caddr_t)cd
	);
	for (i = 0; i < 12; i++)
	{
	    XtAddEventHandler
	    (
		cd->yd->months[i],
		mask,
		False,
		(XtEventHandler)CALTitle,
		(caddr_t)cd
	    );
	}
    }
}

void CALStartupCalendar
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    dtb			date_time;
    struct tm		*local_time;
    char		applname[256];
    int			i;
    int			first_slot;
    int			status;
    Cardinal		day;
    Cardinal		month;
    Cardinal		year;
    Widget		view_w;    
    Pixel		foreground;
    XmString		cs;
    MrmType		class;
#if MEMEX
    lwk_boolean		hyperinvoked = FALSE;
    lwk_string		operation;
    lwk_surrogate	surrogate;
    lwk_status		status_his_init;
    lwk_status		status_his;
    lwk_status		status_his_dwui;
    lwk_string		status_text;
#endif
    Atom		XA_COMPOUND_TEXT;
    long		byte_count, cvt_status;
    char		*ct_applname;

    /*	  
    **  Go get application resources
    */	  
    init_resources(cd);


    cd->print_arg_text		   = NULL;
    cd->selected_entries           = NULL;
    cd->number_of_selected_entries = 0;

    cd->cb = MWCreateCalendarBlock
	(12, (XtCallbackProc)MONTHGetMonthMarkup, cd);
    cd->alarms_out_number = 0;
    cd->alarms_out        = NULL;
    cd->main_help_widget = NULL;
    
    
    XtVaSetValues (cd->toplevel, XmNallowShellResize, TRUE, NULL);

    /*
    **  Let's fetch the mainwindow, menus, etc 
    */
#if MEMEX
    status = MrmFetchWidget
	(cd->ads->hierarchy, "calendar", cd->toplevel, &cd->mainwid, &class);
#else
    status = MrmFetchWidget
	(cd->ads->hierarchy, "calendar_nomemex", cd->toplevel, &cd->mainwid, &class);
#endif /* MEMEX */

    if (status != MrmSUCCESS)
	DWC$UI_Catchall(DWC$DRM_CALENDAR, status, 0);

    /*
    ** Do the copyright text.
    */
    cs = MISCFetchDRMValue (dwc_k_calendar_copyright_text);  /* don't free CS */
    cd->appl_title = MISCGetTextFromCS(cs);

/* #if defined(I18N_MULTIBYTE) */
/* this comment is a reminder, that I can make the text in the profile */
/* compound text and thus make this processing shorter */
    XA_COMPOUND_TEXT = XmInternAtom
	(XtDisplay(cd->toplevel), "COMPOUND_TEXT", False);

    ct_applname = XmCvtXmStringToCT(cs);

    XtVaSetValues
    (
	cd->toplevel,
	XtNtitle, ct_applname,
	XtNtitleEncoding, XA_COMPOUND_TEXT,
	NULL
    );
    XtFree(ct_applname);
    XtFree(cd->appl_title);

    /*	  
    **  Go set the mainwid width and height based on font size and screen size
    **	and the particular first_display chosen
    */	  
    set_mainwid_widthheight(cd);

    /*
    **  General widget setup
    **	**********************************************************************
    */
    
    cd->db_nameas_created   = FALSE;
    cd->db_print_created    = FALSE;
    cd->db_general_created  = FALSE;
    cd->db_general_up       = FALSE;
    cd->db_dayview_created  = FALSE;
    cd->db_dayview_up	    = FALSE;
    cd->db_iconbox_created  = FALSE;
    cd->db_iconbox_up       = FALSE;
    cd->db_alarms_created   = FALSE;
    cd->db_alarms_up        = FALSE;

    cd->delete_pending	    = FALSE;

    cd->db_open = NULL;
    cd->db_open_new = NULL;
    cd->db_nameas = NULL;
    cd->cb_delete = NULL;
    cd->db_print = NULL;
    cd->db_include_file = NULL;
    cd->general_db = NULL;
    cd->dayview_db = NULL;
    cd->iconbox_db = NULL;
    cd->alarms_db = NULL;


    cd->db_repeat_questions = NULL;
    
    cd->timeslot_small_font_label  =
      XmCreateLabel (cd->mainwid, "font_small_tb",  NULL, 0);
    cd->timeslot_medium_font_label =
      XmCreateLabel (cd->mainwid, "font_medium_tb", NULL, 0);
    cd->timeslot_large_font_label  =
      XmCreateLabel (cd->mainwid, "font_large_tb",  NULL, 0);

    /*
    ** Make sure the medium pixmaps are loaded (for now).
    */
    MISCLoadPixmapArrays(cd->mainwid, 1);

    /*
    **  Create the daydisplay
    */
#if !defined(DWC_CREATE_DAY_ONLY_WHEN_NEEDED)
    create_daydisplay(cd);
#else
    cd->lw_day_display = NULL;
    if (cd->profile.first_display == show_day)
    {
	create_daydisplay(cd);
    }
#endif

    /*
    **  Month and Year Displays -- deferred if not needed immediately.
    **	***********************************************************************
    */
    cd->lw_month_display = NULL;
    if (cd->profile.first_display == show_month)
    {
	MONTHCreateMonthDisplay (cd);
    }
    else
    {
        cd->month_display = cd->day_yd->months [0];
    }

    cd->yd = NULL;
    if (cd->profile.first_display == show_year)
    {
	CALCreateYearDisplay (cd);
    }


    XtManageChild (cd->mainwid);

    MISCGetTime(&local_time);	/* get the locl internal time */

    date_time.day   = local_time->tm_mday;
    date_time.month = local_time->tm_mon;

    cd->clock_id = 0;
    cd->alarm_id = 0;
    cd->button_timer_id = 0;
    cd->auto_scroll_day = FALSE;

    /*
    **  'last_timer_day' is the last day on which the timer was fired. If we
    **	set it to 32, this will force an icon update at startup time (since
    **	we'll never get a month with 32 days in it
    */    
    cd->last_timer_day = 32;    
    ALARMSClockTimerTick (cd);

    /*
    ** I believe that this method of checking up on the color capabilities is
    ** inadequate.
    */
    if (((DefaultVisualOfScreen (XtScreen (cd->toplevel)))->class == StaticGray) ||
	((DefaultVisualOfScreen (XtScreen (cd->toplevel)))->class == GrayScale)) {
	XtVaGetValues (cd->mainwid, XmNforeground, &foreground, NULL);

	XtVaSetValues (cd->pb_mark_special, XmNforeground, foreground, NULL);
    }

    cd->mapped = False;
    XtSetMappedWhenManaged (cd->toplevel, False);
    XtRealizeWidget (cd->toplevel);

#if 0
    /*
    ** Make any initializations needed for this main window.
    */
    HELPInitializeForDisplay (cd);
#else
    cd->help_context = (Opaque)0;
#endif

    /*
    ** Set up the icons.
    */
    ICONSSetIconifyIcon (cd->toplevel, 0);

    ICONSSetIconBoxIcon
    (
	cd->toplevel,
	NULL,
	&date_time,
	cd->icon_day_fontlist,
	cd->icon_month_fontlist,
	False,
	True,
	cd->mapped
    );

    /*
    ** Add an event handler to change the icon on reparent.
    */
    XtAddEventHandler
    (
	cd->toplevel,
	StructureNotifyMask,
	False,
	(XtEventHandler)CALMap,
	(caddr_t)cd
    );

    /*
    ** Add event handlers to change the copyright notice to the usual
    ** text.
    */
    CALAddCopyrightHandlers (cd);

    /*	  
    **  Declare some atoms for communication with the window manager.
    */
    DwcWmChangeStateAtom = XmInternAtom
	(XtDisplay(cd->toplevel), "WM_CHANGE_STATE", FALSE);

    /*
    ** Set up the DELETE_WINDOW and SAVE_YOURSELF callbacks.
    ** This has to happen after we've realized our top shell or it won't work.
    */
    MISCAddProtocols
    (
	cd->toplevel,
	(XtCallbackProc)FILEDO_CLOSE,
	(XtCallbackProc)FILESAVE_YOURSELF
    );

    XtAddEventHandler
    (
	cd->mainwid,
	NoEventMask,
	TRUE,
	(XtEventHandler)CLIPReceiveClientMessage,
	cd
    );

    cd->iconised      = FALSE;
    cd->zero_time_up  = FALSE;
    cd->active_widget = NULL;
    cd->last_day = 0;
    cd->day      = 0;
    cd->month    = 0;
    cd->year     = 0;

    DSWDoScrollCallback ((DayslotsWidget)cd->dayslots.widget);
    DSWMoveDayslotsToTime
	((DayslotsWidget)cd->dayslots.widget, cd->profile.work_minutes_start);

#if MEMEX
    /*
    **  Initialize the MEMEX Services
    */
    status_his_init = lwk_initialize(&hyperinvoked, &operation, &surrogate);
    if (status_his_init != lwk_s_success)
    {
	/* unable to lwk_initialize */
	lwk_status_to_string(status_his_dwui, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorHisInitFail", status_text);
	lwk_delete_string(&status_text);
    }	    
    else
    {
        /*	  
	**  lwk_initialized, go create the Memex ui stuff
	*/	  
        status_his_dwui = MEMEXCreateDwUi(cd);
	if (status_his_dwui != lwk_s_success)
	{
	    /*	  
	    **	The DwUiCreate didn't work, then report it. We do it here
	    **	so that the message box will come out on top of the
	    **	mainwid
	    */	  
	    lwk_status_to_string(status_his_dwui, &status_text);
	    if (status_his_dwui == lwk_s_drm_open_error)
	    {
		ERRORDisplayText
		    (cd->mainwid, "ErrorHisDrmNotFound", status_text);
	    }
	    else
	    {
		ERRORDisplayText
		    (cd->mainwid, "ErrorDwUICreateFail", status_text);
	    }
	    lwk_delete_string(&status_text);
	}
    }


    if ((hyperinvoked) &&
	(status_his_init == lwk_s_success) &&
	(status_his_dwui == lwk_s_success))
    {
	/*	  
	**	We've been hyperinvoked so we better try and do
	**	something with the surrogate that was passed to us. If
	**	we fail then we should just die gracefully since the
	**	user was doing a visit or goto from some other
	**	application.
	*/	  
	status_his = MEMEXApplyCallback
	(
	    (lwk_ui)cd->hisdwui,
	    (lwk_reason)0,
	    (XmAnyCallbackStruct *)NULL,
	    (lwk_closure)cd,
	    (lwk_surrogate)surrogate,
	    (lwk_string)operation,
	    (lwk_integer)lwk_c_follow_go_to
	);
	if (status_his != lwk_s_success)
	{
	    /* report the error */
	    ERRORDisplayError(cd->mainwid, "ErrorColdStart");
	    exit(DwcCleanExitCode);
	}
    }

    if (!hyperinvoked)
    {
	/*	  
	**  We weren't hyperinvoked so just start up like we normally would
	*/	  
#endif /* MEMEX */

        /*	  
	**  Pre-build the dayview to save time later. This also has the effect
	**  of making the yearview faster since we'll have already loaded the
	**  fonts.
	*/	  
        MISCChangeView
	(
	    cd,
	    show_day,
	    local_time->tm_mday,
	    local_time->tm_mon,
	    local_time->tm_year
	);

        /*	  
	**  If their first view is one other than the day, go and switch to that
	**  one.
	*/	  
        if (cd->profile.first_display != show_day)
	{
	    MISCChangeView(cd, cd->profile.first_display, local_time->tm_mday,
			     local_time->tm_mon, local_time->tm_year);
	}
#if MEMEX
    }
#endif /* MEMEX */




    XtSetMappedWhenManaged (cd->toplevel, True);
    XtMapWidget (cd->toplevel);

    /*	  
    **	This needs to be done after we've created the MEMEX ui so that the
    **	MEMEX accelerators can be added also.
    */	  
    XtInstallAllAccelerators (cd->mainwid, cd->mainwid);

    /*	  
    **  Make sure any messages that may have been popped up in startup are
    **	really on top so the user can see them.
    */	  
    MISCPutMessagesOnTop (cd->mainwid);


    CALStartTimeTextWorkProc (cd);
    
}

void CALDestroyCalendar
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    Boolean		found;
    Cardinal		i;

    
    if (cd->button_timer_id != 0)
    {
	XtRemoveTimeOut (cd->button_timer_id);
    }
    if (cd->clock_id != 0)
    {
	XtRemoveTimeOut (cd->clock_id);
    }
    if (cd->alarm_id != 0)
    {
	XtRemoveTimeOut (cd->alarm_id);
    }

    XtFree (cd->filespec);
    XtFree (cd->filename);

    DAYClearAllItems(&cd->dayslots);
    DAYClearAllItems(&cd->daynotes);

    found = FALSE;
    for (i = 0;  i < cd->ads->number_of_calendars;  i++)
    {
	if (found)
	{
	    cd->ads->cds [i - 1] = cd->ads->cds [i];
	}
	else
	{
	    if (cd->ads->cds [i] == cd)
	    {
		found = TRUE;
	    }
	}
    }

    cd->ads->number_of_calendars--;
    if (cd->ads->number_of_calendars == 0)
    {
	XtFree ((char *)cd->ads->cds);
	cd->ads->cds = NULL;
    }

    XtFree ((char *)cd);

}

int calendar_main
#if defined(_DWC_PROTO_)
	(
	int	argc,
	char	**argv)
#else	/* no prototypes */
	(argc, argv)
	int	argc;
	char	**argv;
#endif	/* prototype */
{
    static char *uid_filename_list [] = {DwcTCalendarUIDFile};
    int static uid_filename_count = (XtNumber(uid_filename_list));

    MrmOsOpenParamPtr	*uid_ancillary_list = NULL;

    int			status;
    char		*filespec;
    Boolean		sync = FALSE;    
    CalendarDisplay	new_cd;
    Display		*top_display;

    MISCUpdateTime();
    
#ifdef R5_XLIB
    /* XtSetLanguageProc() should be called before XtAppInitialize() */
    XtSetLanguageProc(NULL, NULL, NULL);
#endif /* R5_XLIB */

    MrmInitialize();
    
    DXmInitialize();

#if (((XmVERSION == 1) && (XmREVISION == 2)) || XmVERSION == 2)
    XmRepTypeInstallTearOffModelConverter();
#endif
    /*
    **	Register the class of our user defined widgets
    */
    status = DwcIbwInitializeForMrm ();

    if (status != MrmSUCCESS)
	DWC$UI_Catchall(DWC$DRM_ICONBOX, status, 0);

    /*
    **	Initialize the toolkit.  This call returns the id of the "toplevel"
    **	widget.  The applications "main" widget must be the only child of this
    **	widget.
    */
#if 0
    ads.root = XtAppInitialize
    (
	&app_context,	/* app context */
	CalendarTopClassName,
	NULL, 0,	/* Options and num_options  */
	&argc, argv,	/* cmd line args num and args */
	NULL,		/* fallback resources */
	NULL, 0		/* override resources values and num */
    );
#else
    XtToolkitInitialize();

    app_context = XtCreateApplicationContext();

    top_display = XtOpenDisplay
    (
	app_context,
	NULL,
	CalendarDisplayAppName,
	CalendarTopClassName,
	NULL,
	0,
	&argc,
	argv
    );

    if (top_display == 0)

	/* exit with error */
	XtAppErrorMsg
	(
	    app_context,
	    "invalidDisplay",
	    "XtOpenDisplay",
	    "XtToolkitError",
	    "Can't Open display",
	    (String *) NULL,
	    (Cardinal *) NULL
	);

    ads.root = XtAppCreateShell
    (
	CalendarDisplayAppName,
	CalendarTopClassName,
	applicationShellWidgetClass,
	top_display,
	NULL,
	0
    );

#endif
    /*
    **  For debugging
    */
    if (sync)
    {
	XSynchronize (top_display, TRUE);
    } 
    
    /*
    **  Let's open up the hierarchy. Use PerDisplay for Motif 1.2. The 
    **  other call is now deprecated. 
    */
    status = 
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
	MrmOpenHierarchyPerDisplay
    (
	top_display, 		/* display                      */
#else
	MrmOpenHierarchy
    (
#endif
	uid_filename_count,	/* number of UID files          */
	uid_filename_list,	/* names of the UID files       */
	uid_ancillary_list,     /* MrmOpenParamPtr =  null      */
	&ads.hierarchy		/* hierachy return ID           */
    );                          

    if ( status != MrmSUCCESS )
    {
	DWC$UI_Catchall(DWC$DRM_HIERARCHY, status, 0);
    }
	
    register_names ();


    init_time_texts ();
    
    HELPInitialize (&ads);

    CLIPSMInitialise (ads.root);
    
    /*	  
    **  Define cursors
    */	  
    ads.wait_cursor     = ICONSWaitCursorCreate(ads.root);
    ads.inactive_cursor = ICONSInactiveCursorCreate (ads.root);

    ads.number_of_calendars = 0;
    ads.filespec            = NULL;
    
    ads.requestor = ads.root;

    /*
    ** Xt Errors.
    */
    XtAppSetErrorHandler(app_context, OurError);
    XtAppSetWarningHandler(app_context, OurWarning);

    /*
    ** X Errors.
    */
    XSetErrorHandler (ErrorHandler);

    /*	  
    **  Get the db filename
    */	  
    if (argc == 1)
    {
	filespec = db_filespec();
	FILEOpenCalendar (&ads, filespec, &new_cd);
    }
    else
    {
	int i;
	for (i = 1; i < argc; i++)
	{
	    if (*argv[i]=='-')
	    {
		/* error, illegal switch */
	    }
	    else
	    {
		FILEOpenCalendar (&ads, XtNewString(argv[i]), &new_cd);
	    }
	}
    }

    /*
    **  Loop and process events
    */
    XtAppMainLoop (app_context);

    /*
    **   Should never get here!
    */
    exit (DwcErrorExitCode);

}

#if !defined (AUD) && !defined (COMBINE)
/*
** Main program when built as a standalone executable.  On ULTRIX, calendar
** is combined with other applications into a single executable and so this
** routine is not compiled (just calendar_main).
*/
int main
#if defined(_DWC_PROTO_)
	(
	unsigned int	argc,
	char		**argv)
#else	/* no prototypes */
	(argc, argv)
	unsigned int	argc;
	char		**argv;
#endif	/* prototype */
{
#if !defined(FVM_ON)
#define FVM_ON 0
#endif

#if FVM_ON && defined(VMS)
#pragma nostandard
    globalref FAKE_VM_STRING_OFF;
    globalref FAKE_VM_CHECK_ACTIVE_LIST;
    globalref FAKE_VM_REAL_FREE_OFF;
#pragma standard

    FAKE_VM_STRING_OFF = 1;

#if defined(FVM_REAL_FREE_OFF)
    /* 1==pattern, 3==separate pages */
    FAKE_VM_REAL_FREE_OFF = FVM_REAL_FREE_OFF;
#endif

#if defined(FVM_CHECK_ACTIVE_LIST)
    FAKE_VM_CHECK_ACTIVE_LIST = 1;  /* this makes things super slow */
#endif

    FAKE_VM_INTERCEPT_XFER_VECTOR();
#endif

    calendar_main (argc, argv);
}
#else
#if defined (AUD)
void calshr_init ()
{
}
#endif
#endif	/* COMBINE && AUD*/

void CALCreateYearDisplay
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    Arg			arglist [20];
    Cardinal		ac;
    Cardinal		month;
    Cardinal		day;
    int			bit;
    struct tm		*local_time;

    MONTHmonth_dc_cb[0].closure = (caddr_t) cd;
    MONTHmonth_help_cb[0].closure = (caddr_t) cd;

    ac = 0;
    XtSetArg(arglist[ac], DwcMwNhelpCallback, MONTHmonth_help_cb); ac++;
    XtSetArg(arglist[ac], DwcMwNdcCallback, MONTHmonth_dc_cb); ac++;
    XtSetArg(arglist[ac], DwcMwNcalendarBlock, cd->cb); ac++;
    XtSetArg(arglist[ac], DwcMwNworkDays, cd->profile.work_days); ac++;
    XtSetArg(arglist[ac], DwcMwNstyleMask, cd->profile.month_style_mask); ac++;
    XtSetArg(arglist[ac], DwcMwNshowWeekNumbers,
	cd->profile.show_week_numbers); ac++;
    XtSetArg(arglist[ac], DwcMwNweekNumbersStartingDay,
	cd->profile.week_numbers_start_day); ac++;
    XtSetArg(arglist[ac], DwcMwNweekNumbersStartingMonth,
	cd->profile.week_numbers_start_month); ac++;
    XtSetArg(arglist[ac], DwcMwNweekStart, cd->profile.first_day_of_week); ac++;
    XtSetArg(arglist[ac], XmNshadowThickness, 0); ac++;

    HELPwidget_help_cb [0].closure = (caddr_t) MISCFetchDRMValue
	(dwc_k_help_year_general);

    /*
    **  Get the local internal time
    */
    MISCGetTime(&local_time);

    assert (ac <= XtNumber(arglist));
    cd->yd = YEARCreateYearDisplay
    (
	(Widget)cd->mainwid,			/* parent */
	"YearDisplay",				/* name */
	(Dimension)cd->profile.year_width,
	(Dimension)cd->profile.year_height,
	(Boolean)TRUE,				/* vertical scrollbar */
	(Cardinal)local_time->tm_year,		/* first_year */
	(Cardinal)10,				/* scroll_width */
	(Arg *)arglist, (Cardinal)ac,
	(XtCallbackList)HELPwidget_help_cb,	/* help callback */
	(XtCallbackList)NULL,			/* no scroll help cb */
	(YearResizeCallbackProc)YEARResize,	/* resize proc */
	(dwcaddr_t)cd
    );

}

static void register_names
#if defined(_DWC_PROTO_)
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */

{
    static MrmRegisterArg regvec[] =
    {
	/*
	**  Identifiers
	*/

	/*
	**  Callbacks	
	*/
	{"dwc_help_from_menu_activate",	(caddr_t)HELPFromMenu},
	{"dwc_help_for_widget_activate",(caddr_t)HELPForWidget},
	{"dwc_help_for_modal_activate",	(caddr_t)HELPForModalWidget},
	{"generic_create_proc",		(caddr_t)MISCGenericCreateProc},
	{"generic_modal_create",	(caddr_t)MISCFetchModal},
	{"file_menu_map",		(caddr_t)FILEDO_FILE_MENU},
	{"do_open_file",		(caddr_t)FILEDO_OPEN_FILE},
	{"do_close",			(caddr_t)FILEDO_CLOSE},
	{"do_nameas",			(caddr_t)FILEDO_NAMEAS},
	{"file_nameas_ok",		(caddr_t)FILEDO_NAMEAS_OK},
	{"file_nameas_cancel",		(caddr_t)FILEDO_NAMEAS_CANCEL},
	{"do_delete",			(caddr_t)FILEDO_DELETE},
	{"caution_delete_file",		(caddr_t)FILEDO_CAUTION_DELETE_FILE},
	{"file_print_menu",		(caddr_t)FILE_PRINT_MENU},
	{"do_include",			(caddr_t)FILEDO_INCLUDE},
	{"bye_bye",			(caddr_t)FILEBYE_BYE},
	{"do_edit_menu",		(caddr_t)EDITDO_EDIT_MENU},
	{"do_cut",			(caddr_t)CLIPDO_CUT},
	{"do_copy",			(caddr_t)CLIPDO_COPY},
	{"do_clear",			(caddr_t)CLIPDO_CLEAR},
	{"do_paste",			(caddr_t)CLIPDO_PASTE},
	{"do_selall",			(caddr_t)CLIPDO_SELALL},
	{"do_view_menu",		(caddr_t)VIEWDO_VIEW_MENU},
	{"view_selected",		(caddr_t)VIEW_SELECTED},
	{"view_today",			(caddr_t)VIEW_TODAY},
	{"view_day",			(caddr_t)VIEW_DAY},
	{"view_month",			(caddr_t)VIEW_MONTH},
	{"view_year",			(caddr_t)VIEW_YEAR},
	{"do_schedule_menu",		(caddr_t)DAYDO_ENTRY_MENU},
	{"schedule_entry_edit",		(caddr_t)DAYDO_ENTRY_EDIT},
	{"schedule_entry_delete",	(caddr_t)DAYDO_ENTRY_DELETE},
	{"schedule_entry_close",	(caddr_t)DAYDO_ENTRY_CLOSE},
	{"schedule_entry_reset",	(caddr_t)DAYDO_ENTRY_RESET},
	{"custom_menu",			(caddr_t)OPTIONSCUSTOM_MENU},
	{"do_general_display",		(caddr_t)CUSTDO_GENERAL_DISPLAY},
	{"do_dayview_display",		(caddr_t)CUSTDO_DAYVIEW_DISPLAY},
	{"do_iconbox_display",		(caddr_t)CUSTDO_ICONBOX_DISPLAY},
	{"do_alarms_display",		(caddr_t)CUSTDO_ALARMS_DISPLAY},
	{"do_save_current_settings",(caddr_t)PROFILEDO_SAVE_CURRENT_SETTINGS},
	{"do_restore_settings",		(caddr_t)PROFILEDO_RESTORE_SETTINGS},
	{"do_use_default_settings", (caddr_t)PROFILEDO_USE_DEFAULT_SETTINGS},
	{"mark_menu",			(caddr_t)OPTIONSMARK_MENU},
	{"mark_item",			(caddr_t)OPTIONSMARK_ITEM},
	{"errorbox_destroy",		(caddr_t)ERRORDestroyErrorBox},
	{"destroy_modal_help",		(caddr_t)HELPDestroyForModal}
    };
    MrmCount	    regnum = (XtNumber(regvec));

    /*
    **  Let's register our routines
    */
    MrmRegisterNames (regvec, regnum);
    
    return;
}

XtAppContext CALGetAppContext
#if defined(_DWC_PROTO_)
	(
	)
#else	/* no prototypes */
	()
#endif	/* prototype */
{
    return(app_context);
}

static void create_daydisplay
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    Arg		    arglist[35];
    Cardinal	    ac;
    struct tm	    *local_time;
    XmFontList	    fontlist;
    Widget	    junkw;
    Widget	    timebar_frame;
    Dimension	    height;

    /*
    **  Create the DayDisplay
    */
    HELPwidget_help_cb [0].closure = (caddr_t) MISCFetchDRMValue
	(dwc_k_help_day_general);
    DAYresize_day_cb [0].closure = (caddr_t) cd;
    DAYfocus_day_cb  [0].closure = (caddr_t) cd;
    cd->lw_day_display = LwLayout
    (
	cd->mainwid,
	"DayDisplay",
	TRUE,
	(Position)0, (Position)0,
	(Dimension)0, (Dimension)0,
	(XtCallbackList)DAYresize_day_cb,
	(XtCallbackList)NULL,
	(XtCallbackList)DAYfocus_day_cb,
	(XtCallbackList)HELPwidget_help_cb,
	(caddr_t)cd
    );

    XtVaSetValues
    (
	(Widget) cd->lw_day_display,
	XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
	XmNtraversalOn, True,
	XmNshadowThickness, 2,
	XmNshadowType, XmSHADOW_OUT,
	XmNmarginWidth, 14,
	XmNmarginHeight, 10,
	NULL
    );

    /*	  
    **  Create the Months at the top of the daydisplay
    */	  
    MONTHmonth_dc_cb[0].closure = (caddr_t) cd;
    MONTHmonth_help_cb[0].closure = (caddr_t) cd;
    
    HELPwidget_help_cb[0].closure =
	    (caddr_t)MISCFetchDRMValue(dwc_k_help_day_months);

    ac = 0;
    XtSetArg(arglist[ac], DwcMwNhelpCallback, MONTHmonth_help_cb); ac++;
    XtSetArg(arglist[ac], DwcMwNdcCallback, MONTHmonth_dc_cb); ac++;
    XtSetArg(arglist[ac], DwcMwNcalendarBlock, cd->cb); ac++;
    XtSetArg(arglist[ac], DwcMwNweekStart, cd->profile.first_day_of_week); ac++;
    XtSetArg(arglist[ac], DwcMwNworkDays, cd->profile.work_days); ac++;
    XtSetArg(arglist[ac], DwcMwNstyleMask,cd->profile.month_style_mask); ac++;
    XtSetArg(arglist[ac], DwcMwNshowWeekNumbers, cd->profile.show_week_numbers);
			ac++;
    XtSetArg(arglist[ac], DwcMwNweekNumbersStartingDay,
			cd->profile.week_numbers_start_day); ac++;
    XtSetArg(arglist[ac], DwcMwNweekNumbersStartingMonth,
			cd->profile.week_numbers_start_month); ac++;
    XtSetArg(arglist[ac], XmNshadowThickness, 0); ac++;
    XtSetArg(arglist[ac], XmNtraversalOn, True); ac++;

    MISCGetTime(&local_time);	/* get the locl internal time */

    assert (ac <= XtNumber(arglist));
    cd->day_yd = YEARCreateYearDisplay
    (
	(Widget) cd->lw_day_display, /* parent */
	"Months",		/* name */
	(Dimension)100,			/* width */
	(Dimension)100,			/* height */
	(Boolean)FALSE,			/* no vertical_scrollb */
	(Cardinal)local_time->tm_year,	/* first year */
	(Cardinal)1,			/* no of years */
	(Arg *)arglist, (Cardinal)ac,
	HELPwidget_help_cb,	/* help callback */
	NULL,			/* scroll help callback */
        NULL,			/* resize proc */
	(dwcaddr_t)NULL			/* tag */
    );
    XtVaSetValues
    (
	(Widget)cd->day_yd->layout,
	XmNshadowThickness, 0,
	XmNmarginWidth, 14,	/* same as parent */
	XmNmarginHeight, 10,	/* same as parent */
	NULL
    );


    ac = 0;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    XtSetArg(arglist[ac], XmNshadowThickness, 4); ac++;
    assert (ac <= XtNumber(arglist));
    cd->day_sep_1 = XmCreateSeparator
	((Widget)cd->lw_day_display, "separator2", arglist, ac);
    cd->day_sep_2 = XmCreateSeparator
	((Widget)cd->lw_day_display, "separator2", arglist, ac);

    /*
    **  Day notes and Dayslots widgets setup
    */
    
    create_daynote(cd);
    
    /*	  
    **  Timebar create
    */	  
    /*	  
    **  What font are we going to use for the Timebar widget
    */	  
    switch (cd->profile.day_font_size)
    {
    case 0:
	junkw = cd->timeslot_small_font_label;
	break;
    case 1:
	junkw = cd->timeslot_medium_font_label;
	break;
    case 2:
	junkw = cd->timeslot_large_font_label;
	break;
    }
    XtVaGetValues (junkw, XmNfontList, &fontlist, NULL);

    HELPwidget_help_cb[0].closure =
		(caddr_t)MISCFetchDRMValue(dwc_k_help_day_timebar);

    /*	  
    **  Create the frame around the timebar.
    */	  
    ac = 0;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    assert (ac <= XtNumber(arglist));
    timebar_frame = (Widget)XmCreateFrame
	((Widget)cd->lw_day_display, "timebarFrame", arglist, ac);

    ac = 0;
    XtSetArg(arglist[ac], XmNhelpCallback, HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], XmNfontList, fontlist); ac++;
    XtSetArg(arglist[ac], DwcTbwNhorizontalMargin, 1); ac++;
    XtSetArg(arglist[ac], XmNheight, 100); ac++;
    XtSetArg(arglist[ac], XmNwidth, cd->profile.day_timebar_width); ac++;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    XtSetArg(arglist[ac], DwcTbwNverticalMargin, 0); ac++;
    assert (ac <= XtNumber(arglist));
    cd->day_wrkhrsbar = TBWTimebarCreate
	(timebar_frame, "timebar", arglist, ac);

    XtManageChild(cd->day_wrkhrsbar);

    /*	  
    **  ScrollWindow create
    */
    ac = 0;
    XtSetArg(arglist[ac], XmNwidth, 100); ac++;
    XtSetArg(arglist[ac], XmNheight, 100); ac++;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    XtSetArg(arglist[ac], XmNshadowThickness, 2); ac++;
    XtSetArg(arglist[ac], XmNshadowType, XmSHADOW_IN); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;
    assert (ac <= XtNumber(arglist));
    cd->lw_day_scrollwin = LwLayoutCreate
	((Widget)cd->lw_day_display, "dayslotssw", arglist, ac);

    ac = 0;
    XtSetArg(arglist[ac], XmNwidth, 100); ac++;
    XtSetArg(arglist[ac], XmNheight, 100); ac++;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    XtSetArg(arglist[ac], XmNshadowThickness, 0); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;
    assert (ac <= XtNumber(arglist));
    cd->lw_day_scrollwin = LwLayoutCreate
	((Widget)cd->lw_day_scrollwin, "dayslotssw", arglist, ac);

    /*	  
    **  Scrollbar create
    */	  
    DAYday_scroll_cb [0].closure = (caddr_t) cd;

    ac = 0;
    XtSetArg(arglist[ac], XmNx, 0); ac++;
    XtSetArg(arglist[ac], XmNy, 0); ac++;
    XtSetArg(arglist[ac], XmNwidth, 0); ac++;
    XtSetArg(arglist[ac], XmNheight, 50); ac++;
    XtSetArg(arglist[ac], XmNincrement, 1); ac++;
    XtSetArg(arglist[ac], XmNpageIncrement, 1); ac++;
    XtSetArg(arglist[ac], XmNsliderSize, 1); ac++;
    XtSetArg(arglist[ac], XmNvalue, 0); ac++;
    XtSetArg(arglist[ac], XmNminimum, -1); ac++;
    XtSetArg(arglist[ac], XmNmaximum, 1441); ac++;
    XtSetArg(arglist[ac], XmNorientation, XmVERTICAL); ac++;
    XtSetArg(arglist[ac], XmNtraversalOn, True); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmSTICKY_TAB_GROUP); ac++;
    XtSetArg(arglist[ac], XmNincrementCallback, DAYday_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNdecrementCallback, DAYday_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNpageIncrementCallback, DAYday_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNpageDecrementCallback, DAYday_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNtoTopCallback, DAYday_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNtoBottomCallback, DAYday_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNdragCallback, DAYday_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNvalueChangedCallback, DAYday_scroll_cb); ac++;
    XtSetArg(arglist[ac], XmNhighlightThickness, 2); ac++;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    assert (ac <= XtNumber(arglist));
    cd->day_scrollbar = XmCreateScrollBar
	((Widget)cd->lw_day_display, "VScrollbar", arglist, ac);

    XtManageChild (cd->day_scrollbar);

    cd->day_scroll_min = -1 ;
    cd->day_scroll_max = 1441 ;

    /*	  
    **  Go create the dayslotswidget which is inside the lw_day_scrollwin
    */	  
    create_dayslotswidget(cd);
    
    XtManageChildren
    (
	DWC_CHILDREN(cd->lw_day_scrollwin),
	DWC_NUM_CHILDREN(cd->lw_day_scrollwin)
    );

    /*	  
    **  Create "Previous Day" and "Next Day" pushbuttons at bottom of DayDisplay
    */	  
    create_day_pbs(cd);

    XtVaGetValues
    (
	cd->pb_day_time,
	XmNheight, &height,
	NULL
    );
    XtVaSetValues
    (
	cd->pb_day_last,
	XmNheight, height,
	XmNwidth, height,
	NULL
    );
    XtVaSetValues
    (
	cd->pb_day_next,
	XmNheight, height,
	XmNwidth, height,
	NULL
    );

    /*	  
    **  Create the "Date" label at the top of the DayDisplay Scrollwindow
    */	  
    create_fulldate_lb(cd);

    XtManageChildren
    (
	DWC_CHILDREN(cd->lw_day_display),
	DWC_NUM_CHILDREN(cd->lw_day_display)
    );
    XtManageChild ((Widget)cd->lw_day_scrollwin);

}

static void create_day_pbs
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay cd;
#endif /* prototype */
{
    Arg			arglist [10];
    Cardinal		ac;
    Dimension		last_width;
    Dimension		next_width;
    Boolean		rtol;

#ifdef DEC_MOTIF_EXTENSION
    rtol = (cd->day_yd->layout->manager.dxm_layout_direction == (int)DXmLAYOUT_LEFT_DOWN);
#else
    rtol = (cd->day_yd->layout->manager.string_direction == (int)XmSTRING_DIRECTION_R_TO_L);
#endif

    DAYpb_last_day_cb[0].closure = (caddr_t) cd;
    day_clock_cb[0].closure = (caddr_t) cd;
    DAYpb_next_day_cb [0].closure = (caddr_t) cd;

    /*	  
    **  Create "Previous Day" pushbutton at bottom of DayDisplay
    */	  
    if (rtol)
	HELPwidget_help_cb[0].closure = (caddr_t) MISCFetchDRMValue
	    (dwc_k_help_day_next);
    else
	HELPwidget_help_cb[0].closure = (caddr_t) MISCFetchDRMValue
	    (dwc_k_help_day_last);

    ac = 0;
    if (rtol)
    {
	XtSetArg(arglist[ac], XmNactivateCallback, DAYpb_next_day_cb); ac++;
    }
    else
    {
	XtSetArg(arglist[ac], XmNactivateCallback, DAYpb_last_day_cb); ac++;
    }
    XtSetArg(arglist[ac], XmNhelpCallback, HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], XmNarrowDirection, XmARROW_LEFT); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmSTICKY_TAB_GROUP); ac++;
    assert (ac <= XtNumber(arglist));
    cd->pb_day_last = XmCreateArrowButton
	((Widget)cd->lw_day_display, "Last", arglist, ac); ac++;

    /*	  
    **  Create the "time" pushbutton at the bottom of the DayDisplay
    */
    HELPwidget_help_cb [0].closure = (caddr_t) MISCFetchDRMValue
	(dwc_k_help_day_clock);

    ac = 0;
    XtSetArg(arglist[ac], XmNhelpCallback, HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], XmNactivateCallback, day_clock_cb); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmSTICKY_TAB_GROUP); ac++;
    assert (ac <= XtNumber(arglist));
    cd->pb_day_time = XmCreatePushButton
	((Widget)cd->lw_day_display, "Time", arglist, ac);

    /*	  
    **  Create the "Next Day" pushbutton at the bottom of the DayDisplay
    */
    if (rtol)
	HELPwidget_help_cb[0].closure = (caddr_t) MISCFetchDRMValue
	    (dwc_k_help_day_last);
    else
	HELPwidget_help_cb[0].closure = (caddr_t) MISCFetchDRMValue
	    (dwc_k_help_day_next);

    ac = 0;
    if (rtol)
    {
	XtSetArg(arglist[ac], XmNactivateCallback, DAYpb_last_day_cb); ac++;
    }
    else
    {
	XtSetArg(arglist[ac], XmNactivateCallback, DAYpb_next_day_cb); ac++;
    }
    XtSetArg(arglist[ac], XmNhelpCallback, HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], XmNarrowDirection, XmARROW_RIGHT); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmSTICKY_TAB_GROUP); ac++;
    assert (ac <= XtNumber(arglist));
    cd->pb_day_next = XmCreateArrowButton
	((Widget)cd->lw_day_display, "Next", arglist, ac);

}

static void create_fulldate_lb
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay cd;
#endif /* prototype */
{
    Arg			arglist [10];
    Cardinal		ac;

    /*	  
    **  Create the "Date" label at the top of the DayDisplay Scrollwindow
    */	  
    HELPwidget_help_cb [0].closure = (caddr_t) MISCFetchDRMValue (dwc_k_help_day_date) ;

    ac = 0 ;
    XtSetArg(arglist[ac], XmNhelpCallback, HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], XmNrecomputeSize, FALSE); ac++;
    cd->day_label = XmCreateLabel
	((Widget)cd->lw_day_display, "fulldate", arglist, ac);

    XtManageChild(cd->day_label);
}

static void init_resources
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay cd;
#endif /* prototype */
{


    XtGetApplicationResources
      (cd->toplevel,		    /* Widget for name const.   */
       cd,			    /* Storage for values	*/
       DwcApplResources,	    /* Resource list		*/
       XtNumber (DwcApplResources), /* Number of resources	*/
       NULL,			    /* arglist			*/
       0) ;			    /* count			*/

}

static void
do_screen_sizes
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    XmFontList		fontlist;
    XFontStruct		*font;

    /*	  
    **  We used to be able to get the default font off of the menubar, however
    **	now that the menubar is a rowcolumn widget we can't do that, so let's
    **	just use the font from the toplevel
    */
#if (((XmVERSION == 1) && (XmREVISION == 2)) || XmVERSION == 2)
    XtVaGetValues (cd->toplevel, XmNlabelFontList, &fontlist, NULL);
#else
    XtVaGetValues (cd->toplevel, XmNdefaultFontList, &fontlist, NULL);
#endif

    MISCGetFontFromFontlist(fontlist, &font);
    cd->screen_font_size = (Dimension)(font->max_bounds.ascent +
			   font->max_bounds.descent);

    cd->width_of_screen  = XWidthOfScreen  (XtScreen (cd->toplevel)) ;
    cd->height_of_screen = XHeightOfScreen (XtScreen (cd->toplevel)) ;

    /*	  
    **  Hack any old profiles that might be hanging around, since they used
    **	different stuff for height, width, etc.
    */	  
    PROFILEOldDBProfileHack (cd) ;

}

static void
set_mainwid_widthheight
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay cd;
#endif /* prototype */
{
    Dimension		main_height;
    Dimension		main_width;

    /*	  
    **  Get and set cd->screen_font_size, cd->width_of_screen and
    **	cd->height_of_screen
    */	  
    do_screen_sizes(cd);
    
    switch (cd->profile.first_display)
    {
      case show_day   :
	main_width  = MISCCvtUnitsToPixels((Dimension)cd->profile.day_width,
						cd->screen_font_size);
	main_height = MISCCvtUnitsToPixels((Dimension)cd->profile.day_height,
						cd->screen_font_size);
	break ;
      case show_week  :
	main_width  = MISCCvtUnitsToPixels((Dimension)cd->profile.week_width,
					    cd->screen_font_size);
	main_height = MISCCvtUnitsToPixels((Dimension)cd->profile.week_height,
					    cd->screen_font_size);
	break ;
      case show_month :
	main_width  = MISCCvtUnitsToPixels((Dimension)cd->profile.month_width,
					    cd->screen_font_size);
	main_height = MISCCvtUnitsToPixels((Dimension)cd->profile.month_height,
					    cd->screen_font_size);
	break ;
      case show_year  :
	main_width  = MISCCvtUnitsToPixels((Dimension)cd->profile.year_width,
					    cd->screen_font_size);
	main_height = MISCCvtUnitsToPixels((Dimension)cd->profile.year_height,
					    cd->screen_font_size);
	break ;
    }
}

static void create_dayslotswidget
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    Arg		    arglist[41];
    Cardinal	    ac;
    unsigned char   *default_icons ;
    Cardinal	    default_icons_number ;
    int		    alarms_number ;
    unsigned short int	*alarms_times ;
    Pixmap	    *pixmaps ;
    Dimension	    pixmap_size;
    XmFontList	    fontlist;
    Widget	    junkw;

    /*	  
    **  What font are we going to use for the Daynotes widget
    */	  
    switch (cd->profile.day_font_size) {
	case 0 :  junkw = cd->timeslot_small_font_label ;   break ;
	case 1 :  junkw = cd->timeslot_medium_font_label ;  break ;
	case 2 :  junkw = cd->timeslot_large_font_label ;   break ;
    }
    XtVaGetValues (junkw, XmNfontList, &fontlist, NULL);

    /*
    **	We fetch the pixmaps only if it hasn't been done yet. !!! this is a
    **	real hack, because if mainwid goes away, then we lose our pixmaps.
    */
    MISCLoadPixmapArrays (cd->mainwid, cd->profile.day_icon_size);

    /*	  
    **  Which pixmaps are we going to use?
    */
    switch (cd->profile.day_icon_size)
    {
    case 0 :
	pixmaps = small_pixmaps;
	pixmap_size = 12;
	break;
    case 1 :
	pixmaps = medium_pixmaps;
	pixmap_size = 17;
	break;
    case 2 :
	pixmaps = big_pixmaps;
	pixmap_size = 32;
	break;
    }

    /*	  
    **  Let's find out what alarms are on by default.
    */	  
    CUSTGetProfileAlarms (&cd->profile, &alarms_number, &alarms_times) ;

    /*	  
    **  If we don't have any alarms on my default, don't put in a bell as one of
    **	the default icons, otherwise, do.
    */	  
    if (alarms_number == 0) {
	default_icons = (unsigned char *) XtMalloc (sizeof (unsigned char)) ;
	default_icons_number = 1 ;
    } else {
	default_icons = (unsigned char *) XtMalloc (sizeof (unsigned char) * 2);
	default_icons_number = 2 ;
	default_icons [1] = k_pixmap_bell ;
    }

    default_icons [0] = cd->profile.default_entry_icon ;

    cd->dayslots.default_item =
      DAYCreateDayItemRecord (alarms_number, alarms_times, 1, default_icons) ;

    if (cd->profile.entries_significant)
    {
	cd->dayslots.default_item->flags = 
	    cd->dayslots.default_item->flags & ~DWC$m_item_insignif;
    }
    else
    {
	cd->dayslots.default_item->flags = 
	    cd->dayslots.default_item->flags |  DWC$m_item_insignif;
    }

    XtFree ((char *)alarms_times);

    DAYday_entry_cb[0].closure = (caddr_t) cd;
    DAYdisp_focus_cb[0].closure = (caddr_t) cd;
    DAYposition_scrollbar_cb[0].closure = (caddr_t) cd;

    HELPwidget_help_cb[0].closure = (caddr_t) MISCFetchDRMValue
	(dwc_k_help_day_timeslots);
    HELPwidget_help_cb[0].closure = (caddr_t) MISCFetchDRMValue
	(dwc_k_help_day_timeslots_entry);

    ac = 0 ;
    XtSetArg(arglist[ac], XmNhelpCallback, HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], DwcDswNentryHelpCallback, HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], XmNy, -9999); ac++;
    XtSetArg(arglist[ac], XmNstringDirection, (XmStringDirection)cd->profile.directionRtoL); ac++;
    XtSetArg(arglist[ac], XmNfontList, fontlist); ac++;
    XtSetArg(arglist[ac], DwcDswNpixmaps, pixmaps); ac++;
    XtSetArg(arglist[ac], DwcDswNpixmapHeight, pixmap_size); ac++;
    XtSetArg(arglist[ac], DwcDswNpixmapWidth, pixmap_size); ac++;
    XtSetArg(arglist[ac], DwcDswNgetTextCallback, get_timetext_cb); ac++;
    XtSetArg(arglist[ac], DwcDswNscrollCallback, DAYposition_scrollbar_cb); ac++;
    XtSetArg(arglist[ac], DwcDswNfocusDisposeCallback, DAYdisp_focus_cb); ac++;
    XtSetArg(arglist[ac], XmNwidth, 400); ac++;
    XtSetArg(arglist[ac], DwcDswNentryCallback, DAYday_entry_cb); ac++;
    XtSetArg(arglist[ac], DwcDswNwrkhrsTimebar, cd->day_wrkhrsbar); ac++;
    XtSetArg(arglist[ac], DwcDswNdefaultNumIcons, default_icons_number); ac++;
    XtSetArg(arglist[ac], DwcDswNdefaultIcons, default_icons); ac++;
    XtSetArg(arglist[ac], DwcDswNtimebarWidth, cd->profile.day_timebar_width);
			    ac++;
    XtSetArg(arglist[ac], DwcDswNstackTopDown, cd->profile.timeslot_stacking);
			    ac++;
    XtSetArg(arglist[ac], DwcDswNonTheLine, cd->profile.times_on_the_line); ac++;
    XtSetArg(arglist[ac], DwcDswNpreferredDayslotsSize,
			    cd->profile.timeslot_size); ac++;
    XtSetArg(arglist[ac], DwcDswNpreferredFineIncrement,
			    cd->profile.fine_increment); ac++;
    XtSetArg(arglist[ac], DwcDswNtimeVerticalMargin,
			    cd->profile.day_v_spacing); ac++;
    XtSetArg(arglist[ac], DwcDswNtimeHorizontalMargin,
			    cd->profile.day_h_spacing); ac++;

    XtSetArg(arglist[ac], DwcDswNworkStart, cd->profile.work_minutes_start); ac++;
    XtSetArg(arglist[ac], DwcDswNworkFinish, cd->profile.work_minutes_end); ac++;
    XtSetArg(arglist[ac], DwcDswNlunchStart, cd->profile.lunch_minutes_start); ac++;
    XtSetArg(arglist[ac], DwcDswNlunchFinish, cd->profile.lunch_minutes_end); ac++;
    XtSetArg(arglist[ac], DwcDswNeditable, !cd->read_only); ac++;
    XtSetArg(arglist[ac], XmNborderWidth, 0); ac++;
    XtSetArg(arglist[ac], XmNshadowThickness, 2); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;

    if (cd->profile.day_show_full_times) {
	XtSetArg(arglist[ac], DwcDswNminuteTexts, NULL); ac++;
    } else {
	XtSetArg(arglist[ac], DwcDswNminuteTexts, minute_texts) ; ac++;
    }

    if (cd->profile.time_am_pm) {
	XtSetArg(arglist[ac], DwcDswNtimeTexts, time_texts_12) ; ac++;
    } else {
	XtSetArg(arglist[ac], DwcDswNtimeTexts, time_texts_24) ; ac++;
    }

    assert (ac <= XtNumber(arglist));
    cd->dayslots.widget = DSWDayslotsCreate
	((Widget) cd->lw_day_scrollwin, "dayslots", arglist, ac);

    cd->dayslots.items  = NULL ;
    cd->dayslots.number_of_items = 0 ;

    XtFree ((char *)default_icons);

}

static void create_daynote
#if defined(_DWC_PROTO_)
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    Arg		    arglist[31];
    Cardinal	    ac;
    XmFontList	    fontlist;
    unsigned char   *default_icons ;
    Cardinal	    default_icons_number ;
    Pixmap	    *pixmaps ;
    Dimension	    pixmap_size;
    Widget	    junkw;
    Widget	    daynote_frame;
    
    /*	  
    **  What font are we going to use for the Daynotes widget
    */	  
    switch (cd->profile.day_font_size)
    {
	case 0 :  junkw = cd->timeslot_small_font_label ;   break ;
	case 1 :  junkw = cd->timeslot_medium_font_label ;  break ;
	case 2 :  junkw = cd->timeslot_large_font_label ;   break ;
    }
    XtVaGetValues (junkw, XmNfontList, &fontlist, NULL);

    /*
    **	We fetch the pixmaps only if it hasn't been done yet. !!! this is a
    **	real hack, because if mainwid goes away, then we lose our pixmaps.
    */
    MISCLoadPixmapArrays (cd->mainwid, cd->profile.day_icon_size);

    /*	  
    **  Which pixmaps are we going to use?
    */	  
    switch (cd->profile.day_icon_size)
    {
    case 0 :
	pixmaps = small_pixmaps;
	pixmap_size = 12;
	break;
    case 1 :
	pixmaps = medium_pixmaps;
	pixmap_size = 17;
	break;
    case 2 :
	pixmaps = big_pixmaps;
	pixmap_size = 32;
	break;
    }

    default_icons = (unsigned char *) XtMalloc (sizeof (unsigned char)) ;
    default_icons_number = 1 ;
    default_icons [0] = cd->profile.default_notes_icon ;

    cd->daynotes.default_item =
      DAYCreateDayItemRecord (0, NULL, default_icons_number, default_icons) ;

    if (cd->profile.notes_significant) {
	cd->daynotes.default_item->flags = 
	    cd->daynotes.default_item->flags & ~DWC$m_item_insignif ;
    } else {
	cd->daynotes.default_item->flags = 
	    cd->daynotes.default_item->flags |  DWC$m_item_insignif ;
    }


    /*	  
    **  Create the frame around the daynote.
    */	  
    ac = 0 ;
    XtSetArg(arglist[ac], XmNshadowThickness, 2); ac++;
    XtSetArg(arglist[ac], XmNshadowType, XmSHADOW_IN); ac++;
    assert (ac <= XtNumber(arglist));
    daynote_frame = (Widget)XmCreateFrame
	((Widget)cd->lw_day_display, "daynoteFrame", arglist, ac);

    /*	  
    **  Daynotes widget create
    */	  
    HELPwidget_help_cb [0].closure = (caddr_t) MISCFetchDRMValue (dwc_k_help_day_daynotes) ;
    HELPwidget_help_cb  [0].closure = (caddr_t) MISCFetchDRMValue
						(dwc_k_help_day_daynote_entry) ;

    DAYdns_entry_cb[0].closure = (caddr_t) cd ;
    DAYdisp_focus_cb [0].closure = (caddr_t) cd;

    ac = 0 ;
    XtSetArg(arglist[ac], XmNhelpCallback, HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], DwcDswNentryHelpCallback,HELPwidget_help_cb); ac++;
    XtSetArg(arglist[ac], XmNstringDirection, (XmStringDirection)cd->profile.directionRtoL); ac++;
    XtSetArg(arglist[ac], XmNfontList, fontlist); ac++;
    XtSetArg(arglist[ac], XmNpageIncrement, 1); ac++;
    XtSetArg(arglist[ac], DwcDswNpixmaps,pixmaps); ac++;
    XtSetArg(arglist[ac], DwcDswNpixmapHeight, pixmap_size); ac++;
    XtSetArg(arglist[ac], DwcDswNpixmapWidth, pixmap_size); ac++;
    XtSetArg(arglist[ac], DwcDswNtimeVerticalMargin,
				cd->profile.day_v_spacing); ac++;
    XtSetArg(arglist[ac], DwcDswNtimeHorizontalMargin,
				cd->profile.day_h_spacing); ac++;
    XtSetArg(arglist[ac], XmNwidth, 400); ac++;
    XtSetArg(arglist[ac], DwcDswNentryCallback, DAYdns_entry_cb); ac++;
    XtSetArg(arglist[ac], DwcDswNfocusDisposeCallback, DAYdisp_focus_cb); ac++;
    XtSetArg(arglist[ac], DwcDswNdayNotes, TRUE); ac++;
    XtSetArg(arglist[ac], DwcDswNdefaultNumIcons, default_icons_number); ac++;
    XtSetArg(arglist[ac], DwcDswNdefaultIcons,    default_icons); ac++;
    XtSetArg(arglist[ac], DwcDswNeditable, !cd->read_only); ac++;
    XtSetArg(arglist[ac], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ac++;

    assert (ac <= XtNumber(arglist));
    cd->daynotes.widget = DSWDayslotsCreate
	(daynote_frame, "dnsslots", arglist, ac) ;

    XtManageChild(cd->daynotes.widget);

    cd->daynotes.items  = NULL;
    cd->daynotes.number_of_items = 0;

    XtFree ((char *)default_icons);



}

#define EH_BUFSIZ 256

static int ErrorHandler(dpy, event)
    Display *dpy;
    XErrorEvent *event;
{
    char bufA[EH_BUFSIZ];
    char msgA[EH_BUFSIZ];
    char numberA[32];
    char *mtype = "XlibMessage";

    /* Ignore soft errors. */

    if (
	/* Attempted to assign input focus to a window, but that window was 
	 * iconified at the time the server processed the request. */

        ((event->error_code == BadMatch) &&
         (event->request_code == X_SetInputFocus)) ||

	/* Attempted to select for ButtonPress on the root window, but
	 * another application already has selected for it. */

        ((event->error_code == BadAccess) &&
         (event->request_code == X_ChangeWindowAttributes))) return (1);

/*  PGLF 8/28/90 I think that this is probably overkill and we should	    */
/*  probably just let the server report things it wants to report
    XGetErrorText (dpy, event->error_code, bufA, EH_BUFSIZ);
    XGetErrorDatabaseText (dpy, mtype, "XError", "X Error", msgA, EH_BUFSIZ);
    fprintf (stderr, "%s:  %s\n  ", msgA, bufA);
    XGetErrorDatabaseText (dpy, mtype, "MajorCode", "Request Major code %d", 
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, event->request_code);
    sprintf (numberA, "%d", event->request_code);
    XGetErrorDatabaseText (dpy, "XRequest", numberA, "", bufA, EH_BUFSIZ);
    fprintf (stderr, " (%s)", bufA);
    fputs ("\n  ", stderr);
    XGetErrorDatabaseText (dpy, mtype, "MinorCode", "Request Minor code", 
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, event->minor_code);
    fputs ("\n  ", stderr);
    XGetErrorDatabaseText (dpy, mtype, "ResourceID", "ResourceID 0x%x",
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, event->resourceid);
    fputs ("\n  ", stderr);
    XGetErrorDatabaseText (dpy, mtype, "ErrorSerial", "Error Serial #%d", 
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, event->serial);
    fputs ("\n  ", stderr);
    XGetErrorDatabaseText (dpy, mtype, "CurrentSerial", "Current Serial #%d",
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, dpy->request);
    fputs ("\n", stderr);

*/

    return (1);
}

#define		StrOrNull(str)	    (str? str: Nullstr)
#define		IsNullStr(str)	    (str == NullStr)
#define		Ngetenv(str)	    strOrNull(getenv(str))
    char	Nullstr[1];

char	*
strOrNull(str)
char	*str;
{
    return StrOrNull(str);
}

/* db_filespec() returns the name of the calendar file for this particular  */
/* user.  On VMS this is constant, on non-VMS systems, will look in the env */
/* for an alternative file before pickign the login user's home directory   */
/* based .dxcalendar.dwc */
char	*
db_filespec
#if defined(_DWC_PROTO_)
	()
#else	/* no prototypes */
	()
#endif	/* prototype */
{
    char	*ret;
    char	buf[512];
    int		len;

    Nullstr[0] = '\0';

#if defined(VMS)
    ret = (char *) XtNewString ("DECW$CALENDAR_FILE");
#else
    /* if no HOME, looks in the root */
    if (!(ret = (char *)getenv("DXCALENDAR_FILE")))
    {
	sprintf(buf, "%s/.dxcalendar.dwc", Ngetenv("HOME") );
	ret = (char *)XtNewString(buf);
    }
    else
	ret = (char *)XtNewString(ret);
#endif /* VMS */


    return ret;
}

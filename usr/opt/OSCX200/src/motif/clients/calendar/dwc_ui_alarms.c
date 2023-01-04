/* dwc_ui_alarms.c */
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
**	Marios Cleovoulou, November-1987
**	Ken Cowan February-1989
**
**  ABSTRACT:
**
**	This module contains the support for alarm pop-ups
**
**--
*/

#include    "dwc_compat.h"

#include <time.h>
#include <stdio.h>
#include <assert.h>

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Intrinsic.h>	/* for stuff like Boolean */
#include <X11/Shell.h>		/* for XtNiconPixmap */
#include <Xm/BulletinBP.h>	/* FormP needs it */
#include <Xm/FormP.h>		/* for XmFormWidget */
#include <Xm/MwmUtil.h>		/* for mwm_functions */
#include <Xm/Protocols.h>	/* for XmAddWmProtocolCallback */
#include <Xm/AtomMgr.h>		/* for XmInternAtom */
#include <DXm/DECspecific.h>
#include <dwi18n_lib.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include    "dwc_db_public_structures.h"
#include    "dwc_db_private_include.h"
#include    "dwc_db_public_include.h"

#include    "dwc_ui_calendar.h"
#include    "dwc_ui_alarms.h"	
#include    "dwc_ui_dateformat.h"
#include    "dwc_ui_catchall.h"		    /* for DWC$DRM code */
#include    "dwc_ui_misc.h"		    /* for MISCDayClock */
#include    "dwc_ui_monthwidget.h"	    /* for MWMARKUP stuff */
#include    "dwc_ui_datefunctions.h"
#include    "dwc_ui_icons.h"
#include    "dwc_ui_iconboxwidget.h"
#include    "dwc_ui_day.h"
#include    "dwc_ui_errorboxes.h"

#undef _XmGetFlagsBit
#define _XmGetFlagsBit(field, bit) \
      ((field[ ((unsigned int)bit >> 3) ]) & (1 << ((unsigned int)bit & 0x07)))

extern Pixmap	medium_pixmaps[];	/* defined in dwc_ui_misc.c */

static void ALARM_WIDGET_CREATE PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cb));

static void ALARM_PB_FIXUP PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cb));

static void alarm_timer_tick PROTOTYPE ((
	CalendarDisplay	cd));

static void ALARM_DISMISS PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct *cbs));

XmFormWidget find_top_adb PROTOTYPE ((
	Widget	w));

static Boolean alarms_set_title PROTOTYPE ((
	CalendarDisplay	cd,
	int		index));

static void alarms_set_times PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	index,
	Cardinal	start_time,
	Cardinal	duration,
	Boolean		ampm));

void ALARMSSetupNextAlarm
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Boolean		use_previous)
#else	/* no prototypes */
	(cd, use_previous)
	CalendarDisplay	cd;
	Boolean		use_previous;
#endif	/* prototype */
    {
    Cardinal		dsbot;
    unsigned short int	now;
    int			now_time;
    int			timer_time;
    unsigned int        alarm_day;
    unsigned short int	alarm_time;
    unsigned int        from_day;
    unsigned short int	from_time;
    int			i;
    struct tm		*local_time;

    if (cd->alarm_id != 0) {
	XtRemoveTimeOut (cd->alarm_id);
	cd->alarm_id = 0;
    }

    /*	  
    **  Update the internal time
    */	  
    MISCUpdateTime();

    /*	  
    **  Get the local internal time
    */	  
    MISCGetTime(&local_time);

    dsbot = DATEFUNCDaysSinceBeginOfTime (local_time->tm_mday,
					  local_time->tm_mon,
					  local_time->tm_year);
    now   = (local_time->tm_hour * 60) + local_time->tm_min;

    if (use_previous) {
	from_day  = cd->alarm_day;
	from_time = cd->alarm_time + 1;
    } else {
	from_day  = 0;
    }

    if (from_day == 0) {
	from_day  = dsbot;
	from_time = now + 1;
    }

    if (from_time >= 24 * 60) {
	from_day  = from_day + 1;
	from_time = 0;
    }

    if (DWC$DB_Get_next_alarm_time
	(cd->cab, from_day, from_time, &alarm_day, &alarm_time) ==
	    DWC$k_db_normal)
    {

	timer_time = ((alarm_day * 24 * 60) + alarm_time) -
		     ((dsbot     * 24 * 60) + now);
	timer_time = (timer_time * 60) - local_time->tm_sec;
	timer_time = MAX (1, timer_time);

	cd->alarm_id = XtAppAddTimeOut
	(
	    CALGetAppContext(),
	    timer_time * 1000,
	    (XtTimerCallbackProc)alarm_timer_tick,
	    cd
	);

	cd->alarm_day  = alarm_day;
	cd->alarm_time = alarm_time;
    }
    else
    {
	cd->alarm_day  = 0;
	cd->alarm_time = 0;
    }

}

static void ALARM_DISMISS
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag,
	XmAnyCallbackStruct *cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget	w;
	caddr_t	tag;
	XmAnyCallbackStruct *cbs;
#endif	/* prototype */
    {
    Position		x, y;
    Dimension		width, height;
    Arg			arglist [10];
    Cardinal		ac;
    XmFormWidget	adb;
    AlarmOut		alarm_out;
    CalendarDisplay	cd;
    Widget		shell;

    adb = find_top_adb( w );
    alarm_out = (AlarmOut)adb->manager.user_data;
    cd = alarm_out->cd;

    if (cd->alarms_out [0]->popup == alarm_out->popup) {
	ac = 0;
	XtSetArg (arglist [ac], XtNx, &x);  ac++;
	XtSetArg (arglist [ac], XtNy, &y);  ac++;
	XtGetValues (XtParent (cd->alarms_out [0]->popup), arglist, ac);

	ac = 0;
	XtSetArg (arglist [ac], XtNwidth, &width); ac++;
	XtSetArg (arglist [ac], XtNheight, &height); ac++;
	XtGetValues (cd->alarms_out [0]->popup, arglist, ac);

	cd->profile.alarm_width  =
	    (Cardinal)((width * 100) / cd->screen_font_size);
	cd->profile.alarm_height =
	    (Cardinal)((height * 100) / cd->screen_font_size);

	MISCGetScreenFractionalPosition
	    (cd,
	    (Dimension)cd->profile.alarm_width,
	    (Dimension)cd->profile.alarm_height,
	    x, y, &cd->profile.alarm_x, &cd->profile.alarm_y);
    }
		
    alarm_out->popup = NULL;

    XtUnmanageChild((Widget)adb);
    XtUnmapWidget(XtParent(adb));
    XtDestroyWidget(XtParent(adb));

}


static void alarm_timer_tick
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {
    Cardinal		alarm_day;
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
    int			was_repeated;
    XmString		entry_text;
    int			text_length;
    int			text_class;
    dtb			date_time;
    dtb			alarm_date_time;
    XmString		cs_text;
    char		*slot_text;
    Widget		form_box;
    Widget		w;
    Cardinal		i;
    int			next_free;
    Boolean		found;
    Boolean		now_flag;
    Arg			arglist [10];
    Cardinal		ac;
    int			entry_flags;
    int			status;
    Cardinal		num_icons;
    unsigned char	*icons;
    unsigned char	icon;
    Position		x;
    Position		y;
    Dimension		width;
    Dimension		height;
    int			alarm_index;
    int			mwm_functions;
    Atom		DwcWmTakeFocusAtom;
    Boolean		change_height = False;

    static MrmRegisterArg regvec[] =
    {
	/*
	**  Identifiers
	*/


	{"alarm_out",			(caddr_t)0 },
	{"sloteditor_off_pixmap_array", (caddr_t)medium_pixmaps},	
	/*
	**
	*/
	{"alarm_widget_create",		(caddr_t)ALARM_WIDGET_CREATE},
	{"alarm_dismiss",		(caddr_t)ALARM_DISMISS},
	{"alarm_pb_fixup",		(caddr_t)ALARM_PB_FIXUP}
    };
    MrmCount		regnum = XtNumber(regvec);
    MrmType		class;
    long		byte_count, cvt_status;

    cd->alarm_id = 0;

    /*
    ** Make sure the medium pixmaps are loaded.
    */
    MISCLoadPixmapArrays (cd->mainwid, 1);

    /*	  
    **  Let's start getting alarms
    */	  
    while (DWC$DB_Get_next_alarm_id (cd->cab, &alarm_day, &item_id) ==
	   DWC$k_db_normal)
    {
        /*	  
        **  We've found an alarm
        */	  
        found = FALSE;
	if (cd->alarms_out_number == 0)
	{

            /*	  
            **  This is the first alarm we've got as far as the UI is concerned.
	    **	Let's allocate space to keep track of it.
            */	  
            cd->alarms_out_number = 1;
	    next_free = 0;
	    cd->alarms_out = (AlarmOut *) XtMalloc (sizeof (AlarmOut));
	    cd->alarms_out [0] = (AlarmOut) XtMalloc (sizeof (AlarmOutRecord));

	}
	else
	{

            /*	  
            **  We've already setup for some alarms before. Indicate that we
	    **	don't know what the next free slot is.
            */	  
            next_free  = - 1;

	    /* run through our alarmoutrecords */
	    for (i = 0;  i < cd->alarms_out_number; i++)
	    {
		if (cd->alarms_out [i]->popup == NULL)
		{
		    /* we found a free one slot, since there is no alarm    */
		    /* entry widget associated with it			    */
		    if (next_free == -1)
		    {
                        /*	  
			**  We want to keep track of the first free one we find
			**  so we can reuse it if we get to the end of the array
			**  and haven't found a match.
			*/	  
                        next_free = i;
		    }
		}
		else
		{
                    /*	  
                    **  This alarmoutrecord has a alarm entry widget associated
		    **	with it. Check to see if 1) the item id of the alarm
		    **	matches the one we got from the database and 2) if the
		    **	alarm from the database was a repeat one, see if it is
		    **	for the right day (since repeat alarms have a negative
		    **	number that is only unique to the day)
                    */	  
                    if ((cd->alarms_out [i]->item_id == item_id) &&
		        ((cd->alarms_out [i]->day    == alarm_day) ||
		         (item_id < 0)))
		    {
		        /*	  
                        **  This alarmrecord matches the db alarm we just got.
			**  Set up some variables and pop down the Alarm entry
                        */	  
                        alarm_day  = cd->alarms_out [i]->day;
			start_time = cd->alarms_out [i]->time;
			form_box = cd->alarms_out [i]->popup;
			alarm_index = i;
			XtUnmanageChild (form_box);
			XtUnmapWidget (XtParent(form_box));
			found = TRUE;
                        /*	  
                        **  Break out of the for loop since we found the alarm
			**  we were looking for.
                        */	  
                        break;
		    } /* if that checks to see if alarmrec matches db alarm */
		} /* if checking if alarmrec has a popup widget */
	    } /* for loop running through alarmrec records */
	} /* if checking to see if we had any alarmrec records */

        /*	  
        **  Okay, we didn't find an alarm which matched the db alarm in our list
	**  of alarmrecords, so we're going to have to build one.
        */	  
	if (!found)
	{
	    found = TRUE;
	    if (next_free < 0)
	    {
		/*	  
		**  We do have alarmrecords but none of them were free so we
		**  better allocate some more space and tag it on to the end.
		*/	  
		next_free = cd->alarms_out_number;
		cd->alarms_out_number++;

                /*	  
                **  Increase the size of the list of pointers to add another and
		**  grab space for the new alarmrecord we'll be adding.
                */	  
                cd->alarms_out = (AlarmOut *) XtRealloc
		(
		    (char *)cd->alarms_out,
		    sizeof(AlarmOut)*cd->alarms_out_number
		);
		cd->alarms_out [next_free] = (AlarmOut) XtMalloc
		    (sizeof(AlarmOutRecord));
	    } /* end of if where we had no free alarmrecords */

	    /*	  
	    ** Get all the details from the db about the alarm we're interested
	    ** in.
	    */
	    status = DWCDB_GetSpecificRItem
	    (
		cd->cab, alarm_day, item_id,
		&repeat_start_day, &repeat_start_min,
		&start_time, &duration_days, &duration, 
		&alarms_number, &alarms_times, &entry_flags,
		&entry_text, &text_class,
		&num_icons, &icons,
		&repeat_p1, &repeat_p2, &repeat_p3,
		&end_day, &end_min);

	    if (status != DWC$k_db_normal)
	    {
		ERRORReportErrno
		    (cd->ads->root, "ErrorDatabaseError", NULL, NULL);
		found = FALSE;
	    }
	    else
	    {
		Widget shell;

		shell = XtAppCreateShell
		(
		    "entryAlarm",
		    CalendarTopClassName,
		    applicationShellWidgetClass,
		    XtDisplay(cd->ads->root),
		    NULL, 0
		);

		regvec[0].value = (caddr_t)cd->alarms_out [next_free];
		MrmRegisterNames (regvec, regnum);

		status = MrmFetchWidget
		(
		    cd->ads->hierarchy,
		    "EntryAlarm",
		    shell,
		    &form_box,
		    &class
		);

		if (status != MrmSUCCESS)
		    DWC$UI_Catchall(DWC$DRM_ALARM, status, 0);

		if (next_free != 0)
		{
		    /*	  
		    **	There is at least one existing alarm. If there is a
		    **	widget associated with the first alarm then get
		    **	pertinent info from it.
		    */	  
		    if (cd->alarms_out [0]->popup != NULL)
		    {
			/*	  
			**  Get x an y from the dialog parent of the alarm.
			*/
			XtVaGetValues
			(
			    XtParent(cd->alarms_out [0]->popup),
			    XtNx, &x,
			    XtNy, &y,
			    NULL
			);

                        XtVaGetValues
			(
			    cd->alarms_out [0]->popup,
			    XtNwidth, &width,
			    XtNheight, &height,
			    NULL
			);

			cd->profile.alarm_width =
			    (Cardinal)((width * 100) / cd->screen_font_size); 
			cd->profile.alarm_height =
			    (Cardinal)((height * 100) /	cd->screen_font_size);

			MISCGetScreenFractionalPosition
			(
			    cd,
			    (Dimension)cd->profile.alarm_width,
			    (Dimension)cd->profile.alarm_height,
			    x, y, &cd->profile.alarm_x, &cd->profile.alarm_y
			);
		    }
		}

		MISCGetScreenPixelPosition
		(
		    cd,
		    cd->profile.alarm_x,
		    cd->profile.alarm_y,
		    (Dimension)cd->profile.alarm_width,
		    (Dimension)cd->profile.alarm_height,
		    &width,
		    &height,
		    &x,
		    &y
		);

		XtVaSetValues
		(
		    form_box,
		    XmNnoResize, True,
		    XtNx, x + (25 * next_free),
		    XtNy, y + (25 * next_free),
		    XtNwidth,  width,
		    XtNheight, height,
		    NULL
		);
		change_height=True;


		/*	  
		**  BulletinB doesn't allow minimize if it is a dialog
		**	shell, but we want to be able to iconify alarms other
		**	than the first.
		**  We also want to avoid having the box GRAB focus.
		*/	  
		DwcWmTakeFocusAtom = XmInternAtom
		    (XtDisplay(form_box), "WM_TAKE_FOCUS", False);
		XmDeactivateWMProtocol (XtParent(form_box), DwcWmTakeFocusAtom);

		mwm_functions =
		(
		    MWM_FUNC_RESIZE |
		    MWM_FUNC_MINIMIZE |
		    MWM_FUNC_MOVE |
		    MWM_FUNC_CLOSE
		);
		XtVaSetValues
		(
		    XtParent(form_box),
		    XmNmwmFunctions, mwm_functions,
		    XmNinput, False,
		    XtNx, x + (25 * next_free),
		    XtNy, y + (25 * next_free),
		    XtNwidth,  width,
		    XtNheight, height,
		    NULL
		);

		/*
		** Make the CLOSE item in the window manager menu call the
		** CANCEL callback.
		*/
		MISCAddProtocols
		(
		    XtParent(form_box),
		    (XtCallbackProc) ALARM_DISMISS,
		    NULL
		);

                /*	  
		**  Set the date label of the alarm
		*/	  
                DATEFUNCDateForDayNumber
		(
		    alarm_day,
		    &date_time.day,
		    &date_time.month,
		    &date_time.year
		);
		date_time.weekday = DATEFUNCDayOfWeek
		    (date_time.day, date_time.month, date_time.year);

		cs_text = DATEFORMATTimeToCS (dwc_k_alarm_format, &date_time);
		XtVaSetValues
		(
		    cd->alarms_out[next_free]->date_lb,
		    XmNlabelString, cs_text,
		    NULL
		);
		XmStringFree (cs_text);

                /*	  
		**  Set the proper icons in the alarm body
		*/
#if 0
                DAYGetIconsFromText
		(
		    text_class,
		    &entry_text,
		    &text_length,
		    &num_icons,
		    &icons
		);
#endif
		DwcIbwSetSelectedIcons
		    (cd->alarms_out[next_free]->alarm_ibw, icons, num_icons);
#if 0
		XtFree ((char *)icons);
#endif


                /*	  
		**  Set the start and finish time labels on the alarm
		*/	  
                alarms_set_times
		(
		    cd,
		    next_free,
		    start_time,
		    duration,
		    cd->profile.time_am_pm
		);

		XtVaSetValues
		(
		    cd->alarms_out [next_free]->alarm_tw,
		    XmNvalue, entry_text,
		    NULL
		);

                /*	  
                **  Manage the buttons, labels, etc of the form box.
                */	  
                XtManageChildren
		    (DWC_CHILDREN(form_box), DWC_NUM_CHILDREN(form_box));

		cd->alarms_out [next_free]->alarm_day = cd->alarm_day;
		cd->alarms_out [next_free]->alarm_time = cd->alarm_time;
		cd->alarms_out [next_free]->day      = alarm_day;
		cd->alarms_out [next_free]->time     = start_time;
		cd->alarms_out [next_free]->duration = duration;
		cd->alarms_out [next_free]->item_id  = item_id;
		cd->alarms_out [next_free]->popup    = form_box;
		cd->alarms_out [next_free]->cd       = cd;
		alarm_index = next_free;
	    } /* end of if where we get specific info from db */

	} /* end of if !found */

	if (found)
	{
            /*	  
	    **  Make sure we update the alarm time if we're reusing one
	    */	  
            cd->alarms_out [alarm_index]->alarm_time = cd->alarm_time;

            /*	  
	    **  Go set the alarm title and the icon name
	    */
            now_flag = alarms_set_title (cd, alarm_index);

	    XtManageChild (form_box);
	    if (!XtIsRealized(XtParent(form_box)))
		XtRealizeWidget(XtParent(form_box));
	    else
		XtMapWidget(XtParent(form_box));

	    /*	  
	    **  Set the iconpixmap on the alarm
	    */
	    ICONSSetIconifyIcon (form_box, 1);

	    ICONSSetIconBoxIcon
	    (
		XtParent(form_box),
		NULL,
		NULL,
		cd->icon_day_fontlist,
		cd->icon_month_fontlist,
		False,
		False,
		True
	    );

	    XBell (XtDisplay (form_box), 0);
	    if (now_flag)
	    {
		XBell (XtDisplay (form_box), 50);
		XBell (XtDisplay (form_box), -50);
	    }
	    XBell (XtDisplay (form_box), 0);

	    /*
	    ** Make things turn on.
	    */
	    XResetScreenSaver (XtDisplay(form_box));

	} /* end of if found */
    } /* done with while through the db */

    ALARMSSetupNextAlarm (cd, TRUE);

}

static Boolean alarms_set_title
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	int		index)
#else	/* no prototypes */
	(cd, index)
	CalendarDisplay	cd;
	int		index;
#endif	/* prototype */
    {
    dtb		date_time;
    dtb		alarm_date_time;
    Boolean	now_flag;
    char	*day_text, *time_text, *title_text, *text;
    int		i;
    Cardinal	ac;
    Arg		arglist[ 10 ];
    Widget	dialog_box = cd->alarms_out[index]->popup;
    int		alarm_day = cd->alarms_out[index]->day;
    int		start_time = cd->alarms_out[index]->time;
    int		title_text_len, icon_text_len, day_text_len, time_text_len;
    XmString	xm_title,  xm_icon;
    long	byte_count, cvt_status;


    DATEFUNCDateForDayNumber (alarm_day, &date_time.day, &date_time.month,
				    &date_time.year);
    date_time.weekday = DATEFUNCDayOfWeek (date_time.day, date_time.month,
				     date_time.year);

    date_time.hour   = start_time / 60;
    date_time.minute = start_time % 60;

    now_flag = FALSE;
    if (alarm_day == cd->alarms_out[index]->alarm_day)
    {
	if (cd->alarms_out[index]->alarm_time == start_time)
	{
	    now_flag = TRUE;
	    day_text = DATEFORMATTimeToText
		(dwc_k_alarm_icon_now_format, &date_time);
	}
	else
	{
	    day_text = NULL;
	}
    }
    else
    {
	if (alarm_day == cd->alarms_out[index]->alarm_day + 1)
	{
	    day_text = DATEFORMATTimeToText
		(dwc_k_alarm_icon_tmrow_format, &date_time);
	}
	else
	{
	    day_text = DATEFORMATTimeToText
		(dwc_k_alarm_icon_date_format, &date_time);
	}
    }

    alarm_date_time.hour   = cd->alarms_out[index]->alarm_time / 60;
    alarm_date_time.minute = cd->alarms_out[index]->alarm_time % 60;

    if (cd->profile.time_am_pm)
    {
	time_text  = DATEFORMATTimeToText
	    (dwc_k_icon_time_ampm_format, &date_time);
	title_text = DATEFORMATTimeToText
	    (dwc_k_alarm_title_ampm_format, &alarm_date_time);
    }
    else
    {
	time_text  = DATEFORMATTimeToText
	    (dwc_k_icon_time_format, &date_time);
	title_text = DATEFORMATTimeToText
	    (dwc_k_alarm_title_format, &alarm_date_time);
    }

    if (cd->profile.icon_show_text)
    {
	title_text_len = strlen (title_text);
	icon_text_len = strlen (cd->profile.icon_text);
	i = title_text_len + icon_text_len + 1;
	title_text = XtRealloc (title_text, i);
	memcpy
	(
	    &title_text[title_text_len],
	    cd->profile.icon_text,
	    icon_text_len + 1
	);
    }
    else
    {
	title_text_len = strlen (title_text);
	icon_text_len = strlen (cd->filename);
	i = title_text_len + icon_text_len + 1;
	title_text = XtRealloc (title_text, i);
	memcpy (&title_text[title_text_len], cd->filename, icon_text_len + 1);
    }

    if (day_text == NULL)
    {
	text = time_text;
    }
    else
    {
	day_text_len = strlen (day_text);
	time_text_len = strlen (time_text);
	i = day_text_len + 1 + time_text_len + 1;
	text = XtMalloc (i);
	memcpy (text, day_text, day_text_len);
	if (cd->profile.icon_nl_after_day)
	{
	    text[day_text_len] = '\n';
	}
	else
	{
	    text[day_text_len] = ' ';
	}
	memcpy (&text[day_text_len+1], time_text, time_text_len+1);
	XtFree (day_text);
	XtFree (time_text);
    }

    xm_title = DXmCvtFCtoCS(title_text, &byte_count, &cvt_status);
    xm_icon  = DXmCvtFCtoCS(text, &byte_count, &cvt_status);
    DWI18n_SetIconName(XtParent(dialog_box), xm_icon);
    DWI18n_SetTitle(XtParent(dialog_box), xm_title);

    XmStringFree(xm_title);
    XmStringFree(xm_icon);

    XtFree (title_text);
    XtFree (text);

    return (now_flag);

}

/* set the alarm start and finish time labels */
static void alarms_set_times
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Cardinal	index,
	Cardinal	start_time,
	Cardinal	duration,
	Boolean		ampm)
#else	/* no prototypes */
	(cd, index, start_time, duration, ampm)
	CalendarDisplay	cd;
	Cardinal	index;
	Cardinal	start_time;
	Cardinal	duration;
	Boolean		ampm;
#endif	/* prototype */
    {    
    dtb			date_time;
    XmString		cs_text;
    Arg			arglist [1];
    

    date_time.hour   = start_time / 60;
    date_time.minute = start_time % 60;
    if (ampm) {
	cs_text = (XmString)DATEFORMATTimeToCS(dwc_k_alarm_from_ampm_format, 
						&date_time);
    } else {
	cs_text = (XmString)DATEFORMATTimeToCS(dwc_k_alarm_from_format,
						&date_time);
    }
    XtSetArg(arglist[0], XmNlabelString, cs_text);
    XtSetValues( cd->alarms_out [index]->start_lb, arglist, 1);
    XmStringFree(cs_text);

    date_time.hour   = (start_time + duration) / 60;
    date_time.minute = (start_time + duration) % 60;
    if (ampm) {
	cs_text = DATEFORMATTimeToCS(dwc_k_alarm_to_ampm_format, &date_time);
    } else {
	cs_text = DATEFORMATTimeToCS(dwc_k_alarm_to_format, &date_time);
    }
    XtSetArg(arglist[0], XmNlabelString, cs_text);
    XtSetValues( cd->alarms_out [index]->finish_lb, arglist, 1);
    XmStringFree(cs_text);

}

void
ALARMSSetAlarmsDirection
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Boolean		directionRtoL)
#else	/* no prototypes */
	(cd, directionRtoL)
	CalendarDisplay	cd;
	Boolean		directionRtoL;
#endif	/* prototype */
    {
    Cardinal		i;
    Arg			argltxt [10];
    Cardinal		at;

    at = 0;
    XtSetArg(argltxt[at], XmNscrollLeftSide, directionRtoL); at++;

    for (i = 0;  i < cd->alarms_out_number; i++) {
	if (cd->alarms_out [i]->popup != NULL) {
	    XtSetValues (cd->alarms_out [i]->alarm_tw, argltxt, at);
	}
    }

}

void ALARMSSetAlarmsAmPm
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Boolean		ampm)
#else	/* no prototypes */
	(cd, ampm)
	CalendarDisplay	cd;
	Boolean		ampm;
#endif	/* prototype */
    {
    Cardinal		i;

    for (i = 0;  i < cd->alarms_out_number; i++) {
	if (cd->alarms_out [i]->popup != NULL) {
	    alarms_set_times(cd, i, cd->alarms_out [i]->time,
			   cd->alarms_out [i]->duration, ampm);
	    alarms_set_title(cd, i );
	}
    }

}

void ALARMSClockTimerTick
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    struct tm	    *local_time;

    /*
    **	Get time and adjust to our standards (!).  Remove any old timer and add
    **	a new timeout to occur at the turn of the next minute.
    */
    
    if (cd->clock_id != 0)
    {
	XtRemoveTimeOut (cd->clock_id);
	cd->clock_id = 0;
    }

    /*	  
    **  Update the internal time
    */	  
    MISCUpdateTime();

    /*	  
    **  Get the internal time
    */	  
    MISCGetTime(&local_time);

    cd->clock_id = XtAppAddTimeOut
    (
	CALGetAppContext(),
	(60 - local_time->tm_sec) * 1000,
	(XtTimerCallbackProc)ALARMSClockTimerTick,
	cd
    );

    ALARMSUpdateTimeDisplay( cd );

    return;
}

void ALARMSUpdateTimeDisplay
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    dtb		    date_time;
    Arg		    arglist[1];
    XmString	    cs_text;
    Cardinal	    year, month, day;    
    unsigned char   markup;
    unsigned char   style;
    Cardinal	    dsbot;
    struct tm	    *local_time;

    /*
    **  Update the current time display.  Take care of AM/PM setting. First
    **	we'll need to get the internal time
    */
    MISCGetTime(&local_time);

    date_time.day     = local_time->tm_mday;
    date_time.hour    = local_time->tm_hour;
    date_time.minute  = local_time->tm_min;
    date_time.weekday = DATEFUNCDayOfWeek
	(date_time.day, local_time->tm_mon, local_time->tm_year);

    if (cd->pb_day_time != NULL)
    {
	if (cd->profile.time_am_pm)
	{
	    cs_text = DATEFORMATTimeToCS(dwc_k_day_pb_ampm_format, &date_time);
	}
	else
	{
	    cs_text = DATEFORMATTimeToCS(dwc_k_day_pb_format, &date_time);
	}

	XtSetArg (arglist[0], XmNlabelString, cs_text);
	XtSetValues (cd->pb_day_time, arglist, 1);
	XmStringFree (cs_text);
    }
       
    if (cd->profile.icon_show_time)
    {
	ICONSSetIconTime
	(
	    cd->toplevel,
	    &date_time,
	    cd->profile.icon_show_text,
	    cd->profile.icon_text,
	    FALSE,
	    cd->profile.icon_show_day,
	    cd->profile.icon_full_day,
	    FALSE,
	    cd->profile.icon_show_time,
	    cd->profile.time_am_pm
	);
    }

    /*
    **  If we ticked into a new day then update past day and current day markers
    **	on the month displays.
    **	?????????? In here we should also change the displayed day if we are on
    **	a day display, it's the (old) current day AND we are iconized.
    */
    
    if (local_time->tm_mday != cd->last_timer_day)
    {
	if (! cd->profile.icon_show_time)
	{
	    ICONSSetIconTime
	    (
		cd->toplevel,
		&date_time,
		cd->profile.icon_show_text,
		cd->profile.icon_text,
		cd->profile.icon_nl_after_text,
		cd->profile.icon_show_day,
		cd->profile.icon_full_day,
		cd->profile.icon_nl_after_day,
		cd->profile.icon_show_time,
		cd->profile.time_am_pm
	    );
	}
	date_time.month = local_time->tm_mon;

	ICONSSetIconBoxIcon
	(
	    cd->toplevel,
	    NULL,
	    &date_time,
	    cd->icon_day_fontlist,
	    cd->icon_month_fontlist,
	    False,
	    False,
	    cd->mapped
	);

	ALARMSSetupNextAlarm (cd, FALSE);

	dsbot = DATEFUNCDaysSinceBeginOfTime 
	    (local_time->tm_mday, local_time->tm_mon, local_time->tm_year);

	DATEFUNCDateForDayNumber
	    (dsbot - 1, (int *)&day, (int *)&month, (int *)&year);

	MWGetDayStyleMarkup
	    (cd->month_display, day, month, year, &markup, &style);
	MWSetDayStyleMarkup
	(
	    cd->month_display,
	    day,
	    month,
	    year,
	    markup,
	    (MWSTYLE_CROSS_THRU | MWSTYLE_SLASH_THRU)
	);

	MWGetDayStyleMarkup	
	(
	    cd->month_display,
	    local_time->tm_mday,
	    local_time->tm_mon,
	    local_time->tm_year,
	    &markup,
	    &style
	);
	MWSetDayStyleMarkup
	(
	    cd->month_display,
	    local_time->tm_mday,
	    local_time->tm_mon,
	    local_time->tm_year,
	    markup,
	    MWSTYLE_SQUARE
	);

	cd->last_timer_day = local_time->tm_mday;
    }

    /*	  
    **  If autoscrolling is to be on, we're showing a day and we don't have any
    **	open entries then go update the clock and move the timeslots.
    */	  
    if (cd->auto_scroll_day && (cd->showing == show_day))
    {
	if ((DSWGetOpenEntry ((DayslotsWidget) cd->dayslots.widget) == NULL) &&
	    (DSWGetOpenEntry ((DayslotsWidget) cd->daynotes.widget) == NULL))
	{
	    MISCDayClock (NULL, (caddr_t) cd, NULL);
	}
    }

    return;
}

static int offset_table [] =
{
    XtOffset( AlarmOut, date_lb ),
    XtOffset( AlarmOut, start_lb ),
    XtOffset( AlarmOut, finish_lb ),
    XtOffset( AlarmOut, alarm_tw ),
    XtOffset( AlarmOut, alarm_ibw ),
};

static void ALARM_WIDGET_CREATE
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cb)
#else	/* no prototypes */
	(w, tag, cb)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cb;
#endif	/* prototype */
{
    int		    status;
    int		    index = (int)*tag;
    AlarmOut	    alarm_out;
    Widget	    *wp;
    XmFormWidget    adb;


    /*
    ** Assert that the offset table has enough stuff in it
    */
    assert (XtNumber(offset_table) == K_ALARM_WIDGET_COUNT);

    adb = find_top_adb(w);

    alarm_out = (AlarmOut)adb->manager.user_data;

    wp = (Widget *)((char *)alarm_out + offset_table[ index ]);
    *wp = w;

    return;
}

XmFormWidget find_top_adb
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    Widget adb = NULL;
    Widget temp;
    Widget  *children;

    temp = w;

    if (XtIsShell(temp))
    {
	children = DXmChildren ((CompositeWidget)temp);
	temp = children[0];
	return ((XmFormWidget)temp);
    }

    do 
    {
	if (XmIsForm(temp))
	    adb = temp;
	temp = XtParent( temp );
    }
    while ( temp != 0 );

    return ( (XmFormWidget)adb );
}

static void ALARM_PB_FIXUP
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cb)
#else	/* no prototypes */
	(w, tag, cb)
	Widget			w;
	caddr_t			*tag;
	XmAnyCallbackStruct	*cb;
#endif	/* prototype */
{
    Dimension	width;
    Position	offset;

    XtVaGetValues (w, XmNwidth, &width, NULL);
    offset = (Position)width;
    XtVaSetValues (w, XmNleftOffset, -((offset+1)/2), NULL);
}

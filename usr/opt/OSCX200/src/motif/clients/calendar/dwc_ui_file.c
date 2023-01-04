/* dwc_ui_file.c */
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
**
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
**	This is the module that deals with file menus
**
**-- 
*/

#include "dwc_compat.h"

#include <time.h>
#ifdef VMS
#include <unixio.h>
#endif

#include <errno.h>
#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <Xm/Text.h>			/* for XmTextGetString */
#include <DXm/DXmCSText.h>		/* for DXmCSTextGetString */
#include <Xm/AtomMgr.h>			/* for XmInternAtom */
#include <DXm/DECspecific.h>
#include <dwi18n_lib.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* for DWC$k_db_... */

#include "dwc_ui_datestructures.h"
#include "dwc_ui_calendar.h"
#include "dwc_ui_file.h"	
#include "dwc_ui_misc.h"		/* for k_include, MISCTestDayConditions... */
#include "dwc_ui_monthwidget.h"		/* for MWGetSelection */
#include "dwc_ui_custom.h"		/* for CUSTTellDBWorkDays */
#include "dwc_ui_clipboard.h"		/* for CLIPImportInterchange */
#include "dwc_ui_catchall.h"		/* for Dwc*ExitCode */
#include "dwc_ui_icons.h"
#include "dwc_ui_errorboxes.h"
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_help.h"
#if MEMEX
#include "dwc_ui_memex.h"
#endif

static void ErrorRenameFatal
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
    AllDisplays	    ads = (AllDisplays) tag;

    if (ads->number_of_calendars == 0)
    {
#ifdef DEBUG
	MISCFreeAllDRMValues ();
#endif
	exit (DwcCleanExitCode);
    }

}

void FILEDO_NAMEAS_OK
#ifdef _DWC_PROTO_
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
#ifndef VMS
    int			vaxc$errno;
#endif
    Time		time;
    char		applname [256];
    int			status;
    char		*filespec;
    int			l;
    CalendarDisplay	cd;
    int			appllen;
    XmString		xm_text;
    long		byte_count, cvt_status;

    status = MISCFindCalendarDisplay( &cd, w );

    time = MISCGetTimeFromAnyCBS (cbs);

    ICONSInactiveCursorRemove( w );

    xm_text = DXmCSTextGetString(cd->tw_nameas);
    if (xm_text)
    {
	filespec = DXmCvtCStoFC(xm_text, &byte_count, &cvt_status);
	XmStringFree(xm_text);
    }
    else
    {
	filespec = XtMalloc(1);
	filespec[0] = '\0';
	byte_count = 1;
    }

    status = DWC$DB_Rename_calendar (cd->cab, filespec);

    if (status == DWC$k_db_normal)
    {
	XtFree (cd->filespec);
	XtFree (cd->filename);
	cd->filename = NULL;

	cd->filespec = XtMalloc (byte_count);
	memcpy (cd->filespec, filespec, byte_count);

	status = DWC$DB_Get_calendar_name (cd->cab, &cd->filename);
	if (status != DWC$k_db_normal)
	{
	    DWC$DB_Get_error_codes (cd->cab, (int *)&errno, (int *)&vaxc$errno);
	    ERRORReportErrno (cd->db_nameas, "ErrorCantRename", NULL, NULL);
	    /*
	    ** since we know the size here, we can use memcpy
	    */
	    cd->filename = (char *) XtMalloc (byte_count);
	    memcpy (cd->filename, filespec, byte_count);
	}

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

	xm_text = DXmCvtFCtoCS (applname, &byte_count, &cvt_status);
	DWI18n_SetTitle (cd->toplevel, xm_text);
	XmStringFree(xm_text);

	XtFree (filespec);
	XtUnmanageChild (cd->db_nameas);

    }
    else
    {
	DWC$DB_Get_error_codes (cd->cab, (int *)&errno, (int *)&vaxc$errno);
	if (status == DWC$k_db_notren)
	{
	    ERRORReportErrno (cd->db_nameas, "ErrorCantRename", NULL, NULL);
	}
	else /* DWC$k_db_renfail */
	{
	    XtUnmanageChild (cd->db_nameas);
	    ERRORReportErrno
	    (
		cd->ads->root,
		"ErrorFatalRename",
		(XtCallbackProc) ErrorRenameFatal,
		(char *)cd->ads
	    );
	    cd->cab = NULL;
	    FILECloseCalendar (cd, time);
	}
    }
}

void FILEDO_NAMEAS_CANCEL
#ifdef _DWC_PROTO_
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
    CalendarDisplay	cd;
    int			status;

    status = MISCFindCalendarDisplay( &cd, w );

    ICONSInactiveCursorRemove( w );

    XtUnmanageChild (cd->db_nameas);

}

void FILEDO_NAMEAS
#ifdef _DWC_PROTO_
	(
	Widget			w,
	char			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	char			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay	cd;
    int			status;
    Time		time;
    WidgetList		children;
    int			num_children;

    time = MISCGetTimeFromAnyCBS( cbs );

    status = MISCFindCalendarDisplay( &cd, w );
    ICONSWaitCursorDisplay( cd->mainwid, cd->ads->wait_cursor );

    if (! DSWRequestCloseOpenEntry
	((DayslotsWidget) (cd->dayslots.widget), time))
    {
	return;
    }

    if (! DSWRequestCloseOpenEntry
	((DayslotsWidget) (cd->daynotes.widget), time))
    {
	return;
    }

    if (! cd->db_nameas_created)
    {
	/*
	** Get the widget.
	*/
	MISCFetchWidget (w, tag, cbs, False);

	/*
	** We can take down the wait cursor now.
	*/
	ICONSWaitCursorRemove(cd->mainwid);

	cd->db_nameas_created = TRUE;

	/*
	** Make the CLOSE item in the window manager menu call the CANCEL
	** callback.
	*/
	MISCAddProtocols
	(
	    XtParent(cd->db_nameas),
	    (XtCallbackProc) FILEDO_NAMEAS_CANCEL,
	    NULL
	);

	/*
	** Put up the inactive cursor everywhere else.
	*/
	ICONSInactiveCursorDisplay (cd->db_nameas, cd->ads->inactive_cursor);

	/*
	** Force all of the children to the same size.
	*/
	children = DXmChildren ((CompositeWidget)cd->fm_nameas_controls);
	num_children = DXmNumChildren ((CompositeWidget)cd->fm_nameas_controls);
	MISCWidthButtonsEqually (children, num_children);

	/*
	** When it really appears, put focus there.
	*/
	MISCFocusOnMap (XtParent(cd->db_nameas), NULL);

	/*
	** Get it on the screen.  Must happen before we fix up the attachments.
	*/
	XtManageChild (cd->db_nameas);

	/*
	** Fix up the attachments.
	*/
	MISCSpaceButtonsEqually (children, num_children);

	XtVaSetValues
	    (cd->fm_nameas_controls, XmNrightAttachment, XmATTACH_FORM, NULL);

    }
    else
    {
	/*
	** Let's make sure we have an inactive cursor again
	*/
	ICONSWaitCursorRemove(cd->mainwid);

	ICONSInactiveCursorDisplay( cd->db_nameas, cd->ads->inactive_cursor );

	/*
	** When it really appears, put focus there.
	*/
	MISCFocusOnMap (XtParent(cd->db_nameas), NULL);

	/*
	** Get it on the screen.
	*/
	XtManageChild (cd->db_nameas);

    }
}



static void ErrorDeleteFile
#ifdef _DWC_PROTO_
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
    AllDisplays	    ads = (AllDisplays) tag;

    if (ads->number_of_calendars == 0)
    {
#ifdef DEBUG
	MISCFreeAllDRMValues ();
#endif
	exit (DwcCleanExitCode);
    }

}

void FILEDO_CAUTION_DELETE_FILE
#ifdef _DWC_PROTO_
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
    Time	    time;
    Cardinal	    length;
    char	    *filespec;
    AllDisplays	    ads;
    int		    status;
    CalendarDisplay cd;

    status = MISCFindCalendarDisplay( &cd, w );

    time = MISCGetTimeFromAnyCBS (cbs);

    ICONSInactiveCursorRemove( w );

    if (cbs->reason != (int)XmCR_OK)
    {
	return;
    }
    
    XtUnmanageChild (w);
    
    length   = strlen (cd->filename);
    filespec = XtMalloc (length + 1);
    memcpy (filespec, cd->filename, length + 1);
    ads = cd->ads;

    FILECloseCalendar (cd, time);

#ifdef VMS
    status = delete (filespec);
#else
    status = unlink (filespec);
#endif

    if (status != 0)
    {
	ERRORReportErrno
	(
	    ads->root,
	    "ErrorDeleteFile",
	    (XtCallbackProc) ErrorDeleteFile,
	    (char *) ads
	);
    }
    else
    {
	if (ads->number_of_calendars == 0)
	{
#ifdef DEBUG
	    MISCFreeAllDRMValues ();
#endif
	    exit (DwcCleanExitCode);
	}
    }

    XtFree (filespec);

}

void FILEDO_DELETE
#ifdef _DWC_PROTO_
	(
	Widget			w,
	char			*tag,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget			w;
	char			*tag;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */
{
    CalendarDisplay cd;
    int			status;
    Time		time;

    time = MISCGetTimeFromAnyCBS( cbs );
    status = MISCFindCalendarDisplay( &cd, w );
    ICONSWaitCursorDisplay(cd->mainwid, cd->ads->wait_cursor);

    if (! DSWRequestCloseOpenEntry
	((DayslotsWidget) (cd->dayslots.widget), time))
    {
	return;
    }

    if (! DSWRequestCloseOpenEntry
	((DayslotsWidget) (cd->daynotes.widget), time))
    {
	return;
    }

    MISCFetchModal( w, tag, cbs);
    ICONSWaitCursorRemove(cd->mainwid);
}

void FILEBYE_BYE
#ifdef _DWC_PROTO_
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
    AllDisplays	    ads;
    CalendarDisplay cd;
    int			status;
    Time		time;
    Widget		mainwid;

    time = MISCGetTimeFromAnyCBS (cbs);

    status = MISCFindCalendarDisplay (&cd, w);

    mainwid = cd->mainwid;
    ads = cd->ads;

    ICONSWaitCursorDisplay (mainwid, cd->ads->wait_cursor);

    if (! DSWRequestCloseOpenEntry
	((DayslotsWidget) (cd->dayslots.widget), time))
    {
	return;
    }

    if (! DSWRequestCloseOpenEntry
	((DayslotsWidget) (cd->daynotes.widget), time))
    {
	return;
    }

    ICONSWaitCursorRemove(mainwid);

    /*
    ** Now go and save the calendars.
    */
    while (ads->number_of_calendars != 0)
    {
	FILECloseCalendar (ads->cds [0], time);
    }

#ifdef DEBUG
    MISCFreeAllDRMValues ();
#endif

    exit (DwcCleanExitCode);
        
}


void FILESAVE_YOURSELF
#ifdef _DWC_PROTO_
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
    AllDisplays	    ads;
    CalendarDisplay cd;
    int			status;
    Time		time;
    Widget		mainwid;
    Widget		toplevel;
    char		**argv;
    static char		*argv_const[]={"dxcalendar","FOO"};
    int			argc = 1;

    time = MISCGetTimeFromAnyCBS (cbs);

    status = MISCFindCalendarDisplay (&cd, w);

    toplevel = cd->toplevel;
    mainwid = cd->mainwid;
    ads = cd->ads;

    ICONSWaitCursorDisplay (mainwid, cd->ads->wait_cursor);

    if (! DSWRequestCloseOpenEntry
	((DayslotsWidget) (cd->dayslots.widget), time))
    {
	return;
    }

    if (! DSWRequestCloseOpenEntry
	((DayslotsWidget) (cd->daynotes.widget), time))
    {
	return;
    }

    /*
    ** Set the command to recover this.
    */
    if (XGetCommand (XtDisplay(toplevel), XtWindow(toplevel), &argv, &argc))
    {
	XSetCommand (XtDisplay(toplevel), XtWindow(toplevel), argv, argc);
	XFlush (XtDisplay(toplevel));
    }
    else
    {
	XSetCommand (XtDisplay(toplevel), XtWindow(toplevel), argv_const, 1);
	XFlush (XtDisplay(toplevel));
    }
    ICONSWaitCursorRemove(mainwid);

}

void FILEDO_CLOSE
#ifdef _DWC_PROTO_
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
    AllDisplays	    ads;
    Time		time;
    CalendarDisplay	cd;
    int			status;
    Widget		mainwid;

    /*
    ** Find the calendar.
    */
    status = MISCFindCalendarDisplay( &cd, w );

    /*
    ** Get the list of calendars.
    */
    ads = cd->ads;
    mainwid = cd->mainwid;

    /*
    ** Turn on the wait cursor.
    */
    ICONSWaitCursorDisplay(mainwid, cd->ads->wait_cursor);

    /*
    ** Destroy this calendar.
    */
    time = MISCGetTimeFromAnyCBS (cbs);

    FILECloseCalendar (cd, time);

    /*
    ** Turn off the wait cursor.
    */
    ICONSWaitCursorRemove(mainwid);        

    /*
    ** Just in case we get here from the close in the window menu, we need to
    ** see if this is the last one.
    */
    if (ads->number_of_calendars == 0)
    {
#ifdef DEBUG
	MISCFreeAllDRMValues ();
#endif
	exit (DwcCleanExitCode);
    }
}


void FILEDO_OPEN_FILE
#ifdef _DWC_PROTO_
	(
	Widget					w,
	int					*tag,
	XmFileSelectionBoxCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget					w;
	int					*tag;
	XmFileSelectionBoxCallbackStruct	*cbs;
#endif	/* prototype */
{
    char		*filespec;
    CalendarDisplay	cd,
			new_cd;
    int			status;
    Time                time;
    AllDisplays		ads;

    ICONSInactiveCursorRemove( w );

    time = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *) cbs);

    status = MISCFindCalendarDisplay( &cd, w );

    if (cbs->reason == (int)XmCR_CANCEL)
    {
	ICONSWaitCursorRemove (cd->mainwid);
	XtUnmanageChild (w);
	cd->delete_pending = FALSE;

	if (!XmProcessTraversal (cd->dayslots.widget, XmTRAVERSE_CURRENT))
	{
	    XmProcessTraversal (cd->daynotes.widget, XmTRAVERSE_CURRENT);
	}
    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */
	return;
    }
 
    filespec = MISCGetTextFromCS(cbs->value);
    if (filespec == NULL)
    {
        ERRORDisplayError (w, "ErrorFileOpenSpec");
	return;
    }

    cd->ads->filespec = NULL;
    
    ads = cd->ads;
    ads->requestor = w;

    ICONSWaitCursorDisplay (cd->mainwid, cd->ads->wait_cursor);

    if (*tag == k_open_replace )
    {
	cd->delete_pending = TRUE;
    }

    status = FILEOpenCalendar (ads, filespec, &new_cd);

}


static void ErrorCantOpen
#ifdef _DWC_PROTO_
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
    AllDisplays	    ads = (AllDisplays) tag;

    XtFree (ads->filespec);
    ads->filespec = NULL;

    if (ads->number_of_calendars == 0)
    {
#ifdef DEBUG
	MISCFreeAllDRMValues ();
#endif
	exit (DwcCleanExitCode);
    }
    
}

static void ErrorNeedNew
#ifdef _DWC_PROTO_
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
#ifndef VMS
    int		    vaxc$errno;
#endif
    Widget	    parent;
    Cardinal	    today;
    int		    i;
    AllDisplays	    ads = (AllDisplays) tag;
    struct tm	    *local_time;
    CalendarDisplay new_cd;

    if (ads->filespec == NULL)	/* if the filespec is null, this is the */
	return;			/* second time we're being called for */
				/*  this same box */

    if (cbs->reason == (int)XmCR_CANCEL)
    {
	XtFree (ads->filespec);
	ads->filespec = NULL;
	if (ads->number_of_calendars == 0)
	{
#ifdef DEBUG
	    MISCFreeAllDRMValues ();
#endif
	    exit (DwcCleanExitCode);
	}
	else
	{
	    XtUnmanageChild(w);

	    return;
	}
    }

    XtUnmanageChild(w);

    if (cbs->reason == (int)XmCR_OK)
    {
	MISCGetTime (&local_time);
	today = DATEFUNCDaysSinceBeginOfTime
	    (local_time->tm_mday, local_time->tm_mon, local_time->tm_year);

	if
	(
	    DWC$DB_Create_calendar
		(ads->filespec, today, (int *)&errno, (int *)&vaxc$errno) ==
	    DWC$k_db_failure
	)
	{
	    ERRORReportErrno
	    (
		ads->requestor,
		"ErrorCantCreate",
		(XtCallbackProc) ErrorCantOpen,
		(char *) ads
	    );
	    return;
	}
	FILEOpenCalendar (ads, ads->filespec, &new_cd);
    }

    return;
}

static void ErrorNeedUpgrade
#ifdef _DWC_PROTO_
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
    AllDisplays			ads = (AllDisplays)tag;
    CalendarDisplay		new_cd;
    struct DWC$db_access_block	*cab;
    int				status;
    Widget			parent;
#ifndef VMS
    int		vaxc$errno;
#endif

    if (cbs->reason == (int)XmCR_CANCEL)
    {
	XtFree (ads->filespec);
	ads->filespec = NULL;
	if (ads->number_of_calendars == 0)
	{
#ifdef DEBUG
	    MISCFreeAllDRMValues ();
#endif
	    exit (DwcCleanExitCode);
	}
	else
	{
	    XtUnmanageChild(w);

	    return;
	}
    } 

    /*
    **  Not sure, but the dialog box is probably already unmanaged
    */
    XtUnmanageChild(w);

    if (cbs->reason == (int)XmCR_OK)
    {
	status = DWC$DB_Open_calendar
	(
	    ads->filespec,
	    &cab,
	    DWC$m_force_upg,
	    0,
	    0, 
	    (int *)&errno,
	    (int *)&vaxc$errno
	);
	/*
	**  The only errors that can happen at this point when upgrading are: 
	**  DWC$k_db_insdisk, DWC$k_db_upgread, DWC$k_db_failure; the
	**  DWC$k_db_open_wt is a success status code.
	*/
	if (status == DWC$k_db_insdisk)
	{
	    ERRORReportError
	    (
		ads->requestor,
		"ErrorInsdisk",
		(XtCallbackProc) ErrorCantOpen,
		(char *) ads
	    );
	}
	else if (status == DWC$k_db_failure)
	{
	    ERRORReportErrno
	    (
		ads->requestor,
		"ErrorDatabaseError",
		(XtCallbackProc) ErrorCantOpen,
		(char *) ads
	    );
	}
	else if (status == DWC$k_db_upgread)
	{
	    ERRORReportError
	    (
		ads->requestor,
		"ErrorCantUpgrade",
		(XtCallbackProc) ErrorCantOpen,
		(char *) ads
	    );
	}
	else if (status == DWC$k_db_open_wt)
	{
	    status = DWC$DB_Close_calendar(cab);
	    FILEOpenCalendar (ads, ads->filespec, &new_cd);
	}
	else
	{
	    ERRORReportErrno
	    (
		ads->requestor,
		"ErrorDatabaseError",
		(XtCallbackProc) ErrorCantOpen,
		(char *) ads
	    );
	}
    }

    return;
}

int FILEOpenCalendar
#ifdef _DWC_PROTO_
	(
	AllDisplays		ads,
	char			*filespec,
	CalendarDisplay		*new_cd_ptr)
#else	/* no prototypes */
	( ads, filespec, new_cd_ptr)
	AllDisplays		ads;
	char			*filespec;
	CalendarDisplay		*new_cd_ptr;
#endif /* prototypes */
{
    Cardinal			dsbot;
    unsigned short int		now;
    unsigned int		a_day;
    unsigned short int		a_time;
    int				status;
    Cardinal			this;
    struct DWC$db_access_block	*cab;
    Cardinal			i, l;
    Arg				arglist [10];
    Cardinal			ac;
    CalendarDisplay		new_cd;
    struct tm			*local_time;
#ifndef VMS
    int		    vaxc$errno;
#endif


    status = DWC$DB_Open_calendar
	(filespec, &cab, FALSE, 0, 0, (int *)&errno, (int *)&vaxc$errno);
    if (status == DWC$k_db_nosuchfile )
    {
	if (ads->filespec == NULL)
	{
	    ads->filespec = filespec;
	    ERRORReportErrno
	    (
		ads->requestor,
		"ErrorNeedNew",
		(XtCallbackProc) ErrorNeedNew,
		(char *) ads
	    );
	}
	else
	{
	    ERRORReportErrno
	    (
		ads->requestor,
		"ErrorCantOpen",
		(XtCallbackProc) ErrorCantOpen,
		(char *) ads
	    );
	}
	return FALSE;
    }

    if (status == DWC$k_db_failure)
    {
	ERRORReportErrno
	(
	    ads->requestor,
	    "ErrorDatabaseError",
	    (XtCallbackProc) ErrorCantOpen,
	    (char *) ads
	);
	return FALSE;
    }

    if (status == DWC$k_db_upgrade)
    {
	if (ads->filespec == NULL)
	{
	    ads->filespec = filespec;
	}
	ERRORReportError
	(
	    ads->requestor,
	    "ErrorUpgrade",
	    (XtCallbackProc) ErrorNeedUpgrade,
	    (char *) ads
	);
	return FALSE;
    }

    if (status == DWC$k_db_upgread)
    {
	ERRORReportError
	(
	    ads->requestor,
	    "ErrorCantUpgrade",
	    (XtCallbackProc) ErrorCantOpen,
	    (char *) ads
	);
	return FALSE;
    }

    if (status == DWC$k_db_locked)
    {
	ERRORReportError
	(
	    ads->requestor,
	    "ErrorLocked",
	    (XtCallbackProc) ErrorCantOpen,
	    (char *) ads
	);
	return FALSE;
    }

    if (status == DWC$k_db_badfile)
    {
	ERRORReportError
	(
	    ads->requestor,
	    "ErrorBadfile",
	    (XtCallbackProc) ErrorCantOpen,
	    (char *) ads
	);
	return FALSE;
    }


    for (i = 0; i < ads->number_of_calendars; i++)
    {
	ICONSWaitCursorDisplay (ads->cds [i]->mainwid, ads->wait_cursor);
    }


    if (ads->number_of_calendars == 0)
    {
	ads->cds = (CalendarDisplay *) XtMalloc (sizeof (CalendarDisplay));
    }
    else
    {
	ads->cds = (CalendarDisplay *) XtRealloc
	(   (char *) ads->cds,
	    sizeof (CalendarDisplay) * (ads->number_of_calendars + 1)
	);
    }

    this = ads->number_of_calendars;
    ads->number_of_calendars++;

    new_cd = (CalendarDisplay) XtCalloc(sizeof (CalendarDisplayRecord), 1);
    *new_cd_ptr = new_cd;
    ads->cds [this] = new_cd;
    new_cd->number_of_sloteditors     = 0;
    new_cd->ads		       = ads;    
    new_cd->cab		       = cab;    
    new_cd->read_only		       = (status == DWC$k_db_open_rd);
    new_cd->filespec		       = filespec;    

    ads->filespec = NULL;

    if (DWC$DB_Get_calendar_name (cab, &new_cd->filename) != DWC$k_db_normal)
    {
	ERRORReportErrno
	(
	    ads->requestor,
	    "ErrorCantOpen",
	    (XtCallbackProc) ErrorCantOpen,
	    (char *) ads
	);
	DWC$DB_Close_calendar( new_cd->cab );  /* ignore return */
	XtFree (new_cd->filespec);
	XtFree ((char *)new_cd);
	ads->number_of_calendars--;
	return FALSE;
    }

    PROFILESetDefaults(&new_cd->profile);
    PROFILEByteSwap(&new_cd->profile);
    status = DWC$DB_Get_profile
    (
	cab,
	sizeof (ProfileStructure),
	&new_cd->profile
    );
    PROFILEByteSwap(&new_cd->profile);

    if (status != DWC$k_db_normal)
    {
	ERRORReportErrno
	(
	    ads->requestor,
	    "ErrorGetProfile",
	    (XtCallbackProc) ErrorCantOpen,
	    (char *) ads
	);
	DWC$DB_Close_calendar (new_cd->cab);  /* ignore return */
	XtFree (new_cd->filespec);
	XtFree ((char *)new_cd);
	ads->number_of_calendars--;
	return FALSE;
    }

    CUSTTellDBWorkDays( new_cd, new_cd->profile.work_days );
    
    MISCGetTime(&local_time);
    dsbot = DATEFUNCDaysSinceBeginOfTime 
	(local_time->tm_mday, local_time->tm_mon, local_time->tm_year);
    now   = (local_time->tm_hour * 60) + local_time->tm_min;
 
    status = DWC$DB_Get_next_alarm_time (cab, dsbot, now, &a_day, &a_time);
    if (status == DWC$k_db_failure)
    {
	ERRORReportErrno
	(
	    ads->requestor,
	    "ErrorGetAlarm",
	    (XtCallbackProc) ErrorCantOpen,
	    (char *) ads
	);
	DWC$DB_Close_calendar( new_cd->cab );  /* ignore return */
	XtFree (new_cd->filespec);
	XtFree ((char *)new_cd);
	ads->number_of_calendars--;
	return FALSE;
    }

    /*	  
    **  Make sure we unmanage any message boxes we may have still hanging
    **	around.
    */	  
    for (i = 0; i < this; i++ )
    {
	CalendarDisplay t;
	t = ads->cds[i];
	if ( ads->cds[i]->db_open != NULL) 
	    XtUnmanageChild( ads->cds[i]->db_open );
	if ( ads->cds[i]->db_open_new != NULL) 
	    XtUnmanageChild( ads->cds[i]->db_open_new );
    }

    for (i = 0; i < this; i++ )
    {
	if ( ads->cds[i]->delete_pending )
	{
            /*	  
	    **  note that FILECloseCalendar rearranges cds[]
	    */	  
	    FILECloseCalendar( ads->cds[i], CurrentTime);
	    break;
	}
    }

    ac = 0;

    /*	  
    **  We don't want the delete handler to do anything since we've set up a
    **	handler to do a FILEDO_CLOSE when we get a DeleteWindow message.
    */	  
    XtSetArg(arglist[ac], XmNdeleteResponse, XmDO_NOTHING); ac++;

    if (new_cd->profile.start_iconized)
    {
	XtSetArg(arglist[ac], XtNinitialState, IconicState);  ac++;
    }

    new_cd->toplevel = XtAppCreateShell(CalendarDisplayAppName,
					CalendarTopClassName,
					applicationShellWidgetClass,
					XtDisplay (ads->root),
					arglist, ac);

    /*	  
    **  Update the internal time
    */	  
    MISCUpdateTime();

    CALStartupCalendar (new_cd);

    for (i = 0;  i < this;  i++)
    {
	ICONSWaitCursorRemove (ads->cds [i]->mainwid);
    }

    return TRUE;
}

void FILECloseCalendar
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	( cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif /* prototypes */
{

#if MEMEX
    /*	  
    **  Go close the MEMEX stuff for this calendar
    */	  
    MEMEXDeleteDwUi(cd);
#endif

    if (cd->cab != NULL)
    {
	if (! DSWRequestCloseOpenEntry
	    ((DayslotsWidget) (cd->dayslots.widget), time))
	{
	    return;
	}

	if (! DSWRequestCloseOpenEntry
	    ((DayslotsWidget) (cd->daynotes.widget), time))
	{
	    return;
	}

	if (DWC$DB_Close_calendar (cd->cab) == DWC$k_db_failure)
	{
	    ERRORDisplayErrno (cd->ads->root, "ErrorCantClose");
	}
    }

    HELPCloseForDisplay (cd);

    XtUnmapWidget   (cd->toplevel);
    XtDestroyWidget (cd->toplevel);
    CALDestroyCalendar (cd);

    return;
}

void FILESaveCalendar
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	( cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif /* prototypes */
{

    if (cd->cab != NULL)
    {
	if (! DSWRequestCloseOpenEntry
	    ((DayslotsWidget) (cd->dayslots.widget), time))
	{
	    return;
	}

	if (! DSWRequestCloseOpenEntry
	    ((DayslotsWidget) (cd->daynotes.widget), time))
	{
	    return;
	}

	if (DWC$DB_Close_calendar (cd->cab) == DWC$k_db_failure)
	{
	    ERRORDisplayErrno (cd->ads->root, "ErrorCantClose");
	}
    }

    return;
}

void FILEDO_FILE_MENU
#ifdef _DWC_PROTO_
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
    DSWEntryDataStruct	data;
    MwTypeSelected	s_type;
    Cardinal		day;
    Cardinal		week;
    Cardinal		month;
    Cardinal		year;
    Boolean		print_ok;
    Boolean		include_ok;
    Boolean		repeat_open;
    DwcDswEntry		entry;
    CalendarDisplay	cd;
    int			status;

    status = MISCFindCalendarDisplay( &cd, w );

    if (cbs->reason != (int)XmCR_MAP)
    {
	return;
    }

    print_ok   = FALSE;
    include_ok = ! cd->read_only;

    entry = DSWGetOpenEntry ((DayslotsWidget) (cd->dayslots.widget));
    if (entry == NULL)
    {
	entry = DSWGetOpenEntry ((DayslotsWidget) (cd->daynotes.widget));
    }
    
    if (entry != NULL)
    {
	DSWGetEntryData(entry, &data);
	cd->print_arg_start = data.start;
	cd->print_arg_end   = data.start + data.duration;

	XmStringFree (cd->print_arg_text);
	cd->print_arg_text  = DSWGetEntryCSText (entry);
	print_ok = !(XmStringEmpty(cd->print_arg_text));

	cd->print_arg_type  = MwNothingSelected;
	cd->print_arg_day   = cd->day;
	cd->print_arg_month = cd->month;
	cd->print_arg_year  = cd->year;
    }
    else
    {
	MWGetSelection (cd->month_display, &s_type, &day, &week, &month, &year);
	switch (s_type)
	{
	case MwNothingSelected :
	    if (cd->showing != show_day)
	    {
		break;
	    }
	    day = cd->day;  month = cd->month;  year = cd->year;
	case MwDaySelected :
	    if (cd->profile.print_blank_days)
	    {
		print_ok = TRUE;
	    }
	    else
	    {
		print_ok = MISCTestDayConditions (cd, FALSE, TRUE, day, month, year);
	    }

	    if (print_ok)
	    {
		cd->print_arg_type  = MwDaySelected;
		cd->print_arg_day   = day;
		cd->print_arg_month = month;
		cd->print_arg_year  = year;
	    }
	    break;

	case MwWeekSelected :
	    if (cd->profile.print_blank_days)
	    {
		print_ok = TRUE;
	    }
	    else
	    {
		print_ok =
		  MISCFirstDOWWithCondition (cd, week, year, FALSE, TRUE,
					   &day, &month, &cd->print_arg_year);
	    }

	    if (print_ok)
	    {
		cd->print_arg_type = MwWeekSelected;
		DATEFUNCStartDateForWeekNo
		(
		    week, year, cd->profile.first_day_of_week,
		    (int)cd->profile.week_numbers_start_day,
		    (int)cd->profile.week_numbers_start_month,
		    (int*)&cd->print_arg_day,
		    (int*)&cd->print_arg_month,
		    (int*)&cd->print_arg_year
		);
	    }
	    break;
	}
    }

    repeat_open = MISCTestRepeatOpen (cd);

    XtSetSensitive (cd->pb_open,         ! repeat_open);
    XtSetSensitive (cd->pb_close,
		       (cd->ads->number_of_calendars != 1) && (! repeat_open));
    XtSetSensitive (cd->pb_nameas,       (! cd->read_only) && (! repeat_open));
/*
    XtSetSensitive (cd->pb_remove,	 ! cd->read_only);
*/
    XtSetSensitive (cd->pb_delete,       (! cd->read_only) && (! repeat_open));
    XtSetSensitive (cd->pb_print,	 print_ok);
    XtSetSensitive (cd->pb_file_include, include_ok);
    XtSetSensitive (cd->pb_file_exit,    ! repeat_open);

}

void FILEDO_INCLUDE
#ifdef _DWC_PROTO_
	(
	Widget					w,
	caddr_t					tag,
	XmFileSelectionBoxCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, tag, cbs)
	Widget					w;
	caddr_t					tag;
	XmFileSelectionBoxCallbackStruct	*cbs;
#endif	/* prototype */
{
    DwcDswEntry		    entry;
    char		    *filespec;
    XmString		    buffer;
    int			    c_err;
    int			    vaxc_err;
    CalendarDisplay	    cd;
    int			    status;
    Time		    time;
    

    status = MISCFindCalendarDisplay (&cd, w);

    time = MISCGetTimeFromAnyCBS ((XmAnyCallbackStruct *)cbs);

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
	    ERRORDisplayError (cd->db_include_file, "ErrorIncludeSpec");
	    return;
	}

	entry = DSWGetOpenEntry ((DayslotsWidget) (cd->dayslots.widget));
	if (entry == NULL)
	{
	    entry = DSWGetOpenEntry ((DayslotsWidget) (cd->daynotes.widget));
	}

	ICONSWaitCursorDisplay( cd->mainwid, cd->ads->wait_cursor );

	if (entry == NULL)
	{
	    status = DWCDB_LoadInterchange
		(cd->cab, filespec, &buffer, &c_err, &vaxc_err);
	    if (status == DWC$k_db_normal)
	    {
		if (CLIPImportInterchange (cd, buffer, time))
		{
		    XtUnmanageChild (w);
		}
		else
		{
		    ERRORDisplayError (cd->db_include_file, "ErrorInclSyntax");
		}
		XmStringFree (buffer);
	    }
	    else
	    {
		ERRORDisplayErrno (cd->db_include_file, "ErrorInclude");
	    }
	}
	else
	{
	    status = MISCIncludeFileIntoText
		(cd, filespec, (DXmCSTextWidget) DSWGetEntryTextWidget (entry));
	    if (status == k_include_success)
	    {
		XtUnmanageChild (w);
	    }
	    else if (status == k_include_empty)
	    {
		ERRORDisplayError (cd->db_include_file, "ErrorEmptyInclude");
	    } 
	    else
	    {
		ERRORDisplayErrno (cd->db_include_file, "ErrorInclude");
	    }
	}

	ICONSWaitCursorRemove (cd->mainwid);

	XtFree (filespec);

	if ((status == DWC$k_db_normal) || (status == k_include_success))
	{
	    if (!XmProcessTraversal (cd->dayslots.widget, XmTRAVERSE_CURRENT))
	    {
		XmProcessTraversal (cd->daynotes.widget, XmTRAVERSE_CURRENT);
	    }
    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */
	}
    
    }
    
    return;
}

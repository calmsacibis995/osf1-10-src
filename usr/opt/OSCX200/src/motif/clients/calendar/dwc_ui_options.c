/* dwc_ui_options.c */
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
**	This is the module deals with the options menu
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**  V3-003  Paul Ferwerda					08-May-1990
**		Tweaked include files, removed LIB$:, port to Motif.
**	V1-002	Ken Cowan					28-Feb-1989
**		Fix build errors.
**	V1-001  Ken Cowan					27-Feb-1989
**		Initial version.  Split out from DWC_UI_CALENDAR.C
*--
**/

#include "dwc_compat.h"

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"	/* for DWC$k_db_normal */

#include "dwc_ui_calendar.h"		    /* CalendarDisplay */
#include "dwc_ui_datestructures.h"
#include "dwc_ui_misc.h"		    /* MISCFindCalendarDisplay */
#include "dwc_ui_monthwidget.h"	    /* for MWGetSelection */
#include "dwc_ui_alarms.h"		    /* for ALARMSSetupNextAlarm */
#include "dwc_ui_day.h"		    /* for DAYTestAnyOpenEntries */
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_errorboxes.h"

void OPTIONSCUSTOM_MENU
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
    {
    CalendarDisplay	cd ;
    int			status ;

    if (*reason != (int)XmCR_MAP)
    {
	return ;
    }

    status = MISCFindCalendarDisplay (&cd, w) ;
    
    XtSetSensitive (cd->pb_custom_save, ! cd->read_only) ;

}

void OPTIONSMARK_MENU
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
    {
    Cardinal		dsbot ;
    MwTypeSelected	s_type ;
    Cardinal		day ;
    Cardinal		week ;
    Cardinal		month ;
    Cardinal		year ;
    unsigned char	marks ;
    unsigned char	junk ;
    CalendarDisplay	cd ;
    int			status ;
    Boolean		mark_default     = FALSE ;
    Boolean		mark_workday     = FALSE ;
    Boolean		mark_non_workday = FALSE ;
    Boolean		mark_normal      = FALSE ;
    Boolean		mark_special     = FALSE ;
    

    if (*reason != (int)XmCR_MAP)
    {
	return;
    }

    status = MISCFindCalendarDisplay (&cd, w) ;
    
    MWGetSelection (cd->month_display, &s_type, &day, &week, &month, &year) ;

    if (s_type != MwDaySelected) {
	mark_default = mark_workday = mark_non_workday = 
    	 mark_normal = mark_special = TRUE ;
    } else {

	dsbot = DATEFUNCDaysSinceBeginOfTime (day, month, year) ;

	if (DWC$DB_Get_day_attr (cd->cab, dsbot, &marks, &junk, &junk)
							   != DWC$k_db_normal) {
	    ERRORDisplayError (cd->mainwid, "ErrorDayAttr") ;
	} else {

	    if ((marks & DWC$m_day_special) == 0) {
		mark_special = TRUE ;
	    } else {
		mark_normal  = TRUE ;
	    }
	    marks = marks & ~DWC$m_day_special ;

	    if ((marks & DWC$m_day_defaulted) == 0) {
		mark_default = TRUE ;
		if (marks == DWC$k_day_workday) {
		    mark_non_workday = TRUE ;
		} else {
		    mark_workday     = TRUE ;
		}
	    } else {
		mark_non_workday = TRUE ;
		mark_workday     = TRUE ;
	    }
	}
    }
    
    XtSetSensitive (cd->pb_mark_default,      mark_default) ;
    XtSetSensitive (cd->pb_mark_work_day,     mark_workday) ;
    XtSetSensitive (cd->pb_mark_non_work_day, mark_non_workday) ;
    XtSetSensitive (cd->pb_mark_normal,       mark_normal) ;
    XtSetSensitive (cd->pb_mark_special,      mark_special) ;

}

void OPTIONSMARK_ITEM
#ifdef _DWC_PROTO_
	(
	Widget	w,
	caddr_t	tag,
	int	*reason)
#else	/* no prototypes */
	(w, tag, reason)
	Widget	w;
	caddr_t	tag;
	int	*reason;
#endif	/* prototype */
{
    unsigned char	*entry_type;
    unsigned char	*marks;
    unsigned char	*junk;
    unsigned char	newmark;
    MwTypeSelected	s_type;
    Cardinal		day;
    Cardinal		week;
    Cardinal		month;
    Cardinal		year;
    Cardinal		i;
    Cardinal		first;
    Cardinal		last;
    Boolean		change_work;
    CalendarDisplay	cd;
    int			status;


    status = MISCFindCalendarDisplay (&cd, w);

    change_work = TRUE;

    if (w == cd->pb_mark_default)
    {
	newmark = DWC$k_day_default;
    }
    else
    {
	if (w == cd->pb_mark_work_day)
	{
	    newmark = DWC$k_day_workday;
	}
	else
	{
	    if (w == cd->pb_mark_non_work_day)
	    {
		newmark = DWC$k_day_nonwork;
	    }
	    else
	    {
		change_work = FALSE;
		if (w == cd->pb_mark_special)
		{
		    newmark = DWC$m_day_special;
		}
		else
		{
		    newmark = 0;
		}
	    }
	}
    }
    
    if (change_work)
    {
	if (DAYTestAnyOpenEntries(cd))
	{
	    ERRORDisplayError (cd->mainwid, "ErrorCloseAllMark");
	    return;
	}
    }

    MWGetSelection (cd->month_display, &s_type, &day, &week, &month, &year);

    if (s_type == MwDaySelected)
    {
	first = last = DATEFUNCDaysSinceBeginOfTime (day, month, year);
    }
    else
    {
	DATEFUNCStartDateForWeekNo
	(
	    week,
	    year,
	    cd->profile.first_day_of_week,
	    cd->profile.week_numbers_start_day,
	    cd->profile.week_numbers_start_month,
	    (int *)&day,
	    (int *)&month,
	    (int *)&year
	);
	first = DATEFUNCDaysSinceBeginOfTime (day, month, year);
	last  = first + 6;
    }

    i = last - first + 1;
    entry_type = (unsigned char *) XtMalloc (sizeof (unsigned char) * i);
    marks      = (unsigned char *) XtMalloc (sizeof (unsigned char) * i);
    junk       = (unsigned char *) XtMalloc (sizeof (unsigned char) * i);

    if (DWC$DB_Get_day_attr_rng (cd->cab, first, last, marks, junk,
				 entry_type) != DWC$k_db_normal)
    {
	ERRORDisplayError (cd->mainwid, "ErrorDayAttr");
	return;
    }

    for (i = 0;  i < last - first + 1;  i++)
    {
	if (change_work)
	{
	    marks [i] = (marks [i] & DWC$m_day_special) | newmark;
	}
	else
	{
	    if ((marks [i] & DWC$m_day_defaulted) == 0)
	    {
		marks [i] = (marks [i] & ~DWC$m_day_special) | newmark;
	    }
	    else
	    {
		marks [i] = DWC$k_day_default | newmark;
	    }
	}

	if (DWC$DB_Put_day_attr (cd->cab, first + i, marks [i], 0)
							   != DWC$k_db_normal)
	{
	    ERRORDisplayErrno (cd->mainwid, "ErrorPutAttr");
	    break;
	}
    }

    XtFree ((char *) entry_type);
    XtFree ((char *) marks);
    XtFree ((char *) junk);

    DAYUpdateMonthMarkup(cd, first, last, change_work);

    if (change_work)
    {
	ALARMSSetupNextAlarm (cd, FALSE);

	if ((cd->showing == show_day) &&
	    (cd->dsbot >= first) &&
	    (cd->dsbot <= last))
	{
	    MISCChangeView(cd, show_day, cd->day, cd->month, cd->year);
	}
    }
    
}

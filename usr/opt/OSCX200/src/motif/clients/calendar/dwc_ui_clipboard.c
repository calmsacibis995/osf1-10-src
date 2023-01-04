/* dwc_ui_clipboard.c */
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
**	This module contains the support for clip-board support
**
**  MODIFICATION HISTORY
**
**	17-DEC-1993	Dhiren M Patel (dp)
**			Fix ootb_bug 442. 
**			Routine CLIPSMTestTargets () had assignment
**			operators where there should have been
**			equality operators.
**
**
**
**--
*/

#include "dwc_compat.h"

#include <stdio.h>

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Xatom.h>			/* for XA_STRING */
#if 0
#include <X11/Selection.h>		/* for XT_CONVERT_FAIL */
#endif
#if (((XmVERSION == 1) && (XmREVISION == 2)) || XmVERSION == 2)
#include <Xm/PrimitiveP.h>
#endif
#include <Xm/CutPaste.h>		/* for Clipboard returns */
#include <Xm/AtomMgr.h>			/* for XmGetAtomName */
#include <Xm/Xm.h>			/* xmManagerWidgetClass */
#include <DXm/DXmCSTextP.h>
#include <DXm/DXmCSText.h>
#include <Xm/TextP.h>
#include <Xm/Text.h>
#include <Xm/SashP.h>
#include <dwi18n_lib.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h" 	
#include "dwc_db_public_include.h" 	/* for DWC$k_db_normal */

#include "dwc_ui_dateformat.h"   	/* for dtb.. */
#include "dwc_ui_dayslotswidgetp.h"	/* for DwcDswEntry */
#include "dwc_ui_timeslotwidgetp.h"
#include "dwc_ui_calendar.h"	    	/* for CalendarDisplay */
#include "dwc_ui_clipboard.h"
#include "dwc_ui_day.h"			/* for DAYCreateDayItemRecord */
#include "dwc_ui_monthwidget.h"		/* MWGetSelection */
#include "dwc_ui_misc.h"		/* for MISCGetTimeFrom... */
#include "dwc_ui_sloteditor.h"		/* for SLOTEDITORWIDGETS */
#include "dwc_ui_datefunctions.h"
#include "dwc_ui_errorboxes.h"

static Atom SMAtomTargets;
static Atom SMAtomKill;
static Atom SMAtomDDIF;
static Atom SMAtomCOMPOUND_TEXT;
static Atom SMAtomCalendarEntries;

/*
**  Function prototypes
*/


static Boolean clip_convert_selected_to_interchange_CS PROTOTYPE ((
	CalendarDisplay	cd,
	XmString	*return_text));

static void delete_all_selected PROTOTYPE ((
	CalendarDisplay	cd,
	Time		time));

static char *clip_build_interchange_buffer PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	*buf_len));

/*
** Should probably change this over to sizes that are guaranteed to be the
** the same on all architectures.
*/
typedef struct
{
    Cardinal		start_time;
    Cardinal		duration;
    int			flags;
    Cardinal		alarms_number;
    Cardinal		icons_number;
    Cardinal		text_length;
    char		rest; /* Alarms, then icons, then text */
} DwcClipboardItem;

typedef struct
{
    Cardinal	    	magic;
    Cardinal		item_count;
    char		items; /* DwcClipboardItem(s)... */
} DwcClipboardHeader;

#define	MagicNumber	14011955
#define CTMagicNumber	21018246

static void clip_build_buffer_from_day_items
#ifdef _DWC_PROTO_
	(
	Cardinal	num_items,
	DwcDayItem	*items,
	char		**texts,
	long		*text_lengths,
	char		**return_buffer,
	Cardinal	*return_length)
#else	/* no prototypes */
	(num_items, items, texts, text_lengths, return_buffer, return_length)
	Cardinal	num_items;
	DwcDayItem	*items;
	char		**texts;
	long		*text_lengths;
	char		**return_buffer;
	Cardinal	*return_length;
#endif	/* prototype */
{
    Cardinal		i;
    char		*buffer;
    Cardinal		buffer_length;
    Cardinal		*cbi_offsets;
    DwcClipboardItem	*cbi;
    Cardinal		size;
    char		*rest;
    DwcClipboardHeader	*cbh;


    cbi_offsets  = (Cardinal *) XtMalloc (sizeof (Cardinal) * num_items);
    
    buffer_length = XtOffset (DwcClipboardHeader *, items);

    for (i = 0;  i < num_items;  i++)
    {
	buffer_length = (buffer_length + 3) & ~3;
	cbi_offsets [i] = buffer_length;

	buffer_length = buffer_length + XtOffset (DwcClipboardItem *, rest);
	buffer_length = buffer_length + (items [i]->alarms_number *
					 sizeof (unsigned short int));
	buffer_length = buffer_length + (items [i]->icons_number *
					 sizeof (unsigned char));
	buffer_length = buffer_length + text_lengths [i];
    }

    buffer = XtMalloc (buffer_length);

    cbh = (DwcClipboardHeader *) buffer;

    cbh->magic = CTMagicNumber;
    cbh->item_count = num_items;

#if BYTESWAP
    DWC$$DB_Byte_swap_field (&cbh->magic, sizeof(cbh->magic));
    DWC$$DB_Byte_swap_field (&cbh->item_count, sizeof(cbh->item_count));
#endif

    for (i = 0;  i < num_items;  i++)
    {
	cbi = (DwcClipboardItem *) (buffer + cbi_offsets [i]);

	cbi->start_time    = items [i]->start_time;
	cbi->duration      = items [i]->duration;
	cbi->flags         = items [i]->flags;
	cbi->alarms_number = items [i]->alarms_number;
	cbi->icons_number  = items [i]->icons_number;
	cbi->text_length   = text_lengths [i];

#if BYTESWAP
	DWC$$DB_Byte_swap_field(cbi->start_time, sizeof(cbi->start_time));
	DWC$$DB_Byte_swap_field(cbi->duration, sizeof(cbi->duration));
	DWC$$DB_Byte_swap_field(cbi->flags, sizeof(cbi->flags));
	DWC$$DB_Byte_swap_field(cbi->alarms_number, sizeof(cbi->alarms_number));
	DWC$$DB_Byte_swap_field(cbi->icons_number, sizeof(cbi->icons_number));
	DWC$$DB_Byte_swap_field(cbi->text_length, sizeof(cbi->text_length));
#endif

	rest = &(cbi->rest);

	if (cbi->alarms_number != 0)
	{
#if BYTESWAP
	    int		j;
#endif
	    size = sizeof (unsigned short int) * items[i]->alarms_number;
	    memcpy (rest, items [i]->alarms_times, size);
#if BYTESWAP
	    for (j = 0; j < items[i]->alarms_number; j++)
	    {
		DWC$$DB_Byte_swap_field
		    (&rest[j*2], sizeof(unsigned short int));
	    }
#endif
	    rest = rest + size;
	}

	if (cbi->icons_number != 0)
	{
	    size = sizeof (unsigned char) * cbi->icons_number;
	    memcpy (rest, items [i]->icons, size);
	    rest = rest + size;
	}

	memcpy (rest, texts [i], text_lengths [i]);
    }

    XtFree ((char *)cbi_offsets);

    *return_buffer = buffer;
    *return_length = buffer_length;

}

static Boolean clip_parse_buffer_for_dayitems_old
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Cardinal	dsbot,
	char		*buffer,
	Time		time)
#else	/* no prototypes */
	(cd, dsbot, buffer, time)
	CalendarDisplay	cd;
	Cardinal	dsbot;
	char		*buffer;
	Time		time;
#endif	/* prototype */
{
    DwcClipboardHeader	*cbh;
    DwcClipboardItem	*cbi;
    Cardinal		num_items;
    DwcDayItem		di;
    DwcDaySlots		ds;
    DayItemUpdate	diu;
    Cardinal		i;
    Cardinal		text_length;
    char		*slot_text;
    XmString		xm_text;
    char		*text;
    char		*rest;    
    unsigned short int	*alarms;
    unsigned char	*icons;
    long		byte_count, cvt_status;


    cbh = (DwcClipboardHeader *) buffer;

    if (cbh->magic != MagicNumber)
    {
	return (FALSE);
    }

    num_items = cbh->item_count;

    cbi = (DwcClipboardItem *) &(cbh->items);

    for (i = 0;  i < num_items;  i++)
    {

	rest = &(cbi->rest);

#if BYTESWAP
	DWC$$DB_Byte_swap_field(cbi->start_time, sizeof(cbi->start_time));
	DWC$$DB_Byte_swap_field(cbi->duration, sizeof(cbi->duration));
	DWC$$DB_Byte_swap_field(cbi->flags, sizeof(cbi->flags));
	DWC$$DB_Byte_swap_field(cbi->alarms_number, sizeof(cbi->alarms_number));
	DWC$$DB_Byte_swap_field(cbi->icons_number, sizeof(cbi->icons_number));
	DWC$$DB_Byte_swap_field(cbi->text_length, sizeof(cbi->text_length));
#endif

	if (cbi->alarms_number == 0)
	{
	    alarms = NULL;
	}
	else
	{
	    alarms = (unsigned short int *) rest;
#if BYTESWAP
	    {
		int		j;
		for (j = 0; j < cbi->alarms_number; j++)
		{
		    DWC$$DB_Byte_swap_field (&alarms[j], sizeof(alarms));
		}
	    }
#endif
	    rest   = rest + (cbi->alarms_number * sizeof (unsigned short int));
	}
	
	if (cbi->icons_number == 0)
	{
	    icons = NULL;
	}
	else
	{
	    icons = (unsigned char *) rest;
	    rest  = rest + (cbi->icons_number * sizeof (unsigned char));
	}
	
	di = DAYCreateDayItemRecord
	    (cbi->alarms_number, alarms, cbi->icons_number,  icons);

	di->start_time = cbi->start_time;
	di->duration   = cbi->duration;
	di->flags      = cbi->flags;

	text_length    = cbi->text_length;
	text	       = rest;

	cbi = (DwcClipboardItem *) (((int) rest + text_length + 3) & ~3);

	if (di->duration == 0)
	{
	    ds = &(cd->daynotes);
	}
	else
	{
	    ds = &(cd->dayslots);
	}
	
	slot_text = XtMalloc (text_length + 1);
	memcpy (slot_text, text, text_length);
	slot_text [text_length] = '\0';
	xm_text = DXmCvtFCtoCS (slot_text, &byte_count, &cvt_status);

	diu = DAYCreateDayItemUpdateRecord
	    (cd, dsbot, xm_text, NULL, di, RCKNotRepeat);
	diu->slots = ds;

	(void) DAYDoDayItemUpdate (diu, time);
	DAYDestroyDayItemUpdate (diu);

	XtFree (slot_text);
    }

    return (TRUE);

}

static Boolean clip_parse_buffer_for_dayitems
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Cardinal	dsbot,
	char		*buffer,
	Time		time)
#else	/* no prototypes */
	(cd, dsbot, buffer, time)
	CalendarDisplay	cd;
	Cardinal	dsbot;
	char		*buffer;
	Time		time;
#endif	/* prototype */
{
    DwcClipboardHeader	*cbh;
    DwcClipboardItem	*cbi;
    Cardinal		num_items;
    DwcDayItem		di;
    DwcDaySlots		ds;
    DayItemUpdate	diu;
    Cardinal		i;
    Cardinal		text_length;
    char		*slot_text;
    XmString		xm_text;
    char		*text;
    char		*rest;    
    unsigned short int	*alarms;
    unsigned char	*icons;
    long		byte_count, cvt_status;


    cbh = (DwcClipboardHeader *) buffer;

#if BYTESWAP
    DWC$$DB_Byte_swap_field (&cbh->magic, sizeof(cbh->magic));
    DWC$$DB_Byte_swap_field (&cbh->item_count, sizeof(cbh->item_count));
#endif

    if (cbh->magic == MagicNumber)
    {
	return (clip_parse_buffer_for_dayitems_old (cd, dsbot, buffer, time));
    }

    if (cbh->magic != CTMagicNumber)
    {
	return (FALSE);
    }

    num_items = cbh->item_count;

    cbi = (DwcClipboardItem *) &(cbh->items);

    for (i = 0;  i < num_items;  i++)
    {

	rest = &(cbi->rest);

#if BYTESWAP
	DWC$$DB_Byte_swap_field(cbi->start_time, sizeof(cbi->start_time));
	DWC$$DB_Byte_swap_field(cbi->duration, sizeof(cbi->duration));
	DWC$$DB_Byte_swap_field(cbi->flags, sizeof(cbi->flags));
	DWC$$DB_Byte_swap_field(cbi->alarms_number, sizeof(cbi->alarms_number));
	DWC$$DB_Byte_swap_field(cbi->icons_number, sizeof(cbi->icons_number));
	DWC$$DB_Byte_swap_field(cbi->text_length, sizeof(cbi->text_length));
#endif

	if (cbi->alarms_number == 0)
	{
	    alarms = NULL;
	}
	else
	{
	    alarms = (unsigned short int *) rest;
#if BYTESWAP
	    {
		int		j;
		for (j = 0; j < cbi->alarms_number; j++)
		{
		    DWC$$DB_Byte_swap_field (&alarms[j], sizeof(alarms));
		}
	    }
#endif
	    rest   = rest + (cbi->alarms_number * sizeof (unsigned short int));
	}
	
	if (cbi->icons_number == 0)
	{
	    icons = NULL;
	}
	else
	{
	    icons = (unsigned char *) rest;
	    rest  = rest + (cbi->icons_number * sizeof (unsigned char));
	}
	
	di = DAYCreateDayItemRecord
	    (cbi->alarms_number, alarms, cbi->icons_number,  icons);

	di->start_time = cbi->start_time;
	di->duration   = cbi->duration;
	di->flags      = cbi->flags;

	text_length    = cbi->text_length;
	text	       = rest;

	cbi = (DwcClipboardItem *) (((int) rest + text_length + 3) & ~3);

	if (di->duration == 0)
	{
	    ds = &(cd->daynotes);
	}
	else
	{
	    ds = &(cd->dayslots);
	}
	
	slot_text = XtMalloc (text_length + 1);
	memcpy (slot_text, text, text_length);
	slot_text [text_length] = '\0';
	xm_text = DXmCvtDDIFtoCS (slot_text, &byte_count, &cvt_status);

	diu = DAYCreateDayItemUpdateRecord
	    (cd, dsbot, xm_text, NULL, di, RCKNotRepeat);
	diu->slots = ds;

	(void) DAYDoDayItemUpdate (diu, time);
	DAYDestroyDayItemUpdate (diu);

	XtFree (slot_text);
    }

    return (TRUE);

}

ClipContents CLIPTestClipboardContents
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    register int	status;
    register Display	*disp;
    register Window	wind;
    unsigned long	length;
    char		*xa_string;

    disp = XtDisplay(w);
    wind = XtWindow(w);

    status = XmClipboardInquireLength (disp, wind, "Calendar-Entries", &length);
    if ((status == ClipboardSuccess) && (length != 0))
    {
	return (ClipHasEntry);
    }

    status = XmClipboardInquireLength (disp, wind, "COMPOUND_TEXT", &length);
    if ((status == ClipboardSuccess) && (length != 0))
    {
	return (ClipHasCT);
    }

    status = XmClipboardInquireLength (disp, wind, "DDIF", &length);
    if ((status == ClipboardSuccess) && (length != 0))
    {
	return (ClipHasDDIF);
    }

    xa_string = XmGetAtomName (disp, XA_STRING);
    status = XmClipboardInquireLength (disp, wind, xa_string, &length);
    XFree (xa_string);
    if ((status == ClipboardSuccess) && (length != 0))
    {
	return (ClipHasString);
    }

    status = XmClipboardInquireLength
	(disp, wind, "DEC_COMPOUND_STRING", &length);
    if ((status == ClipboardSuccess) && (length != 0))
    {
	return (ClipHasCS);
    }

    return (ClipHasNothing);
}

static int copy_text_to_clipboard
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time,
	XmString	text)
#else	/* no prototypes */
	(cd, time, text)
	CalendarDisplay	cd;
	Time		time;
	XmString	text;
#endif	/* prototype */
{
    register Display	*disp;
    register Window	wind;
    register int	status;
    long		item_id;
    XmString		name;
    XmString		xm_str;
    long		byte_count, cvt_status;

    disp = XtDisplay(cd->mainwid);
    wind = XtWindow(cd->mainwid);
    name   = XmStringCreateSimple("Calendar");

    /*	  
    **  Begin the copy.
    */
    status = XmClipboardStartCopy
	(disp, wind, name, time, NULL, NULL, &item_id);
    if (status == ClipboardLocked)
    {
	ERRORDisplayError (cd->mainwid, "ErrorClipCopy");
	XmStringFree (name);
	return (status);
    }

    /*	  
    **  Copy the data. - All formats!
    */
    status = DWI18n_ClipboardCopy (disp, wind, item_id, 0, text);

    if (status != ClipboardSuccess)
    {
	ERRORDisplayError (cd->mainwid, "ErrorClipCopy");
	XmClipboardCancelCopy (disp, wind, item_id);
	return (status);
    }

    /*	  
    **  End the transaction.
    */	  
    status = XmClipboardEndCopy (disp, wind, item_id);

    XmStringFree (name);

    if (status != ClipboardSuccess)
    {
	ERRORDisplayError(cd->mainwid, "ErrorClipCopy");
    }

    return (status);
}

/*
**++
**
**  CALLING SEQUENCE:
**
**	static int copy_timeslots_to_clipboard(
**	    CalendarDisplay	cd,
**	    Time		time,	    time of the event
**	    char		*buff,
**	    Cardinal		blen,
**	    char		*slot_text)
**
**  FUNCTIONAL DESCRIPTION:
**
**      Copies both the Timeslots buffer and the text to the clipboard. If the
**	Clipboard is locked it will keep trying until the Clipboard is unlocked.
**	
**
**   
**  COMPLETION CODES:
**       
**      ClipboardSuccess -
*
**              Everything went as planned
**   
**  SIDE EFFECTS:
**   
**      none
**   
**  KNOWN PROBLEMS:
**   
**      none
**
**--
*/
static int copy_timeslots_to_clipboard
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time,
	char		*buff,
	Cardinal	blen,
	char		*slot_text)
#else	/* no prototypes */
	(cd, time, buff, blen, slot_text)
	CalendarDisplay	cd;
	Time		time;
	char		*buff;
	Cardinal	blen;
	char		*slot_text;
#endif	/* prototype */
{
    register Window	wind;
    register Display	*disp;
    register int	status;
    long		item_id;
    int			data_id;
    char		*xa_string;
    XmString		name;
    XmString		xm_str;
    char		*ct_str;
    Opaque		ddif_str;
    long		byte_count, cvt_status;

    disp = XtDisplay(cd->mainwid);
    wind = XtWindow(cd->mainwid);
    name = XmStringCreateSimple("Calendar");

    /*
    ** Dummy loop to clean up error processing.
    */
    do
    {
	/*	  
	**  Begin the copy.
	*/
	status = XmClipboardStartCopy
	    (disp, wind, name, time, NULL, NULL, &item_id);
	if (status == ClipboardLocked) break;

	/*
	** Do the calendar specific "Entry" format.
	*/
	status = XmClipboardCopy
	    (disp, wind, item_id, "Calendar-Entries", buff, blen, 0, &data_id);
	if (status == ClipboardLocked)
	{
	    XmClipboardCancelCopy (disp, wind, item_id);
	    break;
	}

	/*
	** Do the generic text formats.
	*/
	if (slot_text != NULL)
	{
	    xm_str = DXmCvtFCtoCS(slot_text,  &byte_count, &cvt_status);
	    status = DWI18n_ClipboardCopy (disp, wind, item_id, 0, xm_str);
	    XmStringFree(xm_str);
	    if (status != ClipboardSuccess)
	    {
		XmClipboardCancelCopy (disp, wind, item_id);
		break;
	    }
	}

	/*
	** Done.
	*/
	status = XmClipboardEndCopy(disp, wind, item_id);
	if (status != ClipboardSuccess) break;

    } while (False);

    XmStringFree (name);

    if (status != ClipboardSuccess)
    {
	ERRORDisplayError (cd->mainwid, "ErrorClipCopy");
    }
    
    return (status);

}

/*
**++
**
**  CALLING SEQUENCE:
**
**	static int copy_timeslots_to_clipboard_CS(
**	    CalendarDisplay	cd,
**	    Time		time,	    time of the event
**	    char		*buff,
**	    Cardinal		blen,
**	    XmString		slot_text)
**
**  FUNCTIONAL DESCRIPTION:
**
**      Copies both the Timeslots buffer and the text to the clipboard. If the
**	Clipboard is locked it will keep trying until the Clipboard is unlocked.
**	
**
**   
**  COMPLETION CODES:
**       
**      ClipboardSuccess -
*
**              Everything went as planned
**   
**  SIDE EFFECTS:
**   
**      none
**   
**  KNOWN PROBLEMS:
**   
**      none
**
**--
*/
static int copy_timeslots_to_clipboard_CS
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time,
	char		*buff,
	Cardinal	blen,
	XmString	slot_text)
#else	/* no prototypes */
	(cd, time, buff, blen, slot_text)
	CalendarDisplay	cd;
	Time		time;
	char		*buff;
	Cardinal	blen;
	XmString	slot_text;
#endif	/* prototype */
{
    register Window	wind;
    register Display	*disp;
    register int	status;
    long		item_id;
    int			data_id;
    XmString		name;
    XmString		xm_str;
    long		byte_count, cvt_status;

    disp = XtDisplay(cd->mainwid);
    wind = XtWindow(cd->mainwid);
    name = XmStringCreateSimple("Calendar");

    /*
    ** Dummy loop to clean up error processing.
    */
    do
    {
	/*	  
	**  Begin the copy.
	*/
	status = XmClipboardStartCopy
	    (disp, wind, name, time, NULL, NULL, &item_id);
	if (status == ClipboardLocked) break;

	/*
	** Do the calendar specific "Entry" format.
	*/
	status = XmClipboardCopy
	    (disp, wind, item_id, "Calendar-Entries", buff, blen, 0, &data_id);
	if (status == ClipboardLocked)
	{
	    XmClipboardCancelCopy (disp, wind, item_id);
	    break;
	}

	/*
	** Do the generic text formats.
	*/
	if (slot_text != NULL)
	{
	    status = DWI18n_ClipboardCopy (disp, wind, item_id, 0, slot_text);
	    if (status != ClipboardSuccess)
	    {
		XmClipboardCancelCopy (disp, wind, item_id);
		break;
	    }
	}

	/*
	** Done.
	*/
	status = XmClipboardEndCopy(disp, wind, item_id);
	if (status != ClipboardSuccess) break;

    } while (False);

    XmStringFree (name);

    if (status != ClipboardSuccess)
    {
	ERRORDisplayError (cd->mainwid, "ErrorClipCopy");
    }
    
    return (status);

}

static int copy_timeslots_from_clipboard
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	(cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif	/* prototype */
{
    register Display	*disp;
    register Window	wind;
    register int	status;
    int			private_id;
    long		item_id;
    char		*buffer;
    unsigned long	outlength;
    unsigned long	length;
    MwTypeSelected	s_type;
    Cardinal		day;
    Cardinal		week;
    Cardinal		month;
    Cardinal		year;
    Cardinal		dsbot;


    disp = XtDisplay(cd->mainwid);
    wind = XtWindow(cd->mainwid);
    status = XmClipboardStartRetrieve (disp, wind, time);
    if (status == ClipboardLocked)
    {
	ERRORDisplayError(cd->mainwid, "ErrorClipGet");
	return (status);
    }

    status = XmClipboardInquireLength (disp, wind, "Calendar-Entries", &length);
    if (status != ClipboardSuccess)
    {
	(void)XmClipboardEndRetrieve (disp, wind);

	/*
	** The only error that can occur here is NoData.  Locked is handled
	** above.
	*/
	ERRORDisplayError(cd->mainwid, "ErrorClipNoData");
	return (status);
    }

    buffer = XtMalloc(length + 1);
    status = XmClipboardRetrieve
    (
	disp,
	wind,
	"Calendar-Entries",
	buffer,
	length,
	&outlength,
	&private_id
    );
    if (status != ClipboardSuccess)
    {
	XtFree (buffer);
	(void)XmClipboardEndRetrieve (disp, wind);
	/*
	** The only error that can occur here is Truncate.  This is
	** because we have already accounted for Locked and NoData above.
	*/
	ERRORDisplayError(cd->mainwid, "ErrorClipTruncate");

	return ClipboardLocked;
    }

    /*	  
    **  Unlock the clipboard
    */	  
    (void)XmClipboardEndRetrieve (disp, wind);

    MWGetSelection (cd->month_display, &s_type, &day, &week, &month, &year);
    if (s_type == MwDaySelected)
    {
	dsbot = DATEFUNCDaysSinceBeginOfTime (day, month, year);
    }
    else
    {
	dsbot = cd->dsbot;
    }

    /*	  
    **  Let's extract the entries from the buffer we got
    */	  
    if (!clip_parse_buffer_for_dayitems(cd, dsbot, buffer, time))
    {
	ERRORDisplayError(cd->mainwid, "ErrorClipParse");
    }

    XtFree (buffer);

    return ClipboardSuccess;
}

static int get_text_from_clipboard_CS
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time,
	XmString	*return_text)
#else	/* no prototypes */
	(cd, time, return_text)
	CalendarDisplay	cd;
	Time		time;
	XmString	*return_text;
#endif	/* prototype */
{
    register Display	*disp;
    register Window	wind;
    register int	status;
    int			private_id;
    char		*buffer;
    XmString		xm_str;
    long		byte_count, cvt_status;
    char		*target;
    

    status = ClipboardSuccess;
    disp = XtDisplay(cd->mainwid);
    wind = XtWindow(cd->mainwid);

    /*
    ** lock the clipboard for retrieve.
    */
    status = XmClipboardStartRetrieve (disp, wind, time);
    if (status == ClipboardLocked)
    {
	ERRORDisplayError (cd->mainwid, "ErrorClipGet");
	return (status);
    }

    status = DWI18n_ClipboardPaste (disp, wind, &xm_str, &private_id);

    switch (status)
    {
    case ClipboardNoData:
	ERRORDisplayError (cd->mainwid, "ErrorClipNoData");
	return (status);
    case ClipboardTruncate:
	ERRORDisplayError (cd->mainwid, "ErrorClipTruncate");
	return (status);
    case ClipboardFail:
    case ClipboardLocked:
	ERRORDisplayError (cd->mainwid, "ErrorClipGet");
	return (status);
    }

    (void)XmClipboardEndRetrieve (disp, wind);

    *return_text = xm_str;

    return (status);
    
}

static int copy_text_from_clipboard
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time,
	Widget		tw)
#else	/* no prototypes */
	(cd, time, tw)
	CalendarDisplay	cd;
	Time		time;
	Widget		tw;
#endif	/* prototype */
{
    struct DWC$db_interchange   *work_context;
    int			clipboard_status;
    int			err_line;
    char		*buffer;
    char		*texts;
    int			status;
    Cardinal		insert_point;
    Arg			arglist [1];
    XmString		xm_text;
    long		byte_count, cvt_status;
    

    clipboard_status = ClipboardLocked;

    clipboard_status = get_text_from_clipboard_CS (cd, time, &xm_text);
    if ( clipboard_status == ClipboardSuccess)
    {
	/*
	** Temorarily to a convert to FC for interchange parse.  Later try
	** to parse CS directly.
	*/
	XtSetArg (arglist [0], XmNcursorPosition, &insert_point);
	XtGetValues (tw, arglist, 1);
	status = DWCDB_ParseInterchange
	    (cd->cab, xm_text, &work_context, &err_line);
	if (status == DWC$k_db_normal)
	{
	    XmStringFree (xm_text);
	    (void) DWCDB_GetAllITexts (cd->cab, work_context, &xm_text);
	    (void) DWCDB_RundownInterchange (cd->cab, work_context);

	    DXmCSTextReplace (tw, insert_point, insert_point, xm_text);
	}
	else
	{
	    DXmCSTextReplace (tw, insert_point, insert_point, xm_text);
	}

	XmStringFree (xm_text);

	return (clipboard_status);
    }

    return (clipboard_status);
    
}

static void copy_or_cut_to_clipboard
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time,
	Boolean		delete)
#else	/* no prototypes */
	(cd, time, delete)
	CalendarDisplay	cd;
	Time		time;
	Boolean		delete;
#endif	/* prototype */
{
    DSWEntryDataStruct	data;
    DwcDswEntry		entry;
    DwcDayItem		di;
    DwcDaySlots		ds;
    Widget		text_widget;
    XmString		select_text;
    Cardinal		i;
    int			status;
    long		first;
    long		last;
    DwcDayItem		*day_items;
    char		**txt_items;
    char		*buffer;
    Cardinal		buf_len;
    XmString		xm_text;
    long		byte_count, cvt_status;
    long		*text_lengths;
    

    /*	  
    **  See if we own the selection and if so then translate the selection owner
    **	window to a widget and make sure it is our widget.
    */	  
    if (! CLIPTestSelectionWidget (cd, &text_widget))
    {
	return; /* Uh? */
    }
    
    if (text_widget != NULL)
    {
	xm_text = DXmCSTextGetSelection (text_widget);
    
	if (!XmStringEmpty(xm_text))
	{
	    status = copy_text_to_clipboard (cd, time, xm_text);
	    XmStringFree(xm_text);
	    if (status == ClipboardLocked)
	    {
		ERRORDisplayError(cd->mainwid, "ErrorClipCopy");
	    }
	    else
	    {
		if (delete)
		{
		    DXmCSTextGetSelectionInfo (text_widget, &first, &last);
		    xm_text = DXmCvtFCtoCS("", &byte_count, &cvt_status);
		    DXmCSTextReplace(text_widget, first, last, xm_text);
		    XmStringFree(xm_text);
		}
		DXmCSTextClearSelection (text_widget, time);
	    }
	}
	else
	{
	    XmStringFree(xm_text);
	}

	return;
    }


    /*	  
    **  Allocate memory to hold a DwcDayItem structure for each selected entry
    **	day_items will contain duplicates of the DwcDayItem structures for each
    **	entry selected, and txt_items will contain pointers to the text for each
    **	entry.
    */	  
    day_items = (DwcDayItem *) XtMalloc
	(sizeof (DwcDayItem) * cd->number_of_selected_entries);
    txt_items = (char **) XtMalloc
	(sizeof (char *) * cd->number_of_selected_entries);
    text_lengths = (long *) XtMalloc
	(sizeof (long) * cd->number_of_selected_entries);

    for (i = 0;  i < cd->number_of_selected_entries;  i++)
    {
	entry = cd->selected_entries [i];
	DSWGetEntryData (entry, &data);
	xm_text = DSWGetEntryCSText (entry);

	di = (DwcDayItem) data.tag;
	if (di == NULL)
	{
	    if (data.parent == cd->daynotes.widget)
	    {
		di = DAYCloneDayItem (cd, cd->daynotes.default_item);
	    }
	    else
	    {
		di = DAYCloneDayItem (cd, cd->dayslots.default_item);
	    }
	    di->start_time = data.start;
	    di->duration   = data.duration;
	}
	else
	{
	    di = DAYCloneDayItem (cd, di);
	}

	day_items [i] = di;
	txt_items [i] = DXmCvtCStoDDIF (xm_text, &text_lengths[i], &cvt_status);
    }


    /*	  
    **  Day_items and txt_items now contain the DwcDayItem info and the text for
    **	the day items for each selected entry.
    */	  
    

    clip_build_buffer_from_day_items
    (
	cd->number_of_selected_entries,
	day_items,
	txt_items,
	text_lengths,
	&buffer,
	&buf_len
    );

    clip_convert_selected_to_interchange_CS (cd, &select_text);

    status  = copy_timeslots_to_clipboard_CS
	(cd, time, buffer, buf_len, select_text);
    if (status != ClipboardSuccess)
    {
	delete = FALSE;
    }

    for (i = 0;  i < cd->number_of_selected_entries;  i++)
    {
	DAYDestroyDayItem (day_items [i]);
	XtFree (txt_items [i]);
    }

    XtFree ((char *)day_items);
    XtFree ((char *)txt_items);
    XtFree ((char *)text_lengths);

    XtFree (buffer);

    if (select_text != NULL)
    {
	XmStringFree (select_text);
    }

    if (delete)
    {
	delete_all_selected (cd, time);
    }

}

void CLIPDO_CUT
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
    CalendarDisplay	    cd;
    int			    status;
    Time		    time;

    if	(!MISCFindCalendarDisplay (&cd, w) )
	return;

    /*	  
    **  This is a CUT
    */	  
    time = MISCGetTimeFromAnyCBS(cbs);
    copy_or_cut_to_clipboard (cd, time, TRUE);

}

void DO_TSE_CUT
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
    long		first, last;
    Sloteditor		se = (Sloteditor) tag;
    CalendarDisplay	cd = se->cd;
    Time		time;
    XmString		xm_text;
    int			status;
    long		byte_count, cvt_status;

    xm_text = DXmCSTextGetSelection
	(SLOTEDITORWIDGETS (se) [k_ts_timeslot_stext]);

    if (XmStringEmpty(xm_text))
    {
	XmStringFree(xm_text);
	return;
    }

    time = MISCGetTimeFromAnyCBS(cbs);

    status = copy_text_to_clipboard (cd, time, xm_text);
    XmStringFree (xm_text);
    if (status == ClipboardLocked)
    {
	ERRORDisplayError
	    (SLOTEDITORWIDGETS (se) [k_ts_popup_attached_db], "ErrorClipCopy");
    }
    else
    {
	DXmCSTextGetSelectionInfo
	  (SLOTEDITORWIDGETS (se) [k_ts_timeslot_stext], &first, &last);
	DXmCSTextClearSelection
	    (SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext], time);
	xm_text = DXmCvtFCtoCS ("", &byte_count, &cvt_status);
	DXmCSTextReplace
	    (SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext], first, last, xm_text);
	XmStringFree(xm_text);
    }
}

void CLIPDO_COPY
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
    int			    status;
    CalendarDisplay	    cd;
    Time		    time;

    status = MISCFindCalendarDisplay (&cd, w);
    time = MISCGetTimeFromAnyCBS(cbs);
    /*	  
    **  This is a COPY
    */	  
    copy_or_cut_to_clipboard(cd, time, FALSE);

}

void DO_TSE_COPY
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
    Sloteditor		se = (Sloteditor) tag;
    CalendarDisplay	cd = se->cd;
    Time		time;
    XmString		xm_text;
    int			status;

    xm_text = DXmCSTextGetSelection
	(SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext]);

    if (XmStringEmpty(xm_text))
    {
	XmStringFree(xm_text);
	return;
    }

    time = MISCGetTimeFromAnyCBS(cbs);

    status = copy_text_to_clipboard (cd, time, xm_text);
    XmStringFree (xm_text);

    if (status == ClipboardLocked)
    {
	ERRORDisplayError
	    (SLOTEDITORWIDGETS (se) [k_ts_popup_attached_db], "ErrorClipCopy");
    }
    else
    {
	DXmCSTextClearSelection 
	    (SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext], time);
    }

}

void CLIPDO_PASTE
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
    Time		    time;
    Widget		    text_widget;
    ClipContents	    cbc;
    char		    *buffer;
    CalendarDisplay	    cd;
    int			    status;
    XmString		    xm_text;
    long		    byte_count, cvt_status;

    /*
    ** We're the Heckawi!?!
    */
    status = MISCFindCalendarDisplay (&cd, w);

    /*
    ** What time is it?
    */
    time = MISCGetTimeFromAnyCBS (cbs);

    /*
    ** What type of clipboard contents.
    */
    cbc = CLIPTestClipboardContents (cd->mainwid);

    if (cbc == ClipHasNothing)
    {
	return; /* uh? */
    }
    
    /*
    ** Are we currently editing a slot?
    */
    if (!CLIPTestEntryOpen(cd,&text_widget))
    {
	/*
	** There are no entries open.  Therefore, we will be creating entries
	** from the clipboard contents.
	*/
	if (cbc == ClipHasEntry)
	{
	    /*
	    ** We've got entries in the clipboard.  Create timeslots from
	    ** them.
	    */
	    copy_timeslots_from_clipboard (cd, time);
	}	    
	else
	{
	    /*
	    ** We've got a string in the clipboard, either plain text or
	    ** entries in the text interchange format. we need to figure out
	    ** what to do with it.
	    */
	    if (get_text_from_clipboard_CS (cd, time, &xm_text) == ClipboardSuccess)
	    {
		if (!CLIPImportInterchange (cd, xm_text, time))
		{
		    /*
		    ** Not in interchange format.
		    */
		    ERRORDisplayError(cd->mainwid, "ErrorClipSyntax");
		}
	    }
	    else
	    {
		/*
		** Unable to get text from clipboard.
		*/
		ERRORDisplayError(cd->mainwid, "ErrorClipGet");
	    }
	    XmStringFree (xm_text);
	}
    }
    else
    {
	/*	  
	** We've got a text_widget editable so dump the clipboard contents
	** into it as text.
	*/
	if (copy_text_from_clipboard (cd, time, text_widget) == ClipboardLocked)
	{
	    ERRORDisplayError(cd->mainwid, "ErrorClipGet");
	}
    }


}

void DO_TSE_PASTE
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
    Sloteditor		se = (Sloteditor) tag;
    CalendarDisplay	cd = se->cd;
    Time		time;
    int			status;
    ClipContents	    cbc;

    time = MISCGetTimeFromAnyCBS(cbs);
    cbc = CLIPTestClipboardContents(cd->mainwid);
    if ((cbc != ClipHasNothing) && (cbc != ClipHasEntry))
    {
	status = copy_text_from_clipboard
	    (cd, time, SLOTEDITORWIDGETS (se) [k_ts_timeslot_stext]);
	if (status == ClipboardLocked)
	{
	    ERRORDisplayError
		(SLOTEDITORWIDGETS (se)[k_ts_timeslot_stext], "ErrorClipGet");
	}
    }
}

void CLIPDO_CLEAR
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
    Time		    time;
    long		    first, last;
    Widget		    text_widget;
    CalendarDisplay	    cd;
    int			    status;
    XmString		    xm_text;
    long		    byte_count, cvt_status;


    status = MISCFindCalendarDisplay (&cd, w);
    time = MISCGetTimeFromAnyCBS (cbs);

    /*	  
    **  See if we own the selection and if so then translate the selection owner
    **	window to a widget and make sure it is our widget.
    */	  
    if (! CLIPTestSelectionWidget (cd, &text_widget))
    {
	return; /* Uh? */
    }

    if (text_widget == NULL)
    {
	delete_all_selected(cd, time);
    }
    else
    {
	DXmCSTextGetSelectionInfo (text_widget, &first, &last);
	DXmCSTextClearSelection (text_widget, time);
	xm_text = DXmCvtFCtoCS ("", &byte_count, &cvt_status);
	DXmCSTextReplace (text_widget, first, last, xm_text);
	XmStringFree (xm_text);
    }
}

void DO_TSE_CLEAR
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
    Time		    time;
    long		first, last;
    Sloteditor		se = (Sloteditor) tag;
    CalendarDisplay	cd = se->cd;
    XmString		xm_text;
    long		byte_count, cvt_status;

    time = MISCGetTimeFromAnyCBS (cbs);

    DXmCSTextGetSelectionInfo 
	(SLOTEDITORWIDGETS (se) [k_ts_timeslot_stext], &first, &last);
	
    DXmCSTextClearSelection       
	(SLOTEDITORWIDGETS (se) [k_ts_timeslot_stext], time);
    
    xm_text = DXmCvtFCtoCS("", &byte_count, &cvt_status);
    DXmCSTextReplace              
	(SLOTEDITORWIDGETS (se) [k_ts_timeslot_stext], first, last, xm_text);
    XmStringFree(xm_text);
}

void CLIPDO_SELALL
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
    Time		    time;
    Widget		    text_widget;
    CalendarDisplay	    cd;
    int			    string_length;

    MISCFindCalendarDisplay (&cd, w);
    time = MISCGetTimeFromAnyCBS (cbs);

    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */

    /*	  
    **  Do we have an entry open?
    */	  
    if (CLIPTestEntryOpen(cd, &text_widget))
    {
	/*	  
	**  Yes, we've got an entry open, set the selection to the text in the
	**  entry
	*/	  
	string_length = DXmCSTextGetLastPosition ((DXmCSTextWidget)text_widget);
	if (string_length != 0)
	    DXmCSTextSetSelection (text_widget, 0, string_length, time);
    }
    else
    {
        /*	  
	**  Nope, no entry open. We want to go off and select all the daynotes
	**  and dayslots on this day.
	*/	  


        /*	  
	**  Clear out any currently selected ones (since we'll be selecting all
	**  of them).
	*/	  
        XtFree ((char *)cd->selected_entries);
	cd->selected_entries = NULL;
	cd->number_of_selected_entries = 0;

        /*	  
	**  Go do the selection.
	*/	  
        DSWCallSelectExtendOnAll ((DayslotsWidget) cd->daynotes.widget, time);
	DSWCallSelectExtendOnAll ((DayslotsWidget) cd->dayslots.widget, time);
    }

}

void DO_TSE_SELALL
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
    Time		time;
    Sloteditor		se = (Sloteditor) tag;
    Widget		text_widget;
    CalendarDisplay	cd = se->cd;
    int			string_length;

    time = MISCGetTimeFromAnyCBS (cbs);

    text_widget = SLOTEDITORWIDGETS (se) [k_ts_timeslot_stext];
    string_length = DXmCSTextGetLastPosition ((DXmCSTextWidget)text_widget);
    if (string_length != 0)
	DXmCSTextSetSelection (text_widget, 0, string_length, time);
}

static void delete_all_selected
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	(cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif	/* prototype */
{
    Cardinal		    i;
    DSWEntryDataStruct   data;
    DwcDswEntry		    entry;
    DwcDayItem		    di;
    DwcDaySlots		    ds;
    DayItemUpdate	    diu;

    if (cd->read_only) {
	return;
    } else {
	for (i = 0;  i < cd->number_of_selected_entries;  i++) {
	    if (! DSWGetEntryEditable (cd->selected_entries [i])) {
		return;
	    }
	}
    }

    while (cd->number_of_selected_entries > 0) {
	entry = cd->selected_entries [0];
	DSWGetEntryData (entry, &data);

	if (data.duration == 0) {
	    ds = &(cd->daynotes);
	} else {
	    ds = &(cd->dayslots);
	}

        /*	  
        **  Check to see if this entry has a DwcDayItem or not
        */	  
        if (data.tag == NULL)
	{
	    DAYRemoveEntry(cd, ds, entry, time);
	}
	else
	{
	    di  = DAYCloneDayItem (cd, (DwcDayItem) data.tag);
	    diu = DAYCreateDayItemUpdateRecord
		(cd, cd->dsbot, NULL, di, NULL, RCKNotRepeat);

	    if (di->repeat_p1 != DWC$k_db_none)
	    {
		diu->kind = RCKThisInstance;
	    }

	    diu->slots = ds;

	    (void) DAYDoDayItemUpdate (diu, time);
	    DAYDestroyDayItemUpdate (diu);
	}
	
    }

}

Boolean CLIPImportInterchange
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	XmString	buffer,
	Time		time)
#else	/* no prototypes */
	(cd, buffer, time)
	CalendarDisplay	cd;
	XmString	buffer;
	Time		time;
#endif	/* prototype */
{
    struct DWC$db_interchange   *work_context;
    int				dsbot;
    int				start;
    int				duration;
    char			*text;
    int				text_length;
    int				text_class;
    char			*slot_text;
    XmString			xm_text;
    DwcDayItem			di;
    DwcDaySlots			ds;
    DayItemUpdate		diu;
    int				status;
    int				err_line;
    long			byte_count, cvt_status;

    status = DWCDB_ParseInterchange
	(cd->cab, buffer, &work_context, &err_line);
    if (status != DWC$k_db_normal)
    {
	return (FALSE);
    }

    while (TRUE)
    {
	status = DWCDB_GetNextIItem
	(
	    cd->cab,
	    work_context,
	    &dsbot,
	    &start,
	    &duration,
	    &xm_text,
	    &text_length,
	    &text_class
	);
	if (status != DWC$k_db_normal) break;

	if (duration == 0)
	{
	    ds = &(cd->daynotes);
	}
	else
	{
	    ds = &(cd->dayslots);
	}
	
	di  = DAYCloneDayItem (cd, ds->default_item);

	di->start_time = start;
	di->duration   = duration;

	diu = DAYCreateDayItemUpdateRecord
	    (cd, dsbot, xm_text, NULL, di, RCKNotRepeat);
	diu->slots = ds;

	(void) DAYDoDayItemUpdate (diu, time);
	DAYDestroyDayItemUpdate (diu);

    }

    (void) DWCDB_RundownInterchange (cd->cab, work_context);

    return (TRUE);

}

static char *format_date
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	int		year,
	int		month,
	int		day,
	Boolean		is_entry)
#else	/* no prototypes */
	(cd, year, month, day, is_entry)
	CalendarDisplay	cd;
	int		year;
	int		month;
	int		day;
	Boolean		is_entry;
#endif	/* prototype */
{
    dtb			date_time;
    char		*time_str;
    int			time_fmt;

    
    date_time.year    = year;
    date_time.month   = month;
    date_time.day     = day;
    date_time.weekday = DATEFUNCDayOfWeek (day, month, year);
        
    if (is_entry)
    {
	time_fmt = dwc_k_clip_entry_date_format;
    }
    else
    {
	time_fmt = dwc_k_clip_note_date_format;
    }

    return (DATEFORMATTimeToText (time_fmt, &date_time));

}
    
static char *format_time
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	int		time_of_day,
	Boolean		is_start)
#else	/* no prototypes */
	(cd, time_of_day, is_start)
	CalendarDisplay	cd;
	int		time_of_day;
	Boolean		is_start;
#endif	/* prototype */
{
    dtb			date_time;
    char		*time_str;
    int			time_fmt;

    
    date_time.hour   = time_of_day / 60;
    date_time.minute = time_of_day % 60;

    if (is_start)
    {
	if (cd->profile.time_am_pm)
	{
	    time_fmt = dwc_k_clip_from_ampm_format;
	}
	else
	{
	    time_fmt = dwc_k_clip_from_format;
	}
    }
    else
    {
	if (cd->profile.time_am_pm)
	{
	    time_fmt = dwc_k_clip_to_ampm_format;
	}
	else
	{
	    time_fmt = dwc_k_clip_to_format;
	}
    }

    return (DATEFORMATTimeToText (time_fmt, &date_time));
    
}

static Boolean clip_convert_selected_to_interchange_CS
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	XmString	*return_text)
#else	/* no prototypes */
	(cd, return_text)
	CalendarDisplay	cd;
	XmString	*return_text;
#endif	/* prototype */
{
    struct DWC$db_interchange   *work_context;
    DSWEntryDataStruct		data;
    XmString			xm_text;
    DwcDswEntry			entry;
    Cardinal			i;
    int				status;


    status = DWCDB_CreateIHandle (cd->cab, &work_context);
    if (status != DWC$k_db_normal)
    {
	return (FALSE);
    }

    for (i = 0; i < cd->number_of_selected_entries; i++)
    {
	entry = cd->selected_entries [i];
	DSWGetEntryData (entry, &data);
	xm_text = DSWGetEntryCSText (entry);

	(void) DWCDB_PutIItem
	(
	    cd->cab,
	    work_context,
	    cd->dsbot,
	    data.start,
	    data.duration,
	    xm_text,
	    DWC$k_item_cstr
	);

	XmStringFree (xm_text);
    }

    status = DWCDB_WriteInterchange
    (
	cd->cab,
	work_context,
	format_date,
	format_time,
	cd,
	&xm_text
    );

    (void) DWCDB_RundownInterchange (cd->cab, work_context);

    *return_text = xm_text;
    
    return (status == DWC$k_db_normal);

}

static Boolean clip_convert_selection
#ifdef _DWC_PROTO_
	(
	Widget		w,
	Atom		*selection,
	Atom		*target,
	Atom		*type,
	XtPointer	*value,
	unsigned long	*length,
	int		*format)
#else	/* no prototypes */
	(w, selection, target, type, value, length, format)
	Widget		w;
	Atom		*selection;
	Atom		*target;
	Atom		*type;
	XtPointer	*value;
	unsigned long	*length;
	int		*format;
#endif	/* prototype */
{    
    char		*slot_text;
    XmString		xm_text;
    long		byte_count, cvt_status;
    Atom		*targets_array;
    CalendarDisplay	cd;
    Cardinal		i;
    int			status;


    if (*selection != XA_PRIMARY)
    {
	/*								    
	**  We only support primary selection.  Period.
	*/
	return (FALSE);
    }

    status = MISCFindCalendarDisplay (&cd, w);

    if (cd->number_of_selected_entries == 0)
    {
	return (FALSE); /* ? Eh? */
    }

    if (*target == SMAtomTargets)
    {
	/*
	**  we build an array of atoms			    
	*/
	targets_array = (Atom *) XtMalloc (5 * sizeof (Atom));
	targets_array [0] = SMAtomCalendarEntries;
	targets_array [1] = SMAtomDDIF;
	targets_array [2] = SMAtomCOMPOUND_TEXT;
	targets_array [3] = XA_STRING;
	targets_array [4] = SMAtomTargets;
	*value  = (XtPointer) targets_array;
	*length = 5;
	*format = 32;
	*type   = XA_ATOM;
	return (TRUE);
    }

    if (*target == SMAtomCalendarEntries)
    {
	Cardinal    buf_len;
	*value  = (XtPointer) clip_build_interchange_buffer (cd, &buf_len);
	*length = (unsigned long) buf_len;
	*format = 8;
	*type   = *target;

	return (TRUE);
    }

    if (*target == SMAtomDDIF || *target == SMAtomCOMPOUND_TEXT)
    {

	if (!clip_convert_selected_to_interchange_CS (cd, &xm_text))
	{
	    return (FALSE);
	}

	if (*target == SMAtomDDIF)
	{
	    *value = (XtPointer)DXmCvtCStoDDIF
		(xm_text, &byte_count, &cvt_status);
	    *length = byte_count;
	}
	else
	{
	    *value = (XtPointer)XmCvtXmStringToCT (xm_text);
	    *length = strlen(*value);
	}
	*format = 8;
	*type = *target;

	XmStringFree (xm_text);

	return (TRUE);
    }

    if (*target == XA_STRING)
    {
	if (!clip_convert_selected_to_interchange_CS (cd, &xm_text))
	{
	    return (FALSE);
	}

	slot_text = DXmCvtCStoFC (xm_text, &byte_count, &cvt_status);
	*length = byte_count - 1;   /* length does not count the terminator */
	*value  = (XtPointer)slot_text;
	*format = 8;
	*type   = XA_STRING;

	XmStringFree (xm_text);

	return (TRUE);
    }

    return (FALSE);

}

static char *clip_build_interchange_buffer
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Cardinal	*buf_len)
#else	/* no prototypes */
	(cd, buf_len)
	CalendarDisplay	cd;
	Cardinal	*buf_len;
#endif	/* prototype */
{
    DSWEntryDataStruct	data;
    DwcDswEntry		entry;
    DwcDayItem		di;
    DwcDaySlots		ds;
    Cardinal		i;
    int			status;
    long		first;
    long		last;
    DwcDayItem		*day_items;
    char		**txt_items;
    char		*buffer;
    XmString		xm_text;
    long		byte_count, cvt_status;
    long		*text_lengths;
    

    /*	  
    **  Allocate memory to hold a DwcDayItem structure for each selected entry
    **	day_items will contain duplicates of the DwcDayItem structures for each
    **	entry selected, and txt_items will contain pointers to the text for each
    **	entry.
    */	  
    day_items = (DwcDayItem *) XtMalloc
	(sizeof (DwcDayItem) * cd->number_of_selected_entries);
    txt_items = (char **) XtMalloc
	(sizeof (char *) * cd->number_of_selected_entries);
    text_lengths = (long *) XtMalloc
	(sizeof (long) * cd->number_of_selected_entries);

    for (i = 0;  i < cd->number_of_selected_entries;  i++)
    {
	entry = cd->selected_entries [i];
	DSWGetEntryData (entry, &data);
	xm_text = DSWGetEntryCSText (entry);

	di = (DwcDayItem) data.tag;
	if (di == NULL)
	{
	    if (data.parent == cd->daynotes.widget)
	    {
		di = DAYCloneDayItem (cd, cd->daynotes.default_item);
	    }
	    else
	    {
		di = DAYCloneDayItem (cd, cd->dayslots.default_item);
	    }
	    di->start_time = data.start;
	    di->duration   = data.duration;
	}
	else
	{
	    di = DAYCloneDayItem (cd, di);
	}

	day_items [i] = di;
	txt_items [i] = DXmCvtCStoDDIF (xm_text, &text_lengths[i], &cvt_status);
    }


    /*	  
    **  Day_items and txt_items now contain the DwcDayItem info and the text for
    **	the day items for each selected entry.
    */	  
    
    clip_build_buffer_from_day_items
    (
	cd->number_of_selected_entries,
	day_items,
	txt_items,
	text_lengths,
	&buffer,
	buf_len
    );

    for (i = 0;  i < cd->number_of_selected_entries;  i++)
    {
	DAYDestroyDayItem (day_items [i]);
	XtFree (txt_items [i]);
    }

    XtFree ((char *)day_items);
    XtFree ((char *)txt_items);
    XtFree ((char *)text_lengths);

    return (buffer);
}

static void lose_selection
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Atom	*selection)
#else	/* no prototypes */
	(w, selection)
	Widget	w;
	Atom	*selection;
#endif	/* prototype */
{    
    Cardinal		i;
    CalendarDisplay	cd;
    Window		newsel;
    Widget		widget;
    int			status;

    if (*selection != XA_PRIMARY)
    {
	/*								    
	**  We only support primary selection.  Period.
	*/
	return;
    }

    status = MISCFindCalendarDisplay (&cd, w);

    for (i = 0;  i < cd->number_of_selected_entries;  i++)
    {
	DSWSetEntrySelected(cd->selected_entries [i], FALSE);
    }

    newsel = XGetSelectionOwner (XtDisplay (w), XA_PRIMARY);
    if (newsel != None)
    {
	widget = XtWindowToWidget (XtDisplay (w), newsel);
	while (widget != NULL)
	{
	    if (widget == (Widget)cd->lw_day_display)
	    {
		XtFree ((char *)cd->selected_entries);
		cd->selected_entries = NULL;
		cd->number_of_selected_entries = 0;
		break;
	    }
	    widget = XtParent (widget);
	}
    }
}

Boolean CLIPSMTestSelectionOurs
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{    
    Window		selwin;
    Widget		selwid;
    Display		*d = XtDisplay (cd->mainwid);

    selwin = XGetSelectionOwner (d, XA_PRIMARY);
    if (selwin == None) {
	return (FALSE);
    }

    selwid = XtWindowToWidget (d, selwin);

    if (selwid == cd->mainwid)
    {
	return (cd->number_of_selected_entries != 0);
    }

    return (FALSE);

}

static Boolean test_widget_ours
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Widget		widget)
#else	/* no prototypes */
	(cd, widget)
	CalendarDisplay	cd;
	Widget		widget;
#endif	/* prototype */
{
    Widget		parent;

    parent = widget;
    while (parent != NULL) {
	if (parent == cd->toplevel) {
	    return (TRUE);
	}
	parent = XtParent (parent);
    }

    return (FALSE);

}

/* returns true and the text_widget id if an entry is open, otherwise it    */
/* returns false */
Boolean CLIPTestEntryOpen
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Widget		*text_widget)
#else	/* no prototypes */
	(cd, text_widget)
	CalendarDisplay	cd;
	Widget		*text_widget;
#endif	/* prototype */
{    
    DwcDswEntry	    open_entry;
    Boolean	    return_value;

    return_value = FALSE;
    
    /*	  
    **  First check to see if a dayslot is open
    */	  
    open_entry = DSWGetOpenEntry((DayslotsWidget)cd->dayslots.widget);
    if (open_entry != NULL)
	{
	*text_widget = DSWGetEntryTextWidget (open_entry);
	return_value = TRUE;
	}
    else
	{
        /*	  
	**  No dayslot open, check the daynote
	*/	  
        open_entry = DSWGetOpenEntry((DayslotsWidget)cd->daynotes.widget);
	if (open_entry != NULL)
	    {
	    *text_widget = DSWGetEntryTextWidget (open_entry);
	    return_value = TRUE;
	    }
	else
	    {
            /*	  
	    **  No Dayslot or daynote open
	    */	  
            return_value = FALSE;
	    }
	}

    return return_value;
}

/*	  
**  See if we own the selection and if so then translate the selection owner
**	window to a widget and make sure it is our widget.
*/	  
Boolean CLIPTestSelectionWidget
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Widget		*text_widget)
#else	/* no prototypes */
	(cd, text_widget)
	CalendarDisplay	cd;
	Widget		*text_widget;
#endif	/* prototype */
{    
    Window		selwin;
    Widget		selwid;
    Display		*d = XtDisplay (cd->mainwid);

    *text_widget = NULL;

    selwin = XGetSelectionOwner (d, XA_PRIMARY);

    if (selwin == None) {
	return (FALSE);
    }

    selwid = XtWindowToWidget (d, selwin);

    if (selwid == NULL) {
	return (FALSE);
    }
    if (selwid == cd->mainwid) {
	return (cd->number_of_selected_entries != 0);
    }

    if (! XtIsSubclass (selwid, dxmCSTextWidgetClass))
    {
	return (FALSE);
    }

    if (test_widget_ours (cd, selwid))
    {
	*text_widget = selwid;
	return (TRUE);
    }

    return (FALSE);

}

void CLIPMainwidOwnPRIMARY
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	(cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif	/* prototype */
{
    Cardinal		i;
    Boolean		status;

    /*	  
    **  Do we have any selected entries?
    */	  
    if (cd->number_of_selected_entries != 0)
    {
        /*	  
	**  Yes, try to own the PRIMARY selection
	*/
	status = XtOwnSelection
	(
	    cd->mainwid,
	    XA_PRIMARY,
	    time,
	    clip_convert_selection,
	    lose_selection,
	    (XtSelectionDoneProc)NULL
	);
        if (status)
	{
            /*	  
	    **  We successfully own the PRIMARY selection
	    */	  
            for (i = 0;  i < cd->number_of_selected_entries;  i++)
	    {
		DSWSetEntrySelected(cd->selected_entries [i], TRUE);
	    }
	}
    }

}

void CLIPSMSetSelected
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	DwcDswEntry	entry,
	Time		time)
#else	/* no prototypes */
	(cd, entry, time)
	CalendarDisplay	cd;
	DwcDswEntry	entry;
	Time		time;
#endif	/* prototype */
{
    Cardinal		i;
    Boolean		reset = TRUE;
    TimeslotWidget  tsw;
    XmOffsetPtr	    o;


    /*	  
    **  Let's see if we have to do anything at all. If we have more than one
    **	entry selected then we're going to deselect them. If we have one entry
    **	and it is the requested one, then this is basically a noop.
    */	  
    if (cd->number_of_selected_entries == 1)
    {
	reset = (cd->selected_entries [0] != entry);
    }

    /*	  
    **  Either there were a bunch selected, or the single one selected wasn't
    **	the one we've been asked to select.
    */	  
    if (reset) {
        /*	  
	**  Go deselect the selected entries.
	*/	  
        for (i = 0;  i < cd->number_of_selected_entries;  i++) {
	    DSWSetEntrySelected(cd->selected_entries [i], FALSE);
	}

        /*	  
	**  Make sure we have enough room then remember what we've selected.
	*/	  
        cd->selected_entries = (DwcDswEntry *) XtRealloc
	    ((char *)cd->selected_entries, sizeof (DwcDswEntry));
	cd->number_of_selected_entries = 1;
	cd->selected_entries [0] = entry;
    }

    /*	  
    **  Make sure we've got the input focus
    */
    tsw = (TimeslotWidget) entry->timeslot;
    o = TswOffsetPtr(tsw);

    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */
    /*	  
    **  Make sure we've got the input focus
    */	  
    (void) XtCallAcceptFocus (cd->active_widget, &time);


}

void CLIPSMAddSelected
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	DwcDswEntry	entry,
	Time		time)
#else	/* no prototypes */
	(cd, entry, time)
	CalendarDisplay	cd;
	DwcDswEntry	entry;
	Time		time;
#endif	/* prototype */
{
    Cardinal		i;
    TimeslotWidget  tsw;
    XmOffsetPtr	    o;

    for (i = 0;  i < cd->number_of_selected_entries;  i++)
    {
	if (entry == cd->selected_entries [i])
	{
	    return;
	}
    }

    i = cd->number_of_selected_entries;
    cd->number_of_selected_entries++;
    cd->selected_entries = (DwcDswEntry *) XtRealloc
    (
	(char *)cd->selected_entries,
	sizeof (DwcDswEntry) * cd->number_of_selected_entries
    );

    cd->selected_entries [i] = entry;

    /*	  
    **  Make sure we've got the input focus
    */
    tsw = (TimeslotWidget) entry->timeslot;
    o = TswOffsetPtr(tsw);

    /*
    ** MAYBE AN XSetInputFocus TO THE SHELL'S WINDOW HERE!
    */
    /*	  
    **  Make sure we've got the input focus
    */	  
    (void) XtCallAcceptFocus (cd->active_widget, &time);


}

void
CLIPDeselectAllEntries
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	(cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif	/* prototype */
{
    Cardinal		i;

    for (i = 0;  i < cd->number_of_selected_entries;  i++)
    {
	DSWSetEntrySelected(cd->selected_entries [i], FALSE);
    }

    XtFree ((char *)cd->selected_entries);
    cd->selected_entries = NULL;
    cd->number_of_selected_entries = 0;

    /*	  
    **  This gets called when we first start up before we even have an active
    **	widget, in which case we done't have either an active_widget or the
    **	selection.
    */	  
    if (cd->active_widget != NULL)
	XtDisownSelection (cd->mainwid, XA_PRIMARY, time);

}

void CLIPSMRemoveSelected
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	DwcDswEntry	entry,
	Time		time)
#else	/* no prototypes */
	(cd, entry, time)
	CalendarDisplay	cd;
	DwcDswEntry	entry;
	Time		time;
#endif	/* prototype */
{
    Cardinal		i, j;
    Boolean		found = FALSE;

    for (i = 0;  i < cd->number_of_selected_entries;  i++)
    {
	if (entry == cd->selected_entries [i])
	{
	    DSWSetEntrySelected(entry, FALSE);
	    for (j = i + 1;  j < cd->number_of_selected_entries;  j++)
	    {
		cd->selected_entries [j - 1] = cd->selected_entries [j];
	    }
	    found = TRUE;
	    break;
	}
    }

    if (! found)
    {
	return;
    }

    if (cd->number_of_selected_entries == 1)
    {
	CLIPDeselectAllEntries(cd, time);
    }
    else
    {
	cd->number_of_selected_entries--;
	cd->selected_entries = (DwcDswEntry *) XtRealloc
	(
	    (char *)cd->selected_entries,
	    sizeof (DwcDswEntry) * cd->number_of_selected_entries
	);
    }
    

}

void CLIPSMInitialise
#ifdef _DWC_PROTO_
	(
	Widget	toplevel)
#else	/* no prototypes */
	(toplevel)
	Widget	toplevel;
#endif	/* prototype */
{
    Display *disp = XtDisplay(toplevel);
    SMAtomCalendarEntries = XmInternAtom (disp, "Calendar-Entries", FALSE);
    SMAtomDDIF = XmInternAtom (disp, "DDIF", FALSE);
    SMAtomCOMPOUND_TEXT = XmInternAtom (disp, "COMPOUND_TEXT", FALSE);
    SMAtomTargets = XmInternAtom (disp, "TARGETS", FALSE);
    SMAtomKill = XmInternAtom (disp, "KILL_SELECTION", FALSE);
}

typedef struct
{
    Boolean	    waiting_for_answer;
    Atom	    type;
    XtPointer	    value;
    unsigned long   length;
    int		    format;
} SMClosure;

static void got_closure
#ifdef _DWC_PROTO_
	(
	Widget		widget,
	caddr_t		*closure,
	Atom		*selection,
	Atom		*type,
	XtPointer	value,
	int		*length,
	unsigned long	*format)
#else	/* no prototypes */
	(widget, closure, selection, type, value, length, format)
	Widget		widget;
	XtPointer	*closure;
	Atom		*selection;
	Atom		*type;
	XtPointer	value;
	unsigned long	*length;
	int		*format;
#endif	/* prototype */
{
    SMClosure	    *closure_ptr = (SMClosure *) closure;

    closure_ptr->waiting_for_answer = FALSE;

    closure_ptr->type	= *type;
    closure_ptr->value	= value;
    closure_ptr->length = *length;
    closure_ptr->format = *format;

    return;
}

Boolean CLIPSMGetStringSelection
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	XmString	*value,		/* returns  */
	unsigned long	*length,	/* returns	*/
	Time		time)
#else	/* no prototypes */
	(cd, value, length, time)
	CalendarDisplay	cd;
	XmString	*value;		/* returns  */
	unsigned long	*length;	/* returns	*/
	Time		time;
#endif	/* prototype */
{
    SMClosure		*closure_ptr;
    XmString		xm_text;
    long		byte_count, cvt_status;

    closure_ptr = (SMClosure *) XtMalloc (sizeof (SMClosure));
    closure_ptr->waiting_for_answer = TRUE;
    
    XtGetSelectionValue
    (
	cd->mainwid,
	XA_PRIMARY,
	XA_STRING,
	(XtSelectionCallbackProc)got_closure,
	closure_ptr,
	time
    );
			 
    /*
    **	We loop, and process X events, timer events and other events from
    **	alternate input sources until our selection callback gets called.
    */

    while (closure_ptr->waiting_for_answer)
    {
	/*
	**  XtAppProcessEvent returns when it has processed an X event, a
	**  timer, or an event coming from an alternate inut source.
	*/
	XtAppProcessEvent
	    (CALGetAppContext(), XtIMXEvent | XtIMTimer | XtIMAlternateInput);
    }

    if ((closure_ptr->type == None) ||
        (closure_ptr->type == XT_CONVERT_FAIL))
    {
	XtFree ((char *)closure_ptr);
	return (FALSE);
    }

    if (closure_ptr->type != XA_STRING)
    {
	XtFree ((char *)closure_ptr->value);
	XtFree ((char *)closure_ptr);
	return (FALSE);
    }

    xm_text = DXmCvtFCtoCS (closure_ptr->value, &byte_count, &cvt_status);

    *value  = xm_text;
    *length = closure_ptr->length;

    XtFree ((char *)closure_ptr->value);
    XtFree ((char *)closure_ptr);
    return (TRUE);

}

void CLIPSMSendKillSelection
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	(cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif	/* prototype */
{
    XClientMessageEvent	cm;

    cm.window = XGetSelectionOwner (XtDisplay (cd->mainwid), XA_PRIMARY);
    if (cm.window == None) {
	return;
    }
    cm.type         = ClientMessage;
    cm.display      = XtDisplay (cd->mainwid);
    cm.message_type = SMAtomKill;
    cm.format       = 32;
    cm.data.l [0]   = (long) XA_PRIMARY;
    cm.data.l [1]   = (long) time;

    XSendEvent (cm.display, cm.window, True, NoEventMask, (XEvent *) &cm);

}

void CLIPReceiveClientMessage
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			tag,
	XClientMessageEvent	*event)
#else	/* no prototypes */
	(w, tag, event)
	Widget			w;
	caddr_t			tag;
	XClientMessageEvent	*event;
#endif	/* prototype */
{
    CalendarDisplay	cd = (CalendarDisplay) tag;

    if (event->type != ClientMessage) {
	return;
    }

    if ((event->message_type != SMAtomKill) ||    
        (event->data.l [0]   != (long) XA_PRIMARY)) {
	return;
    }
    
    if (cd->number_of_selected_entries == 0) {
	return;
    }
    
    delete_all_selected(cd, (Time) event->data.l [1]);

}

ClipContents CLIPSMTestTargets
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	(cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif
{
    SMClosure		*closure_ptr;
    XmString		xm_text;
    long		byte_count, cvt_status;
    int			i;
    Atom		*targets_array;

    closure_ptr = (SMClosure *) XtMalloc (sizeof (SMClosure));
    closure_ptr->waiting_for_answer = TRUE;

    XtGetSelectionValue
    (
	cd->mainwid,
	XA_PRIMARY,
	SMAtomTargets,
	(XtSelectionCallbackProc)got_closure,
	closure_ptr,
	time
    );
			 
    /*
    **	We loop, and process X events, timer events and other events from
    **	alternate input sources until our selection callback gets called.
    */

    while (closure_ptr->waiting_for_answer)
    {
	/*
	**  XtAppProcessEvent returns when it has processed an X event, a
	**  timer, or an event coming from an alternate inut source.
	*/
	XtAppProcessEvent
	    (CALGetAppContext(), XtIMXEvent | XtIMTimer | XtIMAlternateInput);
    }

    if ((closure_ptr->type == None) ||
        (closure_ptr->type == XT_CONVERT_FAIL))
    {
	XtFree ((char *)closure_ptr);
	return (ClipHasNothing);
    }

    if ((closure_ptr->type != SMAtomTargets) &&
	(closure_ptr->type != XA_ATOM))
    {
	XtFree ((char *)closure_ptr->value);
	XtFree ((char *)closure_ptr);
	return (ClipHasNothing);
    }

    targets_array = (Atom *) closure_ptr->value;

    for (i = 0; i < closure_ptr->length; i++)
    {
	if (targets_array[i] == SMAtomCalendarEntries)
	{
	    XtFree ((char *)closure_ptr->value);
	    XtFree ((char *)closure_ptr);
	    return (ClipHasEntry);
	}
	else if (targets_array [i] == SMAtomDDIF)
	{
	    XtFree ((char *)closure_ptr->value);
	    XtFree ((char *)closure_ptr);
	    return (ClipHasDDIF);
	}
	else if (targets_array [i] == SMAtomCOMPOUND_TEXT)
	{
	    XtFree ((char *)closure_ptr->value);
	    XtFree ((char *)closure_ptr);
	    return (ClipHasCT);
	}
	else if (targets_array [i] == XA_STRING)
	{
	    XtFree ((char *)closure_ptr->value);
	    XtFree ((char *)closure_ptr);
	    return (ClipHasString);
	}
    }

    XtFree ((char *)closure_ptr->value);
    XtFree ((char *)closure_ptr);

    return (ClipHasNothing);
}

Boolean CLIPSMGetEntriesSelection
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	Time		time)
#else	/* no prototypes */
	(cd, time)
	CalendarDisplay	cd;
	Time		time;
#endif	/* prototype */
{
    SMClosure		*closure_ptr;
    MwTypeSelected	s_type;
    Cardinal		day;
    Cardinal		week;
    Cardinal		month;
    Cardinal		year;
    Cardinal		dsbot;
    char		*buff;


    closure_ptr = (SMClosure *) XtMalloc (sizeof (SMClosure));
    closure_ptr->waiting_for_answer = TRUE;
    
    XtGetSelectionValue
    (
	cd->mainwid,
	XA_PRIMARY,
	SMAtomCalendarEntries,
	(XtSelectionCallbackProc)got_closure,
	closure_ptr,
	time
    );
			 
    /*
    **	We loop, and process X events, timer events and other events from
    **	alternate input sources until our selection callback gets called.
    */

    while (closure_ptr->waiting_for_answer)
    {
	/*
	**  XtAppProcessEvent returns when it has processed an X event, a
	**  timer, or an event coming from an alternate inut source.
	*/
	XtAppProcessEvent
	    (CALGetAppContext(), XtIMXEvent | XtIMTimer | XtIMAlternateInput);
    }

    if ((closure_ptr->type == None) ||
        (closure_ptr->type == XT_CONVERT_FAIL))
    {
	XtFree ((char *)closure_ptr);
	return (FALSE);
    }

    if (closure_ptr->type != SMAtomCalendarEntries)
    {
	XtFree ((char *)closure_ptr->value);
	XtFree ((char *)closure_ptr);
	return (FALSE);
    }

    /*
    ** Primary selection transfer always goes to the displayed day in the
    ** day view.
    */
    dsbot = cd->dsbot;

    /*
    **  Let's extract the entries from the buffer we got
    */
    buff = (char *)closure_ptr->value;
    if (!clip_parse_buffer_for_dayitems (cd, dsbot, buff, time))
    {
	ERRORDisplayError(cd->mainwid, "ErrorClipParse");
    }

    XtFree ((char *)closure_ptr->value);
    XtFree ((char *)closure_ptr);

    return (TRUE);
}

Boolean CLIPSMGetComplexStringSelection
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	ClipContents	cbc,
	XmString	*value,		/* returns  */
	unsigned long	*length,	/* returns	*/
	Time		time)
#else	/* no prototypes */
	(cd, cbc, value, length, time)
	CalendarDisplay	cd;
	ClipContents	cbc;
	XmString	*value;		/* returns  */
	unsigned long	*length;	/* returns	*/
	Time		time;
#endif	/* prototype */
{
    SMClosure		*closure_ptr;
    XmString		xm_text = NULL;
    long		byte_count = 0, cvt_status;
    Atom		a;
    Boolean		ret_val = False;

    closure_ptr = (SMClosure *) XtMalloc (sizeof (SMClosure));
    closure_ptr->waiting_for_answer = TRUE;

    switch (cbc)
    {
    case ClipHasCT:
	a = SMAtomCOMPOUND_TEXT;
	break;
    case ClipHasDDIF:
	a = SMAtomDDIF;
	break;
    case ClipHasString:
    default:
	a = XA_STRING;
	break;
    }
    XtGetSelectionValue
    (
	cd->mainwid,
	XA_PRIMARY,
	a,
	(XtSelectionCallbackProc)got_closure,
	closure_ptr,
	time
    );
			 
			 
    /*
    **	We loop, and process X events, timer events and other events from
    **	alternate input sources until our selection callback gets called.
    */

    while (closure_ptr->waiting_for_answer)
    {
	/*
	**  XtAppProcessEvent returns when it has processed an X event, a
	**  timer, or an event coming from an alternate inut source.
	*/
	XtAppProcessEvent
	    (CALGetAppContext(), XtIMXEvent | XtIMTimer | XtIMAlternateInput);
    }

    if ((closure_ptr->type == None) ||
        (closure_ptr->type == XT_CONVERT_FAIL))
    {
	XtFree ((char *)closure_ptr);
	return (False);
    }

    if (closure_ptr->type == XA_STRING)
    {
	xm_text = DXmCvtFCtoCS (closure_ptr->value, &byte_count, &cvt_status);
	ret_val = True;
    }
    else if (closure_ptr->type == SMAtomCOMPOUND_TEXT)
    {
	xm_text = XmCvtCTToXmString (closure_ptr->value);
	byte_count = XmStringLength (xm_text);
	ret_val = True;
    }
    else if (closure_ptr->type == SMAtomDDIF)
    {
	xm_text = DXmCvtDDIFtoCS (closure_ptr->value, &byte_count, &cvt_status);
	ret_val = True;
    }

    *value = xm_text;
    *length = byte_count;

    XtFree ((char *)closure_ptr->value);
    XtFree ((char *)closure_ptr);
    return (ret_val);
}

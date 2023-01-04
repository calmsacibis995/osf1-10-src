/* dwc_ui_edit.c */
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
**	Marios Cleovoulou,  March-1988 (DWC_UI_CALENDAR.C)
**	Ken Cowan,	    March-1989
**	Denis G. Lacroix,   April-1989
**
**  ABSTRACT:
**
**	This is the module that deals with edit menus.
**
*--
*/

#include    "dwc_compat.h"

#if defined(vaxc)
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <DXm/DXmCSText.h>
#include <DXm/DECspecific.h>
#if defined(vaxc)
#pragma standard
#endif

#include    "dwc_ui_calendar.h"
#include    "dwc_ui_clipboard.h"	/* for ClipContents */
#include    "dwc_ui_edit.h"	
#include    "dwc_ui_misc.h"		/* for MISCFindCalendarDisplay */
#include    "dwc_ui_monthwidget.h"	/* for MWGetSelection */
#include    "dwc_ui_sloteditor.h"	/* for SLOTEDITORWIDGETS */



void EDITDO_EDIT_MENU
#ifdef _DWC_PROTO_
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
    Cardinal		i ;
    Widget		select_widget ;
    Widget		focus_widget ;
    Boolean		cut_ok ;
    Boolean		copy_ok ;
    Boolean		select_all_ok ;
    Boolean		clear_ok ;
    Boolean		paste_ok ;
    Boolean		mark_ok ;
    ClipContents	CBcontents ;
    MwTypeSelected	s_type ;
    Cardinal		day ;
    Cardinal		week ;
    Cardinal		month ;
    Cardinal		year ;
    CalendarDisplay	cd;
    int			status;

    /*
    **  Not sure exactly why we need to do this
    */
    if (cbs->reason != (int)XmCR_MAP)
    {
	return;
    } 
    
    status = MISCFindCalendarDisplay( &cd, w );

    cut_ok	  = FALSE ;
    copy_ok       = FALSE ;
    clear_ok      = FALSE ;
    paste_ok      = FALSE ;
    select_all_ok = FALSE ;

    mark_ok       = FALSE ;

#if 0	
    CBcontents = CLIPTestClipboardContents (cd->mainwid) ;
#else
    CBcontents = ClipHasString;
#endif

    /*	  
    **  Figure out "Mark" sensitivity
    */	  
    if (! cd->read_only) {
	MWGetSelection(cd->month_display, &s_type, &day, &week, &month, &year);
	mark_ok = ((s_type == MwDaySelected) || (s_type == MwWeekSelected));
    }

    if (CLIPTestEntryOpen(cd, &focus_widget))
    {
        /*	  
	**  We've got an open entry and we've got its text_widget id
	*/
        paste_ok = (CBcontents != ClipHasNothing) &&
	    DXmCSTextGetEditable ((DXmCSTextWidget) focus_widget);
    }
    else
    {
        /*	  
	**  No open entries
	*/	  
        paste_ok = ((CBcontents != ClipHasNothing) && (! cd->read_only)) ;
	if (cd->showing == show_day)
	{
	    select_all_ok = (cd->dayslots.number_of_items != 0) ||
			    (cd->daynotes.number_of_items != 0) ;
	}
    }

    if (CLIPTestSelectionWidget (cd, &select_widget))
    {
        /*	  
	**  We own the selection
	*/	  
        copy_ok = TRUE ;

	if (select_widget == NULL)
	{
            /*	  
	    **  selection owner was our mainwindow
	    */	  
            if (! cd->read_only)
	    {
		cut_ok = clear_ok = TRUE ;
		for (i = 0 ;  i < cd->number_of_selected_entries ;  i++)
		{
		    if (! DSWGetEntryEditable (cd->selected_entries [i]))
		    {
			cut_ok = clear_ok = FALSE ;
			break ;
		    }
		}
	    }
	}
	else
	{
            /*	  
	    **  selection owner was our text widget
	    */	  
            cut_ok = clear_ok = DXmCSTextGetEditable
		((DXmCSTextWidget) select_widget);
	}
    }

    XtSetSensitive (cd->pb_edit_cut,     cut_ok) ;
    XtSetSensitive (cd->pb_edit_copy,    copy_ok) ;
    XtSetSensitive (cd->pb_edit_paste,   paste_ok) ;
    XtSetSensitive (cd->pb_edit_clear,   clear_ok) ;
    XtSetSensitive (cd->pb_edit_selall,  select_all_ok) ;
    XtSetSensitive (cd->pb_options_mark, mark_ok) ;

}

void DO_TSE_EDIT_MENU
#ifdef _DWC_PROTO_
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
    Sloteditor		se = (Sloteditor) tag;
    CalendarDisplay	cd = se->cd;    
    Boolean		cut_ok;
    Boolean		copy_ok;
    Boolean		select_all_ok;
    Boolean		clear_ok;
    Boolean		paste_ok;
    Cardinal		length;
    char		*slot_text;
    XmString		xm_text;
    long		byte_count, cvt_status;
    ClipContents	cbc;

    /*
    **  Not sure exactly why we need to do this
    */
    if (cbs->reason != (int)XmCR_MAP)
    {
	return;
    } 
    
    cut_ok	  = FALSE;
    copy_ok       = FALSE;
    clear_ok      = FALSE;
    paste_ok      = FALSE;
    select_all_ok = FALSE;

    xm_text = DXmCSTextGetString (SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext]);
    if (xm_text)
    {
	slot_text = DXmCvtCStoFC(xm_text, &byte_count, &cvt_status);
	XmStringFree(xm_text);
    }
    else
    {
	slot_text = NULL;
    }

    if (slot_text != NULL)
    {
	/*
	**  There is a non null string in the text widget
	*/
	if (strcmp (slot_text, "") != 0)
	{
	    /*
	    **	There's a non zero length string in the text widget: selectall
	    **	is available
	    */	    
	    select_all_ok   = TRUE;
	    XtFree (slot_text);

	    xm_text = DXmCSTextGetSelection(
		SLOTEDITORWIDGETS(se)[k_ts_timeslot_stext]);
	    slot_text = DXmCvtCStoFC(xm_text, &byte_count, &cvt_status);
	    XmStringFree(xm_text);

	    if (slot_text != NULL)
	    {
		/*
		**  There is a selection to cut, copy and clear, but only if the
		**  Calendar file isn't read only!
		*/
		if (! cd->read_only)
		{
		    cut_ok = TRUE ;
		    clear_ok = TRUE ;		    
		}
		copy_ok = TRUE;
		XtFree (slot_text) ;
	    } 
	} 
    } 

    /*
    **	We can paste if the calendar file isn't read only and if the clipboard
    **	isn't empty!
    */
#if 0
    cbc = CLIPTestClipboardContents (cd->mainwid);
#else
    cbc = ClipHasString;
#endif
    paste_ok = ((!cd->read_only) && (cbc != ClipHasNothing));

    /*
    **  Let's set the button's sensivity to what we have found
    */        
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_cut_menu_item], cut_ok);
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_copy_menu_item], copy_ok);
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_paste_menu_item], paste_ok);
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_clear_menu_item], clear_ok);
    XtSetSensitive (SLOTEDITORWIDGETS(se)[k_ts_selectall_menu_item],
	select_all_ok);

}

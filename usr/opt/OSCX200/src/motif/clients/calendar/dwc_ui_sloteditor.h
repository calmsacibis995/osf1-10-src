#ifndef dwc_ui_sloteditor_h 
#define dwc_ui_sloteditor_h 1
/* $Header$ */
/* #module dwc_ui_sloteditor.h */
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
**	Denis G. Lacroix, February-1989
**
**  ABSTRACT:
**
**	TBD.
**
**--
*/

#include    "dwc_compat.h"

#define SLOTEDITORWIDGETS(se) ( (se)->sloteditor_widget_array ) 


void
SEUpdateSlotEditorScrollBar PROTOTYPE ((
	CalendarDisplay	cd,
	Boolean		directionRtoL));

Boolean
SEAnySlotEditorsUpAndRunning PROTOTYPE ((
	CalendarDisplay	cd));

Boolean
SEAnyNoteEditorsUpAndRunning PROTOTYPE ((
	CalendarDisplay	cd));

void
SECreateSlotEditor PROTOTYPE ((
	CalendarDisplay	cd,
	WhichEditor	editor,
	DwcDaySlots	ds,
	DwcDswEntry	entry,
	Boolean		text_changed));

void
SELinkItemToEntry PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	index,
	DwcDswEntry	link));

void
SEUnlinkAllItems PROTOTYPE ((
	CalendarDisplay	cd));

int
SEFindItem PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	dsbot,
	int		item_id,
	DwcDswEntry	link,
	Boolean		popup,
	int		*return_start,
	int		*return_duration,
	XmString	*return_text));

int
SEGetNewItems PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	dsbot,
	Cardinal	index,
	DwcDayItem	*return_di,
	XmString	*return_text));

void
SEChangeAMPM PROTOTYPE ((
	CalendarDisplay	cd));

void
SEPoseRepeatingEntryQuestions PROTOTYPE ((
	CalendarDisplay	cd,
	int		text_index,
	DayItemUpdate	diu));

#endif	

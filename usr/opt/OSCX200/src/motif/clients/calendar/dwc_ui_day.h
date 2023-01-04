#ifndef _dwc_ui_day_h_
#define _dwc_ui_day_h_ 1
/* $Header$ */
/* #module dwc_ui_day.h */
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
**	Ken Cowan, March 1989
**
**  ABSTRACT:
**
**	Public entry points from dwc_ui_day.c
**
**--
*/


#include    "dwc_compat.h"


void DAYClearAllItems PROTOTYPE ((DwcDaySlots	ds));

DwcDayItem DAYCloneDayItem PROTOTYPE ((CalendarDisplay cd, DwcDayItem old));

void DAYConfigureDayDisplay PROTOTYPE ((CalendarDisplay cd));

void DAYDestroyDayItem PROTOTYPE ((DwcDayItem di));

void DAYDestroyDayItemUpdate PROTOTYPE ((DayItemUpdate diu));

Boolean DAYDoDayItemUpdate PROTOTYPE ((DayItemUpdate diu, Time time));

void DAYGetIconsForDayItem PROTOTYPE ((
	DwcDayItem	di,
	unsigned char	**ret_icons,
	Cardinal	*ret_num_icons));

void DAYResetTimeslotEntry PROTOTYPE ((
	CalendarDisplay	cd,
	DwcDaySlots	ds,
	Cardinal	dsbot,
	DwcDswEntry	entry,
	Time		time));

void DAYGetDayItems PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year));

DayItemUpdate DAYCreateDayItemUpdateRecord PROTOTYPE ((
	CalendarDisplay		cd,
	Cardinal		dsbot,
	XmString		text,
	DwcDayItem		old,
	DwcDayItem		new,
	RepeatChangeKind	kind));

void DAYGetIconsFromText PROTOTYPE ((
	int		text_class,
	char		**text_p,
	int		*length_p,
	Cardinal	*icons_num_p,
	unsigned char	**icons_p));

DwcDayItem DAYCreateDayItemRecord PROTOTYPE ((
	Cardinal		alarms_number,
	unsigned short int	*alarms_times,
	Cardinal		icons_number,
	unsigned char		*icons));

void DAYRemoveEntry PROTOTYPE ((
	CalendarDisplay	cd,
	DwcDaySlots	ds,
	DwcDswEntry	entry,
	Time		time));

void DAYSetupDayNameWidget PROTOTYPE ((
	Widget		lb,
	Cardinal	day,
	Cardinal	month,
	Cardinal	year));

Boolean DAYTestAnyOpenEntries PROTOTYPE ((CalendarDisplay cd));

void DAYUpdateMonthMarkup PROTOTYPE ((
	CalendarDisplay	cd,
	Cardinal	first,
	Cardinal	last,
	Boolean		all));

void DAYDO_ENTRY_MENU PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void DAYDO_ENTRY_EDIT PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void DAYDO_ENTRY_DELETE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void DAYDO_ENTRY_CLOSE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void DAYDO_ENTRY_RESET PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

Boolean DAYTraverseToOpen PROTOTYPE ((CalendarDisplay	cd));

Boolean DAYTraverseToClose PROTOTYPE ((CalendarDisplay	cd));

#endif	/* _dwc_ui_day_h_ */

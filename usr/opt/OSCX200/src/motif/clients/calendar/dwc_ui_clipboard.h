#ifndef _dwc_ui_clipboard_h_
#define _dwc_ui_clipboard_h_
/* $Header$								    */
/* #module DWC$UI_CLIPBOARD "V3-002"				    */
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
**	Denis G. Lacroix    March-1989
**
**  ABSTRACT:
**
**	TBD.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**  V3-002  Paul Ferwerda				    13-Apr-1990
**		Moved ClipContents here from DWC_UI_CALENDAR.H where it didn't
**		belong.
**--
**/


#include    "dwc_compat.h"

typedef enum
{
    ClipHasNothing,
    ClipHasString,
    ClipHasEntry,
    ClipHasCT,
    ClipHasDDIF,
    ClipHasCS
} ClipContents;


/*
**  Function Prototypes
*/

void CLIPDO_CUT PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void CLIPDO_COPY PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void CLIPDO_PASTE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void CLIPDO_CLEAR PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void CLIPDO_SELALL PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void CLIPDeselectAllEntries PROTOTYPE ((CalendarDisplay	cd, Time time));

ClipContents CLIPTestClipboardContents PROTOTYPE ((Widget w));

Boolean CLIPTestEntryOpen PROTOTYPE ((
	CalendarDisplay	cd,
	Widget		*text_widget));

Boolean CLIPSMTestSelectionOurs PROTOTYPE ((CalendarDisplay cd));

Boolean CLIPTestSelectionWidget PROTOTYPE ((
	CalendarDisplay	cd,
	Widget		*text_widget));

void CLIPMainwidOwnPRIMARY PROTOTYPE ((
	CalendarDisplay	cd,
	Time		time));

void CLIPReceiveClientMessage PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XClientMessageEvent	*event));

Boolean CLIPImportInterchange PROTOTYPE ((
	CalendarDisplay	cd,
	XmString	buffer,
	Time		time));

void DO_TSE_CUT PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void DO_TSE_COPY PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void DO_TSE_PASTE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void DO_TSE_CLEAR PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void DO_TSE_SELALL PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));


void CLIPSMSetSelected PROTOTYPE ((
	CalendarDisplay	cd,
	DwcDswEntry	entry,
	Time		time));

void CLIPSMAddSelected PROTOTYPE ((
	CalendarDisplay	cd,
	DwcDswEntry	entry,
	Time		time));

void CLIPSMRemoveSelected PROTOTYPE ((
	CalendarDisplay	cd,
	DwcDswEntry	entry,
	Time		time));

void CLIPSMInitialise PROTOTYPE ((Widget toplevel));

Boolean CLIPSMGetStringSelection PROTOTYPE ((
	CalendarDisplay	cd,
	XmString	*value,
	unsigned long	*length,
	Time		time));

Boolean CLIPSMGetComplexStringSelection PROTOTYPE ((
	CalendarDisplay	cd,
	ClipContents	cbc,
	XmString	*value,
	unsigned long	*length,
	Time		time));

void CLIPSMSendKillSelection PROTOTYPE ((
	CalendarDisplay	cd,
	Time		time));

ClipContents CLIPSMTestTargets PROTOTYPE ((
	CalendarDisplay	cd,
	Time		time));

Boolean CLIPSMGetEntriesSelection PROTOTYPE ((
	CalendarDisplay	cd,
	Time		time));

#endif /* _dwc_ui_clipboard_h_ */

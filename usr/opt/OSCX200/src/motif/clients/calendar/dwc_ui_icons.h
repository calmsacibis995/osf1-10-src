#ifndef _icons_h_
#define _icons_h_ 1
/* $Header$								    */
/* #module DWC_UI_ICONS.H "V3-001"					    */
/*
**  Copyright (c) Digital Equipment Corporation, 1991
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
**	Dick Schoeller,  3-APR-1991
**
**  ABSTRACT:
**
**	Function prototypes for dwc_ui_datefunctions.c.
**
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**--
**/

void ICONSSetIconifyIcon PROTOTYPE ((
	Widget		toplevel,
	int		icon_type));

void ICONSSetIconBoxIcon PROTOTYPE ((
	Widget		toplevel,
	char		icon_bits[],
	dtb		*date_time,
	XmFontList	day_fontlist,
	XmFontList	month_fontlist,
	Boolean		invert,
	Boolean		reparent,
	Boolean		mapped));

void ICONSSetIconTime PROTOTYPE ((
	Widget	toplevel,
	dtb	*date_time,
	Boolean	show_text,
	char	*text,
	Boolean	nl_after_text,
	Boolean	show_day,
	Boolean	full_day,
	Boolean	nl_after_day,
	Boolean	show_time,
	Boolean	ampm));

Cursor ICONSWaitCursorCreate PROTOTYPE ((Widget w));

Cursor ICONSInactiveCursorCreate PROTOTYPE ((Widget w));

void ICONSWaitCursorDisplay PROTOTYPE ((Widget w, Cursor cursor));

void ICONSInactiveCursorDisplay PROTOTYPE ((Widget w, Cursor cursor));

void ICONSWaitCursorRemove PROTOTYPE ((Widget w));

void ICONSInactiveCursorRemove PROTOTYPE ((Widget w));

#endif /* _icons_h_ */

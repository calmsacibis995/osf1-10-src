#ifndef _dwc_ui_file_h_
#define _dwc_ui_file_h_
/* $Header$								    */
/* #module DWC_UI_FILE.H "V3-002"				    */
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
**	Public entry points from DWC_UI_FILE.H
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3-002 PGLF	16-Apr-1990 Added header for ultrix rcs
** 
**	V2-001	Ken Cowan					17-Mar-1989
**		Initial version
**--
**/


#include    "dwc_compat.h"

void
FILESaveCalendar PROTOTYPE ((
	CalendarDisplay		cd,
	Time			time));

void
FILECloseCalendar PROTOTYPE ((
	CalendarDisplay		cd,
	Time			time));

void
FILEDO_FILE_MENU PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void
FILEDO_NAMEAS_OK PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void
FILEDO_NAMEAS_CANCEL PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void
FILEDO_OPEN_FILE PROTOTYPE ((
	Widget					w,
	int					*tag,
	XmFileSelectionBoxCallbackStruct	*cbs));

void
FILEDO_CLOSE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void
FILEDO_NAMEAS PROTOTYPE ((
	Widget			w,
	char			*tag,
	XmAnyCallbackStruct	*cbs));

void
FILEDO_DELETE PROTOTYPE ((
	Widget			w,
	char			*tag,
	XmAnyCallbackStruct	*cbs));

void
FILEDO_INCLUDE PROTOTYPE ((
	Widget					w,
	caddr_t					tag,
	XmFileSelectionBoxCallbackStruct	*cbs));

void
FILEBYE_BYE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void
FILESAVE_YOURSELF PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void
FILEDO_CAUTION_DELETE_FILE PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

int
FILEOpenCalendar PROTOTYPE ((
	AllDisplays	ads,
	char		*filespec,
	CalendarDisplay	*new_cd_ptr));

#endif	/* _dwc_ui_file_h_ */

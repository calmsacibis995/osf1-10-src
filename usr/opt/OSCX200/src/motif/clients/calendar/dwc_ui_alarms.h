#ifndef dwc_ui_alarms_public_h
#define dwc_ui_alarms_public_h 1
/* $Header$ */
/* #module DWC$UI_ALARMS_PUBLIC "V2-001"				    */
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
**	Denis G. Lacroix, February 1989
**
**  ABSTRACT:
**
**	Public include file for Calendar's User Interface error handling 
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	V2-001  Ken Cowan					 3-May-1989
**		Initial version.
**--
**/

#include    "dwc_compat.h"

void
ALARMSClockTimerTick PROTOTYPE ((
	CalendarDisplay	cd));

void
ALARMSUpdateTimeDisplay PROTOTYPE ((
	CalendarDisplay	cd));

void
ALARMSSetupNextAlarm PROTOTYPE ((
	CalendarDisplay	cd,
	Boolean		aftertick));

void
ALARMSSetAlarmsDirection PROTOTYPE ((
	CalendarDisplay	cd,
	Boolean		directionRtoL));

void
ALARMSSetAlarmsAmPm PROTOTYPE ((
	CalendarDisplay	cd,
	Boolean		ampm));

#endif


#ifndef _dwc_ui_month_public_h_
#define _dwc_ui_month_public_h_ 1
/* $Header$ */
/* #module DWC$UI_ALARMS_PUBLIC "V2-001"					    */
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
**	Public include for month routines
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
MONTHConfigMonthDisplay PROTOTYPE ((
	CalendarDisplay	cd));

void
MONTHCreateMonthDisplay PROTOTYPE ((
	CalendarDisplay	cd));

#endif /* _dwc_ui_month_public_h_ */


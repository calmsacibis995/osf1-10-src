#ifndef _dwc_ui_view_h_
#define _dwc_ui_view_h_
/* $Header$ */
/* #module DWC_UI_VIEW.H "V2-001"				    */
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
**	Public entry points from DWC_UI_VIEW.C
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	V2-001	Ken Cowan					17-Mar-1989
**		Initial version
**--
**/


#include    "dwc_compat.h"


void
VIEWMONTH_DC PROTOTYPE ((
	Widget			mw,
	caddr_t			tag,
	DwcMonthCallbackStruct	*cbs));

void
VIEW_SELECTED PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	int	*reason));

void
VIEW_TODAY PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	int	*reason));

void
VIEW_DAY PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	int	*reason));

void
VIEW_MONTH PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	int	*reason));

void
VIEW_YEAR PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	int	*reason));

void
VIEWDO_VIEW_MENU PROTOTYPE ((
	Widget	w,
	caddr_t	tag,
	int	*reason));

#endif	/* _dwc_ui_view_h_ */

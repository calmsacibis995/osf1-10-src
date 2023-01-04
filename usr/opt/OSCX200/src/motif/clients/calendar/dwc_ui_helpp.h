#ifndef _dwc_ui_helpp_h_
#define _dwc_ui_helpp_h_ 1
/* $Header$ */
/* #module dwc_ui_helpp.h */
/*
**++
**
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
**	Marios Cleovoulou,  March-1988
**	Denis G. Lacroix,   Apr-1989
**	
**  ABSTRACT:
**
**	Generic help routines.
**
**--
*/

#include    "dwc_compat.h"

#define DwcTHelpApplName    "Calendar"

#ifdef	VMS
#define	    DwcTHelpLibrary	    "DECW$CALENDAR"
#else	/*VMS*/
#define		DwcTHelpLibrary	    "dxcalendar"
#endif	/*VMS*/

#define DwcTHelpOnWindow	"overview"
#define DwcTHelpOnContext	"context"
#define DwcTHelpOnTerms		"glossary"
#define DwcTHelpOnVersion	"aboutframe"
#define DwcTHelpOnHelp		"onhelp_menu_item"

#endif	/* _dwc_ui_helpp_h_ */

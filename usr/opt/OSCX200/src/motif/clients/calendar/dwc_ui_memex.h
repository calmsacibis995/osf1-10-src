#ifndef _dwc_ui_memex_public_h_
#define _dwc_ui_memex_public_h_ 1
/* $Header$ */
/* #module DWC_UI_MEMEX_PUBLIC "V3-005" */
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
**	Denis G. Lacroix	October 1989
**
**  ABSTRACT:
**
**	This is the module contains miscellaneous support routines
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**	V3.0-005 Paul Ferwerda					05-Oct-1990
**		Changed declaration of MEMEXCreateDwUi
**	V3-004	Paul Ferwerda					15-Feb-1990
**		Integrated into mainline code
**	V3-003  Andre Pavanello					1-Nov-89
**		Add macro for MEMEX error reporting
**	V3-002	Andre Pavanello					28-Oct-89
**		Convert to new API
**	V3-001	Denis G. Lacroix				6-Oct-1989
**		Initial version
**
**--
*/


#include    "dwc_compat.h"

/*
**  Typdefs
*/

typedef struct _DwcMemexSurrogateList
{
    struct  _DwcMemexSurrogateList	*next;
    lwk_surrogate			surrogate;
} DwcMemexSurrogateList;


/*
**  Macro Definitions
*/

#define TimeOffset  30	

#define T 1
#define F 0

#ifdef VMS
#define TerminationSuccess 1
#define TerminationFailure 0
#else
#define TerminationSuccess 0
#define TerminationFailure -1
#endif

/*
**  MEMEX surrogate types 
*/

#define DWC_SURROGATE			"DwCalendar"

#define DWC_SURROGATE_TYPE		"%Type"

    /*
    **	This pre-defined property must be of type string (with the % in front)
    **	to be a key in the MEMEX database.  When caching will be implemeted,
    **	this property can become an integer because we will do one query for
    **	all Calendar surrogates and then sort then by type in memory (int is
    **	easier than string for comparison!)
    */

#define	DWC_MEMEX_TIMESLOT_ENTRY	"DwCTimeslot"
#define DWC_MEMEX_DAY			"DwCDay"
#define DWC_MEMEX_MONTH			"DwCMonth"
#define DWC_MEMEX_YEAR			"DwCYear"
#define DWC_MEMEX_OPEN_TIMESLOT_ENTRY	"DwCOpenTimeslot" /* not implemented */

/*
**  Calendar supported operation for MEMEX
*/

#define DWC_MEMEX_VIEW_ENTRY	"View"

/*
**  Calendar Surrogate properties
*/

#define DWC_FILE_PROPERTY		"%Container"
#define DWC_DSBOT_PROPERTY		"%Id"
#define	DWC_ITEM_ID_PROPERTY		"ItemId"

#define DWC_TYPE




/*
**  Function prototypes
*/

lwk_status
MEMEXApplyCallback PROTOTYPE ((
	lwk_ui			dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_surrogate		surrogate,
	lwk_string		operation,
	lwk_integer		follow_type));

lwk_status
MEMEXCreateDwUi PROTOTYPE ((
	CalendarDisplay	cd));

void
MEMEXDeleteDwUi PROTOTYPE ((
	CalendarDisplay	cd));

void
MEMEXSurrogateHighlightForDay PROTOTYPE ((
	CalendarDisplay	cd));

#endif /* _dwc_ui_memex_public_h_ */

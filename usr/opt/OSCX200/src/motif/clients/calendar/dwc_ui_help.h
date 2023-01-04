#ifndef _dwc_ui_help_h_
#define _dwc_ui_help_h_ 1
/* $Id$ */
/* #module dwc_ui_help.h */
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
**	Denis G. Lacroix,   April 1989
**
**  ABSTRACT:
**
**	TBD.
**
**--
*/


#include    "dwc_compat.h"


/*
**  Typedefs
*/

typedef struct
    {
    Widget  help_widget;
    Widget  parent_widget;
    } DwcHelpContextRecord, *DwcHelpContext ;


/*
**  Function Prototypes
*/

void HELPInitialize PROTOTYPE((AllDisplaysRecord *ads));

void HELPClose PROTOTYPE((void));

void HELPInitializeForDisplay PROTOTYPE((CalendarDisplay cd));

void HELPCloseForDisplay PROTOTYPE((CalendarDisplay cd));

void HELPForWidget PROTOTYPE((Widget w, caddr_t tag, XmAnyCallbackStruct *cbs));

void HELPForModalWidget PROTOTYPE((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

void HELPFromMenu PROTOTYPE((Widget w, caddr_t *tag, XmAnyCallbackStruct *cbs));

void HELPDisplay
#if defined(NEW_HYPERHELP) || defined(OLD_HYPERHELP)
 PROTOTYPE((CalendarDisplay cd, Widget cursor_widget, char *topic));
#else
 PROTOTYPE((
	Widget		parent_widget,
	Widget		cursor_widget,
	Widget		*help_widget,
	char		*topic,
	Boolean		reuse));
#endif

void HELPDestroyForModal PROTOTYPE((
	Widget			w,
	int			*tag,
	XmAnyCallbackStruct	*cbs));

#endif /* _dwc_ui_help_h_ */

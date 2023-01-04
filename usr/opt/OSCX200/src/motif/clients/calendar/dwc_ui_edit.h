#ifndef _dwc_ui_edit_h_
#define _dwc_ui_edit_h_
/* $Header$								    */
/* #module DWC_UI_EDIT.H "V3-003"				    */
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
**	Public entry points from DWC_UI_EDIT.C
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3-003 PGLF 16-Apr-1990  Added RCS header
**	V2-002	Denis G. Lacroix				 6-Apr-1989
**		Fix routine declarations.
**	V2-001	Ken Cowan					17-Mar-1989
**		Initial version
**--
**/


#include    "dwc_compat.h"


void
DO_TSE_EDIT_MENU PROTOTYPE ((
	Widget			widget,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

void
EDITDO_EDIT_MENU PROTOTYPE ((
	Widget			w,
	caddr_t			*tag,
	XmAnyCallbackStruct	*cbs));

#endif	/* _dwc_ui_edit_h_ */

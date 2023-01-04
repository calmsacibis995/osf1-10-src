#ifndef _errorboxes_h_
#define _errorboxes_h_
/* $Header$ */
/* #module DWC_UI_ERRORBOXES.H "V3.0-005" */
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
**	Marios Cleovoulou, March-1988
**
**  ABSTRACT:
**
**	Generic error messages box routines.
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3.0-005 Paul Ferwerda					23-Oct-1990
**		Change name of DestoryErrorBox to ERRORDestroyErrorBox
** V3.0-004 Paul Ferwerda					05-Oct-1990
**		Added ERRORDisplayTextCall which will display text and do a
**		callback if we need it to.
** V3.0-003 Paul Ferwerda					27-Sep-1990
**		Added prototype declaration for ERRORDisplayText
** V3.0-002 Paul Ferwerda					08-Feb-1990
**		Took out ERRORReportMessage since it was identical to
**		ERRORReportError.
**	V1-001  Marios Cleovoulou	  			 2-Apr-1988
**		Initial version.
**/


#include    "dwc_compat.h"

void
ERRORDisplayError PROTOTYPE ((
	Widget	parent,
	char	*name));

void
ERRORDisplayErrno PROTOTYPE ((
	Widget	parent,
	char	*name));

void
ERRORDisplayText PROTOTYPE ((
	Widget	parent,
	char	*name,
	char	*text));

void
ERRORDisplayTextCall PROTOTYPE ((
	Widget	parent,
	char	*name,
	char	*text,
	XtCallbackProc callback_proc,
	caddr_t	tag));

void
ERRORReportError PROTOTYPE ((
	Widget	parent,
	char	*name,
	XtCallbackProc callback_proc,
	caddr_t	tag));

void
ERRORReportErrno PROTOTYPE ((
	Widget	parent,
	char	*name,
	XtCallbackProc callback_proc,
	caddr_t	tag));

void
ERRORDestroyErrorBox PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

#endif /* _errorboxes_h_ */

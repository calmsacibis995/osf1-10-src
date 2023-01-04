/*
*****************************************************************************

              Copyright (c) Digital Equipment Corporation, 1990
              All Rights Reserved.  Unpublished rights reserved
              under the copyright laws of the United States.

              The software contained on this media is proprietary
              to and embodies the confidential technology of
              Digital Equipment Corporation.  Possession, use,
              duplication or dissemination of the software and
              media is authorized only pursuant to a valid written
              license from Digital Equipment Corporation.

              RESTRICTED RIGHTS LEGEND   Use, duplication, or
              disclosure by the U.S. Government is subject to
              restrictions as set forth in Subparagraph (c)(1)(ii)
              of DFARS 252.227-7013, or in FAR 52.227-19, as
              applicable.

*****************************************************************************
**++
**  FACILITY:
**
**      DECwindows Toolkit
**
**  ABSTRACT:
**
**      Asian Input Method Widget
**
**
**  MODIFICATION HISTORY:
**
**      November 13, 1990   Begin working on the AIM Widget
**
**--
**/
#ifndef _DXmAIM_h
#define _DXmAIM_h

#include <X11/IntrinsicP.h>
#include "AIMProtocol.h"

/* Class record constants */

#define DXmNaimContinueIStateEvent	"aimContinueIStateEvent"
#define DXmNaimInputMode		"aimInputMode"
#define DXmNaimLanguage			"aimLanguage"
#define DXmNaimUserDefinedIM		"aimUserDefinedIM"
#define DXmNaimPreEditStart		"aimPreEditStart"
#define DXmNaimSetCursorPosition	"aimSetCursorPosition"
#define DXmNaimGetCurrentCursorChar	"aimGetCurrentCursorChar"
#define DXmNaimSecPreEditStart		"aimSecPreEditStart"
#define DXmNaimPreEditDraw		"aimPreEditDraw"
#define DXmNaimDrawIntermediateChar	"aimDrawIntermediateChar"
#define DXmNaimQueryXYPosition		"aimQueryXYPosition"
#define DXmNaimRimpDestroy		"aimRimpDestroy"
#define DXmNaimTextWidgetFontList	"aimTextWidgetFontList"

#define DXmCAimContinueIStateEvent	"AimContinueIStateEvent"
#define DXmCAimInputMode		"AimInputMode"
#define DXmCAimLanguage			"AimLanguage"
#define DXmCAimUserDefinedIM		"AimUserDefinedIM"

#define DXmCRaimPreEditStart		1
#define DXmCRaimSetCursorPosition	2
#define DXmCRaimGetCurrentCursorChar	3
#define DXmCRaimSecPreEditStart		4
#define DXmCRaimPreEditDraw		5
#define DXmCRaimDrawIntermediateChar	6
#define DXmCRaimQueryXYPosition		7
#define DXmCRaimRimpDestroy		8

typedef struct
{
    int     		reason;
    XEvent  		*event;
    Opaque		*base_position;
    int			start_offset;
    int			end_offset;
    XmString		new_string;
    AIMTextRendering	*renditions;
    int                 num_rendition;
    int			x;
    int			y;
    int			status;
} DXmAIMCallbackStruct;

typedef struct _DXmAIMClassRec	*DXmAIMWidgetClass;
typedef struct _DXmAIMRec	*DXmAIMWidget;

#endif


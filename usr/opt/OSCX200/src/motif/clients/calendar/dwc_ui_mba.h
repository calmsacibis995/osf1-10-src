#ifndef _dwc_ui_mba_h_
#define _dwc_ui_mba_h_ 1
/* $Id$ */
/*
** COPYRIGHT (c) 1988,1989,1990 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**++
**  Subsystem:
**      calendar
**
**  Abstract:
**	Handles making sense out of bouse button clicks.
**
**  Environment:
**	User mode, executable image
**
**  Author:
**	Denis G. Lacroix
**
**  Creation Date: 5-Dec-88
**
**  Modification History:
**  V3-003  Paul Ferwerda					01-Mar-1990
**		Port to Motif, VoidProc to XmVoidProc
**	V1-002  Marios Cleovoulou			       14-Dec-1988
**		Combine MBAPROTO.H into DWC_UI_MBA.H
**	V1-001  Denis G. Lacroix	  			5-Dec-1988
**		Initial version.
**--
*/

#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/XmP.h>	/* for caddr_t and Widget, XmVoidProc */
#ifdef vaxc
#pragma standard
#endif

#include    "dwc_compat.h"

typedef enum
    {
    MbaCallback,	/* private: activate motion callback		    */
    MbaNoAction,	/* private: do nothing				    */
    MbaButtonDown,
    MbaMotionStart,
    MbaMotion,
    MbaButtonUp,
    MbaButtonUpEndingMotion,
    MbaDoubleClick,
    MbaCancelAction,
    MbaCancelMotion,
    MbaIgnoreEvent
    } MbaAction;


caddr_t
MBAInitContext PROTOTYPE ((
	Widget		widget,
	Boolean		use_threshold,
	Boolean		report_motion,
	void		(*motion_start_proc)()));

void
MBAFreeContext PROTOTYPE ((
	caddr_t	context));

void
MBAResetContext PROTOTYPE ((
	caddr_t	context));

MbaAction
MBAMouseButton1Down PROTOTYPE ((
	caddr_t	context,
	XEvent	*event));

MbaAction
MBAMouseButton1Up PROTOTYPE ((
	caddr_t	context,
	XEvent	*event));

MbaAction
MBAMouseButtonOther PROTOTYPE ((
	caddr_t	context,
	XEvent	*event));

MbaAction
MBAMouseButton1Motion PROTOTYPE ((
	caddr_t	context,
	XEvent	*event));

#endif	/* _dwc_ui_mba_h_ */

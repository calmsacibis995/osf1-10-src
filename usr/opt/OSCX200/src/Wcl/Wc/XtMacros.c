/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#include <X11/Wc/COPY>

/*
* SCCS_data: @(#) XtMacros.c 1.2 92/03/18 11:02:29
*
* Some old implementations of Xt provide some of the Xt functions only as
* macros, in violation of the Xt spec.  Here are implementations of the ones
* not provided by SCO Open Desk Top (Motif 1.0).
*/

#include <X11/IntrinsicP.h>
#include <X11/Wc/WcCreateP.h>

#ifdef XtMapWidget
#undef XtMapWidget
#endif
#ifdef XtUnmapWidget
#undef XtUnmapWidget
#endif

void XtMapWidget(widget)
    Widget widget;
{
    XMapWindow(XtDisplay(widget), XtWindow(widget));
}

void XtUnmapWidget(widget)
    Widget widget;
{
    XUnmapWindow(XtDisplay(widget), XtWindow(widget));
}

char* XtName( w )
    Widget w;
{
    if (XtIsWidget(w))
	return w->core.name;
    else
	return XrmQuarkToString(w->core.xrm_name);
}

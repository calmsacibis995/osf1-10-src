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
#ifndef _Xmp_h_
#define _Xmp_h_
#include <X11/Xmp/COPY>

/*
* SCCS_data: @(#) Xmp.h 1.6 92/06/10 06:14:01
*
*	This module contains declarations useful to clients of the
*	Xmp library.
*
*******************************************************************************
*/

#include <X11/Wc/WcCreate.h>	/* for _() macro */

/* XmpRegisterMotif registers all Motif and Xmp widgets.
 * XmpRegisterAll and MriRegisterMotif are aliases for XmpRegisterMotif
 * for backward compatibility.
 */
void XmpRegisterMotif _(( XtAppContext ));
void XmpRegisterAll   _(( XtAppContext ));
void MriRegisterMotif _(( XtAppContext ));

void XmpAddMwmCloseCallback _(( Widget, XtCallbackProc, XtPointer ));

void XmpChangeNavigationType _(( Widget ));

#endif /* _Xmp_h_ */

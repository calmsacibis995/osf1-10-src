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
#ifndef _Xcp_h_
#define _Xcp_h_

#include <X11/Xcp/COPY>
/*
* SCCS_data: @(#) Xcp.h	1.1 92/03/18 11:08:27
*
* Xcp - Cornell Widget Set Public Utilities for Wcl - Xcp.h
*
* Declare registration routines and convenience callbacks for the 
* Cornell widget set.
*
*******************************************************************************
*/

#include <X11/Wc/WcCreate.h>	/* for _() macro */

/* Registration routines are exactly equivalent
*/
extern void XcpRegisterCornell	_((XtAppContext));
extern void XcpRegisterAll	_((XtAppContext));
extern void CriRegisterCornell	_((XtAppContext));

/* Register all the Xcu callbacks - called by XcpRegisterCornell etc
*/
extern void XcuRegisterXcuCBs	_((XtAppContext));

/* Callbacks - all take widget names as arguments
*/
extern void XcuBmgrManageCB		_(( Widget, XtPointer, XtPointer ));
extern void XcuBmgrSampleCB		_(( Widget, XtPointer, XtPointer ));
extern void XcuBmgrSetAllCB		_(( Widget, XtPointer, XtPointer ));
extern void XcuBmgrUnsetAllCB		_(( Widget, XtPointer, XtPointer ));
extern void XcuBmgrToggleAllCB		_(( Widget, XtPointer, XtPointer ));
extern void XcuDeckRaiseLowestCB	_(( Widget, XtPointer, XtPointer ));
extern void XcuDeckLowerHighestCB	_(( Widget, XtPointer, XtPointer ));
extern void XcuDeckRaiseWidgetCB	_(( Widget, XtPointer, XtPointer ));

#endif

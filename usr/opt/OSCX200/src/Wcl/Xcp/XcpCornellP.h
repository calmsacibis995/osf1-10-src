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
#include <X11/Xcp/COPY>
/*
* SCCS_data: @(#) XcpCornellP.h 1.1 92/03/18 11:08:28
*/

#ifndef _AriAthenaP_h_
#define _AriAthenaP_h_

/* Core, Object, RectObj, WindowObj, 
** XmGadget, XmPrimitive, and XmComposite, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint, XmManager.
*/
#include <X11/IntrinsicP.h>

/* include all the *P.h files in heirarchical order */

#include <X11/CoreP.h>
#include <X11/ObjectP.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>

/* Xcu Privates */
#include <Xcu/BmgrP.h>
#include <Xcu/ButtonP.h>
#include <Xcu/CommandP.h>
#include <Xcu/DeckP.h>
#include <Xcu/LabelP.h>
#include <Xcu/RcP.h>
#include <Xcu/TblP.h>
#include <Xcu/EntryP.h>
/* #include <Xcu/WlmP.h> */

#endif

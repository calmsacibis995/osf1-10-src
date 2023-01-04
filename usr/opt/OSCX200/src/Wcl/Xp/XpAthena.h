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
#ifndef _XpAthena_h_
#define _XpAthena_h_
#include <X11/Xp/COPY>

/* SCCS_data: @(#) XpAthena.h	1.2 92/03/18 15:16:52
*/

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint.
*/
#include <X11/Intrinsic.h>

#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4
#define R5
#endif

/* include all the *.h files in heirarchical order */

#include <X11/Xaw/Simple.h>
#include <X11/Vendor.h>

/* Core */
#include <X11/Xaw/Clock.h>
#include <X11/Xaw/Logo.h>
#include <X11/Xaw/Mailbox.h>

/* Simple */
#include <X11/Xaw/Grip.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/StripChart.h>
#include <X11/Xaw/Text.h>
#ifdef R5
#include <X11/Xaw/Mailbox.h>
#include <X11/Xaw/Panner.h>
#endif /*R5*/

/* Label */
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Toggle.h>

/* Command */
#ifdef R5
#include <X11/Xaw/Repeater.h>
#endif

/* Sme */
#include <X11/Xaw/Sme.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>


/* Text */
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/TextSrc.h>
#include <X11/Xaw/AsciiSrc.h>
#include <X11/Xaw/TextSink.h>
#include <X11/Xaw/AsciiSink.h>

/* Composite and Constraint */
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#ifdef R5
#include <X11/Xaw/Porthole.h>
#include <X11/Xaw/Tree.h>
#endif /*R5*/
#include <X11/Xp/Table.h>

/* Form */
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Viewport.h>

#undef R5

#endif /* _XpAthena_h_ */

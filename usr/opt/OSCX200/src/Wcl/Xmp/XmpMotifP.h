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
#ifndef _MriMotifP_h_
#define _MriMotifP_h_
#include <X11/Xmp/COPY>

/*
* SCCS_data: @(#) XmpMotifP.h	1.2 92/03/18 11:12:15
*
*	This module includes all of the private headers for all Motif
*	widgets and Xmp widgets (public widgets based on Motif).  The
*	files get included in the appropriate order (top down in the
*	widget class heirarchy).
*
*******************************************************************************
*/

/* Core, Object, RectObj, WindowObj, 
** XmGadget, XmPrimitive, and XmComposite, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint, XmManager.
*/
#include <X11/Xm/XmP.h>

/* XmGadget Subclasses
*/
#include <X11/Xm/ArrowBGP.h>
#include <X11/Xm/SeparatoGP.h>
#include <X11/Xm/LabelGP.h>
#include <X11/Xm/CascadeBGP.h>
#include <X11/Xm/PushBGP.h>
#include <X11/Xm/ToggleBGP.h>

/* XmPrimitive Subclasses
*/
#include <X11/Xm/ArrowBP.h>
#include <X11/Xm/ListP.h>
#include <X11/Xm/ScrollBarP.h>
#include <X11/Xm/SeparatorP.h>
#include <X11/Xm/TextP.h>

/* TextField: Motif 1.0 doesn't have them, the TextFP.h collides with TextP.h,
 * and they are useless anyway.  Nevertheless, at least get TextF.h if there.
 */
#ifndef _OLD_MOTIF
#ifdef FIXED_TextFP_h
#include <X11/Xm/TextFP.h>
#else
#include <X11/Xm/TextF.h>
#endif
#endif

#include <X11/Xm/LabelP.h>
#include <X11/Xm/CascadeBP.h>
#include <X11/Xm/DrawnBP.h>
#include <X11/Xm/PushBP.h>
#include <X11/Xm/ToggleBP.h>

/* XmManager Subclasses
*/
#include <X11/Xm/DrawingAP.h>
#include <X11/Xm/FrameP.h>
#include <X11/Xm/PanedWP.h>
#include <X11/Xm/RowColumnP.h>
#include <X11/Xm/ScaleP.h>
#include <X11/Xm/ScrolledWP.h>
#include <X11/Xm/MainWP.h>
#include <X11/Xm/BulletinBP.h>
#include <X11/Xm/FormP.h>
#include <X11/Xm/MessageBP.h>
#include <X11/Xm/SelectioBP.h>
#include <X11/Xm/CommandP.h>
#ifndef XtSpecificationRelease
#include <X11/Xm/FileSB.h>
#endif
#include <X11/Xm/FileSBP.h>

/* Shell Subclasses
*/
#include <X11/ShellP.h>
#include <X11/VendorP.h>
#include <X11/Xm/MenuShellP.h>
#include <X11/Xm/DialogSP.h>

/* Apparently Obsolete 
*/
#include <X11/Xm/SashP.h>

/* Public widgets derived from Motif
*/
#include <X11/Xmp/TableP.h>

#endif /* _MriMotifP_h_ */

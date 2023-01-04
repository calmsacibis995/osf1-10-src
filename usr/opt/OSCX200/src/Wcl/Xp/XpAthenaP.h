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
#ifndef _XpAthenaP_h_
#define _XpAthenaP_h_
#include <X11/Xp/COPY>

/* SCCS_data: @(#) XpAthenaP.h	1.2 92/03/18 15:16:53
*/

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint.
*/
#include <X11/IntrinsicP.h>

#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4
#define R5
#endif

/* include all the *P.h files in heirarchical order */

#include <X11/CoreP.h>
#include <X11/ObjectP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>

/* Core */
#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/LogoP.h>

/* Core with lotsa system dependencies: using these P.h files requires a
** very well configured Imake configuration due to the many OS dependencies,
** and - hey - we really only do this for kicks anyway!
*/ 
#include <X11/Xaw/Clock.h>
#include <X11/Xaw/Mailbox.h>

/* Simple */
#include <X11/Xaw/GripP.h>
#include <X11/Xaw/LabelP.h>
#include <X11/Xaw/ListP.h>
#include <X11/Xaw/ScrollbarP.h>
#include <X11/Xaw/StripCharP.h>
#include <X11/Xaw/TextP.h>
#ifdef R5
#include <X11/Xaw/MailboxP.h>
#include <X11/Xaw/PannerP.h>
#endif /*R5*/

/* Label */
#include <X11/Xaw/CommandP.h>
#include <X11/Xaw/MenuButtoP.h>
#include <X11/Xaw/ToggleP.h>

/* Command */
#ifdef R5
#include <X11/Xaw/RepeaterP.h>
#endif

/* Sme */
#include <X11/Xaw/SmeP.h>
#include <X11/Xaw/SimpleMenP.h>
#include <X11/Xaw/SmeBSBP.h>
#include <X11/Xaw/SmeLineP.h>


/* Text */
#include <X11/Xaw/AsciiTextP.h>
#include <X11/Xaw/TextSrcP.h>
#include <X11/Xaw/AsciiSrcP.h>
#include <X11/Xaw/TextSinkP.h>
#include <X11/Xaw/AsciiSinkP.h>

/* Composite and Constraint */
#include <X11/Xaw/BoxP.h>
#include <X11/Xaw/FormP.h>
#include <X11/Xaw/PanedP.h>
#ifdef R5
#include <X11/Xaw/PortholeP.h>
#include <X11/Xaw/TreeP.h>
#endif /*R5*/
#include <X11/Xp/TableP.h>

/* Form */
#include <X11/Xaw/DialogP.h>
#include <X11/Xaw/ViewportP.h>

#undef R5

#endif /* _XpAthenaP_h_ */

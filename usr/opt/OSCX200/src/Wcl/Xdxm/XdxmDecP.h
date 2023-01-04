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
#ifndef _XopOpenLookP_h_
#define _XopOpenLookP_h_
#include <X11/Xop/COPY>

/* SCCS_data: @(#) XopOpenLookP.h	1.2 92/06/10 06:15:19
*/

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint
*/
#include <X11/IntrinsicP.h>

/* include all the *.h files in heirarchical order */

/* Xt Base Classes */
#include <X11/CoreP.h>
#include <X11/ObjectP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>

#include <Xol/EventObjP.h>
#include <Xol/PrimitiveP.h>

/* Under Primitive */
#include <Xol/AbbrevMenP.h>
#include <Xol/AbbrevStaP.h>
#include <Xol/ArrowP.h>
#include <Xol/ButtonP.h>
#include <Xol/MenuButtoP.h>
#include <Xol/ButtonStaP.h>
#include <Xol/ListPaneP.h>
#include <Xol/MagP.h>
#include <Xol/OblongButP.h>
#include <Xol/PushpinP.h>
#include <Xol/RectButtoP.h>
#include <Xol/ScrollbarP.h>
#include <Xol/SliderP.h>
#include <Xol/StaticTexP.h>
#include <Xol/StubP.h>
#include <Xol/TextEditP.h>
#include <Xol/TextPaneP.h>

#include <Xol/FlatP.h>
#include <Xol/FExclusivP.h>
#include <Xol/FNonexcluP.h>
#include <Xol/FCheckBoxP.h>

/* Under TopLevelShell */
#include <Xol/BaseWindoP.h>
#include <Xol/MenuP.h>
#include <Xol/NoticeP.h>
#include <Xol/PopupWindP.h>

/* Under Constraint */
#include <Xol/ManagerP.h>

/* Under Manager */
#include <Xol/BulletinBP.h>
#include <Xol/CaptionP.h>
#include <Xol/CheckBoxP.h>
#include <Xol/ControlArP.h>
#include <Xol/ExclusiveP.h>
#include <Xol/FooterPanP.h>
#include <Xol/FormP.h>
#include <Xol/HelpP.h>
#include <Xol/NonexclusP.h>
#include <Xol/ScrolledWP.h>
#include <Xol/ScrollingP.h>
#include <Xol/TextFieldP.h>
#include <Xol/TextP.h>

#endif

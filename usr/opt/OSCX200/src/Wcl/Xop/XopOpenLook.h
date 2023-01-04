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
#ifndef _XopOpenLook_h_
#define _XopOpenLook_h_
#include <X11/Xop/COPY>

/* SCCS_data: @(#) XopOpenLook.h	1.2 92/06/10 06:15:19
*/

#ifdef XTTRACEMEMORY
#undef XTTRACEMEMORY	/* Thanks whoever wrote Xol/OlXlibExt.h  >:^p */
#endif

/* Core, Object, RectObj, WindowObj, 
** Shell, OverrideShell, WMShell, VendorShell, TopLevelShell, ApplicationShell, 
** Constraint
*/
#include <X11/Intrinsic.h>

#include <Xol/OpenLook.h>

/* include all the *.h files in heirarchical order */

/* Under Primitive */
#include <Xol/AbbrevMenu.h>
#include <Xol/AbbrevStac.h>
#include <Xol/Arrow.h>
#include <Xol/Button.h>
#include <Xol/MenuButton.h>
#include <Xol/ButtonStac.h>
#include <Xol/ListPane.h>
#include <Xol/Mag.h>
#include <Xol/OblongButt.h>
#include <Xol/Pushpin.h>
#include <Xol/RectButton.h>
#include <Xol/Scrollbar.h>
#include <Xol/Slider.h>
#include <Xol/StaticText.h>
#include <Xol/Stub.h>
#include <Xol/TextEdit.h>
#include <Xol/TextPane.h>

#include <Xol/Flat.h>
#include <Xol/FExclusive.h>
#include <Xol/FNonexclus.h>
#include <Xol/FCheckBox.h>

/* Under TopLevelShell */
#include <Xol/BaseWindow.h>
#include <Xol/Menu.h>
#include <Xol/Notice.h>
#include <Xol/PopupWindo.h>

/* Under Constraint */
#include <Xol/Manager.h>
#include <X11/Xp/Table.h>

/* Under Manager */
#include <Xol/BulletinBo.h>
#include <Xol/Caption.h>
#include <Xol/CheckBox.h>
#include <Xol/ControlAre.h>
#include <Xol/Exclusives.h>
#include <Xol/FooterPane.h>
#include <Xol/Form.h>
#include <Xol/Help.h>
#include <Xol/Nonexclusi.h>
#include <Xol/ScrolledWi.h>
#include <Xol/ScrollingL.h>
#include <Xol/TextField.h>
#include <Xol/Text.h>

#endif

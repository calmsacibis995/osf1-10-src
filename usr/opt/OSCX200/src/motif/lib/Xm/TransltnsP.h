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
/*
 * @(#)$RCSfile: TransltnsP.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/18 20:10:09 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: TransltnsP.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/10/18 20:10:09 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTransltnsP_h
#define _XmTransltnsP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifndef _XmConst
#define _XmConst
#endif

#ifdef VMS
/* VMS limit of 31 characters in extern names */
#define _XmBulletinB_defaultTranslations		_XmBulletinB_defaultTranslation
#define _XmDrawingA_traversalTranslations		_XmDrawingA_traversalTranslatio
#define _XmManager_managerTraversalTranslations		_XmManager_managerTraversalTran
#define _XmPrimitive_defaultTranslations		_XmPrimitive_defaultTranslation
#define _XmRowColumn_menu_traversal_table		_XmRowColumn_menu_traversal_tab
#define _XmScrollBar_defaultTranslations		_XmScrollBar_defaultTranslation
#define _XmScrolledW_ScrolledWindowXlations		_XmScrolledW_ScrolledWindowXlat
#define _XmScrolledW_ClipWindowTranslationTable		_XmScrolledW_ClipWindowTranslat
#define _XmScrolledW_WorkWindowTranslationTable		_XmScrolledW_WorkWindowTranslat
#define _XmSelectioB_defaultTextAccelerators		_XmSelectioB_defaultTextAcceler
#define _XmTearOffB_overrideTranslations		_XmTearOffB_overrideTranslation
#define _XmVirtKeys_fallbackBindingString		_XmVirtKeys_fallbackBindingStri
#define _XmVirtKeys_acornFallbackBindingString		_XmVirtKeys_acornFallbackBindin
#define _XmVirtKeys_apolloFallbackBindingString		_XmVirtKeys_apolloFallbackBindi
#define _XmVirtKeys_dgFallbackBindingString		_XmVirtKeys_dgFallbackBindingSt
#define _XmVirtKeys_decFallbackBindingString		_XmVirtKeys_decFallbackBindingS
#ifdef DEC_MOTIF_EXTENSION
#define _XmVirtKeys_declkf11FallbackBindingString	_XmVirtKeys_declkf11FallbackBin
#define _XmVirtKeys_decpcFallbackBindingString		_XmVirtKeys_decpcFallbackBindin
#endif
#define _XmVirtKeys_dblclkFallbackBindingString		_XmVirtKeys_dblclkFallbackBindi
#define _XmVirtKeys_hpFallbackBindingString		_XmVirtKeys_hpFallbackBindingSt
#define _XmVirtKeys_ibmFallbackBindingString		_XmVirtKeys_ibmFallbackBindingS
#define _XmVirtKeys_ingrFallbackBindingString		_XmVirtKeys_ingrFallbackBinding
#define _XmVirtKeys_megatekFallbackBindingString	_XmVirtKeys_megatekFallbackBind
#define _XmVirtKeys_motorolaFallbackBindingString	_XmVirtKeys_motorolaFallbackBin
#define _XmVirtKeys_sgiFallbackBindingString		_XmVirtKeys_sgiFallbackBindingS
#define _XmVirtKeys_siemensWx200FallbackBindingString	_XmVirtKeys_siemensWx200Fallbac
#define _XmVirtKeys_siemens9733FallbackBindingString	_XmVirtKeys_siemens9733Fallback
#define _XmVirtKeys_sunFallbackBindingString		_XmVirtKeys_sunFallbackBindingS
#define _XmVirtKeys_tekFallbackBindingString		_XmVirtKeys_tekFallbackBindingS
#endif

#ifdef DEC_MOTIF_BUG_FIX
externalref _XmConst char _XmArrowB_defaultTranslations[];
externalref _XmConst char _XmBulletinB_defaultTranslations[];
externalref _XmConst char _XmCascadeB_menubar_events[];
externalref _XmConst char _XmCascadeB_p_events[];
externalref _XmConst char _XmDrawingA_defaultTranslations[];
externalref _XmConst char _XmDrawingA_traversalTranslations[];
externalref _XmConst char _XmDrawnB_defaultTranslations[];
externalref _XmConst char _XmFrame_defaultTranslations[];
externalref _XmConst char _XmLabel_defaultTranslations[];
externalref _XmConst char _XmLabel_menuTranslations[];
externalref _XmConst char _XmLabel_menu_traversal_events[];
externalref _XmConst char _XmList_ListXlations1[];
externalref _XmConst char _XmList_ListXlations2[];
externalref _XmConst char _XmManager_managerTraversalTranslations[];
externalref _XmConst char _XmManager_defaultTranslations[];
externalref _XmConst char _XmMenuShell_translations [];
externalref _XmConst char _XmPrimitive_defaultTranslations[];
externalref _XmConst char _XmPushB_defaultTranslations[];
externalref _XmConst char _XmPushB_menuTranslations[];
externalref _XmConst char _XmRowColumn_menu_traversal_table[];
externalref _XmConst char _XmRowColumn_bar_table[];
externalref _XmConst char _XmRowColumn_option_table[];
externalref _XmConst char _XmRowColumn_menu_table[];
externalref _XmConst char _XmSash_defTranslations[];
externalref _XmConst char _XmScrollBar_defaultTranslations[];
externalref _XmConst char _XmScrolledW_ScrolledWindowXlations[];
externalref _XmConst char _XmScrolledW_ClipWindowTranslationTable[];
externalref _XmConst char _XmScrolledW_WorkWindowTranslationTable[];
externalref _XmConst char _XmSelectioB_defaultTextAccelerators[];
externalref _XmConst char _XmTearOffB_overrideTranslations[];
externalref _XmConst char _XmTextF_EventBindings1[];
externalref _XmConst char _XmTextF_EventBindings2[]; 
externalref _XmConst char _XmTextF_EventBindings3[];
externalref _XmConst char _XmTextIn_XmTextEventBindings1[];
externalref _XmConst char _XmTextIn_XmTextEventBindings2[];
externalref _XmConst char _XmTextIn_XmTextEventBindings3[];
externalref _XmConst char _XmTextIn_XmTextEventBindings3[];
externalref _XmConst char _XmToggleB_defaultTranslations[];
externalref _XmConst char _XmToggleB_menuTranslations[];
externalref _XmConst char _XmVirtKeys_fallbackBindingString[];

/* The following keybindings have been "grandfathered" 
   for backward compatablility.
*/
externalref _XmConst char _XmVirtKeys_acornFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_apolloFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_dgFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_decFallbackBindingString[];
#ifdef DEC_MOTIF_EXTENSION
externalref _XmConst char _XmVirtKeys_declkf11FallbackBindingString[];
externalref _XmConst char _XmVirtKeys_decpcFallbackBindingString[];
#endif
externalref _XmConst char _XmVirtKeys_dblclkFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_hpFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_ibmFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_ingrFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_megatekFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_motorolaFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_sgiFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_siemensWx200FallbackBindingString[];
externalref _XmConst char _XmVirtKeys_siemens9733FallbackBindingString[];
externalref _XmConst char _XmVirtKeys_sunFallbackBindingString[];
externalref _XmConst char _XmVirtKeys_tekFallbackBindingString[];
#else
extern _XmConst char _XmArrowB_defaultTranslations[];
extern _XmConst char _XmBulletinB_defaultTranslations[];
extern _XmConst char _XmCascadeB_menubar_events[];
extern _XmConst char _XmCascadeB_p_events[];
extern _XmConst char _XmDrawingA_defaultTranslations[];
extern _XmConst char _XmDrawingA_traversalTranslations[];
extern _XmConst char _XmDrawnB_defaultTranslations[];
extern _XmConst char _XmFrame_defaultTranslations[];
extern _XmConst char _XmLabel_defaultTranslations[];
extern _XmConst char _XmLabel_menuTranslations[];
extern _XmConst char _XmLabel_menu_traversal_events[];
extern _XmConst char _XmList_ListXlations1[];
extern _XmConst char _XmList_ListXlations2[];
extern _XmConst char _XmManager_managerTraversalTranslations[];
extern _XmConst char _XmManager_defaultTranslations[];
extern _XmConst char _XmMenuShell_translations [];
extern _XmConst char _XmPrimitive_defaultTranslations[];
extern _XmConst char _XmPushB_defaultTranslations[];
extern _XmConst char _XmPushB_menuTranslations[];
extern _XmConst char _XmRowColumn_menu_traversal_table[];
extern _XmConst char _XmRowColumn_bar_table[];
extern _XmConst char _XmRowColumn_option_table[];
extern _XmConst char _XmRowColumn_menu_table[];
extern _XmConst char _XmSash_defTranslations[];
extern _XmConst char _XmScrollBar_defaultTranslations[];
extern _XmConst char _XmScrolledW_ScrolledWindowXlations[];
extern _XmConst char _XmScrolledW_ClipWindowTranslationTable[];
extern _XmConst char _XmScrolledW_WorkWindowTranslationTable[];
extern _XmConst char _XmSelectioB_defaultTextAccelerators[];
extern _XmConst char _XmTearOffB_overrideTranslations[];
extern _XmConst char _XmTextF_EventBindings1[];
extern _XmConst char _XmTextF_EventBindings2[]; 
extern _XmConst char _XmTextF_EventBindings3[];
extern _XmConst char _XmTextIn_XmTextEventBindings1[];
extern _XmConst char _XmTextIn_XmTextEventBindings2[];
extern _XmConst char _XmTextIn_XmTextEventBindings3[];
extern _XmConst char _XmTextIn_XmTextEventBindings3[];
extern _XmConst char _XmToggleB_defaultTranslations[];
extern _XmConst char _XmToggleB_menuTranslations[];
extern _XmConst char _XmVirtKeys_fallbackBindingString[];

/* The following keybindings have been "grandfathered" 
   for backward compatablility.
*/
extern _XmConst char _XmVirtKeys_acornFallbackBindingString[];
extern _XmConst char _XmVirtKeys_apolloFallbackBindingString[];
extern _XmConst char _XmVirtKeys_dgFallbackBindingString[];
extern _XmConst char _XmVirtKeys_decFallbackBindingString[];
#ifdef DEC_MOTIF_EXTENSION
extern _XmConst char _XmVirtKeys_declkf11FallbackBindingString[];
extern _XmConst char _XmVirtKeys_decpcFallbackBindingString[];
#endif
extern _XmConst char _XmVirtKeys_dblclkFallbackBindingString[];
extern _XmConst char _XmVirtKeys_hpFallbackBindingString[];
extern _XmConst char _XmVirtKeys_ibmFallbackBindingString[];
extern _XmConst char _XmVirtKeys_ingrFallbackBindingString[];
extern _XmConst char _XmVirtKeys_megatekFallbackBindingString[];
extern _XmConst char _XmVirtKeys_motorolaFallbackBindingString[];
extern _XmConst char _XmVirtKeys_sgiFallbackBindingString[];
extern _XmConst char _XmVirtKeys_siemensWx200FallbackBindingString[];
extern _XmConst char _XmVirtKeys_siemens9733FallbackBindingString[];
extern _XmConst char _XmVirtKeys_sunFallbackBindingString[];
extern _XmConst char _XmVirtKeys_tekFallbackBindingString[];
#endif

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmTransltnsP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

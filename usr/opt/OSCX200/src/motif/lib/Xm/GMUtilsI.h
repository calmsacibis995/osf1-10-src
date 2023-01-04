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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: GMUtilsI.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/15 16:33:17 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmGMUtilsI_h
#define _XmGMUtilsI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif


/* Include files:
*/
#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern void _XmGMCalcSize() ;
extern Boolean _XmGMDoLayout() ;
extern void _XmGMEnforceMargin() ;
extern XtGeometryResult _XmGMHandleQueryGeometry() ;
extern Boolean _XmGMOverlap() ;
extern XtGeometryResult _XmGMHandleGeometryManager() ;
extern XtGeometryResult _XmGMReplyToQueryGeometry() ;

#else

extern void _XmGMCalcSize( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        Dimension *replyWidth,
                        Dimension *replyHeight) ;
extern Boolean _XmGMDoLayout( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy,
#if NeedWidePrototypes
                        int queryonly) ;
#else
                        Boolean queryonly) ;
#endif /* NeedWidePrototypes */
extern void _XmGMEnforceMargin( 
                        XmManagerWidget manager,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
                        int setvalue) ;
#else
                        Dimension margin_width,
                        Dimension margin_height,
                        Boolean setvalue) ;
#endif /* NeedWidePrototypes */
extern XtGeometryResult _XmGMHandleQueryGeometry( 
                        Widget widget,
                        XtWidgetGeometry *intended,
                        XtWidgetGeometry *desired,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy) ;
extern Boolean _XmGMOverlap( 
                        XmManagerWidget manager,
                        Widget w) ;
extern XtGeometryResult _XmGMHandleGeometryManager( 
                        Widget parent,
                        Widget w,
                        XtWidgetGeometry *request,
                        XtWidgetGeometry *reply,
#if NeedWidePrototypes
                        int margin_width,
                        int margin_height,
#else
                        Dimension margin_width,
                        Dimension margin_height,
#endif /* NeedWidePrototypes */
                        int resize_policy,
                        int allow_overlap) ;
extern XtGeometryResult _XmGMReplyToQueryGeometry( 
                        Widget widget,
                        XtWidgetGeometry *intended,
                        XtWidgetGeometry *desired) ;
#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmGMUtilsI_h */
 /* DON'T ADD STUFF AFTER THIS #endif */

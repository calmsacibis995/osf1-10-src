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
 * @(#)$RCSfile: VendorSP.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:57:02 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
/*   $RCSfile: VendorSP.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:57:02 $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef  _XmVendorSP_h
#define _XmVendorSP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifndef _XmNO_BC_INCL
#define _XmNO_BC_INCL
#endif

#include <Xm/XmP.h>
#include <Xm/VendorS.h>
#include <X11/ShellP.h>
#include <Xm/DesktopP.h>
#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref VendorShellClassRec  vendorShellClassRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern Cardinal _XmFilterResources() ;
extern void _XmAddGrab() ;
extern void _XmRemoveGrab() ;
extern XtGeometryResult _XmRootGeometryManager() ;
extern void _XmVendorExtRealize() ;
extern Display * _XmGetDefaultDisplay() ;

#else

extern Cardinal _XmFilterResources( 
                        XtResource *resources,
                        Cardinal numResources,
                        WidgetClass filterClass,
                        XtResource **filteredResourcesRtn) ;
extern void _XmAddGrab( 
                        Widget wid,
#if NeedWidePrototypes
                        int exclusive,
                        int spring_loaded) ;
#else
                        Boolean exclusive,
                        Boolean spring_loaded) ;
#endif /* NeedWidePrototypes */
extern void _XmRemoveGrab( 
                        Widget wid) ;
extern XtGeometryResult _XmRootGeometryManager( 
                        Widget w,
                        XtWidgetGeometry *request,
                        XtWidgetGeometry *reply) ;
extern void _XmVendorExtRealize( 
                        Widget w,
                        XtPointer closure,
                        XtPointer call_data) ;
extern Display * _XmGetDefaultDisplay( void ) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif  /* _XmVendorSP_h */

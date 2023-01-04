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
 * @(#)$RCSfile: RCUtilsP.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:45:07 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: RCUtilsP.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:45:07 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
#ifndef _XmRCUtilsP_h
#define _XmRCUtilsP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/RowColumnP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern XmRCKidGeometry _XmRCGetKidGeo() ;
extern void _XmRCSetKidGeo() ;

#else

extern XmRCKidGeometry _XmRCGetKidGeo( 
                        Widget wid,
                        Widget instigator,
                        XtWidgetGeometry *request,
                        int uniform_border,
#if NeedWidePrototypes
                        int border,
#else
                        Dimension border,
#endif /* NeedWidePrototypes */
                        int uniform_width_margins,
                        int uniform_height_margins,
                        Widget help,
			Widget toc,
                        int geo_type) ;
extern void _XmRCSetKidGeo( 
                        XmRCKidGeometry kg,
                        Widget instigator) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif  /* _XmRCUtilsP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

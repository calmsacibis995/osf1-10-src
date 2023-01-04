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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: CascadeB.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:30:09 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmCascadeB_h
#define _XmCascadeB_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmCascadeButtonWidgetClass;

typedef struct _XmCascadeButtonRec      * XmCascadeButtonWidget;
typedef struct _XmCascadeButtonClassRec * XmCascadeButtonWidgetClass;

/* fast subclass define */
#ifndef XmIsCascadeButton 
#define XmIsCascadeButton(w) XtIsSubclass(w, xmCascadeButtonWidgetClass)
#endif /* XmIsCascadeButton */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateCascadeButton() ;
extern void XmCascadeButtonHighlight() ;

#else

extern Widget XmCreateCascadeButton( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;
extern void XmCascadeButtonHighlight( 
                        Widget cb,
#if NeedWidePrototypes
                        int highlight) ;
#else
                        Boolean highlight) ;
#endif /* NeedWidePrototypes */

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmCascadeB_h */
/* DON'T ADD STUFF AFTER THIS #endif */

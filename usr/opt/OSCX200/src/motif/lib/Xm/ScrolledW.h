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
/*   $RCSfile: ScrolledW.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:48:39 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScrolledWindow_h
#define _XmScrolledWindow_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsScrolledWindow
#define XmIsScrolledWindow(w)	XtIsSubclass(w, xmScrolledWindowWidgetClass)
#endif /* XmIsScrolledWindow */


externalref WidgetClass xmScrolledWindowWidgetClass;

typedef struct _XmScrolledWindowClassRec * XmScrolledWindowWidgetClass;
typedef struct _XmScrolledWindowRec      * XmScrolledWindowWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void XmScrolledWindowSetAreas() ;
extern Widget XmCreateScrolledWindow() ;
extern void XmScrollVisible() ;

#else

extern void XmScrolledWindowSetAreas( 
                        Widget w,
                        Widget hscroll,
                        Widget vscroll,
                        Widget wregion) ;
extern Widget XmCreateScrolledWindow( 
                        Widget parent,
                        char *name,
                        ArgList args,
                        Cardinal argCount) ;
extern void XmScrollVisible(
			Widget      	scrw,
			Widget          wid,
			Dimension       hor_margin, 
			Dimension       ver_margin) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmScrolledWindow_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

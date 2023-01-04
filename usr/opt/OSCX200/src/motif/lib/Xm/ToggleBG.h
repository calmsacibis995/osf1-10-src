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
/*   $RCSfile: ToggleBG.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:54:53 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/***********************************************************************
 *
 * Toggle Gadget
 *
 ***********************************************************************/
#ifndef _XmToggleG_h
#define _XmToggleG_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


externalref WidgetClass xmToggleButtonGadgetClass;

typedef struct _XmToggleButtonGadgetClassRec     *XmToggleButtonGadgetClass;
typedef struct _XmToggleButtonGadgetRec          *XmToggleButtonGadget;
typedef struct _XmToggleButtonGCacheObjRec       *XmToggleButtonGCacheObject;


/*fast subclass define */
#ifndef XmIsToggleButtonGadget
#define XmIsToggleButtonGadget(w)     XtIsSubclass(w, xmToggleButtonGadgetClass)
#endif /* XmIsToggleButtonGadget */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Boolean XmToggleButtonGadgetGetState() ;
extern void XmToggleButtonGadgetSetState() ;
extern Widget XmCreateToggleButtonGadget() ;

#else

extern Boolean XmToggleButtonGadgetGetState( 
                        Widget w) ;
extern void XmToggleButtonGadgetSetState( 
                        Widget w,
#if NeedWidePrototypes
                        int newstate,
                        int notify) ;
#else
                        Boolean newstate,
                        Boolean notify) ;
#endif /* NeedWidePrototypes */
extern Widget XmCreateToggleButtonGadget( 
                        Widget parent,
                        char *name,
                        Arg *arglist,
                        Cardinal argCount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmToggleG_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

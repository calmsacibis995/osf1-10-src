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
 * @(#)$RCSfile: WorldP.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:57:46 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WorldP.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:57:46 $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef _XmWorldP_h
#define _XmWorldP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/DesktopP.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmIsWorldObject
#define XmIsWorldObject(w)	XtIsSubclass(w, xmWorldClass)
#endif /* XmIsWorldObject */

typedef struct _XmWorldRec *XmWorldObject;
typedef struct _XmWorldClassRec *XmWorldObjectClass;
externalref WidgetClass xmWorldClass;


typedef struct _XmWorldClassPart{
    XtPointer		extension;
}XmWorldClassPart, *XmWorldClassPartPtr;

typedef struct _XmWorldClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmWorldClassPart		world_class;
}XmWorldClassRec;

typedef struct {
    int				foo;
} XmWorldPart, *XmWorldPartPtr;

externalref XmWorldClassRec 	xmWorldClassRec;

typedef struct _XmWorldRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmWorldPart			world;
}XmWorldRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern XmWorldObject _XmGetWorldObject() ;

#else

extern XmWorldObject _XmGetWorldObject( 
                        Widget shell,
                        ArgList args,
                        Cardinal *num_args) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmWorldP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

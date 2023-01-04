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
/*   $RCSfile: FrameP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:38:27 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmFrameP_h
#define _XmFrameP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Frame.h>
#include <Xm/ManagerP.h>

#ifdef __cplusplus
extern "C" {
#endif



/* Full class records */

typedef struct
{
   XtPointer extension;
} XmFrameClassPart;

typedef struct _XmFrameClassRec
{
   CoreClassPart       core_class;
   CompositeClassPart  composite_class;
   ConstraintClassPart constraint_class;
   XmManagerClassPart  manager_class;
   XmFrameClassPart    frame_class;
} XmFrameClassRec;

externalref XmFrameClassRec xmFrameClassRec;


/*  Frame instance records  */

typedef struct
{
   Dimension margin_width;
   Dimension margin_height;
   unsigned char shadow_type;
   Dimension old_width;
   Dimension old_height;
   Dimension old_shadow_thickness;
   Position old_shadow_x;
   Position old_shadow_y;
   Widget work_area;
   Widget title_area;
   Boolean processing_constraints;
} XmFramePart;

typedef struct _XmFrameRec
{
    CorePart	   core;
    CompositePart  composite;
    ConstraintPart constraint;
    XmManagerPart  manager;
    XmFramePart    frame;
} XmFrameRec;


/*  Frame constraint records  */

typedef struct _XmFrameConstraintPart
{
   int unused;
   unsigned char child_type;
   unsigned char child_h_alignment;
   Dimension child_h_spacing;
   unsigned char child_v_alignment;
} XmFrameConstraintPart, * XmFrameConstraint;

typedef struct _XmFrameConstraintRec
{
   XmManagerConstraintPart manager;
   XmFrameConstraintPart   frame;
} XmFrameConstraintRec, * XmFrameConstraintPtr;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmFrameP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

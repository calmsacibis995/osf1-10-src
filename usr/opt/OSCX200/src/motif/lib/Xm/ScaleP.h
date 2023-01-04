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
/*   $RCSfile: ScaleP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:47:30 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScaleP_h
#define _XmScaleP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif


#include <Xm/Scale.h>
#include <Xm/ManagerP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  New fields for the Scale widget class record  */

typedef struct
{
   int mumble;   /* No new procedures */
} XmScaleClassPart;


/* Full class record declaration */

typedef struct _XmScaleClassRec
{
   CoreClassPart       core_class;
   CompositeClassPart  composite_class;
   ConstraintClassPart constraint_class;
   XmManagerClassPart  manager_class;
   XmScaleClassPart    scale_class;
} XmScaleClassRec;

externalref XmScaleClassRec xmScaleClassRec;


/* New fields for the Scale widget record */

typedef struct
{
   int            value;
   int            maximum;
   int            minimum;
   unsigned char  orientation;
   unsigned char  processing_direction;
   XmString       title; 
   XmFontList     font_list;
   XFontStruct  * font_struct;
   Boolean        show_value;
   short          decimal_points;
   Dimension      scale_width;
   Dimension      scale_height;
   Dimension      highlight_thickness;
   Boolean        highlight_on_enter;
   XtCallbackList drag_callback;
   XtCallbackList value_changed_callback;

   int last_value;
   int slider_size;
   GC  foreground_GC;
   int show_value_x;
   int show_value_y;
   int show_value_width;
   int show_value_height;
   int scale_multiple;
} XmScalePart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmScaleRec
{
    CorePart       core;
    CompositePart  composite;
    ConstraintPart constraint;
    XmManagerPart  manager;
    XmScalePart    scale;
} XmScaleRec;


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
#endif /* _XmScaleP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

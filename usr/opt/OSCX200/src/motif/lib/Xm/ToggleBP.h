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
/*   $RCSfile: ToggleBP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:55:04 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/********************************************
 *
 *   No new fields need to be defined
 *   for the Toggle widget class record
 *
 ********************************************/

#ifndef _XmToggleButtonP_h
#define _XmToggleButtonP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/ToggleB.h>
#include <Xm/LabelP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmToggleButtonClassPart
 {
   int foo; /* No new fields needed */
 } XmToggleButtonClassPart;


/****************************************************
 *
 * Full class record declaration for Toggle class
 *
 ****************************************************/
typedef struct _XmToggleButtonClassRec {
    CoreClassPart	  	core_class;
    XmPrimitiveClassPart  	primitive_class;
    XmLabelClassPart      	label_class;
    XmToggleButtonClassPart	toggle_class;
} XmToggleButtonClassRec;


externalref XmToggleButtonClassRec xmToggleButtonClassRec;


/********************************************
 *
 * No new fields needed for instance record
 *
 ********************************************/

typedef struct _XmToggleButtonPart
{ 
   unsigned char	ind_type;
   Boolean		visible;
   Dimension		spacing;
   Dimension		indicator_dim;
   Boolean		indicator_set;
   Pixmap		on_pixmap; 
   Pixmap		insen_pixmap; 
   Boolean		set;
   Boolean     		visual_set; /* used for visuals and does not reflect
                                        the true state of the button */
   Boolean		ind_on;
   Boolean		fill_on_select;
   Pixel		select_color;
   GC			select_GC;
   GC			background_gc;
   XtCallbackList 	value_changed_CB,
                        arm_CB,
                        disarm_CB;
   Boolean      	Armed;
} XmToggleButtonPart;



/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _XmToggleButtonRec {
    CorePart	        core;
    XmPrimitivePart     primitive;
    XmLabelPart		label;
    XmToggleButtonPart  toggle;
} XmToggleButtonRec;


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
#endif /* _XmToggleButtonP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

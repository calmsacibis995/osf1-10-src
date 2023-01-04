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
/*   $RCSfile: ScrolledWP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:48:46 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScrolledWindowP_h
#define _XmScrolledWindowP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/ManagerP.h>
#include <Xm/ScrolledW.h>

#include <Xm/ScrollBar.h>
#include <Xm/DrawingA.h>

#ifdef __cplusplus
extern "C" {
#endif

/* New fields for the ScrolledWindow widget class record */
typedef struct {
     int mumble;   /* No new procedures */
} XmScrolledWindowClassPart;

/****************
 *
 * Class record declaration
 *
 ****************/
typedef struct _XmScrolledWindowClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    XmScrolledWindowClassPart	swindow_class;
} XmScrolledWindowClassRec;

externalref XmScrolledWindowClassRec xmScrolledWindowClassRec;

/****************
 *
 * Scrolled Window instance structure.
 *
 ****************/
typedef struct {

   int vmin;		  /*  slider minimum coordiate position     */
   int vmax;		  /*  slider maximum coordiate position     */
   int vOrigin;		  /*  slider edge location                  */
   int vExtent;		  /*  slider size                           */

   int hmin;		  /*  Same as above for horizontal bar.     */
   int hmax;
   int hOrigin;
   int hExtent;

   Position hsbX,hsbY;
   Dimension hsbWidth,hsbHeight;    /* Dimensions for the horiz bar */

   Position vsbX,vsbY;
   Dimension vsbWidth,vsbHeight;    /* Dimensions for the vertical bar */

   Dimension    GivenHeight, GivenWidth;

   Dimension	AreaWidth,AreaHeight;
   Dimension	WidthPad,HeightPad;
   Position	XOffset, YOffset;

   Dimension	pad;

   Boolean	hasHSB;
   Boolean	hasVSB;
   Boolean	InInit;
   Boolean	FromResize;

   unsigned char	VisualPolicy;
   unsigned char	ScrollPolicy;
   unsigned char	ScrollBarPolicy;
   unsigned char	Placement;
   
   XmScrollBarWidget   	hScrollBar;
   XmScrollBarWidget   	vScrollBar;
   XmDrawingAreaWidget 	ClipWindow;
   Widget              	WorkWindow;
   
   XtCallbackList       traverseObscuredCallback;
} XmScrolledWindowPart;


/************************************************************************
 *									*
 * Full instance record declaration					*
 *									*
 ************************************************************************/

typedef struct _XmScrolledWindowRec {
    CorePart	    core;
    CompositePart   composite;
    ConstraintPart constraint;
    XmManagerPart   manager;
    XmScrolledWindowPart   swindow;
} XmScrolledWindowRec;

#define DEFAULT_HEIGHT 20
#define DEFAULT_WIDTH 20


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern char * _XmGetRealXlations() ;
extern void _XmInitializeScrollBars() ;

#else

extern char * _XmGetRealXlations( 
                        Display *dpy,
                        _XmBuildVirtualKeyStruct *keys,
                        int num_keys) ;
extern void _XmInitializeScrollBars( 
                        Widget w) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmScrolledWindowP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

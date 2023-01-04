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
 * Motif Release 1.2.1
*/ 
/*   $RCSfile: SeparatoGP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:49:36 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmSeparatorGadgetP_h
#define _XmSeparatorGadgetP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/SeparatoG.h>
#include <Xm/GadgetP.h>
#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************/
/* The Separator Gadget Cache Object's class and instance records*/
/*****************************************************************/

typedef struct _XmSeparatorGCacheObjClassPart
{
    int foo;
} XmSeparatorGCacheObjClassPart;


typedef struct _XmSeparatorGCacheObjClassRec  /* separator cache class record */
{
    ObjectClassPart                     object_class;
    XmExtClassPart                      ext_class;
    XmSeparatorGCacheObjClassPart       separator_class_cache;
} XmSeparatorGCacheObjClassRec;

externalref XmSeparatorGCacheObjClassRec xmSeparatorGCacheObjClassRec;


/*  The Separator Gadget Cache instance record  */

typedef struct _XmSeparatorGCacheObjPart
{
   Dimension	  margin;
   unsigned char  orientation;
   unsigned char  separator_type;
   GC             separator_GC;
} XmSeparatorGCacheObjPart;

typedef struct _XmSeparatorGCacheObjRec
{
  ObjectPart                object;
  XmExtPart		    ext;
  XmSeparatorGCacheObjPart  separator_cache;
} XmSeparatorGCacheObjRec;


/*****************************************************/
/*  The Separator Widget Class and instance records  */
/*****************************************************/

typedef struct _XmSeparatorGadgetClassPart
{
   XtPointer               extension;
} XmSeparatorGadgetClassPart;


/*  Full class record declaration for Separator class  */

typedef struct _XmSeparatorGadgetClassRec
{
   RectObjClassPart            rect_class;
   XmGadgetClassPart           gadget_class;
   XmSeparatorGadgetClassPart  separator_class;
} XmSeparatorGadgetClassRec;

externalref XmSeparatorGadgetClassRec xmSeparatorGadgetClassRec;

typedef struct _XmSeparatorGadgetPart
{
  XmSeparatorGCacheObjPart  *cache;
} XmSeparatorGadgetPart;

/*  Full instance record declaration  */

typedef struct _XmSeparatorGadgetRec
{
   ObjectPart             object;
   RectObjPart            rectangle;
   XmGadgetPart           gadget;
   XmSeparatorGadgetPart  separator;
} XmSeparatorGadgetRec;

/* MACROS for accessing instance fields*/
#define SEPG_Margin(w)			(((XmSeparatorGadget)(w))->   \
					   separator.cache->margin)
#define SEPG_Orientation(w)		(((XmSeparatorGadget)(w))->   \
					   separator.cache->orientation)
#define SEPG_SeparatorType(w)		(((XmSeparatorGadget)(w))->   \
					   separator.cache->separator_type)
#define SEPG_SeparatorGC(w)		(((XmSeparatorGadget)(w))->   \
					   separator.cache->separator_GC)

/* Convenience Macros */
#define SEPG_Cache(w)                    (((XmSeparatorGadget)(w))->\
					   separator.cache)
#define SEPG_ClassCachePart(w) \
        (((XmSeparatorGadgetClass)xmSeparatorGadgetClass)->gadget_class.cache_part)


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern int _XmSeparatorCacheCompare() ;

#else

extern int _XmSeparatorCacheCompare( 
                        XtPointer A,
                        XtPointer B) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmSeparatorGadgetP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

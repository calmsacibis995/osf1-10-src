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
/*   $RCSfile: SeparatorP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:49:54 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmSeparatorP_h
#define _XmSeparatorP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Separator.h>
#include <Xm/PrimitiveP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Separator class structure  */

typedef struct _XmSeparatorClassPart
{
   int foo;	/*  No new fields needed  */
} XmSeparatorClassPart;


/*  Full class record declaration for Separator class  */

typedef struct _XmSeparatorClassRec
{
   CoreClassPart         core_class;
   XmPrimitiveClassPart  primitive_class;
   XmSeparatorClassPart  separator_class;
} XmSeparatorClassRec;

externalref XmSeparatorClassRec xmSeparatorClassRec;


/*  The Separator instance record  */

typedef struct _XmSeparatorPart
{
   Dimension	  margin;
   unsigned char  orientation;
   unsigned char  separator_type;
   GC             separator_GC;
} XmSeparatorPart;


/*  Full instance record declaration  */

typedef struct _XmSeparatorRec
{
   CorePart	    core;
   XmPrimitivePart  primitive;
   XmSeparatorPart  separator;
} XmSeparatorRec;


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
#endif /* _XmSeparatorP_h */
/* DON'T ADD STUFF AFTER THIS #endif */

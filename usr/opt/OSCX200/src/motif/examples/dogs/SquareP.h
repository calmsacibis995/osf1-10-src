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
 * @(#)$RCSfile: SquareP.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 16:27:15 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 

/*   $RCSfile: SquareP.h,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 16:27:15 $ */

#ifndef _SquareP_h
#define _SquareP_h

#ifdef DEC_MOTIF_BUG_FIX
#include "Square.h"
#else
#include <Square.h>
#endif
#include <Xm/BulletinBP.h>

#define SquareIndex (XmBulletinBIndex + 1)

typedef struct _SquareClassPart
{
   XtPointer reserved;
} SquareClassPart;


typedef struct _SquareClassRec
{
   CoreClassPart       core_class;
   CompositeClassPart  composite_class;
   ConstraintClassPart constraint_class;
   XmManagerClassPart  manager_class;
   XmBulletinBoardClassPart  bulletin_board_class;
   SquareClassPart     square_class;
} SquareClassRec;

externalref SquareClassRec squareClassRec;

typedef struct _SquarePart
{
    int major_dimension;
} SquarePart;


/*  Full instance record declaration  */

typedef struct _SquareRec
{
   CorePart	  core;
   CompositePart  composite;
   ConstraintPart constraint;
   XmManagerPart  manager;
   XmBulletinBoardPart  bulletin_board;
   SquarePart     square;
} SquareRec;

typedef struct _SquareConstraintPart
{
   Boolean make_square;
} SquareConstraintPart;

typedef struct _SquareConstraintRec
{
   XmManagerConstraintPart manager;
   SquareConstraintPart    square;
} SquareConstraintRec, *SquareConstraint;


#endif /* _SquareP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

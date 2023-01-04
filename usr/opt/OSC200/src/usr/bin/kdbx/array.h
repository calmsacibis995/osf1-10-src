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
 * @(#)$RCSfile: array.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:47:25 $
 */
#ifndef __ARRAY_H_
#define __ARRAY_H_

/*      array.h (ULTRIX) 11/27/90        */

/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1990 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   This software is  derived  from  software  received  from  the     *
 *   University    of   California,   Berkeley,   and   from   Bell     *
 *   Laboratories.  Use, duplication, or disclosure is  subject  to     *
 *   restrictions  under  license  agreements  with  University  of     *
 *   California and with AT&T.                                          *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/

#include <util.h>

typedef struct _Array {
  unsigned int alloced;
  unsigned int used;
  char *elements;
} Array;

#define ArrayAlloced(a) ((a)->alloced)
#define ArrayUsed(a) ((a)->used)
#define ArrayElements(a) ((a)->elements)
#ifdef notdef
#define ArrayElement(a, n) (&((a)->elements[(n)]))
#else
#define ArrayElement(a, n) (((a)->elements + (n)))
#endif
#define ArrayIndex(a, e, s) \
  (((unsigned long) (e) - (unsigned long) ArrayElements(a))/(s))

extern Array *ArrayNew();
extern void ArrayDestroy(), ArraySetElement();
extern void ArrayInsert(), ArrayDelete(), ArraySort(), ArrayEmpty();
#ifdef notdef
extern unsigned int ArrayNextIndex(), ArrayIndex();
#endif
extern unsigned int ArrayNextIndex();
extern char *ArrayNextElement();
extern Boolean ArrayContains();

#endif

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

#ifndef _CVT_H_
#define _CVT_H_

#ifdef _NO_PROTO
    extern int cvt_ftof();
#else /* _NO_PROTO */
    extern int cvt_ftof( void *, int, void *, int,  int);
#endif /* _NO_PROTO */


/*
 *  The floating point data types below can be the source or target
 *  of a conversion. 
 */

#define   CVT_VAX_F              1
#define   CVT_VAX_D              2
#define   CVT_VAX_G              3
#define   CVT_VAX_H              4
#define   CVT_IEEE_S             5
#define   CVT_IEEE_T             6
#define   CVT_IEEE_X             7
#define   CVT_BIG_ENDIAN_IEEE_S  8
#define   CVT_BIG_ENDIAN_IEEE_T  9
#define   CVT_BIG_ENDIAN_IEEE_X  10


#endif  /* _CVT_H_ */

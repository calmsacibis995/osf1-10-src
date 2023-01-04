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
/*      @(#)environ.s   9.1 ULTRIX 8/7/90 */
/*
 *      Define environ globally for those who need it, while letting
 *      ANSI purists pretend it doesn't exist (because all the internal
 *      usage in the library uses __environ instead).
 *
/*      @(#)environ.s   9.1 ULTRIX 8/7/90 */
/*
 *      Define environ globally for those who need it, while letting
 *      ANSI purists pretend it doesn't exist (because all the internal
 *      usage in the library uses __environ instead).
 *
 *      It's in sdata for maximum flexibility; this way, it can be
 *      referenced as either large or small.
 */

        .globl  __environ
        .weakext  environ
        .sdata
environ:
__environ:
        .space  8

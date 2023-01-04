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
 * @(#)$RCSfile: types_setup.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/15 11:55:58 $
 */
/* KLUDGE.
 * On OSF, <sys/un.h> defines "u_short sun_family". However,
 * on Ultrix, it's defined as  "short sun_family".  We include 
 * <sys/types.h> here, which defines the "u_" typedefs.
 * <sys/types.h> defines the "u_" typedefs only if _OSF_SOURCE
 * is defined.  To get around that problem, typedefs for the "u_"
 * are explicitly done here if _XOPEN_SOURCE is defined. 
 *
 * This file is called from mom_defs.h, which is #included by every
 * momgen module.
 */

#include <sys/types.h>

#ifdef _XOPEN_SOURCE
#ifndef u_short
typedef uchar_t         u_char;
typedef ushort_t        u_short;
typedef uint_t          u_int;
typedef ulong_t         u_long;
#endif
#endif

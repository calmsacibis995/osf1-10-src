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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: NCisNLchar.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:16:29 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCisNLchar
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sccsid[] = "NCisNLchar.c	1.4  com/lib/c/nls,3.1,9021 3/15/90 14:23:29";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NCisNLchar = __NCisNLchar
#endif
#include <sys/types.h>
#include <NLctype.h>

#ifdef NCisNLchar
#undef NCisNLchar
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define NCisNLchar __NCisNLchar
#endif

int
#ifdef _NO_PROTO
NCisNLchar(c)
int c;
#else
NCisNLchar(int c)
#endif
{
#ifdef _KJI
	if ( (unsigned) c < 0x80)
		return (1);
	if ( (_jctype1_[_jctype0_[(unsigned short)(c) >> 8]] [(c) & 0xff]) != 0)
		return (1);
	else	return (0);
#else
	return ( (unsigned) c < 256 );
#endif
}



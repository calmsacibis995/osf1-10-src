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
static char	*sccsid = "@(#)$RCSfile: NCenc.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:15:58 $";
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
 * FUNCTIONS: NCenc
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
 * NCenc.c	1.2  com/lib/c/nls,3.1,9013 2/27/90 21:28:55
 */

/* Note that this function is NLS/JLS independent */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NCenc = __NCenc
#endif
#include <sys/types.h>
#include <sys/NLchar.h>

#ifdef NCenc
#undef NCenc
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define NCenc __NCenc
#endif

int
#ifdef _NO_PROTO
NCenc(x, c)
wchar_t *x;
char    *c;
#else
NCenc(wchar_t *x, char *c)
#endif
{
	if (*x > 0xff) {			/* multi-byte char? */
		 c[0] = ( (*x >> 8) & 0xff );	/* yes, shift top & store */
		 c[1] = ( *x & 0xff );		/*      store bottom */
		 return (2);			/* return length */
	} else { c[0] = ( *x & 0xff );		/* only one char to string */
		 return (1);			/* return length */
	}
}

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
static char	*sccsid = "@(#)$RCSfile: NCdechr.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:15:49 $";
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
 * FUNCTIONS: NCdechr
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
 * NCdechr.c	1.1.1.1  com/lib/c/nls,3.1,9013 2/28/90 18:30:59
 */

/* This function converts 1 or 2 chars to a "wchar_t" and returns it. */


/*	NOTE! This function is dependent on NLchar.h definition of
 *	what constitutes a multi-byte character (NCisshift) which
 *      differs between NLS and JLS...
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NCdechr = __NCdechr
#endif
#include <sys/types.h>
#include <NLchar.h>

#ifdef NCdechr
#undef NCdechr
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define NCdechr __NCdechr
#endif

int
#ifdef _NO_PROTO
NCdechr(c)
char *c;
#else
NCdechr(char *c)
#endif
{
	if (NCisshift(c[0]))   				 /* multi-byte? */
		 return( ((unsigned char)c[0] << 8) | (c[1] & 0xff) );  
	else 	 return( (unsigned char) c[0] ); 
}

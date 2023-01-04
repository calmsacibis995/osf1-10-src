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
static char	*sccsid = "@(#)$RCSfile: a64l.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:22:52 $";
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
 * COMPONENT_NAME: LIBCCNV a64l
 *
 * FUNCTIONS: a64l
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * a64l.c	1.4  com/lib/c/cnv,3.1,8943 9/13/89 09:18:23
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak a64l = __a64l
#endif
#endif
#define BITSPERCHAR	6 /* to hold entire character set */

/*
 * NAME: a64l
 *                                                                    
 * FUNCTION: convert base 64 ascii to long int
 *                                                                    
 * NOTES:
 * char set is [./0-9A-Za-z]
 *
 * RETURNS: a long value
 *
 */

long
a64l(s)
register char *s;
{
	register int i, c;
	long lg = 0;

	for (i = 0; (c = *s++) != '\0'; i += BITSPERCHAR) {
		if (c > 'Z')
			c -= 'a' - 'Z' - 1;
		if (c > '9')
			c -= 'A' - '9' - 1;
		lg |= (long)(c - ('0' - 2)) << i;
	}
	return (lg);
}

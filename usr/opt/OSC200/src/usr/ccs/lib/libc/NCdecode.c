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
static char	*sccsid = "@(#)$RCSfile: NCdecode.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:15:52 $";
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
 * FUNCTIONS: NCdecode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * NCdecode.c	1.11  com/lib/c/nls,3.1,9013 2/11/90 17:07:57
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NCdecode = __NCdecode
#endif
#include <sys/types.h>
#include <NLchar.h>

/*
 * NAME: NCdecode
 *
 * FUNCTION: Convert 1 or 2 chars to a wchar_t
 *
 * NOTE:     Macro NCisshift tests shift char 
 *
 * RETURN VALUE DESCRIPTION: Return the number of characters converted.
 */
/*
 * Convert 1 or 2 chars to a wchar_t; return # chars converted.
 */
int
NCdecode(c, nlc)
register unsigned char *c;	/* char type string */
register wchar_t *nlc;   /* wchar_t type string */
{
	if ( NCisshift(c[0]) ) {
		nlc[0] = (c[0] << 8) | (c[1] & 0xff);
		return(2);
	}
	nlc[0] = c[0] & 0xff;
	return(1);
}

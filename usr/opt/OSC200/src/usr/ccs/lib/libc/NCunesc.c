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
static char	*sccsid = "@(#)$RCSfile: NCunesc.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:17:19 $";
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
 * FUNCTIONS: NCunesc
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
 * NCunesc.c	1.2  com/lib/c/nls,3.1,9013 3/10/90 12:59:41
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NCunesc = __NCunesc
#endif
#include <sys/types.h>
#include <NLctype.h>

#ifndef KJI
#ifndef _NAME_SPACE_WEAK_STRONG
static int
__NCunesc(char * c, NLchar *nlc)
{
	return(NCunesc(c, nlc));
}
#endif

#ifdef NCunesc
#undef NCunesc
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define NCunesc __NCunesc
#endif

int
NCunesc(char * c, NLchar *nlc)
{
/*  Translate char escape string at c to single NLchar at nlc.
 */
	if (c[0] != '\\' || c[1] != '<' || 
	    _NLunescval(&c[2],c[3] == '>' ? 1 : 2, nlc) < 0) { 
	    nlc[0] = c[0];
	}
	else {
	    if (c[3] == '>') return (4);
	    if (c[4] == '>') return (5);
	    nlc[0] = c[0];
	}
 	return(1);
}

#else

int
NCunesc(src, dest)
register char *src;
register NLchar *dest;
{
	register int length;

	/* return the character if not the start of an escape sequence
	 */
	if (src[0] != '\\' || src[1] != '<')  {
		dest[0] = src[0];	
		return (1);
	}
	
	if (src[4] == '>')
		length = 2;
	else if (src[5] == '>')
		length = 3;
	else if (src[6] == '>')
		length = 4;
	else  {
		dest[0] = src[0];	
		return (1);
	}

	/* start of a mneumonic in unesctab 
	 */
	if (src[2] == 'j')  {
		if (_NLunescval (&src[2], length, dest) == -1)   {
			dest[0] = src[0];
			return (1);
		}
		else
			return (length + 3);
	}

	if (ishexesc(&src[2]) == -1)  {
		dest[0] = src[0];
		return (1);
	}
	else  {
		NCuneschex(src, dest);
		return (length + 3);
	}
}
#endif /* KJI */

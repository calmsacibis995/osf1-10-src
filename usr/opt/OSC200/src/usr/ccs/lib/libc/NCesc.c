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
static char	*sccsid = "@(#)$RCSfile: NCesc.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:16:10 $";
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
 * FUNCTIONS: NCesc
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
 * sccsid[] = "NCesc.c	1.3  com/lib/c/nls,3.1,9021 3/23/90 16:43:35";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NCesc = __NCesc
#endif
#include <sys/types.h>
#include <NLctype.h>

#ifndef KJI
#ifndef _NAME_SPACE_WEAK_STRONG
static int 
__NCesc(NLchar *nlc, char *c)
{
	return(NCesc(nlc, c));
}
#endif

#ifdef NCesc
#undef NCesc
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define NCesc __NCesc
#endif

int
NCesc(NLchar *nlc, char *c) 
{
    	int retcode = 0;
	/* if NLchar value is less than 0x80 then return ascii character */
	if (*nlc < 0x80) { 
	    c[0] = *nlc;
	    retcode++;
	}
	else {
	    c[0] = '\\';
	    c[1] = '<';
	    if (_NLesctsize <= (unsigned)*(nlc)-0x80)  {
	        c[2] = '?';
	        c[3] = '?';
		retcode = 4;
	    }
	    else {
	        c[2] = _NLesctab[(int)(*nlc - 0x80)][0];
		c[3] = _NLesctab[(int)(*nlc - 0x80)][1];
		if (c[3]) {
		    c[4] = '>';
		    retcode = 5;
		}
		else {
		    c[3] = '>';
		    retcode = 4;
		}
	    }
	}
	return(retcode);
}

#else

int
NCesc(src, dest)
register NLchar *src;
register char *dest;
{
	register int length;

	/* the character does not need to be escaped
	 */
	if ((unsigned) *src < 0x80)   {
		dest[0] = src[0];
		return (1);
	}

	/* Try to find the escape sequence in NLesctab
	 */
	if (*src >= MINESCVAL && *src <= MAXESCVAL)
		return (_NLescval (src, dest));

	/* return a hex escape string
	 */
	NCeschex (src, dest);
	return (7);
}
#endif /* KJI */

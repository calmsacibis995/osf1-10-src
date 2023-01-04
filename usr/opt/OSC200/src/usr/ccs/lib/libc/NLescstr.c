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
static char	*sccsid = "@(#)$RCSfile: NLescstr.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:17:38 $";
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
 * FUNCTIONS: NLescstr
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
 * NLescstr.c	1.7  com/lib/c/nls,3.1,9013 1/18/90 09:44:16
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NLescstr = __NLescstr
#endif
#endif
#include <NLctype.h>

/*
 * NAME: NLescstr
 *
 * FUNCTION: Translate each NLS code point in src into a printable esc 
 *	     sequence in standard ASCII unique to that code point.
 *
 * RETURN VALUE DESCRIPTION: The length of a resulting string

 */
/*
 *  Translate each NLS code point in src into a printable escape sequence
 *  in standard ASCII unique to that code point.  Return length of resulting
 *  string.
 */
int
NLescstr(src, dest, dlen)
register unsigned char *src, *dest;	/* a source and destination strings */
register int dlen;       	/* the length of dest string */
{
	register int n;
	NLchar nlc;		        
	unsigned char *odest = dest;	/* the next char position */

	/*  Always NUL-terminate output string, if any; but never count
	 *  NUL as part of length.
	 */
	for ( ; dlen && *src; --dlen)   
		if (n = NLisNLcp(src)) {	/* test if src is NLchars */
			if (dlen < NLESCMAX)
				break;
			nlc = NCdechr(src);	/* change to NLchar */
			src += n;
			n = NCesc(&nlc, dest);	/* Esc sequence */
			dest += n;
			dlen -= n - 1;
		} else
			*dest++ = *src++;
	if (odest < dest) {			/* more char ? */
		if (!dlen)			/* dlen = 0 ? */
			--dest;
		*dest = '\0';
	}
	return (dest - odest);
}

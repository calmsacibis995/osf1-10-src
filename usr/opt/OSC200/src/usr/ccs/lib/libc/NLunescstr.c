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
static char	*sccsid = "@(#)$RCSfile: NLunescstr.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:19:00 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLunescstr
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
 * sccsid[] = "NLunescstr.c      1.7  com/lib/c/nls,3.1,9021 1/18/90 09:47:07";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NLunescstr = __NLunescstr
#endif
#endif
#include <NLctype.h>

/*
 * NAME: NLunescstr
 *
 * FUNCTION: Translate any escape sequences found in src that indicate
 *	     an NLS code point back to the code point represented.
 *
 * RETURN VALUE DESCRIPTION: The number of chars converted.
 */
/*
 *  Translate any escape sequences found in src that indicate an NLS code
 *  point back to the code point represented.
 */
int
NLunescstr(src, dest, dlen)
register unsigned char *src, *dest;
register int dlen;	/* the length to be converted */
{
	register int n;	/* the length of NLchar converted to char */
	NLchar nc;
	unsigned char *odest = dest; /* the next char position */

	/*  Always NUL-terminate output string, if any; but never count
	 *  NUL as part of length.
	 */
	for ( ; dlen && *src; dest += n, dlen -= n) {
		src += NCunesc(src, &nc);
		if (dlen < NCchrlen(nc))
			break;
		n = NCenc(&nc, dest);
	}
	if (odest < dest) {
		if (!dlen)
			--dest;
		*dest = '\0';
	}
	return (dest - odest);
}

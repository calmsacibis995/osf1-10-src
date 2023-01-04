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
static char	*sccsid = "@(#)$RCSfile: NLflatstr.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:17:46 $";
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
 * FUNCTIONS: NLflatstr
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
 * sccsid[] = "NLflatstr.c       1.10  com/lib/c/nls,3.1,9021 1/18/90 09:45:37";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NLflatstr = __NLflatstr
#endif
#endif
#include <NLctype.h>

/*
 * NAME: NLflatstr
 *
 * FUNCTION: Translate each NLS code point in src into the single 
 *           standard ASCII char most nearly matching the code 
 *           point's appearance.
 *
 * RETURN VALUE DESCRIPTION: The length of the converted char.
 */
/*
 *
 *  Translate each NLS code point in src into the single standard ASCII
 *  character most nearly matching the code point's appearance.
 */
#ifndef _KJI   	/*  This function not included on JLS platform. */
int
NLflatstr(src, dest, dlen)
register unsigned char *src, *dest;
register int dlen;	/* the length of a dest string */
{
	register int n;	/* the length of NLS code */
	unsigned char *odest = dest;	/* the next char position */

	/*  Always NUL-terminate output string, if any; but never count
	 *  NUL as part of length.
	 */
	for ( ; dlen && *src; --dlen)
		if (n = NLisNLcp(src)) {		/* NLS ? */
			*dest++ = NCflatchr(NCdechr(src));
				/* decode to NLchar and convert to ASCII */
			src += n;
		} else
			*dest++ = *src++;
	if (odest < dest) {	/* more char? */
		if (!dlen)
			--dest;
		*dest = '\0';
	}
	return (dest - odest);
}
#endif /* ndef _KJI */

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
static char	*sccsid = "@(#)$RCSfile: NCgetbuf.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:16:16 $";
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
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCgetbuf
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
 * NCgetbuf.c	1.6  com/lib/c/nls,3.1,8943 9/13/89 16:49:09
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/types.h>
#include <NLchar.h>
#include <stdlib.h>

/*
 * NAME: _NCgetbuf
 *
 * FUNCTION: Malloc a buffer of dlen NLchar's and decode string src into it.
 *
 * RETURN VALUE DESCRIPTION: NLchar ptr to the decoded string.
 */
/* 
 *  Malloc a buffer of dlen NLchar's and decode string src into it.
 */

NLchar *
_NCgetbuf(src, dlen)
char *src;       	/* a char string */
register int dlen;	/* the length of converted NLchar string */
{
	register NLchar *dest = (NLchar *)
		malloc((size_t)(dlen = (strlen(src) + 2) * sizeof (NLchar)));
			/* get a buffer for NLchar string */
	dest[0] = 1;
	(void)NCdecstr(src, &dest[1], (dlen / sizeof (NLchar)) - 1);
			/* decode to NLchar string */
	return (&dest[1]);
}
